#ifndef MATH_FUNCTIONS_HPP
#define MATH_FUNCTIONS_HPP

// clang-format off
/**
 * Define (uncomment line with #define) if you want to compile some
 *  functions with a bit faster instruction (see description below)
 */
// #define MATH_FUNCTIONS_HPP_ENABLE_TARGET_OPTIONS
/**
 * From lzcnt: bsr -> lzcnt (leading zeros count)
 * Used in: log2_floor, log2_ceil, log10_floor, base_10_len
 *
 * From bmi: bsf -> tzcnt (trailing zeros count)
 * Used in: extract_pow2, next_n_bits_permutation, gcd
 */
// clang-format on

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <map>
#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

#include "../misc/assert.hpp"
#include "../misc/config_macros.hpp"

#if CONFIG_HAS_AT_LEAST_CXX_20 && CONFIG_HAS_INCLUDE(<bit>)
#include <bit>
#define MATH_FUNCTIONS_HAS_BIT
#endif
#if CONFIG_HAS_AT_LEAST_CXX_20 && CONFIG_HAS_INCLUDE(<ranges>)
#include <ranges>
#define MATH_FUNCTIONS_HAS_RANGES
#endif
#if CONFIG_HAS_AT_LEAST_CXX_20 && CONFIG_HAS_INCLUDE(<numbers>)
#include <numbers>
#define MATH_FUNCTIONS_HAS_NUMBERS
#endif

#if CONFIG_HAS_CONCEPTS
#include <concepts>
#endif

#if CONFIG_HAS_INCLUDE("integers_128_bit.hpp")
#include "integers_128_bit.hpp"
#endif

// Visual C++ thinks that unary minus on unsigned is an error
#if CONFIG_COMPILER_IS_MSVC
#pragma warning(push)
#pragma warning(disable : 4146)
#endif  // _MSC_VER

#if defined(MATH_FUNCTIONS_HPP_ENABLE_TARGET_OPTIONS)
#if defined(__GNUG__)
#if !defined(__clang__)
#pragma GCC push_options
#pragma GCC target("lzcnt,bmi")
#else
#pragma clang attribute push(__attribute__((target("lzcnt,bmi"))), apply_to = function)
#endif
#endif
#endif

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

namespace math_functions {

using std::int32_t;
using std::int64_t;
using std::ptrdiff_t;
using std::size_t;
using std::uint32_t;
using std::uint64_t;

namespace detail {

#if defined(HAS_INT128_TYPEDEF)
namespace helper_ns = int128_traits;
#else
namespace helper_ns = std;
#endif

#if CONFIG_COMPILER_SUPPORTS_CONCEPTS
template <class T>
concept InplaceMultipliable = requires(T a, const T b) {
    { a *= b };
};
#endif

}  // namespace detail

template <class T>
inline constexpr bool is_integral_v = math_functions::detail::helper_ns::is_integral_v<T>;

template <class T>
inline constexpr bool is_unsigned_v = math_functions::detail::helper_ns::is_unsigned_v<T>;

template <class T>
inline constexpr bool is_signed_v = math_functions::detail::helper_ns::is_signed_v<T>;

template <class T>
inline constexpr bool is_arithmetic_v = math_functions::detail::helper_ns::is_arithmetic_v<T>;

template <class T>
using make_unsigned_t = typename math_functions::detail::helper_ns::make_unsigned_t<T>;

template <class T>
using make_signed_t = typename math_functions::detail::helper_ns::make_signed_t<T>;

template <class T>
inline constexpr bool is_math_integral_type_v = math_functions::is_integral_v<T> && !std::is_same_v<T, bool> &&
                                                !std::is_same_v<T, char> && !std::is_same_v<T, wchar_t>
#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
                                                && !std::is_same_v<T, char8_t>
#endif
                                                && !std::is_same_v<T, char16_t> && !std::is_same_v<T, char32_t>;

#if CONFIG_HAS_CONCEPTS

template <class T>
concept integral = math_functions::detail::helper_ns::integral<T>;

template <class T>
concept signed_integral = math_functions::detail::helper_ns::signed_integral<T>;

template <class T>
concept unsigned_integral = math_functions::detail::helper_ns::unsigned_integral<T>;

template <class T>
concept math_integral_type = math_functions::is_math_integral_type_v<T>;

#endif

namespace detail {

template <class T>
struct double_bits {
    using type = void;
};

template <>
struct double_bits<int8_t> {
    using type = int16_t;
};

template <>
struct double_bits<uint8_t> {
    using type = uint16_t;
};

template <>
struct double_bits<int16_t> {
    using type = int32_t;
};

template <>
struct double_bits<uint16_t> {
    using type = uint32_t;
};

template <>
struct double_bits<int32_t> {
    using type = int64_t;
};

template <>
struct double_bits<uint32_t> {
    using type = uint64_t;
};

#ifdef HAS_INT128_TYPEDEF

template <>
struct double_bits<int64_t> {
    using type = int128_t;
};

template <>
struct double_bits<uint64_t> {
    using type = uint128_t;
};

#endif

template <class T>
using double_bits_t = typename double_bits<T>::type;

template <class T>
using try_double_bits_t = std::conditional_t<std::is_void_v<math_functions::detail::double_bits_t<T>>,
                                             T,
                                             math_functions::detail::double_bits_t<T>>;

template <typename T>
inline constexpr bool is_trivial_arithmetic_v = std::is_arithmetic_v<T>
#if defined(HAS_INT128_TYPEDEF) && INT128_IS_BUILTIN_TYPE
                                                || std::is_same_v<T, int128_t> || std::is_same_v<T, uint128_t>
#endif
    ;

template <class T>
ATTRIBUTE_ALWAYS_INLINE constexpr void check_math_int_type() noexcept {
    static_assert(math_functions::is_math_integral_type_v<T>,
                  "integral type that is not a bool nor a char is expected");
}

template <class T>
ATTRIBUTE_ALWAYS_INLINE constexpr void check_math_unsigned_int_type() noexcept {
    math_functions::detail::check_math_int_type<T>();
    static_assert(math_functions::is_unsigned_v<T>, "unsigned integral type is expected");
}

template <class IntType>
ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_CONST [[nodiscard]]
constexpr auto to_uint_at_least_32(const IntType n) noexcept {
    using UIntType = math_functions::make_unsigned_t<IntType>;
    using UIntTypeAtLeastUInt32 = std::common_type_t<UIntType, uint32_t>;
    return UIntTypeAtLeastUInt32{static_cast<UIntType>(n)};
}

ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t isqrt_u32(uint32_t n) noexcept {
    /**
     * In the runtime `sqrt` is used (but not for the msvc prior to the c++20).
     *
     * Quick Bench benchmark source code on the godbolt:
     *  https://godbolt.org/z/7jK8xcjjf
     */

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG || CONFIG_HAS_AT_LEAST_CXX_20
    if (config::is_constant_evaluated()) {
#endif
        uint32_t y = 0;

        /**
         * See Hackers Delight Chapter 11.
         */
        for (uint32_t m = 0x40000000; m != 0; m >>= 2U) {
            const uint32_t b = y | m;
            y >>= 1U;
            if (n >= b) {
                n -= b;
                y |= m;
            }
        }

        return y;
#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG || CONFIG_HAS_AT_LEAST_CXX_20
    } else {
        return static_cast<uint32_t>(std::sqrt(static_cast<double>(n)));
    }
#endif
}

ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t isqrt_u64(const uint64_t n) noexcept {
    /**
     * In the runtime `sqrtl` is used (but not for the msvc prior to the c++20).
     */
#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG || CONFIG_HAS_AT_LEAST_CXX_20
    if (config::is_constant_evaluated() || sizeof(long double) < 16) {
#endif
        /**
         * See Hackers Delight Chapter 11.
         */
        constexpr uint32_t kMaxUInt32 = std::numeric_limits<uint32_t>::max();
        uint64_t l = n > kMaxUInt32 ? std::numeric_limits<uint16_t>::max() : 0U;
        const uint64_t r_approx = (n >> 5U) + 8U;
        uint64_t r = std::min(r_approx, uint64_t{kMaxUInt32});
        CONFIG_ASSUME_STATEMENT(l < r);
        do {
            CONFIG_ASSUME_STATEMENT(r <= kMaxUInt32);
            const uint64_t m = (l + r + 1) / 2;
            CONFIG_ASSUME_STATEMENT(m <= kMaxUInt32);
            if (n >= m * m) {
                l = m;
            } else {
                r = m - 1;
            }
        } while (r > l);
        CONFIG_ASSUME_STATEMENT(l <= kMaxUInt32);
        return static_cast<uint32_t>(l);
#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG || CONFIG_HAS_AT_LEAST_CXX_20
    }

    return static_cast<uint32_t>(std::sqrt(static_cast<long double>(n)));
#endif
}

#if defined(HAS_INT128_TYPEDEF)

/// @note  See Hackers Delight Chapter 11.
ATTRIBUTE_CONST
[[nodiscard]]
I128_CONSTEXPR uint64_t isqrt_u128(const uint128_t n) noexcept(detail::is_trivial_arithmetic_v<uint128_t>) {
    uint64_t l = n > std::numeric_limits<uint64_t>::max() ? std::numeric_limits<uint32_t>::max() : 0U;
    const uint128_t r_approx = (n >> 6U) + 16U;
    uint64_t r = r_approx > std::numeric_limits<uint64_t>::max() ? std::numeric_limits<uint64_t>::max()
                                                                 : static_cast<uint64_t>(r_approx);
    CONFIG_ASSUME_STATEMENT(l < r);
    do {
        // m = (l + r + 1) / 2
        const uint64_t m = (l / 2) + (r / 2) + ((r % 2) | (l % 2));
        if (n >= uint128_t{m} * m) {
            l = m;
        } else {
            r = m - 1;
        }
    } while (r > l);
    return l;
}

#endif

/// @note  See Hackers Delight Chapter 11, section 11-2.
ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t icbrt_u32(uint32_t n) noexcept {
    /**
     * cbrt and cbrtl are not used here because according
     * to the Quick Bench results they are:
     *  1.9 and 6.5 times slower accordingly on the GCC 13.2 + libstdc++
     *  1.7 and 5.8 times slower accordingly on the Clang 17.0 + libstdc++ / libc++
     *
     * Benchmark source code on the godbolt:
     *  https://godbolt.org/z/j3cnKT3vr
     * Quick bench (not sure it will be kept for too long,
     *  probably it's safer to use godbolt and open Quick Bench through it):
     *  https://quick-bench.com/q/MZlkkxIvHTOWQV5__RPJnJS-WkM
     *
     * If in the future the results are changed (overloads of sqrt and sqrtl are
     *  much faster then the isqrt implementation above used in the constexpr),
     *  this should be taken into account: when using the libstdc++,
     *  `uint32_t(std::cbrt(3375.0))` may be equal to 14
     */

    uint32_t y = 0;
    for (int32_t s = 30; s >= 0; s -= 3) {
        y *= 2;
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        const uint32_t b = (3U * y * (y + 1) | 1U) << s;
        if (n >= b) {
            n -= b;
            y++;
        }
    }

    return y;
}

/// @note  See Hackers Delight Chapter Chapter 11, ex. 2.
ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t icbrt_u64(uint64_t n) noexcept {
    uint64_t y = 0;
    if (n >= 0x1000000000000000ULL) {
        if (n >= 0x8000000000000000ULL) {
            n -= 0x8000000000000000ULL;
            y = 2U;
        } else {
            n -= 0x1000000000000000ULL;
            y = 1U;
        }
    }
    for (int32_t s = 57; s >= 0; s -= 3) {
        CONFIG_ASSUME_STATEMENT(y <= 1321122U);
        y *= 2U;
        CONFIG_ASSUME_STATEMENT(y <= 2642244U);
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        const uint64_t bs = (3U * y * (y + 1) | 1U) << s;
        if (n >= bs) {
            n -= bs;
            y++;
        }
    }
    CONFIG_ASSUME_STATEMENT(y <= 2642245U);
    return static_cast<uint32_t>(y);
}

template <class T>
ATTRIBUTE_CONST [[nodiscard]]
constexpr uint32_t max_ifrrt() noexcept {
    if constexpr (detail::is_trivial_arithmetic_v<T>) {
        constexpr uint32_t ifrrt_of_one_plus_max_T = []() constexpr noexcept {
            constexpr auto kNumBits = CHAR_BIT * sizeof(T);
            if constexpr (kNumBits == 128) {
                return 0;
            } else {
                return 1u << (kNumBits / 4);
            }
        }();
        return ifrrt_of_one_plus_max_T - 1u;
    } else {
        return std::numeric_limits<uint32_t>::max();
    }
}

template <class T, class P>
ATTRIBUTE_CONST [[nodiscard]]
constexpr T bin_pow_impl(T n, P p) noexcept(detail::is_trivial_arithmetic_v<T>) {
    math_functions::detail::check_math_int_type<P>();

    T res(1);
    while (true) {
        if (p % 2 != 0) {
            res *= n;
        }
        p /= 2;
        if (p == 0) {
            return res;
        }
        n *= n;
    }
}

}  // namespace detail

template <class IntType>
ATTRIBUTE_CONST [[nodiscard]]
constexpr int32_t sign(const IntType n) noexcept {
    math_functions::detail::check_math_int_type<IntType>();

    if constexpr (math_functions::is_signed_v<IntType>) {
#if defined(HAS_INT128_TYPEDEF)
        if constexpr (std::is_same_v<IntType, int128_t>) {
            const uint32_t sign_bit = static_cast<uint32_t>(static_cast<uint128_t>(n) >> 127U);
            return static_cast<int32_t>(n != 0) - static_cast<int32_t>(2 * sign_bit);
        }
#endif

        return static_cast<int32_t>(n > 0) - static_cast<int32_t>(n < 0);
    } else {
        return n > 0 ? 1 : 0;
    }
}

template <class IntType>
ATTRIBUTE_CONST [[nodiscard]]
constexpr auto uabs(const IntType n) noexcept {
    math_functions::detail::check_math_int_type<IntType>();

    if constexpr (math_functions::is_signed_v<IntType>) {
        using UIntType = math_functions::make_unsigned_t<IntType>;

#if defined(HAS_INT128_TYPEDEF)
        if constexpr (std::is_same_v<IntType, int128_t>) {
            const uint128_t t = static_cast<uint128_t>(n >> 127U);
            return (static_cast<uint128_t>(n) ^ t) - t;
        }
#endif
        return n >= 0 ? static_cast<UIntType>(n) : static_cast<UIntType>(-static_cast<UIntType>(n));
    } else {
        return n;
    }
}

/// @brief a > 0 and b > 0 => true
///        a > 0 and b = 0 => false
///        a > 0 and b < 0 => false
///        a = 0 and b > 0 => false
///        a = 0 and b = 0 => true
///        a = 0 and b < 0 => false
///        a < 0 and b > 0 => false
///        a < 0 and b = 0 => false
///        a < 0 and b < 0 => true
/// @param[in] a
/// @param[in] b
/// @return
template <class IntType>
ATTRIBUTE_CONST [[nodiscard]]
constexpr bool same_sign(const IntType a, const IntType b) noexcept {
    math_functions::detail::check_math_int_type<IntType>();

    return math_functions::sign(a) == math_functions::sign(b);
}

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG
#if CONFIG_COMPILER_ID == CONFIG_GCC_COMPILER_ID
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
#endif
#endif

/// @brief Calculate n ^ p
/// @tparam T
/// @tparam P integral type
/// @param[in] n
/// @param[in] p
/// @return n ^ p
template <class T, class P>
#if CONFIG_COMPILER_SUPPORTS_CONCEPTS
    requires math_functions::detail::InplaceMultipliable<T>
#endif
ATTRIBUTE_ALWAYS_INLINE [[nodiscard]]
constexpr T bin_pow(T n, const P p) noexcept(detail::is_trivial_arithmetic_v<T>) {
    math_functions::detail::check_math_int_type<P>();

    if constexpr (math_functions::is_integral_v<T>) {
        math_functions::detail::check_math_int_type<T>();
    }

    if constexpr (math_functions::is_arithmetic_v<T>) {
        if (config::is_constant_evaluated() || config::is_gcc_constant_p(n)) {
            if (n == 0) {
                assert(p >= 0);
                return p == 0 ? T{1} : T{0};
            } else if (n == 1) {
                return T{1};
            } else if constexpr (!math_functions::is_unsigned_v<T>) {
                if (n == -1) {
                    return p % 2 == 0 ? T{1} : T{-1};
                }
            }
        }
    }

    T ret = math_functions::detail::bin_pow_impl(std::move(n), math_functions::uabs(p));
    if constexpr (math_functions::is_unsigned_v<P>) {
        return ret;
    } else {
        static_assert(!math_functions::is_integral_v<T>,
                      "use floating point types in bin_pow because type of power is signed and "
                      "power may be < 0");
        if (p >= 0) {
            return ret;
        }

        assert(ret != 0);
        return T{1} / std::move(ret);
    }
}

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG
#if CONFIG_COMPILER_ID == CONFIG_GCC_COMPILER_ID
#pragma GCC diagnostic pop
#else
#pragma clang diagnostic pop
#endif
#endif

/// @brief Calculate (n ^ p) % mod
/// @tparam T unsigned integral type
/// @param[in] n
/// @param[in] p
/// @param[in] mod
/// @return (n ^ p) % mod
template <class T>
ATTRIBUTE_CONST [[nodiscard]]
constexpr T bin_pow_mod(T n, uint64_t p, const T mod) noexcept {
    math_functions::detail::check_math_unsigned_int_type<T>();

    using Q = math_functions::detail::double_bits_t<T>;
    [[maybe_unused]] constexpr T kMaxT = std::numeric_limits<T>::max();

    assert(mod != 0);

    Q res = Q{mod != 1};
    Q widen_n = n;
    while (true) {
        if (p % 2 != 0) {
            CONFIG_ASSUME_STATEMENT(res <= kMaxT);
            CONFIG_ASSUME_STATEMENT(res < mod);
            CONFIG_ASSUME_STATEMENT(widen_n <= kMaxT);
            res = (res * widen_n) % mod;
            CONFIG_ASSUME_STATEMENT(res <= kMaxT);
            CONFIG_ASSUME_STATEMENT(res < mod);
        }
        p /= 2;
        if (p == 0) {
            CONFIG_ASSUME_STATEMENT(res <= kMaxT);
            CONFIG_ASSUME_STATEMENT(res < mod);
            return static_cast<T>(res);
        }
        CONFIG_ASSUME_STATEMENT(widen_n <= kMaxT);
        widen_n = (widen_n * widen_n) % mod;
        CONFIG_ASSUME_STATEMENT(widen_n <= kMaxT);
        CONFIG_ASSUME_STATEMENT(widen_n < mod);
    }
}

ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint8_t isqrt(const uint8_t n) noexcept {
    const uint32_t ret = math_functions::detail::isqrt_u32(n);
    CONFIG_ASSUME_STATEMENT(ret < (1U << 4U));
    CONFIG_ASSUME_STATEMENT(ret * ret <= n);
    return static_cast<uint8_t>(ret);
}

ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint8_t isqrt(const uint16_t n) noexcept {
    const uint32_t ret = math_functions::detail::isqrt_u32(n);
    CONFIG_ASSUME_STATEMENT(ret < (1U << 8U));
    CONFIG_ASSUME_STATEMENT(ret * ret <= n);
    return static_cast<uint8_t>(ret);
}

ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint16_t isqrt(const uint32_t n) noexcept {
    const uint32_t ret = math_functions::detail::isqrt_u32(n);
    CONFIG_ASSUME_STATEMENT(ret < (1U << 16U));
    CONFIG_ASSUME_STATEMENT(ret * ret <= n);
    return static_cast<uint16_t>(ret);
}

ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t isqrt(const uint64_t n) noexcept {
    const uint32_t ret = math_functions::detail::isqrt_u64(n);
    CONFIG_ASSUME_STATEMENT(uint64_t{ret} * uint64_t{ret} <= n);
    return ret;
}

#if defined(HAS_INT128_TYPEDEF)

ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_CONST
[[nodiscard]]
I128_CONSTEXPR uint64_t isqrt(const uint128_t n) noexcept {
    const uint64_t ret = math_functions::detail::isqrt_u128(n);
#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG
    CONFIG_ASSUME_STATEMENT(uint128_t{ret} * uint128_t{ret} <= n);
#endif
    return ret;
}

#endif

/// @brief Return integer part of the cube root of n, i.e. ⌊n^(1/3)⌋
/// @param[in] n
/// @return ⌊n^(1/3)⌋
ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t icbrt(const uint32_t n) noexcept {
    const uint32_t ret = math_functions::detail::icbrt_u32(n);

    // 1625^3 = 4291015625 < 2^32 - 1 = 4294967295 < 4298942376 = 1626^3
    CONFIG_ASSUME_STATEMENT(ret <= 1625U);
    CONFIG_ASSUME_STATEMENT(ret * ret * ret <= n);

#if CONFIG_COMPILER_IS_GCC && CONFIG_HAS_AT_LEAST_CXX_17
    // Clang ignores this assumption because it contains potential side effects (fpu register
    // flags), while GCC has made almost all math functions constexpr long before the C++26
    CONFIG_ASSUME_STATEMENT(ret == static_cast<uint32_t>(std::cbrt(static_cast<long double>(n))));
#endif

    return ret;
}

/// @brief Return integer part of the cube root of n, i.e. ⌊n^(1/3)⌋
/// @param[in] n
/// @return ⌊n^(1/3)⌋
ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t icbrt(const uint64_t n) noexcept {
    const uint32_t ret = math_functions::detail::icbrt_u64(n);

    // clang-format off
    // 2642245^3 = 18446724184312856125 < 2^64 - 1 = 18446744073709551615 < 18446745128696702936 = 2642246^3
    // clang-format on
    CONFIG_ASSUME_STATEMENT(ret <= 2642245U);
    CONFIG_ASSUME_STATEMENT(uint64_t{ret} * uint64_t{ret} * uint64_t{ret} <= n);

    return ret;
}

/// @brief Return integer part of the fourth root of n, i.e. ⌊n^0.25⌋
/// @note ⌊n^0.25⌋ = ⌊⌊n^0.5⌋^0.5⌋ (see Hackers Delight Chapter 11, ex.1)
/// @param[in] n
/// @return ⌊n^0.25⌋
template <class T>
ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_CONST [[nodiscard]]
constexpr auto ifrrt(const T n) noexcept(detail::is_trivial_arithmetic_v<T>)
    -> decltype(math_functions::isqrt(math_functions::isqrt(n))) {
    math_functions::detail::check_math_unsigned_int_type<T>();

    const auto ret = math_functions::isqrt(math_functions::isqrt(n));

    CONFIG_ASSUME_STATEMENT(ret <= math_functions::detail::max_ifrrt<T>());
    CONFIG_ASSUME_STATEMENT(T{ret} * T{ret} * T{ret} * T{ret} <= n);

    return ret;
}

template <class T>
struct IsPerfectSquareResult {
    T root{};
    bool is_perfect_square{};

