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

#include "config_macros.hpp"

#if CONFIG_HAS_AT_LEAST_CXX_20
#include <bit>
#include <ranges>
#endif

#if CONFIG_HAS_INCLUDE("integers_128_bit.hpp")
#include "integers_128_bit.hpp"
#endif

// Visual C++ thinks that unary minus on unsigned is an error
#if defined(_MSC_VER)
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
using std::size_t;
using std::uint32_t;
using std::uint64_t;

#if CONFIG_HAS_CONCEPTS
template <class T>
concept InplaceMultipliable = requires(T a, const T b) {
    { a *= b };
};
#endif

/// @brief Calculates T ^ p
/// @tparam T
/// @param[in] n
/// @param[in] p
/// @return T ^ p
#if CONFIG_HAS_CONCEPTS
template <InplaceMultipliable T>
#else
template <class T>
#endif
[[nodiscard]] ATTRIBUTE_CONST constexpr T bin_pow(T n, size_t p) noexcept(noexcept(n *= n)) {
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

/// @brief Calculates T ^ p
/// @tparam T
/// @param[in] n
/// @param[in] p
/// @return T ^ p
#if CONFIG_HAS_CONCEPTS
template <InplaceMultipliable T>
#else
template <class T>
#endif
[[nodiscard]] ATTRIBUTE_CONST constexpr T bin_pow(T n,
                                                  std::ptrdiff_t p) noexcept(noexcept(n *= n) &&
                                                                             noexcept(1 / n)) {
    const bool not_inverse = p >= 0;
    const size_t p_u       = p >= 0 ? static_cast<size_t>(p) : -static_cast<size_t>(p);
    const T res            = ::math_functions::bin_pow(std::move(n), p_u);
    return not_inverse ? res : 1 / res;
}

/// @brief Calculate (n ^ p) % mod
/// @param[in] n
/// @param[in] p
/// @param[in] mod
/// @return (n ^ p) % mod
[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t bin_pow_mod(uint32_t n, uint64_t p,
                                                             uint32_t mod) noexcept {
    uint64_t res     = mod != 1;
    uint64_t widen_n = n;
    while (true) {
        if (p % 2 != 0) {
            CONFIG_ASSUME_STATEMENT(widen_n < (uint64_t{1} << 32U));
            res = (res * widen_n) % mod;
        }
        p /= 2;
        if (p == 0) {
            return static_cast<uint32_t>(res);
        }
        CONFIG_ASSUME_STATEMENT(widen_n < (uint64_t{1} << 32U));
        widen_n = (widen_n * widen_n) % mod;
        CONFIG_ASSUME_STATEMENT(widen_n < (uint64_t{1} << 32U));
    }
}

#if defined(INTEGERS_128_BIT_HPP)

/// @brief Calculate (n ^ p) % mod
/// @param[in] n
/// @param[in] p
/// @param[in] mod
/// @return (n ^ p) % mod
[[nodiscard]]
ATTRIBUTE_CONST I128_CONSTEXPR uint64_t bin_pow_mod(uint64_t n, uint64_t p, uint64_t mod) noexcept {
    uint64_t res = mod != 1U;
    while (true) {
        if (p % 2 != 0) {
            res = static_cast<uint64_t>((uint128_t{res} * n) % mod);
        }
        p /= 2;
        if (p == 0) {
            return res;
        }
        n = static_cast<uint64_t>((uint128_t{n} * n) % mod);
    }
}

#endif

[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t isqrt(uint32_t n) noexcept {
    /**
     * In the runtime `sqrt` is used (but not for the msvc prior to the c++20).
     *
     * Quick Bench benchmark source code on the godbolt:
     *  https://godbolt.org/z/7jK8xcjjf
     */

    uint32_t y = 0;
#if defined(__GNUG__) || defined(__clang__) || CONFIG_HAS_AT_LEAST_CXX_20
    if (::config::is_constant_evaluated() || ::config::is_gcc_constant_p(n)) {
#endif
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
#if defined(__GNUG__) || defined(__clang__) || CONFIG_HAS_AT_LEAST_CXX_20
    } else {
        y = static_cast<uint32_t>(std::sqrt(static_cast<double>(n)));
    }
#endif

    CONFIG_ASSUME_STATEMENT(y < (1U << 16U));
    return y;
}

[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t isqrt(uint64_t n) noexcept {
    /**
     * In the runtime `sqrtl` is used (but not for the msvc prior to the c++20).
     */
#if defined(__GNUG__) || defined(__clang__) || CONFIG_HAS_AT_LEAST_CXX_20
    if (::config::is_constant_evaluated() || ::config::is_gcc_constant_p(n) ||
        sizeof(long double) < 16) {
#endif
        /**
         * See Hackers Delight Chapter 11.
         */
        uint64_t l = 1;
        uint64_t r = std::min((n >> 5U) + 8U, uint64_t{std::numeric_limits<uint32_t>::max()});
        do {
            CONFIG_ASSUME_STATEMENT(l <= r);
            CONFIG_ASSUME_STATEMENT((r >> 32U) == 0);
            const uint64_t m = (l + r) / 2;
            CONFIG_ASSUME_STATEMENT((m >> 32U) == 0);
            if (n >= m * m) {
                l = m + 1;
            } else {
                r = m - 1;
            }
        } while (r >= l);
        CONFIG_ASSUME_STATEMENT(((l - 1) >> 32U) == 0);
        return static_cast<uint32_t>(l - 1);
#if defined(__GNUG__) || defined(__clang__) || CONFIG_HAS_AT_LEAST_CXX_20
    }
    return static_cast<uint32_t>(std::sqrt(static_cast<long double>(n)));

#endif
}

#if defined(INTEGERS_128_BIT_HPP)

[[nodiscard]] ATTRIBUTE_CONST I128_CONSTEXPR uint64_t isqrt(uint128_t n) noexcept {
    /**
     * See Hackers Delight Chapter 11.
     */
    uint64_t l               = 0;
    const uint128_t r_approx = (n >> 6U) + 16U;
    uint64_t r               = r_approx > std::numeric_limits<uint64_t>::max()
                                   ? std::numeric_limits<uint64_t>::max()
                                   : static_cast<uint64_t>(r_approx);
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

/// @brief Return integer part of the cube root of n, i.e. ⌊n^(1/3)⌋
/// @note  See Hackers Delight Chapter 11, section 11-2.
/// @param[in] n
/// @return ⌊n^(1/3)⌋
[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t icbrt(uint32_t n) noexcept {
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

#if defined(__GNUG__) && !defined(__clang__)
    [[maybe_unused]] const auto n_original_value = n;
#endif

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
    // 1625^3 = 4291015625 < 2^32 - 1 = 4294967295 < 4298942376 = 1626^3
    CONFIG_ASSUME_STATEMENT(y <= 1625U);
#if defined(__GNUG__) && !defined(__clang__) && CONFIG_HAS_AT_LEAST_CXX_17
    // Clang ignores this assumption because it contains potential side effects (fpu register
    // flags), while GCC has made almost all math functions constexpr long before the C++26
    CONFIG_ASSUME_STATEMENT(
        y == (static_cast<uint32_t>(std::cbrt(static_cast<long double>(n_original_value)))));
#endif
    return y;
}

/// @brief Return integer part of the cube root of n, i.e. ⌊n^(1/3)⌋
/// @note  See Hackers Delight Chapter Chapter 11, ex. 2.
/// @param[in] n
/// @return ⌊n^(1/3)⌋
[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t icbrt(uint64_t n) noexcept {
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

/// @brief Return integer part of the fourth root of n, i.e. ⌊n^0.25⌋
/// @note ⌊n^0.25⌋ = ⌊⌊n^0.5⌋^0.5⌋ (see Hackers Delight Chapter 11, ex.1)
/// @param[in] n
/// @return ⌊n^0.25⌋
[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t ifrrt(uint64_t n) noexcept {
    return ::math_functions::isqrt(::math_functions::isqrt(n));
}

#if defined(INTEGERS_128_BIT_HPP)

/// @brief Return integer part of the fourth root of n, that is ⌊n^0.25⌋
///         It can be shown that ⌊n^0.25⌋ = ⌊⌊n^0.5⌋^0.5⌋
/// @param[in] n
/// @return
[[nodiscard]] ATTRIBUTE_CONST I128_CONSTEXPR uint32_t ifrrt(uint128_t n) noexcept {
    return ::math_functions::isqrt(::math_functions::isqrt(n));
}

#endif

template <class T>
struct IsPerfectSquareResult {
    T root{};
    bool is_perfect_square{};

    [[nodiscard]] constexpr explicit operator bool() const noexcept {
        return is_perfect_square;
    }
};

/// @brief Checks whether @a `n` is perfect square or not.
/// @param[in] n
/// @return {true, sqrt(n)} if @a `n` is perfect square and {false, 0} otherwise.
[[nodiscard]]
ATTRIBUTE_CONST constexpr IsPerfectSquareResult<uint32_t> is_perfect_square(uint32_t n) noexcept {
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
    switch (n % 16U) {
        case 0:
        case 1:
        case 4:
        case 9: {
            const uint32_t root       = ::math_functions::isqrt(n);
            const bool is_perf_square = root * root == n;
            return {is_perf_square ? root : 0, is_perf_square};
        }
        default:
            return {0, false};
    }
}

/// @brief Checks whether @a `n` is perfect square or not.
/// @param[in] n
/// @return {true, sqrt(n)} if @a `n` is perfect square and {false, 0} otherwise.
[[nodiscard]]
ATTRIBUTE_CONST constexpr IsPerfectSquareResult<uint32_t> is_perfect_square(uint64_t n) noexcept {
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
    switch (n % 16U) {
        case 0:
        case 1:
        case 4:
        case 9: {
            const uint32_t root       = ::math_functions::isqrt(n);
            const bool is_perf_square = uint64_t{root} * root == n;
            return {is_perf_square ? root : 0, is_perf_square};
        }
        default:
            return {0, false};
    }
}

#if defined(INTEGERS_128_BIT_HPP)

/// @brief Checks whether @a `n` is perfect square or not.
/// @param[in] n
/// @return {true, sqrt(n)} if @a `n` is perfect square and {false, 0} otherwise.
[[nodiscard]]
ATTRIBUTE_CONST I128_CONSTEXPR
    IsPerfectSquareResult<uint64_t> is_perfect_square(uint128_t n) noexcept {
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
    switch (static_cast<uint64_t>(n) % 16) {
        case 0:
        case 1:
        case 4:
        case 9: {
            const uint64_t root       = ::math_functions::isqrt(n);
            const bool is_perf_square = uint128_t{root} * root == n;
            return {is_perf_square ? root : 0, is_perf_square};
        }
        default:
            return {0, false};
    }
}

#endif

/// @brief This function reverses bits of the @a `b`
/// @param[in] b
/// @return 8-bit number whose bits are reversed bits of the @a `b`.
[[nodiscard]] ATTRIBUTE_CONST constexpr uint8_t bit_reverse(uint8_t b) noexcept {
    // See https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    return static_cast<uint8_t>(((b * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32U);
}

/// @brief This function reverses bits of the @a `n`
/// @param[in] b
/// @return 32-bit number whose bits are reversed bits of the @a `n`.
[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t bit_reverse(uint32_t n) noexcept {
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
[[nodiscard]] ATTRIBUTE_CONST constexpr uint64_t bit_reverse(uint64_t n) noexcept {
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

#if defined(INTEGERS_128_BIT_HPP)

/// @brief This function reverses bits of the @a `n`
/// @param[in] b
/// @return 128-bit number whose bits are reversed bits of the @a `n`.
[[nodiscard]] ATTRIBUTE_CONST I128_CONSTEXPR uint128_t bit_reverse(uint128_t n) noexcept {
    uint128_t m = ~uint128_t{0};
    for (uint32_t s = sizeof(uint128_t) * CHAR_BIT; s >>= 1U;) {
        m ^= m << s;
        n = ((n >> s) & m) | ((n << s) & ~m);
    }
    return n;
}

#endif

template <class Functor>
#if CONFIG_HAS_AT_LEAST_CXX_20
    requires requires(Functor f, uint64_t mask) { f(uint64_t{mask}); }
#endif
ATTRIBUTE_ALWAYS_INLINE constexpr void visit_all_submasks(uint64_t mask, Functor visiter) noexcept(
    std::is_nothrow_invocable_v<Functor, const uint64_t>) {
    uint64_t s = mask;
    do {
        visiter(uint64_t{s});
        s = (s - 1) & mask;
    } while (s != 0);
}

// NOLINTBEGIN(google-runtime-int)

int32_t sign(bool x) = delete;
int32_t sign(char x) = delete;

[[nodiscard]] ATTRIBUTE_CONST constexpr int32_t sign(signed char x) noexcept {
    return static_cast<int32_t>(x > 0) - static_cast<int32_t>(x < 0);
}

[[nodiscard]] ATTRIBUTE_CONST constexpr int32_t sign(unsigned char x) noexcept {
    return x > 0 ? 1 : 0;
}

[[nodiscard]] ATTRIBUTE_CONST constexpr int32_t sign(short x) noexcept {
    return static_cast<int32_t>(x > 0) - static_cast<int32_t>(x < 0);
}

[[nodiscard]] ATTRIBUTE_CONST constexpr int32_t sign(unsigned short x) noexcept {
    return x > 0 ? 1 : 0;
}

[[nodiscard]] ATTRIBUTE_CONST constexpr int32_t sign(int x) noexcept {
    return static_cast<int32_t>(x > 0) - static_cast<int32_t>(x < 0);
}

[[nodiscard]] ATTRIBUTE_CONST constexpr int32_t sign(unsigned x) noexcept {
    return x > 0 ? 1 : 0;
}

[[nodiscard]] ATTRIBUTE_CONST constexpr int32_t sign(long x) noexcept {
    return static_cast<int32_t>(x > 0) - static_cast<int32_t>(x < 0);
}

[[nodiscard]] ATTRIBUTE_CONST constexpr int32_t sign(unsigned long x) noexcept {
    return x > 0 ? 1 : 0;
}

[[nodiscard]] ATTRIBUTE_CONST constexpr int32_t sign(long long x) noexcept {
    return static_cast<int32_t>(x > 0) - static_cast<int32_t>(x < 0);
}

[[nodiscard]] ATTRIBUTE_CONST constexpr int32_t sign(unsigned long long x) noexcept {
    return x > 0 ? 1 : 0;
}

#if defined(INTEGERS_128_BIT_HPP)

[[nodiscard]] ATTRIBUTE_CONST I128_CONSTEXPR int32_t sign(int128_t x) noexcept {
    const uint32_t sign_bit = static_cast<uint32_t>(static_cast<uint128_t>(x) >> 127U);
    return static_cast<int32_t>(x != 0) - static_cast<int32_t>(2 * sign_bit);
}

[[nodiscard]] ATTRIBUTE_CONST constexpr int32_t sign(uint128_t x) noexcept {
    return x > 0 ? 1 : 0;
}

#endif

/// @brief a >= 0 and b > 0 => true
///        a >= 0 and b = 0 => true
///        a >= 0 and b < 0 => false
///        a < 0 and b > 0 => false
///        a < 0 and b = 0 => false
///        a < 0 and b < 0 => true
/// @param[in] a
/// @param[in] b
/// @return
[[nodiscard]] ATTRIBUTE_CONST constexpr bool same_sign_soft(int a, int b) noexcept {
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return (a ^ b) >= 0;
}

/// @brief a >= 0 and b > 0 => true
///        a >= 0 and b = 0 => true
///        a >= 0 and b < 0 => false
///        a < 0 and b > 0 => false
///        a < 0 and b = 0 => false
///        a < 0 and b < 0 => true
/// @param[in] a
/// @param[in] b
/// @return
[[nodiscard]] ATTRIBUTE_CONST constexpr bool same_sign_soft(long a, long b) noexcept {
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return (a ^ b) >= 0;
}

/// @brief a >= 0 and b > 0 => true
///        a >= 0 and b = 0 => true
///        a >= 0 and b < 0 => false
///        a < 0 and b > 0 => false
///        a < 0 and b = 0 => false
///        a < 0 and b < 0 => true
/// @param[in] a
/// @param[in] b
/// @return
[[nodiscard]] ATTRIBUTE_CONST constexpr bool same_sign_soft(long long a, long long b) noexcept {
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return (a ^ b) >= 0;
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
[[nodiscard]] ATTRIBUTE_CONST constexpr bool same_sign(int a, int b) noexcept {
    return ::math_functions::sign(a) == ::math_functions::sign(b);
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
[[nodiscard]] ATTRIBUTE_CONST constexpr bool same_sign(long a, long b) noexcept {
    return ::math_functions::sign(a) == ::math_functions::sign(b);
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
[[nodiscard]] ATTRIBUTE_CONST constexpr bool same_sign(long long a, long long b) noexcept {
    return ::math_functions::sign(a) == ::math_functions::sign(b);
}

bool uabs(bool n)          = delete;
unsigned char uabs(char n) = delete;

[[nodiscard]] ATTRIBUTE_CONST constexpr unsigned char uabs(signed char n) noexcept {
    return n >= 0 ? static_cast<unsigned char>(n)
                  : static_cast<unsigned char>(-static_cast<unsigned char>(n));
}

[[nodiscard]] ATTRIBUTE_CONST constexpr unsigned char uabs(unsigned char n) noexcept {
    return n;
}

[[nodiscard]] ATTRIBUTE_CONST constexpr unsigned short uabs(short n) noexcept {
    return n >= 0 ? static_cast<unsigned short>(n)
                  : static_cast<unsigned short>(-static_cast<unsigned short>(n));
}

[[nodiscard]] ATTRIBUTE_CONST constexpr unsigned short uabs(unsigned short n) noexcept {
    return n;
}

[[nodiscard]] ATTRIBUTE_CONST constexpr unsigned uabs(int n) noexcept {
    return n >= 0 ? static_cast<unsigned>(n) : -static_cast<unsigned>(n);
}

[[nodiscard]] ATTRIBUTE_CONST constexpr unsigned uabs(unsigned n) noexcept {
    return n;
}

[[nodiscard]] ATTRIBUTE_CONST constexpr unsigned long uabs(long n) noexcept {
    return n >= 0 ? static_cast<unsigned long>(n) : -static_cast<unsigned long>(n);
}

[[nodiscard]] ATTRIBUTE_CONST constexpr unsigned long uabs(unsigned long n) noexcept {
    return n;
}

[[nodiscard]] ATTRIBUTE_CONST constexpr unsigned long long uabs(long long n) noexcept {
    return n >= 0 ? static_cast<unsigned long long>(n) : -static_cast<unsigned long long>(n);
}

[[nodiscard]] ATTRIBUTE_CONST constexpr unsigned long long uabs(unsigned long long n) noexcept {
    return n;
}

// NOLINTEND(google-runtime-int)

#if defined(INTEGERS_128_BIT_HPP)

[[nodiscard]] ATTRIBUTE_CONST I128_CONSTEXPR uint128_t uabs(int128_t n) noexcept {
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    const uint128_t t = static_cast<uint128_t>(n >> 127U);
    return (static_cast<uint128_t>(n) ^ t) - t;
}

[[nodiscard]] ATTRIBUTE_CONST I128_CONSTEXPR uint128_t uabs(uint128_t n) noexcept {
    return n;
}

#endif

namespace detail {

#if defined(INTEGERS_128_BIT_HPP)
namespace helper_ns = int128_traits;
#else
namespace helper_ns = std;
#endif

template <class T>
inline constexpr bool is_integral_v = ::math_functions::detail::helper_ns::is_integral_v<T>;

template <class T>
inline constexpr bool is_unsigned_v = ::math_functions::detail::helper_ns::is_unsigned_v<T>;

template <class T>
inline constexpr bool is_signed_v = ::math_functions::detail::helper_ns::is_signed_v<T>;

template <class T>
using make_unsigned_t = typename ::math_functions::detail::helper_ns::make_unsigned_t<T>;

#if CONFIG_HAS_CONCEPTS

template <class T>
concept integral = ::math_functions::detail::helper_ns::integral<T>;

template <class T>
concept signed_integral = ::math_functions::detail::helper_ns::signed_integral<T>;

template <class T>
concept unsigned_integral = ::math_functions::detail::helper_ns::unsigned_integral<T>;

#endif

[[nodiscard]] ATTRIBUTE_CONST inline uint32_t log2_floor_software(uint64_t n) {
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

[[nodiscard]] ATTRIBUTE_CONST inline uint32_t log2_ceil_software(uint64_t n) {
    return ::math_functions::detail::log2_floor_software(n) + ((n & (n - 1)) != 0);
}

/**
 *  Returns the integer (floor) log of the specified value, base 2.
 *  Note that by convention, input value 0 returns 0 since log(0) is
 * undefined.
 */
[[nodiscard]] ATTRIBUTE_CONST inline uint32_t de_bruijn_log2(uint32_t value) {
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

[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t lz_count_32_software(uint32_t n) noexcept {
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

[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t lz_count_64_software(uint64_t n) noexcept {
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

[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t tz_count_32_software(uint32_t n) noexcept {
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

[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t tz_count_64_software(uint64_t n) noexcept {
    uint32_t m = 0U;
    for (n = ~n & (n - 1); n != 0U; n >>= 1U) {
        m++;
    }
    CONFIG_ASSUME_STATEMENT(m <= 64U);
    return m;
}

[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t pop_count_32_software(uint32_t n) noexcept {
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

[[nodiscard]] ATTRIBUTE_CONST constexpr uint64_t pop_count_64_software(uint64_t n) noexcept {
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

[[nodiscard]] ATTRIBUTE_CONST constexpr int32_t pop_diff(uint32_t x, uint32_t y) noexcept {
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

[[nodiscard]] ATTRIBUTE_CONST constexpr int32_t pop_cmp(uint32_t x, uint32_t y) noexcept {
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
    requires ::math_functions::detail::unsigned_integral<T>
#endif
[[nodiscard]] ATTRIBUTE_CONST constexpr int32_t countr_zero(T n) noexcept {
    static_assert(::math_functions::detail::is_unsigned_v<T>, "Unsigned integral type expected");

    if (unlikely(n == 0)) {
        return sizeof(n) * CHAR_BIT;
    }

#if defined(INTEGERS_128_BIT_HPP)
    if constexpr (std::is_same_v<T, uint128_t>) {
        const uint64_t low = static_cast<uint64_t>(n);
        if (low != 0) {
#if CONFIG_HAS_AT_LEAST_CXX_20
            return std::countr_zero(low);
#elif defined(__GNUG__)
            return __builtin_ctzll(low);
#else
            return static_cast<int32_t>(::math_functions::detail::tz_count_64_software(low));
#endif
        }

        const uint64_t high = static_cast<uint64_t>(n >> 64U);
        CONFIG_ASSUME_STATEMENT(high != 0);
        return 64 + ::math_functions::countr_zero<uint64_t>(high);
    } else
#endif

#if CONFIG_HAS_AT_LEAST_CXX_20
    {
        return std::countr_zero(n);
    }
#else
    // NOLINTBEGIN(google-runtime-int)
    if constexpr (std::is_same_v<T, unsigned long long>) {
#if defined(__GNUG__)
        return __builtin_ctzll(n);
#else
        return static_cast<int32_t>(::math_functions::detail::tz_count_64_software(n));
#endif
    } else if constexpr (std::is_same_v<T, unsigned long>) {
#if defined(__GNUG__)
        return __builtin_ctzl(n);
#else
        return static_cast<int32_t>(
            ::math_functions::detail::tz_count_64_software(static_cast<unsigned long long>(n)));
#endif
    } else {
        static_assert(std::is_same_v<T, unsigned int> || std::is_same_v<T, unsigned short> ||
                          std::is_same_v<T, unsigned char>,
                      "Inappropriate integer type in countr_zero");
#if defined(__GNUG__)
        return __builtin_ctz(n);
#else
        return static_cast<int32_t>(::math_functions::detail::tz_count_32_software(n));
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
    requires ::math_functions::detail::unsigned_integral<T>
#endif
[[nodiscard]] ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_CONST constexpr int32_t countl_zero(T n) noexcept {
    static_assert(::math_functions::detail::is_unsigned_v<T>, "Unsigned integral type expected");

    if (unlikely(n == 0)) {
        return sizeof(n) * CHAR_BIT;
    }

#if defined(INTEGERS_128_BIT_HPP)
    if constexpr (std::is_same_v<T, uint128_t>) {
        const uint64_t high = static_cast<uint64_t>(n >> 64U);
        if (high != 0) {
            // Avoid recursive call to countl_zero<uint64_t>
#if CONFIG_HAS_AT_LEAST_CXX_20
            return std::countl_zero(high);
#elif defined(__GNUG__)
            return __builtin_clzll(high);
#else
            return static_cast<int32_t>(::math_functions::detail::lz_count_64_software(high));
#endif
        }

        const uint64_t low = static_cast<uint64_t>(n);
        CONFIG_ASSUME_STATEMENT(low != 0);
        return 64U + ::math_functions::countl_zero<uint64_t>(low);
    } else
#endif

#if CONFIG_HAS_AT_LEAST_CXX_20
    {
        return std::countl_zero(n);
    }
#else
    // NOLINTBEGIN(google-runtime-int)
    if constexpr (std::is_same_v<T, unsigned long long>) {
#if defined(__GNUG__)
        return __builtin_clzll(n);
#else
        return static_cast<int32_t>(::math_functions::detail::lz_count_64_software(n));
#endif
    } else if constexpr (std::is_same_v<T, unsigned long>) {
#if defined(__GNUG__)
        return __builtin_clzl(n);
#else
        return static_cast<int32_t>(::math_functions::detail::lz_count_64_software(n));
#endif
    } else {
        static_assert(std::is_same_v<T, unsigned int> || std::is_same_v<T, unsigned short> ||
                          std::is_same_v<T, unsigned char>,
                      "Inappropriate integer type in countl_zero");
#if defined(__GNUG__)
        return __builtin_clz(n);
#else
        return static_cast<int32_t>(::math_functions::detail::lz_count_32_software(n));
#endif
    }
    // NOLINTEND(google-runtime-int)
#endif
}

template <class T>
#if CONFIG_HAS_CONCEPTS
    requires ::math_functions::detail::unsigned_integral<T>
#endif
[[nodiscard]] ATTRIBUTE_CONST constexpr int32_t popcount(T n) noexcept {
    static_assert(::math_functions::detail::is_unsigned_v<T>, "Unsigned integral type expected");

#if defined(INTEGERS_128_BIT_HPP)
    if constexpr (std::is_same_v<T, uint128_t>) {
        // Reason: cppcheck can not deduce that n is uint128_t here
        // cppcheck-suppress [shiftTooManyBits]
        const uint64_t high = static_cast<uint64_t>(n >> 64U);
        const uint64_t low  = static_cast<uint64_t>(n);
        return ::math_functions::popcount<uint64_t>(high) +
               ::math_functions::popcount<uint64_t>(low);
    } else
#endif

#if CONFIG_HAS_AT_LEAST_CXX_20
    {
        return std::popcount(n);
    }
#else
    // NOLINTBEGIN(google-runtime-int)
    if constexpr (std::is_same_v<T, unsigned long long>) {
#if defined(__GNUG__)
        return __builtin_popcountll(n);
#else
        return static_cast<int32_t>(::math_functions::detail::pop_count_64_software(n));
#endif
    } else if constexpr (std::is_same_v<T, unsigned long>) {
#if defined(__GNUG__)
        return __builtin_popcountl(n);
#else
        return static_cast<int32_t>(::math_functions::detail::pop_count_64_software(n));
#endif
    } else {
        static_assert(std::is_same_v<T, unsigned int> || std::is_same_v<T, unsigned short> ||
                          std::is_same_v<T, unsigned char>,
                      "Inappropriate integer type in popcount");
#if defined(__GNUG__)
        return __builtin_popcount(n);
#else
        return static_cast<int32_t>(::math_functions::detail::pop_count_32_software(n));
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
[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t next_n_bits_permutation(uint32_t x) noexcept {
    // See
    // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2

    // t gets x's least significant 0 bits set to 1
    const uint32_t t = x | (x - 1);
    // Next set to 1 the most significant bit to change,
    // set to 0 the least significant ones, and add the necessary 1 bits.
    return (t + 1) |
           static_cast<uint32_t>(uint64_t{(~t & -~t) - 1} >>
                                 static_cast<uint32_t>(::math_functions::countr_zero(x) + 1));
}

// NOLINTBEGIN(google-runtime-int)

bool is_power_of_two(bool) = delete;
bool is_power_of_two(char) = delete;

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_power_of_two(signed char n) noexcept {
    // Cast to unsigned to avoid potential overflow (ub for signed char)
    const unsigned m = unsigned{static_cast<unsigned char>(n)};
    return (m & (m - 1)) == 0 && n > 0;
}

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_power_of_two(unsigned char n) noexcept {
    const unsigned n_int = unsigned{n};
    return (n_int & (n_int - 1)) == 0 && n_int != 0;
}

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_power_of_two(int n) noexcept {
    // Cast to unsigned to avoid potential overflow (ub for int)
    const unsigned m = static_cast<unsigned>(n);
    return (m & (m - 1)) == 0 && n > 0;
}

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_power_of_two(long n) noexcept {
    // Cast to unsigned to avoid potential overflow (ub for long)
    const unsigned long m = static_cast<unsigned long>(n);
    return (m & (m - 1)) == 0 && n > 0;
}

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_power_of_two(long long n) noexcept {
    // Cast to unsigned to avoid potential overflow (ub for long long)
    const unsigned long long m = static_cast<unsigned long long>(n);
    return (m & (m - 1)) == 0 && n > 0;
}

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_power_of_two(unsigned int n) noexcept {
    return (n & (n - 1)) == 0 && n != 0;
}

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_power_of_two(unsigned long n) noexcept {
    return (n & (n - 1)) == 0 && n != 0;
}

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_power_of_two(unsigned long long n) noexcept {
    return (n & (n - 1)) == 0 && n != 0;
}

// NOLINTEND(google-runtime-int)

#if defined(INTEGERS_128_BIT_HPP)

[[nodiscard]] ATTRIBUTE_CONST I128_CONSTEXPR bool is_power_of_two(int128_t n) noexcept {
    // Cast to unsigned to avoid potential overflow (ub for int128_t)
    const uint128_t m = static_cast<uint128_t>(n);
    return (m & (m - 1)) == 0 && n > 0;
}

[[nodiscard]] ATTRIBUTE_CONST I128_CONSTEXPR bool is_power_of_two(uint128_t n) noexcept {
    return (n & (n - 1)) == 0 && n != 0;
}

#endif

template <class UIntType>
[[nodiscard]]
ATTRIBUTE_CONST constexpr auto nearest_greater_equal_power_of_two(const UIntType n) noexcept {
    static_assert(
        ::math_functions::detail::is_unsigned_v<UIntType> && sizeof(UIntType) >= sizeof(unsigned),
        "unsigned integral type (at least unsigned int) is expected");

    using ShiftType       = int32_t;
    const ShiftType shift = ShiftType{sizeof(n) * CHAR_BIT} -
                            ShiftType{::math_functions::countl_zero(n | 1U)} -
                            ShiftType{(n & (n - 1)) == 0};
    using RetType =
        typename std::conditional_t<(sizeof(UIntType) > sizeof(uint32_t)), UIntType, uint64_t>;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return RetType{1} << shift;
}

template <class UIntType>
[[nodiscard]]
ATTRIBUTE_CONST constexpr auto nearest_greater_power_of_two(const UIntType n) noexcept {
    static_assert(
        ::math_functions::detail::is_unsigned_v<UIntType> && sizeof(UIntType) >= sizeof(unsigned),
        "unsigned integral type (at least unsigned int) is expected");

    using ShiftType = int32_t;
    const ShiftType shift =
        ShiftType{sizeof(n) * CHAR_BIT} - ShiftType{::math_functions::countl_zero(n)};
    using RetType =
        typename std::conditional_t<(sizeof(UIntType) > sizeof(uint32_t)), UIntType, uint64_t>;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return RetType{1} << shift;
}

/// @brief If @a n != 0, return number that is power of 2 and
///         whose only bit is the lowest bit set in the @a n
///        Otherwise, return 0
/// @tparam IntType
/// @param[in] n
/// @return
template <class IntType>
[[nodiscard]] ATTRIBUTE_CONST constexpr IntType least_bit_set(IntType n) noexcept {
    static_assert(::math_functions::detail::is_integral_v<IntType>, "integral type expected");

    using UIntType              = ::math_functions::detail::make_unsigned_t<IntType>;
    using UIntTypeAtLeastUInt32 = std::common_type_t<UIntType, uint32_t>;
    auto unsigned_n             = static_cast<UIntTypeAtLeastUInt32>(static_cast<UIntType>(n));
    return static_cast<IntType>(unsigned_n & -unsigned_n);
}

/// @brief Return sum: sum of popcount(i & k) for all i from 0 to n inclusive
///  Using tex notation: \sum_{i = 0}^{n} popcount(i bitand k)
/// @tparam IntType
/// @param n
/// @param k
/// @return
template <class IntType>
#if CONFIG_HAS_CONCEPTS
    requires ::math_functions::detail::is_unsigned_v<IntType> &&
             (sizeof(IntType) >= sizeof(unsigned))
#endif
[[nodiscard]] ATTRIBUTE_CONST constexpr IntType masked_popcount_sum(IntType n, IntType k) noexcept {
    static_assert(
        ::math_functions::detail::is_unsigned_v<IntType> && sizeof(IntType) >= sizeof(unsigned),
        "unsigned integral type (at least unsigned int) is expected");

    IntType popcount_sum = 0;
    for (uint32_t j = 0; j < sizeof(IntType) * CHAR_BIT; j++) {
        const bool k_has_jth_bit = (k & (IntType{1} << j)) != 0;
        if (!k_has_jth_bit) {
            continue;
        }

        if ((n >> j) == 0) {
            break;
        }

        const bool n_has_jth_bit                           = (n & (IntType{1} << j)) != 0;
        const IntType number_of_full_blocks_with_j_bit_set = ((n >> j) / 2);
        const IntType nums_from_last_possibly_nonfull_block =
            n_has_jth_bit ? n - (((n >> j) << j) - 1) : IntType{0};
        const IntType number_of_nums_with_j_bit_set_from_blocks =
            number_of_full_blocks_with_j_bit_set << j;
        popcount_sum +=
            number_of_nums_with_j_bit_set_from_blocks + nums_from_last_possibly_nonfull_block;
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
[[nodiscard]] ATTRIBUTE_CONST constexpr IntType popcount_sum(IntType n) noexcept {
    return ::math_functions::masked_popcount_sum(n, ~IntType{0});
}

namespace detail {

/// @brief Realization taken from the gcc libstdc++ __to_chars_len
/// @tparam T
/// @param[in] value
/// @param[in] base
/// @return
template <class T>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_CONST constexpr uint32_t base_b_len_impl(
    T value, const uint8_t base = 10) noexcept {
    const uint32_t b  = base;
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
template <typename T>
[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t base_b_len(T value,
                                                            const uint8_t base = 10) noexcept {
    static_assert(::math_functions::detail::is_integral_v<T>);

    if constexpr (::math_functions::detail::is_signed_v<T>) {
        const uint32_t is_negative = uint32_t{value < 0};
        return is_negative +
               ::math_functions::detail::base_b_len_impl(::math_functions::uabs(value), base);
    } else {
        return ::math_functions::detail::base_b_len_impl(value, base);
    }
}

/// @brief For n > 0 returns ⌈log_2(n)⌉. For n = 0 returns (uint32_t)-1
/// @tparam UIntType unsigned integral type (at least unsigned int in size)
/// @param[in] n
/// @return
template <class UIntType>
#if CONFIG_HAS_CONCEPTS
    requires ::math_functions::detail::unsigned_integral<UIntType>
#endif
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_CONST constexpr uint32_t log2_floor(const UIntType n) noexcept {
    static_assert(
        ::math_functions::detail::is_unsigned_v<UIntType> && sizeof(UIntType) >= sizeof(unsigned),
        "unsigned integral type (at least unsigned int) is expected");

#if defined(INTEGERS_128_BIT_HPP)
    if constexpr (std::is_same_v<UIntType, uint128_t>) {
        const auto hi = static_cast<uint64_t>(n >> 64U);
        return hi != 0 ? (127 - static_cast<uint32_t>(::math_functions::countl_zero(hi)))
                       : (::math_functions::log2_floor(static_cast<uint64_t>(n)));
    } else
#endif
    {
        return uint32_t{sizeof(n) * CHAR_BIT - 1} -
               static_cast<uint32_t>(::math_functions::countl_zero(n));
    }
}

/// @brief For n > 0 returns ⌈log_2(n)⌉. For n = 0 returns (uint32_t)-1
/// @tparam UIntType unsigned integral type (at least unsigned int in size)
/// @param[in] n
/// @return
template <class UIntType>
#if CONFIG_HAS_CONCEPTS
    requires ::math_functions::detail::unsigned_integral<UIntType>
#endif
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_CONST constexpr uint32_t log2_ceil(const UIntType n) noexcept {
    return ::math_functions::log2_floor(n) + ((n & (n - 1)) != 0);
}

template <class UIntType>
#if CONFIG_HAS_CONCEPTS
    requires ::math_functions::detail::unsigned_integral<UIntType>
#endif
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_CONST constexpr uint32_t base_2_len(const UIntType n) noexcept {
    // " | 1" operation does not affect answer for all
    //  numbers except n = 0. For n = 0 answer is 1.
    return ::math_functions::log2_floor(n | 1) + 1;
}

namespace detail {

ATTRIBUTE_CONST constexpr uint32_t log10_floor_compile_time_impl(uint32_t n) noexcept {
    constexpr std::array<uint8_t, 33> table1 = {
        10, 9, 9, 8, 8, 8, 7, 7, 7, 6, 6, 6, 6, 5, 5, 5, 4,
        4,  4, 3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0,
    };
    constexpr std::array<uint32_t, 11> table2 = {
        1U, 10U, 100U, 1000U, 10000U, 100000U, 1000000U, 10000000U, 100000000U, 1000000000U, 0U,
    };
    uint32_t digits = table1[static_cast<uint32_t>(::math_functions::countl_zero(n))];
    digits -= ((n - table2[digits]) >> 31U);
    return digits;
}

ATTRIBUTE_CONST inline uint32_t log10_floor_runtime_impl(uint32_t n) noexcept {
#if defined(__cpp_constexpr) && __cpp_constexpr >= 202211L && defined(__GNUG__)
    constexpr
#elif defined(__cpp_constinit) && __cpp_constinit >= 201907L
    constinit
#endif
        // clang-format off
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
        static const uint8_t table1[33] = {
            // clang-format on
            10, 9, 9, 8, 8, 8, 7, 7, 7, 6, 6, 6, 6, 5, 5, 5, 4,
            4,  4, 3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0,
        };
#if defined(__cpp_constexpr) && __cpp_constexpr >= 202211L && defined(__GNUG__)
    constexpr
#elif defined(__cpp_constinit) && __cpp_constinit >= 201907L
    constinit
#endif
        // clang-format off
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
        static const uint32_t table2[11] = {
            // clang-format on
            1U, 10U, 100U, 1000U, 10000U, 100000U, 1000000U, 10000000U, 100000000U, 1000000000U, 0U,
        };

    uint32_t digits = table1[::math_functions::countl_zero(n)];
    digits -= ((n - table2[digits]) >> 31U);
    return digits;
}

}  // namespace detail

/// @brief For n > 0 returns ⌊log_10(n)⌋. For n = 0 returns (uint32_t)-1
/// @param[in] n
/// @return
[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t log10_floor(uint32_t n) noexcept {
    /**
     * See Hackers Delight 11-4
     */

#if CONFIG_HAS_AT_LEAST_CXX_20 || \
    (CONFIG_HAS_AT_LEAST_CXX_17 && (defined(__clang__) || CONFIG_GNUC_AT_LEAST(10, 0)))
    if (::config::is_constant_evaluated() || ::config::is_gcc_constant_p(n)) {
#endif
        return ::math_functions::detail::log10_floor_compile_time_impl(n);
#if CONFIG_HAS_AT_LEAST_CXX_20 || \
    (CONFIG_HAS_AT_LEAST_CXX_17 && (defined(__clang__) || CONFIG_GNUC_AT_LEAST(10, 0)))
    }
    return ::math_functions::detail::log10_floor_runtime_impl(n);

#endif
}

namespace detail {

ATTRIBUTE_CONST constexpr uint32_t log10_floor_compile_time_impl(uint64_t n,
                                                                 int32_t approx_log10) noexcept {
    constexpr std::array<uint64_t, 20> table2 = {
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
    const int32_t adjustment =
        static_cast<int32_t>((table2[static_cast<uint32_t>(approx_log10 + 1)] - n) >> 63U);
    return static_cast<uint32_t>(approx_log10 + adjustment);
}

ATTRIBUTE_CONST static inline uint32_t log10_floor_runtime_impl(uint64_t n,
                                                                int32_t approx_log10) noexcept {
#if defined(__cpp_constexpr) && __cpp_constexpr >= 202211L && defined(__GNUG__)
    constexpr
#elif defined(__cpp_constinit) && __cpp_constinit >= 201907L
    constinit
#endif
        // clang-format off
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
        static const uint64_t table2[20] = {
            // clang-format on
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
    const int32_t adjustment =
        static_cast<int32_t>((table2[static_cast<uint32_t>(approx_log10 + 1)] - n) >> 63U);
    return static_cast<uint32_t>(approx_log10 + adjustment);
}

}  // namespace detail

/// @brief For @a `n` > 0 returns ⌊log_10(n)⌋. For @a `n` = 0 returns
/// (uint32_t)-1
/// @param[in] n
/// @return
[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t log10_floor(uint64_t n) noexcept {
    /**
     * See Hackers Delight 11-4
     */

    static_assert(::math_functions::countl_zero(uint64_t{0}) == 64, "countl_zero detail error");
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    const int32_t approx_log10 = (19 * (63 - ::math_functions::countl_zero(n))) >> 6U;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    CONFIG_ASSUME_STATEMENT((-19 >> 6U) <= approx_log10 && approx_log10 <= ((19 * 63) >> 6U));

#if CONFIG_HAS_AT_LEAST_CXX_20 || \
    (CONFIG_HAS_AT_LEAST_CXX_17 && (defined(__clang__) || CONFIG_GNUC_AT_LEAST(10, 0)))
    if (::config::is_constant_evaluated() || ::config::is_gcc_constant_p(n)) {
#endif
        return ::math_functions::detail::log10_floor_compile_time_impl(n, approx_log10);
#if CONFIG_HAS_AT_LEAST_CXX_20 || \
    (CONFIG_HAS_AT_LEAST_CXX_17 && (defined(__clang__) || CONFIG_GNUC_AT_LEAST(10, 0)))
    }

    return ::math_functions::detail::log10_floor_runtime_impl(n, approx_log10);
#endif
}

[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t base_10_len(uint32_t n) noexcept {
    // or `n` with 1 so that base_10_len(0) = 1
    return ::math_functions::log10_floor(n | 1U) + 1;
}

[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t base_10_len(uint64_t n) noexcept {
    // or `n` with 1 so that base_10_len(0) = 1
    return ::math_functions::log10_floor(n | 1U) + 1;
}

template <class T>
struct ExtractPow2Result {
    T odd_part;
    uint32_t power;
};

/// @brief Find q and r such n = q * (2 ^ r), q is odd if n != 0
/// @note  For n = 0 answer is { q = 0, r = sizeof(n) * CHAR_BIT }
/// @param[in] n
/// @return Pair of q and r
template <typename T>
#if CONFIG_HAS_CONCEPTS
    requires ::math_functions::detail::unsigned_integral<T>
#endif
[[nodiscard]] ATTRIBUTE_CONST constexpr ExtractPow2Result<T> extract_pow2(T n) noexcept {
    auto r = static_cast<uint32_t>(::math_functions::countr_zero(n));
    CONFIG_ASSUME_STATEMENT(r <= sizeof(n) * CHAR_BIT);
    return {n != 0 ? (n >> r) : 0, r};
}

/// @brief Returns median of boolean variables x, y and z
/// @param x x
/// @param y y
/// @param z z
/// @return median of x, y and z
[[nodiscard]] ATTRIBUTE_CONST constexpr bool bool_median(bool x, bool y, bool z) noexcept {
    return (x || y) && (y || z) && (x || z);
}

#if CONFIG_HAS_CONCEPTS

template <::math_functions::detail::unsigned_integral T>
[[nodiscard]] ATTRIBUTE_CONST constexpr T next_even(T n) noexcept {
    return n + 2 - n % 2;
}

#else

// clang-format off

template <class T>
[[nodiscard]]
ATTRIBUTE_CONST
constexpr
std::enable_if_t<::math_functions::detail::is_unsigned_v<T>, T> next_even(T n) noexcept {
    return n + 2 - n % 2;
}

// clang-format on

#endif

template <class FloatType>
struct SumSinCos {
    FloatType sines_sum;
    FloatType cosines_sum;
};

/// @brief Function returns pair of 2 sums:
///         (
///           sin(alpha) +
///           + sin(alpha + beta) +
///           + sin(alpha + 2 beta) +
///           + sin(alpha + 3 beta) +
///           + ... + sin(alpha + (n - 1) beta)
///         ),
///         (
///           cos(alpha) +
///           + cos(alpha + beta) +
///           + cos(alpha + 2 beta) +
///           + cos(alpha + 3 beta) +
///           + ... + cos(alpha + (n - 1) beta)
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
[[nodiscard]] ATTRIBUTE_CONST SumSinCos<FloatType> sum_of_sines_and_cosines(FloatType alpha,
                                                                            FloatType beta,
                                                                            uint32_t n) noexcept {
    static_assert(std::is_floating_point_v<FloatType>, "Invalid type in sum_of_sines_and_cosines");

    const FloatType nf       = static_cast<FloatType>(n);
    const auto half_beta     = beta / 2;
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
    const auto arg                      = alpha + (nf - 1) * half_beta;
    const auto sin_mult                 = std::sin(arg);
    const auto cos_mult                 = std::cos(arg);
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
[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t max_number_of_unique_prime_divisors(
    uint32_t n) noexcept {
    constexpr uint32_t kBoundary2 = 2 * 3;
    constexpr uint32_t kBoundary3 = 2 * 3 * 5;
    constexpr uint32_t kBoundary4 = 2 * 3 * 5 * 7;
    constexpr uint32_t kBoundary5 = 2 * 3 * 5 * 7 * 11;
    constexpr uint32_t kBoundary6 = 2 * 3 * 5 * 7 * 11 * 13;
    constexpr uint32_t kBoundary7 = 2 * 3 * 5 * 7 * 11 * 13 * 17;
    constexpr uint32_t kBoundary8 = 2 * 3 * 5 * 7 * 11 * 13 * 17 * 19;
    constexpr uint32_t kBoundary9 = 2 * 3 * 5 * 7 * 11 * 13 * 17 * 19 * 23;

#if defined(__GNUG__) || defined(__clang__)
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-case-range"
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
    switch (n) {
        case 0 ... kBoundary2 - 1:
            return 1;
        case kBoundary2 ... kBoundary3 - 1:
            return 2;
        case kBoundary3 ... kBoundary4 - 1:
            return 3;
        case kBoundary4 ... kBoundary5 - 1:
            return 4;
        case kBoundary5 ... kBoundary6 - 1:
            return 5;
        case kBoundary6 ... kBoundary7 - 1:
            return 6;
        case kBoundary7 ... kBoundary8 - 1:
            return 7;
        case kBoundary8 ... kBoundary9 - 1:
            return 8;
        default:
            return 9;
    }
#if defined(__clang__)
#pragma clang diagnostic pop
#else
#pragma GCC diagnostic pop
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
                return 1;
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
    constexpr PrimeFactor(IntType prime_factor, uint32_t prime_factor_power) noexcept
        : factor(prime_factor), factor_power(prime_factor_power) {}

    IntType factor;
    uint32_t factor_power;
};

#if CONFIG_HAS_AT_LEAST_CXX_20 && !defined(_GLIBCXX_DEBUG) && !defined(_GLIBCXX_ASSERTIONS)
#define CONSTEXPR_VECTOR constexpr
#else
#define CONSTEXPR_VECTOR inline
#endif

template <class NumericType, class Function>
#if CONFIG_HAS_CONCEPTS
    requires(sizeof(NumericType) >= sizeof(int)) &&
            ::math_functions::detail::integral<NumericType> &&
            std::is_invocable_v<
                Function, typename ::math_functions::PrimeFactor<
                              typename ::math_functions::detail::make_unsigned_t<NumericType>>>
#endif
[[nodiscard]] ATTRIBUTE_ALWAYS_INLINE constexpr auto
visit_prime_factors(NumericType n, Function visitor) noexcept(
    std::is_nothrow_invocable_v<
        Function, typename ::math_functions::PrimeFactor<
                      typename ::math_functions::detail::make_unsigned_t<NumericType>>>) {
    using UnsignedNumericType = typename ::math_functions::detail::make_unsigned_t<NumericType>;
    using PrimeFactorType     = typename ::math_functions::PrimeFactor<UnsignedNumericType>;
    UnsignedNumericType n_abs = ::math_functions::uabs(n);

    constexpr bool check_early_exit = std::is_invocable_r_v<bool, Function, PrimeFactorType>;

    if (n_abs % 2 == 0 && n_abs > 0) {
        // n_abs = s * 2^pow_of_2, where s is odd
        auto [s, pow_of_2] = ::math_functions::extract_pow2(n_abs);
        n_abs              = s;
        if constexpr (check_early_exit) {
            if (!visitor(PrimeFactorType{UnsignedNumericType{2}, pow_of_2})) {
                return;
            }
        } else {
            visitor(PrimeFactorType{UnsignedNumericType{2}, pow_of_2});
        }
    }

    for (UnsignedNumericType d = 3; d * d <= n_abs; d += 2) {
        CONFIG_ASSUME_STATEMENT(d >= 3);
        if (n_abs % d == 0) {
            uint32_t pow_of_d = 0;
            do {
                pow_of_d++;
                n_abs /= d;
            } while (n_abs % d == 0);

            if constexpr (check_early_exit) {
                if (!visitor(PrimeFactorType(d, pow_of_d))) {
                    return;
                }
            } else {
                visitor(PrimeFactorType(d, pow_of_d));
            }
        }
    }

    if (n_abs > 1) {
        visitor(PrimeFactorType(n_abs, uint32_t{1}));
    }
}

/// @brief
/// @tparam NumericType
/// @param[in] n
/// @return vector of pairs { prime_div : power_of_prime_div },
///          sorted by prime_div.
template <class NumericType>
[[nodiscard]] CONSTEXPR_VECTOR auto prime_factors_as_vector(NumericType n) {
    using UnsignedNumericType = typename ::math_functions::detail::make_unsigned_t<NumericType>;

    std::vector<typename ::math_functions::PrimeFactor<UnsignedNumericType>> prime_factors_vector;
    constexpr bool kReservePlaceForFactors = std::is_same_v<UnsignedNumericType, uint32_t>;
    if constexpr (kReservePlaceForFactors) {
        prime_factors_vector.reserve(::math_functions::detail::max_number_of_unique_prime_divisors(
            ::math_functions::uabs(n)));
    }

    ::math_functions::visit_prime_factors(
        n, [&prime_factors_vector](::math_functions::PrimeFactor<UnsignedNumericType> pf)
#if CONFIG_HAS_AT_LEAST_CXX_20 && !defined(_GLIBCXX_DEBUG) && !defined(_GLIBCXX_ASSERTIONS)
               constexpr
#endif
        noexcept(kReservePlaceForFactors) { prime_factors_vector.push_back(std::move(pf)); });

    return prime_factors_vector;
}

template <class NumericType>
[[nodiscard]] inline auto prime_factors_as_map(NumericType n) {
    using UnsignedNumericType = typename ::math_functions::detail::make_unsigned_t<NumericType>;

    std::map<UnsignedNumericType, uint32_t> prime_factors_map;

    ::math_functions::visit_prime_factors(
        n, [&prime_factors_map, iter = prime_factors_map.end()](
               ::math_functions::PrimeFactor<UnsignedNumericType> pf) mutable {
            iter = prime_factors_map.emplace_hint(iter, pf.factor, pf.factor_power);
        });

    return prime_factors_map;
}

/// @brief https://cp-algorithms.com/algebra/prime-sieve-linear.html
class [[nodiscard]] Factorizer final {
public:
    using PrimeFactors = std::vector<PrimeFactor<uint32_t>>;

    explicit CONSTEXPR_VECTOR Factorizer(uint32_t n) : primes(), least_prime_factor(size_t{n} + 1) {
        for (uint32_t i = 2; i <= size_t{n}; i++) {
            if (least_prime_factor[i] == 0) {
                least_prime_factor[i] = i;
                primes.push_back(i);
            }
            for (size_t prime_index = 0;; prime_index++) {
                const auto p = primes[prime_index];
                const auto x = size_t{p} * i;
                if (x > n) {
                    break;
                }
                least_prime_factor[x] = p;
                // assert(p <= least_prime_factor[i]);
                if (p == least_prime_factor[i]) {
                    break;
                }
            }
        }
    }

    [[nodiscard]] constexpr const auto& sorted_primes() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return primes;
    }
    [[nodiscard]] constexpr const auto& least_prime_factors() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return least_prime_factor;
    }
    [[nodiscard]] CONSTEXPR_VECTOR bool is_prime(uint32_t n) const noexcept {
        return least_prime_factor[n] == n && n >= 2;
    }

    [[nodiscard]] CONSTEXPR_VECTOR PrimeFactors prime_factors(uint32_t n) const {
        PrimeFactors pfs;
        if (n % 2 == 0 && n > 0) {
            const auto [n_div_pow_of_2, power_of_2] = ::math_functions::extract_pow2(n);
            pfs.emplace_back(uint32_t{2}, power_of_2);
            n = n_div_pow_of_2;
        }

        while (n >= 3) {
            const auto lpf = least_prime_factor[n];
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
    CONSTEXPR_VECTOR uint32_t number_of_unique_prime_factors(uint32_t n) const noexcept {
        return ::math_functions::Factorizer::number_of_unique_prime_factors_impl(
            least_prime_factor.data(), n);
    }

private:
    ATTRIBUTE_PURE
    ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
    static constexpr uint32_t number_of_unique_prime_factors_impl(
        const uint32_t* least_prime_factor, uint32_t n) noexcept {
        uint32_t unique_pfs_count = 0;
        uint32_t last_pf          = 0;
        if (n % 2 == 0) {
            if (unlikely(n == 0)) {
                return unique_pfs_count;
            }
            n       = ::math_functions::extract_pow2(n).odd_part;
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

    std::vector<uint32_t> primes;
    std::vector<uint32_t> least_prime_factor;
};

/// @brief Find all prime numbers in [2; n]
/// @param n inclusive upper bound
/// @return vector, such that vector[n] == true \iff n is prime
[[nodiscard]] CONSTEXPR_VECTOR auto dynamic_primes_sieve(uint32_t n) {
    std::vector<bool> primes(size_t{n} + 1, true);
    primes[0] = false;
    if (likely(n > 0)) {
        primes[1]           = false;
        const uint32_t root = ::math_functions::isqrt(n);
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
    (__cpp_lib_constexpr_bitset >= 202207L || CONFIG_HAS_AT_LEAST_CXX_23)
#define CONSTEXPR_BITSET_OPS constexpr
#if defined(__cpp_constexpr) && __cpp_constexpr >= 202211L
#define CONSTEXPR_FIXED_PRIMES_SIEVE constexpr
#define CONSTEXPR_PRIMES_SIEVE       constexpr
#else
#define CONSTEXPR_FIXED_PRIMES_SIEVE
#define CONSTEXPR_PRIMES_SIEVE constinit
#endif
#else
#define CONSTEXPR_BITSET_OPS
#define CONSTEXPR_FIXED_PRIMES_SIEVE
#define CONSTEXPR_PRIMES_SIEVE
#endif

/// @brief Find all prime numbers in [2; N]
/// @tparam N exclusive upper bound
/// @return bitset, such that bitset[n] == true \iff n is prime
template <uint32_t N>
[[nodiscard]] CONSTEXPR_FIXED_PRIMES_SIEVE inline const auto& fixed_primes_sieve() noexcept {
    using PrimesSet                                         = std::bitset<size_t{N} + 1>;
    static CONSTEXPR_PRIMES_SIEVE const PrimesSet primes_bs = []() CONSTEXPR_BITSET_OPS noexcept {
        PrimesSet primes{};
        primes.set();
        primes[0] = false;
        if constexpr (primes.size() > 1) {
            primes[1]               = false;
            constexpr uint32_t root = ::math_functions::isqrt(N);
            if constexpr (constexpr uint32_t i = 2; i <= root) {
                // NOLINTNEXTLINE(bugprone-implicit-widening-of-multiplication-result)
                for (size_t j = i * i; j <= N; j += i) {
                    primes[j] = false;
                }
            }
            static_assert(root < std::numeric_limits<uint16_t>::max(), "isqrt impl error");
            for (uint32_t i = 3; i <= root; i += 2) {
                if (primes[i]) {
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

#undef CONSTEXPR_PRIMES_SIEVE
#undef CONSTEXPR_FIXED_PRIMES_SIEVE
#undef CONSTEXPR_BITSET_OPS

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
template <typename IntType>
[[nodiscard]] ATTRIBUTE_CONST constexpr auto extended_euclid_algorithm(IntType a,
                                                                       IntType b) noexcept {
    static_assert(::math_functions::detail::is_integral_v<IntType>, "Integral type expected");

    int64_t u_previous = a != 0;
    int64_t u_current  = 0;
    int64_t v_previous = 0;
    int64_t v_current  = 1;

    using CompIntType = std::conditional_t<sizeof(IntType) >= sizeof(int), IntType, int64_t>;

    CompIntType r_previous = a;
    CompIntType r_current  = b;
    while (r_current != 0) {
        const int64_t q_current  = static_cast<int64_t>(r_previous / r_current);
        const CompIntType r_next = r_previous % r_current;

        r_previous = r_current;
        r_current  = r_next;

        const int64_t u_next = u_previous - u_current * q_current;
        u_previous           = u_current;
        u_current            = u_next;

        const int64_t v_next = v_previous - v_current * q_current;
        v_previous           = v_current;
        v_current            = v_next;
    }

    if constexpr (::math_functions::detail::is_signed_v<CompIntType>) {
        if (r_previous < 0) {
            u_previous = -u_previous;
            v_previous = -v_previous;
            r_previous = -r_previous;
        }
    }

    using RetUIntType = typename ::math_functions::detail::make_unsigned_t<CompIntType>;
    return ExtEuclidAlgoRet<RetUIntType>{
        u_previous,
        v_previous,
        static_cast<RetUIntType>(r_previous),
    };
}

inline constexpr auto kNoCongruenceSolution = std::numeric_limits<uint32_t>::max();

namespace detail {

struct HelperRetType {
    uint32_t x0 = kNoCongruenceSolution;  // first solution of congruence
    uint32_t d  = 0;                      // gcd(a, m)
    uint32_t m_ = 0;                      // m / d
};

ATTRIBUTE_CONST
ATTRIBUTE_ALWAYS_INLINE
constexpr HelperRetType congruence_helper(const uint32_t a, const uint32_t c,
                                          const uint32_t m) noexcept {
    const uint32_t d = std::gcd(a, m);
    if (m == 0 || c % d != 0) {
        return {};
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
    const int64_t u_       = ::math_functions::extended_euclid_algorithm<uint32_t>(a_, m_).u_value;
    const auto unsigned_u_ = static_cast<uint64_t>(u_ >= 0 ? u_ : u_ + m_);

    // a_ * (u_ * c_) + m_ * (v_ * c_) == c_
    // a * (u_ * c_) + m * (v_ * c_) == c
    // x0 = u_ * c_
    const auto x0 = static_cast<uint32_t>((unsigned_u_ * c_) % m_);
    CONFIG_ASSUME_STATEMENT(x0 != ::math_functions::kNoCongruenceSolution);
    return {x0, d, m_};
}

CONSTEXPR_VECTOR
std::vector<uint32_t> solve_congruence_modulo_m_all_roots_impl(uint32_t a, uint32_t c, uint32_t m) {
    const auto [x0, d, m_] = ::math_functions::detail::congruence_helper(a, c, m);
    std::vector<uint32_t> solutions(d);
    auto x = x0;
    for (uint32_t& ith_solution : solutions) {
        ith_solution = x;
        x += m_;
    }

    return solutions;
}

ATTRIBUTE_CONST constexpr uint32_t solve_congruence_modulo_m_impl(uint32_t a, uint32_t c,
                                                                  uint32_t m) noexcept {
    return ::math_functions::detail::congruence_helper(a, c, m).x0;
}

template <class T>
ATTRIBUTE_CONST ATTRIBUTE_ALWAYS_INLINE constexpr uint32_t congruence_arg(
    T x, [[maybe_unused]] uint32_t m) noexcept {
    static_assert(::math_functions::detail::is_integral_v<T>,
                  "Expected integral type in the congruence");
    if constexpr (::math_functions::detail::is_unsigned_v<T>) {
        if constexpr (sizeof(x) > sizeof(uint32_t)) {
            return static_cast<uint32_t>(x % m);
        } else {
            return static_cast<uint32_t>(x);
        }
    } else {
        if constexpr (sizeof(x) > sizeof(uint32_t)) {
            const auto x_mod_m = x % m;
            return static_cast<uint32_t>(x_mod_m >= 0 ? x_mod_m : x_mod_m + m);
        } else {
            const auto x_mod_m = static_cast<int64_t>(x) % m;
            return static_cast<uint32_t>(x_mod_m >= 0 ? x_mod_m : x_mod_m + m);
        }
    }
}

}  // namespace detail

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
[[nodiscard]] ATTRIBUTE_ALWAYS_INLINE CONSTEXPR_VECTOR std::vector<uint32_t>
solve_congruence_modulo_m_all_roots(T1 a, T2 c, uint32_t m) {
    return ::math_functions::detail::solve_congruence_modulo_m_all_roots_impl(
        ::math_functions::detail::congruence_arg(a, m),
        ::math_functions::detail::congruence_arg(c, m), m);
}

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
[[nodiscard]] ATTRIBUTE_CONST ATTRIBUTE_ALWAYS_INLINE constexpr uint32_t solve_congruence_modulo_m(
    T1 a, T2 c, uint32_t m) noexcept {
    return ::math_functions::detail::solve_congruence_modulo_m_impl(
        ::math_functions::detail::congruence_arg(a, m),
        ::math_functions::detail::congruence_arg(c, m), m);
}

#if CONFIG_HAS_CONCEPTS

/// @brief Solves modulus congruence a * x ≡ 1 (mod m) (e.g. a^{-1} mod m)
/// @note a^{-1} mod m exists <=> gcd(a, m) == 1.
///       Works in O(log(min(a, m))
/// @tparam IntType
/// @param a
/// @param m
/// @return
template <::math_functions::detail::integral IntType>
    requires(!std::is_same_v<IntType, bool>)
[[nodiscard]]
ATTRIBUTE_CONST constexpr uint32_t inv_mod_m(IntType a, uint32_t m) noexcept {
    return ::math_functions::solve_congruence_modulo_m(a, uint32_t{1}, m);
}

#else

// clang-format off

/// @brief Solves modulus congruence a * x ≡ 1 (mod m) (e.g. a^{-1} mod m)
/// @note a^{-1} mod m exists <=> gcd(a, m) == 1.
///       Works in O(log(min(a, m))
/// @tparam IntType
/// @param a
/// @param m
/// @return
template <class IntType>
[[nodiscard]] ATTRIBUTE_CONST constexpr
std::enable_if_t<::math_functions::detail::is_integral_v<IntType>, uint32_t>
inv_mod_m(IntType a, uint32_t m) noexcept {
    return ::math_functions::solve_congruence_modulo_m(a, uint32_t{1}, m);
}

// clang-format on

#endif

struct InverseResult {
    std::vector<uint32_t> numbers_mod_m;
    std::vector<uint32_t> inversed_numbers;
};

namespace detail {

template <class Iter>
CONSTEXPR_VECTOR typename ::math_functions::InverseResult inv_mod_m_impl(Iter nums_begin,
                                                                         Iter nums_end,
                                                                         uint32_t m) {
    const auto n = static_cast<size_t>(std::distance(nums_begin, nums_end));
    auto res     = ::math_functions::InverseResult{
        std::vector<uint32_t>(n),
        std::vector<uint32_t>(n),
    };

    uint32_t prod_mod_m = 1;
    {
        auto nums_mod_m_iter     = res.numbers_mod_m.begin();
        auto inv_nums_mod_m_iter = res.inversed_numbers.begin();
        for (auto iter = nums_begin; iter != nums_end;
             ++iter, ++nums_mod_m_iter, ++inv_nums_mod_m_iter) {
            const auto num_mod_m = ::math_functions::detail::congruence_arg(*iter, m);
            *nums_mod_m_iter     = num_mod_m;
            *inv_nums_mod_m_iter = prod_mod_m;
            prod_mod_m           = static_cast<uint32_t>((uint64_t{prod_mod_m} * num_mod_m) % m);
        }
    }

    const uint32_t inv_nums_prod_mod_m = ::math_functions::inv_mod_m(prod_mod_m, m);

    {
        auto inv_nums_mod_m_iter = res.inversed_numbers.rbegin();
        uint32_t suffix_prod     = 1;
        for (auto iter = res.numbers_mod_m.rbegin(), nums_mod_m_rend = res.numbers_mod_m.rend();
             iter != nums_mod_m_rend; ++iter, ++inv_nums_mod_m_iter) {
            const uint64_t t     = (uint64_t{suffix_prod} * *inv_nums_mod_m_iter) % m;
            *inv_nums_mod_m_iter = static_cast<uint32_t>((t * inv_nums_prod_mod_m) % m);
            suffix_prod          = static_cast<uint32_t>((uint64_t{suffix_prod} * *iter) % m);
        }
    }

    return res;
}

}  // namespace detail

#if CONFIG_HAS_CONCEPTS

// clang-format off

template <std::forward_iterator Iter>
    requires ::math_functions::detail::integral<typename std::iter_value_t<Iter>> &&
             (!std::same_as<typename std::iter_value_t<Iter>, bool>)
[[nodiscard]]
CONSTEXPR_VECTOR
typename ::math_functions::InverseResult inv_mod_m(Iter nums_iter_begin, Iter nums_iter_end, uint32_t m) {
    return ::math_functions::detail::inv_mod_m_impl(nums_iter_begin, nums_iter_end, m);
}

/// @brief Inverse @a nums mod m
/// @note Works in O(nums.size())
/// @tparam T 
/// @param nums 
/// @param m 
/// @return 
template <std::ranges::forward_range Container>
[[nodiscard]]
CONSTEXPR_VECTOR
typename ::math_functions::InverseResult inv_mod_m(const Container& nums, uint32_t m) {
    return ::math_functions::inv_mod_m(std::begin(nums), std::end(nums), m);
}

// clang-format on

#else

template <class Iter>
[[nodiscard]]
CONSTEXPR_VECTOR std::enable_if_t<
    ::math_functions::detail::is_integral_v<typename std::iterator_traits<Iter>::value_type> &&
        !std::is_same_v<typename std::iterator_traits<Iter>::value_type, bool>,
    typename ::math_functions::InverseResult> inv_mod_m(Iter nums_iter_begin, Iter nums_iter_end,
                                                        uint32_t m) {
    return ::math_functions::detail::inv_mod_m_impl(nums_iter_begin, nums_iter_end, m);
}

template <class Container>
[[nodiscard]]
CONSTEXPR_VECTOR
    std::enable_if_t<::math_functions::detail::is_integral_v<typename std::iterator_traits<
                         decltype(std::begin(std::declval<Container&>()))>::value_type>&& ::
                         math_functions::detail::is_integral_v<typename std::iterator_traits<
                             decltype(std::end(std::declval<Container&>()))>::value_type>,
                     typename ::math_functions::InverseResult> inv_mod_m(const Container& nums,
                                                                         uint32_t m) {
    return ::math_functions::inv_mod_m(std::begin(nums), std::end(nums), m);
}

#endif

/// @brief Solve congruence 2^k * x ≡ c (mod m),
///        Works in O(min(k, log(m)))
/// @note  Faster implementation of @fn solve_congruence_modulo_m
///        for a = 2^k.
/// @param k
/// @param c
/// @param m
/// @return
[[nodiscard]]
ATTRIBUTE_CONST constexpr uint32_t solve_binary_congruence_modulo_m(const uint32_t k,
                                                                    const uint32_t c,
                                                                    const uint32_t m) noexcept {
    if (m == 0) {
        return ::math_functions::kNoCongruenceSolution;
    }

    const auto [r, s] = ::math_functions::extract_pow2(m);
    CONFIG_ASSUME_STATEMENT(r >= 1);
    const auto min_k_s = std::min(k, uint32_t{s});
    CONFIG_ASSUME_STATEMENT(min_k_s < 32);
    // gcd(2^k, m)
    const auto gcd_2k_m = uint32_t{1} << min_k_s;
    if (c % gcd_2k_m != 0) {
        return ::math_functions::kNoCongruenceSolution;
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
    uint64_t rhs       = c_mod_m_;
    auto lhs_bin_power = k - s;

    constexpr unsigned kThreshold = 60;
    if (lhs_bin_power > kThreshold) {
        const int64_t u_ = ::math_functions::extended_euclid_algorithm<uint32_t>(
                               ::math_functions::bin_pow_mod(uint32_t{2}, lhs_bin_power, m_), m_)
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
    CONFIG_ASSUME_STATEMENT(x0 != ::math_functions::kNoCongruenceSolution);
    return x0;
}

/// @brief Return max q, such that n! ≡ 0 mod(k^q), where k > 1 and n >= 0
/// @param n
/// @param k
/// @return q if k > 1 and std::numeric_limits<uint32_t>::max() otherwise
[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t solve_factorial_congruence(uint32_t n,
                                                                            uint32_t k) noexcept {
    /*
     * let k  = p_1^a_1 * p_2^a_2 * ... * p_m^a_m
     * let n! = p_1^b_1 * p_2^b_2 * ...
     * then b_i = ⌊n / p_i⌋ + ⌊n / p_i^2⌋ + ⌊n / p_i^3⌋ + ...
     *
     * then q = min{ b_i / a_i | 1 <= i <= m }
     **/

    auto ans = std::numeric_limits<uint32_t>::max();
    ::math_functions::visit_prime_factors(
        k, [&ans, n](::math_functions::PrimeFactor<uint32_t> pf) constexpr noexcept {
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
            ans                = std::min(ans, b_i / a_i);
            return true;
        });

    return ans;
}

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_perfect_number(uint32_t n) noexcept {
    switch (n) {
        case 6:
        case 28:
        case 496:
        case 8128:
        case 33550336:
            return true;
        default:
            return false;
    }
}

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_perfect_number(const uint64_t n) noexcept {
    // See https://en.wikipedia.org/wiki/List_of_Mersenne_primes_and_perfect_numbers
#if defined(MATH_FUNCTIONS_HPP_ENABLE_TARGET_OPTIONS)
    const auto [q, pm1] = extract_pow2(n);
    const auto p        = pm1 + 1;
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
                case 2305843008139952128ULL:
                    break;
                default:
                    CONFIG_UNREACHABLE();
                    break;
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
                case 2305843008139952128ULL:
                    CONFIG_UNREACHABLE();
                    break;
                default:
                    break;
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
        case 2305843008139952128ULL:
            return true;
        default:
            return false;
    }
#endif
}

#if defined(INTEGERS_128_BIT_HPP)

[[nodiscard]] ATTRIBUTE_CONST I128_CONSTEXPR bool is_perfect_number(uint128_t n) noexcept {
    return n > std::numeric_limits<uint64_t>::max()
               ? n == (((uint128_t{1} << 61U) - 1) << (61U - 1U))
               : ::math_functions::is_perfect_number(static_cast<uint64_t>(n));
}

#endif

namespace detail {

template <uint32_t M, class T>
ATTRIBUTE_CONST ATTRIBUTE_ALWAYS_INLINE constexpr T unrolled_pow(
    [[maybe_unused]] const uint32_t n) noexcept {
    if constexpr (M == 0) {
        return 1;
    } else if constexpr (M == 1) {
        return n;
    } else if constexpr (M == 2) {
        return n * n;
    } else {
        const auto tmp = ::math_functions::detail::unrolled_pow<M / 2, T>(n);
        if constexpr (M % 2 == 0) {
            return tmp * tmp;
        } else {
            return tmp * tmp * n;
        }
    }
}

template <uint32_t N, uint32_t K, class T>
ATTRIBUTE_CONST ATTRIBUTE_ALWAYS_INLINE constexpr T unrolled_cnk() noexcept {
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
            return ::math_functions::detail::unrolled_cnk<N - 1, KD, T>() +
                   ::math_functions::detail::unrolled_cnk<N - 1, KD - 1, T>();
        }
    }
}

template <uint32_t M, class T>
ATTRIBUTE_CONST constexpr T powers_sum(const uint32_t n) noexcept;

template <uint32_t M, uint32_t I, class T>
ATTRIBUTE_CONST ATTRIBUTE_ALWAYS_INLINE constexpr T helper(const uint32_t n) noexcept {
    static_assert(I <= M + 1);
    const auto res = ::math_functions::detail::unrolled_cnk<M + 1, I, T>() *
                     ::math_functions::detail::powers_sum<M + 1 - I, T>(n);
    if constexpr (I + 1 <= M + 1) {
        return res + ::math_functions::detail::helper<M, I + 1, T>(n);
    } else {
        return res;
    }
}

template <uint32_t M, class T>
ATTRIBUTE_CONST constexpr T powers_sum(const uint32_t n) noexcept {
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
        const T tmp      = static_cast<T>((n * (n_u64 + 1)) / 2);
        const auto tmp2  = 2 * n_u64 + 1;
        return (tmp2 % 3 == 0) ? (tmp * (tmp2 / 3)) : ((tmp / 3) * tmp2);
    } else if constexpr (M == 3) {
        // n * n * (n + 1) * (n + 1) / 4
        const auto tmp = static_cast<T>((n * (static_cast<uint64_t>(n) + 1)) / 2);
        return tmp * tmp;
    } else if constexpr (M == 4) {
        // n * (n + 1) * (6 * n^3 + 9 * n^2 + n - 1) / 30
        const auto n_u64                = static_cast<uint64_t>(n);
        const uint64_t n_square_u64     = n_u64 * n;
        const auto tmp                  = static_cast<T>((n_square_u64 + n) / 2);
        const uint64_t two_n_plus_3_u64 = 2 * n_u64 + 3;
        const uint32_t n_minus_1        = n - 1;
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
        return (::math_functions::detail::unrolled_pow<M + 1, T>(n + 1) - 1 -
                ::math_functions::detail::helper<M, 2, T>(n)) /
               ::math_functions::detail::unrolled_cnk<M + 1, 1, T>();
    }
}

}  // namespace detail

/// @brief Return 1^M + 2^M + ... + n^M
/// @tparam M
/// @param n
/// @return
template <uint32_t M>
[[nodiscard]]
ATTRIBUTE_CONST constexpr uint64_t powers_sum_u64(const uint32_t n) noexcept {
    return ::math_functions::detail::powers_sum<M, uint64_t>(n);
}

#if defined(INTEGERS_128_BIT_HPP)

/// @brief Return 1^M + 2^M + ... + n^M
/// @tparam M
/// @param n
/// @return
template <uint32_t M>
[[nodiscard]] ATTRIBUTE_CONST constexpr uint128_t powers_sum_u128(const uint32_t n) noexcept {
    return ::math_functions::detail::powers_sum<M, uint128_t>(n);
}

#endif

template <class T>
[[nodiscard]] CONSTEXPR_VECTOR std::vector<T> arange(T begin, T end, T step) {
    static_assert(::math_functions::detail::is_integral_v<T>);

    const size_t size = [begin, end, step]() mutable constexpr noexcept -> size_t {
        if (unlikely(step == 0)) {
            return 0;
        }

        if constexpr (::math_functions::detail::is_signed_v<T>) {
            if (step < 0) {
                step = -step;
                std::swap(begin, end);
            }
        }

        T approx_size = (end - begin + step - 1) / step;
        if constexpr (::math_functions::detail::is_signed_v<T>) {
            approx_size = std::max(approx_size, T{0});
        }

#if defined(INTEGERS_128_BIT_HPP)
        if constexpr (std::is_same_v<T, int128_t> || std::is_same_v<T, uint128_t>) {
            constexpr auto kUsizeMax = std::numeric_limits<size_t>::max();
            return approx_size <= kUsizeMax ? static_cast<size_t>(approx_size) : kUsizeMax;
        } else
#endif
            if constexpr (::math_functions::detail::is_signed_v<T>) {
            return static_cast<size_t>(approx_size);
        } else {
            return size_t{approx_size};
        }
    }();

    std::vector<T> rng(size);
    T i = begin;
    for (T& elem : rng) {
        elem = i;
        i += step;
    }

    return rng;
}

template <class T>
[[nodiscard]] CONSTEXPR_VECTOR std::vector<T> arange(T begin, T end) {
    return ::math_functions::arange(begin, end, T{1});
}

template <class T>
[[nodiscard]] CONSTEXPR_VECTOR std::vector<T> arange(T n) {
    return ::math_functions::arange(T{0}, n);
}

}  // namespace math_functions

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

#if defined(INTEGERS_128_BIT_HPP)

namespace std {

// NOLINTBEGIN(cert-dcl58-cpp)

/// @brief Computes greaters common divisor of @a `a` and @a `b`
///         using Stein's algorithm (binary gcd). Here gcd(0, 0) = 0.
/// @param[in] a
/// @param[in] b
/// @return gcd(a, b)
[[nodiscard]] ATTRIBUTE_CONST I128_CONSTEXPR uint128_t gcd(uint128_t a, uint128_t b) noexcept {
    if (unlikely(a == 0)) {
        return b;
    }
    if (unlikely(b == 0)) {
        return a;
    }

    const uint32_t ra   = static_cast<uint32_t>(::math_functions::countr_zero(a));
    const uint32_t rb   = static_cast<uint32_t>(::math_functions::countr_zero(b));
    const uint32_t mult = std::min(ra, rb);
    a >>= ra;
    b >>= rb;
    while (true) {
        if (a < b) {
            // std::swap is not constexpr in C++17
            const uint128_t tmp = a;
            a                   = b;
            b                   = tmp;
        }

        a -= b;
        if (a == 0) {
            return b << mult;
        }

        a >>= static_cast<uint32_t>(::math_functions::countr_zero(a));
    }
}

[[nodiscard]] ATTRIBUTE_CONST I128_CONSTEXPR uint128_t gcd(uint64_t a, int128_t b) noexcept {
    const uint128_t b0 = ::math_functions::uabs(b);
    if (unlikely(b0 == 0)) {
        return a;
    }

    // gcd(a, b) = gcd(a, b0) = gcd(b0, a % b0) = gcd(a1, b1)
    const uint128_t a1 = b0;
    // b1 = a % b0
    const uint64_t b1 = a < b0 ? a : a % static_cast<uint64_t>(b0);  // a < 2^64 => b1 < 2^64
    if (b1 == 0) {
        return a1;
    }
    // gcd(a1, b1) = gcd(b1, a1 % b1) = gcd(a2, b2)
    const uint64_t a2 = b1;  // b1 < 2^64 => a2 < 2^64
    // b2 = a1 % b1
    // a1 = b0, b1 = a % b0 => b1 < a1
    const uint64_t b2 = static_cast<uint64_t>(a1 % b1);  // b1 < 2^64 => b2 = a1 % b1 < 2^64
    return std::gcd(a2, b2);
}

[[nodiscard]] ATTRIBUTE_CONST I128_CONSTEXPR uint128_t gcd(int128_t a, uint64_t b) noexcept {
    return std::gcd(b, a);
}

// NOLINTEND(cert-dcl58-cpp)

}  // namespace std

#endif  // INTEGERS_128_BIT_HPP

#undef CONSTEXPR_VECTOR

#if defined(MATH_FUNCTIONS_HPP_ENABLE_TARGET_OPTIONS)
#if defined(__GNUG__)
#if !defined(__clang__)
#pragma GCC pop_options
#else
#pragma clang attribute pop
#endif
#endif
#undef MATH_FUNCTIONS_HPP_ENABLE_TARGET_OPTIONS
#endif

#if defined(_MSC_VER)
#pragma warning(pop)
#endif  // _MSC_VER

#endif  // !MATH_FUNCTIONS_HPP
