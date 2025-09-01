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
#include <utility>
#include <vector>

#include "../misc/config_macros.hpp"
#include "../misc/join_strings.hpp"
#include "fft.hpp"
#if CONFIG_HAS_INCLUDE("integers_128_bit.hpp")
#include "integers_128_bit.hpp"
#endif
#include "math_functions.hpp"

#if defined(ENABLE_LONGINT_DEBUG_ASSERTS) && ENABLE_LONGINT_DEBUG_ASSERTS
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LONGINT_DEBUG_ASSERT(expr) assert(expr)
#else
#define LONGINT_DEBUG_ASSERT(expr)
#endif

#define LONGINT_ASSERT_ASSUME(expr)    \
    do {                               \
        LONGINT_DEBUG_ASSERT(expr);    \
        CONFIG_ASSUME_STATEMENT(expr); \
    } while (false)

#define CONCAT_STR_STR_INT2(str_literal_1, str_literal_2, int_literal) \
    str_literal_1 str_literal_2 #int_literal
#define CONCAT_STR_STR_INT1(str_literal_1, str_literal_2, int_literal) \
    CONCAT_STR_STR_INT2(str_literal_1, str_literal_2, int_literal)
#define LONGINT_FILE_LOCATION() \
    CONCAT_STR_STR_INT1(__FILE__, ":", __LINE__), CONFIG_CURRENT_FUNCTION_NAME

#if CONFIG_HAS_AT_LEAST_CXX_20 && !defined(_GLIBCXX_DEBUG) && !defined(_GLIBCXX_ASSERTIONS) && \
    ((CONFIG_COMPILER_ID != CONFIG_CLANG_COMPILER_ID &&                                        \
      CONFIG_COMPILER_ID != CONFIG_CLANG_CL_COMPILER_ID) ||                                    \
     CONFIG_CLANG_AT_LEAST(15, 0)) &&                                                          \
    (CONFIG_COMPILER_ID != CONFIG_GCC_COMPILER_ID || CONFIG_GNUC_AT_LEAST(12, 0))
#define CONSTEXPR_VECTOR constexpr
#else
#define CONSTEXPR_VECTOR inline
#endif

// clang-format off
// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays,
// cppcoreguidelines-avoid-magic-numbers)
// clang-format on

#if CONFIG_COMPILER_ID == CONFIG_GCC_COMPILER_ID ||   \
    CONFIG_COMPILER_ID == CONFIG_CLANG_COMPILER_ID || \
    CONFIG_COMPILER_ID == CONFIG_CLANG_CL_COMPILER_ID

#define HAS_CUSTOM_LONGINT_ALLOCATOR

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

    static constexpr std::size_t kTotalSmallPages = 32;
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
    LI_ALLOC_CONSTINIT static inline std::size_t total_small_pages_used = 0;
    LI_ALLOC_CONSTINIT static inline std::int32_t current_small_pages_used = 0;
    LI_ALLOC_CONSTINIT static inline std::int32_t max_small_pages_used = -(1 << 30);
    LI_ALLOC_CONSTINIT static inline std::size_t total_middle_pages_used = 0;
    LI_ALLOC_CONSTINIT static inline std::int32_t current_middle_pages_used = 0;
    LI_ALLOC_CONSTINIT static inline std::int32_t max_middle_pages_used = -(1 << 30);
    LI_ALLOC_CONSTINIT static inline std::size_t bytes_allocated = 0;
    LI_ALLOC_CONSTINIT static inline std::int32_t malloc_free_count = 0;
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
        const auto init_small_pages = []() noexcept {
            SmallPage* p = first_small_page;
            for (const SmallPage* last_page = p + kTotalSmallPages - 1; p != last_page;) {
                SmallPage* p_next = p + 1;
                p->next = p_next;
                p = p_next;
            }
            p->next = nullptr;
        };
        const auto init_middle_pages = []() noexcept {
            MiddlePage* p = first_middle_page;
            for (const MiddlePage* p_iter_end = p + kTotalMiddlePages - 1; p != p_iter_end;) {
                MiddlePage* p_next = p + 1;
                p->next = p_next;
                p = p_next;
            }
            p->next = nullptr;
        };
        init_small_pages();
        init_middle_pages();
#ifdef DEBUG_LI_ALLOC_PRINTING
        printf("[INIT] Inited pages in %s\n", CONFIG_CURRENT_FUNCTION_NAME);
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
            CONFIG_CURRENT_FUNCTION_NAME, total_small_pages_used, max_small_pages_used,
            current_small_pages_used, total_middle_pages_used, max_middle_pages_used,
            current_middle_pages_used, bytes_allocated, malloc_free_count);
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

