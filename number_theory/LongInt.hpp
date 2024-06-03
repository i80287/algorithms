// #define NDEBUG 1

#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <new>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "fft.hpp"
#if defined(__has_include) && __has_include("integers_128_bit.hpp")
#include "integers_128_bit.hpp"
#endif
#include "math_functions.hpp"

#if !defined(__GNUC__)
#error "Current implementation works only with GCC"
#endif

namespace longint_allocator {

// #define DEBUG_LI_ALLOC_PRINTING

namespace inner_impl {

inline constexpr size_t kPageMemoryOffsetInBytes = sizeof(void*);

enum PageCapacity {
    kSmall  = 32 * sizeof(uint32_t) - kPageMemoryOffsetInBytes,
    kMiddle = 256 * sizeof(uint32_t) - kPageMemoryOffsetInBytes
};

struct SmallPage {
    SmallPage* next;
    std::byte memory[PageCapacity::kSmall];
};

struct MiddlePage {
    MiddlePage* next;
    std::byte memory[PageCapacity::kMiddle];
};

static_assert((sizeof(SmallPage) & (sizeof(SmallPage) - 1)) == 0, "");
static_assert((sizeof(MiddlePage) & (sizeof(MiddlePage) - 1)) == 0, "");

inline constexpr size_t kTotalSmallPages  = 32;
inline constexpr size_t kTotalMiddlePages = 8;

static SmallPage first_small_page[kTotalSmallPages]    = {};
static MiddlePage first_middle_page[kTotalMiddlePages] = {};

static SmallPage* free_small_pages_head   = nullptr;
static MiddlePage* free_middle_pages_head = nullptr;

#ifdef DEBUG_LI_ALLOC_PRINTING
static size_t total_small_pages_used     = 0;
static int32_t current_small_pages_used  = 0;
static int32_t max_small_pages_used      = -(1 << 30);
static size_t total_middle_pages_used    = 0;
static int32_t current_middle_pages_used = 0;
static int32_t max_middle_pages_used     = -(1 << 30);
static ssize_t bytes_allocated           = 0;
static int32_t malloc_free_count         = 0;
#endif

static void InitSmallPages() noexcept {
    SmallPage* p          = first_small_page;
    free_small_pages_head = p;
    for (const SmallPage* last_page = p + kTotalSmallPages - 1; p != last_page;) {
        SmallPage* p_next = p + 1;
        p->next           = p_next;
        p                 = p_next;
    }
    p->next = nullptr;
}

static void InitMiddlePages() noexcept {
    MiddlePage* p          = first_middle_page;
    free_middle_pages_head = p;
    for (const MiddlePage* p_iter_end = p + kTotalMiddlePages - 1; p != p_iter_end;) {
        MiddlePage* p_next = p + 1;
        p->next            = p_next;
        p                  = p_next;
    }
    p->next = nullptr;
}

/**
 * VS Code intellisense: "attribute "constructor" does not take arguments
 * C/C++(1094)"
 */
#if defined(__INTELLISENSE__) && __INTELLISENSE__
#pragma diag_suppress 1094
#endif

__attribute__((constructor(101))) static void InitPages() noexcept {
    InitSmallPages();
    InitMiddlePages();
#ifdef DEBUG_LI_ALLOC_PRINTING
    printf("[INIT] Inited pages in %s\n", FUNCTION_MACRO);
#endif
}

__attribute__((destructor(101))) static void DeinitPages() noexcept {
    free_small_pages_head  = nullptr;
    free_middle_pages_head = nullptr;
#ifdef DEBUG_LI_ALLOC_PRINTING
    printf(
        "[DEINIT] Deinited pages in %s\n"
        "[SMALL]:\n"
        "    total pages rented: %zu\n"
        "    max pages allocated per one time: %d\n"
        "    current pages allocated: %d\n"
        "[MIDDLE]:\n"
        "    total pages rented: %zu\n"
        "    max pages allocated per one time: %d\n"
        "    current pages allocated: %d\n"
        "[MALLOC]:\n"
        "    total bytes allocated: %zd\n"
        "    malloc calls - free calls: %d\n",
        FUNCTION_MACRO, total_small_pages_used, max_small_pages_used, current_small_pages_used,
        total_middle_pages_used, max_middle_pages_used, current_middle_pages_used, bytes_allocated,
        malloc_free_count);
#endif
}

static bool IsSmallPage(const std::byte* offset_memory) noexcept {
    const std::byte* fsp = reinterpret_cast<const std::byte*>(&first_small_page[0]);
    return static_cast<size_t>(offset_memory - fsp) < kTotalSmallPages * sizeof(SmallPage);
}

static bool IsMiddlePage(const std::byte* offset_memory) noexcept {
    const std::byte* fmp = reinterpret_cast<const std::byte*>(&first_middle_page[0]);
    return static_cast<size_t>(offset_memory - fmp) < kTotalMiddlePages * sizeof(MiddlePage);
}

}  // namespace inner_impl

void* Allocate(size_t size) noexcept(false) {
    if (size <= inner_impl::PageCapacity::kSmall && inner_impl::free_small_pages_head != nullptr) {
        inner_impl::SmallPage* p = inner_impl::free_small_pages_head;
#ifdef DEBUG_LI_ALLOC_PRINTING
        inner_impl::current_small_pages_used++;
        if (inner_impl::current_small_pages_used > inner_impl::max_small_pages_used) {
            inner_impl::max_small_pages_used = inner_impl::current_small_pages_used;
        }
        inner_impl::total_small_pages_used++;
#endif
        inner_impl::free_small_pages_head = p->next;
        return static_cast<void*>(reinterpret_cast<std::byte*>(p) +
                                  inner_impl::kPageMemoryOffsetInBytes);
    }

    if (size <= inner_impl::PageCapacity::kMiddle &&
        inner_impl::free_middle_pages_head != nullptr) {
        inner_impl::MiddlePage* p = inner_impl::free_middle_pages_head;
#ifdef DEBUG_LI_ALLOC_PRINTING
        inner_impl::current_middle_pages_used++;
        if (inner_impl::current_middle_pages_used > inner_impl::max_middle_pages_used) {
            inner_impl::max_middle_pages_used = inner_impl::current_middle_pages_used;
        }
        inner_impl::total_middle_pages_used++;
#endif
        inner_impl::free_middle_pages_head = p->next;
        return static_cast<void*>(reinterpret_cast<char*>(p) +
                                  inner_impl::kPageMemoryOffsetInBytes);
    }

    void* p = operator new(size);
#ifdef DEBUG_LI_ALLOC_PRINTING
    inner_impl::bytes_allocated += static_cast<ssize_t>(size);
    inner_impl::malloc_free_count++;
#endif
    return p;
}

void Deallocate(void* memory) noexcept {
    if (unlikely(memory == nullptr)) {
        return;
    }

    std::byte* p = static_cast<std::byte*>(memory) - inner_impl::kPageMemoryOffsetInBytes;
    if (inner_impl::IsSmallPage(p)) {
#ifdef DEBUG_LI_ALLOC_PRINTING
        inner_impl::current_small_pages_used--;
#endif
        inner_impl::SmallPage* page       = reinterpret_cast<inner_impl::SmallPage*>(p);
        page->next                        = inner_impl::free_small_pages_head;
        inner_impl::free_small_pages_head = page;
        return;
    }

    if (inner_impl::IsMiddlePage(p)) {
#ifdef DEBUG_LI_ALLOC_PRINTING
        inner_impl::current_middle_pages_used--;
#endif
        inner_impl::MiddlePage* page       = reinterpret_cast<inner_impl::MiddlePage*>(p);
        page->next                         = inner_impl::free_middle_pages_head;
        inner_impl::free_middle_pages_head = page;
        return;
    }
#ifdef DEBUG_LI_ALLOC_PRINTING
    inner_impl::malloc_free_count--;
#endif
    operator delete(memory);
}

#ifdef DEBUG_LI_ALLOC_PRINTING
#undef DEBUG_LI_ALLOC_PRINTING
#endif

}  // namespace longint_allocator

struct LongInt {
    static constexpr size_t kDefaultLINumsCapacity = 2;
    static constexpr uint32_t kStrConvBase         = 1'000'000'000;
    static constexpr uint32_t kStrConvBaseDigits   = math_functions::base_b_len(kStrConvBase - 1);
    static constexpr uint32_t kNumsBits            = 32;
    static constexpr uint64_t kNumsBase            = uint64_t(1) << kNumsBits;
    static constexpr size_t kFFTPrecisionBorder    = 262144;

    uint32_t* nums_ = nullptr;
    /**
     * size_ < 0 <=> sign = -1; size_ == 0 <=> sign = 0; size > 0 <=> sign = 1
     */
    int32_t size_      = 0;
    uint32_t capacity_ = 0;

    constexpr LongInt() noexcept = default;

    LongInt(uint32_t n) : size_(n != 0) {
        AllocateDefaultCapacity();
        nums_[0] = n;
    }