    [[nodiscard]] ATTRIBUTE_PURE constexpr explicit operator bool() const noexcept {
        return is_perfect_square;
    }
};

template <class T>
using IsPerfectSquareResultRetType = IsPerfectSquareResult<decltype(math_functions::isqrt(T{}))>;

/// @brief Checks whether @a `n` is perfect square or not.
/// @param[in] n
/// @return {sqrt(n), true} if @a `n` is perfect square and {0, false} otherwise.
template <class T>
ATTRIBUTE_CONST [[nodiscard]]
constexpr IsPerfectSquareResultRetType<T> is_perfect_square(const T n) noexcept(detail::is_trivial_arithmetic_v<T>) {
    math_functions::detail::check_math_unsigned_int_type<T>();

    // clang-format off
    /**
     * +------------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     * |   n mod 16 |  0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 | 11 | 12 | 13 | 14 | 15 |
     * +------------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     * | n*n mod 16 |  0 |  1 |  4 |  9 |  0 |  9 |  4 |  1 |  0 |  1 |  4 |  9 |  0 |  9 |  4 |  1 |
     * +------------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     *
     * If we peek mod 32, then we should check only for n & 31 in { 0, 1, 4, 9, 16, 17, 25 },
     * but switch statement could be less efficient in this case
     */
    // clang-format on
    switch (static_cast<uint32_t>(n) % 16U) {
        case 0:
        case 1:
        case 4:
        case 9: {
            const auto root = math_functions::isqrt(n);
            const bool is_perf_square = T{root} * T{root} == n;
            return {
#if CONFIG_HAS_AT_LEAST_CXX_20
                .root =
#endif
                    is_perf_square ? root : 0,
#if CONFIG_HAS_AT_LEAST_CXX_20
                .is_perfect_square =
#endif
                    is_perf_square,
            };
        }
        default: {
            return {
#if CONFIG_HAS_AT_LEAST_CXX_20
                .root =
#endif
                    0,
#if CONFIG_HAS_AT_LEAST_CXX_20
                .is_perfect_square =
#endif
                    false,
            };
        }
    }
}

/// @brief This function reverses bits of the @a `b`
/// @param[in] b
/// @return 8-bit number whose bits are reversed bits of the @a `b`.
ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint8_t bit_reverse(const uint8_t b) noexcept {
    // See https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    return static_cast<uint8_t>(((b * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32U);
}

/// @brief This function reverses bits of the @a `n`
/// @param[in] b
/// @return 32-bit number whose bits are reversed bits of the @a `n`.
ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t bit_reverse(uint32_t n) noexcept {
    // clang-format off
    /**
     * See Hackers Delight 7.1
     */
    n = ((n & 0x55555555U) << 1U) | ((n >> 1U) & 0x55555555U);
    n = ((n & 0x33333333U) << 2U) | ((n >> 2U) & 0x33333333U);
    n = ((n & 0x0F0F0F0FU) << 4U) | ((n >> 4U) & 0x0F0F0F0FU);
    // n = ((n & 0x00FF00FFU) << 8U) | ((n >> 8U) & 0x00FF00FFU);
    // n = ((n & 0x0000FFFFU) << 16U) | ((n & 0xFFFF0000u) >> 16U);
    n = (n << 24U) | ((n & 0xFF00U) << 8U) | ((n >> 8U) & 0xFF00U) | (n >> 24U);
    // clang-format on
    return n;
}

/// @brief This function reverses bits of the @a `n`
/// @param[in] b
/// @return 64-bit number whose bits are reversed bits of the @a `n`.
ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint64_t bit_reverse(uint64_t n) noexcept {
    // clang-format off
    /**
     * See Knuth's algorithm in Hackers Delight 7.4
     */
    uint64_t t = 0;
    n = (n << 31U) | (n >> 33U);  // I.e., shlr(x, 31).
    t = (n ^ (n >> 20U)) & 0x00000FFF800007FFULL;
    n = (t | (t << 20U)) ^ n;
    t = (n ^ (n >> 8U)) & 0x00F8000F80700807ULL;
    n = (t | (t << 8U)) ^ n;
    t = (n ^ (n >> 4U)) & 0x0808708080807008ULL;
    n = (t | (t << 4U)) ^ n;
    t = (n ^ (n >> 2U)) & 0x1111111111111111ULL;
    n = (t | (t << 2U)) ^ n;
    // clang-format on
    return n;
}

#if defined(HAS_INT128_TYPEDEF)

/// @brief This function reverses bits of the @a `n`
/// @param[in] b
/// @return 128-bit number whose bits are reversed bits of the @a `n`.
ATTRIBUTE_CONST
[[nodiscard]]
I128_CONSTEXPR uint128_t bit_reverse(uint128_t n) noexcept {
    uint128_t m = ~uint128_t{0};
    for (uint32_t s = sizeof(uint128_t) * CHAR_BIT; s >>= 1U;) {
        m ^= m << s;
        n = ((n >> s) & m) | ((n << s) & ~m);
    }
    return n;
}

#endif

template <class F>
#if CONFIG_COMPILER_SUPPORTS_CONCEPTS
    requires requires(F f, const uint64_t mask) { f(uint64_t{mask}); }
#endif
ATTRIBUTE_ALWAYS_INLINE constexpr void visit_all_submasks(const uint64_t mask, F visiter) noexcept(
    std::is_nothrow_invocable_v<F, uint64_t>) {
    static_assert(std::is_invocable_v<F, uint64_t>, "callable object that accepts uint64_t is expected");

    uint64_t s = mask;
    do {
        visiter(uint64_t{s});
        s = (s - 1) & mask;
    } while (s != 0);
}

template <class IntType>
ATTRIBUTE_CONST [[nodiscard]]
constexpr bool is_power_of_two(const IntType n) noexcept {
    math_functions::detail::check_math_int_type<IntType>();

    if constexpr (math_functions::is_signed_v<IntType>) {
        // Cast to unsigned type to avoid potential overflow (ub for the signed type)
        const auto m = math_functions::detail::to_uint_at_least_32(n);
        return (m & (m - 1)) == 0 && n > 0;
    } else {
        return (n & (n - 1)) == 0 && n != 0;
    }
}

namespace detail {

ATTRIBUTE_CONST
[[nodiscard]]
inline uint32_t log2_floor_software(uint64_t n) noexcept {
    static const std::array<uint64_t, 6> t = {
        0xFFFFFFFF00000000ULL, 0x00000000FFFF0000ULL, 0x000000000000FF00ULL,
        0x00000000000000F0ULL, 0x000000000000000CULL, 0x0000000000000002ULL,
    };

    uint32_t y = 0;
    uint32_t j = 32;

    for (const uint64_t t_mask : t) {
        const uint32_t k = (((n & t_mask) == 0) ? 0 : j);
        y += k;
        n >>= k;
        j >>= 1U;
    }

    return y;
}

ATTRIBUTE_CONST
[[nodiscard]]
inline uint32_t log2_ceil_software(uint64_t n) noexcept {
    return math_functions::detail::log2_floor_software(n) + ((n & (n - 1)) != 0);
}

/**
 *  Returns the integer (floor) log of the specified value, base 2.
 *  Note that by convention, input value 0 returns 0 since log(0) is
 * undefined.
 */
ATTRIBUTE_CONST
[[nodiscard]]
inline uint32_t de_bruijn_log2(uint32_t value) noexcept {
    // See
    // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2

    static const std::array<unsigned char, 32> MultiplyDeBruijnBitPosition = {
        0, 9,  1,  10, 13, 21, 2,  29, 11, 14, 16, 18, 22, 25, 3, 30,
        8, 12, 20, 28, 15, 17, 24, 7,  19, 27, 23, 6,  26, 5,  4, 31,
    };

    // first round down to one less than a power of 2
    value |= (value >> 1U);
    value |= (value >> 2U);
    value |= (value >> 4U);
    value |= (value >> 8U);
    value |= (value >> 16U);

    // Using de Bruijn sequence, k=2, n=5 (2^5=32) :
    // 0b_0000_0111_1100_0100_1010_1100_1101_1101
    return MultiplyDeBruijnBitPosition[((value * 0x07C4ACDDU) >> 27U)];
}

ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t lz_count_32_software(uint32_t n) noexcept {
    /**
     * See Hackers Delight Chapter 5
     */
    if (unlikely(n == 0)) {
        return 32U;
    }
    uint32_t m = 1U;
    if ((n >> 16U) == 0) {
        m += 16U;
        n <<= 16U;
    }
    if ((n >> 24U) == 0) {
        m += 8U;
        n <<= 8U;
    }
    if ((n >> 28U) == 0) {
        m += 4U;
        n <<= 4U;
    }
    if ((n >> 30U) == 0) {
        m += 2U;
        n <<= 2U;
    }
    m -= n >> 31U;
    CONFIG_ASSUME_STATEMENT(m <= 31);
    return m;
}

ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t lz_count_64_software(uint64_t n) noexcept {
    /**
     * See Hackers Delight Chapter 5
     */
    if (unlikely(n == 0)) {
        return 64U;
    }
    uint32_t m = 1;
    if ((n >> 32U) == 0) {
        m += 32U;
        n <<= 32U;
    }
    if ((n >> 48U) == 0) {
        m += 16U;
        n <<= 16U;
    }
    if ((n >> 56U) == 0) {
        m += 8U;
        n <<= 8U;
    }
    if ((n >> 60U) == 0) {
        m += 4U;
        n <<= 4U;
    }
    if ((n >> 62U) == 0) {
        m += 2U;
        n <<= 2U;
    }
    m -= static_cast<uint32_t>(n >> 63U);
    CONFIG_ASSUME_STATEMENT(m <= 63U);
    return m;
}

ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t tz_count_32_software(uint32_t n) noexcept {
    /**
     * See Hackers Delight Chapter 5
     */
    if (unlikely(n == 0U)) {
        return 32U;
    }
    uint32_t m = 1U;
    if ((n & 0x0000FFFFU) == 0U) {
        m += 16U;
        n >>= 16U;
    }
    if ((n & 0x000000FFU) == 0U) {
        m += 8U;
        n >>= 8U;
    }
    if ((n & 0x0000000FU) == 0U) {
        m += 4U;
        n >>= 4U;
    }
    if ((n & 0x00000003U) == 0U) {
        m += 2U;
        n >>= 2U;
    }
    m -= (n & 1U);
    CONFIG_ASSUME_STATEMENT(m <= 31U);
    return m;
}

ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t tz_count_64_software(uint64_t n) noexcept {
    uint32_t m = 0U;
    for (n = ~n & (n - 1); n != 0U; n >>= 1U) {
        m++;
    }
    CONFIG_ASSUME_STATEMENT(m <= 64U);
    return m;
}

ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t pop_count_32_software(uint32_t n) noexcept {
    /**
     * See Hackers Delight Chapter 5.
     */
    n = (n & 0x55555555U) + ((n >> 1U) & 0x55555555U);
    n = (n & 0x33333333U) + ((n >> 2U) & 0x33333333U);
    n = (n & 0x0F0F0F0FU) + ((n >> 4U) & 0x0F0F0F0FU);
    n = (n & 0x00FF00FFU) + ((n >> 8U) & 0x00FF00FFU);
    n = (n & 0x0000FFFFU) + ((n >> 16U) & 0x0000FFFFU);
    CONFIG_ASSUME_STATEMENT(n <= 32U);
    return n;
}

ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint64_t pop_count_64_software(uint64_t n) noexcept {
    /**
     * See Hackers Delight Chapter 5.
     */
    n = (n & 0x5555555555555555ULL) + ((n >> 1U) & 0x5555555555555555ULL);
    n = (n & 0x3333333333333333ULL) + ((n >> 2U) & 0x3333333333333333ULL);
    n = (n & 0x0F0F0F0F0F0F0F0FULL) + ((n >> 4U) & 0x0F0F0F0F0F0F0F0FULL);
    n = (n & 0x00FF00FF00FF00FFULL) + ((n >> 8U) & 0x00FF00FF00FF00FFULL);
    n = (n & 0x0000FFFF0000FFFFULL) + ((n >> 16U) & 0x0000FFFF0000FFFFULL);
    n = (n & 0x00000000FFFFFFFFULL) + ((n >> 32U) & 0x00000000FFFFFFFFULL);
    CONFIG_ASSUME_STATEMENT(n <= 64U);
    return n;
}

}  // namespace detail

ATTRIBUTE_CONST
[[nodiscard]]
constexpr int32_t diff_popcount(uint32_t x, uint32_t y) noexcept {
    /**
     * See Hackers Delight Chapter 5.
     */
    x = x - ((x >> 1U) & 0x55555555U);
    x = (x & 0x33333333U) + ((x >> 2U) & 0x33333333U);
    y = ~y;
    y = y - ((y >> 1U) & 0x55555555U);
    y = (y & 0x33333333U) + ((y >> 2U) & 0x33333333U);
    x = x + y;
    x = (x & 0x0F0F0F0FU) + ((x >> 4U) & 0x0F0F0F0FU);
    x = x + (x >> 8U);
    x = x + (x >> 16U);
    return static_cast<int32_t>(x & 0x0000007FU) - 32;
}

ATTRIBUTE_CONST
[[nodiscard]]
constexpr int32_t compare_popcount(const uint32_t x, const uint32_t y) noexcept {
    /**
     * See Hackers Delight Chapter 5.
     */
    uint32_t n = x & ~y;  // Clear bits where
    uint32_t m = y & ~x;  // both bits are 1
    while (true) {
        if (n == 0) {
            return static_cast<int32_t>(m | -m);
        }
        if (m == 0) {
            return 1;
        }
        n &= n - 1;  // Clear one bit
        m &= m - 1;  // from each
    }
}

/// @brief Count trailing zeros for n
/// @param[in] n
/// @return trailing zeros count (sizeof(n) * 8 for n = 0)
template <typename T>
#if CONFIG_HAS_CONCEPTS
    requires math_functions::unsigned_integral<T>
#endif
ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_CONST [[nodiscard]]
constexpr int32_t countr_zero(const T n) noexcept {
    static_assert(math_functions::is_unsigned_v<T>, "Unsigned integral type expected");

    if (unlikely(n == 0)) {
        return sizeof(n) * CHAR_BIT;
    }

#ifdef HAS_INT128_TYPEDEF
    if constexpr (std::is_same_v<T, uint128_t>) {
        const uint64_t low = static_cast<uint64_t>(n);
        if (low != 0) {
            return math_functions::countr_zero<uint64_t>(low);
        }

        const uint64_t high = static_cast<uint64_t>(n >> 64U);
        CONFIG_ASSUME_STATEMENT(high != 0);
        return 64 + math_functions::countr_zero<uint64_t>(high);
    } else
#endif

#ifdef MATH_FUNCTIONS_HAS_BIT
    {
        return std::countr_zero(n);
    }
#else
    // NOLINTBEGIN(google-runtime-int)
    if constexpr (std::is_same_v<T, unsigned long long>) {
#if defined(__GNUG__)
        return __builtin_ctzll(n);
#else
        return static_cast<int32_t>(math_functions::detail::tz_count_64_software(n));
#endif
    } else if constexpr (std::is_same_v<T, unsigned long>) {
#if defined(__GNUG__)
        return __builtin_ctzl(n);
#else
        return static_cast<int32_t>(math_functions::detail::tz_count_64_software(static_cast<unsigned long long>(n)));
#endif
    } else {
        static_assert(
            std::is_same_v<T, unsigned int> || std::is_same_v<T, unsigned short> || std::is_same_v<T, unsigned char>,
            "Inappropriate integer type in countr_zero");
#if defined(__GNUG__)
        return __builtin_ctz(n);
#else
        return static_cast<int32_t>(math_functions::detail::tz_count_32_software(n));
#endif
    }
    // NOLINTEND(google-runtime-int)
#endif
}

/// @brief Count leading zeros for n
/// @param[in] n
/// @return leading zeros count (sizeof(n) * 8 for n = 0)
template <typename T>
#if CONFIG_HAS_CONCEPTS
    requires math_functions::unsigned_integral<T>
#endif
ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_CONST [[nodiscard]]
constexpr int32_t countl_zero(const T n) noexcept {
    static_assert(math_functions::is_unsigned_v<T>, "Unsigned integral type expected");

    if (unlikely(n == 0)) {
        return sizeof(n) * CHAR_BIT;
    }

#if defined(HAS_INT128_TYPEDEF)
    if constexpr (std::is_same_v<T, uint128_t>) {
        const uint64_t high = static_cast<uint64_t>(n >> 64U);
        if (high != 0) {
            return math_functions::countl_zero<uint64_t>(high);
        }

        const uint64_t low = static_cast<uint64_t>(n);
        CONFIG_ASSUME_STATEMENT(low != 0);
        return 64U + math_functions::countl_zero<uint64_t>(low);
    } else
#endif

#ifdef MATH_FUNCTIONS_HAS_BIT
    {
        return std::countl_zero(n);
    }
#else
    // NOLINTBEGIN(google-runtime-int)
    if constexpr (std::is_same_v<T, unsigned long long>) {
#if defined(__GNUG__)
        return __builtin_clzll(n);
#else
        return static_cast<int32_t>(math_functions::detail::lz_count_64_software(n));
#endif
    } else if constexpr (std::is_same_v<T, unsigned long>) {
#if defined(__GNUG__)
        return __builtin_clzl(n);
#else
        return static_cast<int32_t>(math_functions::detail::lz_count_64_software(n));
#endif
    } else {
        static_assert(
            std::is_same_v<T, unsigned int> || std::is_same_v<T, unsigned short> || std::is_same_v<T, unsigned char>,
            "Inappropriate integer type in countl_zero");
        const auto diff = static_cast<int>((sizeof(unsigned int) - sizeof(T)) * CHAR_BIT);
#if defined(__GNUG__)
        return __builtin_clz(n) - diff;
#else
        return static_cast<int32_t>(math_functions::detail::lz_count_32_software(n)) - diff;
#endif
    }
    // NOLINTEND(google-runtime-int)
#endif
}

template <class T>
#if CONFIG_HAS_CONCEPTS
    requires math_functions::unsigned_integral<T>
#endif
ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_CONST [[nodiscard]]
constexpr int32_t popcount(const T n) noexcept {
    static_assert(math_functions::is_unsigned_v<T>, "Unsigned integral type expected");

#if defined(HAS_INT128_TYPEDEF)
    if constexpr (std::is_same_v<T, uint128_t>) {
        // Reason: cppcheck can not deduce that n is uint128_t here
        // cppcheck-suppress [shiftTooManyBits]
        const uint64_t high = static_cast<uint64_t>(n >> 64U);
        const uint64_t low = static_cast<uint64_t>(n);
        return math_functions::popcount<uint64_t>(high) + math_functions::popcount<uint64_t>(low);
    } else
#endif

#ifdef MATH_FUNCTIONS_HAS_BIT
    {
        return std::popcount(n);
    }
#else
    // NOLINTBEGIN(google-runtime-int)
    if constexpr (std::is_same_v<T, unsigned long long>) {
#if defined(__GNUG__)
        return __builtin_popcountll(n);
#else
        return static_cast<int32_t>(math_functions::detail::pop_count_64_software(n));
#endif
    } else if constexpr (std::is_same_v<T, unsigned long>) {
#if defined(__GNUG__)
        return __builtin_popcountl(n);
#else
        return static_cast<int32_t>(math_functions::detail::pop_count_64_software(n));
#endif
    } else {
        static_assert(
            std::is_same_v<T, unsigned int> || std::is_same_v<T, unsigned short> || std::is_same_v<T, unsigned char>,
            "Inappropriate integer type in popcount");
#if defined(__GNUG__)
        return __builtin_popcount(n);
#else
        return static_cast<int32_t>(math_functions::detail::pop_count_32_software(n));
#endif
    }
    // NOLINTEND(google-runtime-int)
#endif
}

/// @brief Generates next n bits permutation of @a `x`, e.g.
///         0b0010011 ->
///         0b0010101 ->
///         0b0010110 ->
///         0b0011001 ->
///         0b0011010 ->
///         0b0011100 ->
///         0b0100011 ->
///         etc.
/// @note Special case:
///        0 -> 0
/// @param[in] x
/// @return
ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t next_n_bits_permutation(const uint32_t x) noexcept {
    // See
    // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2

    // t gets x's least significant 0 bits set to 1
    const uint32_t t = x | (x - 1);
    // Next set to 1 the most significant bit to change,
    // set to 0 the least significant ones, and add the necessary 1 bits.
    return (t + 1) |
           static_cast<uint32_t>(uint64_t{(~t & -~t) - 1} >> static_cast<uint32_t>(math_functions::countr_zero(x) + 1));
}

template <class UIntType>
ATTRIBUTE_CONST [[nodiscard]]
constexpr auto nearest_greater_equal_power_of_two(const UIntType n) noexcept {
    static_assert(math_functions::is_unsigned_v<UIntType> && sizeof(UIntType) >= sizeof(unsigned),
                  "unsigned integral type (at least unsigned int) is expected");

    using ShiftType = int32_t;
    const ShiftType shift = ShiftType{sizeof(n) * CHAR_BIT} - ShiftType{math_functions::countl_zero(n | 1U)} -
                            ShiftType{(n & (n - 1)) == 0};
    using RetType = typename std::conditional_t<(sizeof(UIntType) > sizeof(uint32_t)), UIntType, uint64_t>;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return RetType{1} << shift;
}

template <class UIntType>
ATTRIBUTE_CONST [[nodiscard]]
constexpr auto nearest_greater_power_of_two(const UIntType n) noexcept {
    static_assert(math_functions::is_unsigned_v<UIntType> && sizeof(UIntType) >= sizeof(unsigned),
                  "unsigned integral type (at least unsigned int) is expected");

    using ShiftType = int32_t;
    const ShiftType shift = ShiftType{sizeof(n) * CHAR_BIT} - ShiftType{math_functions::countl_zero(n)};
    using RetType = typename std::conditional_t<(sizeof(UIntType) > sizeof(uint32_t)), UIntType, uint64_t>;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return RetType{1} << shift;
}

template <class IntType>
ATTRIBUTE_CONST [[nodiscard]]
constexpr uint32_t most_significant_set_bit_position(const IntType n) noexcept {
    math_functions::detail::check_math_int_type<IntType>();

    const auto unsigned_n = math_functions::detail::to_uint_at_least_32(n);
    return uint32_t{sizeof(unsigned_n) * CHAR_BIT - 1} - static_cast<uint32_t>(math_functions::countl_zero(unsigned_n));
}

template <class IntType>
ATTRIBUTE_CONST [[nodiscard]]
constexpr IntType most_significant_set_bit(const IntType n) noexcept {
    using UIntType = math_functions::make_unsigned_t<IntType>;
    return static_cast<IntType>(n == 0 ? 0 : UIntType{1} << math_functions::most_significant_set_bit_position(n));
}

/// @brief If @a n != 0, return number that is power of 2 and
///         whose only bit is the lowest bit set in the @a n
///        Otherwise, return 0
/// @tparam IntType
/// @param[in] n
/// @return
template <class IntType>
ATTRIBUTE_CONST [[nodiscard]]
constexpr IntType least_significant_set_bit(const IntType n) noexcept {
    math_functions::detail::check_math_int_type<IntType>();

    const auto unsigned_n = math_functions::detail::to_uint_at_least_32(n);
    return static_cast<IntType>(unsigned_n & -unsigned_n);
}

/// @brief Return sum: sum of popcount(i & k) for all i from 0 to n inclusive
///  Using tex notation: \sum_{i = 0}^{n} popcount(i bitand k)
/// @tparam IntType
/// @param n
/// @param k
/// @return
template <class T>
#if CONFIG_HAS_CONCEPTS
    requires math_functions::unsigned_integral<T>
#endif
ATTRIBUTE_CONST [[nodiscard]]
constexpr math_functions::detail::try_double_bits_t<T> masked_popcount_sum(const T n, const T k) noexcept(
    detail::is_trivial_arithmetic_v<T>) {
    math_functions::detail::check_math_unsigned_int_type<T>();

    math_functions::detail::try_double_bits_t<T> popcount_sum = 0;
    if (unlikely(n == 0 || k == 0)) {
        return popcount_sum;
    }

    const uint32_t max_feasible_bit_position = std::min(math_functions::most_significant_set_bit_position(n),
                                                        math_functions::most_significant_set_bit_position(k));

    for (uint32_t j = 0; j <= max_feasible_bit_position; j++) {
        const bool k_has_jth_bit = (k & (T{1} << j)) != 0;
        if (!k_has_jth_bit) {
            continue;
        }

        const bool n_has_jth_bit = (n & (T{1} << j)) != 0;
        const T number_of_full_blocks_with_j_bit_set = (n >> j) / 2;
        const T nums_from_last_possibly_nonfull_block = n_has_jth_bit ? n - (((n >> j) << j) - 1) : T{0};
        const T number_of_nums_with_j_bit_set_from_blocks = number_of_full_blocks_with_j_bit_set << j;
        popcount_sum += number_of_nums_with_j_bit_set_from_blocks + nums_from_last_possibly_nonfull_block;
    }

    return popcount_sum;
}

/// @brief Return sum: sum of popcount(i) for all i from 0 to n inclusive
///  Using tex notation: \sum_{i = 0}^{n} popcount(i)
/// @tparam IntType
/// @param n
/// @param k
/// @return
template <class IntType>
ATTRIBUTE_CONST [[nodiscard]] constexpr IntType popcount_sum(const IntType n) noexcept {
    return math_functions::masked_popcount_sum(n, ~IntType{0});
}

ATTRIBUTE_CONST
[[nodiscard]]
constexpr bool is_correct_base_b_len_base(const uint8_t base) noexcept {
    return 2 <= base && base <= 36;
}

namespace detail {

/// @brief Realization taken from the gcc libstdc++ __to_chars_len
/// @tparam T
/// @param[in] value
/// @param[in] base
/// @return
template <class T>
ATTRIBUTE_CONST [[nodiscard]]
constexpr uint32_t base_b_len_impl(T value, const uint8_t base) noexcept {
    static_assert(math_functions::is_unsigned_v<T>);

    CONFIG_ASSUME_STATEMENT(is_correct_base_b_len_base(base));

    const uint32_t b = base;
    const uint32_t b2 = b * b;
    const uint32_t b3 = b2 * b;
    const uint32_t b4 = b3 * b;
    for (uint32_t n = 1;;) {
        if (value < b) {
            return n;
        }
        n++;
        if (value < b2) {
            return n;
        }
        n++;
        if (value < b3) {
            return n;
        }
        n++;
        if (value < b4) {
            return n;
        }
        n++;
        value /= b4;
    }
}

}  // namespace detail

/// @brief Length of the @a value in the base @a base
/// For example, with base = 10 return the same value as std::to_string(value).size() do
/// @tparam T
/// @param[in] value
/// @param[in] base
/// @return
template <class T>
ATTRIBUTE_ALWAYS_INLINE [[nodiscard]]
constexpr uint32_t base_b_len(const T value, const uint8_t base = 10) {
    math_functions::detail::check_math_int_type<T>();

    THROW_IF_NOT(math_functions::is_correct_base_b_len_base(base));

    if constexpr (math_functions::is_signed_v<T>) {
        const uint32_t is_negative{value < 0};
        return is_negative + math_functions::detail::base_b_len_impl(math_functions::uabs(value), base);
    } else {
        return math_functions::detail::base_b_len_impl(value, base);
    }
}

/// @brief For n > 0 returns ⌊log_2(n)⌋. For n = 0 returns (uint32_t)-1
/// @tparam UIntType unsigned integral type (at least unsigned int in size)
/// @param[in] n
/// @return
template <class UIntType>
#if CONFIG_HAS_CONCEPTS
    requires math_functions::unsigned_integral<UIntType>
#endif
ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_CONST [[nodiscard]]
constexpr uint32_t log2_floor(const UIntType n) noexcept {
    math_functions::detail::check_math_unsigned_int_type<UIntType>();

    const uint32_t log2_value = [n]() {
        const uint32_t shift{sizeof(n) * CHAR_BIT - 1};
#if defined(HAS_INT128_TYPEDEF)
        if constexpr (std::is_same_v<UIntType, uint128_t>) {
            const auto hi = static_cast<uint64_t>(n >> 64U);
            return hi != 0 ? (shift - static_cast<uint32_t>(math_functions::countl_zero(hi)))
                           : (math_functions::log2_floor(static_cast<uint64_t>(n)));
        } else
#endif
        {
            return shift - static_cast<uint32_t>(math_functions::countl_zero(n));
        }
    }();

    if (n != 0) {
        CONFIG_ASSUME_STATEMENT(log2_value < n);
        CONFIG_ASSUME_STATEMENT((UIntType{1} << log2_value) <= n);
    } else {
        CONFIG_ASSUME_STATEMENT(log2_value == static_cast<uint32_t>(-1));
    }

    return log2_value;
}

/// @brief For n > 0 returns ⌈log_2(n)⌉. For n = 0 returns (uint32_t)-1
/// @tparam UIntType unsigned integral type (at least unsigned int in size)
/// @param[in] n
/// @return
template <class UIntType>
#if CONFIG_HAS_CONCEPTS
    requires math_functions::unsigned_integral<UIntType>
#endif
ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_CONST [[nodiscard]]
constexpr uint32_t log2_ceil(const UIntType n) noexcept {
    return math_functions::log2_floor(n) + uint32_t{(n & (n - 1)) != 0};
}

template <class UIntType>
#if CONFIG_HAS_CONCEPTS
    requires math_functions::unsigned_integral<UIntType>
#endif
ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_CONST [[nodiscard]]
constexpr uint32_t base_2_len(const UIntType n) noexcept {
    // " | 1" operation does not affect answer for all
    //  numbers except n = 0. For n = 0 answer is 1.
    return math_functions::log2_floor(n | 1) + 1;
}

namespace detail {

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
inline constexpr const uint8_t log10_u32_table_1[33] = {
    10, 9, 9, 8, 8, 8, 7, 7, 7, 6, 6, 6, 6, 5, 5, 5, 4, 4, 4, 3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0,
};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
inline constexpr const uint32_t log10_u32_table_2[11] = {
    1U, 10U, 100U, 1000U, 10000U, 100000U, 1000000U, 10000000U, 100000000U, 1000000000U, 0U,
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
inline constexpr const uint64_t log10_u64_table[20] = {
    0ULL,
    9ULL,
    99ULL,
    999ULL,
    9999ULL,
    99999ULL,
    999999ULL,
    9999999ULL,
    99999999ULL,
    999999999ULL,
    9999999999ULL,
    99999999999ULL,
    999999999999ULL,
    9999999999999ULL,
    99999999999999ULL,
    999999999999999ULL,
    9999999999999999ULL,
    99999999999999999ULL,
    999999999999999999ULL,
    9999999999999999999ULL,
};

}  // namespace detail

/// @brief For n > 0 returns ⌊log_10(n)⌋. For n = 0 returns (uint32_t)-1
/// @param[in] n
/// @return
ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t log10_floor(const uint32_t n) noexcept {
    /**
     * See Hackers Delight 11-4
     */

    static_assert(math_functions::countl_zero(uint32_t{0}) == 32, "countl_zero detail error");
    const auto leading_zeros_count = static_cast<uint32_t>(math_functions::countl_zero(n));
    uint32_t digits = detail::log10_u32_table_1[leading_zeros_count];
    digits -= ((n - detail::log10_u32_table_2[digits]) >> 31U);
    return digits;
}

/// @brief For @a `n` > 0 returns ⌊log_10(n)⌋. For @a `n` = 0 returns
/// (uint32_t)-1
/// @param[in] n
/// @return
ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t log10_floor(const uint64_t n) noexcept {
    /**
     * See Hackers Delight 11-4
     */

    static_assert(math_functions::countl_zero(uint64_t{0}) == 64, "countl_zero detail error");
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    const int32_t approx_log10 = (19 * (63 - math_functions::countl_zero(n))) >> 6U;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    CONFIG_ASSUME_STATEMENT((-19 >> 6U) <= approx_log10 && approx_log10 <= ((19 * 63) >> 6U));

    const uint64_t adjustment = (detail::log10_u64_table[static_cast<uint32_t>(approx_log10 + 1)] - n) >> 63U;
    return static_cast<uint32_t>(approx_log10 + static_cast<int32_t>(adjustment));
}

ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t base_10_len(const uint32_t n) noexcept {
    // or `n` with 1 so that base_10_len(0) = 1
    return math_functions::log10_floor(n | 1U) + 1;
}

ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t base_10_len(const uint64_t n) noexcept {
    // or `n` with 1 so that base_10_len(0) = 1
    return math_functions::log10_floor(n | 1U) + 1;
}

template <class T>
struct ExtractPow2Result {
    T odd_part;
    uint32_t power;
};

/// @brief Find 2 numbers `odd_part` and `power` such that:
///         n = odd_part * (2 ^ power), where `odd_part` is odd if n != 0
/// @note  For n = 0 answer is { odd_part = 0, power = sizeof(n) * CHAR_BIT }
/// @param[in] n
/// @return Pair of odd_part and r
template <class UIntType>
#if CONFIG_HAS_CONCEPTS
    requires math_functions::unsigned_integral<UIntType>
#endif
ATTRIBUTE_CONST [[nodiscard]]
constexpr ExtractPow2Result<UIntType> extract_pow2(const UIntType n) noexcept {
    math_functions::detail::check_math_unsigned_int_type<UIntType>();

    const auto power = static_cast<uint32_t>(math_functions::countr_zero(n));
    const UIntType odd_part = n != 0 ? (n >> power) : 0;
    return {odd_part, power};
}

/// @brief Returns median of boolean variables x, y and z
/// @param x x
/// @param y y
/// @param z z
/// @return median of x, y and z
ATTRIBUTE_CONST
[[nodiscard]]
constexpr bool bool_median(const bool x, const bool y, const bool z) noexcept {
    return (x || y) && (y || z) && (x || z);
}

template <class T>
ATTRIBUTE_CONST [[nodiscard]]
constexpr T next_even(const T n) noexcept {
    math_functions::detail::check_math_unsigned_int_type<T>();
    return n + 2 - n % 2;
}

template <class T>
ATTRIBUTE_CONST [[nodiscard]]
constexpr T next_odd(const T n) noexcept {
    math_functions::detail::check_math_unsigned_int_type<T>();
    return n + 1 + n % 2;
}

template <class FloatType>
struct SumSinCos {
    FloatType sines_sum;
    FloatType cosines_sum;
};

/// @brief Function returns pair of 2 sums:
///         (
///             sin(alpha) +
///           + sin(alpha + beta) +
///           + sin(alpha + 2 beta) +
///           + sin(alpha + 3 beta) +
///           + ... +
///           + sin(alpha + (n - 1) beta)
///         ),
///         (
///             cos(alpha) +
///           + cos(alpha + beta) +
///           + cos(alpha + 2 beta) +
///           + cos(alpha + 3 beta) +
///           + ... +
///           + cos(alpha + (n - 1) beta)
///         ).
///        For n = 0 function returns (0, 0).
///        Proof of the formula can found on
///         https://blog.myrank.co.in/sum-of-sines-or-cosines-of-n-angles-in-a-p/
/// @tparam FloatType
/// @param alpha
/// @param beta
/// @param n
/// @return pair of (
///           sin(alpha) + sin(alpha + beta) + sin(alpha + 2 beta) + ... +
///            + sin(alpha + (n - 1) beta),
///           cos(alpha) + cos(alpha + beta) + cos(alpha + 2 beta) + ... +
///            + cos(alpha + (n - 1) beta)
///         )
template <class FloatType>
#if CONFIG_HAS_CONCEPTS
    requires std::floating_point<FloatType>
#endif
ATTRIBUTE_CONST [[nodiscard]]
SumSinCos<FloatType> sum_of_sines_and_cosines(const FloatType alpha, const FloatType beta, const uint32_t n) noexcept {
    static_assert(std::is_floating_point_v<FloatType>, "Invalid type in sum_of_sines_and_cosines");

    const FloatType nf = static_cast<FloatType>(n);
    const auto half_beta = beta / 2;
    const auto half_beta_sin = std::sin(half_beta);
    if (unlikely(std::abs(half_beta_sin) <= std::numeric_limits<FloatType>::epsilon())) {
        // If beta = 2 pi k, k \in Z
        return {
#if CONFIG_HAS_AT_LEAST_CXX_20
            .sines_sum =
#endif
                std::sin(alpha) * nf,
#if CONFIG_HAS_AT_LEAST_CXX_20
            .cosines_sum =
#endif
                std::cos(alpha) * nf,
        };
    }

    const auto sin_numer_over_sin_denum = std::sin(nf * half_beta) / half_beta_sin;
    const auto arg = alpha + (nf - 1) * half_beta;
    const auto sin_mult = std::sin(arg);
    const auto cos_mult = std::cos(arg);
    return {
#if CONFIG_HAS_AT_LEAST_CXX_20
        .sines_sum =
#endif
            sin_numer_over_sin_denum * sin_mult,
#if CONFIG_HAS_AT_LEAST_CXX_20
        .cosines_sum =
#endif
            sin_numer_over_sin_denum * cos_mult,
    };
}

namespace detail {

/// @brief Returns max number of possible different
///         prime divisors for the given @a `n`.
///        Note that it may be greater than the real
///         number of different prime divisors of @a `n`.
/// @param[in] n
/// @return
ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t max_number_of_unique_prime_divisors(const uint32_t n) noexcept {
    constexpr uint32_t kBoundary1 = 2;
    constexpr uint32_t kBoundary2 = kBoundary1 * 3;
    constexpr uint32_t kBoundary3 = kBoundary2 * 5;
    constexpr uint32_t kBoundary4 = kBoundary3 * 7;
    constexpr uint32_t kBoundary5 = kBoundary4 * 11;
    constexpr uint32_t kBoundary6 = kBoundary5 * 13;
    constexpr uint32_t kBoundary7 = kBoundary6 * 17;
    constexpr uint32_t kBoundary8 = kBoundary7 * 19;
    constexpr uint32_t kBoundary9 = kBoundary8 * 23;

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG
#if CONFIG_COMPILER_ID == CONFIG_GCC_COMPILER_ID
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-case-range"
#endif
    switch (n) {
        case 0 ... kBoundary1 - 1: {
            return 0;
        }
        case kBoundary1 ... kBoundary2 - 1: {
            return 1;
        }
        case kBoundary2 ... kBoundary3 - 1: {
            return 2;
        }
        case kBoundary3 ... kBoundary4 - 1: {
            return 3;
        }
        case kBoundary4 ... kBoundary5 - 1: {
            return 4;
        }
        case kBoundary5 ... kBoundary6 - 1: {
            return 5;
        }
        case kBoundary6 ... kBoundary7 - 1: {
            return 6;
        }
        case kBoundary7 ... kBoundary8 - 1: {
            return 7;
        }
        case kBoundary8 ... kBoundary9 - 1: {
            return 8;
        }
        default: {
            return 9;
        }
    }
#if CONFIG_COMPILER_ID == CONFIG_GCC_COMPILER_ID
#pragma GCC diagnostic pop
#else
#pragma clang diagnostic pop
#endif
#else
    if (n >= kBoundary5) {
        if (n >= kBoundary7) {
            if (n >= kBoundary8) {
                if (n >= kBoundary9) {
                    return 9;
                } else {
                    return 8;
                }
            } else {
                return 7;
            }
        } else {
            if (n >= kBoundary6) {
                return 6;
            } else {
                return 5;
            }
        }
    } else {
        if (n >= kBoundary3) {
            if (n >= kBoundary4) {
                return 4;
            } else {
                return 3;
            }
        } else {
            if (n >= kBoundary2) {
                return 2;
            } else {
                if (n >= kBoundary1) {
                    return 1;
                } else {
                    return 0;
                }
            }
        }
    }
#endif
}

}  // namespace detail

template <class NumericType>
struct [[nodiscard]] PrimeFactor final {
    using IntType = NumericType;

