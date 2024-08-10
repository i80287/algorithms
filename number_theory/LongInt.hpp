// #define NDEBUG 1

#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iterator>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "config_macros.hpp"
#include "fft.hpp"
#if CONFIG_HAS_INCLUDE("integers_128_bit.hpp")
#include "integers_128_bit.hpp"
#endif
#include "math_functions.hpp"

#if !defined(__GNUG__)
#error "Current implementation works only with GCC"
#endif

namespace longint_allocator {

// #define DEBUG_LI_ALLOC_PRINTING

class inner_impl {
private:
    static constexpr std::size_t kPageMemoryOffsetInBytes = sizeof(void*);

    struct alignas(64) SmallPage {
        static constexpr std::size_t kCapacity =
            32 * sizeof(std::uint32_t) - kPageMemoryOffsetInBytes;

        std::byte memory[kCapacity];
        SmallPage* next;
    };
    static_assert((sizeof(SmallPage) & (sizeof(SmallPage) - 1)) == 0, "");
    static_assert(sizeof(SmallPage) == sizeof(SmallPage::memory) + kPageMemoryOffsetInBytes, "");

    struct alignas(64) MiddlePage {
        static constexpr std::size_t kCapacity =
            256 * sizeof(std::uint32_t) - kPageMemoryOffsetInBytes;

        std::byte memory[kCapacity];
        MiddlePage* next;
    };

    static_assert((sizeof(MiddlePage) & (sizeof(MiddlePage) - 1)) == 0, "");
    static_assert(sizeof(MiddlePage) == sizeof(MiddlePage::memory) + kPageMemoryOffsetInBytes, "");

    static constexpr std::size_t kTotalSmallPages  = 32;
    static constexpr std::size_t kTotalMiddlePages = 8;

#ifdef __cpp_constinit
#define LI_ALLOC_CONSTINIT constinit
#else
#define LI_ALLOC_CONSTINIT
#endif

    LI_ALLOC_CONSTINIT static inline SmallPage first_small_page[kTotalSmallPages]    = {};
    LI_ALLOC_CONSTINIT static inline MiddlePage first_middle_page[kTotalMiddlePages] = {};

    LI_ALLOC_CONSTINIT
    static inline SmallPage* free_small_pages_head = std::addressof(first_small_page[0]);
    LI_ALLOC_CONSTINIT
    static inline MiddlePage* free_middle_pages_head = std::addressof(first_middle_page[0]);

#ifdef DEBUG_LI_ALLOC_PRINTING
    LI_ALLOC_CONSTINIT static inline std::size_t total_small_pages_used     = 0;
    LI_ALLOC_CONSTINIT static inline std::int32_t current_small_pages_used  = 0;
    LI_ALLOC_CONSTINIT static inline std::int32_t max_small_pages_used      = -(1 << 30);
    LI_ALLOC_CONSTINIT static inline std::size_t total_middle_pages_used    = 0;
    LI_ALLOC_CONSTINIT static inline std::int32_t current_middle_pages_used = 0;
    LI_ALLOC_CONSTINIT static inline std::int32_t max_middle_pages_used     = -(1 << 30);
    LI_ALLOC_CONSTINIT static inline std::size_t bytes_allocated            = 0;
    LI_ALLOC_CONSTINIT static inline std::int32_t malloc_free_count         = 0;
#endif

#undef LI_ALLOC_CONSTINIT

/**
 * VS Code intellisense: "attribute "constructor" does not take arguments
 * C/C++(1094)"
 */
#if defined(__INTELLISENSE__) && __INTELLISENSE__
#pragma diag_suppress 1094
#endif

    __attribute__((constructor(101))) static inline void InitPages() noexcept {
        auto init_small_pages = []() noexcept {
            SmallPage* p = first_small_page;
            for (const SmallPage* last_page = p + kTotalSmallPages - 1; p != last_page;) {
                SmallPage* p_next = p + 1;
                p->next           = p_next;
                p                 = p_next;
            }
            p->next = nullptr;
        };
        auto init_middle_pages = []() noexcept {
            MiddlePage* p = first_middle_page;
            for (const MiddlePage* p_iter_end = p + kTotalMiddlePages - 1; p != p_iter_end;) {
                MiddlePage* p_next = p + 1;
                p->next            = p_next;
                p                  = p_next;
            }
            p->next = nullptr;
        };
        init_small_pages();
        init_middle_pages();
#ifdef DEBUG_LI_ALLOC_PRINTING
        printf("[INIT] Inited pages in %s\n", FUNCTION_MACRO);
#endif
    }

#ifdef DEBUG_LI_ALLOC_PRINTING
    __attribute__((destructor(101))) static inline void DeinitPages() noexcept {
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
            total_middle_pages_used, max_middle_pages_used, current_middle_pages_used,
            bytes_allocated, malloc_free_count);
    }
#endif

    static inline bool IsSmallPage(const std::byte* offset_memory) noexcept {
        const std::byte* fsp =
            reinterpret_cast<const std::byte*>(std::addressof(first_small_page[0]));
        return static_cast<std::size_t>(offset_memory - fsp) < kTotalSmallPages * sizeof(SmallPage);
    }

    static inline bool IsMiddlePage(const std::byte* offset_memory) noexcept {
        const std::byte* fmp =
            reinterpret_cast<const std::byte*>(std::addressof(first_middle_page[0]));
        return static_cast<std::size_t>(offset_memory - fmp) <
               kTotalMiddlePages * sizeof(MiddlePage);
    }

    friend void* Allocate(std::size_t size);
    friend void Deallocate(void* memory) noexcept;
};

void Deallocate(void* memory) noexcept {
    if (unlikely(memory == nullptr)) {
        return;
    }

    std::byte* p = static_cast<std::byte*>(memory);
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
    ::operator delete(memory);
}

#ifdef __clang__
__attribute__((malloc))
#else
__attribute__((malloc, malloc(::longint_allocator::Deallocate, 1)))
#endif
ATTRIBUTE_RETURNS_NONNULL void*
Allocate(std::size_t size) {
    if (size <= inner_impl::SmallPage::kCapacity && inner_impl::free_small_pages_head != nullptr) {
        inner_impl::SmallPage* p = inner_impl::free_small_pages_head;
#ifdef DEBUG_LI_ALLOC_PRINTING
        inner_impl::current_small_pages_used++;
        if (inner_impl::current_small_pages_used > inner_impl::max_small_pages_used) {
            inner_impl::max_small_pages_used = inner_impl::current_small_pages_used;
        }
        inner_impl::total_small_pages_used++;
#endif
        inner_impl::free_small_pages_head = p->next;

        return static_cast<void*>(std::addressof(p->memory[0]));
    }

    if (size <= inner_impl::MiddlePage::kCapacity &&
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
        return static_cast<void*>(std::addressof(p->memory[0]));
    }

    void* p = ::operator new(size);
#ifdef DEBUG_LI_ALLOC_PRINTING
    inner_impl::bytes_allocated += static_cast<std::size_t>(size);
    inner_impl::malloc_free_count++;
#endif
    return p;
}

#ifdef DEBUG_LI_ALLOC_PRINTING
#undef DEBUG_LI_ALLOC_PRINTING
#endif

}  // namespace longint_allocator

struct longint {
    using digit_t        = std::uint32_t;
    using pointer        = digit_t*;
    using const_pointer  = const digit_t*;
    using iterator       = pointer;
    using const_iterator = const_pointer;