    LongInt(int32_t n) {
        AllocateDefaultCapacity();
        int32_t sgn = int32_t(n > 0) - int32_t(n < 0);
        nums_[0]    = n > 0 ? uint32_t(n) : -uint32_t(n);
        size_       = sgn;
    }

    LongInt(uint64_t n) : size_(n != 0) {
        AllocateDefaultCapacity();
        nums_[0] = uint32_t(n);
        n >>= 32;
        size_ += n != 0;
        nums_[1] = uint32_t(n);
    }

    LongInt(int64_t n) {
        AllocateDefaultCapacity();
        int32_t sgn = int32_t(n > 0) - int32_t(n < 0);
        uint64_t m  = n > 0 ? uint64_t(n) : -uint64_t(n);
        size_       = m != 0;
        nums_[0]    = uint32_t(m);
        m >>= 32;
        size_ += m != 0;
        nums_[1] = uint32_t(m);
        size_ *= sgn;
    }

#if defined(INTEGERS_128_BIT_HPP)
    LongInt(uint128_t n) : size_(n != 0), capacity_(4) {
        nums_    = static_cast<uint32_t*>(longint_allocator::Allocate(4 * sizeof(uint32_t)));
        nums_[0] = uint32_t(n);
        n >>= 32;
        size_ += n != 0;
        nums_[1] = uint32_t(n);
        n >>= 32;
        size_ += n != 0;
        nums_[2] = uint32_t(n);
        n >>= 32;
        size_ += n != 0;
        nums_[3] = uint32_t(n);
    }
#endif

    LongInt& pow(size_t p) {
        LongInt res = uint32_t(1);
        Reserve(uint32_t(USize() * p));
        while (true) {
            if (p & 1) {
                res *= *this;
            }
            p >>= 1;
            if (p == 0) {
                break;
            }
            this->Square();
        }
        *this = std::move(res);
        return *this;
    }

    void SquareThisTo(LongInt& other) const {
        size_t usize = USize();
        if (unlikely(usize == 0)) {
            other.size_ = 0;
            return;
        }
        const uint32_t* nums_ptr = nums_;
        size_t prod_size         = usize + usize;
        if (prod_size <= 16) {
            other.capacity_ = uint32_t(prod_size);
            uint32_t* ans =
                static_cast<uint32_t*>(longint_allocator::Allocate(prod_size * sizeof(uint32_t)));
            std::memset(ans, 0, prod_size * sizeof(uint32_t));
            for (size_t j = 0; j < usize; j++) {
                uint64_t b_j   = nums_ptr[j];
                uint64_t carry = 0;
                for (size_t i = 0; i < usize; i++) {
                    uint64_t a_i = nums_ptr[i];
                    uint64_t res = a_i * b_j + uint64_t(ans[j + i]) + carry;
                    ans[j + i]   = uint32_t(res);
                    carry        = res >> 32;
                }

                ans[j + usize] = uint32_t(carry);
            }

            longint_allocator::Deallocate(other.nums_);
            other.nums_ = ans;
        } else {
            size_t n                  = 2 * math_functions::nearest_pow2_ge(prod_size);
            const bool high_precision = n > kFFTPrecisionBorder;
            n <<= high_precision;
            // Allocate n complex numbers for p1 and n complex numbers for p2
            fft::complex* p1 = static_cast<fft::complex*>(
                longint_allocator::Allocate(2 * n * sizeof(fft::complex)));
            fft::complex* p = p1;
            if (likely(!high_precision)) {
                for (size_t i = 0; i < usize; i++) {
                    uint32_t value = nums_ptr[i];
                    *p             = fft::complex(value & 0xFFFF, value & 0xFFFF);
                    p++;
                    value >>= 16;
                    *p = fft::complex(value, value);
                    p++;
                }
            } else {
                for (size_t i = 0; i < usize; i++) {
                    uint32_t value = nums_ptr[i];
                    *p             = fft::complex(uint8_t(value), uint8_t(value));
                    p++;
                    value >>= 8;
                    *p = fft::complex(uint8_t(value), uint8_t(value));
                    p++;
                    value >>= 8;
                    *p = fft::complex(uint8_t(value), uint8_t(value));
                    p++;
                    value >>= 8;
                    *p = fft::complex(value, value);
                    p++;
                }
            }
            std::memset(static_cast<void*>(p), 0,
                        (n - (prod_size << high_precision)) * sizeof(fft::complex));

            other.ReserveWithoutCopy(uint32_t(prod_size));
            fft::complex* p2 = p1 + n;
            fft::forward_backward_fft(p1, p2, n);
            uint64_t carry            = 0;
            uint32_t* ans_p           = other.nums_;
            const uint32_t* ans_p_end = ans_p + prod_size;
            if (likely(!high_precision)) {
                do {
                    uint64_t res = carry;
                    res += uint64_t(p2->real() + 0.5);
                    p2++;
                    res += uint64_t(p2->real() + 0.5) << 16;
                    p2++;
                    *ans_p = uint32_t(res);
                    carry  = res >> 32;
                    ans_p++;
                } while (ans_p != ans_p_end);
            } else {
                do {
                    uint64_t res = carry;
                    res += uint64_t(p2->real() + 0.5);
                    p2++;
                    res += uint64_t(p2->real() + 0.5) << 8;
                    p2++;
                    res += uint64_t(p2->real() + 0.5) << 16;
                    p2++;
                    res += uint64_t(p2->real() + 0.5) << 24;
                    p2++;
                    *ans_p = uint32_t(res);
                    carry  = res >> 32;
                    ans_p++;
                } while (ans_p != ans_p_end);
            }
            assert(carry == 0);
            longint_allocator::Deallocate(p1);
        }

        other.size_ = int32_t(prod_size);
        other.PopLeadingZeros();
    }

    LongInt& Square() {
        SquareThisTo(*this);
        return *this;
    }

    explicit LongInt(std::string_view s) {
        set_string(s);
    }

    LongInt(const LongInt& other) : nums_(nullptr), size_(other.size_), capacity_(other.capacity_) {
        if (capacity_ != 0) {
            nums_ =
                static_cast<uint32_t*>(longint_allocator::Allocate(capacity_ * sizeof(uint32_t)));
            size_t bsz = USize() * sizeof(uint32_t);
            if (bsz != 0) {
                std::memcpy(nums_, other.nums_, bsz);
            }
        }
    }

    LongInt& operator=(const LongInt& other) {
        return *this = LongInt(other);
    }

    constexpr LongInt(LongInt&& other) noexcept
        : nums_(other.nums_), size_(other.size_), capacity_(other.capacity_) {
        other.nums_     = nullptr;
        other.size_     = 0;
        other.capacity_ = 0;
    }