    ATTRIBUTE_ALWAYS_INLINE
    constexpr PrimeFactor(const IntType prime_factor, const uint32_t prime_factor_power) noexcept
        : factor(prime_factor), factor_power(prime_factor_power) {}

    IntType factor;
    uint32_t factor_power;
};

#if CONFIG_VECTOR_SUPPORTS_CONSTEXPR_OPERATIONS
#define CONSTEXPR_VECTOR constexpr
#else
#define CONSTEXPR_VECTOR inline
#endif

// clang-format off

template <class IntType, class F>
ATTRIBUTE_ALWAYS_INLINE
constexpr void visit_prime_factors(const IntType n, F visitor) noexcept(
    std::is_nothrow_invocable_v<F, math_functions::PrimeFactor<math_functions::make_unsigned_t<IntType>>>) {
    // clang-format on

    math_functions::detail::check_math_int_type<IntType>();

    static_assert(sizeof(IntType) >= sizeof(int), "integral type should be at least int in size");

    using UnsignedIntType = math_functions::make_unsigned_t<IntType>;
    using PrimeFactorType = math_functions::PrimeFactor<UnsignedIntType>;

    static_assert(std::is_invocable_v<F, PrimeFactorType>,
                  "Passed function should accept type PrimeFactor<make_unsigned<IntType>>");

    UnsignedIntType n_abs = math_functions::uabs(n);

    constexpr bool check_early_exit = std::is_invocable_r_v<bool, F, PrimeFactorType>;

    if (n_abs % 2 == 0 && n_abs > 0) {
        // n_abs = s * 2^pow_of_2, where s is odd
        auto [s, pow_of_2] = math_functions::extract_pow2(n_abs);
        n_abs = s;
        if constexpr (check_early_exit) {
            if (!visitor(PrimeFactorType{UnsignedIntType{2}, pow_of_2})) {
                return;
            }
        } else {
            visitor(PrimeFactorType{UnsignedIntType{2}, pow_of_2});
        }
    }

    for (UnsignedIntType d = 3; d * d <= n_abs; d += 2) {
        CONFIG_ASSUME_STATEMENT(d >= 3);
        if (n_abs % d == 0) {
            uint32_t pow_of_d = 0;
            do {
                pow_of_d++;
                n_abs /= d;
            } while (n_abs % d == 0);

            if constexpr (check_early_exit) {
                if (!visitor(PrimeFactorType{d, pow_of_d})) {
                    return;
                }
            } else {
                visitor(PrimeFactorType{d, pow_of_d});
            }
        }
    }

    if (n_abs > 1) {
        visitor(PrimeFactorType{n_abs, uint32_t{1}});
    }
}

/// @brief
/// @tparam IntType
/// @param[in] n
/// @return vector of pairs { prime_div : power_of_prime_div },
///          sorted by prime_div.
template <class IntType>
[[nodiscard]] CONSTEXPR_VECTOR auto prime_factors_as_vector(const IntType n)
    -> std::vector<math_functions::PrimeFactor<math_functions::make_unsigned_t<IntType>>> {
    math_functions::detail::check_math_int_type<IntType>();

    using UnsignedIntType = math_functions::make_unsigned_t<IntType>;
    std::vector<math_functions::PrimeFactor<UnsignedIntType>> prime_factors_vector;

    constexpr bool kReservePlaceForFactors = std::is_same_v<UnsignedIntType, uint32_t>;
    if constexpr (kReservePlaceForFactors) {
        prime_factors_vector.reserve(
            math_functions::detail::max_number_of_unique_prime_divisors(math_functions::uabs(n)));
    }

    math_functions::visit_prime_factors(
        n, [&prime_factors_vector](math_functions::PrimeFactor<UnsignedIntType> pf)
#if CONFIG_VECTOR_SUPPORTS_CONSTEXPR_OPERATIONS
               constexpr
#endif
        noexcept(kReservePlaceForFactors) { prime_factors_vector.push_back(std::move(pf)); });

    return prime_factors_vector;
}

template <class IntType>
[[nodiscard]] inline auto prime_factors_as_map(const IntType n)
    -> std::map<math_functions::make_unsigned_t<IntType>, uint32_t> {
    math_functions::detail::check_math_int_type<IntType>();

    using UnsignedIntType = math_functions::make_unsigned_t<IntType>;
    std::map<UnsignedIntType, uint32_t> prime_factors_map;

    math_functions::visit_prime_factors(n, [&prime_factors_map, iter = prime_factors_map.end()](
                                               math_functions::PrimeFactor<UnsignedIntType> pf) mutable {
        iter = prime_factors_map.emplace_hint(iter, pf.factor, pf.factor_power);
    });

    return prime_factors_map;
}

/// @brief https://cp-algorithms.com/algebra/prime-sieve-linear.html
class [[nodiscard]] ATTRIBUTE_GSL_OWNER(std::vector<uint32_t>) Factorizer final {
public:
    using PrimeFactors = std::vector<PrimeFactor<uint32_t>>;
    using NumbersContainer = std::vector<uint32_t>;