inline void Deallocate(void* const memory) noexcept {
    if (unlikely(memory == nullptr)) {
        return;
    }

    std::byte* const p = static_cast<std::byte*>(memory);

#if CONFIG_COMPILER_ID == CONFIG_GCC_COMPILER_ID
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#endif

    if (inner_impl::IsSmallPage(p)) {
#ifdef DEBUG_LI_ALLOC_PRINTING
        inner_impl::current_small_pages_used--;
        assert(inner_impl::current_small_pages_used >= 0);
#endif
        auto* page = reinterpret_cast<inner_impl::SmallPage*>(p);
        page->next = inner_impl::free_small_pages_head;
        inner_impl::free_small_pages_head = page;
        return;
    }

    if (inner_impl::IsMiddlePage(p)) {
#ifdef DEBUG_LI_ALLOC_PRINTING
        inner_impl::current_middle_pages_used--;
        assert(inner_impl::current_middle_pages_used >= 0);
#endif
        auto* page = reinterpret_cast<inner_impl::MiddlePage*>(p);
        page->next = inner_impl::free_middle_pages_head;
        inner_impl::free_middle_pages_head = page;
        return;
    }

#if CONFIG_COMPILER_ID == CONFIG_GCC_COMPILER_ID
#pragma GCC diagnostic pop
#endif

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
[[gnu::malloc]]
#else
[[gnu::malloc, gnu::malloc(::longint_allocator::Deallocate, 1)]]
#endif
#endif
ATTRIBUTE_RETURNS_NONNULL ATTRIBUTE_ALLOC_SIZE(1) INLINE_LONGINT_ALLOCATE
    void* Allocate(const std::size_t size) {
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

    void* const p = ::operator new(size);
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

#endif

namespace longint_detail {
struct longint_static_storage;
}  // namespace longint_detail

class longint final {
    // Bug in the clang 14.0.0: if this method is defined after
    //  longint::divmod(uint32_t) (where it is used), CE will occured
    //  because of the 'not defined declared inline function' divmod_impl

    template <bool DoDivide,
              class LongIntType = std::conditional_t<DoDivide, longint, const longint>>
    [[nodiscard]] static constexpr int64_t divmod_impl(LongIntType& lhs,
                                                       const uint32_t n) noexcept {
        static_assert(sizeof(digit_t) == sizeof(uint32_t));

        const bool is_lhs_negative = lhs.is_negative();

        double_digit_t carry = 0;
        using IterType = std::conditional_t<DoDivide, digit_t, const digit_t>*;
        IterType const nums_iter_rend = lhs.nums_ - 1;
        for (IterType nums_riter = nums_iter_rend + lhs.usize(); nums_riter != nums_iter_rend;
             --nums_riter) {
            const double_digit_t cur = (carry << kDigitBits) | double_digit_t{*nums_riter};
            const double_digit_t q = cur / n;
            const double_digit_t r = cur % n;
            if constexpr (DoDivide) {
                *nums_riter = static_cast<digit_t>(q);
            }
            carry = r;
        }

        if constexpr (DoDivide) {
            lhs.pop_leading_zeros();
        }

        LONGINT_ASSERT_ASSUME(carry < n);
        const auto remainder = static_cast<uint32_t>(carry);
        return is_lhs_negative ? -int64_t{remainder} : int64_t{remainder};
    }

    enum class AddOrSub : bool {
        kAdd,
        kSub,
    };

    template <AddOrSub Operation, class T>
    void add_or_sub_32_bit_int(const T n) {
        if (unlikely(size() == 0)) {
            *this = n;
            if constexpr (Operation == AddOrSub::kSub) {
                this->flip_sign();
            }
            return;
        }

        const bool n_is_non_negative = n >= 0;
        const uint32_t n_abs = math_functions::uabs(n);
        const bool this_is_positive = is_positive();
        const bool same_sign = this_is_positive == n_is_non_negative;
        if (same_sign == (Operation == AddOrSub::kAdd)) {
            nonZeroSizeAddUInt(n_abs);
        } else {
            nonZeroSizeSubUInt(n_abs);
        }
    }

    template <class T>
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr T to_uint_unchecked() const noexcept {
        T value = 0;
        switch (usize()) {
            default: {
                static_assert(kDigitBits * 4 == 128, "add more cases");
                [[fallthrough]];
            }
            case 4: {
                if constexpr (sizeof(T) >= sizeof(digit_t) * 4) {
                    value |= T{nums_[4 - 1]} << (kDigitBits * (4 - 1));
                }

                [[fallthrough]];
            }
            case 3: {
                if constexpr (sizeof(T) >= sizeof(digit_t) * 3) {
                    value |= T{nums_[3 - 1]} << (kDigitBits * (3 - 1));
                }

                [[fallthrough]];
            }
            case 2: {
                if constexpr (sizeof(T) >= sizeof(digit_t) * 2) {
                    value |= double_digit_t{nums_[2 - 1]} << (kDigitBits * (2 - 1));
                }

                [[fallthrough]];
            }
            case 1: {
                if constexpr (sizeof(T) >= sizeof(digit_t) * 1) {
                    value |= nums_[1 - 1] << (kDigitBits * (1 - 1));
                }

                if constexpr (sizeof(T) < sizeof(digit_t)) {
                    value = nums_[0] & std::numeric_limits<T>::max();
                }

                [[fallthrough]];
            }
            case 0: {
                return value;
            }
        }
    }

public:
    using digit_t = std::uint32_t;
    using double_digit_t = std::uint64_t;
    using pointer = digit_t*;
    using const_pointer = const digit_t*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = typename std::reverse_iterator<iterator>;
    using const_reverse_iterator = typename std::reverse_iterator<const_iterator>;

    using size_type = std::uint32_t;
    using ssize_type = std::int32_t;

    static constexpr std::size_t kDefaultLINumsCapacity32 = 2;
    static constexpr std::size_t kDefaultLINumsCapacity64 = 2;
    static constexpr std::size_t kDefaultLINumsCapacity128 = 4;

    static constexpr std::uint32_t kStrConvBase = 1'000'000'000;
    static constexpr std::uint32_t kStrConvBaseDigits =
        math_functions::base_b_len(kStrConvBase - 1);
    static constexpr std::uint32_t kDigitBits = sizeof(digit_t) * CHAR_BIT;
    static_assert(sizeof(digit_t) < sizeof(double_digit_t));
    static constexpr double_digit_t kNumsBase = double_digit_t{1} << kDigitBits;

    static constexpr std::size_t kFFTPrecisionBorder = 1u << 18;
    static constexpr auto kFFTFloatRoundError =
        std::numeric_limits<typename fft::complex::value_type>::round_error();

    static constexpr uint32_t kDecimalBase = kStrConvBase;
    static constexpr uint32_t kFFTDecimalBase = 1'000;

    longint() = default;

    longint(const longint& other) : nums_(nullptr), size_(0), capacity_(0) {
        if (other.capacity_ > 0) {
            nums_ = allocate_uninitialized(other.capacity_);
            size_ = other.size_;
            capacity_ = other.capacity_;
            std::uninitialized_copy_n(other.nums_, other.size_, nums_);
        }
    }

    longint& operator=(const longint& other) ATTRIBUTE_LIFETIME_BOUND {
        return *this = longint{other};
    }

    constexpr longint(longint&& other) noexcept
        : nums_(other.nums_), size_(other.size_), capacity_(other.capacity_) {
        other.nums_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

#if CONFIG_HAS_AT_LEAST_CXX_20
    constexpr
#endif
        longint&
        operator=(longint&& other) noexcept ATTRIBUTE_LIFETIME_BOUND {
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

    /* implicit */ longint(const uint32_t n) {
        this->allocate_default_capacity_32();
        this->assign_u32_unchecked(n);
    }
    /* implicit */ longint(const int32_t n) {
        this->allocate_default_capacity_32();
        this->assign_i32_unchecked(n);
    }
    /* implicit */ longint(const uint64_t n) {
        this->allocate_default_capacity_64();
        this->assign_u64_unchecked(n);
    }
    /* implicit */ longint(const int64_t n) {
        this->allocate_default_capacity_64();
        this->assign_i64_unchecked(n);
    }
#if defined(INTEGERS_128_BIT_HPP)
    /* implicit */ longint(const uint128_t n) {
        this->allocate_default_capacity_128();
        this->assign_u128_unchecked(n);
    }
    /* implicit */ longint(const int128_t n) {
        this->allocate_default_capacity_128();
        this->assign_i128_unchecked(n);
    }
#endif

    [[nodiscard]] static longint from_string(const std::string_view s) {
        longint ret;
        ret.set_string(s);
        return ret;
    }

    class Reserve {
    public:
        explicit constexpr Reserve(const size_type capacity) noexcept : capacity_(capacity) {}

        [[nodiscard]] ATTRIBUTE_PURE constexpr size_type get_capacity() const noexcept {
            return capacity_;
        }

    private:
        const size_type capacity_{};
    };

    explicit longint(const Reserve reserve_tag) {
        reserve(reserve_tag.get_capacity());
    }

    longint& operator=(const uint32_t n) ATTRIBUTE_LIFETIME_BOUND {
        this->ensure_default_capacity_op_asgn_32();
        this->assign_u32_unchecked(n);
        return *this;
    }
    longint& operator=(const int32_t n) ATTRIBUTE_LIFETIME_BOUND {
        this->ensure_default_capacity_op_asgn_32();
        this->assign_i32_unchecked(n);
        return *this;
    }
    longint& operator=(const uint64_t n) ATTRIBUTE_LIFETIME_BOUND {
        this->ensure_default_capacity_op_asgn_64();
        this->assign_u64_unchecked(n);
        return *this;
    }
    longint& operator=(const int64_t n) ATTRIBUTE_LIFETIME_BOUND {
        this->ensure_default_capacity_op_asgn_64();
        this->assign_i64_unchecked(n);
        return *this;
    }
#if defined(INTEGERS_128_BIT_HPP)
    longint& operator=(const uint128_t n) ATTRIBUTE_LIFETIME_BOUND {
        this->ensure_default_capacity_op_asgn_128();
        this->assign_u128_unchecked(n);
        return *this;
    }
    longint& operator=(const int128_t n) ATTRIBUTE_LIFETIME_BOUND {
        this->ensure_default_capacity_op_asgn_128();
        this->assign_i128_unchecked(n);
        return *this;
    }
#endif

    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_CONST
    [[nodiscard]]
    static constexpr size_type max_size() noexcept {
        constexpr auto kMaxSSize = std::numeric_limits<ssize_type>::max();
        constexpr auto kMaxUSize = std::numeric_limits<size_type>::max();
        constexpr auto kMaxSize = static_cast<size_type>(std::min({
            static_cast<std::size_t>(kMaxSSize),
            static_cast<std::size_t>(kMaxUSize / 2),
            static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) / sizeof(digit_t),
            std::numeric_limits<std::size_t>::max() / sizeof(digit_t),
        }));
        return kMaxSize;
    }

    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr ssize_type size() const noexcept {
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-type-limit-compare"
#endif
        const ssize_type value = size_;
        if (value > static_cast<ssize_type>(max_size())) {
            CONFIG_UNREACHABLE();
        }
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
        return value;
    }

    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr size_type usize() const noexcept {
        const auto value = math_functions::uabs(size());
        if (value > max_size()) {
            CONFIG_UNREACHABLE();
        }
        return value;
    }

    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr size_type capacity() const noexcept {
        const auto value = capacity_;
        if (value > max_size()) {
            CONFIG_UNREACHABLE();
        }
        return value;
    }

    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr std::int32_t sign() const noexcept {
        return math_functions::sign(size());
    }

    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr bool is_zero() const noexcept {
        return size() == 0;
    }

    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr bool is_positive() const noexcept {
        return size() > 0;
    }

    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr bool is_negative() const noexcept {
        return size() < 0;
    }

    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr bool empty() const noexcept {
        return is_zero();
    }

    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] explicit constexpr operator bool() const noexcept {
        return !is_zero();
    }

    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr bool operator!() const noexcept {
        return is_zero();
    }

    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr const_iterator begin() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return nums_;
    }
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr const_iterator end() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return nums_ + usize();
    }
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return begin();
    }
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr const_iterator cend() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return end();
    }
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return std::make_reverse_iterator(end());
    }
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return std::make_reverse_iterator(begin());
    }
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return rbegin();
    }
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return rend();
    }

    constexpr void flip_sign() noexcept {
        size_ = -size_;
    }

    ATTRIBUTE_REINITIALIZES constexpr void assign_zero() noexcept {
        size_ = 0;
    }

    longint& pow(std::size_t p) ATTRIBUTE_LIFETIME_BOUND {
        longint res = uint32_t{1};
        reserve((std::max(usize(), size_type{1}) - 1) * p);
        while (true) {
            if (p & 1) {
                res *= *this;
            }
            p >>= 1;
            if (p == 0) {
                break;
            }
            square_inplace();
        }
        return *this = std::move(res);
    }

    void square_this_to(longint& other) const {
        const size_type nums_size = usize();
        if (unlikely(nums_size == 0)) {
            other.assign_zero();
            return;
        }

        const digit_t* nums_ptr = nums_;
        static_assert(max_size() + max_size() > max_size());
        const size_type prod_size = check_size(nums_size * 2);
        if (prod_size <= 16) {
            digit_t* ans = allocate_uninitialized(prod_size);
            std::fill_n(ans, prod_size, digit_t{0});
            for (std::size_t j = 0; j < nums_size; j++) {
                const double_digit_t b_j = nums_ptr[j];
                double_digit_t carry = 0;
                for (std::size_t i = 0; i < nums_size; i++) {
                    const double_digit_t a_i = nums_ptr[i];
                    const double_digit_t res = a_i * b_j + double_digit_t{ans[j + i]} + carry;
                    ans[j + i] = static_cast<digit_t>(res);
                    carry = res >> kDigitBits;
                }

                ans[j + nums_size] = static_cast<digit_t>(carry);
            }

            other.adopt_digits_sequence_without_changing_size(ans, prod_size);
        } else {
            const auto [n, need_high_precision] = LongIntFFT::compute_fft_product_params(prod_size);
            // Allocate n complex numbers for p1 and n complex numbers for p2
            const std::unique_ptr<fft::complex, struct ComplexDeleter> p1(
                allocate_complex_array_for_unique_ptr(2 * n));
            LongIntFFT::convert_longint_nums_to_fft_poly(nums_ptr, nums_size, p1.get(), n,
                                                         need_high_precision);
            other.reserveUninitializedWithoutCopy(prod_size);
            fft::complex* p2 = p1.get() + n;
            fft::forward_backward_fft(p1.get(), p2, n);
            LongIntFFT::convert_fft_poly_to_longint_nums(need_high_precision, p2, other.nums_,
                                                         prod_size);
        }

        other.size_ = static_cast<ssize_type>(prod_size);
        other.pop_leading_zeros();
    }

    longint& square_inplace() ATTRIBUTE_LIFETIME_BOUND {
        square_this_to(*this);
        return *this;
    }

    [[nodiscard]] constexpr digit_t operator[](const std::size_t pos) const noexcept {
        return nums_[pos];
    }

    longint& operator*=(const longint& other) ATTRIBUTE_LIFETIME_BOUND {
        size_type k = usize();
        size_type m = other.usize();
        const digit_t* k_ptr = nums_;
        const digit_t* m_ptr = other.nums_;

        if (m > k) {
            std::swap(m_ptr, k_ptr);
            std::swap(m, k);
        }

        if (unlikely(m == 0)) {
            assign_zero();
            return *this;
        }

        static_assert(max_size() + max_size() > max_size());
        const size_type prod_size = check_size(m + k);
        if (m <= 16 || m * k <= 1024) {
            digit_t* ans = allocate_uninitialized(prod_size);
            std::fill_n(ans, prod_size, digit_t{0});
            LongIntNaive::multiply_and_store_to(m_ptr, m, k_ptr, k, ans);
            deallocate(nums_, capacity_);
            nums_ = ans;
            capacity_ = prod_size;
        } else {
            const auto [n, need_high_precision] = LongIntFFT::compute_fft_product_params(prod_size);
            // Allocate n complex numbers for p1 and n complex numbers for p2
            const std::unique_ptr<fft::complex, struct ComplexDeleter> p1(
                allocate_complex_array_for_unique_ptr(2 * n));
            LongIntFFT::convert_longint_nums_to_fft_poly(m_ptr, m, k_ptr, k, p1.get(), n,
                                                         need_high_precision);
            reserveUninitializedWithoutCopy(prod_size);
            fft::complex* const p2 = p1.get() + n;
            fft::forward_backward_fft(p1.get(), p2, n);
            LongIntFFT::convert_fft_poly_to_longint_nums(need_high_precision, p2, nums_, prod_size);
        }

        const ssize_type sign_product = size_ ^ other.size_;
        set_ssize_from_size_and_sign(prod_size, /* sign = */ sign_product);
        pop_leading_zeros();
        return *this;
    }

    [[nodiscard]] friend longint operator*(longint lhs, const longint& rhs) {
        lhs *= rhs;
        return lhs;
    }

    [[nodiscard]] longint divmod(const longint& other) {
        longint rem;
        this->divmod(other, rem);
        return rem;
    }

    void divmod(const longint& other, longint& rem) {
        this->divmod_impl(other, rem);
    }

    longint& operator/=(const longint& other) ATTRIBUTE_LIFETIME_BOUND {
        std::ignore = divmod(other);
        return *this;
    }

    [[nodiscard]] friend longint operator/(longint lhs, const longint& rhs) {
        lhs /= rhs;
        return lhs;
    }

    longint& operator+=(const longint& other) ATTRIBUTE_LIFETIME_BOUND {
        const bool find_sum = (size() ^ other.size()) >= 0;
        const size_type other_usize = other.usize();

        static_assert(max_size() + 1 > max_size());
        const size_type this_usize =
            set_size_at_least(std::max(usize(), other_usize) + (find_sum ? 1u : 0u));

        LONGINT_ASSERT_ASSUME(this_usize >= other_usize);
        if (find_sum) {
            LONGINT_ASSERT_ASSUME(this_usize > other_usize);
            longint_add_with_free_space(nums_, this_usize, other.nums_, other_usize);
        } else {
            if (longint_subtract_with_free_space(nums_, this_usize, other.nums_, other_usize)) {
                flip_sign();
            }
        }

        pop_leading_zeros();
        return *this;
    }

    longint& operator-=(const longint& other) ATTRIBUTE_LIFETIME_BOUND {
        flip_sign();
        *this += other;
        flip_sign();
        return *this;
    }

    [[nodiscard]] friend longint operator+(longint lhs, const longint& rhs) {
        lhs += rhs;
        return lhs;
    }

    [[nodiscard]] friend longint operator-(longint lhs, const longint& rhs) {
        lhs -= rhs;
        return lhs;
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator==(const int32_t n) const noexcept {
        if ((config::is_constant_evaluated() || config::is_gcc_constant_p(n)) && n == 0) {
            return is_zero();
        }

        static_assert(kDigitBits == 32);
        switch (size_) {
            case 0: {
                return n == 0;
            }
            case 1: {
                return nums_[0] == static_cast<uint32_t>(n) && n > 0;
            }
            case -1: {
                return nums_[0] == -static_cast<uint32_t>(n) && n < 0;
            }
            default: {
                return false;
            }
        }
    }
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator==(const int64_t n) const noexcept {
        if ((config::is_constant_evaluated() || config::is_gcc_constant_p(n)) && n == 0) {
            return is_zero();
        }

        if (this->sign() != math_functions::sign(n)) {
            return false;
        }

        const uint64_t n_abs = math_functions::uabs(n);
        static_assert(kDigitBits == 32);
        switch (size_) {
            case 0: {
                return n == 0;
            }
            case 1:
            case -1: {
                return nums_[0] == n_abs;
            }
            case 2:
            case -2: {
                return ((static_cast<uint64_t>(nums_[1]) << 32) | nums_[0]) == n_abs;
            }
            default: {
                return false;
            }
        }
    }
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator==(const uint32_t n) const noexcept {
        if ((config::is_constant_evaluated() || config::is_gcc_constant_p(n)) && n == 0) {
            return is_zero();
        }

        static_assert(kDigitBits == 32);
        switch (size_) {
            case 0: {
                return n == 0;
            }
            case 1: {
                return nums_[0] == n;
            }
            default: {
                return false;
            }
        }
    }
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator==(const uint64_t n) const noexcept {
        if ((config::is_constant_evaluated() || config::is_gcc_constant_p(n)) && n == 0) {
            return is_zero();
        }

        static_assert(kDigitBits == 32);
        switch (size_) {
            case 0: {
                return n == 0;
            }
            case 1: {
                return nums_[0] == n;
            }
            case 2: {
                return ((static_cast<uint64_t>(nums_[1]) << 32) | nums_[0]) == n;
            }
            default: {
                return false;
            }
        }
    }
#if defined(INTEGERS_128_BIT_HPP)
    [[nodiscard]] ATTRIBUTE_PURE I128_CONSTEXPR bool operator==(const uint128_t n) const noexcept {
        if ((config::is_constant_evaluated() || config::is_gcc_constant_p(n)) && n == 0) {
            return is_zero();
        }

        static_assert(kDigitBits == 32);
        switch (size_) {
            case 0: {
                return n == 0;
            }
            case 1: {
                return nums_[0] == n;
            }
            case 2: {
                return ((static_cast<uint64_t>(nums_[1]) << 32) | nums_[0]) == n;
            }
            case 3: {
                const uint64_t low = (static_cast<uint64_t>(nums_[1]) << 32) | nums_[0];
                return ((static_cast<uint128_t>(nums_[2]) << 64) | low) == n;
            }
            case 4: {
                const uint64_t low = (static_cast<uint64_t>(nums_[1]) << 32) | nums_[0];
                const uint64_t hi = (static_cast<uint64_t>(nums_[3]) << 32) | nums_[2];
                return ((static_cast<uint128_t>(hi) << 64) | low) == n;
            }
            default: {
                return false;
            }
        }
    }
    [[nodiscard]] ATTRIBUTE_PURE I128_CONSTEXPR bool operator==(const int128_t n) const noexcept {
        if ((config::is_constant_evaluated() || config::is_gcc_constant_p(n)) && n == 0) {
            return is_zero();
        }

        if (this->sign() != math_functions::sign(n)) {
            return false;
        }

        const uint128_t n_abs = math_functions::uabs(n);
        static_assert(kDigitBits == 32);
        switch (size_) {
            case 0: {
                return n == 0;
            }
            case 1:
            case -1: {
                return nums_[0] == n_abs;
            }
            case 2:
            case -2: {
                return ((uint64_t{nums_[1]} << 32) | nums_[0]) == n_abs;
            }
            case 3:
            case -3: {
                const uint64_t low = (uint64_t{nums_[1]} << 32) | nums_[0];
                return ((uint128_t{nums_[2]} << 64) | low) == n_abs;
            }
            case 4:
            case -4: {
                const uint64_t low = (uint64_t{nums_[1]} << 32) | nums_[0];
                const uint64_t hi = (uint64_t{nums_[3]} << 32) | nums_[2];
                return ((uint128_t{hi} << 64) | low) == n_abs;
            }
            default: {
                return false;
            }
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

        const size_type usize_value = usize();
        const digit_t* const r_end = nums_ - 1;
        const digit_t* r_nums = r_end + usize_value;
        const digit_t* r_other_nums = other.nums_ - 1 + usize_value;
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

        const size_type usize_value = usize();
        const digit_t* r_end = nums_ - 1;
        const digit_t* r_nums = r_end + usize_value;
        const digit_t* r_other_nums = other.nums_ - 1 + usize_value;
        for (; r_nums != r_end; r_nums--, r_other_nums--) {
            if (*r_nums != *r_other_nums) {
                static_assert(sizeof(int64_t) == sizeof(double_digit_t), "");
                return int64_t{*r_nums} * sign() < int64_t{*r_other_nums} * other.sign();
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
    [[nodiscard]] ATTRIBUTE_PURE I128_CONSTEXPR bool operator!=(uint128_t n) const noexcept {
        return !(*this == n);
    }
    [[nodiscard]] ATTRIBUTE_PURE I128_CONSTEXPR bool operator!=(int128_t n) const noexcept {
        return !(*this == n);
    }
#endif
    [[nodiscard]] ATTRIBUTE_PURE constexpr bool operator!=(const longint& other) const noexcept {
        return !(*this == other);
    }
#endif

    longint& operator+=(const uint32_t n) ATTRIBUTE_LIFETIME_BOUND {
        add_or_sub_32_bit_int<AddOrSub::kAdd>(n);
        return *this;
    }

    longint& operator-=(const uint32_t n) ATTRIBUTE_LIFETIME_BOUND {
        add_or_sub_32_bit_int<AddOrSub::kSub>(n);
        return *this;
    }

    longint& operator+=(const int32_t n) ATTRIBUTE_LIFETIME_BOUND {
        add_or_sub_32_bit_int<AddOrSub::kAdd>(n);
        return *this;
    }

    longint& operator-=(const int32_t n) ATTRIBUTE_LIFETIME_BOUND {
        add_or_sub_32_bit_int<AddOrSub::kSub>(n);
        return *this;
    }

    longint& operator*=(const uint32_t n) ATTRIBUTE_LIFETIME_BOUND {
        if (unlikely(n == 0)) {
            assign_zero();
            return *this;
        }

        double_digit_t carry = 0;
        const double_digit_t b_0 = n;
        const size_type usize_value = usize();
        for (digit_t *nums_it = nums_, *nums_it_end = nums_it + usize_value; nums_it != nums_it_end;
             ++nums_it) {
            const double_digit_t res = *nums_it * b_0 + carry;
            *nums_it = static_cast<digit_t>(res);
            carry = res >> kDigitBits;
        }

        // n != 0 => sign won't change and there will be no leading zeros
        if (carry != 0) {
            assert(usize_value <= capacity_);
            if (unlikely(usize_value >= capacity_)) {
                grow_capacity();
                assert(usize_value < capacity_);
            }

            nums_[usize_value] = static_cast<digit_t>(carry);
            size_ += sign();
        }

        return *this;
    }

    longint& operator*=(const int32_t n) ATTRIBUTE_LIFETIME_BOUND {
        if (n < 0) {
            flip_sign();
        }
        return *this *= math_functions::uabs(n);
    }

    ATTRIBUTE_ALWAYS_INLINE
    constexpr longint& operator/=(const uint32_t n) noexcept ATTRIBUTE_LIFETIME_BOUND {
        std::ignore = divmod(n);
        return *this;
    }

    ATTRIBUTE_ALWAYS_INLINE
    constexpr longint& operator/=(const int32_t n) noexcept ATTRIBUTE_LIFETIME_BOUND {
        const bool negative = n < 0;
        *this /= math_functions::uabs(n);
        if (negative) {
            flip_sign();
        }
        return *this;
    }

    /// @brief Set *this = *this / n and return *this % n
    /// @note  Behaviour is undefined if n equals 0
    /// @param n
    /// @return *this mod n
    ATTRIBUTE_NODISCARD_WITH_MESSAGE("use `/=` if you don't need the result of the `divmod`")
    ATTRIBUTE_ALWAYS_INLINE constexpr int64_t divmod(const uint32_t n) noexcept {
        if ((config::is_constant_evaluated() || config::is_gcc_constant_p(n)) &&
            (n & (n - 1)) == 0) {
            if (n <= 1) {
                /* Quite return when dividing by zero, i.e. n = 0. */
                return 0;
            }

            const int64_t remainder = mod_by_power_of_2_ge_2_impl(n);
            const auto shift = static_cast<uint32_t>(math_functions::countr_zero(n));
            *this >>= shift;
            return remainder;
        }

        return divmod_impl</*DoDivide=*/true>(*this, n);
    }

    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]]
    constexpr int64_t operator%(const uint32_t n) const noexcept {
        return this->mod(n);
    }

    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr int64_t mod(const uint32_t n) const noexcept {
        if ((config::is_constant_evaluated() || config::is_gcc_constant_p(n)) &&
            (n & (n - 1)) == 0) {
            if (n <= 1) {
                /* Quite return when dividing by zero, i.e. n = 0. */
                return 0;
            }

            return mod_by_power_of_2_ge_2_impl(n);
        }

        return divmod_impl</*DoDivide=*/false>(*this, n);
    }

    constexpr longint& operator>>=(size_type shift) noexcept ATTRIBUTE_LIFETIME_BOUND {
        if ((config::is_constant_evaluated() || config::is_gcc_constant_p(shift)) && shift == 0) {
            return *this;
        }

        size_type usize_value = usize();
        const size_type uints_move = shift / kDigitBits;
        if (uints_move >= usize_value) {
            assign_zero();
            return *this;
        }

        if (uints_move > 0) {
            usize_value -= uints_move;
            set_ssize_from_size(usize_value);
            digit_t* copy_dst_start = nums_;
            const digit_t* copy_src_start = copy_dst_start + uints_move;
            const digit_t* copy_src_end = copy_src_start + usize_value;
            std::copy(copy_src_start, copy_src_end, copy_dst_start);
        }

        shift %= kDigitBits;
        digit_t* nums_iter = nums_;
        digit_t* const nums_iter_last = nums_iter + usize_value - 1;
        if (shift > 0) {
            for (; nums_iter != nums_iter_last; ++nums_iter) {
                const digit_t current_digit = nums_iter[0];
                const digit_t next_digit = nums_iter[1];
                *nums_iter = static_cast<digit_t>(
                    (current_digit | double_digit_t{next_digit} << kDigitBits) >> shift);
            }
            *nums_iter_last >>= shift;
        }

        if (*nums_iter_last == 0) {
            size_ -= size_ >= 0 ? 1 : -1;
        }

        return *this;
    }
    constexpr longint& operator<<=(size_type shift) ATTRIBUTE_LIFETIME_BOUND {
        if ((config::is_constant_evaluated() || config::is_gcc_constant_p(shift)) && shift == 0) {
            return *this;
        }

        size_type usize_value = usize();
        if (usize_value == 0) {
            return *this;
        }

        static_assert(sizeof(digit_t) == sizeof(uint32_t));
        const size_type new_trailig_zeros_digits = shift / kDigitBits;
        // + 1 for potentially new back digit (at the end of the nums_ array)
        const auto new_size = usize_value + new_trailig_zeros_digits + 1;
        reserve(new_size);
        digit_t* last_digit_ptr = nums_ + usize_value + new_trailig_zeros_digits;
        *last_digit_ptr = 0;
        if (new_trailig_zeros_digits > 0) {
            std::copy_backward(nums_, nums_ + usize_value, last_digit_ptr);
            std::fill_n(nums_, new_trailig_zeros_digits, digit_t{0});
        }
        usize_value = new_size;

        shift %= kDigitBits;
        if (shift > 0) {
            digit_t* const nums_iter_begin = nums_ + new_trailig_zeros_digits;
            digit_t* const nums_iter_end = nums_ + usize_value;
            for (digit_t* nums_iter = nums_iter_end - 1; nums_iter > nums_iter_begin; nums_iter--) {
                const digit_t prev_digit = *(nums_iter - 1);
                const digit_t current_digit = *nums_iter;
                const double_digit_t two_digits =
                    (double_digit_t{current_digit} << kDigitBits) | prev_digit;
                *nums_iter = static_cast<digit_t>(two_digits >> (kDigitBits - shift));
            }
            *nums_iter_begin = (*nums_iter_begin) << shift;
        }

        if (nums_[usize_value - 1] == 0) {
            usize_value--;
        }
        set_ssize_from_size(usize_value);
        return *this;
    }

    void set_string(const std::string_view s) {
        check_dec_str(s);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        this->set_dec_str_impl(reinterpret_cast<const unsigned char*>(s.data()), s.size());
    }

    template <class T>
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_PURE constexpr bool fits_in_uint() const noexcept {
        // If size() < 0, this expression will evaluate to large positive number
        const size_type size_value = static_cast<size_type>(size());

        if constexpr (sizeof(T) >= sizeof(digit_t)) {
            static_assert(sizeof(T) % sizeof(digit_t) == 0);
            return size_value <= sizeof(T) / sizeof(digit_t);
        } else {
            return size_value == 0 ||
                   (size_value == 1 && nums_[0] <= std::numeric_limits<T>::max());
        }
    }

    template <class T>
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE constexpr T to_uint() const {
        if (unlikely(!fits_in_uint<T>())) {
            throw_on_failed_uint_checked_cast(LONGINT_FILE_LOCATION());
        }

        return to_uint_unchecked<T>();
    }

    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr bool fits_in_uint32() const noexcept {
        return fits_in_uint<std::uint32_t>();
    }
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] /* implicit */ constexpr operator std::uint32_t() const noexcept {
        return to_uint_unchecked<std::uint32_t>();
    }
    ATTRIBUTE_ALWAYS_INLINE
    [[nodiscard]] constexpr std::uint32_t to_uint32() const {
        return to_uint<std::uint32_t>();
    }

    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr bool fits_in_uint64() const noexcept {
        return fits_in_uint<std::uint64_t>();
    }

    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] /* implicit */ constexpr operator std::uint64_t() const noexcept {
        return to_uint_unchecked<std::uint64_t>();
    }
    ATTRIBUTE_ALWAYS_INLINE
    [[nodiscard]] constexpr std::uint64_t to_uint64() const {
        return to_uint<std::uint64_t>();
    }

#if defined(INTEGERS_128_BIT_HPP)
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] constexpr bool fits_in_uint128() const noexcept {
        return fits_in_uint<uint128_t>();
    }
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_PURE
    [[nodiscard]] /* implicit */ I128_CONSTEXPR operator uint128_t() const noexcept {
        return to_uint_unchecked<uint128_t>();
    }
    ATTRIBUTE_ALWAYS_INLINE
    [[nodiscard]] I128_CONSTEXPR uint128_t to_uint128() const noexcept {
        return to_uint<uint128_t>();
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

        static_assert(sizeof(digit_t) == sizeof(uint32_t));
        const size_type usize_value = usize();
        switch (usize_value) {
            case 0: {
                ans = "0";
                return;
            }
            case 1: {
                ans += std::to_string(nums_[0]);
                return;
            }
            case 2: {
                ans += std::to_string((double_digit_t{nums_[1]} << kDigitBits) | nums_[0]);
                return;
            }
            default: {
                break;
            }
        }

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-type-limit-compare"
#endif
        static_assert(math_functions::nearest_greater_equal_power_of_two(max_size()) <=
                      std::numeric_limits<std::size_t>::max());
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

        const Decimal result = [&]() {
            const size_type n =
                check_size(math_functions::nearest_greater_equal_power_of_two(usize_value));
            ensure_bin_base_pows_capacity(math_functions::log2_floor(n));
            std::vector<digit_t> knums(n);
            std::copy_n(nums_, usize_value, knums.data());
            return convert_bin_base(knums.data(), n);
        }();

        assert(result.digits_.size() >= 3);
        const typename Decimal::dec_size_type full_blocks = result.digits_.size() - 1;
        typename Decimal::dec_digit_t last_a_i = result.digits_[full_blocks];
        const std::size_t string_size =
            full_blocks * kStrConvBaseDigits + math_functions::base_10_len(last_a_i);
        ans.resize(ans.size() + string_size);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        auto* ptr = reinterpret_cast<unsigned char*>(std::addressof(ans[ans.size() - 1]));
        for (typename Decimal::dec_size_type i = 0; i < full_blocks; i++) {
            typename Decimal::dec_digit_t a_i = result.digits_[i];
            for (auto j = kStrConvBaseDigits; j > 0; j--) {
                *ptr = static_cast<unsigned char>('0' + a_i % 10);
                a_i /= 10;
                ptr--;
            }
        }

        do {
            *ptr = static_cast<unsigned char>('0' + last_a_i % 10);
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
        LONGINT_ASSERT_ASSUME(usize() <= capacity());

        const size_type checked_capacity = check_size(requested_capacity);
        if (checked_capacity <= capacity_) {
            return;
        }

        digit_t* const new_nums = allocate_uninitialized(checked_capacity);
        const size_type nums_size = usize();
        if (nums_size > 0) {
            std::uninitialized_copy_n(nums_, nums_size, new_nums);
        }
        adopt_digits_sequence_without_changing_size(new_nums, checked_capacity);
    }

    ~longint() {
        deallocate(nums_, capacity_);
    }

    struct Decimal final {
        using dec_digit_t = uint32_t;
        using double_dec_digit_t = uint64_t;
        using dec_size_type = std::size_t;

        std::vector<dec_digit_t> digits_{};

        Decimal() = default;

        explicit Decimal(const uint32_t n) {
            assign_uint32(n);
        }

        explicit Decimal(const uint64_t n) {
            assign_uint64(n);
        }

        CONSTEXPR_VECTOR void swap(Decimal& other) noexcept {
            digits_.swap(other.digits_);
        }

        friend CONSTEXPR_VECTOR void swap(Decimal& lhs, Decimal& rhs) noexcept {
            lhs.swap(rhs);
        }

        /* implicit */ Decimal& operator=(const uint32_t n) ATTRIBUTE_LIFETIME_BOUND {
            assign_uint32(n);
            return *this;
        }

        /* implicit */ Decimal& operator=(const uint64_t n) ATTRIBUTE_LIFETIME_BOUND {
            assign_uint64(n);
            return *this;
        }

        Decimal& operator+=(const Decimal& other) ATTRIBUTE_LIFETIME_BOUND {
            double_dec_digit_t carry = 0;
            const dec_size_type m = std::min(digits_.size(), other.digits_.size());
            dec_digit_t* p = digits_.data();
            for (dec_size_type i = 0; i < m; i++) {
                const double_dec_digit_t res =
                    double_dec_digit_t{p[i]} + double_dec_digit_t{other.digits_[i]} + carry;
                p[i] = static_cast<dec_digit_t>(res % kDecimalBase);
                carry = res / kDecimalBase;
            }

            const size_t current_size = digits_.size();
            const size_t other_size = other.digits_.size();
            if (current_size < other_size) {
                digits_.resize(other_size);
                std::copy_n(other.digits_.data() + current_size, other_size - current_size,
                            digits_.data() + current_size);
            }

            for (dec_size_type i = m; carry != 0 && i < digits_.size(); i++) {
                const auto res = double_dec_digit_t{digits_[i]} + carry;
                digits_[i] = static_cast<dec_digit_t>(res % kDecimalBase);
                carry = res / kDecimalBase;
            }

            if (carry == 0) {
                pop_leading_zeros();
            } else {
                digits_.emplace_back() = static_cast<dec_digit_t>(carry);
            }

            return *this;
        }

        Decimal& operator*=(const Decimal& other) ATTRIBUTE_LIFETIME_BOUND {
            dec_size_type k = digits_.size();
            dec_size_type m = other.digits_.size();
            const dec_digit_t* k_ptr = digits_.data();
            const dec_digit_t* m_ptr = other.digits_.data();

            if (k < m) {
                std::swap(k_ptr, m_ptr);
                std::swap(k, m);
            }

            if (unlikely(m == 0)) {
                assign_zero();
                return *this;
            }

            LONGINT_ASSERT_ASSUME(1 <= m && m <= k);
            if (m <= 16 || m * k <= 1024) {
                DecNaive::multiply_and_store_to(m_ptr, m, k_ptr, k, *this);
            } else {
                DecFFT::multiply_and_store_to(m_ptr, m, k_ptr, k, *this);
            }

            pop_leading_zeros();
            return *this;
        }

        void square_this_to(Decimal& other) const {
            const dec_size_type digits_size = digits_.size();
            if (unlikely(digits_size == 0)) {
                other.assign_zero();
                return;
            }

            if (digits_size <= 32) {
                DecNaive::square_and_store_to(digits_.data(), digits_size, other);
            } else {
                DecFFT::square_and_store_to(digits_.data(), digits_size, other);
            }

            other.pop_leading_zeros();
        }

        [[nodiscard]]
        ATTRIBUTE_PURE CONSTEXPR_VECTOR bool operator==(const uint32_t n) const noexcept {
            static_assert(sizeof(dec_digit_t) == sizeof(uint32_t));
            switch (digits_.size()) {
                case 0: {
                    return n == 0;
                }
                case 1: {
                    return digits_[0] == n;
                }
                case 2: {
                    return static_cast<uint64_t>(digits_[1]) * kDecimalBase + digits_[0] == n;
                }
                default: {
                    return false;
                }
            }
        }

        [[nodiscard]]
        ATTRIBUTE_PURE CONSTEXPR_VECTOR bool operator!=(const uint32_t n) const noexcept {
            return !(*this == n);
        }

        [[nodiscard]]
        ATTRIBUTE_PURE CONSTEXPR_VECTOR bool operator==(const uint64_t n) const noexcept {
            static_assert(sizeof(dec_digit_t) == sizeof(uint32_t));
            switch (digits_.size()) {
                case 0: {
                    return n == 0;
                }
                case 1: {
                    return digits_[0] == n;
                }
                case 2: {
                    return static_cast<uint64_t>(digits_[1]) * kDecimalBase + digits_[0] == n;
                }
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
                default: {
                    return false;
                }
            }
        }

        ATTRIBUTE_PURE
        [[nodiscard]] CONSTEXPR_VECTOR bool operator!=(const uint64_t n) const noexcept {
            return !(*this == n);
        }

        ATTRIBUTE_PURE
        [[nodiscard]]
        bool operator==(const Decimal& other) const noexcept
#if CONFIG_HAS_AT_LEAST_CXX_20
            = default;
#else
        {
            return digits_ == other.digits_;
        }
#endif

        ATTRIBUTE_REINITIALIZES CONSTEXPR_VECTOR void assign_zero() noexcept {
            digits_.clear();
        }

        CONSTEXPR_VECTOR void pop_leading_zeros() noexcept {
            while (!digits_.empty() && digits_.back() == 0) {
                digits_.pop_back();
            }
        }

    private:
        struct DecNaive final {
            ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
            ATTRIBUTE_NONNULL_ALL_ARGS
            ATTRIBUTE_ALWAYS_INLINE
            static void square_and_store_to(const dec_digit_t digits[],
                                            const dec_size_type digits_size,
                                            Decimal& product_result) {
                DecNaive::multiply_and_store_to(digits, digits_size, digits, digits_size,
                                                product_result);
            }
            ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
            ATTRIBUTE_SIZED_ACCESS(read_only, 3, 4)
            ATTRIBUTE_NONNULL_ALL_ARGS
            static void multiply_and_store_to(const dec_digit_t m_digits[],
                                              const dec_size_type m_size,
                                              const dec_digit_t k_digits[],
                                              const dec_size_type k_size,
                                              Decimal& product_result) {
                LONGINT_ASSERT_ASSUME(m_size <= k_size);
                const dec_size_type new_size = m_size + k_size;
                std::vector<dec_digit_t> ans(new_size);
                DecNaive::multiply_and_store_to_impl(m_digits, m_size, k_digits, k_size,
                                                     ans.data());
                product_result.digits_ = std::move(ans);
            }

        private:
            ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
            ATTRIBUTE_SIZED_ACCESS(read_only, 3, 4)
            ATTRIBUTE_ACCESS(read_write, 5)
            ATTRIBUTE_NONNULL_ALL_ARGS
            static void multiply_and_store_to_impl(const dec_digit_t m_digits[],
                                                   const dec_size_type m_size,
                                                   const dec_digit_t k_digits[],
                                                   const dec_size_type k_size,
                                                   dec_digit_t ans[]) noexcept {
                dec_digit_t* ans_store_ptr = ans;
                for (dec_size_type j = 0; j < m_size; ans_store_ptr++, j++) {
                    const double_dec_digit_t b_j = m_digits[j];
                    double_dec_digit_t carry = 0;
                    for (dec_size_type i = 0; i < k_size; i++) {
                        const double_dec_digit_t a_i = k_digits[i];
                        const double_dec_digit_t res =
                            a_i * b_j + double_dec_digit_t{ans_store_ptr[i]} + carry;
                        ans_store_ptr[i] = static_cast<dec_digit_t>(res % kDecimalBase);
                        carry = res / kDecimalBase;
                    }
                    ans_store_ptr[k_size] = static_cast<dec_digit_t>(carry % kDecimalBase);
                }
            }
        };

        struct DecFFT final {
            ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
            ATTRIBUTE_SIZED_ACCESS(read_only, 3, 4)
            ATTRIBUTE_NONNULL_ALL_ARGS
            ATTRIBUTE_ALWAYS_INLINE
            static void multiply_and_store_to(const dec_digit_t m_digits[],
                                              const dec_size_type m,
                                              const dec_digit_t k_digits[],
                                              const dec_size_type k,
                                              Decimal& product_result) {
                const DecFFT prod(m_digits, m, k_digits, k);
                prod.multiply_and_store_to_impl(product_result);
            }

            ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
            ATTRIBUTE_NONNULL_ALL_ARGS
            ATTRIBUTE_ALWAYS_INLINE
            static void square_and_store_to(const dec_digit_t digits[],
                                            const dec_size_type digits_size,
                                            Decimal& product_result) {
                const DecFFT prod(digits, digits_size);
                prod.multiply_and_store_to_impl(product_result);
            }

        private:
            ATTRIBUTE_SIZED_ACCESS(read_only, 2, 3)
            ATTRIBUTE_SIZED_ACCESS(read_only, 4, 5)
            ATTRIBUTE_NONNULL_ALL_ARGS
            DecFFT(const dec_digit_t m_digits[],
                   const dec_size_type m,
                   const dec_digit_t k_digits[],
                   const dec_size_type k)
                : product_size_(check_size_for_fft(m + k))
                , poly_size_(polys_size(product_size_))
                , poly_(create_and_fill_polynomials(m_digits, m, k_digits, k, poly_size_)) {}

            ATTRIBUTE_SIZED_ACCESS(read_only, 2, 3)
            ATTRIBUTE_NONNULL_ALL_ARGS
            DecFFT(const dec_digit_t digits[], const dec_size_type digits_size)
                : product_size_(check_size_for_fft(digits_size + digits_size))
                , poly_size_(polys_size(product_size_))
                , poly_(create_and_fill_polynomials(digits, digits_size, poly_size_)) {}

            DecFFT(const DecFFT&) = delete;
            DecFFT(DecFFT&&) = delete;
            DecFFT& operator=(const DecFFT&) = delete;
            DecFFT& operator=(DecFFT&&) = delete;

            ~DecFFT() {
                deallocate_polynomials(poly_);
            }

            static constexpr std::size_t kComplexNumsPerOneDecNum = 3;
            static_assert(math_functions::bin_pow(kFFTDecimalBase, kComplexNumsPerOneDecNum) ==
                              kDecimalBase,
                          "");
            static constexpr dec_size_type kMaxDecFFTSize =
                std::numeric_limits<dec_size_type>::max() / kComplexNumsPerOneDecNum / 2;

            void multiply_and_store_to_impl(Decimal& product_result) const {
                LONGINT_ASSERT_ASSUME(product_size_ <= kMaxDecFFTSize);
                product_result.digits_.resize(product_size_);
                fft::forward_backward_fft(lhs_poly(), rhs_poly(), poly_size());
                convert_fft_poly_to_decimal_digits(rhs_poly(), product_result.digits_.data(),
                                                   product_size_);
            }

            ATTRIBUTE_ALWAYS_INLINE
            [[nodiscard]]
            static dec_size_type check_size_for_fft(const dec_size_type value) {
                if (unlikely(value > kMaxDecFFTSize)) {
                    throw_size_error(LONGINT_FILE_LOCATION(), value, kMaxDecFFTSize);
                }

                return value;
            }

            ATTRIBUTE_CONST
            ATTRIBUTE_ALWAYS_INLINE
            [[nodiscard]]
            static constexpr dec_size_type polys_size(const dec_size_type size_value) noexcept {
                LONGINT_ASSERT_ASSUME(size_value <= kMaxDecFFTSize);
                static_assert(3 * kMaxDecFFTSize > kMaxDecFFTSize);
                static_assert(
                    3 * kMaxDecFFTSize <
                    math_functions::nearest_greater_equal_power_of_two(3 * kMaxDecFFTSize));
                static_assert(math_functions::nearest_greater_equal_power_of_two(
                                  3 * kMaxDecFFTSize) <= std::numeric_limits<dec_size_type>::max());
                return math_functions::nearest_greater_equal_power_of_two(3 * size_value);
            }

            ATTRIBUTE_RETURNS_NONNULL
            ATTRIBUTE_ALWAYS_INLINE
            [[nodiscard]]
            static fft::complex* allocate_polynomials(const dec_size_type size_value) {
                // Allocate n for the first polynomial
                //  and n for the second one
                return static_cast<fft::complex*>(
                    ::operator new(size_value * 2 * sizeof(fft::complex)));
            }

            ATTRIBUTE_ALWAYS_INLINE
            static void deallocate_polynomials(fft::complex* const poly) noexcept {
                ::operator delete(static_cast<void*>(poly));
            }

            ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
            ATTRIBUTE_SIZED_ACCESS(read_only, 3, 4)
            ATTRIBUTE_SIZED_ACCESS(write_only, 6, 5)
            ATTRIBUTE_NONNULL_ALL_ARGS
            static void fill_polynomial(const dec_digit_t m_digits[],
                                        const dec_size_type m,
                                        const dec_digit_t k_digits[],
                                        const dec_size_type k,
                                        const dec_size_type n,
                                        fft::complex* RESTRICT_QUALIFIER p) noexcept {
                static_assert(kComplexNumsPerOneDecNum == 3);
                for (dec_size_type i = 0; i < m; i++) {
                    dec_digit_t m_value = m_digits[i];
                    dec_digit_t k_value = k_digits[i];
                    dec_digit_t r1 = m_value % kFFTDecimalBase;
                    m_value /= kFFTDecimalBase;
                    dec_digit_t r2 = k_value % kFFTDecimalBase;
                    k_value /= kFFTDecimalBase;
                    *p = fft::complex{
                        static_cast<double>(r1),
                        static_cast<double>(r2),
                    };
                    p++;
                    r1 = m_value % kFFTDecimalBase;
                    m_value /= kFFTDecimalBase;
                    r2 = k_value % kFFTDecimalBase;
                    k_value /= kFFTDecimalBase;
                    *p = fft::complex{
                        static_cast<double>(r1),
                        static_cast<double>(r2),
                    };
                    p++;
                    *p = fft::complex{
                        static_cast<double>(m_value),
                        static_cast<double>(k_value),
                    };
                    p++;
                }
                for (dec_size_type i = m; i < k; i++) {
                    dec_digit_t k_value = k_digits[i];
                    dec_digit_t r2 = k_value % kFFTDecimalBase;
                    k_value /= kFFTDecimalBase;
                    *p = fft::complex{
                        0.0,
                        static_cast<double>(r2),
                    };
                    p++;
                    r2 = k_value % kFFTDecimalBase;
                    k_value /= kFFTDecimalBase;
                    *p = fft::complex{
                        0.0,
                        static_cast<double>(r2),
                    };
                    p++;
                    *p = fft::complex{
                        0.0,
                        static_cast<double>(k_value),
                    };
                    p++;
                }

                std::memset(static_cast<void*>(p), 0, (n - 3 * k) * sizeof(fft::complex));
            }

            ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
            ATTRIBUTE_SIZED_ACCESS(write_only, 4, 3)
            ATTRIBUTE_NONNULL_ALL_ARGS
            static void fill_polynomial(const dec_digit_t* RESTRICT_QUALIFIER digits,
                                        const dec_size_type digits_size,
                                        const dec_size_type n,
                                        fft::complex* RESTRICT_QUALIFIER p) noexcept {
                static_assert(kComplexNumsPerOneDecNum == 3);
                for (dec_size_type i = 0; i < digits_size; i++) {
                    dec_digit_t value = digits[i];
                    dec_digit_t r1 = value % kFFTDecimalBase;
                    value /= kFFTDecimalBase;
                    *p = fft::complex{
                        static_cast<double>(r1),
                        static_cast<double>(r1),
                    };
                    p++;
                    r1 = value % kFFTDecimalBase;
                    value /= kFFTDecimalBase;
                    *p = fft::complex{
                        static_cast<double>(r1),
                        static_cast<double>(r1),
                    };
                    p++;
                    *p = fft::complex{
                        static_cast<double>(value),
                        static_cast<double>(value),
                    };
                    p++;
                }

                std::memset(static_cast<void*>(p), 0,
                            (n - kComplexNumsPerOneDecNum * digits_size) * sizeof(fft::complex));
            }

            ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
            ATTRIBUTE_SIZED_ACCESS(read_only, 3, 4)
            ATTRIBUTE_NONNULL_ALL_ARGS
            ATTRIBUTE_RETURNS_NONNULL
            ATTRIBUTE_ALWAYS_INLINE
            [[nodiscard]]
            static fft::complex* create_and_fill_polynomials(const dec_digit_t m_digits[],
                                                             const dec_size_type m,
                                                             const dec_digit_t k_digits[],
                                                             const dec_size_type k,
                                                             const dec_size_type n) {
                fft::complex* const polys = allocate_polynomials(n);
                assert(polys != nullptr);
                fill_polynomial(m_digits, m, k_digits, k, n, polys);
                return polys;
            }

            ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
            ATTRIBUTE_NONNULL_ALL_ARGS
            ATTRIBUTE_RETURNS_NONNULL
            ATTRIBUTE_ALWAYS_INLINE
            [[nodiscard]]
            static fft::complex* create_and_fill_polynomials(const dec_digit_t digits[],
                                                             const dec_size_type digits_size,
                                                             const dec_size_type n) {
                fft::complex* const polys = allocate_polynomials(n);
                assert(polys != nullptr);
                fill_polynomial(digits, digits_size, n, polys);
                return polys;
            }

            ATTRIBUTE_ACCESS(read_only, 1)
            ATTRIBUTE_SIZED_ACCESS(write_only, 2, 3)
            ATTRIBUTE_NONNULL_ALL_ARGS
            static void convert_fft_poly_to_decimal_digits(
                const fft::complex* RESTRICT_QUALIFIER poly,
                dec_digit_t* RESTRICT_QUALIFIER digits,
                const dec_size_type digits_size) noexcept {
                LONGINT_ASSERT_ASSUME(digits_size > 0);

                static_assert(sizeof(dec_digit_t) == sizeof(uint32_t));
                double_dec_digit_t carry = 0;
                const dec_digit_t* const digits_end = digits + digits_size;
                do {
                    double_dec_digit_t res = carry;
                    res += static_cast<double_dec_digit_t>(poly->real() + kFFTFloatRoundError);
                    poly++;
                    res += static_cast<double_dec_digit_t>(poly->real() + kFFTFloatRoundError) *
                           kFFTDecimalBase;
                    poly++;
                    res += static_cast<double_dec_digit_t>(poly->real() + kFFTFloatRoundError) *
                           (kFFTDecimalBase * kFFTDecimalBase);
                    poly++;
                    *digits = static_cast<dec_digit_t>(res % kDecimalBase);
                    carry = res / kDecimalBase;
                    digits++;
                } while (digits < digits_end);
                assert(carry == 0);
            }

            ATTRIBUTE_PURE
            ATTRIBUTE_RETURNS_NONNULL
            ATTRIBUTE_ALWAYS_INLINE
            [[nodiscard]]
            constexpr fft::complex* lhs_poly() const noexcept ATTRIBUTE_LIFETIME_BOUND {
                return poly_;
            }

            ATTRIBUTE_PURE
            ATTRIBUTE_RETURNS_NONNULL
            ATTRIBUTE_ALWAYS_INLINE
            [[nodiscard]]
            constexpr fft::complex* rhs_poly() const noexcept ATTRIBUTE_LIFETIME_BOUND {
                return poly_ + poly_size();
            }

            ATTRIBUTE_PURE
            ATTRIBUTE_ALWAYS_INLINE
            [[nodiscard]] constexpr dec_size_type poly_size() const noexcept {
                const auto value = poly_size_;
                LONGINT_ASSERT_ASSUME(math_functions::is_power_of_two(value));
                return value;
            }

            dec_size_type const product_size_;
            dec_size_type const poly_size_;
            fft::complex* const poly_;
        };

        CONSTEXPR_VECTOR void assign_uint32(const uint32_t n) {
            const dec_digit_t low = n % kDecimalBase;
            const dec_digit_t hi = n / kDecimalBase;
            digits_.resize(4);
            digits_[0] = low;
            digits_[1] = hi;
            const size_t real_size = [&]() {
                if (hi != 0) {
                    return size_t{2};
                }
                if (low != 0) {
                    return size_t{1};
                }
                return size_t{0};
            }();
            digits_.resize(real_size);
        }

        CONSTEXPR_VECTOR void assign_uint64(const uint64_t n) {
            const auto low = static_cast<dec_digit_t>(n % kDecimalBase);
            const uint64_t t = n / kDecimalBase;
            const auto mid = static_cast<dec_digit_t>(t % kDecimalBase);
            const auto hi = static_cast<dec_digit_t>(t / kDecimalBase);
            digits_.resize(4);
            digits_[0] = low;
            digits_[1] = mid;
            digits_[2] = hi;
            const size_t real_size = [&]() {
                if (hi != 0) {
                    return size_t{3};
                }
                if (mid != 0) {
                    return size_t{2};
                }
                if (low != 0) {
                    return size_t{1};
                }
                return size_t{0};
            }();
            digits_.resize(real_size);
        }
    };

private:
#if defined(HAS_CUSTOM_LONGINT_ALLOCATOR)
    static constexpr bool kUseCustomLongIntAllocator = false;
#else
    static constexpr bool kUseCustomLongIntAllocator = false;
#endif

    struct LongIntNaive final {
        ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
        ATTRIBUTE_SIZED_ACCESS(read_only, 3, 4)
        ATTRIBUTE_ACCESS(read_write, 5)
        static constexpr void multiply_and_store_to(const digit_t m_ptr[],
                                                    const size_type m,
                                                    const digit_t k_ptr[],
                                                    const size_type k,
                                                    digit_t* const ans) noexcept {
            LONGINT_ASSERT_ASSUME(m <= k);
            digit_t* ans_store_ptr = ans;
            for (size_type j = 0; j < m; ans_store_ptr++, j++) {
                const double_digit_t b_j = m_ptr[j];
                double_digit_t carry = 0;
                for (size_type i = 0; i < k; i++) {
                    const double_digit_t a_i = k_ptr[i];
                    const double_digit_t res = a_i * b_j + double_digit_t{ans_store_ptr[i]} + carry;
                    ans_store_ptr[i] = static_cast<digit_t>(res);
                    carry = res / kNumsBase;
                }

                ans_store_ptr[k] = static_cast<digit_t>(carry);
            }
        }
    };

    struct LongIntFFT final {
        using poly_size_type = std::size_t;

        struct FFTParams final {
            poly_size_type poly_size;
            bool need_high_precision;
        };

        ATTRIBUTE_NODISCARD_WITH_MESSAGE("impl error in fft longint product")
        ATTRIBUTE_ALWAYS_INLINE
        ATTRIBUTE_CONST
        static constexpr FFTParams compute_fft_product_params(
            const size_type product_size) noexcept {
            static_assert(max_size() * 2 > max_size());
            static_assert(2 * math_functions::nearest_greater_equal_power_of_two(max_size()) >=
                          2 * max_size());
            poly_size_type n = 2 * math_functions::nearest_greater_equal_power_of_two(product_size);
            const bool need_high_precision = n > kFFTPrecisionBorder;
            if (need_high_precision) {
                n *= 2;
            }
            LONGINT_ASSERT_ASSUME(math_functions::is_power_of_two(n));
            return {n, need_high_precision};
        }

        ATTRIBUTE_NONNULL_ALL_ARGS
        ATTRIBUTE_ACCESS(read_only, 2)
        ATTRIBUTE_SIZED_ACCESS(write_only, 3, 4)
        static void convert_fft_poly_to_longint_nums(const bool is_high_precision,
                                                     const fft::complex* RESTRICT_QUALIFIER poly,
                                                     digit_t* RESTRICT_QUALIFIER nums,
                                                     const size_type nums_size) {
            LONGINT_ASSERT_ASSUME(nums_size > 0);

            static_assert(sizeof(digit_t) == sizeof(uint32_t));
            double_digit_t carry = 0;
            const digit_t* const nums_end = nums + nums_size;
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
                } while (nums < nums_end);
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
                } while (nums < nums_end);
            }
            assert(carry == 0);
        }

        ATTRIBUTE_NONNULL_ALL_ARGS
        ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
        ATTRIBUTE_SIZED_ACCESS(read_only, 3, 4)
        ATTRIBUTE_SIZED_ACCESS(write_only, 5, 6)
        static void convert_longint_nums_to_fft_poly(const digit_t m_ptr[],
                                                     const size_type m,
                                                     const digit_t k_ptr[],
                                                     const size_type k,
                                                     fft::complex* p,
                                                     const poly_size_type n,
                                                     const bool need_high_precision) noexcept {
            LONGINT_ASSERT_ASSUME(0 < m);
            LONGINT_ASSERT_ASSUME(m <= k);
            LONGINT_ASSERT_ASSUME(m <= max_size());
            LONGINT_ASSERT_ASSUME(k <= max_size());
            LONGINT_ASSERT_ASSUME(m + k <= max_size());
            LONGINT_ASSERT_ASSUME(m + k <= n);
            LONGINT_ASSERT_ASSUME(need_high_precision || n <= kFFTPrecisionBorder);
            LONGINT_ASSERT_ASSUME(!need_high_precision || n > kFFTPrecisionBorder * 2);
            LONGINT_ASSERT_ASSUME(math_functions::is_power_of_two(n));

            static_assert(kDigitBits == 32);
            if (likely(!need_high_precision)) {
                for (size_type i = 0; i < m; i++) {
                    digit_t m_value = m_ptr[i];
                    digit_t k_value = k_ptr[i];

                    *p = fft::complex{
                        static_cast<double>(static_cast<uint16_t>(m_value)),
                        static_cast<double>(static_cast<uint16_t>(k_value)),
                    };
                    p++;
                    m_value >>= 16;
                    k_value >>= 16;
                    *p = fft::complex{
                        static_cast<double>(static_cast<uint16_t>(m_value)),
                        static_cast<double>(static_cast<uint16_t>(k_value)),
                    };
                    p++;
                }
                for (size_type i = m; i < k; i++) {
                    const digit_t k_value = k_ptr[i];

                    *p = fft::complex{
                        0.0,
                        static_cast<double>(k_value & 0xFFFF),
                    };
                    p++;
                    *p = fft::complex{
                        0.0,
                        static_cast<double>(k_value >> 16),
                    };
                    p++;
                }
            } else {
                for (size_type i = 0; i < m; i++) {
                    digit_t m_value = m_ptr[i];
                    digit_t k_value = k_ptr[i];

                    *p = fft::complex{
                        static_cast<double>(static_cast<uint8_t>(m_value)),
                        static_cast<double>(static_cast<uint8_t>(k_value)),
                    };
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = fft::complex{
                        static_cast<double>(static_cast<uint8_t>(m_value)),
                        static_cast<double>(static_cast<uint8_t>(k_value)),
                    };
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = fft::complex{
                        static_cast<double>(static_cast<uint8_t>(m_value)),
                        static_cast<double>(static_cast<uint8_t>(k_value)),
                    };
                    p++;
                    m_value >>= 8;
                    k_value >>= 8;
                    *p = fft::complex{
                        static_cast<double>(static_cast<uint8_t>(m_value)),
                        static_cast<double>(static_cast<uint8_t>(k_value)),
                    };
                    p++;
                }
                for (size_type i = m; i < k; i++) {
                    digit_t k_value = k_ptr[i];

                    *p = fft::complex{
                        0.0,
                        static_cast<double>(static_cast<uint8_t>(k_value)),
                    };
                    p++;
                    k_value >>= 8;
                    *p = fft::complex{
                        0.0,
                        static_cast<double>(static_cast<uint8_t>(k_value)),
                    };
                    p++;
                    k_value >>= 8;
                    *p = fft::complex{
                        0.0,
                        static_cast<double>(static_cast<uint8_t>(k_value)),
                    };
                    p++;
                    k_value >>= 8;
                    *p = fft::complex{
                        0.0,
                        static_cast<double>(static_cast<uint8_t>(k_value)),
                    };
                    p++;
                }
            }
            size_type complex_nums_filled = 2 * k;
            if (need_high_precision) {
                complex_nums_filled *= 2;
            }
            std::memset(static_cast<void*>(p), 0, (n - complex_nums_filled) * sizeof(fft::complex));
        }

        ATTRIBUTE_NONNULL_ALL_ARGS
        ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
        ATTRIBUTE_SIZED_ACCESS(write_only, 3, 4)
        static void convert_longint_nums_to_fft_poly(const digit_t nums_ptr[],
                                                     const size_type nums_size,
                                                     fft::complex* p,
                                                     const poly_size_type n,
                                                     bool need_high_precision) noexcept {
            LONGINT_ASSERT_ASSUME(0 < nums_size);
            LONGINT_ASSERT_ASSUME(nums_size <= max_size());
            LONGINT_ASSERT_ASSUME(nums_size * 2 <= n);
            LONGINT_ASSERT_ASSUME(need_high_precision || n <= kFFTPrecisionBorder);
            LONGINT_ASSERT_ASSUME(!need_high_precision || n > kFFTPrecisionBorder * 2);
            LONGINT_ASSERT_ASSUME(math_functions::is_power_of_two(n));

            static_assert(kDigitBits == 32);
            if (likely(!need_high_precision)) {
                for (size_type i = 0; i < nums_size; i++) {
                    digit_t value = nums_ptr[i];

                    *p = fft::complex{static_cast<double>(static_cast<uint16_t>(value)),
                                      static_cast<double>(static_cast<uint16_t>(value))};
                    p++;
                    value >>= 16;
                    *p = fft::complex{static_cast<double>(static_cast<uint16_t>(value)),
                                      static_cast<double>(static_cast<uint16_t>(value))};
                    p++;
                }
            } else {
                for (size_type i = 0; i < nums_size; i++) {
                    digit_t value = nums_ptr[i];

                    *p = fft::complex{static_cast<double>(static_cast<uint8_t>(value)),
                                      static_cast<double>(static_cast<uint8_t>(value))};
                    p++;
                    value >>= 8;
                    *p = fft::complex{static_cast<double>(static_cast<uint8_t>(value)),
                                      static_cast<double>(static_cast<uint8_t>(value))};
                    p++;
                    value >>= 8;
                    *p = fft::complex{static_cast<double>(static_cast<uint8_t>(value)),
                                      static_cast<double>(static_cast<uint8_t>(value))};
                    p++;
                    value >>= 8;
                    *p = fft::complex{static_cast<double>(static_cast<uint8_t>(value)),
                                      static_cast<double>(static_cast<uint8_t>(value))};
                    p++;
                }
            }
            size_type complex_nums_filled = (2 * nums_size);
            if (need_high_precision) {
                complex_nums_filled *= 2;
            }
            std::memset(static_cast<void*>(p), 0, (n - complex_nums_filled) * sizeof(fft::complex));
        }
    };

    static void check_dec_str(std::string_view str) {
        constexpr auto is_digit = [](const char chr) constexpr noexcept {
            const std::uint32_t widen{static_cast<unsigned char>(chr)};
            return widen - '0' <= '9' - '0';
        };
        if (!str.empty() && str.front() == '-') {
            str.remove_prefix(1);
        }
        if (unlikely(str.empty() || !all_of(str, is_digit))) {
            throw_on_invalid_dec_str(LONGINT_FILE_LOCATION(), str);
        }
    }

    template <typename P>
    [[nodiscard]] static bool all_of(const std::string_view str, P pred) noexcept(
        std::is_nothrow_invocable_r_v<bool, P, char>) {
        static_assert(std::is_invocable_r_v<bool, P, char>);
        bool ret = true;
        for (const char c : str) {
            ret &= pred(c);
        }

        return ret;
    }

    ATTRIBUTE_COLD
    [[noreturn]] static void throw_on_invalid_dec_str(const char* const file_location,
                                                      const char* const function_name,
                                                      const std::string_view str) {
        throw std::invalid_argument{misc::join_strings(
            "Can't convert string '", str, "' to longint at ", file_location, ' ', function_name)};
    }

    ATTRIBUTE_SIZED_ACCESS(read_only, 2, 3)
    inline void set_dec_str_impl(const unsigned char* str, const std::size_t str_size);

    [[nodiscard]]
    ATTRIBUTE_PURE constexpr int64_t mod_by_power_of_2_ge_2_impl(const uint32_t n) const noexcept {
        LONGINT_ASSERT_ASSUME((n & (n - 1)) == 0);
        LONGINT_ASSERT_ASSUME(n >= 2);
        static_assert(kDigitBits >= 32);
        const uint32_t remainder{size_ == 0 ? digit_t{0} : nums_[0] & (n - 1)};
        return is_negative() ? -int64_t{remainder} : int64_t{remainder};
    }

    void divmod_impl(const longint& other, longint& rem) {
        /**
         * See Hackers Delight 9-2.
         */
        const size_type m = usize();
        const size_type n = other.usize();
        if (m < n) {
            rem = std::move(*this);
            this->assign_zero();
            return;
        }

        const ssize_type sign_product = size() ^ other.size();
        switch (n) {
            case 0: {
                /* Quite return when dividing by zero. */
                return;
            }
            case 1: {
                if (other.is_negative()) {
                    flip_sign();
                }
                rem = this->divmod(other[0]);
                if (other.is_negative()) {
                    rem.flip_sign();
                }
                return;
            }
            default: {
                break;
            }
        }

        rem.reserveUninitializedWithoutCopy(n);

        // Normilize by shifting v left just enough so that
        // its high-order bit i on, and shift u left the
        // same amount. We may have to append a high-order
        // digit on the dividend; we do that unconditionally (un size = m + -> 1 <-).

        const size_type vn_and_un_size = check_size(std::size_t{n} + std::size_t{m} + 1);
        digit_t* const vn_and_un = allocate_uninitialized(vn_and_un_size);
        digit_t* const vn = vn_and_un;
        digit_t* const un = vn_and_un + n;

        const digit_t* const u = nums_;
        const digit_t* const v = other.nums_;
        const digit_t last_v_num = v[n - 1];
        LONGINT_ASSERT_ASSUME(last_v_num > 0);
        static_assert(kDigitBits == 32);
        // 0 <= s < kNumsBits
        const auto s = static_cast<std::uint32_t>(math_functions::countl_zero(last_v_num));
        longint::divmod_normalize_vn(vn, v, n, s);
        LONGINT_ASSERT_ASSUME(vn[n - 1] >= digit_t{1} << (kDigitBits - 1));
        longint::divmod_normalize_un(un, u, m, m + 1, s);
        longint::divmod_impl_unchecked(
            /* un = */ un,
            /* un_size = */ m + 1,
            /* vn = */ vn,
            /* vn_size = */ n,
            /* quot = */ nums_);
        // Unnormalize remainder
        longint::divmod_unnormalize_remainder(rem.nums_, un, n, n + 1, s);
        deallocate(vn_and_un, vn_and_un_size);
        rem.size_ = static_cast<ssize_type>(n);
        rem.pop_leading_zeros();
        set_ssize_from_size_and_sign(m - n + 1, /* sign = */ sign_product);
        pop_leading_zeros();
    }

    ATTRIBUTE_SIZED_ACCESS(read_write, 1, 2)
    ATTRIBUTE_SIZED_ACCESS(read_only, 3, 4)
    ATTRIBUTE_ACCESS(write_only, 5)
    ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ALWAYS_INLINE
    static constexpr void divmod_impl_unchecked(digit_t* RESTRICT_QUALIFIER const un,
                                                const size_type un_size,
                                                const digit_t* RESTRICT_QUALIFIER const vn,
                                                const size_type vn_size,
                                                digit_t* RESTRICT_QUALIFIER const quot) noexcept {
        LONGINT_ASSERT_ASSUME(vn_size >= 2);
        LONGINT_ASSERT_ASSUME(un_size > vn_size);
        for (size_type j = un_size - vn_size - 1; static_cast<ssize_type>(j) >= 0; j--) {
            // Compute estimate qhat of q[j].
            const double_digit_t cur =
                (double_digit_t{un[j + vn_size]} << kDigitBits) | un[j + vn_size - 1];
            const digit_t last_vn = vn[vn_size - 1];
            LONGINT_ASSERT_ASSUME(last_vn >= digit_t{1} << (kDigitBits - 1));
            double_digit_t qhat = cur / last_vn;
            double_digit_t rhat = cur % last_vn;
            LONGINT_ASSERT_ASSUME(qhat * last_vn + rhat == cur);
            while (qhat >= kNumsBase ||
                   qhat * vn[vn_size - 2] > kNumsBase * rhat + un[j + vn_size - 2]) {
                qhat--;
                rhat += last_vn;
                LONGINT_ASSERT_ASSUME(qhat * last_vn + rhat == cur);
                if (rhat >= kNumsBase) {
                    break;
                }
            }
            LONGINT_ASSERT_ASSUME(qhat * last_vn + rhat == cur);
            // Multiply and subtract
            double_digit_t t = divmod_mult_sub(un + j, vn, vn_size, qhat);
            quot[j] = static_cast<digit_t>(qhat);  // Store quotient digit
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
                                                    const size_type vn_size,
                                                    const double_digit_t qhat) noexcept {
        LONGINT_ASSERT_ASSUME(vn_size >= 2);
        double_digit_t carry = 0;
        for (size_type i = 0; i < vn_size; i++) {
            double_digit_t p = qhat * vn[i];
            double_digit_t t = double_digit_t{un[i]} - carry - static_cast<digit_t>(p % kNumsBase);
            un[i] = static_cast<digit_t>(t % kNumsBase);
            carry = static_cast<digit_t>((p / kNumsBase) - (t / kNumsBase));
        }
        double_digit_t t = un[vn_size] - carry;
        un[vn_size] = static_cast<digit_t>(t);
        return t;
    }

    ATTRIBUTE_ACCESS(read_write, 1)
    ATTRIBUTE_SIZED_ACCESS(read_only, 2, 3)
    ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ALWAYS_INLINE
    static constexpr void divmod_add_back(digit_t* RESTRICT_QUALIFIER const un,
                                          const digit_t* RESTRICT_QUALIFIER const vn,
                                          const size_type vn_size) noexcept {
        LONGINT_ASSERT_ASSUME(vn_size >= 2);
        double_digit_t carry = 0;
        for (size_type i = 0; i < vn_size; i++) {
            double_digit_t t = double_digit_t{un[i]} + double_digit_t{vn[i]} + carry;
            un[i] = static_cast<digit_t>(t % kNumsBase);
            carry = t / kNumsBase;
        }
        un[vn_size] += static_cast<digit_t>(carry);
    }

    ATTRIBUTE_SIZED_ACCESS(write_only, 1, 3)
    ATTRIBUTE_SIZED_ACCESS(read_only, 2, 3)
    ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ALWAYS_INLINE
    static constexpr void divmod_normalize_vn(digit_t vn[],
                                              const digit_t v[],
                                              size_type n,
                                              std::uint32_t s) noexcept {
        LONGINT_ASSERT_ASSUME(n > 1);
        LONGINT_ASSERT_ASSUME(s < 32);
        for (size_type i = n - 1; i > 0; i--) {
            vn[i] =
                (v[i] << s) | static_cast<digit_t>(double_digit_t{v[i - 1]} >> (kDigitBits - s));
        }
        vn[0] = v[0] << s;
    }

    ATTRIBUTE_SIZED_ACCESS(write_only, 1, 4)
    ATTRIBUTE_SIZED_ACCESS(read_only, 2, 3)
    ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ALWAYS_INLINE
    static constexpr void divmod_normalize_un(digit_t un[],
                                              const digit_t u[],
                                              size_type m,
                                              ATTRIBUTE_MAYBE_UNUSED size_type m_plus_one,
                                              std::uint32_t s) noexcept {
        LONGINT_ASSERT_ASSUME(m > 1);
        LONGINT_ASSERT_ASSUME(s < 32);
        LONGINT_ASSERT_ASSUME(m + 1 == m_plus_one);
        un[m] = static_cast<digit_t>(double_digit_t{u[m - 1]} >> (kDigitBits - s));
        for (size_type i = m - 1; i > 0; i--) {
            un[i] =
                (u[i] << s) | static_cast<digit_t>(double_digit_t{u[i - 1]} >> (kDigitBits - s));
        }
        un[0] = u[0] << s;
    }

    ATTRIBUTE_SIZED_ACCESS(write_only, 1, 3)
    ATTRIBUTE_SIZED_ACCESS(read_only, 2, 4)
    ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ALWAYS_INLINE
    static constexpr void divmod_unnormalize_remainder(digit_t rem[],
                                                       const digit_t un[],
                                                       size_type n,
                                                       ATTRIBUTE_MAYBE_UNUSED size_type n_plus_one,
                                                       std::uint32_t s) noexcept {
        LONGINT_ASSERT_ASSUME(n > 1);
        LONGINT_ASSERT_ASSUME(s < 32);
        LONGINT_ASSERT_ASSUME(n + 1 == n_plus_one);
        for (size_type i = 0; i < n; i++) {
            rem[i] =
                (un[i] >> s) | static_cast<digit_t>(double_digit_t{un[i + 1]} << (kDigitBits - s));
        }
    }

    ATTRIBUTE_ALWAYS_INLINE
    constexpr void set_ssize_from_size(const size_type size) noexcept {
        this->set_ssize_from_size_and_sign(size, /* sign = */ size_);
    }

    ATTRIBUTE_ALWAYS_INLINE
    constexpr void set_ssize_from_size_and_sign(const size_type size,
                                                const ssize_type sign) noexcept {
        static_assert(sizeof(ssize_type) == sizeof(size_type));
        LONGINT_ASSERT_ASSUME(size <= max_size());
        size_ = static_cast<ssize_type>(sign >= 0 ? size : -size);
    }

    constexpr void pop_leading_zeros() noexcept {
        auto usize_value = usize();
        while (usize_value > 0 && nums_[usize_value - 1] == 0) {
            usize_value--;
        }

        set_ssize_from_size(usize_value);
    }

    void reserveUninitializedWithoutCopy(const size_type capacity) {
        size_ = 0;
        if (capacity > capacity_) {
            deallocate(nums_, capacity_);
            nums_ = nullptr;
            capacity_ = 0;
            nums_ = allocate_uninitialized(capacity);
            capacity_ = capacity;
        }
    }

    [[nodiscard]] static std::vector<Decimal> create_initial_conv_bin_base_pows() {
        std::vector<Decimal> pows;
        pows.emplace_back(longint::kNumsBase);
        return pows;
    }

    static inline std::vector<Decimal> conv_bin_base_pows =
        longint::create_initial_conv_bin_base_pows();

    friend struct longint_detail::longint_static_storage;

    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_SIZED_ACCESS(read_write, 1, 2)
    ATTRIBUTE_SIZED_ACCESS(read_write, 4, 2)
    ATTRIBUTE_ACCESS(read_write, 5)
    static void convert_dec_base_mult_add(digit_t conv_digits[],
                                          const size_type conv_len,
                                          const longint& conv_base_pow,
                                          digit_t mult_add_buffer[],
                                          fft::complex fft_poly_buffer[]) {
        LONGINT_ASSERT_ASSUME(0 < conv_base_pow.size_);
        const size_type m_size = conv_base_pow.usize();
        const digit_t* const m_ptr = conv_base_pow.nums_;
        assert(0 < m_size && m_size <= conv_len / 2);
        convert_dec_base_mult_add_impl(conv_digits, conv_len, m_ptr, m_size, mult_add_buffer,
                                       fft_poly_buffer);
    }

    ATTRIBUTE_SIZED_ACCESS(read_write, 1, 2)
    ATTRIBUTE_SIZED_ACCESS(read_only, 3, 4)
    ATTRIBUTE_SIZED_ACCESS(read_write, 5, 2)
    ATTRIBUTE_ACCESS(read_write, 6)
    static void convert_dec_base_mult_add_impl(digit_t conv_digits[],
                                               const size_type conv_len,
                                               const digit_t m_ptr[],
                                               const size_type m_size,
                                               digit_t mult_add_buffer[],
                                               fft::complex fft_poly_buffer[]) {
        const size_type half_conv_len = conv_len / 2;
        LONGINT_ASSERT_ASSUME(0 < m_size);
        LONGINT_ASSERT_ASSUME(m_size <= half_conv_len);
        LONGINT_ASSERT_ASSUME(math_functions::is_power_of_two(half_conv_len));
        LONGINT_ASSERT_ASSUME(conv_len <= max_size());
        const digit_t* num_hi = conv_digits + half_conv_len;
        static_assert(max_size() + max_size() > max_size());
        const size_type prod_size = m_size + half_conv_len;
        std::fill_n(mult_add_buffer, conv_len, digit_t{0});
        if (half_conv_len <= 32) {
            LongIntNaive::multiply_and_store_to(m_ptr, m_size, num_hi, half_conv_len,
                                                mult_add_buffer);
        } else {
            const auto [n, need_high_precision] = LongIntFFT::compute_fft_product_params(prod_size);
            fft::complex* const p1 = fft_poly_buffer;
            LongIntFFT::convert_longint_nums_to_fft_poly(m_ptr, m_size, num_hi, half_conv_len, p1,
                                                         n, need_high_precision);
            fft::complex* const p2 = p1 + n;
            fft::forward_backward_fft(p1, p2, n);
            LongIntFFT::convert_fft_poly_to_longint_nums(need_high_precision, p2, mult_add_buffer,
                                                         prod_size);
        }

        // Now mult_add_buffer == num_hi * CONV_BASE^half_len
        double_digit_t carry = 0;
        for (size_type i = half_conv_len; i > 0; i--, conv_digits++, mult_add_buffer++) {
            const double_digit_t res =
                double_digit_t{*conv_digits} + double_digit_t{*mult_add_buffer} + carry;
            *conv_digits = static_cast<digit_t>(res);
            carry = res >> kDigitBits;
        }
        for (size_type i = half_conv_len; i > 0; i--, conv_digits++, mult_add_buffer++) {
            const double_digit_t res = double_digit_t{*mult_add_buffer} + carry;
            *conv_digits = static_cast<digit_t>(res);
            carry = res >> kDigitBits;
        }
        assert(carry == 0);
    }

    ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
    [[nodiscard]]
    static Decimal convert_bin_base_impl(const digit_t nums[], const size_type size) {
        CONFIG_ASSUME_STATEMENT(math_functions::is_power_of_two(size));
        switch (size) {
            case 0:
            case 1: {
                return Decimal{nums[0]};
            }
            case 2: {
                return Decimal{double_digit_t{nums[1]} * kNumsBase | nums[0]};
            }
            default: {
                break;
            }
        }

        const Decimal low_dec = convert_bin_base_impl(nums, size / 2);
        Decimal high_dec = convert_bin_base_impl(nums + size / 2, size / 2);

        const uint32_t idx = math_functions::log2_floor(size) - 1;
        LONGINT_DEBUG_ASSERT(idx < conv_bin_base_pows.size());
        high_dec *= conv_bin_base_pows[idx];
        high_dec += low_dec;
        return high_dec;
    }

    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
    [[nodiscard]]
    static Decimal convert_bin_base(const digit_t nums[], const size_type size) {
        LONGINT_ASSERT_ASSUME(math_functions::is_power_of_two(size));
        return convert_bin_base_impl(nums, size);
    }

    static void ensure_bin_base_pows_capacity(const std::size_t pows_size) {
        std::size_t i = conv_bin_base_pows.size();
        if (i >= pows_size) {
            return;
        }
        conv_bin_base_pows.reserve(pows_size);
        CONFIG_ASSUME_STATEMENT(0 < i);
        do {
            conv_bin_base_pows.emplace_back();
            conv_bin_base_pows[i - 1].square_this_to(conv_bin_base_pows.back());
        } while (++i != pows_size);
    }

    ATTRIBUTE_NOINLINE void grow_capacity() {
        const size_type current_capacity = capacity();
        static_assert(max_size() * 2 > max_size());
        const size_type new_capacity = (current_capacity * 2) | (current_capacity == 0);
        LONGINT_ASSERT_ASSUME(capacity_ < new_capacity);
        reserve(new_capacity);
    }

    ATTRIBUTE_NODISCARD_WITH_MESSAGE("impl error")
    size_type set_size_at_least(size_type new_size) {
        size_type cur_size = usize();
        if (new_size <= cur_size) {
            return cur_size;
        }

        reserve(new_size);
        std::fill_n(std::addressof(nums_[cur_size]), new_size - cur_size, digit_t{0});
        set_ssize_from_size(new_size);
        return new_size;
    }

    void allocate_default_capacity_32() {
        nums_ = allocate_uninitialized(kDefaultLINumsCapacity32);
        capacity_ = kDefaultLINumsCapacity32;
    }
    void ensure_default_capacity_op_asgn_32() {
        if (capacity_ < kDefaultLINumsCapacity32) {
            deallocate(nums_, capacity_);
            allocate_default_capacity_32();
        }
    }
    constexpr void assign_u32_unchecked(const uint32_t n) noexcept {
        static_assert(kDigitBits >= 32);
        size_ = n != 0;
        nums_[0] = n;
    }
    constexpr void assign_i32_unchecked(const int32_t n) noexcept {
        static_assert(kDigitBits >= 32);
        size_ = math_functions::sign(n);
        nums_[0] = math_functions::uabs(n);
    }
    void allocate_default_capacity_64() {
        nums_ = allocate_uninitialized(kDefaultLINumsCapacity64);
        capacity_ = kDefaultLINumsCapacity64;
    }
    void ensure_default_capacity_op_asgn_64() {
        if (capacity_ < kDefaultLINumsCapacity64) {
            deallocate(nums_, capacity_);
            allocate_default_capacity_64();
        }
    }
    constexpr void assign_u64_unchecked(uint64_t n) noexcept {
        static_assert(kDigitBits == 32);
        size_ = n != 0;
        nums_[0] = static_cast<uint32_t>(n);
        n >>= 32;
        size_ += n != 0;
        nums_[1] = static_cast<uint32_t>(n);
    }
    constexpr void assign_i64_unchecked(int64_t n) noexcept {
        const std::int32_t sgn = math_functions::sign(n);
        this->assign_u64_unchecked(math_functions::uabs(n));
        size_ *= sgn;
    }
#if defined(INTEGERS_128_BIT_HPP)
    void allocate_default_capacity_128() {
        nums_ = allocate_uninitialized(kDefaultLINumsCapacity128);
        capacity_ = kDefaultLINumsCapacity128;
    }
    void ensure_default_capacity_op_asgn_128() {
        if (capacity_ < kDefaultLINumsCapacity128) {
            deallocate(nums_, capacity_);
            allocate_default_capacity_128();
        }
    }
    I128_CONSTEXPR void assign_u128_unchecked(uint128_t n) noexcept {
        static_assert(kDigitBits == 32);
        size_ = n != 0;
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
    I128_CONSTEXPR void assign_i128_unchecked(const int128_t n) noexcept {
        const std::int32_t sgn = math_functions::sign(n);
        assign_u128_unchecked(math_functions::uabs(n));
        size_ *= sgn;
    }
#endif

    ATTRIBUTE_SIZED_ACCESS(read_write, 1, 2)
    ATTRIBUTE_SIZED_ACCESS(read_only, 3, 4)
    ATTRIBUTE_NONNULL_ALL_ARGS
    static constexpr void longint_add_with_free_space(digit_t lhs[],
                                                      const size_type lhs_size,
                                                      const digit_t rhs[],
                                                      const size_type rhs_size) noexcept {
        LONGINT_ASSERT_ASSUME(lhs_size > rhs_size);

        double_digit_t carry = 0;
        const digit_t* const lhs_end = lhs + lhs_size;
        const digit_t* const rhs_end = rhs + rhs_size;
        for (; rhs != rhs_end; ++rhs, ++lhs) {
            const double_digit_t res = double_digit_t{*lhs} + double_digit_t{*rhs} + carry;
            *lhs = static_cast<digit_t>(res);
            carry = res / kNumsBase;
        }

        for (; carry != 0; ++lhs) {
            assert(lhs != lhs_end);
            const double_digit_t res = double_digit_t{*lhs} + carry;
            *lhs = static_cast<digit_t>(res);
            carry = res / kNumsBase;
        }
    }

    ATTRIBUTE_SIZED_ACCESS(read_write, 1, 2)
    ATTRIBUTE_SIZED_ACCESS(read_only, 3, 4)
    ATTRIBUTE_NONNULL_ALL_ARGS
    [[nodiscard]]
    static constexpr bool longint_subtract_with_free_space(digit_t lhs[],
                                                           const size_type lhs_size,
                                                           const digit_t rhs[],
                                                           const size_type rhs_size) noexcept {
        LONGINT_ASSERT_ASSUME(lhs_size >= rhs_size);

        const bool overflowed = longint_subtract_with_carry(lhs, lhs_size, rhs, rhs_size);
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
    [[nodiscard]]
    static constexpr bool longint_subtract_with_carry(digit_t lhs[],
                                                      const size_type lhs_size,
                                                      const digit_t rhs[],
                                                      const size_type rhs_size) noexcept {
        LONGINT_ASSERT_ASSUME(lhs_size >= rhs_size);

        const digit_t* const lhs_end = lhs + lhs_size;
        const digit_t* const rhs_end = rhs + rhs_size;
        bool carry = false;
        for (; rhs != rhs_end; ++rhs, ++lhs) {
            const digit_t lhs_val = *lhs;
            const auto sub_val = double_digit_t{*rhs} + double_digit_t{carry};
            const digit_t res_val = lhs_val - static_cast<digit_t>(sub_val);
            carry = lhs_val < sub_val;
            *lhs = res_val;
        }
        if (carry) {
            for (; lhs != lhs_end; ++lhs) {
                const digit_t lhs_val = *lhs;
                *lhs = lhs_val - 1;
                if (lhs_val > 0) {
                    return false;
                }
            }
        }

        return carry;
    }

    void nonZeroSizeAddUInt(const uint32_t n) {
        LONGINT_ASSERT_ASSUME(size() != 0);

        static_assert(sizeof(digit_t) >= sizeof(uint32_t));
        digit_t* it = nums_;
        const digit_t* end_iter = nums_ + usize();
        double_digit_t carry = n;
        do {
            const double_digit_t res = double_digit_t{*it} + carry;
            carry = res >> kDigitBits;
            *it = static_cast<digit_t>(res);
            if (carry == 0) {
                return;
            }
            ++it;
        } while (it != end_iter);

        if (carry != 0) {
            const size_type usize_value = usize();
            assert(usize_value <= capacity_);
            if (unlikely(usize_value >= capacity_)) {
                grow_capacity();
                assert(usize_value < capacity_);
            }

            nums_[usize_value] = static_cast<digit_t>(carry);
            size_ += sign();
        }
    }

    constexpr void nonZeroSizeSubUInt(const uint32_t n) noexcept {
        LONGINT_ASSERT_ASSUME(size() != 0);

        static_assert(sizeof(digit_t) == sizeof(uint32_t));
        const size_type usize_value = usize();
        digit_t* nums_iter = nums_;
        const digit_t low_num = nums_iter[0];
        if (usize_value != 1) {
            digit_t res = low_num - n;
            const bool carry = res > low_num;
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
                assign_zero();
            }
        } else {
            nums_iter[0] = n - low_num;
            flip_sign();
        }
    }

    static digit_t* allocate_uninitialized(const size_type nums) {
#if defined(HAS_CUSTOM_LONGINT_ALLOCATOR)
        if constexpr (kUseCustomLongIntAllocator) {
            return static_cast<digit_t*>(
                ::longint_allocator::Allocate(std::size_t{nums} * sizeof(digit_t)));
        } else
#endif
        {
            return std::allocator<digit_t>{}.allocate(nums);
        }
    }

    static void deallocate(digit_t* const nums,
                           [[maybe_unused]] const size_type nums_capacity) noexcept {
#if defined(HAS_CUSTOM_LONGINT_ALLOCATOR)
        if constexpr (kUseCustomLongIntAllocator) {
            ::longint_allocator::Deallocate(static_cast<void*>(nums));
        } else
#endif
        {
            std::allocator<digit_t>{}.deallocate(nums, nums_capacity);
        }
    }

    struct ComplexDeleter {
        void operator()(fft::complex* const memory) noexcept {
            ::operator delete(static_cast<void*>(memory));
        }
    };

    ATTRIBUTE_ALWAYS_INLINE static fft::complex* allocate_complex_array_for_unique_ptr(
        std::size_t size) {
        const auto size_bytes = size * sizeof(fft::complex);
        return static_cast<fft::complex*>(::operator new(size_bytes));
    }

    ATTRIBUTE_ALWAYS_INLINE static size_type check_size(const std::size_t value) {
        if (unlikely(value > max_size())) {
            throw_size_error(LONGINT_FILE_LOCATION(), value, max_size());
        }
        const size_type checked_value = static_cast<size_type>(value);
        CONFIG_ASSUME_STATEMENT(checked_value <= max_size());
        LONGINT_ASSERT_ASSUME(checked_value == value);
        return checked_value;
    }

    void adopt_digits_sequence_without_changing_size(digit_t* const new_nums,
                                                     const size_type new_nums_capacity) noexcept {
        deallocate(nums_, capacity_);
        nums_ = new_nums;
        capacity_ = new_nums_capacity;
    }

    ATTRIBUTE_COLD
    [[noreturn]] static void throw_size_error(const char* const file_location,
                                              const char* const function_name,
                                              const std::size_t new_size_value,
                                              const std::size_t max_size_value) {
        static constexpr std::size_t kMessageSize = 1024;
        std::array<char, kMessageSize> message{};
        const int bytes_written =
            std::snprintf(message.data(), message.size(),
                          "%s: size error at %s: new size (which is %zu) > max size (which is %zu)",
                          file_location, function_name, new_size_value, max_size_value);

        if (unlikely(bytes_written < 0)) {
            constexpr std::string_view kFallbackMessage = "size error at ";
            std::char_traits<char>::copy(message.data(), kFallbackMessage.data(),
                                         kFallbackMessage.size());
            const auto fn_name_size = std::char_traits<char>::length(function_name);
            static_assert(kFallbackMessage.size() < kMessageSize);
            const auto fn_name_copy_size =
                std::min(fn_name_size, message.size() - 1 - kFallbackMessage.size());
            std::char_traits<char>::copy(std::addressof(message[kFallbackMessage.size()]),
                                         function_name, fn_name_copy_size);
            message[kFallbackMessage.size() + fn_name_copy_size] = '\0';
        }

        throw std::length_error{message.data()};
    }

    ATTRIBUTE_COLD
    [[noreturn]] static void throw_on_failed_uint_checked_cast(const char* const file_location,
                                                               const char* const function_name) {
        static constexpr std::size_t kMessageSize = 1024;
        std::array<char, kMessageSize> message{};
        const int bytes_written = std::snprintf(
            message.data(), message.size(),
            "%s: checked cast longint to integral type failed at %s", file_location, function_name);
        if (unlikely(bytes_written < 0)) {
            constexpr std::string_view kFallbackMessage =
                "checked cast longint to integral type failed";
            static_assert(kFallbackMessage.size() <= kMessageSize);
            std::char_traits<char>::copy(message.data(), kFallbackMessage.data(),
                                         kFallbackMessage.size());
            message[kFallbackMessage.size()] = '\0';
        }

        throw std::runtime_error{message.data()};
    }

    digit_t* nums_ = nullptr;
    /**
     * size_ < 0 <=> sign = -1; size_ == 0 <=> sign = 0; size > 0 <=> sign = 1
     */
    ssize_type size_ = 0;
    size_type capacity_ = 0;
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
            conv_dec_base_pows[i - 1].square_this_to(conv_dec_base_pows.back());
        } while (++i != pows_size);
    }
};

}  // namespace longint_detail

