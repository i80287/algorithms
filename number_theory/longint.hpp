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

// https://en.cppreference.com/w/cpp/numeric/bit_cast
#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_lib_bit_cast) && __cpp_lib_bit_cast >= 201806L
#include <bit>
#define LONG_INT_USE_BIT_CAST 1
#else
#define LONG_INT_USE_BIT_CAST 0
#endif

#if !defined(__GNUG__)
// cppcheck-suppress [preprocessorErrorDirective]
#error "Current implementation works only with GCC"
#endif

namespace longint_allocator {

// #define DEBUG_LI_ALLOC_PRINTING 1

class inner_impl final {
private:
    static constexpr std::size_t kPageMemoryOffsetInBytes = sizeof(void*);

    struct alignas(64) SmallPage {
        static constexpr std::size_t kCapacity =
            32 * sizeof(std::uint32_t) - kPageMemoryOffsetInBytes;

        std::byte memory[kCapacity];
        SmallPage* next;
    };
    static_assert((sizeof(SmallPage) & (sizeof(SmallPage) - 1)) == 0, "");
    static_assert(sizeof(SmallPage) % alignof(SmallPage) == 0);
    static_assert(sizeof(SmallPage) == sizeof(SmallPage::memory) + kPageMemoryOffsetInBytes, "");

    struct alignas(64) MiddlePage {
        static constexpr std::size_t kCapacity =
            256 * sizeof(std::uint32_t) - kPageMemoryOffsetInBytes;

        std::byte memory[kCapacity];
        MiddlePage* next;
    };

    static_assert((sizeof(MiddlePage) & (sizeof(MiddlePage) - 1)) == 0, "");
    static_assert(sizeof(MiddlePage) % alignof(MiddlePage) == 0);
    static_assert(sizeof(MiddlePage) == sizeof(MiddlePage::memory) + kPageMemoryOffsetInBytes, "");

    static constexpr std::size_t kTotalSmallPages  = 32;
    static constexpr std::size_t kTotalMiddlePages = 8;

#if defined(__cpp_constinit) && __cpp_constinit >= 201907L
#define LI_ALLOC_CONSTINIT constinit
#else
#define LI_ALLOC_CONSTINIT
#endif

    LI_ALLOC_CONSTINIT
    static inline SmallPage first_small_page[kTotalSmallPages] = {};
    LI_ALLOC_CONSTINIT
    static inline MiddlePage first_middle_page[kTotalMiddlePages] = {};

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

    template <class PageType, std::size_t TotalPages>
    ATTRIBUTE_ALWAYS_INLINE static bool IsPage(const std::byte* memory,
                                               const PageType (&pages_array)[TotalPages]) {
        const auto mem_addr = reinterpret_cast<std::uintptr_t>(memory);
        const auto first_page_addr =
            reinterpret_cast<std::uintptr_t>(std::addressof(pages_array[0]));
        return mem_addr - first_page_addr < TotalPages * sizeof(PageType);
    }

    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE static bool IsSmallPage(const std::byte* offset_memory) noexcept {
        return inner_impl::IsPage(offset_memory, first_small_page);
    }

    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE static bool IsMiddlePage(const std::byte* offset_memory) noexcept {
        return inner_impl::IsPage(offset_memory, first_middle_page);
    }

    friend inline void* Allocate(std::size_t size);
    friend inline void Deallocate(void* memory) noexcept;
};

inline void Deallocate(void* memory) noexcept {
    if (unlikely(memory == nullptr)) {
        return;
    }

    std::byte* p = static_cast<std::byte*>(memory);
    if (inner_impl::IsSmallPage(p)) {
#ifdef DEBUG_LI_ALLOC_PRINTING
        inner_impl::current_small_pages_used--;
        assert(inner_impl::current_small_pages_used >= 0);
#endif
        auto* page                        = reinterpret_cast<inner_impl::SmallPage*>(p);
        page->next                        = inner_impl::free_small_pages_head;
        inner_impl::free_small_pages_head = page;
        return;
    }

    if (inner_impl::IsMiddlePage(p)) {
#ifdef DEBUG_LI_ALLOC_PRINTING
        inner_impl::current_middle_pages_used--;
        assert(inner_impl::current_middle_pages_used >= 0);
#endif
        auto* page                         = reinterpret_cast<inner_impl::MiddlePage*>(p);
        page->next                         = inner_impl::free_middle_pages_head;
        inner_impl::free_middle_pages_head = page;
        return;
    }
#ifdef DEBUG_LI_ALLOC_PRINTING
    inner_impl::malloc_free_count--;
    assert(inner_impl::malloc_free_count >= 0);
#endif
    ::operator delete(memory);
}

#define IS_INLINE_LONGINT_ALLOCATE 1
#if IS_INLINE_LONGINT_ALLOCATE
#define INLINE_LONGINT_ALLOCATE inline
#else
#define INLINE_LONGINT_ALLOCATE
#endif

#if CONFIG_GNUC_AT_LEAST(2, 96)
#if IS_INLINE_LONGINT_ALLOCATE || defined(__clang__) || defined(__MINGW32__) || \
    !CONFIG_GNUC_AT_LEAST(10, 0)
__attribute__((malloc))
#else
__attribute__((malloc, malloc(::longint_allocator::Deallocate, 1)))
#endif
#endif
ATTRIBUTE_RETURNS_NONNULL
ATTRIBUTE_ALLOC_SIZE(1) INLINE_LONGINT_ALLOCATE void* Allocate(std::size_t size) {
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

#undef INLINE_LONGINT_ALLOCATE
#undef IS_INLINE_LONGINT_ALLOCATE

#ifdef DEBUG_LI_ALLOC_PRINTING
#undef DEBUG_LI_ALLOC_PRINTING
#endif

}  // namespace longint_allocator

namespace longint_detail {
struct longint_static_storage;
}

struct longint {
    using digit_t                = std::uint32_t;
    using double_digit_t         = std::uint64_t;
    using pointer                = digit_t*;
    using const_pointer          = const digit_t*;
    using iterator               = pointer;
    using const_iterator         = const_pointer;
    using reverse_iterator       = typename std::reverse_iterator<iterator>;
    using const_reverse_iterator = typename std::reverse_iterator<const_iterator>;

    using size_type  = std::uint32_t;
    using ssize_type = std::int32_t;

    static constexpr std::size_t kDefaultLINumsCapacity32  = 2;
    static constexpr std::size_t kDefaultLINumsCapacity64  = 2;
    static constexpr std::size_t kDefaultLINumsCapacity128 = 4;

    static constexpr std::uint32_t kStrConvBase = 1'000'000'000;
    static constexpr std::uint32_t kStrConvBaseDigits =
        math_functions::base_b_len(kStrConvBase - 1);
    static constexpr std::uint32_t kNumsBits         = 32;
    static constexpr double_digit_t kNumsBase        = double_digit_t{1} << kNumsBits;
    static constexpr std::size_t kFFTPrecisionBorder = 1u << 18;
    static constexpr auto kFFTFloatRoundError =
        std::numeric_limits<typename fft::complex::value_type>::round_error();

    using dec_digit_t = uint32_t;

    static constexpr uint32_t kDecimalBase    = kStrConvBase;
    static constexpr uint32_t kFFTDecimalBase = 1'000;