    explicit CONSTEXPR_VECTOR Factorizer(const uint32_t n) : primes_{}, least_prime_factor_(size_t{n} + 1) {
        for (uint32_t i = 2; i <= size_t{n}; i++) {
            if (least_prime_factor_[i] == 0) {
                least_prime_factor_[i] = i;
                primes_.push_back(i);
            }
            for (size_t prime_index = 0;; prime_index++) {
                const auto p = primes_.at(prime_index);
                const size_t x = size_t{p} * i;
                if (x > n) {
                    break;
                }
                least_prime_factor_[x] = p;
                // assert(p <= least_prime_factor_[i]);
                if (p == least_prime_factor_[i]) {
                    break;
                }
            }
        }
    }

    ATTRIBUTE_PURE
    [[nodiscard]]
    constexpr const NumbersContainer& sorted_primes() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return primes_;
    }

    ATTRIBUTE_PURE
    [[nodiscard]]
    constexpr const NumbersContainer& least_prime_factors() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return least_prime_factor_;
    }

    [[nodiscard]] CONSTEXPR_VECTOR bool is_prime(const uint32_t n) const noexcept {
        return least_prime_factor_[n] == n && n >= 2;
    }

    [[nodiscard]]
    CONSTEXPR_VECTOR size_t max_checkable_number() const noexcept {
        return least_prime_factor_.size() - 1;
    }