    LongInt& operator=(LongInt&& other) noexcept {
        uint32_t* tmp_nums    = other.nums_;
        int32_t tmp_size      = other.size_;
        uint32_t tmp_capacity = other.capacity_;
        other.nums_           = nullptr;
        other.size_           = 0;
        other.capacity_       = 0;
        longint_allocator::Deallocate(nums_);
        nums_     = tmp_nums;
        size_     = tmp_size;
        capacity_ = tmp_capacity;
        return *this;
    }

#if __cplusplus >= 202002L
    constexpr
#endif
        void
        Swap(LongInt& other) noexcept {
        std::swap(nums_, other.nums_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    LongInt& operator=(int32_t n) {
        EnsureDefaultCapacityOpEqCall();
        size_    = int32_t(n > 0) - int32_t(n < 0);
        nums_[0] = n > 0 ? uint32_t(n) : -uint32_t(n);
        return *this;
    }

    LongInt& operator=(uint32_t n) {
        EnsureDefaultCapacityOpEqCall();
        size_    = n != 0;
        nums_[0] = n;
        return *this;
    }

    LongInt& operator=(int64_t n) {
        EnsureDefaultCapacityOpEqCall();
        int32_t sgn = int32_t(n > 0) - int32_t(n < 0);
        uint64_t m  = n > 0 ? uint64_t(n) : -uint64_t(n);
        size_       = m != 0;
        nums_[0]    = uint32_t(m);
        m >>= 32;
        size_ += m != 0;
        nums_[1] = uint32_t(m);
        size_ *= sgn;
        return *this;
    }

    LongInt& operator=(uint64_t n) {
        EnsureDefaultCapacityOpEqCall();
        size_    = n != 0;
        nums_[0] = uint32_t(n);
        n >>= 32;
        size_ += n != 0;
        nums_[1] = uint32_t(n);
        return *this;
    }

#if defined(INTEGERS_128_BIT_HPP)
    LongInt& operator=(uint128_t n) {
        if (capacity_ < 4) {
            longint_allocator::Deallocate(nums_);
            nums_     = static_cast<uint32_t*>(longint_allocator::Allocate(4 * sizeof(uint32_t)));
            capacity_ = 4;
        }

        // size_ = ((128 - std::count_leading_zeros(n)) + 31) / 32;
        size_    = n != 0;
        nums_[0] = uint32_t(n);
        n >>= 32;
        size_ += n != 0;
        nums_[1] = uint32_t(n);
        n >>= 32;
        size_ += n != 0;
        nums_[2] = uint32_t(n);
        n >>= 32;
        size_ += n != 0;
        nums_[3] = uint32_t(n);
        return *this;
    }
#endif

    constexpr uint32_t operator[](size_t pos) const noexcept {
        return nums_[pos];
    }

    LongInt& operator*=(const LongInt& other) {
        size_t k = USize();
        size_t m = other.USize();
        const uint32_t* k_ptr;
        const uint32_t* m_ptr;

        if (m <= k) {
            k_ptr = nums_;
            m_ptr = other.nums_;
        } else {
            k_ptr = other.nums_;
            m_ptr = nums_;
            std::swap(m, k);
        }

        if (unlikely(m == 0)) {
            size_ = 0;
            return *this;
        }

        size_t prod_size = m + k;
        if (m <= 16 || m * k <= 1024) {
            capacity_ = uint32_t(prod_size);
            uint32_t* ans =
                static_cast<uint32_t*>(longint_allocator::Allocate(prod_size * sizeof(uint32_t)));
            std::memset(ans, 0, prod_size * sizeof(uint32_t));
            for (size_t j = 0; j < m; j++) {
                uint64_t b_j   = m_ptr[j];
                uint64_t carry = 0;
                for (size_t i = 0; i < k; i++) {
                    uint64_t a_i = k_ptr[i];
                    uint64_t res = a_i * b_j + uint64_t(ans[j + i]) + carry;
                    ans[j + i]   = uint32_t(res);
                    carry        = res >> 32;
                }

                ans[j + k] = uint32_t(carry);
            }

            longint_allocator::Deallocate(nums_);
            nums_ = ans;
        } else {
            size_t n                  = 2 * math_functions::nearest_pow2_ge(prod_size);
            const bool high_precision = n > kFFTPrecisionBorder;
            n <<= high_precision;
            // Allocate n complex numbers for p1 and n complex numbers for p2
            fft::complex* p1 = static_cast<fft::complex*>(
                longint_allocator::Allocate(2 * n * sizeof(fft::complex)));
            fft::complex* p = p1;
            if (likely(!high_precision)) {
                for (size_t i = 0; i < m; i++) {
                    uint32_t m_value = m_ptr[i];
                    uint32_t k_value = k_ptr[i];
                    *p               = fft::complex(m_value & 0xFFFF, k_value & 0xFFFF);
                    p++;
                    *p = fft::complex(m_value >> 16, k_value >> 16);
                    p++;
                }
                for (size_t i = m; i < k; i++) {
                    uint32_t k_value = k_ptr[i];
                    *p               = fft::complex(0, k_value & 0xFFFF);
                    p++;
                    *p = fft::complex(0, k_value >> 16);
                    p++;
                }
            } else {
                for (size_t i = 0; i < m; i++) {
                    uint32_t m_value = m_ptr[i];
                    uint32_t k_value = k_ptr[i];
                    *p               = fft::complex(uint8_t(m_value), uint8_t(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = fft::complex(uint8_t(m_value), uint8_t(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = fft::complex(uint8_t(m_value), uint8_t(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = fft::complex(uint8_t(m_value), uint8_t(k_value));
                    p++;
                }
                for (size_t i = m; i < k; i++) {
                    uint32_t k_value = k_ptr[i];
                    *p               = fft::complex(0, uint8_t(k_value));
                    p++;
                    k_value >>= 8;
                    *p = fft::complex(0, uint8_t(k_value));
                    p++;
                    k_value >>= 8;
                    *p = fft::complex(0, uint8_t(k_value));
                    p++;
                    k_value >>= 8;
                    *p = fft::complex(0, uint8_t(k_value));
                    p++;
                }
            }
            std::memset(static_cast<void*>(p), 0,
                        (n - ((2 * k) << high_precision)) * sizeof(fft::complex));

            ReserveWithoutCopy(uint32_t(prod_size));
            fft::complex* p2 = p1 + n;
            fft::forward_backward_fft(p1, p2, n);
            uint64_t carry            = 0;
            uint32_t* ans_p           = nums_;
            const uint32_t* ans_p_end = ans_p + prod_size;
            if (likely(!high_precision)) {
                do {
                    uint64_t res = carry;
                    res += uint64_t(p2->real() + 0.5);
                    p2++;
                    res += uint64_t(p2->real() + 0.5) << 16;
                    p2++;
                    *ans_p = uint32_t(res);
                    carry  = res >> 32;
                    ans_p++;
                } while (ans_p != ans_p_end);
            } else {
                do {
                    uint64_t res = carry;
                    res += uint64_t(p2->real() + 0.5);
                    p2++;
                    res += uint64_t(p2->real() + 0.5) << 8;
                    p2++;
                    res += uint64_t(p2->real() + 0.5) << 16;
                    p2++;
                    res += uint64_t(p2->real() + 0.5) << 24;
                    p2++;
                    *ans_p = uint32_t(res);
                    carry  = res >> 32;
                    ans_p++;
                } while (ans_p != ans_p_end);
            }
            assert(carry == 0);
            longint_allocator::Deallocate(p1);
        }

        int32_t sign_product = size_ ^ other.size_;
        size_                = sign_product >= 0 ? int32_t(prod_size) : int32_t(-prod_size);
        PopLeadingZeros();
        return *this;
    }

    LongInt operator*(const LongInt& other) const {
        LongInt copy(*this);
        copy *= other;
        return copy;
    }

    void DivMod(const LongInt& other, LongInt& rem) {
        /**
         * See Hackers Delight 9-2.
         */
        size_t m = USize();
        size_t n = other.USize();
        if (m < n) {
            rem   = std::move(*this);
            size_ = 0;
            return;
        }

        switch (n) {
            case 0:
                /* Quite return when division by zero. */
                return;
            case 1:
                rem = DivMod(other[0]);
                return;
        }

        const uint32_t* u = nums_;
        const uint32_t* v = other.nums_;

        rem.ReserveWithoutCopy(uint32_t(n));
        rem.size_ = int32_t(n);

        // Normilize by shifting v left just enough so that
        // its high-order bit i on, and shift u left the
        // same amount. We may have to append a high-order
        // digit on the dividend; we do that unconditionally.

        // 0 <= s < kNumsBits = 32
        uint32_t s   = static_cast<uint32_t>(math_functions::countl_zero(v[n - 1]));
        uint32_t* vn = static_cast<uint32_t*>(longint_allocator::Allocate(2 * n));
        for (size_t i = n - 1; i > 0; i--) {
            vn[i] = (v[i] << s) | (v[i - 1] >> (kNumsBase - s));
        }
        vn[0] = v[0] << s;

        uint32_t* un = static_cast<uint32_t*>(longint_allocator::Allocate(2 * (m + 1)));
        un[m]        = u[m - 1] >> (kNumsBase - s);
        for (size_t i = m - 1; i > 0; i--) {
            un[i] = (u[i] << s) | (u[i - 1] >> (kNumsBase - s));
        }
        un[0] = u[0] << s;

        uint32_t* quot = nums_;

        for (size_t j = m - n; static_cast<ssize_t>(j) >= 0; j--) {
            // Compute estimate qhat of q[j].
            uint64_t cur     = (uint64_t(un[j + n]) << kNumsBits) | un[j + n - 1];
            uint32_t last_vn = vn[n - 1];
            uint64_t qhat    = cur / last_vn;
            uint64_t rhat    = cur - qhat * last_vn;

            while (qhat >= kNumsBase || qhat * vn[n - 2] > kNumsBase * rhat + un[j + n - 2]) {
                qhat--;
                rhat += vn[n - 1];
                if (rhat >= kNumsBase) {
                    break;
                }
            }

            // Multiply and subtract
            uint64_t carry = 0;
            for (size_t i = 0; i < n; i++) {
                uint64_t p = qhat * vn[i];
                uint64_t t = uint64_t(un[i + j]) - carry - uint32_t(p);
                un[i + j]  = uint32_t(t);
                carry      = (p >> kNumsBits) - (t >> kNumsBits);
            }
            uint64_t t = un[j + n] - carry;
            un[j + n]  = uint32_t(t);

            quot[j] = uint32_t(qhat);  // Store quotient digit
            if (static_cast<int64_t>(t) < 0) {
                // If we subtracted too much, add back
                quot[j]--;
                carry = 0;
                for (size_t i = 0; i < n; i++) {
                    t         = uint64_t(un[i + j]) + uint64_t(vn[i]) + carry;
                    un[i + j] = uint32_t(t);
                    carry     = t >> kNumsBits;
                }
                un[j + n] += uint32_t(carry);
            }
        }

        // Unnormalize remainder
        for (size_t i = 0; i < n; i++) {
            rem.nums_[i] = (un[i] >> s) | (un[i + 1] << (16 - s));
        }

        longint_allocator::Deallocate(un);
        longint_allocator::Deallocate(vn);

        rem.PopLeadingZeros();
        size_ = int32_t(m - n + 1);
        PopLeadingZeros();
    }

    LongInt& operator+=(const LongInt& other) {
        size_t usize2 = other.USize();
        if ((size_ ^ other.size_) >= 0) {
            size_t usize1               = SetSizeAtLeast(usize2 + 1);
            uint64_t add_overflow_carry = LongIntAdd(nums_, other.nums_, usize1, usize2);
            if (likely(add_overflow_carry == 0)) {
                PopLeadingZeros();
            } else {
                size_t new_usize1     = GrowSizeByOne();
                nums_[new_usize1 - 1] = uint32_t(add_overflow_carry);
            }
        } else {
            assert(false);
            // LongIntSubtract(*this, other.nums_, usize2);
        }

        return *this;
    }

    constexpr bool operator==(int32_t n) const noexcept {
        switch (size_) {
            case 0:
                return n == 0;
            case 1:
                return nums_[0] == uint32_t(n) && n > 0;
            case -1:
                return nums_[0] == -uint32_t(n) && n < 0;
            default:
                return false;
        }
    }

    constexpr bool operator==(int64_t n) const noexcept {
        bool not_same_sign = (size_ >= 0) == (n < 0);
        if (not_same_sign) {
            return false;
        }

        /* Do not use std::abs in order to make operator== constexpr */
        uint64_t n_abs = math_functions::uabs(n);
        switch (size_) {
            case 0:
                return n == 0;
            case 1:
            case -1:
                return nums_[0] == n_abs;
            case 2:
            case -2:
                return ((uint64_t(nums_[1]) << 32) | nums_[0]) == n_abs;
            default:
                return false;
        }
    }

    constexpr bool operator==(uint32_t n) const noexcept {
        switch (size_) {
            case 0:
                return n == 0;
            case 1:
                return nums_[0] == n;
            default:
                return false;
        }
    }

    constexpr bool operator!=(uint32_t n) const noexcept {
        return !(*this == n);
    }

    constexpr bool operator==(uint64_t n) const noexcept {
        switch (size_) {
            case 0:
                return n == 0;
            case 1:
                return nums_[0] == n;
            case 2:
                return ((uint64_t(nums_[1]) << 32) | nums_[0]) == n;
            default:
                return false;
        }
    }

    constexpr bool operator!=(uint64_t n) const noexcept {
        return !(*this == n);
    }

#if defined(INTEGERS_128_BIT_HPP)
    constexpr bool operator==(uint128_t n) const noexcept {
        switch (size_) {
            case 0:
                return n == 0;
            case 1:
                return nums_[0] == n;
            case 2:
                return ((uint64_t(nums_[1]) << 32) | nums_[0]) == n;
            case 3: {
                uint64_t low = (uint64_t(nums_[1]) << 32) | nums_[0];
                return ((uint128_t(nums_[2]) << 64) | low) == n;
            }
            case 4: {
                uint64_t low = (uint64_t(nums_[1]) << 32) | nums_[0];
                uint64_t hi  = (uint64_t(nums_[3]) << 32) | nums_[2];
                return ((uint128_t(hi) << 64) | low) == n;
            }
            default:
                return false;
        }
    }

    constexpr bool operator!=(uint128_t n) const noexcept {
        return !(*this == n);
    }
#endif

    constexpr bool operator==(const LongInt& other) const noexcept {
        return size_ == other.size_ && std::equal(nums_, nums_ + USize(), other.nums_);
    }

#if CONFIG_HAS_AT_LEAST_CXX_20
    constexpr std::strong_ordering operator<=>(const LongInt& other) const noexcept {
        if (size_ != other.size_) {
            return size_ <=> other.size_;
        }

        size_t usize                 = USize();
        const uint32_t* r_end        = nums_ - 1;
        const uint32_t* r_nums       = r_end + usize;
        const uint32_t* r_other_nums = other.nums_ - 1 + usize;
        for (; r_nums != r_end; r_nums--, r_other_nums--) {
            if (*r_nums != *r_other_nums) {
                return int64_t(*r_nums) * sign() <=> int64_t(*r_other_nums) * other.sign();
            }
        }

        return std::strong_ordering::equivalent;
    }
#else
    constexpr bool operator<(const LongInt& other) const noexcept {
        if (size_ != other.size_) {
            return size_ < other.size_;
        }

        size_t usize                 = USize();
        const uint32_t* r_end        = nums_ - 1;
        const uint32_t* r_nums       = r_end + usize;
        const uint32_t* r_other_nums = other.nums_ - 1 + usize;
        for (; r_nums != r_end; r_nums--, r_other_nums--) {
            if (*r_nums != *r_other_nums) {
                return int64_t(*r_nums) * sign() < int64_t(*r_other_nums) * other.sign();
            }
        }

        return false;
    }

    constexpr bool operator>(const LongInt& other) const noexcept {
        return other < *this;
    }

    constexpr bool operator<=(const LongInt& other) const noexcept {
        return !(*this > other);
    }

    constexpr bool operator>=(const LongInt& other) const noexcept {
        return !(*this < other);
    }
#endif

    LongInt& operator+=(uint32_t n) {
        if (unlikely(size_ == 0)) {
            if (capacity_ == 0) {
                nums_ = static_cast<uint32_t*>(longint_allocator::Allocate(2 * sizeof(uint32_t)));
                capacity_ = 2;
            }

            nums_[0] = n;
            size_    = n != 0;
            return *this;
        }

        assert(capacity_ != 0);
        if (size_ > 0) {
            NonZeroSizeAddUInt(n);
        } else {
            NonZeroSizeSubUInt(n);
        }

        return *this;
    }

    LongInt& operator-=(uint32_t n) {
        if (unlikely(size_ == 0)) {
            if (capacity_ == 0) {
                nums_ = static_cast<uint32_t*>(longint_allocator::Allocate(2 * sizeof(uint32_t)));
                capacity_ = 2;
            }

            nums_[0] = n;
            size_    = -int32_t(n != 0);
            return *this;
        }

        assert(capacity_ != 0);
        if (size_ > 0) {
            NonZeroSizeSubUInt(n);
        } else {
            NonZeroSizeAddUInt(n);
        }

        return *this;
    }

    LongInt& operator*=(uint32_t x) {
        if (unlikely(x == 0)) {
            size_ = 0;
            return *this;
        }

        uint64_t carry   = 0;
        uint64_t b_0     = x;
        uint32_t u32size = math_functions::uabs(size_);
        for (uint32_t *nums_it = nums_, *numes_it_end = nums_it + u32size; nums_it != numes_it_end;
             ++nums_it) {
            uint64_t res = *nums_it * b_0 + carry;
            *nums_it     = uint32_t(res);
            carry        = res >> 32;
        }

        // x != 0 => sign wont change and there will be no leading zeros
        if (carry != 0) {
            if (unlikely(u32size == capacity_)) {
                GrowCapacity();
            }

            assert(u32size < capacity_);
            nums_[u32size] = uint32_t(carry);
            size_ += sign();
        }

        return *this;
    }

    constexpr LongInt& operator/=(uint32_t n) noexcept {
#if (defined(__cpp_lib_is_constant_evaluated) && __cpp_lib_is_constant_evaluated >= 201811L) || \
    (defined(__GNUC__) && defined(__has_builtin) && __has_builtin(__builtin_constant_p))
#if defined(__cpp_lib_is_constant_evaluated) && __cpp_lib_is_constant_evaluated >= 201811L
        if (std::is_constant_evaluated() && (n & (n - 1)) == 0)
#else
        if (__builtin_constant_p(n) && (n & (n - 1)) == 0)
#endif
        {
            if (n == 0) {
                assert(false && "Division by zero in LongInt::operator/=");
            } else if (n > 1) {
                operator>>=(uint32_t(math_functions::countl_zero(n)));
            }
            return *this;
        }
#endif
        DivMod(n);
        return *this;
    }

    constexpr uint32_t DivMod(uint32_t n) noexcept {
        uint64_t carry          = 0;
        uint32_t* nums_iter_end = nums_ - 1;
        uint32_t* nums_iter     = nums_iter_end + USize();
        for (; nums_iter != nums_iter_end; --nums_iter) {
            uint64_t cur = (carry << 32) | uint64_t(*nums_iter);
            uint64_t q   = cur / n;
            uint64_t r   = cur - q * n;
            *nums_iter   = uint32_t(q);
            carry        = r;
        }

        PopLeadingZeros();
        return uint32_t(carry);
    }

    constexpr LongInt& operator>>=(uint32_t shift) noexcept {
        size_t usize        = USize();
        uint32_t uints_move = shift / 32;
        if (uints_move >= usize) {
            size_ = 0;
            return *this;
        }

        if (uints_move != 0) {
            usize -= uints_move;
            size_                          = size_ >= 0 ? int32_t(usize) : -int32_t(usize);
            uint32_t* copy_dst_start       = nums_;
            const uint32_t* copy_src_start = copy_dst_start + uints_move;
            const uint32_t* copy_src_end   = copy_src_start + usize;
            std::copy(copy_src_start, copy_src_end, copy_dst_start);
        }

        shift %= 32;
        uint32_t* nums_iter            = nums_;
        uint32_t* const nums_iter_last = nums_iter + ssize_t(usize) - 1;
        for (; nums_iter != nums_iter_last; ++nums_iter) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            // *nums_iter = uint32_t(*reinterpret_cast<uint64_t*>(nums_iter) >> shift);
            *nums_iter = uint32_t(((*nums_iter) | (uint64_t(*(nums_iter + 1)) << 32)) >> shift);
#else
            uint32_t hi   = *(nums_iter + 1);
            uint32_t low  = *nums_iter;
            uint32_t mask = (1 << shift) - 1;
            *nums_iter    = ((hi & mask) << (32 - shift)) | (low >> shift);
#endif
        }

        *nums_iter_last >>= shift;
        if (*nums_iter_last == 0) {
            size_ += size_ >= 0 ? -1 : 1;
        }

        return *this;
    }

    constexpr void PopLeadingZeros() noexcept {
        size_t usize = USize();
        while (usize != 0 && nums_[usize - 1] == 0) {
            usize--;
        }

        size_ = size_ >= 0 ? int32_t(usize) : -int32_t(usize);
    }

    constexpr bool iszero() const noexcept {
        return size_ == 0;
    }
    constexpr bool empty() const noexcept {
        return iszero();
    }
    constexpr operator bool() noexcept {
        return !iszero();
    }
    constexpr int32_t size() const noexcept {
        return size_;
    }
    constexpr uint32_t* begin() noexcept {
        return nums_;
    }
    constexpr uint32_t* end() noexcept {
        return nums_ + USize();
    }
    constexpr const uint32_t* begin() const noexcept {
        return nums_;
    }
    constexpr const uint32_t* end() const noexcept {
        return nums_ + USize();
    }
    constexpr const uint32_t* cbegin() const noexcept {
        return begin();
    }
    constexpr const uint32_t* cend() const noexcept {
        return end();
    }
    constexpr std::reverse_iterator<uint32_t*> rbegin() noexcept {
        return std::make_reverse_iterator(end());
    }
    constexpr std::reverse_iterator<uint32_t*> rend() noexcept {
        return std::make_reverse_iterator(begin());
    }
    constexpr std::reverse_iterator<const uint32_t*> rbegin() const noexcept {
        return std::make_reverse_iterator(end());
    }
    constexpr std::reverse_iterator<const uint32_t*> rend() const noexcept {
        return std::make_reverse_iterator(begin());
    }
    constexpr size_t USize() const noexcept {
        // std::abs is not used in order to make method constexpr
        return size_t(math_functions::uabs(size_));
    }
    constexpr int32_t sign() const noexcept {
        return int32_t(size_ > 0) - int32_t(size_ < 0);
    }
    constexpr void change_sign() noexcept {
        size_ = -size_;
    }

    void set_string(std::string_view s) {
#if CONFIG_HAS_AT_LEAST_CXX_20
        const unsigned char* str_iter = std::bit_cast<const unsigned char*>(s.begin());
        const unsigned char* str_end  = std::bit_cast<const unsigned char*>(s.end());
#else
        const unsigned char* str_iter = reinterpret_cast<const unsigned char*>(s.begin());
        const unsigned char* str_end  = reinterpret_cast<const unsigned char*>(s.end());
#endif
        int32_t sgn = 1;
        while (str_iter != str_end && !std::isdigit(*str_iter)) {
            sgn = 1 - int32_t(uint32_t(*str_iter == '-') << 1);
            ++str_iter;
        }

        while (str_iter != str_end && *str_iter == '0') {
            ++str_iter;
        }

        size_t digits_count = static_cast<size_t>(str_end - str_iter);
        if (digits_count <= 19) {
            uint64_t num = 0;
            for (; str_iter != str_end; ++str_iter) {
                num = num * 10 + uint64_t(*str_iter) - '0';
            }

            *this = num;
            if (sgn < 0) {
                size_ = -size_;
            }
            return;
        }

        const size_t str_conv_digits_size =
            (digits_count + kStrConvBaseDigits - 1) / kStrConvBaseDigits;
        const size_t aligned_str_conv_digits_size =
            math_functions::nearest_pow2_ge(str_conv_digits_size);
        uint32_t* str_conv_digits;
        ReserveWithoutCopy(uint32_t(aligned_str_conv_digits_size));
        str_conv_digits = nums_;

        {
            uint32_t* str_conv_digits_iter = str_conv_digits + str_conv_digits_size;
            std::memset(str_conv_digits_iter, 0,
                        (aligned_str_conv_digits_size - str_conv_digits_size) * sizeof(uint32_t));
            if (size_t offset = digits_count % kStrConvBaseDigits) {
                uint32_t current = 0;
                do {
                    current = current * 10 + uint32_t(*str_iter) - '0';
                    str_iter++;
                } while (--offset);
                *--str_conv_digits_iter = current;
            }

            do {
                static_assert(kStrConvBaseDigits == 9, "");
                uint32_t current = uint32_t(*str_iter) - '0';
                str_iter++;
                current = current * 10 + uint32_t(*str_iter) - '0';
                str_iter++;
                current = current * 10 + uint32_t(*str_iter) - '0';
                str_iter++;
                current = current * 10 + uint32_t(*str_iter) - '0';
                str_iter++;
                current = current * 10 + uint32_t(*str_iter) - '0';
                str_iter++;
                current = current * 10 + uint32_t(*str_iter) - '0';
                str_iter++;
                current = current * 10 + uint32_t(*str_iter) - '0';
                str_iter++;
                current = current * 10 + uint32_t(*str_iter) - '0';
                str_iter++;
                current = current * 10 + uint32_t(*str_iter) - '0';
                str_iter++;
                *--str_conv_digits_iter = current;
            } while (str_iter != str_end);
        }

        size_t m = 2 * aligned_str_conv_digits_size;
        if (m > kFFTPrecisionBorder) {
            m *= 2;
        }
        // Allocate m complex numbers for p1 and m complex numbers for p2
        size_t max_fft_poly_length = 2 * m;
        uint32_t* mult_add_buffer =
            static_cast<uint32_t*>(operator new(aligned_str_conv_digits_size * sizeof(uint32_t) +
                                                max_fft_poly_length * sizeof(fft::complex)));
        fft::complex* fft_poly_buffer =
            reinterpret_cast<fft::complex*>(mult_add_buffer + aligned_str_conv_digits_size);
        EnsureDecBasePowsCapacity(
            math_functions::log2_floor(uint64_t(aligned_str_conv_digits_size)));
        const LongInt* conv_dec_base_pows_iter = conv_dec_base_pows.data();
        for (size_t half_len = 1; half_len != aligned_str_conv_digits_size;
             half_len *= 2, ++conv_dec_base_pows_iter) {
            for (size_t pos = 0; pos != aligned_str_conv_digits_size; pos += 2 * half_len) {
                ConvertDecBaseMultAdd(str_conv_digits + pos, half_len, conv_dec_base_pows_iter,
                                      mult_add_buffer, fft_poly_buffer);
            }
        }
        operator delete(mult_add_buffer);

        size_t usize = aligned_str_conv_digits_size;
        while (usize != 0 && nums_[usize - 1] == 0) {
            usize--;
        }
        this->size_ = sgn * int32_t(usize);
    }

    ATTRIBUTE_ALWAYS_INLINE std::string to_string() const {
        std::string s;
        to_string(s);
        return s;
    }

    void to_string(std::string& ans) const {
        ans.clear();
        if (size_ < 0) {
            ans.push_back('-');
        }

        size_t usize = USize();
        switch (usize) {
            case 0:
                ans = "0";
                return;
            case 1:
                ans += std::to_string(nums_[0]);
                return;
            case 2:
                ans += std::to_string((uint64_t(nums_[1]) << 32) | nums_[0]);
                return;
        }

        size_t n = math_functions::nearest_pow2_ge(usize);
        EnsureBinBasePowsCapacity(math_functions::log2_floor(n));
        uint32_t* knums = static_cast<uint32_t*>(longint_allocator::Allocate(n * sizeof(uint32_t)));
        std::memcpy(knums, nums_, usize * sizeof(uint32_t));
        std::memset(knums + usize, 0, (n - usize) * sizeof(uint32_t));
        Decimal result = ConvertBinBase(knums, n);
        longint_allocator::Deallocate(knums);
        assert(result.size_ >= 3);
        size_t full_blocks = result.size_ - 1;
        uint32_t last_a_i  = result.digits_[full_blocks];
        size_t string_size =
            full_blocks * kStrConvBaseDigits + math_functions::base_10_len(last_a_i);
        ans.resize((size_ < 0) + string_size);
        uint8_t* ptr = reinterpret_cast<uint8_t*>(&ans[ans.size() - 1]);
        for (size_t i = 0; i < full_blocks; i++) {
            uint32_t a_i = result.digits_[i];
            for (size_t j = kStrConvBaseDigits; j != 0; j--) {
                *ptr = uint8_t('0' + a_i % 10);
                a_i /= 10;
                ptr--;
            }
        }

        do {
            *ptr = uint8_t('0' + last_a_i % 10);
            last_a_i /= 10;
            ptr--;
        } while (last_a_i);
    }

    friend std::ostream& operator<<(std::ostream& out, const LongInt& n) {
        std::string buffer;
        n.to_string(buffer);
        return out << buffer;
    }

    friend std::istream& operator>>(std::istream& in, LongInt& n) {
        std::string s;
        in >> s;
        n.set_string(s);
        return in;
    }

    void Reserve(uint32_t capacity) {
        if (capacity > capacity_) {
            uint32_t* new_nums =
                static_cast<uint32_t*>(longint_allocator::Allocate(capacity * sizeof(uint32_t)));
            if (nums_) {
                std::memcpy(new_nums, nums_, USize() * sizeof(uint32_t));
                longint_allocator::Deallocate(nums_);
            }
            nums_     = new_nums;
            capacity_ = capacity;
        }
    }

    void ReserveWithoutCopy(uint32_t capacity) {
        if (capacity > capacity_) {
            longint_allocator::Deallocate(nums_);
            nums_ =
                static_cast<uint32_t*>(longint_allocator::Allocate(capacity * sizeof(uint32_t)));
            capacity_ = capacity;
        }
        size_ = 0;
    }

    ~LongInt() {
        longint_allocator::Deallocate(nums_);
    }

    struct Decimal {
        static constexpr uint32_t kDecimalBase    = kStrConvBase;
        static constexpr uint32_t kFftDecimalBase = 1'000;

        uint32_t* digits_ = nullptr;
        size_t size_      = 0;

        constexpr Decimal() noexcept = default;

        explicit Decimal(uint32_t n) {
            digits_ = static_cast<uint32_t*>(longint_allocator::Allocate(2 * sizeof(uint32_t)));

            uint32_t low = n % kDecimalBase;
            digits_[0]   = low;
            uint32_t hi  = n / kDecimalBase;
            digits_[1]   = hi;
            size_        = hi != 0 ? 2 : low != 0;
        }

        explicit Decimal(uint64_t n) {
            this->digits_ =
                static_cast<uint32_t*>(longint_allocator::Allocate(3 * sizeof(uint32_t)));

            uint32_t low     = uint32_t(n % kDecimalBase);
            uint64_t t       = n / kDecimalBase;
            uint32_t mid     = uint32_t(t % kDecimalBase);
            uint32_t hi      = uint32_t(t / kDecimalBase);
            this->digits_[0] = low;
            this->digits_[1] = mid;
            this->digits_[2] = hi;
            this->size_      = hi != 0 ? 3 : (mid != 0 ? 2 : low != 0);
        }

        Decimal(const Decimal& other) : digits_(nullptr), size_(other.size_) {
            if (size_) {
                digits_ =
                    static_cast<uint32_t*>(longint_allocator::Allocate(size_ * sizeof(uint32_t)));
                std::copy_n(other.digits_, other.size_, digits_);
            }
        }

        Decimal& operator=(const Decimal& other) {
            return *this = Decimal(other);
        }

        constexpr Decimal(Decimal&& other) noexcept : digits_(other.digits_), size_(other.size_) {
            other.digits_ = nullptr;
            other.size_   = 0;
        }

        Decimal& operator=(Decimal&& other) noexcept {
            const auto tmp_other_digits = other.digits_;
            const auto tmp_other_size   = other.size_;
            other.digits_               = nullptr;
            other.size_                 = 0;
            longint_allocator::Deallocate(digits_);
            digits_ = tmp_other_digits;
            size_   = tmp_other_size;
            return *this;
        }

        Decimal& operator=(uint32_t n) {
            if (this->digits_ == nullptr) {
                this->digits_ =
                    static_cast<uint32_t*>(longint_allocator::Allocate(2 * sizeof(uint32_t)));
            }

            uint32_t low     = n % kDecimalBase;
            this->digits_[0] = low;
            uint32_t hi      = n / kDecimalBase;
            this->digits_[1] = hi;
            this->size_      = hi != 0 ? 2 : low != 0;
            return *this;
        }

        Decimal& operator=(uint64_t n) {
            if (this->digits_ == nullptr) {
                this->digits_ =
                    static_cast<uint32_t*>(longint_allocator::Allocate(3 * sizeof(uint32_t)));
            }

            uint32_t low     = uint32_t(n % kDecimalBase);
            this->digits_[0] = low;
            uint32_t t       = uint32_t(n / kDecimalBase);
            uint32_t mid     = t % kDecimalBase;
            uint32_t hi      = t / kDecimalBase;
            this->digits_[1] = mid;
            this->digits_[2] = hi;
            this->size_      = hi != 0 ? 3 : (mid != 0 ? 2 : low != 0);
            return *this;
        }

        Decimal& operator*=(const Decimal& other) {
            size_t k = size_;
            size_t m = other.size_;
            const uint32_t* k_ptr;
            const uint32_t* m_ptr;

            if (m <= k) {
                k_ptr = this->digits_;
                m_ptr = other.digits_;
            } else {
                k_ptr = other.digits_;
                m_ptr = this->digits_;
                std::swap(m, k);
            }

            if (unlikely(m == 0)) {
                size_ = 0;
                return *this;
            }

            size_t new_size = m + k;
            if (m <= 16 || m * k <= 1024) {
                uint32_t* ans = static_cast<uint32_t*>(
                    longint_allocator::Allocate(new_size * sizeof(uint32_t)));
                std::memset(ans, 0, new_size * sizeof(uint32_t));
                for (size_t j = 0; j < m; j++) {
                    uint64_t b_j   = m_ptr[j];
                    uint64_t carry = 0;
                    for (size_t i = 0; i < k; i++) {
                        uint64_t a_i = k_ptr[i];
                        uint64_t res = a_i * b_j + uint64_t(ans[j + i]) + carry;
                        ans[j + i]   = uint32_t(res % kDecimalBase);
                        carry        = res / kDecimalBase;
                    }
                    ans[j + k] = uint32_t(carry % kDecimalBase);
                }

                longint_allocator::Deallocate(this->digits_);
                this->digits_ = ans;
            } else {
                size_t n = math_functions::nearest_pow2_ge(3 * new_size);
                static_assert(kFftDecimalBase * kFftDecimalBase * kFftDecimalBase == kDecimalBase,
                              "");
                // Allocate n for the first polynomial
                //  and n for the second one
                auto p1         = std::make_unique<fft::complex[]>(n + n);
                fft::complex* p = p1.get();
                for (size_t i = 0; i < m; i++) {
                    uint32_t m_value = m_ptr[i];
                    uint32_t k_value = k_ptr[i];
                    uint32_t r1      = m_value % kFftDecimalBase;
                    m_value /= kFftDecimalBase;
                    uint32_t r2 = k_value % kFftDecimalBase;
                    k_value /= kFftDecimalBase;
                    *p = fft::complex(r1, r2);
                    p++;
                    r1 = m_value % kFftDecimalBase;
                    m_value /= kFftDecimalBase;
                    r2 = k_value % kFftDecimalBase;
                    k_value /= kFftDecimalBase;
                    *p = fft::complex(r1, r2);
                    p++;
                    *p = fft::complex(m_value, k_value);
                    p++;
                }
                for (size_t i = m; i < k; i++) {
                    uint32_t k_value = k_ptr[i];
                    uint32_t r2      = k_value % kFftDecimalBase;
                    k_value /= kFftDecimalBase;
                    *p = fft::complex(0, r2);
                    p++;
                    r2 = k_value % kFftDecimalBase;
                    k_value /= kFftDecimalBase;
                    *p = fft::complex(0, r2);
                    p++;
                    *p = fft::complex(0, k_value);
                    p++;
                }

                std::memset(static_cast<void*>(p), 0, (n - 3 * k) * sizeof(fft::complex));

                if (new_size > size_) {
                    longint_allocator::Deallocate(this->digits_);
                    this->digits_ = static_cast<uint32_t*>(
                        longint_allocator::Allocate(new_size * sizeof(uint32_t)));
                }

                fft::complex* p2 = p1.get() + n;
                fft::forward_backward_fft(p1.get(), p2, n);

                uint64_t carry            = 0;
                uint32_t* ans_p           = this->digits_;
                const uint32_t* ans_p_end = ans_p + new_size;
                do {
                    uint64_t res = carry;
                    res += uint64_t(p2->real() + 0.5);
                    p2++;
                    res += uint64_t(p2->real() + 0.5) * kFftDecimalBase;
                    p2++;
                    res += uint64_t(p2->real() + 0.5) * (kFftDecimalBase * kFftDecimalBase);
                    p2++;
                    *ans_p = uint32_t(res % kDecimalBase);
                    carry  = res / kDecimalBase;
                    ans_p++;
                } while (ans_p != ans_p_end);
            }

            size_ = new_size;
            this->PopLeadingZeros();
            return *this;
        }

        Decimal& operator+=(const Decimal& other) {
            uint64_t carry = 0;
            const size_t m = std::min(this->size_, other.size_);
            uint32_t* p    = this->digits_;
            for (size_t i = 0; i < m; i++) {
                uint64_t res = uint64_t(p[i]) + uint64_t(other.digits_[i]) + carry;
                p[i]         = uint32_t(res % kDecimalBase);
                carry        = res / kDecimalBase;
            }

            if (size_ < other.size_) {
                uint32_t* new_digits = static_cast<uint32_t*>(
                    longint_allocator::Allocate(other.size_ * sizeof(uint32_t)));
                std::memcpy(new_digits, this->digits_, size_ * sizeof(uint32_t));
                std::memcpy(new_digits + size_, other.digits_ + size_,
                            (other.size_ - size_) * sizeof(uint32_t));
                longint_allocator::Deallocate(this->digits_);
                this->digits_ = new_digits;
                size_         = other.size_;
            }

            p                      = this->digits_;
            const size_t this_size = size_;
            for (size_t i = m; carry != 0 && i < this_size; i++) {
                uint64_t res = uint64_t(p[i]) + carry;
                p[i]         = uint32_t(res % kDecimalBase);
                carry        = res / kDecimalBase;
            }

            if (carry == 0) {
                PopLeadingZeros();
            } else {
                uint32_t* new_digits = static_cast<uint32_t*>(longint_allocator::Allocate(
                    (this_size + 1 + (this_size == 0)) * sizeof(uint32_t)));
                std::memcpy(new_digits, this->digits_, this_size * sizeof(uint32_t));
                new_digits[this_size] = uint32_t(carry);
                longint_allocator::Deallocate(this->digits_);
                this->digits_ = new_digits;
                size_         = this_size + 1;
            }

            return *this;
        }

        void SquareThisTo(Decimal& other) const {
            size_t digits_size = size_;
            if (unlikely(digits_size == 0)) {
                other.size_ = 0;
                return;
            }

            const uint32_t* digits_ptr = this->digits_;
            size_t prod_size           = digits_size + digits_size;
            if (prod_size <= 16) {
                uint32_t* ans = static_cast<uint32_t*>(
                    longint_allocator::Allocate(prod_size * sizeof(uint32_t)));
                std::memset(ans, 0, prod_size * sizeof(uint32_t));
                for (size_t j = 0; j < digits_size; j++) {
                    uint64_t b_j   = digits_ptr[j];
                    uint64_t carry = 0;
                    for (size_t i = 0; i < digits_size; i++) {
                        uint64_t a_i = digits_ptr[i];
                        uint64_t res = a_i * b_j + uint64_t(ans[j + i]) + carry;
                        ans[j + i]   = uint32_t(res % kDecimalBase);
                        carry        = res / kDecimalBase;
                    }
                    ans[j + digits_size] = uint32_t(carry % kDecimalBase);
                }

                longint_allocator::Deallocate(other.digits_);
                other.digits_ = ans;
            } else {
                size_t n = math_functions::nearest_pow2_ge(3 * prod_size);
                static_assert(kFftDecimalBase * kFftDecimalBase * kFftDecimalBase == kDecimalBase,
                              "");
                // Allocate n for the first polynomial
                //  and n for the second one
                auto p1         = std::make_unique<fft::complex[]>(n + n);
                fft::complex* p = p1.get();
                for (size_t i = 0; i < digits_size; i++) {
                    uint32_t value = digits_ptr[i];
                    uint32_t r1    = value % kFftDecimalBase;
                    value /= kFftDecimalBase;
                    *p = fft::complex(r1, r1);
                    p++;
                    r1 = value % kFftDecimalBase;
                    value /= kFftDecimalBase;
                    *p = fft::complex(r1, r1);
                    p++;
                    *p = fft::complex(value, value);
                    p++;
                }

                std::memset(static_cast<void*>(p), 0, (n - 3 * digits_size) * sizeof(fft::complex));

                if (prod_size > other.size_) {
                    longint_allocator::Deallocate(other.digits_);
                    other.digits_ = static_cast<uint32_t*>(
                        longint_allocator::Allocate(prod_size * sizeof(uint32_t)));
                }

                fft::complex* p2 = p1.get() + n;
                fft::forward_backward_fft(p1.get(), p2, n);

                uint64_t carry            = 0;
                uint32_t* ans_p           = other.digits_;
                const uint32_t* ans_p_end = ans_p + prod_size;
                do {
                    uint64_t res = carry;
                    res += uint64_t(p2->real() + 0.5);
                    p2++;
                    res += uint64_t(p2->real() + 0.5) * kFftDecimalBase;
                    p2++;
                    res += uint64_t(p2->real() + 0.5) * (kFftDecimalBase * kFftDecimalBase);
                    p2++;
                    *ans_p = uint32_t(res % kDecimalBase);
                    carry  = res / kDecimalBase;
                    ans_p++;
                } while (ans_p != ans_p_end);
            }

            other.size_ = prod_size;
            other.PopLeadingZeros();
        }

        constexpr bool operator==(uint32_t n) const noexcept {
            switch (size_) {
                case 0:
                    return n == 0;
                case 1:
                    return digits_[0] == n;
                case 2:
                    return uint64_t(digits_[1]) * kDecimalBase + digits_[0] == n;
                default:
                    return false;
            }
        }

        constexpr bool operator!=(uint32_t n) const noexcept {
            return !(*this == n);
        }

        constexpr bool operator==(uint64_t n) const noexcept {
            switch (size_) {
                case 0:
                    return n == 0;
                case 1:
                    return digits_[0] == n;
                case 2:
                    return uint64_t(digits_[1]) * kDecimalBase + digits_[0] == n;
                case 3: {
                    constexpr uint64_t kDecimalBase2 = uint64_t(kDecimalBase) * kDecimalBase;
                    uint64_t hi                      = digits_[2];
                    if (hi > 18) {
                        return false;
                    }

                    uint64_t low_mid_m = uint64_t(digits_[1]) * kDecimalBase + digits_[0];
                    if (hi == 18) {
                        return n >= 18 * kDecimalBase2 && n - 18 * kDecimalBase2 == low_mid_m;
                    }
                    uint64_t m = uint64_t(hi) * kDecimalBase2 + low_mid_m;
                    return m == n;
                }
                default:
                    return false;
            }
        }

        constexpr bool operator!=(uint64_t n) const noexcept {
            return !(*this == n);
        }

        constexpr bool operator==(const Decimal& other) const noexcept {
            return size_ == other.size_ && std::equal(digits_, digits_ + size_, other.digits_);
        }

        constexpr void PopLeadingZeros() noexcept {
            size_t usize = size_;
            // std::find_first_of(std::make_reverse_iterator(begin))
            while (usize != 0 && digits_[usize - 1] == 0) {
                usize--;
            }

            size_ = usize;
        }

        ~Decimal() {
            longint_allocator::Deallocate(digits_);
        }
    };

private:
    static std::vector<Decimal> conv_bin_base_pows;

    static std::vector<LongInt> conv_dec_base_pows;

    static void EnsureDecBasePowsCapacity(size_t pows_size) {
        size_t i = conv_dec_base_pows.size();
        if (i >= pows_size) {
            return;
        }
        conv_dec_base_pows.reserve(pows_size);
        do {
            conv_dec_base_pows.emplace_back();
            conv_dec_base_pows[i - 1].SquareThisTo(conv_dec_base_pows.back());
        } while (++i != pows_size);
    }

    static void ConvertDecBaseMultAdd(uint32_t conv_digits[], size_t half_len,
                                      const LongInt* conv_base_pow, uint32_t mult_add_buffer[],
                                      fft::complex fft_poly_buffer[]) {
        const uint32_t* num_hi = conv_digits + half_len;
        size_t m               = uint32_t(conv_base_pow->size_);
        assert(m != 0 && m <= half_len);
        const uint32_t* m_ptr = conv_base_pow->nums_;
        size_t prod_size      = m + half_len;
        std::memset(mult_add_buffer, 0, (half_len + half_len) * sizeof(uint32_t));
        if (half_len <= 32) {
            for (size_t j = 0; j < m; j++) {
                uint64_t b_j   = m_ptr[j];
                uint64_t carry = 0;
                for (size_t i = 0; i < half_len; i++) {
                    uint64_t a_i           = num_hi[i];
                    uint64_t res           = a_i * b_j + uint64_t(mult_add_buffer[j + i]) + carry;
                    mult_add_buffer[j + i] = uint32_t(res);
                    carry                  = res >> kNumsBits;
                }

                mult_add_buffer[j + half_len] = uint32_t(carry);
            }
        } else {
            size_t n                  = 2 * math_functions::nearest_pow2_ge(prod_size);
            const bool high_precision = n > kFFTPrecisionBorder;
            n <<= high_precision;
            fft::complex* p1 = fft_poly_buffer;
            fft::complex* p  = p1;
            if (likely(!high_precision)) {
                for (size_t i = 0; i < m; i++) {
                    uint32_t m_value = m_ptr[i];
                    uint32_t k_value = num_hi[i];
                    *p               = fft::complex(m_value & 0xFFFF, k_value & 0xFFFF);
                    p++;
                    *p = fft::complex(m_value >> 16, k_value >> 16);
                    p++;
                }
                for (size_t i = m; i < half_len; i++) {
                    uint32_t k_value = num_hi[i];
                    *p               = fft::complex(0, k_value & 0xFFFF);
                    p++;
                    *p = fft::complex(0, k_value >> 16);
                    p++;
                }
            } else {
                for (size_t i = 0; i < m; i++) {
                    uint32_t m_value = m_ptr[i];
                    uint32_t k_value = num_hi[i];
                    *p               = fft::complex(uint8_t(m_value), uint8_t(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = fft::complex(uint8_t(m_value), uint8_t(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = fft::complex(uint8_t(m_value), uint8_t(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = fft::complex(m_value, k_value);
                    p++;
                }
                for (size_t i = m; i < half_len; i++) {
                    uint32_t k_value = num_hi[i];
                    *p               = fft::complex(0, uint8_t(k_value));
                    p++;
                    k_value >>= 8;
                    *p = fft::complex(0, uint8_t(k_value));
                    p++;
                    k_value >>= 8;
                    *p = fft::complex(0, uint8_t(k_value));
                    p++;
                    k_value >>= 8;
                    *p = fft::complex(0, k_value);
                    p++;
                }
            }
            std::memset(static_cast<void*>(p), 0,
                        (n - ((2 * half_len) << high_precision)) * sizeof(fft::complex));

            fft::complex* p2 = p1 + n;
            fft::forward_backward_fft(p1, p2, n);
            uint64_t carry            = 0;
            uint32_t* ans_p           = mult_add_buffer;
            const uint32_t* ans_p_end = ans_p + prod_size;
            if (likely(!high_precision)) {
                do {
                    uint64_t res = carry;
                    res += uint64_t(p2->real() + 0.5);
                    p2++;
                    res += uint64_t(p2->real() + 0.5) << 16;
                    p2++;
                    *ans_p = uint32_t(res);
                    carry  = res >> kNumsBits;
                    ans_p++;
                } while (ans_p != ans_p_end);
            } else {
                do {
                    uint64_t res = carry;
                    res += uint64_t(p2->real() + 0.5);
                    p2++;
                    res += uint64_t(p2->real() + 0.5) << 8;
                    p2++;
                    res += uint64_t(p2->real() + 0.5) << 16;
                    p2++;
                    res += uint64_t(p2->real() + 0.5) << 24;
                    p2++;
                    *ans_p = uint32_t(res);
                    carry  = res >> kNumsBits;
                    ans_p++;
                } while (ans_p != ans_p_end);
            }
            assert(carry == 0);
        }

        // Now mult_add_buffer == num_hi * CONV_BASE^half_len
        uint64_t carry = 0;
        for (size_t i = half_len; i != 0; i--, conv_digits++, mult_add_buffer++) {
            uint64_t res = uint64_t(*conv_digits) + uint64_t(*mult_add_buffer) + carry;
            *conv_digits = uint32_t(res);
            carry        = res >> kNumsBits;
        }
        for (size_t i = half_len; i != 0; i--, conv_digits++, mult_add_buffer++) {
            uint64_t res = uint64_t(*mult_add_buffer) + carry;
            *conv_digits = uint32_t(res);
            carry        = res >> kNumsBits;
        }
        assert(carry == 0);
    }

    static Decimal ConvertBinBase(const uint32_t* nums, size_t size) {
        assert(math_functions::is_pow2(size));
        ATTRIBUTE_ASSUME(math_functions::is_pow2(size));
        switch (size) {
            case 0:
            case 1:
                return Decimal(nums[0]);
            case 2:
                return Decimal(uint64_t(nums[1]) * kNumsBase | nums[0]);
        }

        Decimal low_dec  = ConvertBinBase(nums, size / 2);
        Decimal high_dec = ConvertBinBase(nums + size / 2, size / 2);

        const std::size_t idx = math_functions::log2_floor(size) - 1;
        assert(idx < conv_bin_base_pows.size());
        high_dec *= conv_bin_base_pows[idx];
        high_dec += low_dec;
        return high_dec;
    }

    static void EnsureBinBasePowsCapacity(size_t pows_size) {
        size_t i = conv_bin_base_pows.size();
        if (i >= pows_size) {
            return;
        }
        conv_bin_base_pows.reserve(pows_size);
        do {
            conv_bin_base_pows.emplace_back();
            conv_bin_base_pows[i - 1].SquareThisTo(conv_bin_base_pows.back());
        } while (++i != pows_size);
    }

    ATTRIBUTE_NOINLINE void GrowCapacity() {
        uint32_t new_capacity = (capacity_ * 2) | (capacity_ == 0);
        uint32_t* new_nums =
            static_cast<uint32_t*>(longint_allocator::Allocate(new_capacity * sizeof(uint32_t)));
        if (nums_) {
            std::memcpy(new_nums, nums_, USize() * sizeof(uint32_t));
            longint_allocator::Deallocate(nums_);
        }
        nums_     = new_nums;
        capacity_ = new_capacity;
    }

    ATTRIBUTE_NOINLINE size_t GrowSizeByOne() {
        size_t usize = USize();
        if (unlikely(usize == capacity_)) {
            GrowCapacity();
        }

        size_ += sign();
        return usize + 1;
    }

    size_t SetSizeAtLeast(size_t new_size) {
        size_t cur_size = USize();
        if (new_size <= cur_size) {
            return cur_size;
        }

        if (new_size > capacity_) {
            uint32_t* new_nums =
                static_cast<uint32_t*>(longint_allocator::Allocate(new_size * sizeof(uint32_t)));
            if (nums_) {
                std::memcpy(new_nums, nums_, cur_size * sizeof(uint32_t));
                longint_allocator::Deallocate(nums_);
            }
            nums_     = new_nums;
            capacity_ = uint32_t(new_size);
        }

        std::memset(&nums_[cur_size], 0, (new_size - cur_size) * sizeof(uint32_t));
        size_ = size_ >= 0 ? int32_t(new_size) : -int32_t(new_size);
        return new_size;
    }

    void EnsureDefaultCapacityOpEqCall() {
        if (capacity_ < kDefaultLINumsCapacity) {
            longint_allocator::Deallocate(nums_);
            AllocateDefaultCapacity();
        }
    }

    void AllocateDefaultCapacity() {
        nums_ = static_cast<uint32_t*>(
            longint_allocator::Allocate(kDefaultLINumsCapacity * sizeof(uint32_t)));
        capacity_ = kDefaultLINumsCapacity;
    }

    static constexpr uint64_t LongIntAdd(uint32_t nums1[], const uint32_t nums2[], size_t usize1,
                                         size_t usize2) noexcept {
        uint64_t carry            = 0;
        const uint32_t* nums1_end = nums1 + usize1;
        for (const uint32_t* nums2_end = nums2 + usize2; nums2 != nums2_end; ++nums2, ++nums1) {
            uint64_t res = uint64_t(*nums1) + uint64_t(*nums2) + carry;
            *nums1       = uint32_t(res);
            carry        = res >> kNumsBits;
        }

        for (; carry != 0; ++nums1) {
            if (nums1 == nums1_end) {
                return carry;
            }

            uint64_t res = uint64_t(*nums1) + carry;
            *nums1       = uint32_t(res);
            carry        = res >> kNumsBits;
        }

        return 0;
    }

    void NonZeroSizeAddUInt(uint32_t n) {
        uint32_t* it             = begin();
        const uint32_t* end_iter = end();
        uint64_t carry           = n;
        do {
            uint64_t res = uint64_t(*it) + carry;
            carry        = res >> kNumsBits;
            *it          = uint32_t(res);
            if (carry == 0) {
                return;
            }
            ++it;
        } while (it != end_iter);

        if (carry != 0) {
            size_t usize = USize();
            if (unlikely(usize == capacity_)) {
                GrowCapacity();
            }

            assert(usize < capacity_);
            nums_[usize] = uint32_t(carry);
            size_ += sign();
        }
    }

    constexpr void NonZeroSizeSubUInt(uint32_t n) {
        size_t usize        = USize();
        uint32_t* nums_iter = nums_;
        uint32_t low_num    = nums_iter[0];
        if (usize != 1) {
            uint32_t res = low_num - n;
            bool carry   = res > low_num;
            nums_iter[0] = res;
            if (carry) {
                do {
                    ++nums_iter;
                } while (nums_iter[0] == 0);
                if (--nums_iter[0] == 0) {
                    size_ = sign();
                }
            }
        } else if (n <= low_num) {
            nums_iter[0] = low_num - n;
        } else {
            nums_iter[0] = n - low_num;
            change_sign();
        }
    }
};

std::vector<LongInt> LongInt::conv_dec_base_pows = {LongInt(LongInt::kStrConvBase)};

std::vector<LongInt::Decimal> LongInt::conv_bin_base_pows = {LongInt::Decimal(LongInt::kNumsBase)};

#if __cplusplus >= 202002L
constexpr
#endif
    void
    swap(LongInt& n, LongInt& m) noexcept {
    n.Swap(m);
}

std::string to_string(const LongInt& n) {
    return n.to_string();
}
