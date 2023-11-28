// #define NDEBUG 1

#include <cassert>
#include <cinttypes>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <new>
#include <string>
#include <string_view>
#include <vector>

#include "integers_128_bit.hpp"
#include "math_utils.hpp"

namespace LongIntAllocator {

// #define DEBUG_LI_ALLOC_PRINTING

namespace InnerImpl {

enum PageCapacity {
    kSmall = 30 * sizeof(uint32_t),
    kMiddle = 254 * sizeof(uint32_t)
};

struct SmallPage {
    SmallPage* next;
    char memory[PageCapacity::kSmall];
};

struct MiddlePage {
    MiddlePage* next;
    char memory[PageCapacity::kMiddle];
};

inline constexpr size_t kPageMemoryOffsetInBytes = sizeof(void*);

inline constexpr size_t kTotalSmallPages = 32;
inline constexpr size_t kTotalMiddlePages = 8;

static SmallPage first_small_page[kTotalSmallPages] = {};
static MiddlePage first_middle_page[kTotalMiddlePages] = {};

static SmallPage* free_small_pages_head = nullptr;
static MiddlePage* free_middle_pages_head = nullptr;

#ifdef DEBUG_LI_ALLOC_PRINTING
static size_t total_small_pages_used = 0;
static int32_t current_small_pages_used = 0;
static int32_t max_small_pages_used = -(1 << 30);
static size_t total_middle_pages_used = 0;
static int32_t current_middle_pages_used = 0;
static int32_t max_middle_pages_used = -(1 << 30);
static ssize_t bytes_allocated = 0;
static int32_t malloc_free_count = 0;
#endif

static inline void LongIntAllocatorInitSmallPages() noexcept {
    SmallPage* p = first_small_page;
    free_small_pages_head = p;
    for (const SmallPage* last_page = p + kTotalSmallPages - 1;
         p != last_page;) {
        SmallPage* p_next = p + 1;
        p->next = p_next;
        p = p_next;
    }
    p->next = nullptr;
}

static inline void LongIntAllocatorInitMiddlePages() noexcept {
    MiddlePage* p = first_middle_page;
    free_middle_pages_head = p;
    for (const MiddlePage* p_iter_end = p + kTotalMiddlePages - 1;
         p != p_iter_end;) {
        MiddlePage* p_next = p + 1;
        p->next = p_next;
        p = p_next;
    }
    p->next = nullptr;
}

/**
 * VS Code intellisense: "attribute "constructor" does not take arguments
 * C/C++(1094)"
 */
#if __INTELLISENSE__
#pragma diag_suppress 1094
#endif

__attribute__((constructor(101))) static void
LongIntAllocatorInitPages() noexcept {
    LongIntAllocatorInitSmallPages();
    LongIntAllocatorInitMiddlePages();
#ifdef DEBUG_LI_ALLOC_PRINTING
    printf("[INIT] Inited pages in %s\n", __PRETTY_FUNCTION__);
#endif
}

__attribute__((destructor(101))) static void
LongIntAllocatorDeinitPages() noexcept {
    free_small_pages_head = nullptr;
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
        __PRETTY_FUNCTION__, total_small_pages_used, max_small_pages_used,
        current_small_pages_used, total_middle_pages_used,
        max_middle_pages_used, current_middle_pages_used, bytes_allocated,
        malloc_free_count);
#endif
}

static inline bool IsSmallPage(const char* offset_memory) noexcept {
    const char* fsp = reinterpret_cast<const char*>(&first_small_page[0]);
    return static_cast<size_t>(offset_memory - fsp) <
           kTotalSmallPages * sizeof(SmallPage);
}

static inline bool IsMiddlePage(const char* offset_memory) noexcept {
    const char* fmp = reinterpret_cast<const char*>(&first_middle_page[0]);
    return static_cast<size_t>(offset_memory - fmp) <
           kTotalMiddlePages * sizeof(MiddlePage);
}

}  // namespace InnerImpl

void* Allocate(size_t size) noexcept(false) {
    if (size <= InnerImpl::PageCapacity::kSmall &&
        InnerImpl::free_small_pages_head != nullptr) {
        InnerImpl::SmallPage* p = InnerImpl::free_small_pages_head;
#ifdef DEBUG_LI_ALLOC_PRINTING
        InnerImpl::current_small_pages_used++;
        if (InnerImpl::current_small_pages_used >
            InnerImpl::max_small_pages_used) {
            InnerImpl::max_small_pages_used =
                InnerImpl::current_small_pages_used;
        }
        InnerImpl::total_small_pages_used++;
#endif
        InnerImpl::free_small_pages_head = p->next;
        return static_cast<void*>(reinterpret_cast<char*>(p) +
                                  InnerImpl::kPageMemoryOffsetInBytes);
    }

    if (size <= InnerImpl::PageCapacity::kMiddle &&
        InnerImpl::free_middle_pages_head != nullptr) {
        InnerImpl::MiddlePage* p = InnerImpl::free_middle_pages_head;
#ifdef DEBUG_LI_ALLOC_PRINTING
        InnerImpl::current_middle_pages_used++;
        if (InnerImpl::current_middle_pages_used >
            InnerImpl::max_middle_pages_used) {
            InnerImpl::max_middle_pages_used =
                InnerImpl::current_middle_pages_used;
        }
        InnerImpl::total_middle_pages_used++;
#endif
        InnerImpl::free_middle_pages_head = p->next;
        return static_cast<void*>(reinterpret_cast<char*>(p) +
                                  InnerImpl::kPageMemoryOffsetInBytes);
    }

    void* p = malloc(size);
    if (__builtin_expect(p == nullptr, 0)) {
        throw std::bad_alloc();
    }
#ifdef DEBUG_LI_ALLOC_PRINTING
    InnerImpl::bytes_allocated += static_cast<ssize_t>(size);
    InnerImpl::malloc_free_count++;
#endif
    return p;
}

void Deallocate(void* memory) noexcept {
    if (unlikely(memory == nullptr)) {
        return;
    }

    char* p = static_cast<char*>(memory) - InnerImpl::kPageMemoryOffsetInBytes;
    if (InnerImpl::IsSmallPage(p)) {
#ifdef DEBUG_LI_ALLOC_PRINTING
        InnerImpl::current_small_pages_used--;
#endif
        InnerImpl::SmallPage* page = reinterpret_cast<InnerImpl::SmallPage*>(p);
        page->next = InnerImpl::free_small_pages_head;
        InnerImpl::free_small_pages_head = page;
        return;
    }

    if (InnerImpl::IsMiddlePage(p)) {
#ifdef DEBUG_LI_ALLOC_PRINTING
        InnerImpl::current_middle_pages_used--;
#endif
        InnerImpl::MiddlePage* page =
            reinterpret_cast<InnerImpl::MiddlePage*>(p);
        page->next = InnerImpl::free_middle_pages_head;
        InnerImpl::free_middle_pages_head = page;
        return;
    }
#ifdef DEBUG_LI_ALLOC_PRINTING
    InnerImpl::malloc_free_count--;
#endif
    free(memory);
}

#ifdef DEBUG_LI_ALLOC_PRINTING
#undef DEBUG_LI_ALLOC_PRINTING
#endif

}  // namespace LongIntAllocator

using f64 = double;
using complex = std::complex<f64>;

#if __cplusplus >= 202002L
#include <numbers>
inline constexpr f64 PI = std::numbers::pi_v<f64>;
#else
const f64 PI = f64(acosl(-1));
#endif

#if __cplusplus >= 202100L && defined(__GNUC__) && !defined(__clang__)
#define constexpr_on_cfunc_call constexpr
#else
#define constexpr_on_cfunc_call
#endif

/*
 * Save only e^{2pi*0/1}, e^{2pi*0/2}, e^{2pi*0/4}, e^{2pi*1/4}, e^{2pi*0/8},
 * e^{2pi*1/8}, e^{2pi*2/8}, e^{2pi*3/8}, ... because only low n / 2 roots are
 * used (fft_roots[0] never used btw, because in fft step >= 1, so it can be
 * anything)
 */
static std::vector<complex> fft_roots = {complex(0, 0), complex(1, 0)};

static inline void EnsureRootsCapacity(size_t n) {
    assert((n & (n - 1)) == 0);
    size_t current_len = fft_roots.size();
    assert((current_len & (current_len - 1)) == 0);
    if (current_len >= n) {
        return;
    }

    fft_roots.reserve(n);
    do {
        for (size_t i = current_len / 2; i != current_len; i++) {
            fft_roots.emplace_back(fft_roots[i]);
            // double phi = 2 * PI * (2 * i - current_len + 1) / (2 *
            // current_len);
            f64 phi = PI * f64(2 * i - current_len + 1) / f64(current_len);
            fft_roots.emplace_back(std::cos(phi), std::sin(phi));
        }
        current_len *= 2;
    } while (current_len < n);
}

static inline void ForwardFFT(complex* p, const size_t k) noexcept {
    for (size_t i = 1, k_reversed_i = 0; i < k; i++) {
        // 'Increase' k_reversed_i by one
        size_t bit = k >> 1;
        for (; k_reversed_i >= bit; bit >>= 1) {
            k_reversed_i -= bit;
        }

        k_reversed_i += bit;
        if (i < k_reversed_i) {
            std::swap(p[i], p[k_reversed_i]);
        }
    }

    const complex* points = fft_roots.data();

    /* Unroll for step = 1 */
    for (size_t block_start = 0; block_start < k; block_start += 2) {
        complex p0_i = p[block_start];
        p[block_start] = p0_i + p[block_start + 1];
        p[block_start + 1] = p0_i - p[block_start + 1];
    }

    for (size_t step = 2; step < k; step *= 2) {
        for (size_t block_start = 0; block_start < k;) {
            size_t point_index = step;
            size_t block_end = block_start + step;
            for (size_t pos_in_block = block_start; pos_in_block < block_end;
                 pos_in_block++, point_index++) {
                complex p0_i = p[pos_in_block];
                auto w_j_p1_i = points[point_index] * p[pos_in_block + step];
                p[pos_in_block] = p0_i + w_j_p1_i;
                p[pos_in_block + step] = p0_i - w_j_p1_i;
            }

            block_start = block_end + step;
        }
    }
}

static inline void BackwardFFT(complex* p, const size_t k) noexcept {
    for (size_t i = 1, k_reversed_i = 0; i < k; i++) {
        // 'Increase' k_reversed_i by one
        size_t bit = k >> 1;
        for (; k_reversed_i >= bit; bit >>= 1) {
            k_reversed_i -= bit;
        }

        k_reversed_i += bit;
        if (i < k_reversed_i) {
            std::swap(p[i], p[k_reversed_i]);
        }
    }

    const complex* points = fft_roots.data();
    for (size_t step = 1, point_step = k / 2; step < k;
         step *= 2, point_step /= 2) {
        for (size_t block_start = 0; block_start < k;) {
            size_t point_index = step;
            size_t block_end = block_start + step;
            for (size_t pos_in_block = block_start; pos_in_block < block_end;
                 pos_in_block++, point_index++) {
                complex p0_i = p[pos_in_block];
                auto w_j_p1_i =
                    std::conj(points[point_index]) * p[pos_in_block + step];
                p[pos_in_block] = p0_i + w_j_p1_i;
                p[pos_in_block + step] = p0_i - w_j_p1_i;
            }

            block_start = block_end + step;
        }
    }

    f64 one_kth = 1.0 / f64(k);
    for (complex *p_iter = p, *p_end = p_iter + k; p_iter != p_end; ++p_iter) {
        *p_iter *= one_kth;
    }
}

static void ForwardBackwardFFT(complex* p1, complex* p2, const size_t n) {
    assert((n != 0) & ((n & (n - 1)) == 0));
    EnsureRootsCapacity(n);
    ForwardFFT(p1, n);
    /*
     * A(w^j) = a_0 + a_1 * w^j + a_2 * w^{2 j} + ... + a_{n - 1} * w^{(n - 1)
     * j} B(w^j) = b_0 + b_1 * w^j + b_2 * w^{2 j} + ... + b_{n - 1} * w^{(n -
     * 1) j}
     *
     * P = A + B * i = [ A(w^0) + B(w^0) * i, A(w^1) + B(w^1) * i, ... A(w^(n -
     * 1)) + B(w^(n - 1)) * i ]
     *
     * P(w^j) + conj(P(w^{n - j})) =
     * = A(w^j) + B(w^j) * i + conj(A(w^{n - j}) + B(w^{n - j}) * i) =
     * = \sum{k=0}{n-1} (a_k + b_k * i) * w^{jk} + \sum{k=0}{n-1} conj((a_k +
     * b_k * i) * w^{(n - j)k}) = = \sum{k=0}{n-1} (a_k + b_k * i) * w^{jk} +
     * \sum{k=0}{n-1} conj((a_k + b_k * i) * w^{-jk}) = = \sum{k=0}{n-1} (a_k +
     * b_k * i) * w^{jk} + conj((a_k + b_k * i) * w^{-jk}) = = \sum{k=0}{n-1}
     * (a_k + b_k * i) * w^{jk} + conj(a_k + b_k * i) * conj(w^{-jk}) = =
     * \sum{k=0}{n-1} (a_k + b_k * i) * w^{jk} + (a_k - b_k * i) * w^{jk} = =
     * \sum{k=0}{n-1} 2 a_k * w^{jk} = 2 * A(w^j)
     *
     * \implies A(w^j) = (P(w^j) + conj(P(w^{n - j}))) / 2
     *
     * By analogy it is can be proved that A(w^j) = (P(w^j) - conj(P(w^{n -
     * j}))) / (2 * i)
     *
     * C(w^j) = A(w^j) * B(w^j) \implies C(w^j) =
     * = (P(w^j) + conj(P(w^{n - j}))) / 2 * (P(w^j) - conj(P(w^{n - j}))) / (2
     * * i) = = (P(w^j) + conj(P(w^{n - j}))) * (P(w^j) - conj(P(w^{n - j}))) /
     * (4 * i) = = (P(w^j) + conj(P(w^{n - j}))) * (P(w^j) - conj(P(w^{n - j})))
     * / (4 * i) =
     */
    complex one_quat_i = complex(0, -0.25);  // 1 / (4 * i) == -i / 4
    for (size_t j = 0; j < n; j++) {
        size_t n_j = (n - j) & (n - 1);  // <=> mod n because n is power of two
        complex p_w_j = p1[j];
        complex p_w_n_j = std::conj(p1[n_j]);
        p2[j] = (p_w_j + p_w_n_j) * (p_w_j - p_w_n_j) * one_quat_i;
    }
    BackwardFFT(p2, n);
}