    [[nodiscard]] CONSTEXPR_VECTOR PrimeFactors prime_factors(uint32_t n) const {
        PrimeFactors pfs;
        if (n % 2 == 0 && n > 0) {
            const auto [n_div_pow_of_2, power_of_2] = math_functions::extract_pow2(n);
            pfs.emplace_back(uint32_t{2}, power_of_2);
            n = n_div_pow_of_2;
        }

        while (n >= 3) {
            const auto lpf = least_prime_factor_[n];
            if (pfs.empty() || pfs.back().factor != lpf) {
                // assert(pfs.empty() || pfs.back().factor < lpf);
                pfs.emplace_back(lpf, uint32_t{1});
            } else {
                pfs.back().factor_power++;
            }
            // assert(n % lpf == 0);
            CONFIG_ASSUME_STATEMENT(n % lpf == 0);
            n /= lpf;
        }

        return pfs;
    }

    [[nodiscard]]
    CONSTEXPR_VECTOR uint32_t number_of_unique_prime_factors(const uint32_t n) const noexcept {
        return math_functions::Factorizer::number_of_unique_prime_factors_impl(least_prime_factor_.data(), n);
    }

private:
    ATTRIBUTE_PURE
    ATTRIBUTE_ACCESS(read_only, 1)
    [[nodiscard]]
    static constexpr uint32_t number_of_unique_prime_factors_impl(const uint32_t* const
                                                                  RESTRICT_QUALIFIER least_prime_factor,
                                                                  uint32_t n) noexcept {
        uint32_t unique_pfs_count = 0;
        uint32_t last_pf = 0;
        if (n % 2 == 0) {
            if (unlikely(n == 0)) {
                return unique_pfs_count;
            }
            n = math_functions::extract_pow2(n).odd_part;
            last_pf = 2;
            unique_pfs_count++;
        }

        while (n >= 3) {
            const uint32_t least_pf = least_prime_factor[n];
            CONFIG_ASSUME_STATEMENT(least_pf >= 2);
            unique_pfs_count += least_pf != last_pf;
            n /= least_pf;
            last_pf = least_pf;
        }

        return unique_pfs_count;
    }

    NumbersContainer primes_;
    NumbersContainer least_prime_factor_;
};

/// @brief Find all prime numbers in [2; n]
/// @param n inclusive upper bound
/// @return vector, such that vector[n] == true \iff n is prime
[[nodiscard]] CONSTEXPR_VECTOR std::vector<bool> dynamic_primes_sieve(const uint32_t n) {
    std::vector<bool> primes(size_t{n} + 1, true);
    primes[0] = false;
    if (likely(n > 0)) {
        primes[1] = false;
        const uint32_t root = math_functions::isqrt(n);
        if (const uint32_t i = 2; i <= root) {
            for (size_t j = size_t{i} * size_t{i}; j <= n; j += i) {
                primes[j] = false;
            }
        }
        for (uint32_t i = 3; i <= root; i += 2) {
            if (primes[i]) {
                for (size_t j = size_t{i} * size_t{i}; j <= n; j += i) {
                    primes[j] = false;
                }
            }
        }
    }

    return primes;
}

// https://en.cppreference.com/w/cpp/feature_test
#if defined(__cpp_lib_constexpr_bitset) && \
    (__cpp_lib_constexpr_bitset >= 202207L || (CONFIG_HAS_AT_LEAST_CXX_23 && __cpp_lib_constexpr_bitset >= 202202L))
#define PRIMES_SIEVE_INITIALIZED_IN_COMPILE_TIME 1
#else
#define PRIMES_SIEVE_INITIALIZED_IN_COMPILE_TIME 0
#endif

#if PRIMES_SIEVE_INITIALIZED_IN_COMPILE_TIME
#define CONSTEXPR_BITSET_OPS constexpr
#if defined(__cpp_constexpr) && __cpp_constexpr >= 202211L
#define CONSTEXPR_FIXED_PRIMES_SIEVE constexpr
#else
#define CONSTEXPR_FIXED_PRIMES_SIEVE inline
#endif
#else
#define CONSTEXPR_BITSET_OPS
#define CONSTEXPR_FIXED_PRIMES_SIEVE inline
#endif

template <uint32_t N>
using PrimesSet = std::bitset<size_t{N} + 1>;

// clang-format off

/// @brief Find all prime numbers in [2; N]
/// @tparam N exclusive upper bound
/// @return bitset, such that bitset[n] == true \iff n is prime
template <uint32_t N>
#if PRIMES_SIEVE_INITIALIZED_IN_COMPILE_TIME
ATTRIBUTE_CONST
#endif
[[nodiscard]] CONSTEXPR_FIXED_PRIMES_SIEVE const PrimesSet<N>& fixed_primes_sieve() noexcept {
    // clang-format on
    static CONSTEXPR_BITSET_OPS const PrimesSet<N> primes_bs = []() CONSTEXPR_BITSET_OPS noexcept -> PrimesSet<N> {
        PrimesSet<N> primes{};
        primes.set();
        primes[0] = false;
        if constexpr (N >= 1) {
            primes[1] = false;
            constexpr uint32_t root = math_functions::isqrt(N);
            if constexpr (constexpr uint32_t i = 2; i <= root) {
                // NOLINTNEXTLINE(bugprone-implicit-widening-of-multiplication-result)
                for (size_t j = i * i; j <= N; j += i) {
                    primes[j] = false;
                }
            }
            for (uint32_t i = 3; i <= root; i += 2) {
                if (primes[i]) {
                    static_assert(root < std::numeric_limits<uint16_t>::max(), "isqrt impl error");
                    // NOLINTNEXTLINE(bugprone-implicit-widening-of-multiplication-result)
                    for (size_t j = i * i; j <= N; j += i) {
                        primes[j] = false;
                    }
                }
            }
        }
        return primes;
    }();

    return primes_bs;
}