    constexpr longint() noexcept = default;
    longint(const longint& other) : nums_(nullptr), size_(other.size_), capacity_(other.capacity_) {
        if (capacity_ > 0) {
            nums_ = allocate(capacity_);
            std::copy_n(other.nums_, usize32(), nums_);
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
        this->swap(other);
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
        this->allocate_default_capacity_32();
        this->assign_u32_unchecked(n);
    }
    longint(int32_t n) {
        this->allocate_default_capacity_32();
        this->assign_i32_unchecked(n);
    }
    longint(uint64_t n) {
        this->allocate_default_capacity_64();
        this->assign_u64_unchecked(n);
    }
    longint(int64_t n) {
        this->allocate_default_capacity_64();
        this->assign_i64_unchecked(n);
    }
#if defined(INTEGERS_128_BIT_HPP)
    longint(uint128_t n) {
        this->allocate_default_capacity_128();
        this->assign_u128_unchecked(n);
    }
    longint(int128_t n) {
        this->allocate_default_capacity_128();
        this->assign_i128_unchecked(n);
    }
#endif
    explicit longint(std::string_view s) {
        this->set_string(s);
    }
    static longint from_string(std::string_view s) {
        return longint{s};
    }
    struct Reserve {
        explicit constexpr Reserve(std::uint32_t capacity) noexcept : capacity_(capacity) {}
        std::uint32_t capacity_{};
    };
    explicit longint(Reserve reserve_tag)
        : nums_(allocate(reserve_tag.capacity_)), capacity_(reserve_tag.capacity_) {}

    longint& operator=(uint32_t n) {
        this->ensure_default_capacity_op_asgn_32();
        this->assign_u32_unchecked(n);
        return *this;
    }
    longint& operator=(int32_t n) {
        this->ensure_default_capacity_op_asgn_32();
        this->assign_i32_unchecked(n);
        return *this;
    }
    longint& operator=(uint64_t n) {
        this->ensure_default_capacity_op_asgn_64();
        this->assign_u64_unchecked(n);
        return *this;
    }
    longint& operator=(int64_t n) {
        this->ensure_default_capacity_op_asgn_64();
        this->assign_i64_unchecked(n);
        return *this;
    }
#if defined(INTEGERS_128_BIT_HPP)
    longint& operator=(uint128_t n) {
        this->ensure_default_capacity_op_asgn_128();
        this->assign_u128_unchecked(n);
        return *this;
    }
    longint& operator=(int128_t n) {
        this->ensure_default_capacity_op_asgn_128();
        this->assign_i128_unchecked(n);
        return *this;
    }
#endif

    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_CONST static constexpr size_type max_size() noexcept {
        constexpr auto kMaxSSize = std::numeric_limits<decltype(size_)>::max();
        static_assert(kMaxSSize > 0);
        constexpr auto kMaxUSize = std::numeric_limits<decltype(capacity_)>::max();
        return static_cast<size_type>(
            std::min({static_cast<std::size_t>(kMaxSSize), static_cast<std::size_t>(kMaxUSize),
                      std::numeric_limits<std::size_t>::max() / sizeof(digit_t)}));
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr ssize_type size() const noexcept {
        const auto value = size_;
        static_assert(static_cast<ssize_type>(max_size()) == static_cast<std::int64_t>(max_size()));
        if (value > static_cast<ssize_type>(max_size())) {
            CONFIG_UNREACHABLE();
        }

        return value;
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr std::size_t usize() const noexcept {
        return usize32();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr size_type usize32() const noexcept {
        const auto value = math_functions::uabs(size());
        if (value > max_size()) {
            CONFIG_UNREACHABLE();
        }
        return value;
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr size_type capacity() const noexcept {
        const auto value = capacity_;
        if (value > max_size()) {
            CONFIG_UNREACHABLE();
        }
        return value;
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr std::int32_t sign() const noexcept {
        return math_functions::sign(size());
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr bool iszero() const noexcept {
        return size() == 0;
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr bool empty() const noexcept {
        return iszero();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE explicit constexpr operator bool() const noexcept {
        return !iszero();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr bool operator!() const noexcept {
        return iszero();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr iterator begin() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return nums_;
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr iterator end() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return nums_ + usize();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr const_iterator begin() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return nums_;
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr const_iterator end() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return nums_ + usize();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr const_iterator cbegin() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return begin();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr const_iterator cend() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return end();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr reverse_iterator rbegin() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return std::make_reverse_iterator(end());
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr reverse_iterator rend() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return std::make_reverse_iterator(begin());
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr const_reverse_iterator rbegin() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return std::make_reverse_iterator(end());
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr const_reverse_iterator rend() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return std::make_reverse_iterator(begin());
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr const_reverse_iterator crbegin() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return rbegin();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr const_reverse_iterator crend() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return rend();
    }
    constexpr void change_sign() noexcept {
        size_ = -size_;
    }
    constexpr void set_zero() noexcept {
        size_ = 0;
    }

    longint& pow(std::size_t p) {
        longint res = uint32_t{1};
        reserve((std::max(usize32(), size_type{1}) - 1) * p);
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
        const std::size_t usize_value = usize32();
        if (unlikely(usize_value == 0)) {
            other.set_zero();
            return;
        }
        const uint32_t* nums_ptr    = nums_;
        const std::size_t prod_size = usize_value + usize_value;
        if (prod_size <= 16) {
            other.capacity_ = static_cast<uint32_t>(prod_size);
            uint32_t* ans   = allocate(prod_size);
            std::fill_n(ans, prod_size, uint32_t{0});
            for (std::size_t j = 0; j < usize_value; j++) {
                const double_digit_t b_j = nums_ptr[j];
                double_digit_t carry     = 0;
                for (std::size_t i = 0; i < usize_value; i++) {
                    const double_digit_t a_i = nums_ptr[i];
                    const double_digit_t res =
                        a_i * b_j + static_cast<double_digit_t>(ans[j + i]) + carry;
                    ans[j + i] = static_cast<digit_t>(res);
                    carry      = res >> 32;
                }

                ans[j + usize_value] = static_cast<digit_t>(carry);
            }

            deallocate(other.nums_);
            other.nums_ = ans;
        } else {
            const auto checked_prod_size = check_size(prod_size);
            ATTRIBUTE_ASSUME(prod_size == checked_prod_size);
            std::size_t n =
                2 * math_functions::nearest_greater_equal_power_of_two(checked_prod_size);
            const bool high_precision = n > kFFTPrecisionBorder;
            n <<= high_precision;
            // Allocate n complex numbers for p1 and n complex numbers for p2
            const std::unique_ptr<fft::complex, struct ComplexDeleter> p1(
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
                    *p = fft::complex(static_cast<uint8_t>(value), static_cast<uint8_t>(value));
                    p++;
                    value >>= 8;
                    *p = fft::complex(static_cast<uint8_t>(value), static_cast<uint8_t>(value));
                    p++;
                    value >>= 8;
                    *p = fft::complex(static_cast<uint8_t>(value), static_cast<uint8_t>(value));
                    p++;
                    value >>= 8;
                    *p = fft::complex(value, value);
                    p++;
                }
            }
            std::memset(static_cast<void*>(p), 0,
                        (n - (prod_size << high_precision)) * sizeof(fft::complex));

            other.reserveUninitializedWithoutCopy(checked_prod_size);
            fft::complex* p2 = p1.get() + n;
            fft::forward_backward_fft(p1.get(), p2, n);
            convert_fft_poly_to_longint_nums(high_precision, p2, other.nums_, checked_prod_size);
        }

        other.size_ = static_cast<std::int32_t>(prod_size);
        other.pop_leading_zeros();
    }
    longint& SquareInplace() {
        SquareThisTo(*this);
        return *this;
    }
    [[nodiscard]] constexpr digit_t operator[](std::size_t pos) const noexcept {
        return nums_[pos];
    }
    longint& operator*=(const longint& other) {
        std::size_t k        = usize32();
        std::size_t m        = other.usize32();
        const digit_t* k_ptr = nums_;
        const digit_t* m_ptr = other.nums_;

        if (m > k) {
            std::swap(m_ptr, k_ptr);
            std::swap(m, k);
        }

        if (unlikely(m == 0)) {
            set_zero();
            return *this;
        }

        const std::size_t prod_size  = m + k;
        const auto checked_prod_size = check_size(prod_size);
        if (m <= 16 || m * k <= 1024) {
            capacity_    = checked_prod_size;
            digit_t* ans = allocate(prod_size);
            std::fill_n(ans, prod_size, digit_t{0});
            for (std::size_t j = 0; j < m; j++) {
                const double_digit_t b_j = m_ptr[j];
                double_digit_t carry     = 0;
                for (std::size_t i = 0; i < k; i++) {
                    const double_digit_t a_i = k_ptr[i];
                    const double_digit_t res =
                        a_i * b_j + static_cast<double_digit_t>(ans[j + i]) + carry;
                    ans[j + i] = static_cast<digit_t>(res);
                    carry      = res / kNumsBase;
                }

                ans[j + k] = static_cast<digit_t>(carry);
            }

            deallocate(nums_);
            nums_ = ans;
        } else {
            std::size_t n =
                2 * math_functions::nearest_greater_equal_power_of_two(checked_prod_size);
            const bool high_precision = n > kFFTPrecisionBorder;
            n <<= high_precision;
            // Allocate n complex numbers for p1 and n complex numbers for p2
            const std::unique_ptr<fft::complex, struct ComplexDeleter> p1(
                allocate_complex_array_for_unique_ptr(2 * n));
            fft::complex* p = p1.get();
            if (likely(!high_precision)) {
                for (std::size_t i = 0; i < m; i++) {
                    const digit_t m_value = m_ptr[i];
                    const digit_t k_value = k_ptr[i];
                    *p                    = fft::complex(m_value & 0xFFFF, k_value & 0xFFFF);
                    p++;
                    *p = fft::complex(m_value >> 16, k_value >> 16);
                    p++;
                }
                for (std::size_t i = m; i < k; i++) {
                    const digit_t k_value = k_ptr[i];
                    *p                    = fft::complex(0, k_value & 0xFFFF);
                    p++;
                    *p = fft::complex(0, k_value >> 16);
                    p++;
                }
            } else {
                for (std::size_t i = 0; i < m; i++) {
                    uint32_t m_value = m_ptr[i];
                    uint32_t k_value = k_ptr[i];
                    *p = fft::complex(static_cast<uint8_t>(m_value), static_cast<uint8_t>(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = fft::complex(static_cast<uint8_t>(m_value), static_cast<uint8_t>(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = fft::complex(static_cast<uint8_t>(m_value), static_cast<uint8_t>(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = fft::complex(static_cast<uint8_t>(m_value), static_cast<uint8_t>(k_value));
                    p++;
                }
                for (std::size_t i = m; i < k; i++) {
                    uint32_t k_value = k_ptr[i];
                    *p               = fft::complex(0, static_cast<uint8_t>(k_value));
                    p++;
                    k_value >>= 8;
                    *p = fft::complex(0, static_cast<uint8_t>(k_value));
                    p++;
                    k_value >>= 8;
                    *p = fft::complex(0, static_cast<uint8_t>(k_value));
                    p++;
                    k_value >>= 8;
                    *p = fft::complex(0, static_cast<uint8_t>(k_value));
                    p++;
                }
            }
            std::memset(static_cast<void*>(p), 0,
                        (n - ((2 * k) << high_precision)) * sizeof(fft::complex));

            reserveUninitializedWithoutCopy(checked_prod_size);
            fft::complex* const p2 = p1.get() + n;
            fft::forward_backward_fft(p1.get(), p2, n);
            convert_fft_poly_to_longint_nums(high_precision, p2, nums_, checked_prod_size);
        }

        const std::int32_t sign_product = size_ ^ other.size_;
        size_ =
            static_cast<std::int32_t>(sign_product >= 0 ? checked_prod_size : -checked_prod_size);
        pop_leading_zeros();
        return *this;
    }
    [[nodiscard]] longint operator*(const longint& other) const {
        longint copy(*this);
        copy *= other;
        return copy;
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE longint divmod(const longint& other) {
        longint rem;
        this->divmod(other, rem);
        return rem;
    }
    ATTRIBUTE_ALWAYS_INLINE
    void divmod(const longint& other, longint& rem) {
        this->divmod_impl(other, rem);
    }
    longint& operator+=(const longint& other) {
        const bool find_sum    = (size_ ^ other.size_) >= 0;
        const size_type usize2 = other.usize32();
        static_assert(max_size() + 1 > max_size());
        const size_type usize1 =
            set_size_at_least(std::max(usize32(), usize2) + (find_sum ? 1 : 0));
        ATTRIBUTE_ASSUME(usize1 >= usize2);
        if (find_sum) {
            ATTRIBUTE_ASSUME(usize1 > usize2);
            longint_add_with_free_space(nums_, usize1, other.nums_, usize2);
        } else {
            if (longint_subtract_with_free_space(nums_, usize1, other.nums_, usize2)) {
                change_sign();
            }
        }

        pop_leading_zeros();
        return *this;
    }
    longint& operator-=(const longint& other) {
        change_sign();
        this->operator+=(other);
        change_sign();
        return *this;
    }
    longint operator+(const longint& other) const {
        longint res(*this);
        res += other;
        return res;
    }
    longint operator-(const longint& other) const {
        longint res(*this);
        res -= other;
        return res;
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator==(int32_t n) const noexcept {
        if ((config::is_constant_evaluated() || config::is_gcc_constant_p(n)) && n == 0) {
            return iszero();
        }

        static_assert(kNumsBits == 32);
        switch (size_) {
            case 0:
                return n == 0;
            case 1:
                return nums_[0] == static_cast<uint32_t>(n) && n > 0;
            case -1:
                return nums_[0] == -static_cast<uint32_t>(n) && n < 0;
            default:
                return false;
        }
    }
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator==(int64_t n) const noexcept {
        if ((config::is_constant_evaluated() || config::is_gcc_constant_p(n)) && n == 0) {
            return iszero();
        }

        if (this->sign() != ::math_functions::sign(n)) {
            return false;
        }

        /* Do not use std::abs in order to make operator== constexpr */
        const uint64_t n_abs = ::math_functions::uabs(n);
        static_assert(kNumsBits == 32);
        switch (size_) {
            case 0:
                return n == 0;
            case 1:
            case -1:
                return nums_[0] == n_abs;
            case 2:
            case -2:
                return ((static_cast<uint64_t>(nums_[1]) << 32) | nums_[0]) == n_abs;
            default:
                return false;
        }
    }
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator==(uint32_t n) const noexcept {
        if ((config::is_constant_evaluated() || config::is_gcc_constant_p(n)) && n == 0) {
            return iszero();
        }

        static_assert(kNumsBits == 32);
        switch (size_) {
            case 0:
                return n == 0;
            case 1:
                return nums_[0] == n;
            default:
                return false;
        }
    }
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator==(uint64_t n) const noexcept {
        if ((config::is_constant_evaluated() || config::is_gcc_constant_p(n)) && n == 0) {
            return iszero();
        }

        static_assert(kNumsBits == 32);
        switch (size_) {
            case 0:
                return n == 0;
            case 1:
                return nums_[0] == n;
            case 2:
                return ((static_cast<uint64_t>(nums_[1]) << 32) | nums_[0]) == n;
            default:
                return false;
        }
    }
#if defined(INTEGERS_128_BIT_HPP)
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator==(uint128_t n) const noexcept {
        if ((config::is_constant_evaluated() || config::is_gcc_constant_p(n)) && n == 0) {
            return iszero();
        }

        static_assert(kNumsBits == 32);
        switch (size_) {
            case 0:
                return n == 0;
            case 1:
                return nums_[0] == n;
            case 2:
                return ((static_cast<uint64_t>(nums_[1]) << 32) | nums_[0]) == n;
            case 3: {
                const uint64_t low = (static_cast<uint64_t>(nums_[1]) << 32) | nums_[0];
                return ((static_cast<uint128_t>(nums_[2]) << 64) | low) == n;
            }
            case 4: {
                const uint64_t low = (static_cast<uint64_t>(nums_[1]) << 32) | nums_[0];
                const uint64_t hi  = (static_cast<uint64_t>(nums_[3]) << 32) | nums_[2];
                return ((static_cast<uint128_t>(hi) << 64) | low) == n;
            }
            default:
                return false;
        }
    }
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator==(int128_t n) const noexcept {
        if ((config::is_constant_evaluated() || config::is_gcc_constant_p(n)) && n == 0) {
            return iszero();
        }

        if (this->sign() != ::math_functions::sign(n)) {
            return false;
        }

        const uint128_t n_abs = ::math_functions::uabs(n);
        static_assert(kNumsBits == 32);
        switch (size_) {
            case 0:
                return n == 0;
            case 1:
            case -1:
                return nums_[0] == n_abs;
            case 2:
            case -2:
                return ((static_cast<uint64_t>(nums_[1]) << 32) | nums_[0]) == n_abs;
            case 3:
            case -3: {
                const uint64_t low = (static_cast<uint64_t>(nums_[1]) << 32) | nums_[0];
                return ((static_cast<uint128_t>(nums_[2]) << 64) | low) == n_abs;
            }
            case 4:
            case -4: {
                const uint64_t low = (static_cast<uint64_t>(nums_[1]) << 32) | nums_[0];
                const uint64_t hi  = (static_cast<uint64_t>(nums_[3]) << 32) | nums_[2];
                return ((static_cast<uint128_t>(hi) << 64) | low) == n_abs;
            }
            default:
                return false;
        }
    }
#endif
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator==(const longint& other) const noexcept {
        return size_ == other.size_ && std::equal(nums_, nums_ + usize(), other.nums_);
    }
#if CONFIG_HAS_AT_LEAST_CXX_20
    [[nodiscard]]
    ATTRIBUTE_PURE constexpr std::strong_ordering operator<=>(const longint& other) const noexcept {
        if (size_ != other.size_) {
            return size_ <=> other.size_;
        }

        const std::size_t usize_value = usize();
        const digit_t* const r_end    = nums_ - 1;
        const digit_t* r_nums         = r_end + usize_value;
        const digit_t* r_other_nums   = other.nums_ - 1 + usize_value;
        for (; r_nums != r_end; r_nums--, r_other_nums--) {
            if (*r_nums != *r_other_nums) {
                return static_cast<int64_t>(*r_nums) * sign() <=>
                       static_cast<int64_t>(*r_other_nums) * other.sign();
            }
        }

        return std::strong_ordering::equivalent;
    }
#else
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator<(const longint& other) const noexcept {
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
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator>(const longint& other) const noexcept {
        return other < *this;
    }
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator<=(const longint& other) const noexcept {
        return !(*this > other);
    }
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator>=(const longint& other) const noexcept {
        return !(*this < other);
    }
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator!=(int32_t n) const noexcept {
        return !(*this == n);
    }
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator!=(int64_t n) const noexcept {
        return !(*this == n);
    }
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator!=(uint32_t n) const noexcept {
        return !(*this == n);
    }
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator!=(uint64_t n) const noexcept {
        return !(*this == n);
    }
#if defined(INTEGERS_128_BIT_HPP)
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator!=(uint128_t n) const noexcept {
        return !(*this == n);
    }
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator!=(int128_t n) const noexcept {
        return !(*this == n);
    }
#endif
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator!=(const longint& other) const noexcept {
        return !(*this == other);
    }
#endif

    longint& operator+=(uint32_t n) {
        if (unlikely(size() == 0)) {
            return *this = n;
        }

        if (size() > 0) {
            nonZeroSizeAddUInt(n);
        } else {
            nonZeroSizeSubUInt(n);
        }

        return *this;
    }

    longint& operator-=(uint32_t n) {
        if (unlikely(size() == 0)) {
            *this = n;
            this->change_sign();
            return *this;
        }

        if (size() > 0) {
            nonZeroSizeSubUInt(n);
        } else {
            nonZeroSizeAddUInt(n);
        }

        return *this;
    }

    longint& operator*=(uint32_t x) {
        if (unlikely(x == 0)) {
            set_zero();
            return *this;
        }

        double_digit_t carry     = 0;
        const double_digit_t b_0 = x;
        const size_type u32size  = usize32();
        for (digit_t *nums_it = nums_, *nums_it_end = nums_it + u32size; nums_it != nums_it_end;
             ++nums_it) {
            const double_digit_t res = *nums_it * b_0 + carry;
            *nums_it                 = static_cast<digit_t>(res);
            carry                    = res >> kNumsBits;
        }

        // x != 0 => sign won't change and there will be no leading zeros
        if (carry != 0) {
            if (unlikely(u32size == capacity_)) {
                growCapacity();
            }

            assert(u32size < capacity_);
            nums_[u32size] = static_cast<digit_t>(carry);
            size_ += sign();
        }

        return *this;
    }
    longint& operator*=(int32_t n) {
        const bool negative = n < 0;
        this->operator*=(math_functions::uabs(n));
        if (negative) {
            change_sign();
        }
        return *this;
    }
    ATTRIBUTE_ALWAYS_INLINE
    constexpr longint& operator/=(uint32_t n) noexcept {
        static_cast<void>(divmod(n));
        return *this;
    }
    ATTRIBUTE_ALWAYS_INLINE
    constexpr longint& operator/=(const int32_t n) noexcept {
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
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr uint32_t divmod(const uint32_t n) noexcept {
        if ((config::is_constant_evaluated() || config::is_gcc_constant_p(n)) &&
            (n & (n - 1)) == 0) {
            if (n <= 1) {
                /* Quite return when dividing by zero, i.e. n = 0. */
                return 0;
            }

            static_assert(kNumsBits >= 32);
            const auto shift     = static_cast<uint32_t>(math_functions::countr_zero(n));
            const auto remainder = static_cast<uint32_t>(size_ == 0 ? digit_t{0} : nums_[0] % n);
            this->operator>>=(shift);
            return remainder;
        }

        return this->divmod_impl(n);
    }

    constexpr longint& operator>>=(size_type shift) noexcept {
        if ((config::is_constant_evaluated() || config::is_gcc_constant_p(shift)) && shift == 0) {
            return *this;
        }

        size_type usize_value      = usize32();
        const size_type uints_move = shift / kNumsBits;
        if (uints_move >= usize_value) {
            set_zero();
            return *this;
        }

        if (uints_move != 0) {
            usize_value -= uints_move;
            set_ssize_from_size(usize_value);
            uint32_t* copy_dst_start       = nums_;
            const uint32_t* copy_src_start = copy_dst_start + uints_move;
            const uint32_t* copy_src_end   = copy_src_start + usize_value;
            std::copy(copy_src_start, copy_src_end, copy_dst_start);
        }

        shift %= kNumsBits;
        digit_t* nums_iter            = nums_;
        digit_t* const nums_iter_last = nums_iter + usize_value - 1;
        if (shift > 0) {
            for (; nums_iter != nums_iter_last; ++nums_iter) {
                const digit_t current_digit = nums_iter[0];
                const digit_t next_digit    = nums_iter[1];
                *nums_iter                  = static_cast<digit_t>(
                    (current_digit | double_digit_t{next_digit} << kNumsBits) >> shift);
            }
            *nums_iter_last >>= shift;
        }

        if (*nums_iter_last == 0) {
            size_ -= size_ >= 0 ? 1 : -1;
        }

        return *this;
    }
    constexpr longint& operator<<=(size_type shift) {
        if ((config::is_constant_evaluated() || config::is_gcc_constant_p(shift)) && shift == 0) {
            return *this;
        }

        size_type usize_value = usize32();
        if (usize_value == 0) {
            return *this;
        }

        static_assert(sizeof(digit_t) == sizeof(uint32_t));
        const size_type new_trailig_zeros_digits = shift / kNumsBits;
        // + 1 for potentially new back digit (at the end of the nums_ array)
        const auto new_size = usize_value + new_trailig_zeros_digits + 1;
        reserve(new_size);
        digit_t* last_digit_ptr = nums_ + usize_value + new_trailig_zeros_digits;
        *last_digit_ptr         = 0;
        if (new_trailig_zeros_digits > 0) {
            std::copy_backward(nums_, nums_ + usize_value, last_digit_ptr);
            std::fill_n(nums_, new_trailig_zeros_digits, digit_t{0});
        }
        usize_value = new_size;

        shift %= kNumsBits;
        if (shift > 0) {
            digit_t* const nums_iter_begin = nums_ + new_trailig_zeros_digits;
            digit_t* const nums_iter_end   = nums_ + usize_value;
            for (digit_t* nums_iter = nums_iter_end - 1; nums_iter > nums_iter_begin; nums_iter--) {
                const digit_t prev_digit    = *(nums_iter - 1);
                const digit_t current_digit = *nums_iter;
                const double_digit_t two_digits =
                    (double_digit_t{current_digit} << kNumsBits) | prev_digit;
                *nums_iter = static_cast<digit_t>(two_digits >> (kNumsBits - shift));
            }
            *nums_iter_begin = (*nums_iter_begin) << shift;
        }

        if (nums_[usize_value - 1] == 0) {
            usize_value--;
        }
        size_ = static_cast<std::int32_t>(size_ >= 0 ? usize_value : -usize_value);
        return *this;
    }

    inline void set_string(std::string_view s);

    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr bool fits_in_uint32() const noexcept {
        return static_cast<std::uint32_t>(size()) <= 1;
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr std::uint32_t to_uint32() const noexcept {
        static_assert(kNumsBits == 32);
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
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr bool fits_in_uint64() const noexcept {
        return static_cast<std::uint32_t>(size()) <= 2;
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr std::uint64_t to_uint64() const noexcept {
        std::uint64_t value = 0;
        static_assert(kNumsBits == 32);
        switch (usize()) {
            default:
                if (fits_in_uint64()) {
                    CONFIG_UNREACHABLE();
                }
                [[fallthrough]];
            case 2:
                value |= static_cast<std::uint64_t>(nums_[1]) << 32;
                [[fallthrough]];
            case 1:
                value |= nums_[0];
                break;
            case 0:
                break;
        }
        return value;
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE /* implicit */ constexpr operator std::uint32_t()
        const noexcept {
        return to_uint32();
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE /* implicit */ constexpr operator std::uint64_t()
        const noexcept {
        return to_uint64();
    }
#if defined(INTEGERS_128_BIT_HPP)
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr bool fits_in_uint128()
        const noexcept {
        return static_cast<std::uint32_t>(size()) <= 4;
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr uint128_t to_uint128() const noexcept {
        uint128_t value = 0;
        static_assert(kNumsBits == 32);
        switch (usize()) {
            default:
                if (fits_in_uint128()) {
                    CONFIG_UNREACHABLE();
                }
                [[fallthrough]];
            case 4:
                value |= static_cast<uint128_t>(nums_[3]) << 96;
                [[fallthrough]];
            case 3:
                value |= static_cast<uint128_t>(nums_[2]) << 64;
                [[fallthrough]];
            case 2:
                value |= static_cast<std::uint64_t>(nums_[1]) << 32;
                [[fallthrough]];
            case 1:
                value |= nums_[0];
                break;
            case 0:
                break;
        }
        return value;
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE /* implicit */ constexpr
    operator uint128_t() const noexcept {
        return to_uint128();
    }
#endif

    [[nodiscard]] std::string to_string() const {
        std::string s;
        append_to_string(s);
        return s;
    }
    /// @brief Write 10-base representation of *this into the @a ans
    /// @param s
    void to_string(std::string& s) const {
        s.clear();
        append_to_string(s);
    }
    void append_to_string(std::string& ans) const {
        if (size() < 0) {
            ans.push_back('-');
        }

        const std::uint32_t usize32_value = usize32();
        switch (usize32_value) {
            case 0:
                ans = "0";
                return;
            case 1:
                ans += std::to_string(nums_[0]);
                return;
            case 2:
                ans += std::to_string((static_cast<uint64_t>(nums_[1]) << kNumsBits) | nums_[0]);
                return;
            default:
                break;
        }

        std::uint64_t n = math_functions::nearest_greater_equal_power_of_two(usize32_value);
        ensureBinBasePowsCapacity(math_functions::log2_floor(static_cast<uint64_t>(n)));
        digit_t* const knums = allocate(n);
        std::fill_n(std::copy_n(nums_, usize32_value, knums), n - usize32_value, digit_t{0});
        const Decimal result = convertBinBase(knums, n);
        deallocate(knums);
        assert(result.size_ >= 3);
        const std::size_t full_blocks = result.size_ - 1;
        uint32_t last_a_i             = result.digits_[full_blocks];
        const std::size_t string_size =
            full_blocks * kStrConvBaseDigits + math_functions::base_10_len(last_a_i);
        ans.resize(ans.size() + string_size);
        auto* ptr = reinterpret_cast<uint8_t*>(std::addressof(ans[ans.size() - 1]));
        for (std::size_t i = 0; i < full_blocks; i++) {
            uint32_t a_i = result.digits_[i];
            for (std::size_t j = kStrConvBaseDigits; j > 0; j--) {
                *ptr = static_cast<uint8_t>('0' + a_i % 10);
                a_i /= 10;
                ptr--;
            }
        }

        do {
            *ptr = static_cast<uint8_t>('0' + last_a_i % 10);
            last_a_i /= 10;
            ptr--;
        } while (last_a_i);
    }
    [[nodiscard]] friend std::string to_string(const longint& n) {
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
        const auto checked_capacity = check_size(requested_capacity);
        if (checked_capacity > capacity_) {
            digit_t* new_nums = allocate(checked_capacity);
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
        dec_digit_t* digits_ = nullptr;
        std::size_t size_    = 0;

        constexpr Decimal() noexcept = default;
        explicit Decimal(uint32_t n) : digits_(allocate(2)) {
            assign_uint32_unchecked(n);
        }
        explicit Decimal(uint64_t n) : digits_(allocate(3)) {
            assign_uint64_unchecked(n);
        }
        Decimal(const Decimal& other) : digits_(nullptr), size_(other.size_) {
            if (size_) {
                digits_ = allocate(size_);
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
            if (size_ < 2) {
                deallocate(digits_);
                digits_ = allocate(2);
            }
            assign_uint32_unchecked(n);
            return *this;
        }

        Decimal& operator=(uint64_t n) {
            if (size_ < 3) {
                deallocate(digits_);
                digits_ = allocate(3);
            }
            assign_uint64_unchecked(n);
            return *this;
        }

        Decimal& operator+=(const Decimal& other) {
            uint64_t carry      = 0;
            const std::size_t m = std::min(this->size_, other.size_);
            uint32_t* p         = this->digits_;
            for (std::size_t i = 0; i < m; i++) {
                const uint64_t res =
                    static_cast<uint64_t>(p[i]) + static_cast<uint64_t>(other.digits_[i]) + carry;
                p[i]  = static_cast<dec_digit_t>(res % kDecimalBase);
                carry = res / kDecimalBase;
            }

            if (size_ < other.size_) {
                digit_t* new_digits = allocate(other.size_);
                std::copy_n(other.digits_ + size_, other.size_ - size_,
                            std::copy_n(digits_, size_, new_digits));
                deallocate(this->digits_);
                this->digits_ = new_digits;
                size_         = other.size_;
            }

            p                           = this->digits_;
            const std::size_t this_size = size_;
            for (std::size_t i = m; carry != 0 && i < this_size; i++) {
                const auto res = static_cast<uint64_t>(p[i]) + carry;
                p[i]           = static_cast<dec_digit_t>(res % kDecimalBase);
                carry          = res / kDecimalBase;
            }

            if (carry == 0) {
                pop_leading_zeros();
            } else {
                dec_digit_t* new_digits     = allocate((this_size + 1 + (this_size == 0)));
                pointer new_digits_copy_end = std::copy_n(digits_, this_size, new_digits);
                *new_digits_copy_end        = static_cast<dec_digit_t>(carry);
                deallocate(this->digits_);
                this->digits_ = new_digits;
                size_         = this_size + 1;
            }

            return *this;
        }

        Decimal& operator*=(const Decimal& other) {
            std::size_t k            = size_;
            std::size_t m            = other.size_;
            const dec_digit_t* k_ptr = digits_;
            const dec_digit_t* m_ptr = other.digits_;

            if (k < m) {
                std::swap(k_ptr, m_ptr);
                std::swap(k, m);
            }

            if (unlikely(m == 0)) {
                set_zero();
                return *this;
            }
            ATTRIBUTE_ASSUME(1 <= m && m <= k);
            const std::size_t new_size = m + k;
            if (m <= 16 || m * k <= 1024) {
                naive_multiply_and_store_to(m_ptr, m, k_ptr, k, *this);
            } else {
                const DecFFTProduct prod(m_ptr, m, k_ptr, k);
                prod.multiply_and_store_to(*this);
            }

            size_ = new_size;
            this->pop_leading_zeros();
            return *this;
        }

        void SquareThisTo(Decimal& other) const {
            const std::size_t digits_size = size_;
            if (unlikely(digits_size == 0)) {
                other.set_zero();
                return;
            }

            if (digits_size <= 16 || digits_size * digits_size <= 1024) {
                naive_multiply_and_store_to(digits_, digits_size, other);
            } else {
                const DecFFTProduct prod(digits_, digits_size);
                prod.multiply_and_store_to(other);
            }

            other.pop_leading_zeros();
        }

        ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
        ATTRIBUTE_NONNULL_ALL_ARGS
        static void naive_multiply_and_store_to(const dec_digit_t digits[], std::size_t digits_size,
                                                Decimal& product_result) {
            naive_multiply_and_store_to(digits, digits_size, digits, digits_size, product_result);
        }
        ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
        ATTRIBUTE_SIZED_ACCESS(read_only, 3, 4)
        ATTRIBUTE_NONNULL_ALL_ARGS
        static void naive_multiply_and_store_to(const dec_digit_t m_digits[], std::size_t m,
                                                const dec_digit_t k_digits[], std::size_t k,
                                                Decimal& product_result) {
            const std::size_t new_size = m + k;
            dec_digit_t* ans           = allocate(new_size);
            std::fill_n(ans, new_size, dec_digit_t{0});
            for (std::size_t j = 0; j < m; j++) {
                const uint64_t b_j = m_digits[j];
                uint64_t carry     = 0;
                for (std::size_t i = 0; i < k; i++) {
                    uint64_t a_i = k_digits[i];
                    uint64_t res = a_i * b_j + static_cast<uint64_t>(ans[j + i]) + carry;
                    ans[j + i]   = static_cast<dec_digit_t>(res % kDecimalBase);
                    carry        = res / kDecimalBase;
                }
                ans[j + k] = static_cast<dec_digit_t>(carry % kDecimalBase);
            }

            deallocate(product_result.digits_);
            product_result.digits_ = ans;
            product_result.size_   = new_size;
        }

        class [[nodiscard]] DecFFTProduct final {
        public:
            ATTRIBUTE_SIZED_ACCESS(read_only, 2, 3)
            ATTRIBUTE_SIZED_ACCESS(read_only, 4, 5)
            ATTRIBUTE_NONNULL_ALL_ARGS
            DecFFTProduct(const dec_digit_t m_digits[], std::size_t m, const dec_digit_t k_digits[],
                          std::size_t k)
                : product_size_(check_size(m + k)),
                  poly_size_(polys_size(product_size_)),
                  poly_(create_and_fill_polynomials(m_digits, m, k_digits, k, poly_size_)) {}
            ATTRIBUTE_SIZED_ACCESS(read_only, 2, 3)
            ATTRIBUTE_NONNULL_ALL_ARGS
            DecFFTProduct(const dec_digit_t digits[], std::size_t digits_size)
                : product_size_(check_size(digits_size + digits_size)),
                  poly_size_(polys_size(product_size_)),
                  poly_(create_and_fill_polynomials(digits, digits_size, poly_size_)) {}
            DecFFTProduct(const DecFFTProduct&)            = delete;
            DecFFTProduct(DecFFTProduct&&)                 = delete;
            DecFFTProduct& operator=(const DecFFTProduct&) = delete;
            DecFFTProduct& operator=(DecFFTProduct&&)      = delete;
            ~DecFFTProduct() {
                ::operator delete(static_cast<void*>(poly_));
            }
            void multiply_and_store_to(Decimal& product_result) const noexcept {
                if (product_size_ > product_result.size_) {
                    static_assert(sizeof(dec_digit_t) == sizeof(digit_t));
                    deallocate(product_result.digits_);
                    product_result.digits_ = allocate(product_size_);
                }
                fft::forward_backward_fft(lhs_poly(), rhs_poly(), poly_size());
                convert_fft_poly_to_decimal_digits(rhs_poly(), product_result.digits_,
                                                   product_size_);
                product_result.size_ = product_size_;
            }

        private:
            // clang-format off

            static_assert(kFFTDecimalBase * kFFTDecimalBase * kFFTDecimalBase == kDecimalBase, "");

            [[nodiscard]]
            ATTRIBUTE_CONST
            ATTRIBUTE_ALWAYS_INLINE
            static constexpr std::size_t polys_size(std::size_t size) noexcept {
                return math_functions::nearest_greater_equal_power_of_two(
                    3 * static_cast<uint64_t>(size));
            }
            [[nodiscard]]
            ATTRIBUTE_RETURNS_NONNULL
            ATTRIBUTE_ALWAYS_INLINE
            static fft::complex* allocate_polynomials(std::size_t size) {
                // Allocate n for the first polynomial
                //  and n for the second one
                return static_cast<fft::complex*>(::operator new(size * 2 * sizeof(fft::complex)));
            }
            ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
            ATTRIBUTE_SIZED_ACCESS(read_only, 3, 4)
            ATTRIBUTE_SIZED_ACCESS(write_only, 6, 5)
            ATTRIBUTE_NONNULL_ALL_ARGS
            static void fill_polynomial(const dec_digit_t m_digits[], std::size_t m,
                                        const dec_digit_t k_digits[], std::size_t k, std::size_t n,
                                        fft::complex* RESTRICT_QUALIFIER p) noexcept {
                for (std::size_t i = 0; i < m; i++) {
                    dec_digit_t m_value = m_digits[i];
                    dec_digit_t k_value = k_digits[i];
                    uint32_t r1         = m_value % kFFTDecimalBase;
                    m_value /= kFFTDecimalBase;
                    uint32_t r2 = k_value % kFFTDecimalBase;
                    k_value /= kFFTDecimalBase;
                    *p = fft::complex(r1, r2);
                    p++;
                    r1 = m_value % kFFTDecimalBase;
                    m_value /= kFFTDecimalBase;
                    r2 = k_value % kFFTDecimalBase;
                    k_value /= kFFTDecimalBase;
                    *p = fft::complex(r1, r2);
                    p++;
                    *p = fft::complex(m_value, k_value);
                    p++;
                }
                for (std::size_t i = m; i < k; i++) {
                    dec_digit_t k_value = k_digits[i];
                    uint32_t r2         = k_value % kFFTDecimalBase;
                    k_value /= kFFTDecimalBase;
                    *p = fft::complex(0, r2);
                    p++;
                    r2 = k_value % kFFTDecimalBase;
                    k_value /= kFFTDecimalBase;
                    *p = fft::complex(0, r2);
                    p++;
                    *p = fft::complex(0, k_value);
                    p++;
                }

                std::memset(static_cast<void*>(p), 0, (n - 3 * k) * sizeof(fft::complex));
            }
            ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
            ATTRIBUTE_SIZED_ACCESS(write_only, 4, 3)
            ATTRIBUTE_NONNULL_ALL_ARGS
            static void fill_polynomial(const dec_digit_t digits[], std::size_t digits_size,
                                        std::size_t n, fft::complex* RESTRICT_QUALIFIER p) noexcept {
                for (std::size_t i = 0; i < digits_size; i++) {
                    uint32_t value = digits[i];
                    uint32_t r1    = value % kFFTDecimalBase;
                    value /= kFFTDecimalBase;
                    *p = fft::complex(r1, r1);
                    p++;
                    r1 = value % kFFTDecimalBase;
                    value /= kFFTDecimalBase;
                    *p = fft::complex(r1, r1);
                    p++;
                    *p = fft::complex(value, value);
                    p++;
                }

                std::memset(static_cast<void*>(p), 0, (n - 3 * digits_size) * sizeof(fft::complex));
            }
            [[nodiscard]]
            ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
            ATTRIBUTE_SIZED_ACCESS(read_only, 3, 4)
            ATTRIBUTE_NONNULL_ALL_ARGS
            ATTRIBUTE_RETURNS_NONNULL
            ATTRIBUTE_ALWAYS_INLINE
            static fft::complex* create_and_fill_polynomials(const dec_digit_t m_digits[],
                                                             std::size_t m,
                                                             const dec_digit_t k_digits[],
                                                             std::size_t k, std::size_t n) {
                fft::complex* const polys = allocate_polynomials(n);
                assert(polys != nullptr);
                fill_polynomial(m_digits, m, k_digits, k, n, polys);
                return polys;
            }
            [[nodiscard]]
            ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2) 
            ATTRIBUTE_NONNULL_ALL_ARGS
            ATTRIBUTE_RETURNS_NONNULL
            ATTRIBUTE_ALWAYS_INLINE
            static fft::complex* create_and_fill_polynomials(const dec_digit_t digits[],
                                                             std::size_t digits_size,
                                                             std::size_t n) {
                fft::complex* const polys = allocate_polynomials(n);
                assert(polys != nullptr);
                fill_polynomial(digits, digits_size, n, polys);
                return polys;
            }
            ATTRIBUTE_ACCESS(read_only, 1)
            ATTRIBUTE_SIZED_ACCESS(write_only, 2, 3)
            ATTRIBUTE_NONNULL_ALL_ARGS
            static void convert_fft_poly_to_decimal_digits(const fft::complex poly[],
                                                           dec_digit_t digits[],
                                                           std::size_t digits_size) noexcept {
                static_assert(sizeof(digit_t) == sizeof(uint32_t));
                uint64_t carry                      = 0;
                const dec_digit_t* const digits_end = digits + digits_size;
                do {
                    uint64_t res = carry;
                    res += static_cast<uint64_t>(poly->real() + kFFTFloatRoundError);
                    poly++;
                    res +=
                        static_cast<uint64_t>(poly->real() + kFFTFloatRoundError) * kFFTDecimalBase;
                    poly++;
                    res += static_cast<uint64_t>(poly->real() + kFFTFloatRoundError) *
                           (kFFTDecimalBase * kFFTDecimalBase);
                    poly++;
                    *digits = static_cast<dec_digit_t>(res % kDecimalBase);
                    carry   = res / kDecimalBase;
                    digits++;
                } while (digits != digits_end);
                assert(carry == 0);
            }
            [[nodiscard]]
            ATTRIBUTE_PURE
            ATTRIBUTE_RETURNS_NONNULL
            ATTRIBUTE_ALWAYS_INLINE
            constexpr fft::complex* lhs_poly() const noexcept ATTRIBUTE_LIFETIME_BOUND {
                return poly_;
            }
            [[nodiscard]]
            ATTRIBUTE_PURE
            ATTRIBUTE_RETURNS_NONNULL
            ATTRIBUTE_ALWAYS_INLINE
            constexpr fft::complex* rhs_poly() const noexcept ATTRIBUTE_LIFETIME_BOUND {
                return poly_ + poly_size();
            }
            [[nodiscard]]
            ATTRIBUTE_PURE
            ATTRIBUTE_ALWAYS_INLINE
            constexpr std::size_t poly_size() const noexcept {
                const auto value = poly_size_;
                ATTRIBUTE_ASSUME(math_functions::is_power_of_two(value));
                return value;
            }

            // clang-format on

            std::size_t const product_size_;
            std::size_t const poly_size_;
            fft::complex* const poly_;
        };

        [[nodiscard]]
        ATTRIBUTE_PURE constexpr bool operator==(uint32_t n) const noexcept {
            switch (size_) {
                case 0:
                    return n == 0;
                case 1:
                    return digits_[0] == n;
                case 2:
                    return static_cast<uint64_t>(digits_[1]) * kDecimalBase + digits_[0] == n;
                default:
                    return false;
            }
        }

        [[nodiscard]]
        ATTRIBUTE_PURE constexpr bool operator!=(uint32_t n) const noexcept {
            return !(*this == n);
        }

        [[nodiscard]]
        ATTRIBUTE_PURE constexpr bool operator==(uint64_t n) const noexcept {
            switch (size_) {
                case 0:
                    return n == 0;
                case 1:
                    return digits_[0] == n;
                case 2:
                    return static_cast<uint64_t>(digits_[1]) * kDecimalBase + digits_[0] == n;
                case 3: {
                    constexpr uint64_t kDecimalBase2 =
                        static_cast<uint64_t>(kDecimalBase) * kDecimalBase;
                    const uint64_t hi = digits_[2];
                    if (hi > 18) {
                        return false;
                    }

                    const uint64_t low_mid_m =
                        static_cast<uint64_t>(digits_[1]) * kDecimalBase + digits_[0];
                    if (hi == 18) {
                        return n >= 18 * kDecimalBase2 && n - 18 * kDecimalBase2 == low_mid_m;
                    }
                    const uint64_t m = static_cast<uint64_t>(hi) * kDecimalBase2 + low_mid_m;
                    return m == n;
                }
                default:
                    return false;
            }
        }

        [[nodiscard]]
        ATTRIBUTE_PURE constexpr bool operator!=(uint64_t n) const noexcept {
            return !(*this == n);
        }

        [[nodiscard]]
        ATTRIBUTE_PURE constexpr bool operator==(const Decimal& other) const noexcept {
            return size_ == other.size_ && std::equal(digits_, digits_ + size_, other.digits_);
        }

        constexpr void set_zero() noexcept {
            size_ = 0;
        }
        constexpr void pop_leading_zeros() noexcept {
            std::size_t usize_value = size_;
            while (usize_value > 0 && digits_[usize_value - 1] == 0) {
                usize_value--;
            }

            size_ = usize_value;
        }

        ~Decimal() {
            deallocate(digits_);
        }

    private:
        constexpr void assign_uint32_unchecked(uint32_t n) noexcept {
            const dec_digit_t low = n % kDecimalBase;
            digits_[0]            = low;
            const dec_digit_t hi  = n / kDecimalBase;
            digits_[1]            = hi;
            size_                 = hi != 0 ? 2 : low != 0;
        }
        constexpr void assign_uint64_unchecked(uint64_t n) noexcept {
            const auto low   = static_cast<dec_digit_t>(n % kDecimalBase);
            const uint64_t t = n / kDecimalBase;
            const auto mid   = static_cast<dec_digit_t>(t % kDecimalBase);
            const auto hi    = static_cast<dec_digit_t>(t / kDecimalBase);
            digits_[0]       = low;
            digits_[1]       = mid;
            digits_[2]       = hi;
            size_            = hi != 0 ? 3 : (mid != 0 ? 2 : low != 0);
        }
    };

private:
    static constexpr bool kUseCustomLongIntAllocator = false;

    [[nodiscard]] constexpr uint32_t divmod_impl(const uint32_t n) noexcept {
        double_digit_t carry = 0;
        for (digit_t *const nums_iter_rend = nums_ - 1, *nums_riter = nums_iter_rend + usize();
             nums_riter != nums_iter_rend; --nums_riter) {
            const double_digit_t cur =
                (carry << kNumsBits) | static_cast<double_digit_t>(*nums_riter);
            const double_digit_t q = cur / n;
            const double_digit_t r = cur - q * n;
            *nums_riter            = static_cast<digit_t>(q);
            carry                  = r;
        }

        pop_leading_zeros();
        return static_cast<uint32_t>(carry);
    }

    void divmod_impl(const longint& other, longint& rem) {
        /**
         * See Hackers Delight 9-2.
         */
        const std::size_t m = usize();
        const std::size_t n = other.usize();
        if (m < n) {
            rem = std::move(*this);
            this->set_zero();
            return;
        }

        const std::int32_t sign_product = size() ^ other.size();
        switch (n) {
            case 0:
                /* Quite return when dividing by zero. */
                return;
            case 1:
                rem   = this->divmod(other[0]);
                size_ = sign_product >= 0 ? size_ : -size_;
                return;
            default:
                break;
        }

        rem.reserveUninitializedWithoutCopy(static_cast<std::uint32_t>(n));
        rem.size_ = static_cast<std::int32_t>(n);

        // Normilize by shifting v left just enough so that
        // its high-order bit i on, and shift u left the
        // same amount. We may have to append a high-order
        // digit on the dividend; we do that unconditionally (un size = m + -> 1 <-).

        digit_t* const vn_and_un = allocate(n + m + 1);
        digit_t* const vn        = vn_and_un;
        digit_t* const un        = vn_and_un + n;

        const digit_t* const u   = nums_;
        const digit_t* const v   = other.nums_;
        const digit_t last_v_num = v[n - 1];
        assert(last_v_num > 0);
        ATTRIBUTE_ASSUME(last_v_num > 0);
        static_assert(kNumsBits == 32);
        // 0 <= s < kNumsBits
        const auto s = static_cast<std::uint32_t>(math_functions::countl_zero(last_v_num));
        longint::divmod_normalize_vn(vn, v, n, s);
        assert(vn[n - 1] >= digit_t{1} << (kNumsBits - 1));
        ATTRIBUTE_ASSUME(vn[n - 1] >= digit_t{1} << (kNumsBits - 1));
        longint::divmod_normalize_un(un, u, m, m + 1, s);
        longint::divmod_impl_unchecked(
            /* un = */ un,
            /* un_size = */ m + 1,
            /* vn = */ vn,
            /* vn_size = */ n,
            /* quot = */ nums_);
        // Unnormalize remainder
        longint::divmod_unnormalize_remainder(rem.nums_, un, n, n + 1, s);
        deallocate(vn_and_un);
        rem.pop_leading_zeros();
        auto signed_size = static_cast<std::int32_t>(m - n + 1);
        size_            = sign_product >= 0 ? signed_size : -signed_size;
        pop_leading_zeros();
    }

    ATTRIBUTE_SIZED_ACCESS(read_write, 1, 2)
    ATTRIBUTE_SIZED_ACCESS(read_only, 3, 4)
    ATTRIBUTE_ACCESS(write_only, 5)
    ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ALWAYS_INLINE
    static constexpr void divmod_impl_unchecked(digit_t* RESTRICT_QUALIFIER const un,
                                                const std::size_t un_size,
                                                const digit_t* RESTRICT_QUALIFIER const vn,
                                                const std::size_t vn_size,
                                                digit_t* RESTRICT_QUALIFIER const quot) noexcept {
        ATTRIBUTE_ASSUME(vn_size >= 2);
        ATTRIBUTE_ASSUME(un_size > vn_size);
        for (std::size_t j = un_size - vn_size - 1; static_cast<std::ptrdiff_t>(j) >= 0; j--) {
            // Compute estimate qhat of q[j].
            const double_digit_t cur =
                (double_digit_t{un[j + vn_size]} << kNumsBits) | un[j + vn_size - 1];
            const digit_t last_vn = vn[vn_size - 1];
            ATTRIBUTE_ASSUME(last_vn >= digit_t{1} << (kNumsBits - 1));
            double_digit_t qhat = cur / last_vn;
            double_digit_t rhat = cur % last_vn;
            ATTRIBUTE_ASSUME(qhat * last_vn + rhat == cur);
            while (qhat >= kNumsBase ||
                   qhat * vn[vn_size - 2] > kNumsBase * rhat + un[j + vn_size - 2]) {
                qhat--;
                rhat += last_vn;
                ATTRIBUTE_ASSUME(qhat * last_vn + rhat == cur);
                if (rhat >= kNumsBase) {
                    break;
                }
            }
            ATTRIBUTE_ASSUME(qhat * last_vn + rhat == cur);
            // Multiply and subtract
            double_digit_t t = divmod_mult_sub(un + j, vn, vn_size, qhat);
            quot[j]          = static_cast<digit_t>(qhat);  // Store quotient digit
            if (static_cast<std::make_signed_t<double_digit_t>>(t) < 0) {
                assert(qhat > 0);
                // If we subtracted too much, add back
                quot[j]--;
                divmod_add_back(un + j, vn, vn_size);
            }
        }
    }

    /// @brief
    ///  u := un[0] + un[1] * kNumsBase + ... + un[vn_size] * kNumsBase^{vn_size}
    ///  v := vn[0] + vn[1] * kNumsBase + ... + vn[vn_size - 1] * kNumsBase^{vn_size - 1}
    ///  u -= q * v
    /// @param un
    /// @param vn
    /// @param vn_size
    /// @param qhat
    /// @return
    ATTRIBUTE_ACCESS(read_write, 1)
    ATTRIBUTE_SIZED_ACCESS(read_only, 2, 3)
    ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ALWAYS_INLINE
    static constexpr double_digit_t divmod_mult_sub(digit_t* RESTRICT_QUALIFIER const un,
                                                    const digit_t* RESTRICT_QUALIFIER const vn,
                                                    const std::size_t vn_size,
                                                    const double_digit_t qhat) noexcept {
        ATTRIBUTE_ASSUME(vn_size >= 2);
        double_digit_t carry = 0;
        for (std::size_t i = 0; i < vn_size; i++) {
            double_digit_t p = qhat * vn[i];
            double_digit_t t = double_digit_t{un[i]} - carry - static_cast<digit_t>(p % kNumsBase);
            un[i]            = static_cast<digit_t>(t % kNumsBase);
            carry            = static_cast<digit_t>((p / kNumsBase) - (t / kNumsBase));
        }
        double_digit_t t = un[vn_size] - carry;
        un[vn_size]      = static_cast<digit_t>(t);
        return t;
    }

    ATTRIBUTE_ACCESS(read_write, 1)
    ATTRIBUTE_SIZED_ACCESS(read_only, 2, 3)
    ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ALWAYS_INLINE
    static constexpr void divmod_add_back(digit_t* RESTRICT_QUALIFIER const un,
                                          const digit_t* RESTRICT_QUALIFIER const vn,
                                          const std::size_t vn_size) noexcept {
        ATTRIBUTE_ASSUME(vn_size >= 2);
        double_digit_t carry = 0;
        for (std::size_t i = 0; i < vn_size; i++) {
            double_digit_t t = double_digit_t{un[i]} + double_digit_t{vn[i]} + carry;
            un[i]            = static_cast<digit_t>(t % kNumsBase);
            carry            = t / kNumsBase;
        }
        un[vn_size] += static_cast<digit_t>(carry);
    }

    ATTRIBUTE_SIZED_ACCESS(write_only, 1, 3)
    ATTRIBUTE_SIZED_ACCESS(read_only, 2, 3)
    ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ALWAYS_INLINE
    static constexpr void divmod_normalize_vn(digit_t vn[], const digit_t v[], std::size_t n,
                                              std::uint32_t s) noexcept {
        ATTRIBUTE_ASSUME(n > 1);
        ATTRIBUTE_ASSUME(s < 32);
        for (std::size_t i = n - 1; i > 0; i--) {
            vn[i] = (v[i] << s) | static_cast<digit_t>(double_digit_t{v[i - 1]} >> (kNumsBits - s));
        }
        vn[0] = v[0] << s;
    }

    ATTRIBUTE_SIZED_ACCESS(write_only, 1, 4)
    ATTRIBUTE_SIZED_ACCESS(read_only, 2, 3)
    ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ALWAYS_INLINE
    static constexpr void divmod_normalize_un(digit_t un[], const digit_t u[], std::size_t m,
                                              ATTRIBUTE_MAYBE_UNUSED std::size_t m_plus_one,
                                              std::uint32_t s) noexcept {
        ATTRIBUTE_ASSUME(m > 1);
        ATTRIBUTE_ASSUME(s < 32);
        ATTRIBUTE_ASSUME(m + 1 == m_plus_one);
        un[m] = static_cast<digit_t>(double_digit_t{u[m - 1]} >> (kNumsBits - s));
        for (std::size_t i = m - 1; i > 0; i--) {
            un[i] = (u[i] << s) | static_cast<digit_t>(double_digit_t{u[i - 1]} >> (kNumsBits - s));
        }
        un[0] = u[0] << s;
    }

    ATTRIBUTE_SIZED_ACCESS(write_only, 1, 3)
    ATTRIBUTE_SIZED_ACCESS(read_only, 2, 4)
    ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ALWAYS_INLINE
    static constexpr void divmod_unnormalize_remainder(
        digit_t rem[], const digit_t un[], std::size_t n,
        ATTRIBUTE_MAYBE_UNUSED std::size_t n_plus_one, std::uint32_t s) noexcept {
        ATTRIBUTE_ASSUME(n > 1);
        ATTRIBUTE_ASSUME(s < 32);
        ATTRIBUTE_ASSUME(n + 1 == n_plus_one);
        for (std::size_t i = 0; i < n; i++) {
            rem[i] =
                (un[i] >> s) | static_cast<digit_t>(double_digit_t{un[i + 1]} << (kNumsBits - s));
        }
    }

    constexpr void set_ssize_from_size(size_type size) noexcept {
        size_ = static_cast<ssize_type>(size_ >= 0 ? size : -size);
    }

    constexpr void pop_leading_zeros() noexcept {
        auto usize_value = usize32();
        while (usize_value > 0 && nums_[usize_value - 1] == 0) {
            usize_value--;
        }

        set_ssize_from_size(usize_value);
    }

    void reserveUninitializedWithoutCopy(const uint32_t capacity) {
        if (capacity > capacity_) {
            deallocate(nums_);
            nums_     = allocate(capacity);
            capacity_ = capacity;
        }
        size_ = 0;
    }

    static void convert_fft_poly_to_longint_nums(bool is_high_precision, fft::complex* poly,
                                                 digit_t* nums, std::size_t nums_size) {
        static_assert(sizeof(digit_t) == sizeof(uint32_t));
        double_digit_t carry    = 0;
        const digit_t* nums_end = nums + nums_size;
        if (likely(!is_high_precision)) {
            do {
                double_digit_t res = carry;
                res += static_cast<double_digit_t>(poly->real() + kFFTFloatRoundError);
                poly++;
                res += static_cast<double_digit_t>(poly->real() + kFFTFloatRoundError) << 16;
                poly++;
                *nums = static_cast<digit_t>(res);
                carry = res / kNumsBase;
                nums++;
            } while (nums != nums_end);
        } else {
            do {
                double_digit_t res = carry;
                res += static_cast<double_digit_t>(poly->real() + kFFTFloatRoundError);
                poly++;
                res += static_cast<double_digit_t>(poly->real() + kFFTFloatRoundError) << 8;
                poly++;
                res += static_cast<double_digit_t>(poly->real() + kFFTFloatRoundError) << 16;
                poly++;
                res += static_cast<double_digit_t>(poly->real() + kFFTFloatRoundError) << 24;
                poly++;
                *nums = static_cast<digit_t>(res);
                carry = res / kNumsBase;
                nums++;
            } while (nums != nums_end);
        }
        assert(carry == 0);
    }

    static std::vector<Decimal> create_initial_conv_bin_base_pows() {
        std::vector<Decimal> pows;
        auto local_copy{longint::kNumsBase};
        pows.emplace_back(local_copy);
        return pows;
    }

    static inline std::vector<Decimal> conv_bin_base_pows =
        longint::create_initial_conv_bin_base_pows();

    friend struct longint_detail::longint_static_storage;

    static void convertDecBaseMultAdd(digit_t conv_digits[], std::size_t half_len,
                                      const longint* conv_base_pow, digit_t mult_add_buffer[],
                                      fft::complex fft_poly_buffer[]) {
        const digit_t* num_hi = conv_digits + half_len;
        std::size_t m         = static_cast<uint32_t>(conv_base_pow->size_);
        assert(m > 0 && m <= half_len);
        const uint32_t* m_ptr = conv_base_pow->nums_;
        std::size_t prod_size = m + half_len;
        std::fill_n(mult_add_buffer, half_len + half_len, digit_t{0});
        if (half_len <= 32) {
            for (std::size_t j = 0; j < m; j++) {
                const double_digit_t b_j = m_ptr[j];
                double_digit_t carry     = 0;
                for (std::size_t i = 0; i < half_len; i++) {
                    const double_digit_t a_i = num_hi[i];
                    const double_digit_t res =
                        a_i * b_j + double_digit_t{mult_add_buffer[j + i]} + carry;
                    mult_add_buffer[j + i] = static_cast<digit_t>(res);
                    carry                  = res >> kNumsBits;
                }

                mult_add_buffer[j + half_len] = static_cast<digit_t>(carry);
            }
        } else {
            std::size_t n = 2 * math_functions::nearest_greater_equal_power_of_two(
                                    static_cast<uint64_t>(prod_size));
            const bool high_precision = n > kFFTPrecisionBorder;
            n <<= high_precision;
            fft::complex* p1 = fft_poly_buffer;
            fft::complex* p  = p1;
            if (likely(!high_precision)) {
                for (std::size_t i = 0; i < m; i++) {
                    digit_t m_value = m_ptr[i];
                    digit_t k_value = num_hi[i];
                    *p              = fft::complex(m_value & 0xFFFF, k_value & 0xFFFF);
                    p++;
                    *p = fft::complex(m_value >> 16, k_value >> 16);
                    p++;
                }
                for (std::size_t i = m; i < half_len; i++) {
                    digit_t k_value = num_hi[i];
                    *p              = fft::complex(0, k_value & 0xFFFF);
                    p++;
                    *p = fft::complex(0, k_value >> 16);
                    p++;
                }
            } else {
                for (std::size_t i = 0; i < m; i++) {
                    uint32_t m_value = m_ptr[i];
                    uint32_t k_value = num_hi[i];
                    *p = fft::complex(static_cast<uint8_t>(m_value), static_cast<uint8_t>(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = fft::complex(static_cast<uint8_t>(m_value), static_cast<uint8_t>(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = fft::complex(static_cast<uint8_t>(m_value), static_cast<uint8_t>(k_value));
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = fft::complex(m_value, k_value);
                    p++;
                }
                for (std::size_t i = m; i < half_len; i++) {
                    uint32_t k_value = num_hi[i];
                    *p               = fft::complex(0, static_cast<uint8_t>(k_value));
                    p++;
                    k_value >>= 8;
                    *p = fft::complex(0, static_cast<uint8_t>(k_value));
                    p++;
                    k_value >>= 8;
                    *p = fft::complex(0, static_cast<uint8_t>(k_value));
                    p++;
                    k_value >>= 8;
                    *p = fft::complex(0, k_value);
                    p++;
                }
            }

            std::memset(static_cast<void*>(p), 0,
                        (n - ((2 * half_len) << high_precision)) * sizeof(fft::complex));

            fft::complex* const p2 = p1 + n;
            fft::forward_backward_fft(p1, p2, n);
            convert_fft_poly_to_longint_nums(high_precision, p2, mult_add_buffer, prod_size);
        }

        // Now mult_add_buffer == num_hi * CONV_BASE^half_len
        double_digit_t carry = 0;
        for (std::size_t i = half_len; i > 0; i--, conv_digits++, mult_add_buffer++) {
            const double_digit_t res =
                double_digit_t{*conv_digits} + double_digit_t{*mult_add_buffer} + carry;
            *conv_digits = static_cast<digit_t>(res);
            carry        = res >> kNumsBits;
        }
        for (std::size_t i = half_len; i > 0; i--, conv_digits++, mult_add_buffer++) {
            const double_digit_t res = double_digit_t{*mult_add_buffer} + carry;
            *conv_digits             = static_cast<digit_t>(res);
            carry                    = res >> kNumsBits;
        }
        assert(carry == 0);
    }

    static Decimal convertBinBase(const digit_t* nums, std::size_t size) {
        assert(math_functions::is_power_of_two(size));
        ATTRIBUTE_ASSUME(math_functions::is_power_of_two(size));
        switch (size) {
            case 0:
            case 1:
                return Decimal(nums[0]);
            case 2:
                return Decimal(double_digit_t{nums[1]} * kNumsBase | nums[0]);
            default:
                break;
        }

        Decimal low_dec  = convertBinBase(nums, size / 2);
        Decimal high_dec = convertBinBase(nums + size / 2, size / 2);

        const std::size_t idx = math_functions::log2_floor(static_cast<uint64_t>(size)) - 1;
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
        reserve(static_cast<std::size_t>(capacity_) * 2 | (capacity_ == 0));
    }

    ATTRIBUTE_NOINLINE ATTRIBUTE_COLD std::size_t growSizeByOne() {
        std::size_t usize_value = usize();
        if (unlikely(usize_value == capacity_)) {
            growCapacity();
        }

        size_ += sign();
        return usize_value + 1;
    }

    size_type set_size_at_least(size_type new_size) {
        size_type cur_size = usize32();
        if (new_size <= cur_size) {
            return cur_size;
        }

        reserve(new_size);
        std::fill_n(std::addressof(nums_[cur_size]), new_size - cur_size, digit_t{0});
        set_ssize_from_size(new_size);
        return new_size;
    }

    void allocate_default_capacity_32() {
        nums_     = allocate(kDefaultLINumsCapacity32);
        capacity_ = kDefaultLINumsCapacity32;
    }
    void ensure_default_capacity_op_asgn_32() {
        if (capacity_ < kDefaultLINumsCapacity32) {
            deallocate(nums_);
            allocate_default_capacity_32();
        }
    }
    constexpr void assign_u32_unchecked(uint32_t n) noexcept {
        static_assert(kNumsBits >= 32);
        size_    = n != 0;
        nums_[0] = n;
    }
    constexpr void assign_i32_unchecked(int32_t n) noexcept {
        static_assert(kNumsBits >= 32);
        size_    = ::math_functions::sign(n);
        nums_[0] = ::math_functions::uabs(n);
    }
    void allocate_default_capacity_64() {
        nums_     = allocate(kDefaultLINumsCapacity64);
        capacity_ = kDefaultLINumsCapacity64;
    }
    void ensure_default_capacity_op_asgn_64() {
        if (capacity_ < kDefaultLINumsCapacity64) {
            deallocate(nums_);
            allocate_default_capacity_64();
        }
    }
    constexpr void assign_u64_unchecked(uint64_t n) noexcept {
        static_assert(kNumsBits == 32);
        size_    = n != 0;
        nums_[0] = static_cast<uint32_t>(n);
        n >>= 32;
        size_ += n != 0;
        nums_[1] = static_cast<uint32_t>(n);
    }
    constexpr void assign_i64_unchecked(int64_t n) noexcept {
        const std::int32_t sgn = ::math_functions::sign(n);
        this->assign_u64_unchecked(::math_functions::uabs(n));
        size_ *= sgn;
    }
#if defined(INTEGERS_128_BIT_HPP)
    void allocate_default_capacity_128() {
        nums_     = allocate(kDefaultLINumsCapacity128);
        capacity_ = kDefaultLINumsCapacity128;
    }
    void ensure_default_capacity_op_asgn_128() {
        if (capacity_ < kDefaultLINumsCapacity128) {
            deallocate(nums_);
            allocate_default_capacity_128();
        }
    }
    constexpr void assign_u128_unchecked(uint128_t n) noexcept {
        size_    = n != 0;
        nums_[0] = static_cast<uint32_t>(n);
        n >>= 32;
        size_ += n != 0;
        nums_[1] = static_cast<uint32_t>(n);
        n >>= 32;
        size_ += n != 0;
        nums_[2] = static_cast<uint32_t>(n);
        n >>= 32;
        size_ += n != 0;
        nums_[3] = static_cast<uint32_t>(n);
    }
    constexpr void assign_i128_unchecked(int128_t n) noexcept {
        const std::int32_t sgn = ::math_functions::sign(n);
        assign_u128_unchecked(::math_functions::uabs(n));
        size_ *= sgn;
    }
#endif

    ATTRIBUTE_SIZED_ACCESS(read_write, 1, 2)
    ATTRIBUTE_SIZED_ACCESS(read_only, 3, 4)
    ATTRIBUTE_NONNULL_ALL_ARGS
    static constexpr void longint_add_with_free_space(digit_t lhs[], size_type lhs_size,
                                                      const digit_t rhs[],
                                                      size_type rhs_size) noexcept {
        ATTRIBUTE_ASSUME(lhs_size > rhs_size);
        double_digit_t carry         = 0;
        const digit_t* const lhs_end = lhs + lhs_size;
        const digit_t* const rhs_end = rhs + rhs_size;
        for (; rhs != rhs_end; ++rhs, ++lhs) {
            const double_digit_t res = double_digit_t{*lhs} + double_digit_t{*rhs} + carry;
            *lhs                     = static_cast<digit_t>(res);
            carry                    = res / kNumsBase;
        }

        for (; carry != 0; ++lhs) {
            assert(lhs != lhs_end);
            const double_digit_t res = double_digit_t{*lhs} + carry;
            *lhs                     = static_cast<digit_t>(res);
            carry                    = res / kNumsBase;
        }
    }

    ATTRIBUTE_SIZED_ACCESS(read_write, 1, 2)
    ATTRIBUTE_SIZED_ACCESS(read_only, 3, 4)
    ATTRIBUTE_NONNULL_ALL_ARGS
    static constexpr bool longint_subtract_with_free_space(digit_t lhs[], size_type lhs_size,
                                                           const digit_t rhs[],
                                                           size_type rhs_size) noexcept {
        ATTRIBUTE_ASSUME(lhs_size >= rhs_size);
        bool overflowed = longint_subtract_with_carry(lhs, lhs_size, rhs, rhs_size);
        if (overflowed) {
            lhs[0] = -lhs[0];
            for (size_type i = 1; i < lhs_size; i++) {
                lhs[i] = -(lhs[i] + 1);
            }
        }
        return overflowed;
    }

    ATTRIBUTE_SIZED_ACCESS(read_write, 1, 2)
    ATTRIBUTE_SIZED_ACCESS(read_only, 3, 4)
    ATTRIBUTE_NONNULL_ALL_ARGS
    static constexpr bool longint_subtract_with_carry(digit_t lhs[], size_type lhs_size,
                                                      const digit_t rhs[],
                                                      size_type rhs_size) noexcept {
        ATTRIBUTE_ASSUME(lhs_size >= rhs_size);
        const digit_t* const lhs_end = lhs + lhs_size;
        const digit_t* const rhs_end = rhs + rhs_size;
        bool carry                   = false;
        for (; rhs != rhs_end; ++rhs, ++lhs) {
            const digit_t lhs_val = *lhs;
            const auto sub_val    = double_digit_t{*rhs} + double_digit_t{carry};
            const digit_t res_val = lhs_val - static_cast<digit_t>(sub_val);
            carry                 = lhs_val < sub_val;
            *lhs                  = res_val;
        }
        if (carry) {
            for (; lhs != lhs_end; ++lhs) {
                const digit_t lhs_val = *lhs;
                *lhs                  = lhs_val - 1;
                if (lhs_val > 0) {
                    return false;
                }
            }
        }

        return carry;
    }

    void nonZeroSizeAddUInt(uint32_t n) {
        digit_t* it             = begin();
        const digit_t* end_iter = end();
        double_digit_t carry    = n;
        do {
            double_digit_t res = double_digit_t{*it} + carry;
            carry              = res >> kNumsBits;
            *it                = static_cast<digit_t>(res);
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
            nums_[usize_value] = static_cast<digit_t>(carry);
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
            if (n == low_num) {
                set_zero();
            }
        } else {
            nums_iter[0] = n - low_num;
            change_sign();
        }
    }

    ATTRIBUTE_ALWAYS_INLINE static digit_t* allocate(std::size_t nums) {
        if constexpr (kUseCustomLongIntAllocator) {
            return static_cast<digit_t*>(::longint_allocator::Allocate(nums * sizeof(digit_t)));
        } else {
            return static_cast<digit_t*>(::operator new(nums * sizeof(digit_t)));
        }
    }
    ATTRIBUTE_ALWAYS_INLINE static void deallocate(digit_t* nums) noexcept {
        if constexpr (kUseCustomLongIntAllocator) {
            ::longint_allocator::Deallocate(static_cast<void*>(nums));
        } else {
            ::operator delete(static_cast<void*>(nums));
        }
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
    ATTRIBUTE_ALWAYS_INLINE static std::uint32_t check_size(const std::size_t value) {
        if (unlikely(value > max_size())) {
            throw_size_error(value, __FILE__, __LINE__, FUNCTION_MACRO);
        }
        static_assert(max_size() <= std::numeric_limits<std::uint32_t>::max(),
                      "implementation error");
        const auto checked_value = static_cast<std::uint32_t>(value);
        if (checked_value > max_size()) {
            CONFIG_UNREACHABLE();
        }
        ATTRIBUTE_ASSUME(checked_value == value);
        return checked_value;
    }
    [[noreturn]] ATTRIBUTE_NOINLINE ATTRIBUTE_COLD static void throw_size_error(
        std::size_t new_size, const char* file_name, std::uint32_t line,
        const char* function_name) {
        char message[1024]      = {};
        const int bytes_written = std::snprintf(
            message, std::size(message), "%s:%u: size error at %s: %zu > %u = max_size()",
            file_name, line, function_name, new_size, max_size());
        if (unlikely(bytes_written < 0)) {
            constexpr const char kFallbackMessage[] = "size error at ";
            constexpr auto kPrefixSize              = std::size(kFallbackMessage) - 1;
            std::char_traits<char>::copy(message, kFallbackMessage, kPrefixSize);
            const auto fn_name_size = std::char_traits<char>::length(function_name);
            const auto fn_name_copy_size =
                std::min(fn_name_size, std::size(message) - 1 - kPrefixSize);
            std::char_traits<char>::copy(std::addressof(message[kPrefixSize]), function_name,
                                         fn_name_copy_size);
            message[kPrefixSize + fn_name_copy_size] = '\0';
        }

        throw std::length_error(message);
    }

    digit_t* nums_ = nullptr;
    /**
     * size_ < 0 <=> sign = -1; size_ == 0 <=> sign = 0; size > 0 <=> sign = 1
     */
    std::int32_t size_      = 0;
    std::uint32_t capacity_ = 0;
};

namespace longint_detail {

struct longint_static_storage final {
private:
    friend longint;

    static std::vector<longint> create_initial_conv_dec_base_pows() {
        std::vector<longint> pows;
        auto local_copy{longint::kStrConvBase};
        pows.emplace_back(local_copy);
        return pows;
    }

    static inline std::vector<longint> conv_dec_base_pows =
        longint_detail::longint_static_storage::create_initial_conv_dec_base_pows();

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
};

}  // namespace longint_detail

inline void longint::set_string(std::string_view s) {
#if LONG_INT_USE_BIT_CAST
    const auto* str_iter = std::bit_cast<const unsigned char*>(s.begin());
    const auto* str_end  = std::bit_cast<const unsigned char*>(s.end());
#else
    const auto* str_iter = reinterpret_cast<const unsigned char*>(s.begin());
    const auto* str_end  = reinterpret_cast<const unsigned char*>(s.end());
#endif

    std::int32_t sgn        = 1;
    constexpr auto is_digit = [](const std::uint32_t chr) constexpr noexcept {
        return chr - '0' <= '9' - '0';
    };
    while (str_iter != str_end && !is_digit(*str_iter)) {
        sgn = *str_iter == '-' ? -1 : 1;
        ++str_iter;
    }

    while (str_iter != str_end && *str_iter == '0') {
        ++str_iter;
    }

    const auto digits_count = static_cast<std::size_t>(str_end - str_iter);
    if (digits_count <= 19) {
        double_digit_t num = 0;
        for (; str_iter != str_end; ++str_iter) {
            num = num * 10 + double_digit_t{*str_iter} - '0';
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
        math_functions::nearest_greater_equal_power_of_two(
            static_cast<uint64_t>(str_conv_digits_size));
    const auto checked_aligned_str_conv_digits_size = check_size(aligned_str_conv_digits_size);
    reserveUninitializedWithoutCopy(checked_aligned_str_conv_digits_size);
    digit_t* str_conv_digits = nums_;

    {
        digit_t* str_conv_digits_iter = str_conv_digits + str_conv_digits_size;
        std::fill_n(str_conv_digits_iter, aligned_str_conv_digits_size - str_conv_digits_size,
                    digit_t{0});
        if (std::size_t offset = digits_count % kStrConvBaseDigits) {
            digit_t current = 0;
            do {
                current = current * 10 + static_cast<digit_t>(*str_iter) - '0';
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
    longint_detail::longint_static_storage::ensureDecBasePowsCapacity(
        math_functions::log2_floor(checked_aligned_str_conv_digits_size));
    // Allocate m complex numbers for p1 and m complex numbers for p2
    std::size_t max_fft_poly_length = 2 * m;
    auto* const mult_add_buffer =
        static_cast<digit_t*>(operator new(aligned_str_conv_digits_size * sizeof(digit_t) +
                                           max_fft_poly_length * sizeof(fft::complex)));
    auto* const fft_poly_buffer =
        reinterpret_cast<fft::complex*>(mult_add_buffer + aligned_str_conv_digits_size);
    const longint* conv_dec_base_pows_iter =
        longint_detail::longint_static_storage::conv_dec_base_pows.data();
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
    size_ = sgn * static_cast<std::int32_t>(usize_value);
}

#undef LONG_INT_USE_BIT_CAST