struct LongInt {
    static inline constexpr size_t kDefaultLINumsCapacity = 2;
    static inline constexpr uint32_t kStrConvBase = 1'000'000'000;
    static inline constexpr uint32_t kStrConvBaseDigits =
        math_utils::base_10_len(kStrConvBase - 1);
    static inline constexpr uint32_t kNumsBits = 32;
    static inline constexpr uint64_t kNumsBase = uint64_t(1) << kNumsBits;
    static inline constexpr size_t kFFTPrecisionBorder = 262144;

    uint32_t* nums_;
    /**
     * size_ < 0 <=> sign = -1; size_ == 0 <=> sign = 0; size > 0 <=> sign = 1
     */
    int32_t size_;
    uint32_t capacity_;

    constexpr LongInt() noexcept : nums_{nullptr}, size_(0), capacity_(0) {}

    inline LongInt(uint32_t n) : size_(n != 0) {
        AllocateDefaultCapacityCtorCall();
        nums_[0] = n;
    }

    inline LongInt(int32_t n) {
        AllocateDefaultCapacityCtorCall();
        int32_t sign = int32_t(n > 0) - int32_t(n < 0);
        nums_[0] = n > 0 ? uint32_t(n) : -uint32_t(n);
        size_ = sign;
    }

    inline LongInt(uint64_t n) : size_(n != 0) {
        AllocateDefaultCapacityCtorCall();
        nums_[0] = uint32_t(n);
        n >>= 32;
        size_ += n != 0;
        nums_[1] = uint32_t(n);
    }