#undef CONSTEXPR_FIXED_PRIMES_SIEVE
#undef CONSTEXPR_BITSET_OPS
#undef PRIMES_SIEVE_INITIALIZED_IN_COMPILE_TIME

template <class UIntType>
struct [[nodiscard]] ExtEuclidAlgoRet {
    int64_t u_value;
    int64_t v_value;
    UIntType gcd_value;
};

/// @brief
/// Finds such integer u and v so that `a * u + b * v = gcd(a, b)`
/// let gcd(a, b) >= 0
/// if a == 0 => u == 0 && v == sign(b) && (a * u + b * v = b = gcd(0, b))
/// if b == 0 => v == 0 && u == sign(a) && (a * u + b * v = a = gcd(a, 0))
/// if a != 0 => |v| <= |a|
/// if b != 0 => |u| <= |b|
/// Works in O(log(min(a, b)))
/// @tparam IntType integer type
/// @param a a value
/// @param b b value
/// @return {u, v, gcd(a, b)}
template <typename T>
ATTRIBUTE_CONST [[nodiscard]]
constexpr auto extended_euclid_algorithm(const T a, const T b) noexcept(detail::is_trivial_arithmetic_v<T>) {
    math_functions::detail::check_math_int_type<T>();

    int64_t u_previous = a != 0;
    int64_t u_current = 0;
    int64_t v_previous = 0;
    int64_t v_current = 1;

    using CompIntType = std::conditional_t<sizeof(T) >= sizeof(int), T, int64_t>;

    CompIntType r_previous = a;
    CompIntType r_current = b;
    while (r_current != 0) {
        const int64_t q_current = static_cast<int64_t>(r_previous / r_current);
        const CompIntType r_next = r_previous % r_current;

        r_previous = r_current;
        r_current = r_next;

        const int64_t u_next = u_previous - u_current * q_current;
        u_previous = u_current;
        u_current = u_next;

        const int64_t v_next = v_previous - v_current * q_current;
        v_previous = v_current;
        v_current = v_next;
    }

    if constexpr (math_functions::is_signed_v<CompIntType>) {
        if (r_previous < 0) {
            u_previous = -u_previous;
            v_previous = -v_previous;
            r_previous = -r_previous;
        }
    }

    using RetUIntType = typename math_functions::make_unsigned_t<CompIntType>;
    return ExtEuclidAlgoRet<RetUIntType>{
        u_previous,
        v_previous,
        static_cast<RetUIntType>(r_previous),
    };
}

inline constexpr auto kNoCongruenceSolution = std::numeric_limits<uint32_t>::max();

namespace detail {

struct HelperRetType {
    uint32_t x0;  // first solution of congruence
    uint32_t d;   // gcd(a, m)
    uint32_t m_;  // m / d
};

ATTRIBUTE_CONST
[[nodiscard]]
constexpr HelperRetType congruence_helper(const uint32_t a, const uint32_t c, const uint32_t m) noexcept {
    const uint32_t d = std::gcd(a, m);
    if (m == 0 || c % d != 0) {
        return {kNoCongruenceSolution, 0, 0};
    }

    CONFIG_ASSUME_STATEMENT(a == 0 || a >= d);
    CONFIG_ASSUME_STATEMENT(a % d == 0);
    CONFIG_ASSUME_STATEMENT(m >= d);
    CONFIG_ASSUME_STATEMENT(m % d == 0);

    /*
     * Solves a_ * x === c_ (mod m_) as gcd(a_, m_) == 1
     */
    const uint32_t a_ = a / d;
    const uint32_t c_ = c / d;
    const uint32_t m_ = m / d;
    // a_ * u_ + m_ * v_ == 1
    const int64_t u_ = math_functions::extended_euclid_algorithm<uint32_t>(a_, m_).u_value;
    const auto unsigned_u_ = static_cast<uint64_t>(u_ >= 0 ? u_ : u_ + m_);

    // a_ * (u_ * c_) + m_ * (v_ * c_) == c_
    // a * (u_ * c_) + m * (v_ * c_) == c
    // x0 = u_ * c_
    const auto x0 = static_cast<uint32_t>((unsigned_u_ * c_) % m_);
    CONFIG_ASSUME_STATEMENT(x0 != math_functions::kNoCongruenceSolution);
    return {x0, d, m_};
}

[[nodiscard]]
CONSTEXPR_VECTOR std::vector<uint32_t> solve_congruence_modulo_m_all_roots_impl(const uint32_t a,
                                                                                const uint32_t c,
                                                                                const uint32_t m) {
    const auto [x0, d, m_] = math_functions::detail::congruence_helper(a, c, m);
    std::vector<uint32_t> solutions(d);
    auto x = x0;
    for (uint32_t& ith_solution : solutions) {
        ith_solution = x;
        x += m_;
    }

    return solutions;
}

ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t solve_congruence_modulo_m_impl(const uint32_t a, const uint32_t c, const uint32_t m) noexcept {
    return math_functions::detail::congruence_helper(a, c, m).x0;
}

template <class T>
ATTRIBUTE_CONST ATTRIBUTE_ALWAYS_INLINE [[nodiscard]]
constexpr uint32_t congruence_arg(const T x, ATTRIBUTE_MAYBE_UNUSED const uint32_t m) noexcept {
    math_functions::detail::check_math_int_type<T>();

    if constexpr (math_functions::is_unsigned_v<T>) {
        if constexpr (sizeof(x) > sizeof(uint32_t)) {
            return static_cast<uint32_t>(x % m);
        } else {
            return static_cast<uint32_t>(x);
        }
    } else {
        const auto x_mod_m = [x, m]() constexpr noexcept {
            if constexpr (sizeof(x) > sizeof(uint32_t)) {
                return x % m;
            } else {
                return static_cast<int64_t>(x) % m;
            }
        }();
        return static_cast<uint32_t>(x_mod_m >= 0 ? x_mod_m : x_mod_m + m);
    }
}

}  // namespace detail

// clang-format off

/// @brief Solves congruence a * x ≡ c (mod m)
/// @note Roots exist <=> c % gcd(a, m) == 0.
///       If roots exist, then exactly gcd(a, m)
///        roots are returned (see @return section).
///       Works in O(log(min(a, m)) + gcd(a, m)).
/// @tparam T1
/// @tparam T2
/// @param a
/// @param c
/// @param m
/// @return If roots exist, return vector of gcd(a, m) roots
///          such that 0 <= x_{0} < x_{1} < ... < x_{gcd(a, m)-1} < m,
///          x_{0} < m / gcd(a, m), x_{i + 1} = x_{i} + m / gcd(a, m).
///         Otherwise, return empty vector.
template <class T1, class T2>
ATTRIBUTE_ALWAYS_INLINE
[[nodiscard]]
CONSTEXPR_VECTOR
std::vector<uint32_t> solve_congruence_modulo_m_all_roots(const T1 a, const T2 c, const uint32_t m) {
    // clang-format on
    THROW_IF(m == 0);
    return math_functions::detail::solve_congruence_modulo_m_all_roots_impl(
        math_functions::detail::congruence_arg(a, m), math_functions::detail::congruence_arg(c, m), m);
}

// clang-format off

/// @brief Solves modulus congruence a * x ≡ c (mod m)
/// @note Roots exist <=> c % gcd(a, m) == 0.
///       If roots exist, then exactly 1 root is returned, and
///        it is the least of all roots (see @fn solve_congruence_modulo_m_all_roots).
///       Works in O(log(min(a, m))
/// @tparam T1
/// @tparam T2
/// @param a
/// @param c
/// @param m
/// @return If roots exist, return root x such that 0 <= x < m / gcd(a, m).
///         Otherwise, return math_functions::kNoCongruenceSolution
template <class T1, class T2>
ATTRIBUTE_CONST
ATTRIBUTE_ALWAYS_INLINE
[[nodiscard]]
constexpr uint32_t solve_congruence_modulo_m(const T1 a, const T2 c, const uint32_t m) noexcept {
    // clang-format on
    THROW_IF(m == 0);
    return math_functions::detail::solve_congruence_modulo_m_impl(math_functions::detail::congruence_arg(a, m),
                                                                  math_functions::detail::congruence_arg(c, m), m);
}

#if CONFIG_HAS_CONCEPTS

/// @brief Solves modulus congruence a * x ≡ 1 (mod m) (e.g. a^{-1} mod m)
/// @note a^{-1} mod m exists <=> gcd(a, m) == 1.
///       Works in O(log(min(a, m))
/// @tparam T
/// @param a
/// @param m
/// @return
template <math_functions::math_integral_type T>
ATTRIBUTE_CONST [[nodiscard]]
constexpr uint32_t inv_mod_m(const T a, const uint32_t m) noexcept(detail::is_trivial_arithmetic_v<T>) {
    return math_functions::solve_congruence_modulo_m(a, uint32_t{1}, m);
}

#else

/// @brief Solves modulus congruence a * x ≡ 1 (mod m) (e.g. a^{-1} mod m)
/// @note a^{-1} mod m exists <=> gcd(a, m) == 1.
///       Works in O(log(min(a, m))
/// @tparam T
/// @param a
/// @param m
/// @return
template <class T, std::enable_if_t<math_functions::is_integral_v<T>, int> = 0>
ATTRIBUTE_CONST [[nodiscard]]
constexpr uint32_t inv_mod_m(const T a, uint32_t m) noexcept(detail::is_trivial_arithmetic_v<T>) {
    return math_functions::solve_congruence_modulo_m(a, uint32_t{1}, m);
}

#endif

struct InverseResult {
    std::vector<uint32_t> numbers_mod_m;
    std::vector<uint32_t> inversed_numbers;
};

namespace detail {

// clang-format off

template <class Iter, class IterSentinel>
[[nodiscard]]
CONSTEXPR_VECTOR
math_functions::InverseResult inv_range_mod_m_impl(Iter nums_begin, const IterSentinel nums_end, const uint32_t m) {
    // clang-format on

    const auto n = static_cast<size_t>(std::distance(nums_begin, nums_end));
    auto res = math_functions::InverseResult{
#if CONFIG_HAS_AT_LEAST_CXX_20
        .numbers_mod_m =
#endif
            std::vector<uint32_t>(n),
#if CONFIG_HAS_AT_LEAST_CXX_20
        .inversed_numbers =
#endif
            std::vector<uint32_t>(n),
    };

    uint32_t prod_mod_m = 1;
    {
        auto nums_mod_m_iter = res.numbers_mod_m.begin();
        auto inv_nums_mod_m_iter = res.inversed_numbers.begin();
        for (auto iter = nums_begin; iter != nums_end; ++iter, ++nums_mod_m_iter, ++inv_nums_mod_m_iter) {
            const auto num_mod_m = math_functions::detail::congruence_arg(*iter, m);
            *nums_mod_m_iter = num_mod_m;
            *inv_nums_mod_m_iter = prod_mod_m;
            prod_mod_m = static_cast<uint32_t>((uint64_t{prod_mod_m} * num_mod_m) % m);
        }
    }

    const uint32_t inv_nums_prod_mod_m = math_functions::inv_mod_m(prod_mod_m, m);

    {
        auto inv_nums_mod_m_iter = res.inversed_numbers.rbegin();
        uint32_t suffix_prod = 1;
        for (auto iter = res.numbers_mod_m.rbegin(), nums_mod_m_rend = res.numbers_mod_m.rend();
             iter != nums_mod_m_rend; ++iter, ++inv_nums_mod_m_iter) {
            const uint64_t t = (uint64_t{suffix_prod} * *inv_nums_mod_m_iter) % m;
            *inv_nums_mod_m_iter = static_cast<uint32_t>((t * inv_nums_prod_mod_m) % m);
            suffix_prod = static_cast<uint32_t>((uint64_t{suffix_prod} * *iter) % m);
        }
    }

    return res;
}

}  // namespace detail

#if CONFIG_HAS_CONCEPTS && defined(MATH_FUNCTIONS_HAS_RANGES)

template <class Iterator>
concept integral_forward_iterator =
    std::forward_iterator<Iterator> && math_functions::math_integral_type<typename std::iter_value_t<Iterator>>;

template <math_functions::integral_forward_iterator Iterator, std::sentinel_for<Iterator> Sentinel>
[[nodiscard]]
CONSTEXPR_VECTOR math_functions::InverseResult inv_range_mod_m(Iterator nums_begin,
                                                               Sentinel nums_end,
                                                               const uint32_t m) {
    THROW_IF(m == 0);
    return math_functions::detail::inv_range_mod_m_impl(std::move(nums_begin), std::move(nums_end), m);
}

/// @brief Inverse @a nums mod m
/// @note Works in O(nums.size())
/// @tparam T
/// @param nums
/// @param m
/// @return
template <std::ranges::forward_range Range>
[[nodiscard]]
CONSTEXPR_VECTOR math_functions::InverseResult inv_range_mod_m(const Range& nums, const uint32_t m) {
    return math_functions::inv_range_mod_m(std::begin(nums), std::end(nums), m);
}

#else

// clang-format off

template <class Iter,
          class Sentinel,
          std::enable_if_t<math_functions::is_math_integral_type_v<typename std::iterator_traits<Iter>::value_type>, int> = 0>
[[nodiscard]]
CONSTEXPR_VECTOR
math_functions::InverseResult inv_range_mod_m(Iter nums_iter_begin,
                                              Sentinel nums_iter_end,
                                              const uint32_t m) {
    THROW_IF(m == 0);
    return math_functions::detail::inv_range_mod_m_impl(std::move(nums_iter_begin), std::move(nums_iter_end), m);
}

template <class Range,
          std::enable_if_t<math_functions::is_integral_v<
                             typename std::iterator_traits<decltype(std::begin(std::declval<const Range&>()))>::value_type
                           >
                           &&
                           math_functions::is_integral_v<
                             typename std::iterator_traits<decltype(std::end(std::declval<const Range&>()))>::value_type
                           >,
                           int> = 0>
[[nodiscard]]
CONSTEXPR_VECTOR math_functions::InverseResult inv_range_mod_m(const Range& nums, const uint32_t m) {
    return math_functions::inv_range_mod_m(std::begin(nums), std::end(nums), m);
}

// clang-format on

#endif