inline void longint::set_dec_str_impl(const unsigned char* const str, const std::size_t str_size) {
    const unsigned char* str_iter = str;
    const unsigned char* const str_end = str + str_size;

    std::int32_t sgn = 1;
    if (str_iter != str_end && *str_iter == '-') {
        sgn = -1;
        ++str_iter;
    }

    while (str_iter != str_end && *str_iter == '0') {
        ++str_iter;
    }

    const auto digits_count = static_cast<std::size_t>(str_end - str_iter);
    if (digits_count <= 19) {
        static_assert(sizeof(double_digit_t) == sizeof(uint64_t));
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
    const size_type aligned_str_conv_digits_size = check_size(
        math_functions::nearest_greater_equal_power_of_two(uint64_t{str_conv_digits_size}));
    LONGINT_ASSERT_ASSUME(str_conv_digits_size <= aligned_str_conv_digits_size);
    reserveUninitializedWithoutCopy(aligned_str_conv_digits_size);
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
            static_assert(kStrConvBaseDigits == 9);
            uint32_t current = uint32_t{*str_iter} - '0';
            str_iter++;
            current = current * 10 + uint32_t{*str_iter} - '0';
            str_iter++;
            current = current * 10 + uint32_t{*str_iter} - '0';
            str_iter++;
            current = current * 10 + uint32_t{*str_iter} - '0';
            str_iter++;
            current = current * 10 + uint32_t{*str_iter} - '0';
            str_iter++;
            current = current * 10 + uint32_t{*str_iter} - '0';
            str_iter++;
            current = current * 10 + uint32_t{*str_iter} - '0';
            str_iter++;
            current = current * 10 + uint32_t{*str_iter} - '0';
            str_iter++;
            current = current * 10 + uint32_t{*str_iter} - '0';
            str_iter++;
            *--str_conv_digits_iter = current;
        } while (str_iter != str_end);
    }

    static_assert(max_size() * 4 > max_size());
    std::size_t m = aligned_str_conv_digits_size * 2;
    if (m > kFFTPrecisionBorder) {
        m *= 2;
    }
    longint_detail::longint_static_storage::ensureDecBasePowsCapacity(
        math_functions::log2_floor(aligned_str_conv_digits_size));

    // Allocate m complex numbers for p1 and m complex numbers for p2
    const std::size_t max_fft_poly_length = 2 * m;
    static_assert(sizeof(fft::complex) % sizeof(digit_t) == 0);
    const std::size_t allocated_nums_and_poly_size =
        std::size_t{aligned_str_conv_digits_size} +
        max_fft_poly_length * (sizeof(fft::complex) / sizeof(digit_t));
    digit_t* const mult_add_buffer =
        std::allocator<digit_t>{}.allocate(allocated_nums_and_poly_size);