    inline LongInt(int64_t n) {
        AllocateDefaultCapacityCtorCall();
        int32_t sign = int32_t(n > 0) - int32_t(n < 0);
        uint64_t m = n > 0 ? uint64_t(n) : -uint64_t(n);
        size_ = m != 0;
        nums_[0] = uint32_t(m);
        m >>= 32;
        size_ += m != 0;
        nums_[1] = uint32_t(m);
        size_ *= sign;
    }

#if _INTEGERS_128_BIT_
    inline LongInt(uint128_t n) : size_(n != 0), capacity_(4) {
        nums_ = static_cast<uint32_t*>(
            LongIntAllocator::Allocate(4 * sizeof(uint32_t)));
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

    inline LongInt& Pow(size_t p) {
        LongInt res(1u);
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
        size_t prod_size = usize + usize;
        if (prod_size <= 16) {
            other.capacity_ = uint32_t(prod_size);
            uint32_t* ans = static_cast<uint32_t*>(
                LongIntAllocator::Allocate(prod_size * sizeof(uint32_t)));
            memset(ans, 0, prod_size * sizeof(uint32_t));
            for (size_t j = 0; j < usize; j++) {
                uint64_t b_j = nums_ptr[j];
                uint64_t carry = 0;
                for (size_t i = 0; i < usize; i++) {
                    uint64_t a_i = nums_ptr[i];
                    uint64_t res = a_i * b_j + uint64_t(ans[j + i]) + carry;
                    ans[j + i] = uint32_t(res);
                    carry = res >> 32;
                }

                ans[j + usize] = uint32_t(carry);
            }

            LongIntAllocator::Deallocate(other.nums_);
            other.nums_ = ans;
        } else {
            size_t n = 2 * math_utils::nearest_2_pow_greater_equal(prod_size);
            const bool high_precision = n > kFFTPrecisionBorder;
            n <<= high_precision;
            // Allocate n complex numbers for p1 and n complex numbers for p2
            complex* p1 = static_cast<complex*>(
                LongIntAllocator::Allocate(2 * n * sizeof(complex)));
            complex* p = p1;
            if (likely(!high_precision)) {
                for (size_t i = 0; i < usize; i++) {
                    uint32_t value = nums_ptr[i];
                    *p = complex(value & 0xFFFF, value & 0xFFFF);
                    p++;
                    value >>= 16;
                    *p = complex(value, value);
                    p++;
                }
            } else {
                for (size_t i = 0; i < usize; i++) {
                    uint32_t value = nums_ptr[i];
                    *p = complex(uint8_t(value), uint8_t(value));
                    p++;
                    value >>= 8;
                    *p = complex(uint8_t(value), uint8_t(value));
                    p++;
                    value >>= 8;
                    *p = complex(uint8_t(value), uint8_t(value));
                    p++;
                    value >>= 8;
                    *p = complex(value, value);
                    p++;
                }
            }
            memset(static_cast<void*>(p), 0,
                   (n - (prod_size << high_precision)) * sizeof(complex));

            other.ReserveWithoutCopy(uint32_t(prod_size));
            complex* p2 = p1 + n;
            ForwardBackwardFFT(p1, p2, n);
            uint64_t carry = 0;
            uint32_t* ans_p = other.nums_;
            const uint32_t* ans_p_end = ans_p + prod_size;
            if (likely(!high_precision)) {
                do {
                    uint64_t res = carry;
                    res += uint64_t((*p2).real() + 0.5);
                    p2++;
                    res += uint64_t((*p2).real() + 0.5) << 16;
                    p2++;
                    *ans_p = uint32_t(res);
                    carry = res >> 32;
                    ans_p++;
                } while (ans_p != ans_p_end);
            } else {
                do {
                    uint64_t res = carry;
                    res += uint64_t((*p2).real() + 0.5);
                    p2++;
                    res += uint64_t((*p2).real() + 0.5) << 8;
                    p2++;
                    res += uint64_t((*p2).real() + 0.5) << 16;
                    p2++;
                    res += uint64_t((*p2).real() + 0.5) << 24;
                    p2++;
                    *ans_p = uint32_t(res);
                    carry = res >> 32;
                    ans_p++;
                } while (ans_p != ans_p_end);
            }
            assert(carry == 0);
            LongIntAllocator::Deallocate(p1);
        }

        other.size_ = int32_t(prod_size);
        other.PopLeadingZeros();
    }

    inline LongInt& Square() {
        SquareThisTo(*this);
        return *this;
    }

    inline explicit LongInt(std::string_view s)
        : nums_(nullptr), size_(0), capacity_(0) {
        SetString(s);
    }

    inline LongInt(const LongInt& other)
        : nums_(nullptr), size_(other.size_), capacity_(other.capacity_) {
        if (capacity_ != 0) {
            nums_ = static_cast<uint32_t*>(
                LongIntAllocator::Allocate(capacity_ * sizeof(uint32_t)));
            size_t bsz = USize() * sizeof(uint32_t);
            if (bsz != 0) {
                memcpy(nums_, other.nums_, bsz);
            }
        }
    }

    inline LongInt& operator=(const LongInt& other) {
        size_t other_usize = other.USize();
        if (other_usize > capacity_) {
            LongIntAllocator::Deallocate(nums_);
            capacity_ = uint32_t(other_usize);
            nums_ = static_cast<uint32_t*>(
                LongIntAllocator::Allocate(other_usize * sizeof(uint32_t)));
        }
        size_ = other.size_;
        memcpy(nums_, other.nums_, other_usize * sizeof(uint32_t));
        return *this;
    }

    inline constexpr LongInt(LongInt&& other) noexcept
        : nums_(other.nums_), size_(other.size_), capacity_(other.capacity_) {
        other.nums_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    inline LongInt& operator=(LongInt&& other) noexcept {
        LongIntAllocator::Deallocate(nums_);
        nums_ = other.nums_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.nums_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
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

    inline LongInt& operator=(int32_t n) {
        EnsureDefaultCapacityOpEqCall();
        size_ = int32_t(n > 0) - int32_t(n < 0);
        nums_[0] = n > 0 ? uint32_t(n) : -uint32_t(n);
        return *this;
    }

    inline LongInt& operator=(uint32_t n) {
        EnsureDefaultCapacityOpEqCall();
        size_ = n != 0;
        nums_[0] = n;
        return *this;
    }

    inline LongInt& operator=(int64_t n) {
        EnsureDefaultCapacityOpEqCall();
        int32_t sign = int32_t(n > 0) - int32_t(n < 0);
        uint64_t m = n > 0 ? uint64_t(n) : -uint64_t(n);
        size_ = m != 0;
        nums_[0] = uint32_t(m);
        m >>= 32;
        size_ += m != 0;
        nums_[1] = uint32_t(m);
        size_ *= sign;
        return *this;
    }

    inline LongInt& operator=(uint64_t n) {
        EnsureDefaultCapacityOpEqCall();
        size_ = n != 0;
        nums_[0] = uint32_t(n);
        n >>= 32;
        size_ += n != 0;
        nums_[1] = uint32_t(n);
        return *this;
    }

#if _INTEGERS_128_BIT_
    inline LongInt& operator=(uint128_t n) {
        if (capacity_ < 4) {
            LongIntAllocator::Deallocate(nums_);
            nums_ = static_cast<uint32_t*>(
                LongIntAllocator::Allocate(4 * sizeof(uint32_t)));
            capacity_ = 4;
        }

        // size_ = ((128 - std::count_leading_zeros(n)) + 31) / 32;
        size_ = n != 0;
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
            // let compiler decide whether it is faster then 3 xors or not
            size_t tmp = m;
            m = k;
            k = tmp;
        }

        if (unlikely(m == 0)) {
            size_ = 0;
            return *this;
        }

        size_t prod_size = m + k;
        if (m <= 16 || m * k <= 1024) {
            capacity_ = uint32_t(prod_size);
            uint32_t* ans = static_cast<uint32_t*>(
                LongIntAllocator::Allocate(prod_size * sizeof(uint32_t)));
            memset(ans, 0, prod_size * sizeof(uint32_t));
            for (size_t j = 0; j < m; j++) {
                uint64_t b_j = m_ptr[j];
                uint64_t carry = 0;
                for (size_t i = 0; i < k; i++) {
                    uint64_t a_i = k_ptr[i];
                    uint64_t res = a_i * b_j + uint64_t(ans[j + i]) + carry;
                    ans[j + i] = uint32_t(res);
                    carry = res >> 32;
                }

                ans[j + k] = uint32_t(carry);
            }

            LongIntAllocator::Deallocate(nums_);
            nums_ = ans;
        } else {
            size_t n = 2 * math_utils::nearest_2_pow_greater_equal(prod_size);
            const bool high_precision = n > kFFTPrecisionBorder;
            n <<= high_precision;
            // Allocate n complex numbers for p1 and n complex numbers for p2
            complex* p1 = static_cast<complex*>(
                LongIntAllocator::Allocate(2 * n * sizeof(complex)));
            complex* p = p1;
            if (likely(!high_precision)) {
                for (size_t i = 0; i < m; i++) {
                    uint32_t m_value = m_ptr[i];
                    uint32_t k_value = k_ptr[i];
                    *p = complex(m_value & 0xFFFF, k_value & 0xFFFF);
                    p++;
                    *p = complex(m_value >> 16, k_value >> 16);
                    p++;
                }
                for (size_t i = m; i < k; i++) {
                    uint32_t k_value = k_ptr[i];
                    *p = complex(0, k_value & 0xFFFF);
                    p++;
                    *p = complex(0, k_value >> 16);
                    p++;
                }
            } else {
                for (size_t i = 0; i < m; i++) {
                    uint32_t m_value = m_ptr[i];
                    uint32_t k_value = k_ptr[i];
                    *p = complex(uint8_t(m_value), uint8_t(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = complex(uint8_t(m_value), uint8_t(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = complex(uint8_t(m_value), uint8_t(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = complex(uint8_t(m_value), uint8_t(k_value));
                    p++;
                }
                for (size_t i = m; i < k; i++) {
                    uint32_t k_value = k_ptr[i];
                    *p = complex(0, uint8_t(k_value));
                    p++;
                    k_value >>= 8;
                    *p = complex(0, uint8_t(k_value));
                    p++;
                    k_value >>= 8;
                    *p = complex(0, uint8_t(k_value));
                    p++;
                    k_value >>= 8;
                    *p = complex(0, uint8_t(k_value));
                    p++;
                }
            }
            memset(static_cast<void*>(p), 0,
                   (n - ((2 * k) << high_precision)) * sizeof(complex));

            ReserveWithoutCopy(uint32_t(prod_size));
            complex* p2 = p1 + n;
            ForwardBackwardFFT(p1, p2, n);
            uint64_t carry = 0;
            uint32_t* ans_p = nums_;
            const uint32_t* ans_p_end = ans_p + prod_size;
            if (likely(!high_precision)) {
                do {
                    uint64_t res = carry;
                    res += uint64_t((*p2).real() + 0.5);
                    p2++;
                    res += uint64_t((*p2).real() + 0.5) << 16;
                    p2++;
                    *ans_p = uint32_t(res);
                    carry = res >> 32;
                    ans_p++;
                } while (ans_p != ans_p_end);
            } else {
                do {
                    uint64_t res = carry;
                    res += uint64_t((*p2).real() + 0.5);
                    p2++;
                    res += uint64_t((*p2).real() + 0.5) << 8;
                    p2++;
                    res += uint64_t((*p2).real() + 0.5) << 16;
                    p2++;
                    res += uint64_t((*p2).real() + 0.5) << 24;
                    p2++;
                    *ans_p = uint32_t(res);
                    carry = res >> 32;
                    ans_p++;
                } while (ans_p != ans_p_end);
            }
            assert(carry == 0);
            LongIntAllocator::Deallocate(p1);
        }

        int32_t sign_product = size_ ^ other.size_;
        size_ = sign_product >= 0 ? int32_t(prod_size) : int32_t(-prod_size);
        PopLeadingZeros();
        return *this;
    }

    inline LongInt operator*(const LongInt& other) const {
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
            rem = std::move(*this);
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
        uint32_t s =
            static_cast<uint32_t>(math_utils::count_leading_zeros(v[n - 1]));
        uint32_t* vn =
            static_cast<uint32_t*>(LongIntAllocator::Allocate(2 * n));
        for (size_t i = n - 1; i > 0; i--) {
            vn[i] = (v[i] << s) | (v[i - 1] >> (kNumsBase - s));
        }
        vn[0] = v[0] << s;

        uint32_t* un =
            static_cast<uint32_t*>(LongIntAllocator::Allocate(2 * (m + 1)));
        un[m] = u[m - 1] >> (kNumsBase - s);
        for (size_t i = m - 1; i > 0; i--) {
            un[i] = (u[i] << s) | (u[i - 1] >> (kNumsBase - s));
        }
        un[0] = u[0] << s;

        uint32_t* quot = nums_;

        for (size_t j = m - n; static_cast<ssize_t>(j) >= 0; j--) {
            // Compute estimate qhat of q[j].
            uint64_t cur = (uint64_t(un[j + n]) << kNumsBits) | un[j + n - 1];
            uint32_t last_vn = vn[n - 1];
            uint64_t qhat = cur / last_vn;
            uint64_t rhat = cur - qhat * last_vn;
        again:
            if (qhat >= kNumsBase ||
                qhat * vn[n - 2] > kNumsBase * rhat + un[j + n - 2]) {
                qhat--;
                rhat += vn[n - 1];
                if (rhat < kNumsBase) goto again;
            }

            // Multiply and subtract
            uint64_t carry = 0;
            for (size_t i = 0; i < n; i++) {
                uint64_t p = qhat * vn[i];
                uint64_t t = uint64_t(un[i + j]) - carry - uint32_t(p);
                un[i + j] = uint32_t(t);
                carry = (p >> kNumsBits) - (t >> kNumsBits);
            }
            uint64_t t = un[j + n] - carry;
            un[j + n] = uint32_t(t);

            quot[j] = uint32_t(qhat);  // Store quotient digit
            if (static_cast<int64_t>(t) < 0) {
                // If we subtracted too much, add back
                quot[j]--;
                carry = 0;
                for (size_t i = 0; i < n; i++) {
                    t = uint64_t(un[i + j]) + uint64_t(vn[i]) + carry;
                    un[i + j] = uint32_t(t);
                    carry = t >> kNumsBits;
                }
                un[j + n] += uint32_t(carry);
            }
        }

        // Unnormalize remainder
        for (size_t i = 0; i < n; i++) {
            rem.nums_[i] = (un[i] >> s) | (un[i + 1] << (16 - s));
        }

        LongIntAllocator::Deallocate(un);
        LongIntAllocator::Deallocate(vn);

        rem.PopLeadingZeros();
        size_ = int32_t(m - n + 1);
        PopLeadingZeros();
    }

    inline LongInt& operator+=(const LongInt& other) {
        size_t usize2 = other.USize();
        if ((size_ ^ other.size_) >= 0) {
            size_t usize1 = SetSizeAtLeast(usize2 + 1);
            uint64_t add_overflow_carry =
                LongIntAdd(nums_, other.nums_, usize1, usize2);
            if (likely(add_overflow_carry == 0)) {
                PopLeadingZeros();
            } else {
                size_t new_usize1 = GrowSizeByOne();
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
        uint64_t n_abs = n >= 0 ? uint64_t(n) : -uint64_t(n);
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

#if _INTEGERS_128_BIT_
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
                uint64_t hi = (uint64_t(nums_[3]) << 32) | nums_[2];
                return ((uint128_t(hi) << 64) | low) == n;
            }
            default:
                return false;
        }
    }

    inline constexpr bool operator!=(uint128_t n) const noexcept {
        return !(*this == n);
    }
#endif

    constexpr_on_cfunc_call bool operator==(
        const LongInt& other) const noexcept {
        return size_ == other.size_ &&
               memcmp(nums_, other.nums_, USize() * sizeof(uint32_t)) == 0;
    }

    constexpr_on_cfunc_call bool operator!=(
        const LongInt& other) const noexcept {
        return !(*this == other);
    }

    constexpr bool operator<(const LongInt& other) const noexcept {
        if (size_ != other.size_) {
            return size_ < other.size_;
        }

        size_t usize = USize();
        const uint32_t* end = nums_ - 1;
        const uint32_t* nums = end + usize;
        const uint32_t* other_nums = other.nums_ - 1 + usize;
        for (; nums != end; nums--, other_nums--) {
            if (*nums != *other_nums) {
                return ssize_t(*nums) * GetSign() <
                       ssize_t(*other_nums) * other.GetSign();
            }
        }

        return false;
    }

    constexpr bool operator>(const LongInt& other) const noexcept {
        return other < *this;
    }

    constexpr bool operator<=(const LongInt& other) const noexcept {
        return !(other < *this);
    }

    constexpr bool operator>=(const LongInt& other) const noexcept {
        return !(*this < other);
    }

    inline LongInt& operator+=(uint32_t n) {
        if (unlikely(size_ == 0)) {
            if (capacity_ == 0) {
                nums_ = static_cast<uint32_t*>(
                    LongIntAllocator::Allocate(2 * sizeof(uint32_t)));
                capacity_ = 2;
            }

            nums_[0] = n;
            size_ = n != 0;
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

    inline LongInt& operator-=(uint32_t n) {
        if (unlikely(size_ == 0)) {
            if (capacity_ == 0) {
                nums_ = static_cast<uint32_t*>(
                    LongIntAllocator::Allocate(2 * sizeof(uint32_t)));
                capacity_ = 2;
            }

            nums_[0] = -n;
            size_ = -int32_t(n != 0);
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
        if (x == 0) {
            size_ = 0;
            return *this;
        }

        uint64_t carry = 0;
        uint64_t b_0 = x;
        uint32_t u32size = uint32_t(std::abs(size_));
        for (uint32_t *nums_it = nums_, *numes_it_end = nums_it + u32size;
             nums_it != numes_it_end; ++nums_it) {
            uint64_t res = *nums_it * b_0 + carry;
            *nums_it = uint32_t(res);
            carry = res >> 32;
        }

        // x != 0 => sign wont change and there will be no leading zeros
        if (carry != 0) {
            if (unlikely(u32size == capacity_)) {
                GrowCapacity();
            }

            assert(u32size < capacity_);
            nums_[u32size] = uint32_t(carry);
            size_ += GetSign();
        }

        return *this;
    }

    inline LongInt& operator/=(uint32_t n) noexcept {
#if defined(__GNUC__) && !defined(__clang__)
        if (__builtin_constant_p(n) && (n & (n - 1)) == 0) {
            if (n == 0) {
                assert(false && "Division by zero in LongInt::operator/=");
            }
            if (n != 1) {
                operator>>=(uint32_t(math_utils::count_trailing_zeros(n)));
            }
            return *this;
        }
#endif
        DivMod(n);
        return *this;
    }

    constexpr uint32_t DivMod(uint32_t n) noexcept {
        uint64_t carry = 0;
        uint32_t* nums_iter_end = nums_ - 1;
        uint32_t* nums_iter = nums_iter_end + USize();
        for (; nums_iter != nums_iter_end; --nums_iter) {
            uint64_t cur = (carry << 32) | uint64_t(*nums_iter);
            uint64_t q = cur / n;
            uint64_t r = cur - q * n;
            *nums_iter = uint32_t(q);
            carry = r;
        }

        PopLeadingZeros();
        return uint32_t(carry);
    }

    constexpr_on_cfunc_call LongInt& operator>>=(uint32_t shift) noexcept {
        size_t size = USize();
        uint32_t uints_move = shift >> 5;
        if (uints_move >= size) {
            size_ = 0;
            return *this;
        }

        if (uints_move != 0) {
            size -= uints_move;
            size_ = size_ >= 0 ? int32_t(size) : -int32_t(size);
#if defined(__cpp_lib_is_constant_evaluated) && \
    __cpp_lib_is_constant_evaluated >= 201811L
            if (std::is_constant_evaluated()) {
                uint32_t* dst = nums_;
                const uint32_t* src = nums_ + uints_move;
                for (size_t limit = size; limit-- != 0; ++dst, ++src) {
                    *dst = *src;
                }
            } else
#endif
                memmove(nums_, nums_ + uints_move, size * sizeof(uint32_t));
        }

        shift &= 0b11111;
        uint32_t* nums_iter = nums_;
        uint32_t* nums_iter_end = nums_iter + ssize_t(size) - 1;
        for (; nums_iter != nums_iter_end; ++nums_iter) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            // *nums_iter = uint32_t(*reinterpret_cast<uint64_t*>(nums_iter) >>
            // shift);
            *nums_iter = uint32_t(
                ((*nums_iter) | (uint64_t(*(nums_iter + 1)) << 32)) >> shift);
#else
            uint32_t hi = *(nums_iter + 1);
            uint32_t low = *nums_iter;
            uint32_t mask = (1 << shift) - 1;
            *nums_iter = ((hi & mask) << (32 - shift)) | (low >> shift);
#endif
        }

        *nums_iter_end >>= shift;
        if (*nums_iter_end == 0) {
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

    constexpr operator bool() noexcept { return size_ != 0; }

    constexpr bool IsZero() const noexcept { return size_ == 0; }

    constexpr bool empty() const noexcept { return size_ == 0; }

    constexpr int32_t Size() const noexcept { return size_; }

    constexpr size_t USize() const noexcept {
        /**
         * cast to uint32_t to force zero extension when casting to size_t
         * std::abs is not used in order to make method constexpr
         */
        return size_t(size_ >= 0 ? uint32_t(size_) : -uint32_t(size_));
    }

    constexpr int32_t GetSign() const noexcept {
        return int32_t(size_ > 0) - int32_t(size_ < 0);
    }

    constexpr void ChangeSign() noexcept { size_ = -size_; }

    void SetString(std::string_view s) {
        const unsigned char* str_iter =
            reinterpret_cast<const unsigned char*>(s.begin());
        const unsigned char* str_end =
            reinterpret_cast<const unsigned char*>(s.end());
        int32_t sign = 1;
        while (str_iter != str_end && !math_utils::is_digit(*str_iter)) {
            sign = 1 - int32_t(uint32_t(*str_iter == '-') << 1);
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
            if (sign < 0) {
                size_ = -size_;
            }
            return;
        }

        const size_t str_conv_digits_size =
            (digits_count + kStrConvBaseDigits - 1) / kStrConvBaseDigits;
        const size_t aligned_str_conv_digits_size =
            math_utils::nearest_2_pow_greater_equal(str_conv_digits_size);
        uint32_t* str_conv_digits;
        ReserveWithoutCopy(uint32_t(aligned_str_conv_digits_size));
        str_conv_digits = nums_;

        {
            uint32_t* str_conv_digits_iter =
                str_conv_digits + str_conv_digits_size;
            memset(str_conv_digits_iter, 0,
                   (aligned_str_conv_digits_size - str_conv_digits_size) *
                       sizeof(uint32_t));
            if (size_t offset = digits_count % kStrConvBaseDigits) {
                uint32_t current = 0;
                do {
                    current = current * 10 + uint32_t(*str_iter) - '0';
                    str_iter++;
                } while (--offset);
                *--str_conv_digits_iter = current;
            }

            do {
                static_assert(kStrConvBaseDigits == 9);
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
        uint32_t* mult_add_buffer = static_cast<uint32_t*>(operator new(
            aligned_str_conv_digits_size * sizeof(uint32_t) +
            max_fft_poly_length * sizeof(complex)));
        complex* fft_poly_buffer = reinterpret_cast<complex*>(
            mult_add_buffer + aligned_str_conv_digits_size);
        EnsureDecBasePowsCapacity(
            math_utils::log2_floor(uint64_t(aligned_str_conv_digits_size)));
        const LongInt* conv_dec_base_pows_iter = conv_dec_base_pows.data();
        for (size_t half_len = 1; half_len != aligned_str_conv_digits_size;
             half_len *= 2, ++conv_dec_base_pows_iter) {
            for (size_t pos = 0; pos != aligned_str_conv_digits_size;
                 pos += 2 * half_len) {
                ConvertDecBaseMultAdd(str_conv_digits + pos, half_len,
                                      conv_dec_base_pows_iter, mult_add_buffer,
                                      fft_poly_buffer);
            }
        }
        operator delete(mult_add_buffer);

        size_t usize = aligned_str_conv_digits_size;
        while (usize != 0 && nums_[usize - 1] == 0) {
            usize--;
        }
        this->size_ = sign * int32_t(usize);
    }

    inline std::string ToString() const {
        std::string s;
        ToString(s);
        return s;
    }

    void ToString(std::string& ans) const {
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

        size_t n = math_utils::nearest_2_pow_greater_equal(usize);
        EnsureBinBasePowsCapacity(math_utils::log2_floor(n));
        uint32_t* knums = static_cast<uint32_t*>(
            LongIntAllocator::Allocate(n * sizeof(uint32_t)));
        memcpy(knums, nums_, usize * sizeof(uint32_t));
        memset(knums + usize, 0, (n - usize) * sizeof(uint32_t));
        auto result = ConvertBinBase(knums, n);
        LongIntAllocator::Deallocate(knums);
        assert(result.size_ >= 3);
        size_t full_blocks = result.size_ - 1;
        uint32_t last_a_i = result.digits_[full_blocks];
        size_t string_size = full_blocks * kStrConvBaseDigits +
                             math_utils::base_10_digits(last_a_i);
        ans.resize((size_ < 0) + string_size);
        char* ptr = ans.data() + string_size;
        for (size_t i = 0; i < full_blocks; i++) {
            uint32_t a_i = result.digits_[i];
            for (size_t j = kStrConvBaseDigits; j != 0; j--) {
                *--ptr = char('0' + a_i % 10);
                a_i /= 10;
            }
        }

        do {
            *--ptr = char('0' + last_a_i % 10);
            last_a_i /= 10;
        } while (last_a_i);
    }

    inline friend std::ostream& operator<<(std::ostream& out,
                                           const LongInt& n) {
        std::string buffer;
        n.ToString(buffer);
        return out << buffer;
    }

    inline friend std::istream& operator>>(std::istream& in, LongInt& n) {
        std::string s;
        in >> s;
        n.SetString(s);
        return in;
    }

    inline void Reserve(uint32_t capacity) {
        if (capacity > capacity_) {
            uint32_t* new_nums = static_cast<uint32_t*>(
                LongIntAllocator::Allocate(capacity * sizeof(uint32_t)));
            if (nums_) {
                memcpy(new_nums, nums_, USize() * sizeof(uint32_t));
                LongIntAllocator::Deallocate(nums_);
            }
            nums_ = new_nums;
            capacity_ = capacity;
        }
    }

    inline void ReserveWithoutCopy(uint32_t capacity) {
        if (capacity > capacity_) {
            LongIntAllocator::Deallocate(nums_);
            nums_ = static_cast<uint32_t*>(
                LongIntAllocator::Allocate(capacity * sizeof(uint32_t)));
            capacity_ = capacity;
        }
        size_ = 0;
    }

    inline ~LongInt() { LongIntAllocator::Deallocate(nums_); }

    struct Decimal {
        static inline constexpr uint32_t kDecimalBase = kStrConvBase;
        static inline constexpr uint32_t kFftDecimalBase = 1'000;

        uint32_t* digits_;
        size_t size_;

        constexpr Decimal() noexcept : digits_(nullptr), size_(0) {}

        inline explicit Decimal(uint32_t n) {
            digits_ = static_cast<uint32_t*>(
                LongIntAllocator::Allocate(2 * sizeof(uint32_t)));

            uint32_t low = n % kDecimalBase;
            digits_[0] = low;
            uint32_t hi = n / kDecimalBase;
            digits_[1] = hi;
            size_ = hi != 0 ? 2 : low != 0;
        }

        inline explicit Decimal(uint64_t n) {
            this->digits_ = static_cast<uint32_t*>(
                LongIntAllocator::Allocate(3 * sizeof(uint32_t)));

            uint32_t low = uint32_t(n % kDecimalBase);
            uint64_t t = n / kDecimalBase;
            uint32_t mid = uint32_t(t % kDecimalBase);
            uint32_t hi = uint32_t(t / kDecimalBase);
            this->digits_[0] = low;
            this->digits_[1] = mid;
            this->digits_[2] = hi;
            this->size_ = hi != 0 ? 3 : (mid != 0 ? 2 : low != 0);
        }

        inline Decimal(const Decimal& other) : digits_(nullptr), size_(0) {
            if (other.size_) {
                this->digits_ = static_cast<uint32_t*>(
                    LongIntAllocator::Allocate(other.size_ * sizeof(uint32_t)));

                this->size_ = other.size_;
                memcpy(this->digits_, other.digits_,
                       other.size_ * sizeof(uint32_t));
            }
        }

        inline Decimal& operator=(const Decimal& other) {
            LongIntAllocator::Deallocate(this->digits_);
            this->digits_ = nullptr;
            this->size_ = 0;
            if (other.size_) {
                this->digits_ = static_cast<uint32_t*>(
                    LongIntAllocator::Allocate(other.size_ * sizeof(uint32_t)));

                size_ = other.size_;
                memcpy(this->digits_, other.digits_,
                       other.size_ * sizeof(uint32_t));
            }
            return *this;
        }

        Decimal(Decimal&& other) noexcept : size_(other.size_) {
            digits_ = other.digits_;
            other.digits_ = nullptr;
            other.size_ = 0;
        }

        Decimal& operator=(Decimal&& other) noexcept {
            LongIntAllocator::Deallocate(this->digits_);
            this->digits_ = other.digits_;
            this->size_ = other.size_;
            other.digits_ = nullptr;
            other.size_ = 0;
            return *this;
        }

        inline Decimal& operator=(uint32_t n) {
            if (this->digits_ == nullptr) {
                this->digits_ = static_cast<uint32_t*>(
                    LongIntAllocator::Allocate(2 * sizeof(uint32_t)));
            }

            uint32_t low = n % kDecimalBase;
            this->digits_[0] = low;
            uint32_t hi = n / kDecimalBase;
            this->digits_[1] = hi;
            this->size_ = hi != 0 ? 2 : low != 0;
            return *this;
        }

        inline Decimal& operator=(uint64_t n) {
            if (this->digits_ == nullptr) {
                this->digits_ = static_cast<uint32_t*>(
                    LongIntAllocator::Allocate(3 * sizeof(uint32_t)));
            }

            uint32_t low = uint32_t(n % kDecimalBase);
            this->digits_[0] = low;
            uint32_t t = uint32_t(n / kDecimalBase);
            uint32_t mid = t % kDecimalBase;
            uint32_t hi = t / kDecimalBase;
            this->digits_[1] = mid;
            this->digits_[2] = hi;
            this->size_ = hi != 0 ? 3 : (mid != 0 ? 2 : low != 0);
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
                // let compiler decide whether it is faster then 3 xors or not
                size_t tmp = m;
                m = k;
                k = tmp;
            }

            if (unlikely(m == 0)) {
                size_ = 0;
                return *this;
            }

            size_t new_size = m + k;
            if (m <= 16 || m * k <= 1024) {
                uint32_t* ans = static_cast<uint32_t*>(
                    LongIntAllocator::Allocate(new_size * sizeof(uint32_t)));
                memset(ans, 0, new_size * sizeof(uint32_t));
                for (size_t j = 0; j < m; j++) {
                    uint64_t b_j = m_ptr[j];
                    uint64_t carry = 0;
                    for (size_t i = 0; i < k; i++) {
                        uint64_t a_i = k_ptr[i];
                        uint64_t res = a_i * b_j + uint64_t(ans[j + i]) + carry;
                        ans[j + i] = uint32_t(res % kDecimalBase);
                        carry = res / kDecimalBase;
                    }
                    ans[j + k] = uint32_t(carry % kDecimalBase);
                }

                LongIntAllocator::Deallocate(this->digits_);
                this->digits_ = ans;
            } else {
                size_t n =
                    math_utils::nearest_2_pow_greater_equal(3 * new_size);
                static_assert(kFftDecimalBase * kFftDecimalBase *
                                  kFftDecimalBase ==
                              kDecimalBase);
                // Allocate n for the first polynomial and n for
                // the second one
                complex* p1 = static_cast<complex*>(operator new(
                    (n + n) * sizeof(complex)));
                complex* p = p1;
                for (size_t i = 0; i < m; i++) {
                    uint32_t m_value = m_ptr[i];
                    uint32_t k_value = k_ptr[i];
                    uint32_t r1 = m_value % kFftDecimalBase;
                    m_value /= kFftDecimalBase;
                    uint32_t r2 = k_value % kFftDecimalBase;
                    k_value /= kFftDecimalBase;
                    *p = complex(r1, r2);
                    p++;
                    r1 = m_value % kFftDecimalBase;
                    m_value /= kFftDecimalBase;
                    r2 = k_value % kFftDecimalBase;
                    k_value /= kFftDecimalBase;
                    *p = complex(r1, r2);
                    p++;
                    *p = complex(m_value, k_value);
                    p++;
                }
                for (size_t i = m; i < k; i++) {
                    uint32_t k_value = k_ptr[i];
                    uint32_t r2 = k_value % kFftDecimalBase;
                    k_value /= kFftDecimalBase;
                    *p = complex(0, r2);
                    p++;
                    r2 = k_value % kFftDecimalBase;
                    k_value /= kFftDecimalBase;
                    *p = complex(0, r2);
                    p++;
                    *p = complex(0, k_value);
                    p++;
                }

                memset(static_cast<void*>(p), 0, (n - 3 * k) * sizeof(complex));

                if (new_size > size_) {
                    LongIntAllocator::Deallocate(this->digits_);
                    this->digits_ =
                        static_cast<uint32_t*>(LongIntAllocator::Allocate(
                            new_size * sizeof(uint32_t)));
                }

                complex* p2 = p1 + n;
                ForwardBackwardFFT(p1, p2, n);

                uint64_t carry = 0;
                uint32_t* ans_p = this->digits_;
                const uint32_t* ans_p_end = ans_p + new_size;
                do {
                    uint64_t res = carry;
                    res += uint64_t((*p2).real() + 0.5);
                    p2++;
                    res += uint64_t((*p2).real() + 0.5) * kFftDecimalBase;
                    p2++;
                    res += uint64_t((*p2).real() + 0.5) *
                           (kFftDecimalBase * kFftDecimalBase);
                    p2++;
                    *ans_p = uint32_t(res % kDecimalBase);
                    carry = res / kDecimalBase;
                    ans_p++;
                } while (ans_p != ans_p_end);
                operator delete(p1);
            }

            size_ = new_size;
            this->PopLeadingZeros();
            return *this;
        }

        Decimal& operator+=(const Decimal& other) {
            uint64_t carry = 0;
            const size_t m = std::min(this->size_, other.size_);
            uint32_t* p = this->digits_;
            for (size_t i = 0; i < m; i++) {
                uint64_t res =
                    uint64_t(p[i]) + uint64_t(other.digits_[i]) + carry;
                p[i] = uint32_t(res % kDecimalBase);
                carry = res / kDecimalBase;
            }

            if (size_ < other.size_) {
                uint32_t* new_digits = static_cast<uint32_t*>(
                    LongIntAllocator::Allocate(other.size_ * sizeof(uint32_t)));
                memcpy(new_digits, this->digits_, size_ * sizeof(uint32_t));
                memcpy(new_digits + size_, other.digits_ + size_,
                       (other.size_ - size_) * sizeof(uint32_t));
                LongIntAllocator::Deallocate(this->digits_);
                this->digits_ = new_digits;
                size_ = other.size_;
            }

            p = this->digits_;
            const size_t this_size = size_;
            for (size_t i = m; carry != 0 && i < this_size; i++) {
                uint64_t res = uint64_t(p[i]) + carry;
                p[i] = uint32_t(res % kDecimalBase);
                carry = res / kDecimalBase;
            }

            if (carry == 0) {
                PopLeadingZeros();
            } else {
                uint32_t* new_digits =
                    static_cast<uint32_t*>(LongIntAllocator::Allocate(
                        (this_size + 1 + (this_size == 0)) * sizeof(uint32_t)));
                memcpy(new_digits, this->digits_, this_size * sizeof(uint32_t));
                new_digits[this_size] = uint32_t(carry);
                LongIntAllocator::Deallocate(this->digits_);
                this->digits_ = new_digits;
                size_ = this_size + 1;
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
            size_t prod_size = digits_size + digits_size;
            if (prod_size <= 16) {
                uint32_t* ans = static_cast<uint32_t*>(
                    LongIntAllocator::Allocate(prod_size * sizeof(uint32_t)));
                memset(ans, 0, prod_size * sizeof(uint32_t));
                for (size_t j = 0; j < digits_size; j++) {
                    uint64_t b_j = digits_ptr[j];
                    uint64_t carry = 0;
                    for (size_t i = 0; i < digits_size; i++) {
                        uint64_t a_i = digits_ptr[i];
                        uint64_t res = a_i * b_j + uint64_t(ans[j + i]) + carry;
                        ans[j + i] = uint32_t(res % kDecimalBase);
                        carry = res / kDecimalBase;
                    }
                    ans[j + digits_size] = uint32_t(carry % kDecimalBase);
                }

                LongIntAllocator::Deallocate(other.digits_);
                other.digits_ = ans;
            } else {
                size_t n =
                    math_utils::nearest_2_pow_greater_equal(3 * prod_size);
                static_assert(kFftDecimalBase * kFftDecimalBase *
                                  kFftDecimalBase ==
                              kDecimalBase);
                // Allocate n for the first polynomial and n for
                // the second one
                complex* p1 = static_cast<complex*>(operator new(
                    (n + n) * sizeof(complex)));
                complex* p = p1;
                for (size_t i = 0; i < digits_size; i++) {
                    uint32_t value = digits_ptr[i];
                    uint32_t r1 = value % kFftDecimalBase;
                    value /= kFftDecimalBase;
                    *p = complex(r1, r1);
                    p++;
                    r1 = value % kFftDecimalBase;
                    value /= kFftDecimalBase;
                    *p = complex(r1, r1);
                    p++;
                    *p = complex(value, value);
                    p++;
                }

                memset(static_cast<void*>(p), 0,
                       (n - 3 * digits_size) * sizeof(complex));

                if (prod_size > other.size_) {
                    LongIntAllocator::Deallocate(other.digits_);
                    other.digits_ =
                        static_cast<uint32_t*>(LongIntAllocator::Allocate(
                            prod_size * sizeof(uint32_t)));
                }

                complex* p2 = p1 + n;
                ForwardBackwardFFT(p1, p2, n);

                uint64_t carry = 0;
                uint32_t* ans_p = other.digits_;
                const uint32_t* ans_p_end = ans_p + prod_size;
                do {
                    uint64_t res = carry;
                    res += uint64_t((*p2).real() + 0.5);
                    p2++;
                    res += uint64_t((*p2).real() + 0.5) * kFftDecimalBase;
                    p2++;
                    res += uint64_t((*p2).real() + 0.5) *
                           (kFftDecimalBase * kFftDecimalBase);
                    p2++;
                    *ans_p = uint32_t(res % kDecimalBase);
                    carry = res / kDecimalBase;
                    ans_p++;
                } while (ans_p != ans_p_end);
                operator delete(p1);
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
                    return uint64_t(digits_[1]) * kDecimalBase + digits_[0] ==
                           n;
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
                    return uint64_t(digits_[1]) * kDecimalBase + digits_[0] ==
                           n;
                case 3: {
                    constexpr uint64_t kDecimalBase2 =
                        uint64_t(kDecimalBase) * kDecimalBase;
                    uint64_t hi = digits_[2];
                    if (hi > 18) {
                        return false;
                    }

                    uint64_t low_mid_m =
                        uint64_t(digits_[1]) * kDecimalBase + digits_[0];
                    if (hi == 18) {
                        return n >= 18 * kDecimalBase2 &&
                               n - 18 * kDecimalBase2 == low_mid_m;
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

        constexpr_on_cfunc_call bool operator==(
            const Decimal& other) const noexcept {
            return size_ == other.size_ &&
                   memcmp(digits_, other.digits_, size_ * sizeof(uint32_t)) ==
                       0;
        }

        constexpr_on_cfunc_call bool operator!=(
            const Decimal& other) const noexcept {
            return !(*this == other);
        }

        constexpr void PopLeadingZeros() noexcept {
            size_t size = size_;
            while (size != 0 && digits_[size - 1] == 0) {
                size--;
            }

            size_ = size;
        }

        inline ~Decimal() { LongIntAllocator::Deallocate(digits_); }
    };

   protected:
    static std::vector<Decimal> conv_bin_base_pows;

    static std::vector<LongInt> conv_dec_base_pows;

    __attribute__((destructor(102))) static void
    LongIntDataDeallocate() noexcept {
        LongInt::conv_dec_base_pows.clear();
        LongInt::conv_bin_base_pows.clear();
    }

    static inline void EnsureDecBasePowsCapacity(size_t pows_size) {
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

    static inline void ConvertDecBaseMultAdd(uint32_t conv_digits[],
                                             size_t half_len,
                                             const LongInt* conv_base_pow,
                                             uint32_t mult_add_buffer[],
                                             complex fft_poly_buffer[]) {
        const uint32_t* num_hi = conv_digits + half_len;
        size_t m = uint32_t(conv_base_pow->size_);
        assert(m != 0 && m <= half_len);
        const uint32_t* m_ptr = conv_base_pow->nums_;
        size_t prod_size = m + half_len;
        memset(mult_add_buffer, 0, (half_len + half_len) * sizeof(uint32_t));
        if (half_len <= 32) {
            for (size_t j = 0; j < m; j++) {
                uint64_t b_j = m_ptr[j];
                uint64_t carry = 0;
                for (size_t i = 0; i < half_len; i++) {
                    uint64_t a_i = num_hi[i];
                    uint64_t res =
                        a_i * b_j + uint64_t(mult_add_buffer[j + i]) + carry;
                    mult_add_buffer[j + i] = uint32_t(res);
                    carry = res >> kNumsBits;
                }

                mult_add_buffer[j + half_len] = uint32_t(carry);
            }
        } else {
            size_t n = 2 * math_utils::nearest_2_pow_greater_equal(prod_size);
            const bool high_precision = n > kFFTPrecisionBorder;
            n <<= high_precision;
            complex* p1 = fft_poly_buffer;
            complex* p = p1;
            if (likely(!high_precision)) {
                for (size_t i = 0; i < m; i++) {
                    uint32_t m_value = m_ptr[i];
                    uint32_t k_value = num_hi[i];
                    *p = complex(m_value & 0xFFFF, k_value & 0xFFFF);
                    p++;
                    *p = complex(m_value >> 16, k_value >> 16);
                    p++;
                }
                for (size_t i = m; i < half_len; i++) {
                    uint32_t k_value = num_hi[i];
                    *p = complex(0, k_value & 0xFFFF);
                    p++;
                    *p = complex(0, k_value >> 16);
                    p++;
                }
            } else {
                for (size_t i = 0; i < m; i++) {
                    uint32_t m_value = m_ptr[i];
                    uint32_t k_value = num_hi[i];
                    *p = complex(uint8_t(m_value), uint8_t(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = complex(uint8_t(m_value), uint8_t(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = complex(uint8_t(m_value), uint8_t(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = complex(m_value, k_value);
                    p++;
                }
                for (size_t i = m; i < half_len; i++) {
                    uint32_t k_value = num_hi[i];
                    *p = complex(0, uint8_t(k_value));
                    p++;
                    k_value >>= 8;
                    *p = complex(0, uint8_t(k_value));
                    p++;
                    k_value >>= 8;
                    *p = complex(0, uint8_t(k_value));
                    p++;
                    k_value >>= 8;
                    *p = complex(0, k_value);
                    p++;
                }
            }
            memset(static_cast<void*>(p), 0,
                   (n - ((2 * half_len) << high_precision)) * sizeof(complex));

            complex* p2 = p1 + n;
            ForwardBackwardFFT(p1, p2, n);
            uint64_t carry = 0;
            uint32_t* ans_p = mult_add_buffer;
            const uint32_t* ans_p_end = ans_p + prod_size;
            if (likely(!high_precision)) {
                do {
                    uint64_t res = carry;
                    res += uint64_t((*p2).real() + 0.5);
                    p2++;
                    res += uint64_t((*p2).real() + 0.5) << 16;
                    p2++;
                    *ans_p = uint32_t(res);
                    carry = res >> kNumsBits;
                    ans_p++;
                } while (ans_p != ans_p_end);
            } else {
                do {
                    uint64_t res = carry;
                    res += uint64_t((*p2).real() + 0.5);
                    p2++;
                    res += uint64_t((*p2).real() + 0.5) << 8;
                    p2++;
                    res += uint64_t((*p2).real() + 0.5) << 16;
                    p2++;
                    res += uint64_t((*p2).real() + 0.5) << 24;
                    p2++;
                    *ans_p = uint32_t(res);
                    carry = res >> kNumsBits;
                    ans_p++;
                } while (ans_p != ans_p_end);
            }
            assert(carry == 0);
        }

        // Now mult_add_buffer == num_hi * CONV_BASE^half_len
        uint64_t carry = 0;
        for (size_t i = half_len; i != 0;
             i--, conv_digits++, mult_add_buffer++) {
            uint64_t res =
                uint64_t(*conv_digits) + uint64_t(*mult_add_buffer) + carry;
            *conv_digits = uint32_t(res);
            carry = res >> kNumsBits;
        }
        for (size_t i = half_len; i != 0;
             i--, conv_digits++, mult_add_buffer++) {
            uint64_t res = uint64_t(*mult_add_buffer) + carry;
            *conv_digits = uint32_t(res);
            carry = res >> kNumsBits;
        }
        assert(carry == 0);
    }

    static Decimal ConvertBinBase(const uint32_t* nums, size_t size) {
        assert((size != 0) & ((size & (size - 1)) == 0));
        switch (size) {
            case 0:
            case 1:
                return Decimal(nums[0]);
            case 2:
                return Decimal(uint64_t(nums[1]) * kNumsBase | nums[0]);
        }

        Decimal low_dec = ConvertBinBase(nums, size / 2);
        Decimal high_dec = ConvertBinBase(nums + size / 2, size / 2);

        high_dec *= conv_bin_base_pows.at(math_utils::log2_floor(size) - 1);
        high_dec += low_dec;
        return high_dec;
    }

    static inline void EnsureBinBasePowsCapacity(size_t pows_size) {
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

    __attribute__((__noinline__)) void GrowCapacity() {
        uint32_t new_capacity = (capacity_ * 2) | (capacity_ == 0);
        uint32_t* new_nums = static_cast<uint32_t*>(
            LongIntAllocator::Allocate(new_capacity * sizeof(uint32_t)));
        if (nums_) {
            memcpy(new_nums, nums_, USize() * sizeof(uint32_t));
            LongIntAllocator::Deallocate(nums_);
        }
        nums_ = new_nums;
        capacity_ = new_capacity;
    }

    __attribute__((__noinline__)) size_t GrowSizeByOne() {
        size_t usize = USize();
        if (unlikely(usize == capacity_)) {
            GrowCapacity();
        }

        size_ += GetSign();
        return usize + 1;
    }

    inline size_t SetSizeAtLeast(size_t new_size) {
        size_t cur_size = USize();
        if (new_size <= cur_size) {
            return cur_size;
        }

        if (new_size > capacity_) {
            uint32_t* new_nums = static_cast<uint32_t*>(
                LongIntAllocator::Allocate(new_size * sizeof(uint32_t)));
            if (nums_) {
                memcpy(new_nums, nums_, cur_size * sizeof(uint32_t));
                LongIntAllocator::Deallocate(nums_);
            }
            nums_ = new_nums;
            capacity_ = uint32_t(new_size);
        }

        memset(&nums_[cur_size], 0, (new_size - cur_size) * sizeof(uint32_t));
        size_ = size_ >= 0 ? int32_t(new_size) : -int32_t(new_size);
        return new_size;
    }

    inline void EnsureDefaultCapacityOpEqCall() {
        if (capacity_ < kDefaultLINumsCapacity) {
            LongIntAllocator::Deallocate(nums_);
            nums_ = static_cast<uint32_t*>(LongIntAllocator::Allocate(
                kDefaultLINumsCapacity * sizeof(uint32_t)));
            capacity_ = kDefaultLINumsCapacity;
        }
    }

    inline void AllocateDefaultCapacityCtorCall() {
        nums_ = static_cast<uint32_t*>(LongIntAllocator::Allocate(
            kDefaultLINumsCapacity * sizeof(uint32_t)));
        capacity_ = kDefaultLINumsCapacity;
    }

    static constexpr uint64_t LongIntAdd(uint32_t nums1[],
                                         const uint32_t nums2[], size_t usize1,
                                         size_t usize2) noexcept {
        uint64_t carry = 0;
        const uint32_t* nums1_end = nums1 + usize1;
        for (const uint32_t* nums2_end = nums2 + usize2; nums2 != nums2_end;
             ++nums2, ++nums1) {
            uint64_t res = uint64_t(*nums1) + uint64_t(*nums2) + carry;
            *nums1 = uint32_t(res);
            carry = res >> kNumsBits;
        }

        for (; carry != 0; ++nums1) {
            if (nums1 == nums1_end) {
                return carry;
            }

            uint64_t res = uint64_t(*nums1) + carry;
            *nums1 = uint32_t(res);
            carry = res >> kNumsBits;
        }

        return 0;
    }

    void NonZeroSizeAddUInt(uint32_t n) {
        uint32_t* it = nums_;
        const uint32_t* end = nums_ + USize();
        uint64_t carry = n;
        do {
            uint64_t res = uint64_t(*it) + carry;
            carry = res >> kNumsBits;
            *it = uint32_t(res);
            if (carry == 0) {
                goto add_u_end;
            }
            ++it;
        } while (it != end);

        if (carry != 0) {
            size_t usize = USize();
            if (unlikely(usize == capacity_)) {
                GrowCapacity();
            }

            assert(usize < capacity_);
            nums_[usize] = uint32_t(carry);
            size_ += GetSign();
        }

    add_u_end:
        return;
    }

    constexpr void NonZeroSizeSubUInt(uint32_t n) {
        size_t usize = USize();
        uint32_t* nums_iter = nums_;
        uint32_t low_num = nums_iter[0];
        if (usize != 1) {
            uint32_t res = low_num - n;
            bool carry = res > low_num;
            nums_iter[0] = res;
            if (carry) {
                do {
                    ++nums_iter;
                } while (nums_iter[0] == 0);
                if (--nums_iter[0] == 0) {
                    size_ = GetSign();
                }
            }
        } else if (n <= low_num) {
            nums_iter[0] = low_num - n;
        } else {
            nums_iter[0] = n - low_num;
            ChangeSign();
        }
    }
};

std::vector<LongInt> LongInt::conv_dec_base_pows = {
    LongInt(LongInt::kStrConvBase)};

std::vector<LongInt::Decimal> LongInt::conv_bin_base_pows = {
    LongInt::Decimal(LongInt::kNumsBase)};

#if defined(constexpr_on_cfunc_call)
#undef constexpr_on_cfunc_call
#endif

namespace std {
#if __cplusplus >= 202002L
constexpr
#endif
    inline void
    swap(LongInt& n, LongInt& m) noexcept {
    n.Swap(m);
}

inline string to_string(const LongInt& n) { return n.ToString(); }
}  // namespace std

#include <chrono>

namespace long_int_tests {

void TestOperatorEqualsInt() {
    puts(__PRETTY_FUNCTION__);
    LongInt n;

    constexpr int32_t K = 131072;
    for (int32_t i = -K; i < 0; i++) {
        n = i;
        assert(n.GetSign() == -1);
        assert(n.size_ == -1 && n.nums_[0] == uint32_t(-i));
    }
    n = 0;
    assert(n.GetSign() == 0);
    assert(n.size_ == 0);
    for (int32_t i = 1; i <= K; i++) {
        n = i;
        assert(n.GetSign() == 1);
        assert(n.size_ == 1 && n.nums_[0] == uint32_t(i));
    }

    n = 0u;
    assert(n.GetSign() == 0);
    assert(n.size_ == 0);
    for (uint32_t i = 1; i < 2 * K; i++) {
        n = i;
        assert(n.GetSign() == 1);
        assert(n.size_ == 1 && n.nums_[0] == i);
    }

    for (int64_t i = -K; i < 0; i++) {
        n = i;
        assert(n.GetSign() == -1);
        assert(n.size_ == -1 && n.nums_[0] == uint32_t(-i));
    }
    n = uint64_t(0);
    assert(n.GetSign() == 0);
    assert(n.size_ == 0);
    for (int64_t i = 1; i <= K; i++) {
        n = i;
        assert(n.GetSign() == 1);
        assert(n.size_ == 1 && n.nums_[0] == uint32_t(i));
    }

    n = uint64_t(0);
    assert(n.GetSign() == 0);
    assert(n.size_ == 0);
    for (uint64_t i = 1; i < 2 * K; i++) {
        n = i;
        assert(n.GetSign() == 1);
        assert(n.size_ == 1 && n.nums_[0] == i);
    }

    n = uint128_t(0);
    assert(n.GetSign() == 0);
    assert(n.size_ == 0);
    n = uint128_t(-1);
    assert(n.GetSign() == 1);
    assert(n.size_ == 4);
    assert(n.nums_[0] == uint32_t(-1) && n.nums_[1] == uint32_t(-1) &&
           n.nums_[2] == uint32_t(-1) && n.nums_[3] == uint32_t(-1));

    for (uint64_t i = uint64_t(-1) - K; i != 0; i++) {
        n = i;
        assert(n == i);
    }
    for (uint64_t i = uint64_t(-1) - K; i != 0; i++) {
        n = uint128_t(i);
        assert(n == i);
    }
    for (uint128_t i = uint128_t(-1) - K; i != 0; i++) {
        n = i;
        assert(n == i);
    }
}

void TestLongIntMult() {
    puts(__PRETTY_FUNCTION__);
    LongInt n1;
    LongInt n2;
    constexpr uint64_t K = 6000;
    for (uint32_t i = 1; i <= K; i++) {
        for (uint32_t j = 1; j <= K; j++) {
            n1 = i;
            n2 = j;
            assert((n1 < n2) == (i < j));
            n1 *= n2;
            assert(n1 == i * j);
        }
    }

    for (uint64_t i = uint64_t(-1) - K; i != 0; i++) {
        for (uint64_t j = uint64_t(-1) - K; j != 0; j++) {
            n1 = uint64_t(i);
            n2 = uint64_t(j);
            assert((n1 < n2) == (i < j));
            n1 *= n2;
            assert(n1 == (uint128_t(i) * uint128_t(j)));
        }
    }

    const char* test_cases[][3] = {
        {"0", "0", "0"},
        {"0", "1", "0"},
        {"1", "0", "0"},
        {"0", "99999999999999999999999999999999999999999999999999999999999",
         "0"},
        {"99999999999999999999999999999999999999999999999999999999999", "0",
         "0"},
        {"1", "1", "1"},
        {"2", "1", "2"},
        {"1", "2", "2"},
        {"10", "20", "200"},
        {"12", "20", "240"},
        {"1024", "4", "4096"},
        {"12000000", "20000000", "240000000000000"},
        {"13721838", "317826897", "4361169192676686"},
        {"317826897", "13721838", "4361169192676686"},
        {"131241234423234", "5984348957348", "785393344381744834046223432"},
        {"340282367000166625996085689103316680705",
         "340282367000166625996085689103316680705",
         "115792089291236088776703569810027986869841637790274196431140198001898"
         "919297025"},
        {"273467824687236487236847263874236482736428764827364813231889217390184"
         "829348917249234892378961402352793846723542349823849279432378472893534"
         "9239875",
         "234890218513205634872423783145093845309415709814503975130984759381491"
         "4534715981347589134057893417",
         "642349170971159888904111587665401864943296318873272674206402598998481"
         "144001829012658620088464132921787070037675404241160766315820847757326"
         "085007615660887268742042704088972688871700404868535028458255349812618"
         "7128453709828981195657834716402875"},
        {"952263744", "9359517973", "8912729627004270912"},
        {"4219788824", "2743656178", "11577649676822954672"},
        {"2968434375", "517784556", "1537009474874512500"},
        {"84505427146800725206214423388331447259",
         "279405665168189620898465228388855751530",
         "236112950822738450044078891891140511945155493452385217141226440086310"
         "03556270"},
        {"241624483789163450827236083147698994361",
         "176969948248112551859871363564234198810",
         "427602723916451661211499909053727503369996114756759726074868690213215"
         "42910410"},
        {"189745687871346578913675631984859831398756913846314756",
         "23423242342342342342342342345689457967408570698981759840534",
         "444445923042499901900288719235071035985372602038867414228977476967324"
         "0565480827228712761450490207639434131119704"},
        {"23423242342342342342342342345689457967408570698981759840534",
         "189745687871346578913675631984859831398756913846314756",
         "444445923042499901900288719235071035985372602038867414228977476967324"
         "0565480827228712761450490207639434131119704"},
        {"263784627842638746283742347238428746238753489753984753489753895734895"
         "374598347593874590834823942389472893472983472983472984372983742894",
         "217389254359307583476294856287563891627462895613761974651784651756317"
         "561375613856310601356801561875871568715681346134571638145631413008153"
         "301138138",
         "573439435581586836344503148729108267131512809685670894440858871665949"
         "350249919608663834189665543320561328456411904268336709732076203336012"
         "709472247344406733059180011504893572100921001789368036815755131341796"
         "530761123858780924440928033173601703530960115961336341530612366213698"
         "91372"},
        {"497901391203133639584365248205694454191787983065888808021086054694516"
         "231678400769672247210449029211500692922450556195262154964450750615790"
         "589514188335169335767186667337238748906460179043594327975307613550873"
         "497640298659014179164027572907511030105218252598212205594530882278634"
         "877454130119091726705601029352202171195390872242698817060048675435915"
         "135527964210732602071032081196558962307757791960427679399529069324160"
         "549893369013101650049983041135188532935882809944537521495309524288328"
         "855117560004063950897329061825463265670807769803264433240453165419466"
         "124032708997652022855881507054705048360737029012888066511549342858319"
         "594221414437025268041662318574212665165197120587703338083508692868300"
         "677764770579732322663775467094391395549962995658143805157903378864641"
         "845529422222679420854277324025081357759529782057143902526044318608364"
         "773982488660170606054513097933784945464089479906701766065102979875971"
         "016186695987772994526141888182685329927553791394052980374860153388855"
         "6248642308523858954129309895839621",
         "108970620860112407137081415322742626853869366153607749269340200211370"
         "607783690889706401071971517605562911982171828098546334623182361783101"
         "527622674806797503240534309482808265776920525225913147483110997329125"
         "774929345411974578604409416298333424465618343704848658619059126047806"
         "968312617008037576720611676699711149503491022606985517565868797501921"
         "068402587653953249565108588704327397129174524695783763087364999447101"
         "233545982819083696205282296476137023754198789925143561112064292009350"
         "446578133843184706267448822832804838688761964596529899405643175114579"
         "837421411209617498646939693870084669070186947256565085434101186597907"
         "574721562115573828577120603334862160941496987738531194216605182014973"
         "559629133888929125188010648321192064868491615965928704952858784948813"
         "028262472375484232230838139028017592044534551940157817797571827142402"
         "193452406502015912170734287622055641671015226994756461398538648113704"
         "080006243527381200070072950459571659161258052892532223431066903464363"
         "6549786097960658822655773576934232",
         "542566237265191827529937514862954000646132192075670117028672556256875"
         "922981088057291176984953735545161000556996022800239121508505284757162"
         "268909483205498527914555274916168104686152671316244053413629649078592"
         "844221574454571143934298344272998274912039073349090700964702105431117"
         "052463313257026624080682902654613968760291690022965464553805515048545"
         "665814919756077899170782027815231369213297957660369219056733043223216"
         "650840611758386426651922228988776810169486935905786827971524608998934"
         "425480935592871847645329482203292850871800388282012154597553095349607"
         "061204124457639261614130984224601794030397716760678442804462930109239"
         "835200526589104558673734410275262228604661026783817421504731331566088"
         "159708333768459589296158398589645874726780789841965979149435313871784"
         "574855631200841764582159920684861974889545204330460968586302517203122"
         "757973861795664515823524825857550948072156021944827265006595069604640"
         "738609954368159474130242374101753694907633771930949978927315952622963"
         "200910399588310149423264017858244366418817160565161767936461007176434"
         "269452654579036827321940037520503715766834671004963497569735239076366"
         "322219524988756271804939185010822228086674414859379562218488021336743"
         "306279704576159643873067598070257884809075861268963824687803796573476"
         "656878994600856192230007507779855209591502919396895345613654967245608"
         "523561870429177696792471853823824979863659724994067540824448641360311"
         "020227810345973361697859149797441211765701947995273129491616164699781"
         "392550089934232138435257056976549602983289316350764782395107969550665"
         "644683069258104940473930427699017753413415103826696248529241526525773"
         "801855865477090197744456373594926654289133012873441919699920026875851"
         "916411337332255415651235834231397577746492925103509724219067053721769"
         "683258816781174862025112986918398848628743047602431938875162380059549"
         "757320510979602398036873972216468125267715591928946645842024992422856"
         "453314591343472715775836081790421367120745661633730990368643807993416"
         "7085071256457656974379118052493980516605329326027531228264736806072"},
        {"108970620860112407137081415322742626853869366153607749269340200211370"
         "607783690889706401071971517605562911982171828098546334623182361783101"
         "527622674806797503240534309482808265776920525225913147483110997329125"
         "774929345411974578604409416298333424465618343704848658619059126047806"
         "968312617008037576720611676699711149503491022606985517565868797501921"
         "068402587653953249565108588704327397129174524695783763087364999447101"
         "233545982819083696205282296476137023754198789925143561112064292009350"
         "446578133843184706267448822832804838688761964596529899405643175114579"
         "837421411209617498646939693870084669070186947256565085434101186597907"
         "574721562115573828577120603334862160941496987738531194216605182014973"
         "559629133888929125188010648321192064868491615965928704952858784948813"
         "028262472375484232230838139028017592044534551940157817797571827142402"
         "193452406502015912170734287622055641671015226994756461398538648113704"
         "080006243527381200070072950459571659161258052892532223431066903464363"
         "6549786097960658822655773576934232",
         "497901391203133639584365248205694454191787983065888808021086054694516"
         "231678400769672247210449029211500692922450556195262154964450750615790"
         "589514188335169335767186667337238748906460179043594327975307613550873"
         "497640298659014179164027572907511030105218252598212205594530882278634"
         "877454130119091726705601029352202171195390872242698817060048675435915"
         "135527964210732602071032081196558962307757791960427679399529069324160"
         "549893369013101650049983041135188532935882809944537521495309524288328"
         "855117560004063950897329061825463265670807769803264433240453165419466"
         "124032708997652022855881507054705048360737029012888066511549342858319"
         "594221414437025268041662318574212665165197120587703338083508692868300"
         "677764770579732322663775467094391395549962995658143805157903378864641"
         "845529422222679420854277324025081357759529782057143902526044318608364"
         "773982488660170606054513097933784945464089479906701766065102979875971"
         "016186695987772994526141888182685329927553791394052980374860153388855"
         "6248642308523858954129309895839621",
         "542566237265191827529937514862954000646132192075670117028672556256875"
         "922981088057291176984953735545161000556996022800239121508505284757162"
         "268909483205498527914555274916168104686152671316244053413629649078592"
         "844221574454571143934298344272998274912039073349090700964702105431117"
         "052463313257026624080682902654613968760291690022965464553805515048545"
         "665814919756077899170782027815231369213297957660369219056733043223216"
         "650840611758386426651922228988776810169486935905786827971524608998934"
         "425480935592871847645329482203292850871800388282012154597553095349607"
         "061204124457639261614130984224601794030397716760678442804462930109239"
         "835200526589104558673734410275262228604661026783817421504731331566088"
         "159708333768459589296158398589645874726780789841965979149435313871784"
         "574855631200841764582159920684861974889545204330460968586302517203122"
         "757973861795664515823524825857550948072156021944827265006595069604640"
         "738609954368159474130242374101753694907633771930949978927315952622963"
         "200910399588310149423264017858244366418817160565161767936461007176434"
         "269452654579036827321940037520503715766834671004963497569735239076366"
         "322219524988756271804939185010822228086674414859379562218488021336743"
         "306279704576159643873067598070257884809075861268963824687803796573476"
         "656878994600856192230007507779855209591502919396895345613654967245608"
         "523561870429177696792471853823824979863659724994067540824448641360311"
         "020227810345973361697859149797441211765701947995273129491616164699781"
         "392550089934232138435257056976549602983289316350764782395107969550665"
         "644683069258104940473930427699017753413415103826696248529241526525773"
         "801855865477090197744456373594926654289133012873441919699920026875851"
         "916411337332255415651235834231397577746492925103509724219067053721769"
         "683258816781174862025112986918398848628743047602431938875162380059549"
         "757320510979602398036873972216468125267715591928946645842024992422856"
         "453314591343472715775836081790421367120745661633730990368643807993416"
         "7085071256457656974379118052493980516605329326027531228264736806072"},
        {"398745897362857463485376489753649875364854634756893127583461753169476"
         "487536194573741578236174269138178478167188712671340573451475168475768"
         "13468751364056130456875613458136745",
         "348",
         "138763572282274397292911018434270156626969412895398808399044690102977"
         "817662595711662069226188645660086110402181672009626519561113358629567"
         "31087125474691533398992713483431587260"},
        {"348",
         "398745897362857463485376489753649875364854634756893127583461753169476"
         "487536194573741578236174269138178478167188712671340573451475168475768"
         "13468751364056130456875613458136745",
         "138763572282274397292911018434270156626969412895398808399044690102977"
         "817662595711662069226188645660086110402181672009626519561113358629567"
         "31087125474691533398992713483431587260"},
        {"521064401567922879406069432539095585333589848390805645835218385101837"
         "2555735221",
         "521064401567922879406069432539095585333589848390805645835218385101837"
         "2555735221",
         "271508110581337591266374006213668384075074032863180060266512914739142"
         "456172622787686672201433223907591836068343627329838282819700778580870"
         "36385802059859918841"},
        {"398745897362857463485376489753649875364854634756893127583461753169476"
         "487536194573741578236174269138178478167188712671340573451475168475768"
         "13468751364056130456875613458136745",
         "398745897362857463485376489753649875364854634756893127583461753169476"
         "487536194573741578236174269138178478167188712671340573451475168475768"
         "13468751364056130456875613458136745",
         "158998290663710458652907928386102189731894441557379140893911130204468"
         "729629180012082532017682144658368840447283002718093698410970322236959"
         "329738423137118403063843808542333663086419299601252781649552905024490"
         "422756290924999941542817862891684525955199054347881182431401273481363"
         "138211857637015928078713366709245314733667696374248162928864711919502"
         "5"},
        {"538714288092043980610037307269667416713870555024102721656419679779374"
         "923813527446566333299023119588532960327655160416672952650552265438069"
         "480844921279178590685230852805810043617124500309990368055562957",
         "538714288092043980610037307269667416713870555024102721656419679779374"
         "923813527446566333299023119588532960327655160416672952650552265438069"
         "480844921279178590685230852805810043617124500309990368055562957",
         "290213084194517758966516201644973136574935912430101593678728719207376"
         "239283543794185421070570977723421111953765127658270193387362711172917"
         "254842491555209808089298233500256901159707774581220239792305894702400"
         "718422627848242457771440142125225540017207158668021195884313003164633"
         "568953242740947472813766242520888691791570207128296293932060193742259"
         "027655131589237232055417058854401516386978283594190583849"},
        {"378473591711125662205318978307400782427783753923984686802121246042271"
         "316236774444461081305720113769234627500055999892631802251943506510613"
         "041576973905140659553198302268762932232635013075121844024010844209617"
         "001387445367992616947475847556277530823124716511381308443893601036689"
         "835540126414069661052087138994082036710162383386369932806029019860996"
         "789535859361485418449897433469146302768382820057178659536255110169359"
         "845196380565725061526581227067549629866724246482931073170051376781704"
         "197212768365253140559066042002101297055200613269143572019112557085563"
         "899878475510810554198342040963733144408262697470764570314944593526613"
         "934419263025337497357042571717671759023883586867451377168248733405883"
         "910296788303702062215452776471512256941837484725160214833729957202282"
         "648704994064553439348115504885342193852929906418451561210887575546337"
         "507615915632010857173564415901485308965735447198738488397867151647504"
         "526550040119974630239332996345427278593883623686974957277376278631354"
         "869639366838460468748101319039408427701123993442888448154403121426916"
         "704627733981704445112370876108453961872801955347846801218254087739489"
         "562170030806198132295724995482071830436648020515150300468035898691623"
         "269527628850137584131992179304108104221244082970339981314645",
         "242422360745395402404187893934203477400974449065647049988927198970319"
         "805753149705988973274875934705741319528222056518230978113788601185987"
         "597650863481917975147889480918426062465619374716968961931781307592900"
         "017970779362645595784950197534818861618814548119308868190069641017565"
         "582001182178863908348366042579709934563168161190371007834351658980901"
         "439537387725151710448936716057525234422021079168107409249681886185803"
         "656798184787755755302137461956528885902748067318538923549260818296551"
         "459793281096136811002727591059360446183594804912759682504651810130132"
         "241402424826964162913576719514934406842887250631366029807679608779995"
         "452685983217009380013356826166862603999974178446346927185519163124450"
         "042997576507464420018043214571816883931340327689828911413087854407416"
         "013902322113202969761256247555360669534677298033285373411229119624362"
         "114218631744679287995208711756999916330336680306667802886976862157913"
         "540009816489722867943929152313666052647540305002671654304297771170195"
         "703695712037696137032492879795700752067767022685233264574030743197818"
         "202098648173117308846148560570005975871944017411174794281674815109904"
         "233256841885601132453369966630233504941985437823039274173244307950067"
         "942092770927309358035074043249749436812689885822328999166497",
         "917504615823999964812490560695523410616319773621667959556544457902642"
         "228597203892116248150792838436259758633998890742122162662971349908155"
         "647613305941569852250475309540703819614371482131956000579690150103867"
         "335478224759580801158401367416866923388593255048676462217239894005505"
         "009293173974162215718916257677833210773428771087200121260806792118749"
         "401056565747653762978906896044361139468454441915586739520671907037366"
         "061178923181675896266118089995923451280607130137235218424615683169274"
         "638629109230340861539513364511680765354775168309144392850930437629909"
         "540394133815885217123604460615938103773946573454161215645170434248607"
         "885475252350857923430189977279480650126059654087213201630519852181484"
         "420704813284176064997930060799123094778253779356411014104775011619320"
         "890276285350085521042226184089357673933350600306949457583787046722645"
         "223193834579973303690497023431881676046233666783761209898616176106306"
         "729305364655669808769480217965819163342135473082505226302410863190468"
         "064569777465122355280724885358743713689472686982090153578230854097957"
         "195880970282340427948796059589784641830728955991808111545904442719713"
         "968653042471220082243271286370673789701162155117007120303537124233193"
         "275125118972895865679939043604689308535057054691688159958979192970492"
         "683643642024695575369794834267321472216711091276966830924761404911268"
         "187556249989351119873947551162147337246713550069804684687953776739927"
         "160721106545510088122128738416855312105051199067393501114750489112707"
         "303490618722690784630628219351880531189176857749421515617647625613905"
         "992645094759299281495020261468508383302524672961699120310427850850043"
         "608462954117991594441041525615782059524897217309809917382422228775783"
         "745334965416197148513545804017423917503503734483939356285877889547773"
         "524825669114358439184329239325471360281330935188244170013589217201763"
         "068697763253011631858147598565758360894999458452467192507663619987261"
         "612014164670771197354051123684835992187270895966932982720732420678204"
         "203847143455375866973089936570011460388228067165031434155375042497717"
         "403868873136480476131279697289876771903281296615905598206690477049313"
         "392033483312376581841307192363538822368201682123915484751895940728372"
         "363453862818847008212381931201750683359496089644866900948890514635439"
         "499038897053670375385502389269309919840640299039214251339389215673639"
         "932403778057364683711289609946099478058029490734553609251896107587556"
         "209723199428747459201558083692574616874349732490648481890083699349034"
         "17392669574477053593427918093472288422759299448565"},
    };
    for (const char* const(&test)[3] : test_cases) {
        n1.SetString(test[0]);
        n2.SetString(test[1]);
        n1 *= n2;
        n2.SetString(test[2]);
        if (n1 != n2) {
            std::cout << test[2] << ' ' << strlen(test[2]) << '\n'
                      << n1 << ' ' << n1.Size() << '\n';
            return;
        }
    }

    namespace chrono = std::chrono;

    constexpr size_t k = size_t(1e6);
    std::string s1(k, '9');
    n1.SetString(s1);
    {
        auto start = chrono::high_resolution_clock::now();
        n1 *= n1;
        auto end = chrono::high_resolution_clock::now();
        printf("Multiplied %zu digit numbers in %" PRIu64 " ms\n", k,
               uint64_t(chrono::duration_cast<chrono::milliseconds>(end - start)
                            .count()));
    }
    std::string ans(2 * k, '\0');
    memset(ans.data(), '9', (k - 1) * sizeof(char));
    ans[k - 1] = '8';
    memset(ans.data() + k, '0', (k - 1) * sizeof(char));
    ans[2 * k - 1] = '1';
    n2.SetString(ans);
    if (n1 != n2) {
        std::cout << "Long test failed\n";
    }
}

void TestLongIntSquare() {
    puts(__PRETTY_FUNCTION__);
    LongInt n;
    n.Reserve(4);
    constexpr uint64_t K = 8192;
    for (uint32_t i = 0; i <= K; i++) {
        n = i;
        n.Square();
        assert(n == i * i);
    }

    for (uint32_t i = uint32_t(-1) - K; i != 0; i++) {
        n = i;
        n.Square();
        assert(n == uint64_t(i) * i);
    }

    for (uint64_t i = uint64_t(-1) - K; i != 0; i++) {
        n = i;
        n.Square();
        assert(n == uint128_t(i) * i);
    }

    for (uint32_t p = 32; p <= 96; p += 32) {
        n = uint128_t(1) << p;
        n.Square();
        assert(n.size_ == int32_t((p + p) / 32 + 1));
        for (size_t i = 0; i < (p + p) / 32; i++) {
            assert(n[i] == 0);
        }
        assert(n[(p + p) / 32] == 1);
    }

    n.SetString(
        "5210644015679228794060694325390955853335898483908056458352183851018372"
        "555735221");
    n.Square();
    LongInt m(
        "2715081105813375912663740062136683840750740328631800602665129147391424"
        "5617262278768667220143322390759183606834362732983828281970077858087036"
        "385802059859918841");
    assert(n == m);

    n.SetString(
        "3987458973628574634853764897536498753648546347568931275834617531694764"
        "8753619457374157823617426913817847816718871267134057345147516847576813"
        "468751364056130456875613458136745");
    n.Square();
    m.SetString(
        "1589982906637104586529079283861021897318944415573791408939111302044687"
        "2962918001208253201768214465836884044728300271809369841097032223695932"
        "9738423137118403063843808542333663086419299601252781649552905024490422"
        "7562909249999415428178628916845259551990543478811824314012734813631382"
        "118576370159280787133667092453147336676963742481629288647119195025");
    assert(n == m);

    n.SetString(
        "5387142880920439806100373072696674167138705550241027216564196797793749"
        "2381352744656633329902311958853296032765516041667295265055226543806948"
        "0844921279178590685230852805810043617124500309990368055562957");
    n.Square();
    m.SetString(
        "2902130841945177589665162016449731365749359124301015936787287192073762"
        "3928354379418542107057097772342111195376512765827019338736271117291725"
        "4842491555209808089298233500256901159707774581220239792305894702400718"
        "4226278482424577714401421252255400172071586680211958843130031646335689"
        "5324274094747281376624252088869179157020712829629393206019374225902765"
        "5131589237232055417058854401516386978283594190583849");
    assert(n == m);

    constexpr size_t k = 572;
    std::string long_ten_pow(k + 1, '0');
    long_ten_pow[0] = '1';
    n.SetString(long_ten_pow);
    n.Square();

    std::string long_ten_pow_square(2 * k + 1, '0');
    long_ten_pow_square[0] = '1';
    m.SetString(long_ten_pow_square);
    assert(n == m);
}

void TestUIntMult() {
    puts(__PRETTY_FUNCTION__);
    LongInt n;
    constexpr uint64_t K = 6000;
    for (uint64_t i = 0; i <= K; i++) {
        for (uint32_t j = 0; j <= K; j++) {
            n = i;
            n *= j;
            assert(n == i * j);
        }
    }

    for (uint128_t i = (uint128_t(-1) / K) - K; i != (uint128_t(-1) / K); i++) {
        for (uint32_t j = 0; j < K; j++) {
            n = i;
            n *= j;
            assert(n == i * j);
        }
    }

    for (uint32_t i = uint32_t(-1) - K; i != 0; i++) {
        for (uint32_t j = uint32_t(-1) - K; j != 0; j++) {
            n = i;
            n *= uint32_t(j);
            assert(n == uint64_t(i) * uint64_t(j));
        }
    }

    for (uint64_t i = uint64_t(-1) - K; i != 0; i++) {
        for (uint32_t j = uint32_t(-1) - K; j != 0; j++) {
            n = i;
            n *= j;
            assert(n == uint128_t(i) * j);
        }
    }
}

void TestUIntAdd() {
    puts(__PRETTY_FUNCTION__);
    LongInt n;
    n.ReserveWithoutCopy(4);
    constexpr uint32_t K = 6000;
    for (uint32_t i = 0; i <= K; i++) {
        for (uint32_t j = 0; j <= K; j++) {
            n = i;
            assert(n == i);
            n += j;
            assert(n == i + j);
        }
    }

    for (uint32_t i = uint32_t(-1) - K; i != 0; i++) {
        for (uint32_t j = uint32_t(-1) - K; j != 0; j++) {
            n = i;
            assert(n == uint64_t(i));
            n += j;
            assert(n == uint64_t(i) + uint64_t(j));
        }
    }

    for (uint64_t i = uint64_t(-1) - K; i != 0; i++) {
        for (uint32_t j = uint32_t(-1) - K; j != 0; j++) {
            n = i;
            assert(n == i);
            n += j;
            assert(n == uint128_t(i) + uint128_t(j));
        }
    }

    for (uint128_t i = uint64_t(-1); i != uint128_t(uint64_t(-1)) + 2 * K;
         i++) {
        for (uint32_t j = uint32_t(-1) - K; j != 0; j++) {
            n = i;
            assert(n == i);
            n += j;
            assert(n == i + j);
        }
    }

    const uint128_t h = uint128_t(-1) / 2;
    for (uint128_t i = h - 2 * K; i != h; i++) {
        for (uint32_t j = uint32_t(-1) - K; j != 0; j++) {
            n = i;
            n += j;
            assert(n == i + j);
        }
    }

    for (int32_t i = int32_t(1u << 31); i != int32_t(K) + int32_t(1u << 31);
         i++) {
        for (uint32_t j = 0; j <= K; j++) {
            n = i;
            n += j;
            assert(n == i + int32_t(j));
        }
    }

    for (int32_t i = int32_t(1u << 31); i != int32_t(K) + int32_t(1u << 31);
         i++) {
        for (uint32_t j = uint32_t(-1) - K; j != 0; j++) {
            n = i;
            n += j;
            assert(n == i + int64_t(j));
        }
    }

    for (int64_t i = -int64_t(1ll << 62); i != int64_t(K) - int64_t(1ll << 62);
         i++) {
        for (uint32_t j = 0; j <= K; j++) {
            n = i;
            n += j;
            assert(n == i + int32_t(j));
        }
    }

    for (int64_t i = -int64_t(1ll << 62); i != int64_t(K) - int64_t(1ll << 62);
         i++) {
        for (uint32_t j = uint32_t(-1) - K; j != 0; j++) {
            n = i;
            n += j;
            assert(n == i + int64_t(j));
        }
    }

    for (int64_t i = int64_t(1ull << 63); i != K + int64_t(1ull << 63); i++) {
        for (uint32_t j = 0; j <= K; j++) {
            n = i;
            n += j;
            assert(n == i + int32_t(j));
        }
    }

    for (int64_t i = int64_t(1ull << 63); i != K + int64_t(1ull << 63); i++) {
        for (uint32_t j = uint32_t(-1) - K; j != 0; j++) {
            n = i;
            n += j;
            assert(n == i + int64_t(j));
        }
    }
}

void TestLongIntAdd() {
    puts(__PRETTY_FUNCTION__);
    LongInt n;
    n.ReserveWithoutCopy(4);
    LongInt m;
    m.ReserveWithoutCopy(4);
    constexpr uint32_t K = 6000;
    for (uint32_t i = 0; i <= K; i++) {
        for (uint32_t j = 1; j <= K; j++) {
            n = i;
            m = j;
            n += m;
            assert(n == i + j);
        }
    }

    for (uint32_t i = uint32_t(-1) - K; i != 0; i++) {
        for (uint32_t j = uint32_t(-1) - K; j != 0; j++) {
            n = i;
            m = j;
            n += m;
            assert(n == i + uint64_t(j));
        }
    }

    for (uint64_t i = uint64_t(-1) - uint32_t(-1) - K;
         i != uint64_t(-1) - uint32_t(-1); i++) {
        for (uint32_t j = uint32_t(-1) - K; j != 0; j++) {
            n = i;
            m = uint32_t(j);
            n += m;
            assert(n == i + j);
        }
    }

    for (uint128_t i = uint64_t(-1); i != uint128_t(uint64_t(-1)) + 2 * K;
         i++) {
        for (uint32_t j = uint32_t(-1) - K; j != 0; j++) {
            n = i;
            m = j;
            n += m;
            assert(n == i + j);
        }
    }

    const uint128_t h = uint128_t(-1) / 2;
    for (uint128_t i = h - 2 * K; i != h; i++) {
        for (uint128_t j = h - 2 * K; j != h; j++) {
            n = i;
            m = j;
            n += m;
            assert(n == i + j);
        }
    }
}

void TestSetString() {
    puts(__PRETTY_FUNCTION__);
    constexpr int64_t numbersI64[] = {
        0,
        1,
        -1,
        2,
        -2,
        4,
        -4,
        8,
        -8,
        9,
        -9,
        10,
        -10,
        11,
        -11,
        2147483648,
        -2147483648,
        4294967295,
        -4294967295,
        4294967296,
        -4294967296,
        131241234423234ll,
        5984348957348ll,
        9223372036854775807ll,
        -9223372036854775807ll,
        -9223372036854775807ll - 1,
    };

    LongInt n;
    for (int64_t number : numbersI64) {
        n.SetString(std::to_string(number));
        assert(n == number);
    }

    constexpr uint64_t numbersU64[] = {
        0,
        1,
        2,
        4,
        8,
        9,
        10,
        11,
        2147483648,
        4294967295,
        4294967296,
        9223372036854775808ull,
        18446744073709551615ull,
    };
    for (uint64_t number : numbersU64) {
        n.SetString(std::to_string(number));
        assert(n == number);
    }

    constexpr uint128_t numbersU128[]{
        0,
        1,
        2,
        4,
        8,
        uint128_t(9999999999) * 10'000'000'000 + 9999999999,
        ((uint128_t(42576258ull) << 64) | uint128_t(9439515947379090504ull)),
        (uint128_t(4581048384968843434ull) << 64) |
            (uint128_t(15881123738085757915ull)),
        (uint128_t(15146611459005431080ull) << 64) |
            (uint128_t(11003818173265126250ull)),
        (uint128_t(107408329755340997ull) << 64) |
            (uint128_t(10118848797326968254ull)),
        (uint128_t(15406421307076602009ull) << 64) |
            (uint128_t(3266360438134194608ull)),
        (uint128_t(13098489512494978084ull) << 64) |
            (uint128_t(13194323124312210617ull)),
        (uint128_t(9593560117762545909ull) << 64) |
            (uint128_t(6883651453229059866ull)),
        (uint128_t(uint64_t(-1)) << 64) | (uint128_t(uint64_t(-1)))};
    for (uint128_t number : numbersU128) {
        n.SetString(std::to_string(number));
        assert(n == number);
    }

    uint128_t c = 0;
    std::string s;
    s.reserve(39);
    for (size_t i = 0; i < 39; i++) {
        n.SetString(s);
        assert(n == c);
        s.push_back('9');
        c = c * 10 + 9;
    }

    // 2^256 - 1
    n.SetString(
        "1157920892373161954235709850086879078532699846656405640394575840079131"
        "29639935");
    size_t nums_count;
    nums_count = 256 / (sizeof(uint32_t) * 8);
    assert(n.Size() == int32_t(nums_count));
    for (size_t i = 0; i < nums_count; i++) {
        assert(n[i] == uint32_t(-1));
    }

    // 2^512 - 1
    n.SetString(
        "1340780792994259709957402499820584612747936582059239337772356144372176"
        "4030073546976801874298166903427690031858186486050853753882811946569946"
        "433649006084095");
    nums_count = 512 / (sizeof(uint32_t) * 8);
    assert(n.Size() == int32_t(nums_count));
    for (size_t i = 0; i < nums_count; i++) {
        assert(n[i] == uint32_t(-1));
    }
}

void TestToString() {
    puts(__PRETTY_FUNCTION__);
    std::string buffer;

    constexpr int64_t numbersI64[] = {
        0,
        1,
        -1,
        2,
        -2,
        4,
        -4,
        8,
        -8,
        9,
        -9,
        10,
        -10,
        11,
        -11,
        2147483648ll,
        -2147483648ll,
        4294967295ll,
        -4294967295l,
        4294967296ll,
        -4294967296ll,
        131241234423234ll,
        5984348957348ll,
        9223372036854775807ll,
        -9223372036854775807ll,
        -9223372036854775807ll - 1,
    };

    LongInt n;
    for (int64_t number : numbersI64) {
        n = number;
        n.ToString(buffer);
        assert(buffer == std::to_string(number));
    }

    constexpr uint64_t numbersU64[] = {
        0,
        1,
        2,
        4,
        8,
        9,
        10,
        11,
        2147483648,
        4294967295,
        4294967296,
        9223372036854775808ull,
        18446744073709551615ull,
    };

    for (uint64_t number : numbersU64) {
        n = number;
        n.ToString(buffer);
        assert(buffer == std::to_string(number));
    }

    constexpr uint128_t numbersU128[] = {
        0,
        1,
        2,
        4,
        8,
        ((uint128_t(42576258ull) << 64) | uint128_t(9439515947379090504ull)),
        (uint128_t(4581048384968843434ull) << 64) |
            (uint128_t(15881123738085757915ull)),
        (uint128_t(15146611459005431080ull) << 64) |
            (uint128_t(11003818173265126250ull)),
        (uint128_t(107408329755340997ull) << 64) |
            (uint128_t(10118848797326968254ull)),
        (uint128_t(15406421307076602009ull) << 64) |
            (uint128_t(3266360438134194608ull)),
        (uint128_t(13098489512494978084ull) << 64) |
            (uint128_t(13194323124312210617ull)),
        (uint128_t(9593560117762545909ull) << 64) |
            (uint128_t(6883651453229059866ull)),
        uint128_t(-1)};
    for (uint128_t number : numbersU128) {
        n = number;
        n.ToString(buffer);
        assert(buffer == std::to_string(number));
    }

    uint128_t c = 0;
    std::string s;
    s.reserve(39);
    buffer.reserve(39);
    n = c;
    n.ToString(buffer);
    assert(buffer.size() == 1 && buffer[0] == '0');
    for (size_t i = 0; i < 38; i++) {
        s.push_back('9');
        c = c * 10 + 9;
        n = c;
        assert(n == c);
        n.ToString(buffer);
        assert(buffer == s);
    }

    constexpr size_t k = size_t(1e6);
    std::string s1(k, '9');
    n.SetString(s1);
    n *= n;
    std::string ans(2 * k, '\0');
    memset(ans.data(), '9', (k - 1) * sizeof(char));
    ans[k - 1] = '8';
    memset(ans.data() + k, '0', (k - 1) * sizeof(char));
    ans[2 * k - 1] = '1';
    buffer.reserve(2 * k);
    n.ToString(buffer);
    if (buffer != ans) {
        std::cout << "Long test failed\n";
    }
}

void TestBitShifts() {
    puts(__PRETTY_FUNCTION__);
    constexpr uint32_t k = 4096;
    LongInt n;
    n.Reserve(4);
    for (uint32_t i = 0; i <= k; i++) {
        for (uint32_t shift = 0; shift <= 31; shift++) {
            n = i;
            n >>= shift;
            assert(n == (i >> shift));
        }
        for (uint32_t j = 0; j <= 16; j++) {
            n = i;
            n >>= 32 + j;
            assert(n == 0);
        }
    }

    for (uint128_t i = uint128_t(-1) - k; i != 0; i++) {
        for (uint32_t shift = 0; shift <= 127; shift++) {
            n = i;
            n >>= shift;
            assert(n == (i >> shift));
        }
        for (uint32_t j = 0; j <= 16; j++) {
            n = i;
            n >>= 128 + j;
            assert(n == 0);
        }
    }

    // 1 << 255
    n.SetString(
        "5789604461865809771178549250434395392663499233282028201972879200395656"
        "4819968");
    LongInt m;
    m.Reserve(uint32_t(n.USize()));
    for (uint32_t shift = 0; shift <= 127; shift++) {
        m = n;
        m >>= (255 - shift);
        assert(m == uint128_t(1) << shift);
    }
}

void TestDecimal() {
    puts(__PRETTY_FUNCTION__);
    LongInt::Decimal d1(0u);
    LongInt::Decimal d2(0u);
    constexpr uint32_t kC = 2000;

    for (uint32_t i = 0; i <= kC; i++) {
        for (uint32_t j = 0; j <= kC; j++) {
            d1 = i;
            d2 = j;
            d1 += d2;
            assert(d1 == i + j);
        }
    }

    for (uint32_t i = uint32_t(-1) - kC; i != 0; i++) {
        for (uint32_t j = uint32_t(-1) - kC; j != 0; j++) {
            d1 = i;
            d2 = j;
            d1 += d2;
            assert(d1 == uint64_t(i) + j);
        }
    }

    {
        d1 = uint64_t(1e18);
        constexpr size_t k = 29;
        for (size_t i = 0; i < k; i++) {
            d1 += d1;
        }

        assert(d1.size_ == 3 && d1.digits_[0] == 0 && d1.digits_[1] == 0 &&
               d1.digits_[2] == (1 << k));

        d1 += d1;
        assert(
            d1.size_ == 4 && d1.digits_[0] == 0 && d1.digits_[1] == 0 &&
            d1.digits_[2] == (1u << (k + 1)) % LongInt::Decimal::kDecimalBase &&
            d1.digits_[3] == (1u << (k + 1)) / LongInt::Decimal::kDecimalBase);
    }

    {
        d1 = uint32_t(999'999'999u);
        d2 = uint64_t(999'999'999'999'999'999ull);
        d1 += d2;
        assert(d1.size_ == 3 && d1.digits_[0] == 999999998 &&
               d1.digits_[1] == 0 && d1.digits_[2] == 1);
    }

    for (uint32_t i = 0; i <= kC; i++) {
        for (uint32_t j = 0; j <= kC; j++) {
            d1 = i;
            d2 = j;
            d1 *= d2;
            assert(d1 == i * j);
        }
    }

    for (uint32_t i = uint32_t(-1) - kC; i != 0; i++) {
        for (uint32_t j = uint32_t(-1) - kC; j != 0; j++) {
            d1 = i;
            d2 = j;
            d1 *= d2;
            assert(d1 == uint64_t(i) * j);
        }
    }

    {
        d1 = uint64_t(1e18);
        constexpr size_t kInitialZeroLimbs = 2;
        assert(d1.size_ == kInitialZeroLimbs + 1);
        for (size_t i = 0; i < kInitialZeroLimbs; i++) {
            assert(d1.digits_[i] == 0);
        }
        assert(d1.digits_[kInitialZeroLimbs] == 1);

        constexpr size_t kMults = 20;
        for (size_t i = 0; i < kMults; i++) {
            d1 *= d1;
        }

        constexpr size_t kNewZeroLimbs = kInitialZeroLimbs << kMults;
        assert(d1.size_ == kNewZeroLimbs + 1);
        for (size_t i = 0; i < kNewZeroLimbs; i++) {
            assert(d1.digits_[i] == 0);
        }
        assert(d1.digits_[kNewZeroLimbs] == 1);
    }

    {
        d1 = uint64_t(1) << 32;
        assert(d1.size_ == 2);
        assert(d1.digits_[0] == 294967296 && d1.digits_[1] == 4);

        d1 *= d1;
        assert(d1.size_ == 3);
        assert(d1.digits_[0] == 709551616 && d1.digits_[1] == 446744073 &&
               d1.digits_[2] == 18);

        d1 *= d1;
        assert(d1.size_ == 5);
        assert(d1.digits_[0] == 768211456 && d1.digits_[1] == 374607431 &&
               d1.digits_[2] == 938463463 && d1.digits_[3] == 282366920 &&
               d1.digits_[4] == 340);

        d1 *= d1;
        assert(d1.size_ == 9);
        assert(d1.digits_[0] == 129639936 && d1.digits_[1] == 584007913 &&
               d1.digits_[2] == 564039457 && d1.digits_[3] == 984665640 &&
               d1.digits_[4] == 907853269 && d1.digits_[5] == 985008687 &&
               d1.digits_[6] == 195423570 && d1.digits_[7] == 89237316 &&
               d1.digits_[8] == 115792);
    }

    for (uint32_t i = 0; i <= kC; i++) {
        d1 = i;
        d1.SquareThisTo(d1);
        assert(d1 == i * i);
    }

    for (uint32_t i = uint32_t(-1) - kC; i != 0; i++) {
        d1 = i;
        d1.SquareThisTo(d1);
        assert(d1 == uint64_t(i) * i);
    }

    {
        d1 = uint64_t(1e18);
        constexpr size_t kInitialZeroLimbs = 2;
        assert(d1.size_ == kInitialZeroLimbs + 1);
        for (size_t i = 0; i < kInitialZeroLimbs; i++) {
            assert(d1.digits_[i] == 0);
        }
        assert(d1.digits_[kInitialZeroLimbs] == 1);

        constexpr size_t kMults = 20;
        for (size_t i = 0; i < kMults; i++) {
            d1.SquareThisTo(d1);
        }

        constexpr size_t kNewZeroLimbs = kInitialZeroLimbs << kMults;
        assert(d1.size_ == kNewZeroLimbs + 1);
        for (size_t i = 0; i < kNewZeroLimbs; i++) {
            assert(d1.digits_[i] == 0);
        }
        assert(d1.digits_[kNewZeroLimbs] == 1);
    }

    {
        d1 = uint64_t(1) << 32;
        assert(d1.size_ == 2);
        assert(d1.digits_[0] == 294967296 && d1.digits_[1] == 4);

        d1.SquareThisTo(d1);
        assert(d1.size_ == 3);
        assert(d1.digits_[0] == 709551616 && d1.digits_[1] == 446744073 &&
               d1.digits_[2] == 18);

        d1.SquareThisTo(d1);
        assert(d1.size_ == 5);
        assert(d1.digits_[0] == 768211456 && d1.digits_[1] == 374607431 &&
               d1.digits_[2] == 938463463 && d1.digits_[3] == 282366920 &&
               d1.digits_[4] == 340);

        d1.SquareThisTo(d1);
        assert(d1.size_ == 9);
        assert(d1.digits_[0] == 129639936 && d1.digits_[1] == 584007913 &&
               d1.digits_[2] == 564039457 && d1.digits_[3] == 984665640 &&
               d1.digits_[4] == 907853269 && d1.digits_[5] == 985008687 &&
               d1.digits_[6] == 195423570 && d1.digits_[7] == 89237316 &&
               d1.digits_[8] == 115792);
    }
}

}  // namespace long_int_tests

int main() {
    using namespace long_int_tests;
    TestOperatorEqualsInt();
    TestUIntMult();
    TestUIntAdd();
    TestLongIntMult();
    TestLongIntAdd();
    TestSetString();
    TestToString();
    TestLongIntSquare();
    TestBitShifts();
    TestDecimal();

    // std::ios::sync_with_stdio(false);
    // std::cin.tie(nullptr);
    // LongInt n1;
    // LongInt n2;
    // std::cin >> n1 >> n2;
    // n1 *= n2;
    // std::cout << n1;
    // std::cout.flush();
    // return 0;
}