/// @brief Solve congruence 2^k * x ≡ c (mod m),
///        Works in O(min(k, log(m)))
/// @note  Faster implementation of @fn solve_congruence_modulo_m
///        for a = 2^k.
/// @param k
/// @param c
/// @param m
/// @return
ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t solve_binary_congruence_modulo_m(const uint32_t k, const uint32_t c, const uint32_t m) noexcept {
    if (unlikely(m == 0)) {
        return math_functions::kNoCongruenceSolution;
    }

    const auto [r, s] = math_functions::extract_pow2(m);
    CONFIG_ASSUME_STATEMENT(r >= 1);
    const auto min_k_s = std::min(k, uint32_t{s});
    CONFIG_ASSUME_STATEMENT(min_k_s < 32);
    // gcd(2^k, m)
    const auto gcd_2k_m = uint32_t{1} << min_k_s;
    if (c % gcd_2k_m != 0) {
        return math_functions::kNoCongruenceSolution;
    }

    const auto c_ = c >> min_k_s;
    const auto m_ = m >> min_k_s;
#ifdef __clang_analyzer__
    [[clang::suppress]]
#endif
    const auto c_mod_m_ = c_ % m_;
    if (min_k_s == k) {
        return c_mod_m_;
    }

    CONFIG_ASSUME_STATEMENT(min_k_s == s);
    CONFIG_ASSUME_STATEMENT(k > s);
    CONFIG_ASSUME_STATEMENT(m_ == r);
    CONFIG_ASSUME_STATEMENT(m_ % 2 == 1);
    /**
     * Solve 2^{k-s} * x ≡ c / 2^{s} (mod r), where r = m_ = m / 2^{s}
     */
    uint64_t rhs = c_mod_m_;
    auto lhs_bin_power = k - s;

    constexpr unsigned kThreshold = 60;
    if (lhs_bin_power > kThreshold) {
        const int64_t u_ = math_functions::extended_euclid_algorithm<uint32_t>(
                               math_functions::bin_pow_mod(uint32_t{2}, lhs_bin_power, m_), m_)
                               .u_value;
        CONFIG_ASSUME_STATEMENT(u_ < m_);
        const auto unsigned_u_ = static_cast<uint64_t>(u_ >= 0 ? u_ : u_ + m_);
        CONFIG_ASSUME_STATEMENT(unsigned_u_ < m_);
        rhs *= unsigned_u_;
    } else {
        /**
         * The algorithm can be modified by checking:
         * if c % 4 = 1, then if m % 4 = 3, c += m, else c -= m
         * or
         * if c % 4 = 3, then if m % 4 = 3, c -= m, else c += m
         * then
         * k -= 2, c /= 4
         */
        while (lhs_bin_power > 0) {
            if (rhs % 2 != 0) {
                rhs += m_;
            }
            rhs /= 2;
            lhs_bin_power--;
        }
    }

    const auto x0 = static_cast<uint32_t>(rhs % m_);
    CONFIG_ASSUME_STATEMENT(x0 != math_functions::kNoCongruenceSolution);
    return x0;
}

/// @brief Return max q, such that n! ≡ 0 mod(k^q), where k > 1 and n >= 0
/// @param n
/// @param k
/// @return q if k > 1 and std::numeric_limits<uint32_t>::max() otherwise
ATTRIBUTE_CONST
[[nodiscard]]
constexpr uint32_t solve_factorial_congruence(const uint32_t n, const uint32_t k) noexcept {
    /*
     * let k  = p_1^a_1 * p_2^a_2 * ... * p_m^a_m
     * let n! = p_1^b_1 * p_2^b_2 * ...
     * then b_i = ⌊n / p_i⌋ + ⌊n / p_i^2⌋ + ⌊n / p_i^3⌋ + ...
     *
     * then q = min{ b_i / a_i | 1 <= i <= m }
     **/

    auto ans = std::numeric_limits<uint32_t>::max();
    math_functions::visit_prime_factors(k,
                                        [&ans, n](const math_functions::PrimeFactor<uint32_t> pf) constexpr noexcept {
                                            uint64_t pow_of_p_i = pf.factor;
                                            /**
                                             * max b_i can be reached if n = 4294967295 and p_i = 2, then:
                                             * b_i = ⌊4294967295 / 2⌋
                                             *     + ⌊4294967295 / 4⌋
                                             *     + ⌊4294967295 / 8⌋
                                             *     + ...
                                             *     + ⌊4294967295 / 2147483648⌋
                                             * < 4294967295 / 2
                                             * + 4294967295 / 4
                                             * + 4294967295 / 8
                                             * + ...
                                             * + 4294967295 / 2147483648
                                             * = 4294967295 * (1 - 1/(2^31))
                                             * < 4294967295
                                             *
                                             * Hence b_i fits into the uint32_t
                                             */

                                            if (pow_of_p_i > n) {
                                                ans = 0;
                                                return false;
                                            }

                                            uint32_t b_i = 0;
                                            do {
                                                b_i += n / static_cast<uint32_t>(pow_of_p_i);
                                                pow_of_p_i *= pf.factor;
                                            } while (pow_of_p_i <= n);

                                            const uint32_t a_i = pf.factor_power;
                                            ans = std::min(ans, b_i / a_i);
                                            return true;
                                        });

    return ans;
}

ATTRIBUTE_CONST
[[nodiscard]]
constexpr bool is_perfect_number(const uint32_t n) noexcept {
    switch (n) {
        case 6:
        case 28:
        case 496:
        case 8128:
        case 33550336: {
            return true;
        }
        default: {
            return false;
        }
    }
}

ATTRIBUTE_CONST
[[nodiscard]]
constexpr bool is_perfect_number(const uint64_t n) noexcept {
    // See https://en.wikipedia.org/wiki/List_of_Mersenne_primes_and_perfect_numbers
#if defined(MATH_FUNCTIONS_HPP_ENABLE_TARGET_OPTIONS)
    const auto [q, pm1] = extract_pow2(n);
    const auto p = pm1 + 1;
    if (q != (uint64_t{1} << p) - 1) {
        return false;
    }
    switch (p) {
        case 2:
        case 3:
        case 5:
        case 7:
        case 13:
        case 19:
        case 31:
            switch (n) {
                case 6:
                case 28:
                case 496:
                case 8128:
                case 33550336:
                case 8589869056ULL:
                case 137438691328ULL:
                case 2305843008139952128ULL: {
                    break;
                }
                default: {
                    CONFIG_UNREACHABLE();
                    break;
                }
            }
            return true;
        default:
            switch (n) {
                case 6:
                case 28:
                case 496:
                case 8128:
                case 33550336:
                case 8589869056ULL:
                case 137438691328ULL:
                case 2305843008139952128ULL: {
                    CONFIG_UNREACHABLE();
                    break;
                }
                default: {
                    break;
                }
            }
            return false;
    }
#else
    switch (n) {
        case 6:
        case 28:
        case 496:
        case 8128:
        case 33550336:
        case 8589869056ULL:
        case 137438691328ULL:
        case 2305843008139952128ULL: {
            return true;
        }
        default: {
            return false;
        }
    }
#endif
}

#if defined(HAS_INT128_TYPEDEF)

[[nodiscard]]
I128_CONSTEXPR bool is_perfect_number(const uint128_t n) noexcept(detail::is_trivial_arithmetic_v<uint128_t>) {
    return n > std::numeric_limits<uint64_t>::max() ? n == (((uint128_t{1} << 61U) - 1) << (61U - 1U))
                                                    : math_functions::is_perfect_number(static_cast<uint64_t>(n));
}

#endif

namespace detail {

template <uint32_t M, class T>
ATTRIBUTE_CONST ATTRIBUTE_ALWAYS_INLINE [[nodiscard]]
constexpr T unrolled_pow([[maybe_unused]] const uint32_t n) noexcept(detail::is_trivial_arithmetic_v<T>) {
    if constexpr (M == 0) {
        return 1;
    } else if constexpr (M == 1) {
        return n;
    } else if constexpr (M == 2) {
        return n * n;
    } else {
        const T tmp = math_functions::detail::unrolled_pow<M / 2, T>(n);
        if constexpr (M % 2 == 0) {
            return tmp * tmp;
        } else {
            return tmp * tmp * n;
        }
    }
}

template <uint32_t N, uint32_t K, class T>
ATTRIBUTE_CONST ATTRIBUTE_ALWAYS_INLINE [[nodiscard]]
constexpr T unrolled_cnk() noexcept(detail::is_trivial_arithmetic_v<T>) {
    if constexpr (K > N) {
        return 0;
    } else {
        // C_n_k = C_n_{n - k}
        constexpr auto KD = std::min(N - K, K);
        if constexpr (KD == 0) {
            // C_n_0 = 1
            return 1;
        } else if constexpr (KD == 1) {
            // C_n_1 = n
            return N;
        } else if constexpr (KD == 2) {
            // C_n_2 = n * (n - 1) / 2
            return static_cast<T>(N) * static_cast<T>(N - 1) / 2;
        } else {
            // C_n_k = C_{n - 1}_k + C_{n - 1}_{k - 1}
            return math_functions::detail::unrolled_cnk<N - 1, KD, T>() +
                   math_functions::detail::unrolled_cnk<N - 1, KD - 1, T>();
        }
    }
}

template <uint32_t M, class T>
ATTRIBUTE_CONST [[nodiscard]]
constexpr T powers_sum(const uint32_t n) noexcept(detail::is_trivial_arithmetic_v<T>);

template <uint32_t M, uint32_t I, class T>
ATTRIBUTE_CONST ATTRIBUTE_ALWAYS_INLINE [[nodiscard]]
constexpr T helper(const uint32_t n) noexcept(detail::is_trivial_arithmetic_v<T>) {
    static_assert(I <= M + 1);
    const auto res =
        math_functions::detail::unrolled_cnk<M + 1, I, T>() * math_functions::detail::powers_sum<M + 1 - I, T>(n);
    if constexpr (I + 1 <= M + 1) {
        return res + math_functions::detail::helper<M, I + 1, T>(n);
    } else {
        return res;
    }
}

template <uint32_t M, class T>
constexpr T powers_sum(const uint32_t n) noexcept(detail::is_trivial_arithmetic_v<T>) {
    static_assert(sizeof(T) >= sizeof(uint64_t));
    static_assert(M + 1 > M);

    if constexpr (M == 0) {
        return n;
    } else if constexpr (M == 1) {
        // n * (n + 1) / 2
        const auto n_u64 = static_cast<uint64_t>(n);
        return static_cast<T>((n * (n_u64 + 1)) / 2);
    } else if constexpr (M == 2) {
        // n * (n + 1) * (2 * n + 1) / 6
        const auto n_u64 = static_cast<uint64_t>(n);
        const T tmp = static_cast<T>((n * (n_u64 + 1)) / 2);
        const auto tmp2 = 2 * n_u64 + 1;
        return (tmp2 % 3 == 0) ? (tmp * (tmp2 / 3)) : ((tmp / 3) * tmp2);
    } else if constexpr (M == 3) {
        // n * n * (n + 1) * (n + 1) / 4
        const auto tmp = static_cast<T>((n * (static_cast<uint64_t>(n) + 1)) / 2);
        return tmp * tmp;
    } else if constexpr (M == 4) {
        // n * (n + 1) * (6 * n^3 + 9 * n^2 + n - 1) / 30
        const auto n_u64 = static_cast<uint64_t>(n);
        const uint64_t n_square_u64 = n_u64 * n;
        const auto tmp = static_cast<T>((n_square_u64 + n) / 2);
        const uint64_t two_n_plus_3_u64 = 2 * n_u64 + 3;
        const uint32_t n_minus_1 = n - 1;
        switch (n % 5) {
            case 0:
            case 4: {
                const auto tmp2 = tmp / 5;
                const auto tmp3 = static_cast<T>(n_square_u64) * two_n_plus_3_u64;
                if (n_minus_1 % 3 == 0) {
                    return tmp2 * (tmp3 + (n_minus_1 / 3));
                }

                CONFIG_ASSUME_STATEMENT(tmp2 % 3 == 0);
                return tmp2 * tmp3 + ((tmp2 / 3) * n_minus_1);
            }
            case 1: {
                CONFIG_ASSUME_STATEMENT(two_n_plus_3_u64 % 5 == 0);
                const auto lhs = static_cast<T>(n_square_u64) * (two_n_plus_3_u64 / 5);
                const uint32_t n_minus_1_over_5 = n_minus_1 / 5;
                if (n_minus_1_over_5 % 3 == 0) {
                    return tmp * (lhs + (n_minus_1_over_5 / 3));
                }

                return tmp * lhs + ((tmp / 3) * n_minus_1_over_5);
            }
            default:
                break;
        }

        const auto tmp2 = static_cast<T>(n_square_u64) * two_n_plus_3_u64;
        if (n_minus_1 % 3 == 0) {
            return tmp * ((tmp2 + (n_minus_1 / 3)) / 5);
        }

        return (tmp / 3) * ((3 * tmp2 + n_minus_1) / 5);
    } else {
        return (math_functions::detail::unrolled_pow<M + 1, T>(n + 1) - 1 -
                math_functions::detail::helper<M, 2, T>(n)) /
               math_functions::detail::unrolled_cnk<M + 1, 1, T>();
    }
}

inline constexpr double kE =
#ifdef MATH_FUNCTIONS_HAS_NUMBERS
    std::numbers::e_v<double>;
#else
    // M_E constant might not be defined
    static_cast<double>(2.71828182845904523536L);
#endif

}  // namespace detail

/// @brief Return 1^M + 2^M + ... + n^M
/// @tparam M
/// @param n
/// @return
template <uint32_t M>
ATTRIBUTE_CONST [[nodiscard]]
constexpr uint64_t powers_sum_u64(const uint32_t n) noexcept {
    return math_functions::detail::powers_sum<M, uint64_t>(n);
}

#if defined(HAS_INT128_TYPEDEF)

/// @brief Return 1^M + 2^M + ... + n^M
/// @tparam M
/// @param n
/// @return
template <uint32_t M>
ATTRIBUTE_CONST [[nodiscard]]
constexpr uint128_t powers_sum_u128(const uint32_t n) noexcept(detail::is_trivial_arithmetic_v<uint128_t>) {
    return math_functions::detail::powers_sum<M, uint128_t>(n);
}

#endif

namespace detail {

template <class T>
ATTRIBUTE_CONST [[nodiscard]]
constexpr size_t arange_size(T begin, T end, T step) noexcept(detail::is_trivial_arithmetic_v<T>) {
    if (unlikely(step == 0)) {
        return 0;
    }

    if constexpr (math_functions::is_signed_v<T>) {
        if (step < 0) {
            step = -step;
            std::swap(begin, end);
        }
    }

    T approx_size = (end - begin + step - 1) / step;
    if constexpr (math_functions::is_signed_v<T>) {
        approx_size = std::max(approx_size, T{0});
    }

#if defined(HAS_INT128_TYPEDEF)
    if constexpr (std::is_same_v<T, int128_t> || std::is_same_v<T, uint128_t>) {
        constexpr auto kUsizeMax = std::numeric_limits<size_t>::max();
        return approx_size <= kUsizeMax ? static_cast<size_t>(approx_size) : kUsizeMax;
    } else
#endif
        if constexpr (math_functions::is_signed_v<T>) {
        return static_cast<size_t>(approx_size);
    } else {
        return size_t{approx_size};
    }
}

}  // namespace detail

template <class T>
#if CONFIG_HAS_CONCEPTS
    requires math_functions::integral<T>