    static constexpr std::size_t kDefaultLINumsCapacity = 2;
    static constexpr std::uint32_t kStrConvBase         = 1'000'000'000;
    static constexpr std::uint32_t kStrConvBaseDigits =
        math_functions::base_b_len(kStrConvBase - 1);
    static constexpr std::uint32_t kNumsBits         = 32;
    static constexpr std::uint64_t kNumsBase         = std::uint64_t(1) << kNumsBits;
    static constexpr std::size_t kFFTPrecisionBorder = 1u << 18;
    static constexpr auto kFFTFloatRoundError =
        std::numeric_limits<typename fft::complex::value_type>::round_error();

    digit_t* nums_ = nullptr;
    /**
     * size_ < 0 <=> sign = -1; size_ == 0 <=> sign = 0; size > 0 <=> sign = 1
     */
    std::int32_t size_      = 0;
    std::uint32_t capacity_ = 0;

    constexpr longint() noexcept = default;
    longint(const longint& other) : nums_(nullptr), size_(other.size_), capacity_(other.capacity_) {
        if (capacity_ > 0) {
            nums_ = static_cast<uint32_t*>(allocate(capacity_));
            std::copy_n(other.nums_, usize(), nums_);
        }
    }
    longint& operator=(const longint& other) {
        return *this = longint(other);
    }
    constexpr longint(longint&& other) noexcept
        : nums_(other.nums_), size_(other.size_), capacity_(other.capacity_) {
        other.nums_     = nullptr;
        other.size_     = 0;
        other.capacity_ = 0;
    }
#if CONFIG_HAS_AT_LEAST_CXX_20
    constexpr
#endif
        longint&
        operator=(longint&& other) noexcept {
        swap(other);
        return *this;
    }
#if CONFIG_HAS_AT_LEAST_CXX_20
    constexpr
#endif
        void
        swap(longint& other) noexcept {
        std::swap(nums_, other.nums_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
#if CONFIG_HAS_AT_LEAST_CXX_20
    constexpr
#endif
        friend void
        swap(longint& lhs, longint& rhs) noexcept {
        lhs.swap(rhs);
    }
    longint(uint32_t n) : size_(n != 0) {
        allocateDefaultCapacity();
        nums_[0] = n;
    }
    longint(int32_t n) {
        allocateDefaultCapacity();
        std::int32_t sgn = std::int32_t(n > 0) - std::int32_t(n < 0);
        nums_[0]         = n > 0 ? uint32_t(n) : -uint32_t(n);
        size_            = sgn;
    }
    longint(uint64_t n) : size_(n != 0) {
        allocateDefaultCapacity();
        nums_[0] = uint32_t(n);
        n >>= 32;
        size_ += n != 0;
        nums_[1] = uint32_t(n);
    }
    longint(int64_t n) {
        allocateDefaultCapacity();
        std::int32_t sgn = std::int32_t(n > 0) - std::int32_t(n < 0);
        uint64_t m       = n > 0 ? uint64_t(n) : -uint64_t(n);
        size_            = m != 0;
        nums_[0]         = uint32_t(m);
        m >>= 32;
        size_ += m != 0;
        nums_[1] = uint32_t(m);
        size_ *= sgn;
    }
#if defined(INTEGERS_128_BIT_HPP)
    longint(uint128_t n) : size_(n != 0), capacity_(4) {
        nums_    = static_cast<uint32_t*>(allocate(4));
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
    explicit longint(std::string_view s) {
        set_string(s);
    }
    struct Reserve {
        explicit constexpr Reserve(std::uint32_t capacity) noexcept : capacity_(capacity) {}
        std::uint32_t capacity_{};
    };
    explicit longint(Reserve reserve_tag)
        : nums_(static_cast<uint32_t*>(allocate(reserve_tag.capacity_))),
          capacity_(reserve_tag.capacity_) {}
    longint& operator=(int32_t n) {
        ensureDefaultCapacityOpEqCall();
        size_    = std::int32_t(n > 0) - std::int32_t(n < 0);
        nums_[0] = n > 0 ? uint32_t(n) : -uint32_t(n);
        return *this;
    }

    longint& operator=(uint32_t n) {
        ensureDefaultCapacityOpEqCall();
        size_    = n != 0;
        nums_[0] = n;
        return *this;
    }
    longint& operator=(int64_t n) {
        ensureDefaultCapacityOpEqCall();
        std::int32_t sgn = std::int32_t(n > 0) - std::int32_t(n < 0);
        uint64_t m       = n > 0 ? uint64_t(n) : -uint64_t(n);
        size_            = m != 0;
        nums_[0]         = uint32_t(m);
        m >>= 32;
        size_ += m != 0;
        nums_[1] = uint32_t(m);
        size_ *= sgn;
        return *this;
    }
    longint& operator=(uint64_t n) {
        ensureDefaultCapacityOpEqCall();
        size_    = n != 0;
        nums_[0] = uint32_t(n);
        n >>= 32;
        size_ += n != 0;
        nums_[1] = uint32_t(n);
        return *this;
    }
#if defined(INTEGERS_128_BIT_HPP)
    longint& operator=(uint128_t n) {
        if (capacity_ < 4) {
            deallocate(nums_);
            nums_     = static_cast<uint32_t*>(allocate(4));
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

    longint& pow(std::size_t p) {
        longint res = uint32_t(1);
        reserve((std::max(usize(), std::size_t{1}) - 1) * p);
        while (true) {
            if (p & 1) {
                res *= *this;
            }
            p >>= 1;
            if (p == 0) {
                break;
            }
            SquareInplace();
        }
        return *this = std::move(res);
    }
    void SquareThisTo(longint& other) const {
        const std::size_t usize_value = usize();
        if (unlikely(usize_value == 0)) {
            other.size_ = 0;
            return;
        }
        const uint32_t* nums_ptr = nums_;
        std::size_t prod_size    = usize_value + usize_value;
        if (prod_size <= 16) {
            other.capacity_ = uint32_t(prod_size);
            uint32_t* ans   = static_cast<uint32_t*>(allocate(prod_size));
            std::fill_n(ans, prod_size, uint32_t(0));
            for (std::size_t j = 0; j < usize_value; j++) {
                uint64_t b_j   = nums_ptr[j];
                uint64_t carry = 0;
                for (std::size_t i = 0; i < usize_value; i++) {
                    uint64_t a_i = nums_ptr[i];
                    uint64_t res = a_i * b_j + uint64_t(ans[j + i]) + carry;
                    ans[j + i]   = uint32_t(res);
                    carry        = res >> 32;
                }

                ans[j + usize_value] = uint32_t(carry);
            }

            deallocate(other.nums_);
            other.nums_ = ans;
        } else {
            std::size_t n = 2 * math_functions::nearest_greater_equal_power_of_two(prod_size);
            const bool high_precision = n > kFFTPrecisionBorder;
            n <<= high_precision;
            // Allocate n complex numbers for p1 and n complex numbers for p2
            std::unique_ptr<fft::complex, struct ComplexDeleter> p1(
                allocate_complex_array_for_unique_ptr(2 * n));
            fft::complex* p = p1.get();
            if (likely(!high_precision)) {
                for (std::size_t i = 0; i < usize_value; i++) {
                    uint32_t value = nums_ptr[i];
                    *p             = fft::complex(value & 0xFFFF, value & 0xFFFF);
                    p++;
                    value >>= 16;
                    *p = fft::complex(value, value);
                    p++;
                }
            } else {
                for (std::size_t i = 0; i < usize_value; i++) {
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

            other.reserveUninitializedWithoutCopy(uint32_t(prod_size));
            fft::complex* p2 = p1.get() + n;
            fft::forward_backward_fft(p1.get(), p2, n);
            uint64_t carry  = 0;
            uint32_t* ans_p = other.nums_;

            if (const uint32_t* ans_p_end = ans_p + prod_size; likely(!high_precision)) {
                do {
                    uint64_t res = carry;
                    res += uint64_t(p2->real() + kFFTFloatRoundError);
                    p2++;
                    res += uint64_t(p2->real() + kFFTFloatRoundError) << 16;
                    p2++;
                    *ans_p = uint32_t(res);
                    carry  = res >> 32;
                    ans_p++;
                } while (ans_p != ans_p_end);
            } else {
                do {
                    uint64_t res = carry;
                    res += uint64_t(p2->real() + kFFTFloatRoundError);
                    p2++;
                    res += uint64_t(p2->real() + kFFTFloatRoundError) << 8;
                    p2++;
                    res += uint64_t(p2->real() + kFFTFloatRoundError) << 16;
                    p2++;
                    res += uint64_t(p2->real() + kFFTFloatRoundError) << 24;
                    p2++;
                    *ans_p = uint32_t(res);
                    carry  = res >> 32;
                    ans_p++;
                } while (ans_p != ans_p_end);
            }
            assert(carry == 0);
        }

        other.size_ = std::int32_t(prod_size);
        other.popLeadingZeros();
    }
    longint& SquareInplace() {
        SquareThisTo(*this);
        return *this;
    }
    [[nodiscard]] constexpr uint32_t operator[](std::size_t pos) const noexcept {
        return nums_[pos];
    }
    longint& operator*=(const longint& other) {
        std::size_t k         = usize();
        std::size_t m         = other.usize();
        const uint32_t* k_ptr = nums_;
        const uint32_t* m_ptr = other.nums_;

        if (m > k) {
            std::swap(m_ptr, k_ptr);
            std::swap(m, k);
        }

        if (unlikely(m == 0)) {
            size_ = 0;
            return *this;
        }

        std::size_t prod_size = m + k;
        if (m <= 16 || m * k <= 1024) {
            capacity_     = uint32_t(prod_size);
            uint32_t* ans = static_cast<uint32_t*>(allocate(prod_size));
            std::fill_n(ans, prod_size, uint32_t(0));
            for (std::size_t j = 0; j < m; j++) {
                uint64_t b_j   = m_ptr[j];
                uint64_t carry = 0;
                for (std::size_t i = 0; i < k; i++) {
                    uint64_t a_i = k_ptr[i];
                    uint64_t res = a_i * b_j + uint64_t(ans[j + i]) + carry;
                    ans[j + i]   = uint32_t(res);
                    carry        = res >> 32;
                }

                ans[j + k] = uint32_t(carry);
            }

            deallocate(nums_);
            nums_ = ans;
        } else {
            std::size_t n = 2 * math_functions::nearest_greater_equal_power_of_two(prod_size);
            const bool high_precision = n > kFFTPrecisionBorder;
            n <<= high_precision;
            // Allocate n complex numbers for p1 and n complex numbers for p2
            std::unique_ptr<fft::complex, struct ComplexDeleter> p1(
                allocate_complex_array_for_unique_ptr(2 * n));
            fft::complex* p = p1.get();
            if (likely(!high_precision)) {
                for (std::size_t i = 0; i < m; i++) {
                    uint32_t m_value = m_ptr[i];
                    uint32_t k_value = k_ptr[i];
                    *p               = fft::complex(m_value & 0xFFFF, k_value & 0xFFFF);
                    p++;
                    *p = fft::complex(m_value >> 16, k_value >> 16);
                    p++;
                }
                for (std::size_t i = m; i < k; i++) {
                    uint32_t k_value = k_ptr[i];
                    *p               = fft::complex(0, k_value & 0xFFFF);
                    p++;
                    *p = fft::complex(0, k_value >> 16);
                    p++;
                }
            } else {
                for (std::size_t i = 0; i < m; i++) {
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
                for (std::size_t i = m; i < k; i++) {
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

            reserveUninitializedWithoutCopy(uint32_t(prod_size));
            fft::complex* p2 = p1.get() + n;
            fft::forward_backward_fft(p1.get(), p2, n);
            uint64_t carry            = 0;
            uint32_t* ans_p           = nums_;
            const uint32_t* ans_p_end = ans_p + prod_size;
            if (likely(!high_precision)) {
                do {
                    uint64_t res = carry;
                    res += uint64_t(p2->real() + kFFTFloatRoundError);
                    p2++;
                    res += uint64_t(p2->real() + kFFTFloatRoundError) << 16;
                    p2++;
                    *ans_p = uint32_t(res);
                    carry  = res >> 32;
                    ans_p++;
                } while (ans_p != ans_p_end);
            } else {
                do {
                    uint64_t res = carry;
                    res += uint64_t(p2->real() + kFFTFloatRoundError);
                    p2++;
                    res += uint64_t(p2->real() + kFFTFloatRoundError) << 8;
                    p2++;
                    res += uint64_t(p2->real() + kFFTFloatRoundError) << 16;
                    p2++;
                    res += uint64_t(p2->real() + kFFTFloatRoundError) << 24;
                    p2++;
                    *ans_p = uint32_t(res);
                    carry  = res >> 32;
                    ans_p++;
                } while (ans_p != ans_p_end);
            }
            assert(carry == 0);
        }

        std::int32_t sign_product = size_ ^ other.size_;
        size_ = sign_product >= 0 ? std::int32_t(prod_size) : std::int32_t(-prod_size);
        popLeadingZeros();
        return *this;
    }
    [[nodiscard]] longint operator*(const longint& other) const {
        longint copy(*this);
        copy *= other;
        return copy;
    }
    void divmod(const longint& other, longint& rem) {
        /**
         * See Hackers Delight 9-2.
         */
        std::size_t m = usize();
        std::size_t n = other.usize();
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
                rem = divmod(other[0]);
                return;
        }

        const uint32_t* u = nums_;
        const uint32_t* v = other.nums_;

        rem.reserveUninitializedWithoutCopy(uint32_t(n));
        rem.size_ = std::int32_t(n);

        // Normilize by shifting v left just enough so that
        // its high-order bit i on, and shift u left the
        // same amount. We may have to append a high-order
        // digit on the dividend; we do that unconditionally.

        // 0 <= s < kNumsBits = 32
        uint32_t s   = static_cast<uint32_t>(math_functions::countl_zero(v[n - 1]));
        uint32_t* vn = static_cast<uint32_t*>(allocate(2 * n));
        for (std::size_t i = n - 1; i > 0; i--) {
            vn[i] = (v[i] << s) | (v[i - 1] >> (kNumsBase - s));
        }
        vn[0] = v[0] << s;

        uint32_t* un = static_cast<uint32_t*>(allocate(2 * (m + 1)));
        un[m]        = u[m - 1] >> (kNumsBase - s);
        for (std::size_t i = m - 1; i > 0; i--) {
            un[i] = (u[i] << s) | (u[i - 1] >> (kNumsBase - s));
        }
        un[0] = u[0] << s;

        uint32_t* quot = nums_;

        for (std::size_t j = m - n; static_cast<std::ptrdiff_t>(j) >= 0; j--) {
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
            for (std::size_t i = 0; i < n; i++) {
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
                for (std::size_t i = 0; i < n; i++) {
                    t         = uint64_t(un[i + j]) + uint64_t(vn[i]) + carry;
                    un[i + j] = uint32_t(t);
                    carry     = t >> kNumsBits;
                }
                un[j + n] += uint32_t(carry);
            }
        }

        // Unnormalize remainder
        for (std::size_t i = 0; i < n; i++) {
            rem.nums_[i] = (un[i] >> s) | (un[i + 1] << (16 - s));
        }

        deallocate(un);
        deallocate(vn);

        rem.popLeadingZeros();
        size_ = std::int32_t(m - n + 1);
        popLeadingZeros();
    }
    longint& operator+=(const longint& other) {
        std::size_t usize2 = other.usize();
        if ((size_ ^ other.size_) >= 0) {
            std::size_t usize1          = setSizeAtLeast(usize2 + 1);
            uint64_t add_overflow_carry = longIntAdd(nums_, other.nums_, usize1, usize2);
            if (likely(add_overflow_carry == 0)) {
                popLeadingZeros();
            } else {
                std::size_t new_usize1 = growSizeByOne();
                nums_[new_usize1 - 1]  = uint32_t(add_overflow_carry);
            }
        } else {
            throw std::runtime_error(
                "Summation of two longints with different signs is not implemented");
            // longintSubtract(*this, other.nums_, usize2);
        }

        return *this;
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator==(int32_t n) const noexcept {
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
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator==(int64_t n) const noexcept {
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
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator==(uint32_t n) const noexcept {
        switch (size_) {
            case 0:
                return n == 0;
            case 1:
                return nums_[0] == n;
            default:
                return false;
        }
    }
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator!=(uint32_t n) const noexcept {
        return !(*this == n);
    }
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator==(uint64_t n) const noexcept {
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
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator!=(uint64_t n) const noexcept {
        return !(*this == n);
    }
#if defined(INTEGERS_128_BIT_HPP)
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator==(uint128_t n) const noexcept {
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

    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator!=(uint128_t n) const noexcept {
        return !(*this == n);
    }
#endif

    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator==(const longint& other) const noexcept {
        return size_ == other.size_ && std::equal(nums_, nums_ + usize(), other.nums_);
    }

#if CONFIG_HAS_AT_LEAST_CXX_20
    constexpr std::strong_ordering operator<=>(const longint& other) const noexcept {
        if (size_ != other.size_) {
            return size_ <=> other.size_;
        }

        const std::size_t usize_value = usize();
        const uint32_t* const r_end   = nums_ - 1;
        const uint32_t* r_nums        = r_end + usize_value;
        const uint32_t* r_other_nums  = other.nums_ - 1 + usize_value;
        for (; r_nums != r_end; r_nums--, r_other_nums--) {
            if (*r_nums != *r_other_nums) {
                return int64_t(*r_nums) * sign() <=> int64_t(*r_other_nums) * other.sign();
            }
        }

        return std::strong_ordering::equivalent;
    }
#else
    constexpr bool operator<(const longint& other) const noexcept {
        if (size_ != other.size_) {
            return size_ < other.size_;
        }

        std::size_t usize_value      = usize();
        const uint32_t* r_end        = nums_ - 1;
        const uint32_t* r_nums       = r_end + usize_value;
        const uint32_t* r_other_nums = other.nums_ - 1 + usize_value;
        for (; r_nums != r_end; r_nums--, r_other_nums--) {
            if (*r_nums != *r_other_nums) {
                return int64_t(*r_nums) * sign() < int64_t(*r_other_nums) * other.sign();
            }
        }

        return false;
    }

    constexpr bool operator>(const longint& other) const noexcept {
        return other < *this;
    }

    constexpr bool operator<=(const longint& other) const noexcept {
        return !(*this > other);
    }

    constexpr bool operator>=(const longint& other) const noexcept {
        return !(*this < other);
    }
#endif

    longint& operator+=(uint32_t n) {
        if (unlikely(size_ == 0)) {
            if (capacity_ == 0) {
                allocateDefaultCapacity();
            }

            nums_[0] = n;
            size_    = n != 0;
            return *this;
        }

        assert(capacity_ != 0);
        if (size_ > 0) {
            nonZeroSizeAddUInt(n);
        } else {
            nonZeroSizeSubUInt(n);
        }

        return *this;
    }

    longint& operator-=(uint32_t n) {
        if (unlikely(size_ == 0)) {
            if (capacity_ == 0) {
                allocateDefaultCapacity();
            }

            nums_[0] = n;
            size_    = -int32_t(n != 0);
            return *this;
        }

        assert(capacity_ != 0);
        if (size_ > 0) {
            nonZeroSizeSubUInt(n);
        } else {
            nonZeroSizeAddUInt(n);
        }

        return *this;
    }

    longint& operator*=(uint32_t x) {
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
                growCapacity();
            }

            assert(u32size < capacity_);
            nums_[u32size] = uint32_t(carry);
            size_ += sign();
        }

        return *this;
    }

    constexpr longint& operator/=(uint32_t n) noexcept {
        if ((config_is_constant_evaluated() || config_is_gcc_constant_p(n)) && (n & (n - 1)) == 0) {
            if (n > 1) {
                operator>>=(uint32_t(math_functions::countl_zero(n)));
            }
            return *this;
        }
        static_cast<void>(divmod(n));
        return *this;
    }
    constexpr longint& operator/=(int32_t n) noexcept {
        const bool negative = n < 0;
        this->operator/=(math_functions::uabs(n));
        if (negative) {
            change_sign();
        }
        return *this;
    }

    /// @brief Set *this = *this / n and return *this % n
    /// @note  Behaviour is undefined if n equals 0
    /// @param n
    /// @return *this mod n
    [[nodiscard]] constexpr uint32_t divmod(uint32_t n) noexcept {
        uint64_t carry          = 0;
        uint32_t* nums_iter_end = nums_ - 1;
        uint32_t* nums_iter     = nums_iter_end + usize();
        for (; nums_iter != nums_iter_end; --nums_iter) {
            uint64_t cur = (carry << 32) | uint64_t(*nums_iter);
            uint64_t q   = cur / n;
            uint64_t r   = cur - q * n;
            *nums_iter   = uint32_t(q);
            carry        = r;
        }

        popLeadingZeros();
        return uint32_t(carry);
    }

    constexpr longint& operator>>=(uint32_t shift) noexcept {
        std::size_t usize_value = usize();
        uint32_t uints_move     = shift / 32;
        if (uints_move >= usize_value) {
            size_ = 0;
            return *this;
        }

        if (uints_move != 0) {
            usize_value -= uints_move;
            size_ = size_ >= 0 ? std::int32_t(usize_value) : -int32_t(usize_value);
            uint32_t* copy_dst_start       = nums_;
            const uint32_t* copy_src_start = copy_dst_start + uints_move;
            const uint32_t* copy_src_end   = copy_src_start + usize_value;
            std::copy(copy_src_start, copy_src_end, copy_dst_start);
        }

        shift %= 32;
        uint32_t* nums_iter            = nums_;
        uint32_t* const nums_iter_last = nums_iter + std::ptrdiff_t(usize_value) - 1;
        for (; nums_iter != nums_iter_last; ++nums_iter) {
#if CONFIG_BYTE_ORDER_LITTLE_ENDIAN
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

    constexpr void popLeadingZeros() noexcept {
        std::size_t usize_value = usize();
        while (usize_value != 0 && nums_[usize_value - 1] == 0) {
            usize_value--;
        }

        size_ = size_ >= 0 ? std::int32_t(usize_value) : -std::int32_t(usize_value);
    }

    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr std::int32_t size()
        const noexcept {
        const auto value = size_;
        if (value > static_cast<std::int64_t>(max_size()) ||
            value > static_cast<std::int64_t>(capacity_)) {
            CONFIG_UNREACHABLE();
        }
        return value;
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr std::size_t usize()
        const noexcept {
        // std::abs is not used in order to make method constexpr
        const auto value = std::size_t(math_functions::uabs(size_));
        if (value > max_size() || value > capacity_) {
            CONFIG_UNREACHABLE();
        }
        return value;
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr std::uint32_t capacity()
        const noexcept {
        const auto value = capacity_;
        if (value > max_size()) {
            CONFIG_UNREACHABLE();
        }
        return value;
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr std::int32_t sign()
        const noexcept {
        return math_functions::sign(size_);
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr bool iszero() const noexcept {
        return size_ == 0;
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr bool empty() const noexcept {
        return iszero();
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr operator bool() const noexcept {
        return !iszero();
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr bool operator!() const noexcept {
        return iszero();
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr iterator begin() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return nums_;
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr iterator end() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return nums_ + usize();
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr const_iterator begin()
        const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return nums_;
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr const_iterator end()
        const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return nums_ + usize();
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr const_iterator cbegin()
        const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return begin();
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr const_iterator cend()
        const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return end();
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr std::reverse_iterator<iterator>
    rbegin() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return std::make_reverse_iterator(end());
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr std::reverse_iterator<iterator>
    rend() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return std::make_reverse_iterator(begin());
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE
        ATTRIBUTE_PURE constexpr std::reverse_iterator<const_iterator>
        rbegin() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return std::make_reverse_iterator(end());
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE
        ATTRIBUTE_PURE constexpr std::reverse_iterator<const_iterator>
        rend() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return std::make_reverse_iterator(begin());
    }
    constexpr void change_sign() noexcept {
        size_ = -size_;
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_CONST static constexpr std::size_t
    max_size() noexcept {
        constexpr auto kMaxSSize = std::numeric_limits<decltype(longint::size_)>::max();
        static_assert(kMaxSSize > 0);
        constexpr auto kMaxUSize = std::numeric_limits<decltype(longint::capacity_)>::max();
        return std::min({static_cast<std::size_t>(kMaxSSize), static_cast<std::size_t>(kMaxUSize),
                         std::numeric_limits<std::size_t>::max() / sizeof(digit_t)});
    }

    void set_string(std::string_view s) {
#if CONFIG_HAS_AT_LEAST_CXX_20
        const unsigned char* str_iter = std::bit_cast<const unsigned char*>(s.begin());
        const unsigned char* str_end  = std::bit_cast<const unsigned char*>(s.end());
#else
        const unsigned char* str_iter = reinterpret_cast<const unsigned char*>(s.begin());
        const unsigned char* str_end  = reinterpret_cast<const unsigned char*>(s.end());
#endif
        std::int32_t sgn        = 1;
        constexpr auto is_digit = [](std::uint32_t chr) constexpr noexcept {
            return chr - '0' <= '9' - '0';
        };
        while (str_iter != str_end && !is_digit(*str_iter)) {
            sgn = 1 - std::int32_t(uint32_t(*str_iter == '-') * 2);
            ++str_iter;
        }

        while (str_iter != str_end && *str_iter == '0') {
            ++str_iter;
        }

        const auto digits_count = static_cast<std::size_t>(str_end - str_iter);
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

        const std::size_t str_conv_digits_size =
            (digits_count + kStrConvBaseDigits - 1) / kStrConvBaseDigits;
        const std::size_t aligned_str_conv_digits_size =
            math_functions::nearest_greater_equal_power_of_two(str_conv_digits_size);
        reserveUninitializedWithoutCopy(uint32_t(aligned_str_conv_digits_size));
        uint32_t* str_conv_digits = nums_;

        {
            uint32_t* str_conv_digits_iter = str_conv_digits + str_conv_digits_size;
            std::fill_n(str_conv_digits_iter, aligned_str_conv_digits_size - str_conv_digits_size,
                        uint32_t(0));
            if (std::size_t offset = digits_count % kStrConvBaseDigits) {
                uint32_t current = 0;
                do {
                    current = current * 10 + uint32_t(*str_iter) - '0';
                    str_iter++;
                } while (--offset > 0);
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

        std::size_t m = 2 * aligned_str_conv_digits_size;
        if (m > kFFTPrecisionBorder) {
            m *= 2;
        }
        ensureDecBasePowsCapacity(
            math_functions::log2_floor(uint64_t(aligned_str_conv_digits_size)));
        // Allocate m complex numbers for p1 and m complex numbers for p2
        std::size_t max_fft_poly_length = 2 * m;
        uint32_t* mult_add_buffer =
            static_cast<uint32_t*>(operator new(aligned_str_conv_digits_size * sizeof(uint32_t) +
                                                max_fft_poly_length * sizeof(fft::complex)));
        fft::complex* fft_poly_buffer =
            reinterpret_cast<fft::complex*>(mult_add_buffer + aligned_str_conv_digits_size);
        const longint* conv_dec_base_pows_iter = conv_dec_base_pows.data();
        for (std::size_t half_len = 1; half_len != aligned_str_conv_digits_size;
             half_len *= 2, ++conv_dec_base_pows_iter) {
            for (std::size_t pos = 0; pos != aligned_str_conv_digits_size; pos += 2 * half_len) {
                convertDecBaseMultAdd(str_conv_digits + pos, half_len, conv_dec_base_pows_iter,
                                      mult_add_buffer, fft_poly_buffer);
            }
        }
        operator delete(mult_add_buffer);

        std::size_t usize_value = aligned_str_conv_digits_size;
        while (usize_value > 0 && nums_[usize_value - 1] == 0) {
            usize_value--;
        }
        this->size_ = sgn * std::int32_t(usize_value);
    }

    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr bool fits_in_uint32()
        const noexcept {
        return std::uint32_t(size_) <= 1;
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr std::uint32_t to_uint32()
        const noexcept {
        switch (usize()) {
            default:
                if (fits_in_uint32()) {
                    CONFIG_UNREACHABLE();
                }
                [[fallthrough]];
            case 1:
                return nums_[0];
            case 0:
                return 0;
        }
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr bool fits_in_uint64()
        const noexcept {
        return std::uint32_t(size_) <= 2;
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr std::uint64_t to_uint64()
        const noexcept {
        std::uint64_t value = 0;
        switch (usize()) {
            default:
                if (fits_in_uint64()) {
                    CONFIG_UNREACHABLE();
                }
                [[fallthrough]];
            case 2:
                value |= std::uint64_t(nums_[1]) << 32;
                [[fallthrough]];
            case 1:
                value |= nums_[0];
                break;
            case 0:
                break;
        }
        return value;
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr operator std::uint32_t()
        const noexcept {
        return to_uint32();
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr operator std::uint64_t()
        const noexcept {
        return to_uint64();
    }
#if defined(INTEGERS_128_BIT_HPP)
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr bool fits_in_uint128()
        const noexcept {
        return std::uint32_t(size_) <= 4;
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr uint128_t to_uint128()
        const noexcept {
        uint128_t value = 0;
        switch (usize()) {
            default:
                if (fits_in_uint128()) {
                    CONFIG_UNREACHABLE();
                }
                [[fallthrough]];
            case 4:
                value |= uint128_t(nums_[3]) << 96;
                [[fallthrough]];
            case 3:
                value |= uint128_t(nums_[2]) << 64;
                [[fallthrough]];
            case 2:
                value |= std::uint64_t(nums_[1]) << 32;
                [[fallthrough]];
            case 1:
                value |= nums_[0];
                break;
            case 0:
                break;
        }
        return value;
    }
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr operator uint128_t() const noexcept {
        return to_uint128();
    }
#endif

    std::string to_string() const {
        std::string s;
        append_to_string(s);
        return s;
    }
    /// @brief Write 10-base representation of *this into the @a ans
    /// @param ans
    void to_string(std::string& s) const {
        s.clear();
        append_to_string(s);
    }
    void append_to_string(std::string& ans) const {
        if (size_ < 0) {
            ans.push_back('-');
        }

        const std::size_t usize_value = usize();
        assert(usize_value <= max_size());
        switch (usize_value) {
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

        std::size_t n = math_functions::nearest_greater_equal_power_of_two(usize_value);
        ensureBinBasePowsCapacity(math_functions::log2_floor(n));
        uint32_t* knums = static_cast<uint32_t*>(allocate(n));
        std::fill_n(std::copy_n(nums_, usize_value, knums), n - usize_value, uint32_t(0));
        Decimal result = convertBinBase(knums, n);
        deallocate(knums);
        assert(result.size_ >= 3);
        std::size_t full_blocks = result.size_ - 1;
        uint32_t last_a_i       = result.digits_[full_blocks];
        std::size_t string_size =
            full_blocks * kStrConvBaseDigits + math_functions::base_10_len(last_a_i);
        ans.resize((size_ < 0) + string_size);
        uint8_t* ptr = reinterpret_cast<uint8_t*>(&ans[ans.size() - 1]);
        for (std::size_t i = 0; i < full_blocks; i++) {
            uint32_t a_i = result.digits_[i];
            for (std::size_t j = kStrConvBaseDigits; j > 0; j--) {
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
    friend std::string to_string(const longint& n) {
        return n.to_string();
    }
    friend std::ostream& operator<<(std::ostream& out, const longint& n) {
        std::string buffer;
        n.to_string(buffer);
        return out << buffer;
    }
    friend std::istream& operator>>(std::istream& in, longint& n) {
        std::string s;
        in >> s;
        n.set_string(s);
        return in;
    }

    void reserve(const std::size_t requested_capacity) {
        const auto checked_capacity = check_capacity(requested_capacity);
        if (checked_capacity > capacity_) {
            uint32_t* new_nums = static_cast<uint32_t*>(allocate(checked_capacity));
            if (nums_) {
                std::uninitialized_copy_n(nums_, usize(), new_nums);
                deallocate(nums_);
            }
            nums_     = new_nums;
            capacity_ = checked_capacity;
        }
    }

    ~longint() {
        deallocate(nums_);
    }

    struct Decimal {
        static constexpr uint32_t kDecimalBase    = kStrConvBase;
        static constexpr uint32_t kFftDecimalBase = 1'000;

        uint32_t* digits_ = nullptr;
        std::size_t size_ = 0;

        constexpr Decimal() noexcept = default;
        explicit Decimal(uint32_t n) {
            digits_      = static_cast<uint32_t*>(allocate(2));
            uint32_t low = n % kDecimalBase;
            digits_[0]   = low;
            uint32_t hi  = n / kDecimalBase;
            digits_[1]   = hi;
            size_        = hi != 0 ? 2 : low != 0;
        }
        explicit Decimal(uint64_t n) {
            digits_      = static_cast<uint32_t*>(allocate(3));
            uint32_t low = uint32_t(n % kDecimalBase);
            uint64_t t   = n / kDecimalBase;
            uint32_t mid = uint32_t(t % kDecimalBase);
            uint32_t hi  = uint32_t(t / kDecimalBase);
            digits_[0]   = low;
            digits_[1]   = mid;
            digits_[2]   = hi;
            size_        = hi != 0 ? 3 : (mid != 0 ? 2 : low != 0);
        }
        Decimal(const Decimal& other) : digits_(nullptr), size_(other.size_) {
            if (size_) {
                digits_ = static_cast<uint32_t*>(allocate(size_));
                std::copy_n(other.digits_, size_, digits_);
            }
        }
        Decimal& operator=(const Decimal& other) {
            return *this = Decimal(other);
        }
#if CONFIG_HAS_AT_LEAST_CXX_20
        constexpr
#endif
            void
            swap(Decimal& other) noexcept {
            std::swap(digits_, other.digits_);
            std::swap(size_, other.size_);
        }
#if CONFIG_HAS_AT_LEAST_CXX_20
        constexpr
#endif
            friend void
            swap(Decimal& lhs, Decimal& rhs) noexcept {
            lhs.swap(rhs);
        }
        constexpr Decimal(Decimal&& other) noexcept : digits_(other.digits_), size_(other.size_) {
            other.digits_ = nullptr;
            other.size_   = 0;
        }
#if CONFIG_HAS_AT_LEAST_CXX_20
        constexpr
#endif
            Decimal&
            operator=(Decimal&& other) noexcept {
            swap(other);
            return *this;
        }

        Decimal& operator=(uint32_t n) {
            if (digits_ == nullptr) {
                digits_ = static_cast<uint32_t*>(allocate(2));
            }

            uint32_t low = n % kDecimalBase;
            digits_[0]   = low;
            uint32_t hi  = n / kDecimalBase;
            digits_[1]   = hi;
            size_        = hi != 0 ? 2 : low != 0;
            return *this;
        }

        Decimal& operator=(uint64_t n) {
            if (digits_ == nullptr) {
                digits_ = static_cast<uint32_t*>(allocate(3));
            }

            uint32_t low = uint32_t(n % kDecimalBase);
            digits_[0]   = low;
            uint32_t t   = uint32_t(n / kDecimalBase);
            uint32_t mid = t % kDecimalBase;
            uint32_t hi  = t / kDecimalBase;
            digits_[1]   = mid;
            digits_[2]   = hi;
            size_        = hi != 0 ? 3 : (mid != 0 ? 2 : low != 0);
            return *this;
        }

        Decimal& operator*=(const Decimal& other) {
            std::size_t k         = size_;
            std::size_t m         = other.size_;
            const uint32_t* k_ptr = digits_;
            const uint32_t* m_ptr = other.digits_;

            if (k < m) {
                std::swap(k_ptr, m_ptr);
                std::swap(k, m);
            }

            if (unlikely(m == 0)) {
                size_ = 0;
                return *this;
            }

            std::size_t new_size = m + k;
            if (m <= 16 || m * k <= 1024) {
                uint32_t* ans = static_cast<uint32_t*>(allocate(new_size));
                std::fill_n(ans, new_size, uint32_t(0));
                for (std::size_t j = 0; j < m; j++) {
                    uint64_t b_j   = m_ptr[j];
                    uint64_t carry = 0;
                    for (std::size_t i = 0; i < k; i++) {
                        uint64_t a_i = k_ptr[i];
                        uint64_t res = a_i * b_j + uint64_t(ans[j + i]) + carry;
                        ans[j + i]   = uint32_t(res % kDecimalBase);
                        carry        = res / kDecimalBase;
                    }
                    ans[j + k] = uint32_t(carry % kDecimalBase);
                }

                deallocate(this->digits_);
                this->digits_ = ans;
            } else {
                std::size_t n = math_functions::nearest_greater_equal_power_of_two(3 * new_size);
                static_assert(kFftDecimalBase * kFftDecimalBase * kFftDecimalBase == kDecimalBase,
                              "");
                // Allocate n for the first polynomial
                //  and n for the second one
                auto p1         = std::make_unique<fft::complex[]>(n + n);
                fft::complex* p = p1.get();
                for (std::size_t i = 0; i < m; i++) {
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
                for (std::size_t i = m; i < k; i++) {
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
                    deallocate(this->digits_);
                    this->digits_ = static_cast<uint32_t*>(allocate(new_size));
                }

                fft::complex* p2 = p1.get() + n;
                fft::forward_backward_fft(p1.get(), p2, n);

                uint64_t carry            = 0;
                uint32_t* ans_p           = this->digits_;
                const uint32_t* ans_p_end = ans_p + new_size;
                do {
                    uint64_t res = carry;
                    res += uint64_t(p2->real() + kFFTFloatRoundError);
                    p2++;
                    res += uint64_t(p2->real() + kFFTFloatRoundError) * kFftDecimalBase;
                    p2++;
                    res += uint64_t(p2->real() + kFFTFloatRoundError) *
                           (kFftDecimalBase * kFftDecimalBase);
                    p2++;
                    *ans_p = uint32_t(res % kDecimalBase);
                    carry  = res / kDecimalBase;
                    ans_p++;
                } while (ans_p != ans_p_end);
            }

            size_ = new_size;
            this->popLeadingZeros();
            return *this;
        }

        Decimal& operator+=(const Decimal& other) {
            uint64_t carry      = 0;
            const std::size_t m = std::min(this->size_, other.size_);
            uint32_t* p         = this->digits_;
            for (std::size_t i = 0; i < m; i++) {
                uint64_t res = uint64_t(p[i]) + uint64_t(other.digits_[i]) + carry;
                p[i]         = uint32_t(res % kDecimalBase);
                carry        = res / kDecimalBase;
            }

            if (size_ < other.size_) {
                uint32_t* new_digits = static_cast<uint32_t*>(allocate(other.size_));
                std::copy_n(other.digits_ + size_, other.size_ - size_,
                            std::copy_n(digits_, size_, new_digits));
                deallocate(this->digits_);
                this->digits_ = new_digits;
                size_         = other.size_;
            }

            p                           = this->digits_;
            const std::size_t this_size = size_;
            for (std::size_t i = m; carry != 0 && i < this_size; i++) {
                uint64_t res = uint64_t(p[i]) + carry;
                p[i]         = uint32_t(res % kDecimalBase);
                carry        = res / kDecimalBase;
            }

            if (carry == 0) {
                popLeadingZeros();
            } else {
                uint32_t* new_digits =
                    static_cast<uint32_t*>(allocate((this_size + 1 + (this_size == 0))));
                pointer new_digits_copy_end = std::copy_n(digits_, this_size, new_digits);
                *new_digits_copy_end        = uint32_t(carry);
                deallocate(this->digits_);
                this->digits_ = new_digits;
                size_         = this_size + 1;
            }

            return *this;
        }

        void SquareThisTo(Decimal& other) const {
            std::size_t digits_size = size_;
            if (unlikely(digits_size == 0)) {
                other.size_ = 0;
                return;
            }

            const uint32_t* digits_ptr = this->digits_;
            std::size_t prod_size      = digits_size + digits_size;
            if (prod_size <= 16) {
                uint32_t* ans = static_cast<uint32_t*>(allocate(prod_size));
                std::fill_n(ans, prod_size, uint32_t(0));
                for (std::size_t j = 0; j < digits_size; j++) {
                    uint64_t b_j   = digits_ptr[j];
                    uint64_t carry = 0;
                    for (std::size_t i = 0; i < digits_size; i++) {
                        uint64_t a_i = digits_ptr[i];
                        uint64_t res = a_i * b_j + uint64_t(ans[j + i]) + carry;
                        ans[j + i]   = uint32_t(res % kDecimalBase);
                        carry        = res / kDecimalBase;
                    }
                    ans[j + digits_size] = uint32_t(carry % kDecimalBase);
                }

                deallocate(other.digits_);
                other.digits_ = ans;
            } else {
                std::size_t n = math_functions::nearest_greater_equal_power_of_two(3 * prod_size);
                static_assert(kFftDecimalBase * kFftDecimalBase * kFftDecimalBase == kDecimalBase,
                              "");
                // Allocate n for the first polynomial
                //  and n for the second one
                auto p1         = std::make_unique<fft::complex[]>(n + n);
                fft::complex* p = p1.get();
                for (std::size_t i = 0; i < digits_size; i++) {
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
                    deallocate(other.digits_);
                    other.digits_ = static_cast<uint32_t*>(allocate(prod_size));
                }

                fft::complex* p2 = p1.get() + n;
                fft::forward_backward_fft(p1.get(), p2, n);

                uint64_t carry            = 0;
                uint32_t* ans_p           = other.digits_;
                const uint32_t* ans_p_end = ans_p + prod_size;
                do {
                    uint64_t res = carry;
                    res += uint64_t(p2->real() + kFFTFloatRoundError);
                    p2++;
                    res += uint64_t(p2->real() + kFFTFloatRoundError) * kFftDecimalBase;
                    p2++;
                    res += uint64_t(p2->real() + kFFTFloatRoundError) *
                           (kFftDecimalBase * kFftDecimalBase);
                    p2++;
                    *ans_p = uint32_t(res % kDecimalBase);
                    carry  = res / kDecimalBase;
                    ans_p++;
                } while (ans_p != ans_p_end);
            }

            other.size_ = prod_size;
            other.popLeadingZeros();
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

        constexpr void popLeadingZeros() noexcept {
            std::size_t usize_value = size_;
            // std::find_first_of(std::make_reverse_iterator(begin))
            while (usize_value > 0 && digits_[usize_value - 1] == 0) {
                usize_value--;
            }

            size_ = usize_value;
        }

        ~Decimal() {
            deallocate(digits_);
        }
    };

private:
    void reserveUninitializedWithoutCopy(uint32_t capacity) {
        if (capacity > capacity_) {
            deallocate(nums_);
            nums_     = static_cast<uint32_t*>(allocate(capacity));
            capacity_ = capacity;
        }
        size_ = 0;
    }

    static inline std::vector<Decimal> conv_bin_base_pows = {longint::Decimal(longint::kNumsBase)};
    static std::vector<longint> conv_dec_base_pows;

    static void ensureDecBasePowsCapacity(std::size_t pows_size) {
        std::size_t i = conv_dec_base_pows.size();
        if (i >= pows_size) {
            return;
        }
        conv_dec_base_pows.reserve(pows_size);
        do {
            conv_dec_base_pows.emplace_back();
            conv_dec_base_pows[i - 1].SquareThisTo(conv_dec_base_pows.back());
        } while (++i != pows_size);
    }

    static void convertDecBaseMultAdd(uint32_t conv_digits[], std::size_t half_len,
                                      const longint* conv_base_pow, uint32_t mult_add_buffer[],
                                      fft::complex fft_poly_buffer[]) {
        const uint32_t* num_hi = conv_digits + half_len;
        std::size_t m          = uint32_t(conv_base_pow->size_);
        assert(m > 0 && m <= half_len);
        const uint32_t* m_ptr = conv_base_pow->nums_;
        std::size_t prod_size = m + half_len;
        std::fill_n(mult_add_buffer, half_len + half_len, uint32_t(0));
        if (half_len <= 32) {
            for (std::size_t j = 0; j < m; j++) {
                uint64_t b_j   = m_ptr[j];
                uint64_t carry = 0;
                for (std::size_t i = 0; i < half_len; i++) {
                    uint64_t a_i           = num_hi[i];
                    uint64_t res           = a_i * b_j + uint64_t(mult_add_buffer[j + i]) + carry;
                    mult_add_buffer[j + i] = uint32_t(res);
                    carry                  = res >> kNumsBits;
                }

                mult_add_buffer[j + half_len] = uint32_t(carry);
            }
        } else {
            std::size_t n = 2 * math_functions::nearest_greater_equal_power_of_two(prod_size);
            const bool high_precision = n > kFFTPrecisionBorder;
            n <<= high_precision;
            fft::complex* p1 = fft_poly_buffer;
            fft::complex* p  = p1;
            if (likely(!high_precision)) {
                for (std::size_t i = 0; i < m; i++) {
                    uint32_t m_value = m_ptr[i];
                    uint32_t k_value = num_hi[i];
                    *p               = fft::complex(m_value & 0xFFFF, k_value & 0xFFFF);
                    p++;
                    *p = fft::complex(m_value >> 16, k_value >> 16);
                    p++;
                }
                for (std::size_t i = m; i < half_len; i++) {
                    uint32_t k_value = num_hi[i];
                    *p               = fft::complex(0, k_value & 0xFFFF);
                    p++;
                    *p = fft::complex(0, k_value >> 16);
                    p++;
                }
            } else {
                for (std::size_t i = 0; i < m; i++) {
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
                for (std::size_t i = m; i < half_len; i++) {
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
                    res += uint64_t(p2->real() + kFFTFloatRoundError);
                    p2++;
                    res += uint64_t(p2->real() + kFFTFloatRoundError) << 16;
                    p2++;
                    *ans_p = uint32_t(res);
                    carry  = res >> kNumsBits;
                    ans_p++;
                } while (ans_p != ans_p_end);
            } else {
                do {
                    uint64_t res = carry;
                    res += uint64_t(p2->real() + kFFTFloatRoundError);
                    p2++;
                    res += uint64_t(p2->real() + kFFTFloatRoundError) << 8;
                    p2++;
                    res += uint64_t(p2->real() + kFFTFloatRoundError) << 16;
                    p2++;
                    res += uint64_t(p2->real() + kFFTFloatRoundError) << 24;
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
        for (std::size_t i = half_len; i > 0; i--, conv_digits++, mult_add_buffer++) {
            uint64_t res = uint64_t(*conv_digits) + uint64_t(*mult_add_buffer) + carry;
            *conv_digits = uint32_t(res);
            carry        = res >> kNumsBits;
        }
        for (std::size_t i = half_len; i > 0; i--, conv_digits++, mult_add_buffer++) {
            uint64_t res = uint64_t(*mult_add_buffer) + carry;
            *conv_digits = uint32_t(res);
            carry        = res >> kNumsBits;
        }
        assert(carry == 0);
    }

    static Decimal convertBinBase(const uint32_t* nums, std::size_t size) {
        assert(math_functions::is_power_of_two(size));
        ATTRIBUTE_ASSUME(math_functions::is_power_of_two(size));
        switch (size) {
            case 0:
            case 1:
                return Decimal(nums[0]);
            case 2:
                return Decimal(uint64_t(nums[1]) * kNumsBase | nums[0]);
        }

        Decimal low_dec  = convertBinBase(nums, size / 2);
        Decimal high_dec = convertBinBase(nums + size / 2, size / 2);

        const std::size_t idx = math_functions::log2_floor(size) - 1;
        assert(idx < conv_bin_base_pows.size());
        high_dec *= conv_bin_base_pows[idx];
        high_dec += low_dec;
        return high_dec;
    }

    static void ensureBinBasePowsCapacity(std::size_t pows_size) {
        std::size_t i = conv_bin_base_pows.size();
        if (i >= pows_size) {
            return;
        }
        conv_bin_base_pows.reserve(pows_size);
        do {
            conv_bin_base_pows.emplace_back();
            conv_bin_base_pows[i - 1].SquareThisTo(conv_bin_base_pows.back());
        } while (++i != pows_size);
    }

    ATTRIBUTE_NOINLINE ATTRIBUTE_COLD void growCapacity() {
        reserve((std::size_t(capacity_) * 2) | (capacity_ == 0));
    }

    ATTRIBUTE_NOINLINE ATTRIBUTE_COLD std::size_t growSizeByOne() {
        std::size_t usize_value = usize();
        if (unlikely(usize_value == capacity_)) {
            growCapacity();
        }

        size_ += sign();
        return usize_value + 1;
    }

    std::size_t setSizeAtLeast(std::size_t new_size) {
        std::size_t cur_size = usize();
        if (new_size <= cur_size) {
            return cur_size;
        }

        reserve(new_size);
        std::fill_n(std::addressof(nums_[cur_size]), new_size - cur_size, std::uint32_t{});
        size_ = size_ >= 0 ? std::int32_t(new_size) : -int32_t(new_size);
        return new_size;
    }

    void ensureDefaultCapacityOpEqCall() {
        if (capacity_ < kDefaultLINumsCapacity) {
            deallocate(nums_);
            allocateDefaultCapacity();
        }
    }

    void allocateDefaultCapacity() {
        nums_     = static_cast<uint32_t*>(allocate(kDefaultLINumsCapacity));
        capacity_ = kDefaultLINumsCapacity;
    }

    static constexpr uint64_t longIntAdd(uint32_t nums1[], const uint32_t nums2[],
                                         std::size_t usize1, std::size_t usize2) noexcept {
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

    void nonZeroSizeAddUInt(uint32_t n) {
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
            std::size_t usize_value = usize();
            if (unlikely(usize_value == capacity_)) {
                growCapacity();
            }

            assert(usize_value < capacity_);
            nums_[usize_value] = uint32_t(carry);
            size_ += sign();
        }
    }

    constexpr void nonZeroSizeSubUInt(uint32_t n) {
        std::size_t usize_value = usize();
        uint32_t* nums_iter     = nums_;
        uint32_t low_num        = nums_iter[0];
        if (usize_value != 1) {
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

    ATTRIBUTE_ALWAYS_INLINE static void* allocate(std::size_t nums) {
        return ::longint_allocator::Allocate(nums * sizeof(std::uint32_t));
    }
    ATTRIBUTE_ALWAYS_INLINE static void deallocate(std::uint32_t* nums) noexcept {
        ::longint_allocator::Deallocate(static_cast<void*>(nums));
    }
    struct ComplexDeleter {
        void operator()(fft::complex* memory) noexcept {
            ::operator delete(static_cast<void*>(memory));
        }
    };
    ATTRIBUTE_ALWAYS_INLINE static fft::complex* allocate_complex_array_for_unique_ptr(
        std::size_t size) {
        const auto size_bytes = size * sizeof(fft::complex);
        return static_cast<fft::complex*>(::operator new(size_bytes));
    }
    ATTRIBUTE_ALWAYS_INLINE static std::uint32_t check_capacity(
        const std::size_t requested_capacity) {
        if (unlikely(requested_capacity > max_size())) {
            throw_size_error(requested_capacity, __FILE__, __LINE__, FUNCTION_MACRO);
        }
        const auto value = static_cast<std::uint32_t>(requested_capacity);
        if (value > max_size()) {
            CONFIG_UNREACHABLE();
        }
        return value;
    }
    [[noreturn]] ATTRIBUTE_NOINLINE ATTRIBUTE_COLD static void throw_size_error(
        std::size_t new_size, const char* file_name, std::uint32_t line,
        const char* function_name) {
        char message[1024] = {};
        int bytes_written  = std::snprintf(message, std::size(message),
                                           "%s:%u: size error at %s: %zu > %zu = max_size()",
                                           file_name, line, function_name, new_size, max_size());
        if (unlikely(bytes_written < 0)) {
            message[0] = '\0';
        }
        throw std::length_error(message);
    }
};

std::vector<longint> longint::conv_dec_base_pows = {longint(longint::kStrConvBase)};