#if CONFIG_COMPILER_ID == CONFIG_GCC_COMPILER_ID
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#endif

    assert((aligned_str_conv_digits_size * sizeof(digit_t)) % alignof(fft::complex) == 0);
#ifdef __STDCPP_DEFAULT_NEW_ALIGNMENT__
    static_assert(__STDCPP_DEFAULT_NEW_ALIGNMENT__ >= alignof(fft::complex), "");
#endif
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    assert(reinterpret_cast<std::uintptr_t>(mult_add_buffer) % alignof(fft::complex) == 0);
    auto* const fft_poly_buffer =
        reinterpret_cast<fft::complex*>(mult_add_buffer + aligned_str_conv_digits_size);
    // NOLINTPREVLINE(cppcoreguidelines-pro-type-reinterpret-cast)

#if CONFIG_COMPILER_ID == CONFIG_GCC_COMPILER_ID
#pragma GCC diagnostic pop
#endif

    const longint* conv_dec_base_pows_iter =
        longint_detail::longint_static_storage::conv_dec_base_pows.data();
    static_assert(max_size() * 2 > max_size());
    for (size_type conv_len = 2; conv_len <= aligned_str_conv_digits_size;
         conv_len *= 2, ++conv_dec_base_pows_iter) {
        LONGINT_ASSERT_ASSUME(math_functions::is_power_of_two(conv_len));
        for (size_type pos = 0; pos < aligned_str_conv_digits_size; pos += conv_len) {
            convert_dec_base_mult_add(str_conv_digits + pos, conv_len, *conv_dec_base_pows_iter,
                                      mult_add_buffer, fft_poly_buffer);
        }
    }
    std::allocator<digit_t>{}.deallocate(mult_add_buffer, allocated_nums_and_poly_size);

    size_type usize_value = aligned_str_conv_digits_size;
    while (usize_value > 0 && nums_[usize_value - 1] == 0) {
        usize_value--;
    }
    set_ssize_from_size_and_sign(usize_value, sgn);
}

// clang-format off
// NOLINTEND(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays,
// cppcoreguidelines-avoid-magic-numbers)
// clang-format on

#if defined(HAS_CUSTOM_LONGINT_ALLOCATOR)
#undef HAS_CUSTOM_LONGINT_ALLOCATOR
#endif
#undef CONSTEXPR_VECTOR
#undef LONGINT_FILE_LOCATION
#undef CONCAT_STR_STR_INT2
#undef CONCAT_STR_STR_INT1
#undef LONGINT_ASSERT_ASSUME
#undef LONGINT_DEBUG_ASSERT