#endif
[[nodiscard]] CONSTEXPR_VECTOR std::vector<T> arange(const T begin, const T end, const T step) {
    math_functions::detail::check_math_int_type<T>();

    const size_t size = math_functions::detail::arange_size(begin, end, step);
    std::vector<T> rng(size);
    T i = begin;
    for (T& elem : rng) {
        elem = i;
        i += step;
    }

    return rng;
}

template <class T>
[[nodiscard]] CONSTEXPR_VECTOR std::vector<T> arange(const T begin, const T end) {
    return math_functions::arange(begin, end, T{1});
}

template <class T>
[[nodiscard]] CONSTEXPR_VECTOR std::vector<T> arange(const T n) {
    return math_functions::arange(T{0}, n);
}

/// @brief Return vector of elements {log2(0), log2(1), log2(2), log2(3), ..., log2(n)}
/// @note  Here log2(0) := -1
/// @param n
/// @return
[[nodiscard]] CONSTEXPR_VECTOR std::vector<uint32_t> log2_arange(const size_t n) {
    std::vector<uint32_t> values(n + 1 != 0 ? n + 1 : n);
    values[0] = static_cast<uint32_t>(-1);
    for (size_t i = 1; i <= n; i++) {
        values[i] = values[i / 2] + 1;
    }

    return values;
}

/// @brief Return vector of elements {p^0, p^1, p^2, p^3, ..., p^n}
/// @param n
/// @return
template <class T>
[[nodiscard]]
CONSTEXPR_VECTOR std::vector<T> pow_arange(const size_t n, const T p) {
    static_assert(std::is_floating_point_v<T>, "floating point type is expected");

    std::vector<T> values(n + 1 != 0 ? n + 1 : n);
    assert(values.size() == n + 1);

    values[0] = T{1};
    if (likely(n >= 1)) {
        values[1] = p;
    }
    for (size_t i = 2; i <= n; i++) {
        values[i] = values[i / 2] * values[(i + 1) / 2];
    }

    return values;
}

/// @brief Return vector of elements {e^0, e^1, e^2, e^3, ..., e^n}
/// @param n
/// @return
template <class T>
[[nodiscard]]
CONSTEXPR_VECTOR std::vector<T> exp_arange(const size_t n) {
    return math_functions::pow_arange<T>(n, static_cast<T>(math_functions::detail::kE));
}

/// @brief Return vector of elements {p^0 mod m, p^1 mod m, p^2 mod m, p^3 mod m, ..., p^n mod m}
/// @param n
/// @return
[[nodiscard]]
CONSTEXPR_VECTOR std::vector<uint32_t> pow_mod_m_arange(const size_t n, const uint32_t p, const uint32_t m) {
    std::vector<uint32_t> values(n + 1 != 0 ? n + 1 : n);
    assert(values.size() == n + 1);
    uint32_t current_pow = m != 1 ? 1u : 0u;
    values[0] = current_pow;
    for (size_t i = 1; i <= n; i++) {
        current_pow = static_cast<uint32_t>((uint64_t{current_pow} * uint64_t{p}) % m);
        values[i] = current_pow;
    }

    return values;
}

/// @brief Return vector of elements {0! mod m, 1! mod m, 2! mod m, 3! mod m, ..., n! mod m}
/// @param n
/// @return
[[nodiscard]]
CONSTEXPR_VECTOR std::vector<uint32_t> factorial_mod_m_arange(const size_t n, const uint32_t m) {
    std::vector<uint32_t> values(n + 1 != 0 ? n + 1 : n);
    uint32_t current_factorial = m != 1 ? 1U : 0U;
    values[0] = current_factorial;
    for (size_t i = 1; i <= n; i++) {
        current_factorial = static_cast<uint32_t>((uint64_t{current_factorial} * uint64_t{i}) % m);
        values[i] = current_factorial;
    }

    return values;
}

namespace detail {

template <class T>
ATTRIBUTE_SIZED_ACCESS(read_only, 2, 3)
ATTRIBUTE_NONNULL_ALL_ARGS ATTRIBUTE_PURE [[nodiscard]]
constexpr size_t find_wmin_index(T weighted_sum,
                                 const T* const RESTRICT_QUALIFIER prefsums,
                                 const size_t prefsums_size) noexcept(detail::is_trivial_arithmetic_v<T>) {
    assert(prefsums_size > 0);

    T min_weighted_sum = weighted_sum;
    size_t min_weighted_sum_index = 0;

    const size_t n = prefsums_size - 1;
    const T max_prefsum = prefsums[n];
    for (size_t j = 1; j < n; ++j) {
        weighted_sum = weighted_sum + prefsums[j] - (max_prefsum - prefsums[j]);
        if (weighted_sum < min_weighted_sum) {
            min_weighted_sum = weighted_sum;
            min_weighted_sum_index = j;
        }
    }

    return min_weighted_sum_index;
}

// clang-format off

template <class Iterator, class Sentinel>
[[nodiscard]]
CONSTEXPR_VECTOR Iterator wmin_impl(const Iterator begin, const Sentinel end) {
    // clang-format on

    using IterTraits = std::iterator_traits<Iterator>;
    using IntType = typename IterTraits::value_type;
    using DiffType = typename IterTraits::difference_type;

    const DiffType n_signed = std::distance(begin, end);
    assert(n_signed >= 0);
    if (unlikely(n_signed <= 0)) {
        return begin;
    }

    const size_t n = static_cast<size_t>(n_signed);

    using ComputationalType = math_functions::detail::double_bits_t<IntType>;
    static_assert(
        math_functions::is_math_integral_type_v<IntType> && math_functions::is_math_integral_type_v<ComputationalType>,
        "Unsupported value type in the input sequence in the weighted_min "
        "(try using int32_t or uint32_t if your platform doesn't have int128_t)");

    std::vector<ComputationalType> prefsums(n + 1);
    size_t i = 0;
    ComputationalType weighted_sum = 0;
    for (Iterator iter = begin; iter != end; ++iter, ++i) {
        const IntType val = *iter;
        prefsums[i + 1] = prefsums[i] + ComputationalType{val};
        weighted_sum += static_cast<ComputationalType>(i) * ComputationalType{val};
    }

    const size_t index = math_functions::detail::find_wmin_index(weighted_sum, std::as_const(prefsums).data(), n + 1);
    Iterator iter = begin;
    std::advance(iter, static_cast<DiffType>(index));
    return iter;
}

}  // namespace detail

#if CONFIG_HAS_CONCEPTS && defined(MATH_FUNCTIONS_HAS_RANGES)

template <math_functions::integral_forward_iterator Iterator, std::sentinel_for<Iterator> Sentinel>
[[nodiscard]] CONSTEXPR_VECTOR Iterator weighted_min(Iterator begin, Sentinel end) {
    return math_functions::detail::wmin_impl(std::move(begin), std::move(end));
}

// clang-format off

/// @brief Let @a range be a sequence S of elements S_0, S_1, ..., S_m
///        Then this function returns iterator to the element S_j such that
///        \f[
///           j = \argmin_{0 <= j <= m} ( \sum_{i = 0}^{m} (i - j) * S_i )
///        \f]
/// @tparam Range
/// @param range
/// @return
template <std::ranges::forward_range Range>
    requires std::ranges::borrowed_range<Range>
[[nodiscard]]
// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
CONSTEXPR_VECTOR std::ranges::borrowed_iterator_t<Range> weighted_min(Range&& range ATTRIBUTE_LIFETIME_BOUND) {
    return math_functions::weighted_min(std::ranges::begin(range), std::ranges::end(range));
}

// clang-format on

#else

template <class Iterator,
          std::enable_if_t<math_functions::is_math_integral_type_v<typename std::iterator_traits<Iterator>::value_type>,
                           int> = 0>
[[nodiscard]] CONSTEXPR_VECTOR Iterator weighted_min(Iterator begin, Iterator end) {
    return math_functions::detail::wmin_impl(std::move(begin), std::move(end));
}

template <class Range>
// NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
[[nodiscard]] CONSTEXPR_VECTOR auto weighted_min(Range&& range ATTRIBUTE_LIFETIME_BOUND) {
    return math_functions::weighted_min(std::cbegin(range), std::cend(range));
}

#endif

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

#ifdef HAS_INT128_TYPEDEF

namespace detail {

ATTRIBUTE_CONST
[[nodiscard]]
I128_CONSTEXPR uint128_t gcd(uint128_t a, uint128_t b) noexcept(detail::is_trivial_arithmetic_v<uint128_t>) {
    if (unlikely(a == 0)) {
        return b;
    }
    if (unlikely(b == 0)) {
        return a;
    }

    const uint32_t ra = static_cast<uint32_t>(math_functions::countr_zero(a));
    const uint32_t rb = static_cast<uint32_t>(math_functions::countr_zero(b));
    const uint32_t mult = std::min(ra, rb);
    a >>= ra;
    b >>= rb;
    while (true) {
        if (a < b) {
            // std::swap is not constexpr in C++17
            const uint128_t tmp = a;
            a = b;
            b = tmp;
        }

        a -= b;
        if (a == 0) {
            return b << mult;
        }

        a >>= static_cast<uint32_t>(math_functions::countr_zero(a));
    }
}

ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_CONST
[[nodiscard]]
I128_CONSTEXPR uint128_t gcd(const uint128_t a, const int128_t b) noexcept(detail::is_trivial_arithmetic_v<uint128_t>) {
    return math_functions::detail::gcd(a, math_functions::uabs(b));
}

ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_CONST
[[nodiscard]]
I128_CONSTEXPR uint128_t gcd(const int128_t a, const uint128_t b) noexcept(detail::is_trivial_arithmetic_v<uint128_t>) {
    return math_functions::detail::gcd(math_functions::uabs(a), b);
}

ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_CONST
[[nodiscard]]
I128_CONSTEXPR int128_t gcd(const int128_t a, const int128_t b) noexcept(detail::is_trivial_arithmetic_v<uint128_t>) {
    const int128_t value = static_cast<int128_t>(math_functions::detail::gcd(math_functions::uabs(a), b));
    CONFIG_ASSUME_STATEMENT(value >= 0);
    return value;
}

ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_CONST
[[nodiscard]]
I128_CONSTEXPR uint128_t gcd(const uint128_t a, const uint64_t b) noexcept(detail::is_trivial_arithmetic_v<uint128_t>) {
    if ((config::is_constant_evaluated() && a <= std::numeric_limits<uint64_t>::max()) ||
        (config::is_gcc_constant_p(a <= std::numeric_limits<uint64_t>::max()) &&
         a <= std::numeric_limits<uint64_t>::max()) ||
        (config::is_gcc_constant_p(a <= b) && a <= b)) {
        return std::gcd(static_cast<uint64_t>(a), b);
    }

    if (unlikely(b == 0)) {
        return a;
    }
    // gcd(a, b) = gcd(b, a % b) = gcd(a % b, b)
    return std::gcd(static_cast<uint64_t>(a % b), b);
}

ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_CONST
[[nodiscard]]
I128_CONSTEXPR uint128_t gcd(const uint64_t a, const uint128_t b) noexcept(detail::is_trivial_arithmetic_v<uint128_t>) {
    return math_functions::detail::gcd(b, a);
}

ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_CONST
[[nodiscard]]
I128_CONSTEXPR uint128_t gcd(const uint128_t a, const int64_t b) noexcept(detail::is_trivial_arithmetic_v<uint128_t>) {
    return math_functions::detail::gcd(a, math_functions::uabs(b));
}

ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_CONST
[[nodiscard]]
I128_CONSTEXPR uint128_t gcd(const int64_t a, const uint128_t b) noexcept(detail::is_trivial_arithmetic_v<uint128_t>) {
    return math_functions::detail::gcd(b, a);
}

ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_CONST
[[nodiscard]]
I128_CONSTEXPR int128_t gcd(const uint64_t a, const int128_t b) noexcept(detail::is_trivial_arithmetic_v<uint128_t>) {
    const int128_t value = static_cast<int128_t>(math_functions::detail::gcd(a, math_functions::uabs(b)));
    CONFIG_ASSUME_STATEMENT(value >= 0);
    CONFIG_ASSUME_STATEMENT(a == 0 || value <= a);
    return value;
}

ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_CONST
[[nodiscard]]
I128_CONSTEXPR int128_t gcd(const int128_t a, const uint64_t b) noexcept(detail::is_trivial_arithmetic_v<uint128_t>) {
    return math_functions::detail::gcd(b, a);
}

ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_CONST
[[nodiscard]]
I128_CONSTEXPR int128_t gcd(const int128_t a, const int64_t b) noexcept(detail::is_trivial_arithmetic_v<uint128_t>) {
    return math_functions::detail::gcd(a, math_functions::uabs(b));
}

ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_CONST
[[nodiscard]]
I128_CONSTEXPR int128_t gcd(const int64_t a, const int128_t b) noexcept(detail::is_trivial_arithmetic_v<uint128_t>) {
    return math_functions::detail::gcd(b, a);
}

}  // namespace detail

#endif  // HAS_INT128_TYPEDEF

/// @brief Computes greaters common divisor of @a `a` and @a `b`
///         using Stein's algorithm (binary gcd). Here gcd(0, 0) = 0.
/// @param[in] a
/// @param[in] b
/// @return gcd(a, b)
template <class M, class N>
ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_CONST [[nodiscard]]
constexpr std::common_type_t<M, N> gcd(const M m, const N n) noexcept(detail::is_trivial_arithmetic_v<M> &&
                                                                      detail::is_trivial_arithmetic_v<N>) {
    static_assert(math_functions::is_integral_v<M> && math_functions::is_integral_v<N>,
                  "math_functions::gcd arguments must be integers");

#if defined(HAS_INT128_TYPEDEF)
    if constexpr (sizeof(M) <= sizeof(uint64_t) && sizeof(N) <= sizeof(uint64_t)) {
#endif
        return std::gcd(m, n);
#if defined(HAS_INT128_TYPEDEF)
    } else {
        return math_functions::detail::gcd(m, n);
    }
#endif
}

}  // namespace math_functions

#undef CONSTEXPR_VECTOR

#ifdef MATH_FUNCTIONS_HAS_NUMBERS
#undef MATH_FUNCTIONS_HAS_NUMBERS
#endif
#ifdef MATH_FUNCTIONS_HAS_RANGES
#undef MATH_FUNCTIONS_HAS_RANGES
#endif
#ifdef MATH_FUNCTIONS_HAS_BIT
#undef MATH_FUNCTIONS_HAS_BIT
#endif

#ifdef MATH_FUNCTIONS_HPP_ENABLE_TARGET_OPTIONS
#if defined(__GNUG__)
#if !defined(__clang__)
#pragma GCC pop_options
#else
#pragma clang attribute pop
#endif
#endif
#undef MATH_FUNCTIONS_HPP_ENABLE_TARGET_OPTIONS
#endif

#if CONFIG_COMPILER_IS_MSVC
#pragma warning(pop)
#endif  // _MSC_VER

#endif  // !MATH_FUNCTIONS_HPP
