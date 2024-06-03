#ifndef MATH_FUNCTIONS_HPP
#define MATH_FUNCTIONS_HPP 1

// clang-format off
/**
 * Define to 1 to if you want to compile some functions
 *  with a bit faster instruction (see description below)
 */
#define MATH_FUNCTIONS_HPP_ENABLE_TARGET_OPTIONS 0
/**
 * From lzcnt: bsr -> lzcnt (leading zeros count)
 * Used in: log2_floor, log2_ceil, log10_floor, base_10_len
 *
 * From bmi: bsf -> tzcnt (trailing zeros count)
 * Used in: extract_pow2, next_n_bits_permutation, gcd
 */
// clang-format on

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <numeric>
#include <type_traits>
#include <utility>
#include <vector>

#include "config_macros.hpp"

#if CONFIG_HAS_AT_LEAST_CXX_20
#include <bit>  // std::popcount, std::countr_zero, std::countl_zero
#endif

#if CONFIG_HAS_INCLUDE("integers_128_bit.hpp")
#include "integers_128_bit.hpp"
#endif

// Visual C++ thinks that unary minus on unsigned is an error
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4146)
#endif  // _MSC_VER

#if MATH_FUNCTIONS_HPP_ENABLE_TARGET_OPTIONS
#if defined(__GNUC__)
#if !defined(__clang__)
#pragma GCC push_options
#pragma GCC target("lzcnt,bmi")
#else
#pragma clang attribute push(__attribute__((target("lzcnt,bmi"))), apply_to = function)
#endif  // !__clang__
#endif  // __GNUC__
#endif  // MATH_FUNCTIONS_HPP_ENABLE_TARGET_OPTIONS

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
constexpr T bin_pow(T n, size_t p) noexcept {
    T res = 1;
    while (true) {
        if (p & 1) {
            res *= n;
        }
        p >>= 1;
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
template <class T>
constexpr T bin_pow(T n, int64_t p) noexcept {
    const bool not_inverse = p >= 0;
    uint64_t p_u           = p >= 0 ? static_cast<uint64_t>(p) : -static_cast<uint64_t>(p);
    T res                  = 1;
    while (true) {
        if (p_u & 1) {
            res *= n;
        }
        p_u >>= 1;
        if (p_u == 0) {
            return not_inverse ? res : 1 / res;
        }
        n *= n;
    }
}

/// @brief Calculate (n ^ p) % mod
/// @param[in] n
/// @param[in] p
/// @param[in] mod
/// @return (n ^ p) % mod
ATTRIBUTE_CONST constexpr uint32_t bin_pow_mod(uint32_t n, uint32_t p, uint32_t mod) noexcept {
    uint64_t res   = mod != 1;
    uint64_t wdn_n = n;
    while (true) {
        if (p & 1) {
            res = (res * wdn_n) % mod;
        }
        p >>= 1;
        if (p == 0) {
            return static_cast<uint32_t>(res);
        }
        wdn_n = (wdn_n * wdn_n) % mod;
    }
}

#if defined(INTEGERS_128_BIT_HPP)

/// @brief Calculate (n ^ p) % mod
/// @param[in] n
/// @param[in] p
/// @param[in] mod
/// @return (n ^ p) % mod
ATTRIBUTE_CONST inline I128_CONSTEXPR uint64_t bin_pow_mod(uint64_t n, uint64_t p,
                                                           uint64_t mod) noexcept {
    uint64_t res = mod != 1;
    while (true) {
        if (p & 1) {
            res = uint64_t((uint128_t(res) * n) % mod);
        }
        p >>= 1;
        if (p == 0) {
            return res;
        }
        n = uint64_t((uint128_t(n) * n) % mod);
    }
}

#endif

ATTRIBUTE_CONST constexpr uint32_t isqrt(uint32_t n) noexcept {
    uint32_t y = 0;

#if defined(__cpp_lib_is_constant_evaluated) && __cpp_lib_is_constant_evaluated >= 201811L
    if (std::is_constant_evaluated()) {
        /**
         * See Hackers Delight Chapter 11.
         */
        for (uint32_t m = 0x40000000; m != 0; m >>= 2) {
            uint32_t b = y | m;
            y >>= 1;
            if (n >= b) {
                n -= b;
                y |= m;
            }
        }
    } else
#endif
    {
        y = static_cast<uint32_t>(std::sqrt(static_cast<double>(n)));
    }

    ATTRIBUTE_ASSUME(y < (1u << 16));
    return y;
}

ATTRIBUTE_CONST constexpr uint32_t isqrt(uint64_t n) noexcept {
    /**
     * See Hackers Delight Chapter 11.
     */
    uint64_t l = 1;
    uint64_t r = std::min((n >> 5) + 8, uint64_t(0xFFFFFFFFull));
    do {
        uint64_t m = (l + r) / 2;
        if (n >= m * m) {
            l = m + 1;
        } else {
            r = m - 1;
        }
    } while (r >= l);
    ATTRIBUTE_ASSUME(((l - 1) >> 32) == 0);
    return uint32_t(l - 1);
}

#if defined(INTEGERS_128_BIT_HPP)

ATTRIBUTE_CONST inline I128_CONSTEXPR uint64_t isqrt(uint128_t n) noexcept {
    /**
     * See Hackers Delight Chapter 11.
     */
    uint64_t l         = 0;
    uint128_t r_approx = (n >> 6) + 16;
    uint64_t r =
        r_approx > 0xFFFFFFFFFFFFFFFFull ? uint64_t(0xFFFFFFFFFFFFFFFFull) : uint64_t(r_approx);
    do {
        // m = (l + r + 1) / 2
        uint64_t m = (l / 2) + (r / 2) + ((r % 2) | (l % 2));
        if (n >= uint128_t(m) * m) {
            l = m;
        } else {
            r = m - 1;
        }
    } while (r > l);
    return l;
}

#endif

ATTRIBUTE_CONST constexpr uint32_t icbrt(uint32_t n) noexcept {
    /**
     * See Hackers Delight Chapter 11.
     */
    uint32_t y = 0;
    for (int32_t s = 30; s >= 0; s -= 3) {
        y *= 2;
        uint32_t b = (3 * y * (y + 1) | 1) << s;
        if (n >= b) {
            n -= b;
            y++;
        }
    }
    // 1625^3 = 4291015625 < 2^32 - 1 = 4294967295 < 4298942376 = 1626^3
    ATTRIBUTE_ASSUME(y <= 1625u);
    return y;
}

ATTRIBUTE_CONST constexpr uint32_t icbrt(uint64_t n) noexcept {
    /**
     * See Hackers Delight Chapter 11.
     */
    uint64_t y = 0;
    if (n >= 0x1000000000000000ull) {
        if (n >= 0x8000000000000000ull) {
            n -= 0x8000000000000000ull;
            y = 2;
        } else {
            n -= 0x1000000000000000ull;
            y = 1;
        }
    }
    for (int32_t s = 57; s >= 0; s -= 3) {
        ATTRIBUTE_ASSUME(y <= 1321122u);
        y *= 2;
        ATTRIBUTE_ASSUME(y <= 2642244u);
        uint64_t bs = (3 * y * (y + 1) | 1) << s;
        if (n >= bs) {
            n -= bs;
            y++;
        }
    }
    ATTRIBUTE_ASSUME(y <= 2642245u);
    return uint32_t(y);
}

/// @brief Return integer part of the fourth root of n, that is ⌊n^0.25⌋
///         It can be shown that ⌊n^0.25⌋ = ⌊⌊n^0.5⌋^0.5⌋
/// @param[in] n
/// @return
ATTRIBUTE_CONST constexpr uint32_t ifrrt(uint64_t n) noexcept {
    return isqrt(isqrt(n));
}

#if defined(INTEGERS_128_BIT_HPP)

/// @brief Return integer part of the fourth root of n, that is ⌊n^0.25⌋
///         It can be shown that ⌊n^0.25⌋ = ⌊⌊n^0.5⌋^0.5⌋
/// @param[in] n
/// @return
ATTRIBUTE_CONST inline I128_CONSTEXPR uint32_t ifrrt(uint128_t n) noexcept {
    return isqrt(isqrt(n));
}

#endif

/// @brief Checks whether @a `n` is perfect square or not.
/// @param[in] n
/// @return `true` if @a `n` is perfect square and `false` otherwise.
ATTRIBUTE_CONST constexpr bool is_perfect_square(uint64_t n) noexcept {
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
    switch (n & 15) {
        case 0:
        case 1:
        case 4:
        case 9: {
            uint64_t root = isqrt(n);
            return root * root == n;
        }
        default:
            return false;
    }
}

/// @brief Checks whether @a `n` is perfect square or not.
///        If it is, stores square root of @a `n` into the @a `root`.
/// @param[in] n
/// @param[out] root
/// @return `true` if @a `n` is perfect square and `false` otherwise.
ATTRIBUTE_CONST constexpr bool is_perfect_square(uint64_t n, uint32_t& root) noexcept {
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
    switch (n & 15) {
        case 0:
        case 1:
        case 4:
        case 9: {
            uint64_t r = root = isqrt(n);
            return r * r == n;
        }
        default:
            return false;
    }
}

#if defined(INTEGERS_128_BIT_HPP)

/// @brief Checks whether @a `n` is perfect square or not.
/// @param[in] n
/// @return `true` if @a `n` is perfect square and `false` otherwise.
ATTRIBUTE_CONST inline I128_CONSTEXPR bool is_perfect_square(uint128_t n) noexcept {
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
    switch (uint64_t(n) & 15) {
        case 0:
        case 1:
        case 4:
        case 9: {
            uint64_t root = isqrt(n);
            return uint128_t(root) * root == n;
        }
        default:
            return false;
    }
}

/// @brief Checks whether @a `n` is perfect square or not.
///        If it is, stores square root of @a `n` into the @a `root`.
/// @param[in] n
/// @param[out] root
/// @return `true` if @a `n` is perfect square and `false` otherwise.
ATTRIBUTE_CONST inline I128_CONSTEXPR bool is_perfect_square(uint128_t n, uint64_t& root) noexcept {
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
    switch (uint64_t(n) & 15) {
        case 0:
        case 1:
        case 4:
        case 9: {
            root = isqrt(n);
            return uint128_t(root) * root == n;
        }
        default:
            return false;
    }
}

#endif

/// @brief This function reverses bits of the @a `b`
/// @param[in] b
/// @return 8-bit number whose bits are reversed bits of the @a `b`.
ATTRIBUTE_CONST constexpr uint8_t bit_reverse(uint8_t b) noexcept {
    // See
    // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    return uint8_t(((b * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32);
}

/// @brief This function reverses bits of the @a `n`
/// @param[in] b
/// @return 32-bit number whose bits are reversed bits of the @a `n`.
ATTRIBUTE_CONST constexpr uint32_t bit_reverse(uint32_t n) noexcept {
    // clang-format off
    /**
     * See Hackers Delight 7.1
     */
    n = ((n & 0x55555555u) << 1) | ((n >> 1) & 0x55555555u);
    n = ((n & 0x33333333u) << 2) | ((n >> 2) & 0x33333333u);
    n = ((n & 0x0F0F0F0Fu) << 4) | ((n >> 4) & 0x0F0F0F0Fu);
    // n = ((n & 0x00FF00FFu) << 8) | ((n >> 8) & 0x00FF00FFu);
    // n = ((n & 0x0000FFFFu) << 16) | ((n & 0xFFFF0000u) >> 16);
    n = (n << 24) | ((n & 0xFF00u) << 8) | ((n >> 8) & 0xFF00u) | (n >> 24);
    // clang-format on
    return n;
}

/// @brief This function reverses bits of the @a `n`
/// @param[in] b
/// @return 64-bit number whose bits are reversed bits of the @a `n`.
ATTRIBUTE_CONST constexpr uint64_t bit_reverse(uint64_t n) noexcept {
    // clang-format off
    /**
     * See Knuth's algorithm in Hackers Delight 7.4
     */
    uint64_t t = 0;
    n = (n << 31) | (n >> 33);  // I.e., shlr(x, 31).
    t = (n ^ (n >> 20)) & 0x00000FFF800007FFULL;
    n = (t | (t << 20)) ^ n;
    t = (n ^ (n >> 8)) & 0x00F8000F80700807ULL;
    n = (t | (t << 8)) ^ n;
    t = (n ^ (n >> 4)) & 0x0808708080807008ULL;
    n = (t | (t << 4)) ^ n;
    t = (n ^ (n >> 2)) & 0x1111111111111111ULL;
    n = (t | (t << 2)) ^ n;
    // clang-format on
    return n;
}

#if defined(INTEGERS_128_BIT_HPP)

/// @brief This function reverses bits of the @a `n`
/// @param[in] b
/// @return 128-bit number whose bits are reversed bits of the @a `n`.
ATTRIBUTE_CONST inline I128_CONSTEXPR uint128_t bit_reverse(uint128_t n) noexcept {
    uint128_t m = ~uint128_t(0);
    for (uint32_t s = sizeof(uint128_t) * CHAR_BIT; s >>= 1;) {
        m ^= m << s;
        n = ((n >> s) & m) | ((n << s) & ~m);
    }
    return n;
}

#endif

template <class Functor>
#if CONFIG_HAS_AT_LEAST_CXX_20
    requires requires(Functor f) { f(uint64_t()); }
#endif
constexpr void visit_all_submasks(uint64_t mask,
                                  Functor visiter) noexcept(noexcept(visiter(uint64_t()))) {
    uint64_t s = mask;
    do {
        visiter(s);
        s = (s - 1) & mask;
    } while (s != 0);
}

ATTRIBUTE_CONST constexpr int32_t sign(int x) noexcept {
    return int32_t(x > 0) - int32_t(x < 0);
}

ATTRIBUTE_CONST constexpr int32_t sign(long x) noexcept {
    return int32_t(x > 0) - int32_t(x < 0);
}

ATTRIBUTE_CONST constexpr int32_t sign(long long x) noexcept {
    return int32_t(x > 0) - int32_t(x < 0);
}

#if defined(INTEGERS_128_BIT_HPP)

ATTRIBUTE_CONST inline I128_CONSTEXPR int32_t sign(int128_t x) noexcept {
    uint32_t sign_bit = uint32_t(uint128_t(x) >> 127);
    return int32_t(x != 0) - int32_t(2 * sign_bit);
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
ATTRIBUTE_CONST constexpr bool same_sign(int a, int b) noexcept {
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
ATTRIBUTE_CONST constexpr bool same_sign(long a, long b) noexcept {
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
ATTRIBUTE_CONST constexpr bool same_sign(long long a, long long b) noexcept {
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
ATTRIBUTE_CONST constexpr bool same_sign_strict(int a, int b) noexcept {
    return sign(a) == sign(b);
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
ATTRIBUTE_CONST constexpr bool same_sign_strict(long a, long b) noexcept {
    return sign(a) == sign(b);
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
ATTRIBUTE_CONST constexpr bool same_sign_strict(long long a, long long b) noexcept {
    return sign(a) == sign(b);
}

ATTRIBUTE_CONST constexpr unsigned uabs(int n) noexcept {
    return n >= 0 ? static_cast<unsigned>(n) : -static_cast<unsigned>(n);
}

ATTRIBUTE_CONST constexpr unsigned long uabs(long n) noexcept {
    return n >= 0 ? static_cast<unsigned long>(n) : -static_cast<unsigned long>(n);
}

ATTRIBUTE_CONST constexpr unsigned long long uabs(long long n) noexcept {
    return n >= 0 ? static_cast<unsigned long long>(n) : -static_cast<unsigned long long>(n);
}

#if defined(INTEGERS_128_BIT_HPP)

ATTRIBUTE_CONST inline I128_CONSTEXPR uint128_t uabs(int128_t n) noexcept {
    uint128_t t = uint128_t(n >> 127);
    return (uint128_t(n) ^ t) - t;
}

#endif

namespace detail {

ATTRIBUTE_CONST inline uint32_t log2_floor_software(uint64_t n) {
    static const uint64_t t[6] = {0xFFFFFFFF00000000ull, 0x00000000FFFF0000ull,
                                  0x000000000000FF00ull, 0x00000000000000F0ull,
                                  0x000000000000000Cull, 0x0000000000000002ull};

    uint32_t y = 0;
    uint32_t j = 32;

    for (size_t i = 0; i != 6; ++i) {
        uint32_t k = (((n & t[i]) == 0) ? 0 : j);
        y += k;
        n >>= k;
        j >>= 1;
    }

    return y;
}

ATTRIBUTE_CONST inline uint32_t log2_ceil_software(uint64_t n) {
    return log2_floor_software(n) + ((n & (n - 1)) != 0);
}

/**
 *  Returns the integer (floor) log of the specified value, base 2.
 *  Note that by convention, input value 0 returns 0 since log(0) is
 * undefined.
 */
ATTRIBUTE_CONST inline uint32_t de_bruijn_log2(uint32_t value) {
    // See
    // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2

    static const unsigned char MultiplyDeBruijnBitPosition[32] = {
        0, 9,  1,  10, 13, 21, 2,  29, 11, 14, 16, 18, 22, 25, 3, 30,
        8, 12, 20, 28, 15, 17, 24, 7,  19, 27, 23, 6,  26, 5,  4, 31};

    // first round down to one less than a power of 2
    value |= (value >> 1);
    value |= (value >> 2);
    value |= (value >> 4);
    value |= (value >> 8);
    value |= (value >> 16);

    // Using de Bruijn sequence, k=2, n=5 (2^5=32) :
    // 0b_0000_0111_1100_0100_1010_1100_1101_1101
    return MultiplyDeBruijnBitPosition[((value * 0x07C4ACDDu) >> 27)];
}

ATTRIBUTE_CONST constexpr uint32_t lz_count_32_software(uint32_t n) noexcept {
    /**
     * See Hackers Delight Chapter 5
     */
    if (unlikely(n == 0)) {
        return 32;
    }
    uint32_t m = 1;
    if ((n >> 16) == 0) {
        m += 16;
        n <<= 16;
    }
    if ((n >> 24) == 0) {
        m += 8;
        n <<= 8;
    }
    if ((n >> 28) == 0) {
        m += 4;
        n <<= 4;
    }
    if ((n >> 30) == 0) {
        m += 2;
        n <<= 2;
    }
    m -= n >> 31;
    ATTRIBUTE_ASSUME(m <= 31);
    return m;
}

ATTRIBUTE_CONST constexpr uint32_t lz_count_64_software(uint64_t n) noexcept {
    /**
     * See Hackers Delight Chapter 5
     */
    if (unlikely(n == 0)) {
        return 64;
    }
    uint32_t m = 1;
    if ((n >> 32) == 0) {
        m += 32;
        n <<= 32;
    }
    if ((n >> 48) == 0) {
        m += 16;
        n <<= 16;
    }
    if ((n >> 56) == 0) {
        m += 8;
        n <<= 8;
    }
    if ((n >> 60) == 0) {
        m += 4;
        n <<= 4;
    }
    if ((n >> 62) == 0) {
        m += 2;
        n <<= 2;
    }
    m -= uint32_t(n >> 63);
    ATTRIBUTE_ASSUME(m <= 63);
    return m;
}

ATTRIBUTE_CONST constexpr uint32_t tz_count_32_software(uint32_t n) noexcept {
    /**
     * See Hackers Delight Chapter 5
     */
    if (unlikely(n == 0)) {
        return 32;
    }
    uint32_t m = 1;
    if ((n & 0x0000FFFFu) == 0) {
        m += 16;
        n >>= 16;
    }
    if ((n & 0x000000FFu) == 0) {
        m += 8;
        n >>= 8;
    }
    if ((n & 0x0000000Fu) == 0) {
        m += 4;
        n >>= 4;
    }
    if ((n & 0x00000003u) == 0) {
        m += 2;
        n >>= 2;
    }
    m -= (n & 1);
    ATTRIBUTE_ASSUME(m <= 31);
    return m;
}

ATTRIBUTE_CONST constexpr uint32_t tz_count_64_software(uint64_t n) noexcept {
    uint32_t m = 0;
    for (n = ~n & (n - 1); n != 0; n >>= 1) {
        m++;
    }
    ATTRIBUTE_ASSUME(m <= 64);
    return m;
}

ATTRIBUTE_CONST constexpr uint32_t pop_count_32_software(uint32_t n) noexcept {
    /**
     * See Hackers Delight Chapter 5.
     */
    n = (n & 0x55555555) + ((n >> 1) & 0x55555555);
    n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
    n = (n & 0x0F0F0F0F) + ((n >> 4) & 0x0F0F0F0F);
    n = (n & 0x00FF00FF) + ((n >> 8) & 0x00FF00FF);
    n = (n & 0x0000FFFF) + ((n >> 16) & 0x0000FFFF);
    ATTRIBUTE_ASSUME(n <= 32);
    return n;
}

ATTRIBUTE_CONST constexpr uint64_t pop_count_64_software(uint64_t n) noexcept {
    /**
     * See Hackers Delight Chapter 5.
     */
    n = (n & 0x5555555555555555ull) + ((n >> 1) & 0x5555555555555555ull);
    n = (n & 0x3333333333333333ull) + ((n >> 2) & 0x3333333333333333ull);
    n = (n & 0x0F0F0F0F0F0F0F0Full) + ((n >> 4) & 0x0F0F0F0F0F0F0F0Full);
    n = (n & 0x00FF00FF00FF00FFull) + ((n >> 8) & 0x00FF00FF00FF00FFull);
    n = (n & 0x0000FFFF0000FFFFull) + ((n >> 16) & 0x0000FFFF0000FFFFull);
    n = (n & 0x00000000FFFFFFFFull) + ((n >> 32) & 0x00000000FFFFFFFFull);
    ATTRIBUTE_ASSUME(n <= 64);
    return n;
}

}  // namespace detail

ATTRIBUTE_CONST constexpr int32_t pop_diff(uint32_t x, uint32_t y) noexcept {
    /**
     * See Hackers Delight Chapter 5.
     */
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    y = ~y;
    y = y - ((y >> 1) & 0x55555555);
    y = (y & 0x33333333) + ((y >> 2) & 0x33333333);
    x = x + y;
    x = (x & 0x0F0F0F0F) + ((x >> 4) & 0x0F0F0F0F);
    x = x + (x >> 8);
    x = x + (x >> 16);
    return static_cast<int32_t>(x & 0x0000007F) - 32;
}

ATTRIBUTE_CONST constexpr int32_t pop_cmp(uint32_t x, uint32_t y) noexcept {
    /**
     * See Hackers Delight Chapter 5.
     */
    uint32_t n = x & ~y;  // Clear bits where
    uint32_t m = y & ~x;  // both bits are 1
    while (true) {
        if (n == 0)
            return static_cast<int32_t>(m | -m);
        if (m == 0)
            return 1;
        n &= n - 1;  // Clear one bit
        m &= m - 1;  // from each
    }
}

/// @brief Count trailing zeros for n
/// @param[in] n
/// @return trailing zeros count (sizeof(n) * 8 for n = 0)
template <typename T>
#if CONFIG_HAS_AT_LEAST_CXX_20
    requires std::is_unsigned_v<T>
#if defined(INTEGERS_128_BIT_HPP)
             || std::is_same_v<T, uint128_t>
#endif
#endif
ATTRIBUTE_CONST constexpr int32_t countr_zero(T n) noexcept {
    if (unlikely(n == 0)) {
        return sizeof(n) * 8;
    }

#if defined(INTEGERS_128_BIT_HPP)
    if constexpr (std::is_same_v<T, uint128_t>) {
        uint64_t low = static_cast<uint64_t>(n);
        if (low != 0) {
#if CONFIG_HAS_AT_LEAST_CXX_20
            return std::countr_zero(low);
#elif defined(__GNUC__)
            return __builtin_ctzll(low);
#else
            return static_cast<int32_t>(detail::tz_count_64_software(low));
#endif
        }

        uint64_t high = static_cast<uint64_t>(n >> 64);
        ATTRIBUTE_ASSUME(high != 0);
#if CONFIG_HAS_AT_LEAST_CXX_20
        int32_t high_trailing_zeros_count = std::countr_zero(high);
#elif defined(__GNUC__)
        int32_t high_trailing_zeros_count = __builtin_ctzll(high);
#else
        int32_t high_trailing_zeros_count =
            static_cast<int32_t>(detail::tz_count_64_software(high));
#endif
        return high_trailing_zeros_count + 64;
    } else
#endif

#if CONFIG_HAS_AT_LEAST_CXX_20
        return std::countr_zero(n);
#else
    if constexpr (std::is_same_v<T, unsigned long long>) {
#if defined(__GNUC__)
        return __builtin_ctzll(n);
#else
        return static_cast<int32_t>(detail::tz_count_64_software(n));
#endif
    } else if constexpr (std::is_same_v<T, unsigned long>) {
#if defined(__GNUC__)
        return __builtin_ctzl(n);
#else
        return static_cast<int32_t>(
            detail::tz_count_64_software(static_cast<unsigned long long>(n)));
#endif
    } else {
        static_assert(std::is_same_v<T, unsigned int> || std::is_same_v<T, unsigned short> ||
                          std::is_same_v<T, unsigned char>,
                      "Inappropriate integer type in countr_zero");
#if defined(__GNUC__)
        return __builtin_ctz(n);
#else
        return static_cast<int32_t>(detail::tz_count_32_software(n));
#endif
    }
#endif
}

/// @brief Count leading zeros for n
/// @param[in] n
/// @return leading zeros count (sizeof(n) * 8 for n = 0)
template <typename T>
#if CONFIG_HAS_AT_LEAST_CXX_20
    requires std::is_unsigned_v<T>
#if defined(INTEGERS_128_BIT_HPP)
             || std::is_same_v<T, uint128_t>
#endif
#endif
ATTRIBUTE_CONST constexpr int32_t countl_zero(T n) noexcept {
    if (unlikely(n == 0)) {
        return sizeof(n) * 8;
    }

#if defined(INTEGERS_128_BIT_HPP)
    if constexpr (std::is_same_v<T, uint128_t>) {
        uint64_t high = static_cast<uint64_t>(n >> 64);
        if (high != 0) {
            // Avoid recursive call to countl_zero<uint64_t>
#if CONFIG_HAS_AT_LEAST_CXX_20
            return std::countl_zero(high);
#elif defined(__GNUC__)
            return __builtin_clzll(high);
#else
            return static_cast<int32_t>(detail::lz_count_64_software(high));
#endif
        }

        uint64_t low = static_cast<uint64_t>(n);
        ATTRIBUTE_ASSUME(low != 0);
        // Avoid recursive call to countl_zero<uint64_t>
#if CONFIG_HAS_AT_LEAST_CXX_20
        return 64 + std::countl_zero(low);
#elif defined(__GNUC__)
        return 64 + __builtin_clzll(low);
#else
        return 64 + static_cast<int32_t>(detail::lz_count_64_software(low));
#endif
    } else
#endif

#if CONFIG_HAS_AT_LEAST_CXX_20
        return std::countl_zero(n);
#else
    if constexpr (std::is_same_v<T, unsigned long long>) {
#if defined(__GNUC__)
        return __builtin_clzll(n);
#else
        return static_cast<int32_t>(detail::lz_count_64_software(n));
#endif
    } else if constexpr (std::is_same_v<T, unsigned long>) {
#if defined(__GNUC__)
        return __builtin_clzl(n);
#else
        return static_cast<int32_t>(detail::lz_count_64_software(n));
#endif
    } else {
        static_assert(std::is_same_v<T, unsigned int> || std::is_same_v<T, unsigned short> ||
                          std::is_same_v<T, unsigned char>,
                      "Inappropriate integer type in countl_zero");
#if defined(__GNUC__)
        return __builtin_clz(n);
#else
        return static_cast<int32_t>(detail::lz_count_32_software(n));
#endif
    }
#endif
}

template <class T>
#if CONFIG_HAS_AT_LEAST_CXX_20
    requires std::is_unsigned_v<T>
#if defined(INTEGERS_128_BIT_HPP)
             || std::is_same_v<T, uint128_t>
#endif
#endif
ATTRIBUTE_CONST constexpr int32_t popcount(T n) noexcept {
#if defined(INTEGERS_128_BIT_HPP)
    if constexpr (std::is_same_v<T, uint128_t>) {
        uint64_t high = static_cast<uint64_t>(n >> 64);
        uint64_t low  = static_cast<uint64_t>(n);
#if CONFIG_HAS_AT_LEAST_CXX_20
        return std::popcount(high) + std::popcount(low);
#elif defined(__GNUC__)
        return __builtin_popcountll(high) + __builtin_popcountll(low);
#else
        return static_cast<int32_t>(detail::pop_count_64_software(high) +
                                    detail::pop_count_64_software(low));
#endif
    } else
#endif

#if CONFIG_HAS_AT_LEAST_CXX_20
        return std::popcount(n);
#else
    if constexpr (std::is_same_v<T, unsigned long long>) {
#if defined(__GNUC__)
        return __builtin_popcountll(n);
#else
        return static_cast<int32_t>(detail::pop_count_64_software(n));
#endif
    } else if constexpr (std::is_same_v<T, unsigned long>) {
#if defined(__GNUC__)
        return __builtin_popcountl(n);
#else
        return static_cast<int32_t>(detail::pop_count_64_software(n));
#endif
    } else {
        static_assert(std::is_same_v<T, unsigned int> || std::is_same_v<T, unsigned short> ||
                          std::is_same_v<T, unsigned char>,
                      "Inappropriate integer type in popcount");
#if defined(__GNUC__)
        return __builtin_popcount(n);
#else
        return static_cast<int32_t>(detail::pop_count_32_software(n));
#endif
    }
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
/// x = 0 => undefined behaviour (shift by 33 bits)
/// @param[in] x
/// @return
constexpr uint32_t next_n_bits_permutation(uint32_t x) noexcept {
    // See
    // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2

    // t gets x's least significant 0 bits set to 1
    uint32_t t = x | (x - 1);
    // Next set to 1 the most significant bit to change,
    // set to 0 the least significant ones, and add the necessary 1 bits.
    return (t + 1) | (((~t & -~t) - 1) >> (countr_zero(x) + 1));
}

ATTRIBUTE_CONST constexpr bool is_pow2(int n) noexcept {
    // Cast to unsigned to avoid potential overflow
    unsigned m = static_cast<unsigned>(n);
    // To check (m & (m - 1)) == 0 first is necessary
    return (m & (m - 1)) == 0 && n > 0;
}

ATTRIBUTE_CONST constexpr bool is_pow2(long n) noexcept {
    // Cast to unsigned to avoid potential overflow
    unsigned long m = static_cast<unsigned long>(n);
    // To check (m & (m - 1)) == 0 first is necessary
    return (m & (m - 1)) == 0 && n > 0;
}

ATTRIBUTE_CONST constexpr bool is_pow2(long long n) noexcept {
    // Cast to unsigned to avoid potential overflow
    unsigned long long m = static_cast<unsigned long long>(n);
    // To check (m & (m - 1)) == 0 first is necessary
    return (m & (m - 1)) == 0 && n > 0;
}

ATTRIBUTE_CONST constexpr bool is_pow2(unsigned int n) noexcept {
    return (n & (n - 1)) == 0 && n != 0;
}

ATTRIBUTE_CONST constexpr bool is_pow2(unsigned long n) noexcept {
    return (n & (n - 1)) == 0 && n != 0;
}

ATTRIBUTE_CONST constexpr bool is_pow2(unsigned long long n) noexcept {
    return (n & (n - 1)) == 0 && n != 0;
}

#if defined(INTEGERS_128_BIT_HPP)

ATTRIBUTE_CONST inline I128_CONSTEXPR bool is_pow2(int128_t n) noexcept {
    return (n & (n - 1)) == 0 && n > 0;
}

ATTRIBUTE_CONST inline I128_CONSTEXPR bool is_pow2(uint128_t n) noexcept {
    return (n & (n - 1)) == 0 && n != 0;
}

#endif

ATTRIBUTE_CONST constexpr uint64_t nearest_pow2_ge(uint32_t n) noexcept {
    constexpr uint32_t k = sizeof(uint32_t) * CHAR_BIT;
    return uint64_t(1ull) << (k - uint32_t(countl_zero(n | 1)) - ((n & (n - 1)) == 0));
}

ATTRIBUTE_CONST constexpr uint64_t nearest_pow2_ge(uint64_t n) noexcept {
    constexpr uint32_t k = sizeof(uint64_t) * CHAR_BIT;
    return uint64_t(1ull) << (k - uint32_t(countl_zero(n | 1)) - ((n & (n - 1)) == 0));
}

ATTRIBUTE_CONST constexpr uint32_t base_2_len(uint32_t n) noexcept {
    // " | 1" operation does not affect answer for all
    //  numbers except n = 0. For n = 0 answer is 1.
    return 32 - uint32_t(countl_zero(n | 1));
}

ATTRIBUTE_CONST constexpr uint32_t base_2_len(uint64_t n) noexcept {
    // " | 1" operation does not affect answer for all
    //  numbers except n = 0. For n = 0 answer is 1.
    return 64 - uint32_t(countl_zero(n | 1));
}

/// @brief Realization taken from the gcc libstdc++ __to_chars_len
/// @tparam T
/// @param[in] value
/// @param[in] base
/// @return
template <typename T>
#if CONFIG_HAS_AT_LEAST_CXX_20
    requires std::is_unsigned_v<T>
#if defined(INTEGERS_128_BIT_HPP)
             || std::is_same_v<T, uint128_t>
#endif
#endif
ATTRIBUTE_CONST constexpr uint32_t base_b_len(T value, uint8_t base = 10) noexcept {
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

/// @brief For n > 0 returns ⌊log_2(n)⌋. For n = 0 returns (uint32_t)-1
/// @param[in] n
/// @return
ATTRIBUTE_CONST constexpr uint32_t log2_floor(uint32_t n) noexcept {
    return 31 - uint32_t(countl_zero(n));
}

/// @brief For n > 0 returns ⌈log_2(n)⌉. For n = 0 returns (uint32_t)-1
/// @param[in] n
/// @return
ATTRIBUTE_CONST constexpr uint32_t log2_ceil(uint32_t n) noexcept {
    return log2_floor(n) + ((n & (n - 1)) != 0);
}

/// @brief For n > 0 returns ⌊log_2(n)⌋. For n = 0 returns (uint32_t)-1
/// @param[in] n
/// @return
ATTRIBUTE_CONST constexpr uint32_t log2_floor(uint64_t n) noexcept {
    return 63 - uint32_t(countl_zero(n));
}

/// @brief For n > 0 returns ⌈log_2(n)⌉. For n = 0 returns (uint32_t)-1
/// @param[in] n
/// @return
ATTRIBUTE_CONST constexpr uint32_t log2_ceil(uint64_t n) noexcept {
    return log2_floor(n) + ((n & (n - 1)) != 0);
}

#if defined(INTEGERS_128_BIT_HPP)
/// @brief For n > 0 returns ⌊log_2(n)⌋. For n = 0 returns (uint32_t)-1
/// @param[in] n
/// @return
ATTRIBUTE_CONST constexpr uint32_t log2_floor(uint128_t n) noexcept {
    uint64_t hi = uint64_t(n >> 64);
    return hi != 0 ? (127 - uint32_t(countl_zero(hi))) : (log2_floor(uint64_t(n)));
}

/// @brief For n > 0 returns ⌈log_2(n)⌉. For n = 0 returns (uint32_t)-1
/// @param[in] n
/// @return
ATTRIBUTE_CONST constexpr uint32_t log2_ceil(uint128_t n) noexcept {
    return log2_floor(n) + ((n & (n - 1)) != 0);
}

#endif

/// @brief For n > 0 returns ⌊log_10(n)⌋. For n = 0 returns (uint32_t)-1
/// @param[in] n
/// @return
ATTRIBUTE_CONST
#if defined(__cpp_constexpr) && __cpp_constexpr >= 202211L && defined(__GNUC__)
constexpr
#endif
    inline uint32_t
    log10_floor(uint32_t n) noexcept {
    /**
     * See Hackers Delight 11-4
     */
#if defined(__cpp_constexpr) && __cpp_constexpr >= 202211L && defined(__GNUC__)
    constexpr
#elif defined(__cpp_constinit) && __cpp_constinit >= 201907L
    constinit
#endif
        static const uint8_t table1[33] = {
            10, 9, 9, 8, 8, 8, 7, 7, 7, 6, 6, 6, 6, 5, 5, 5, 4,
            4,  4, 3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0,
        };
#if defined(__cpp_constexpr) && __cpp_constexpr >= 202211L && defined(__GNUC__)
    constexpr
#elif defined(__cpp_constinit) && __cpp_constinit >= 201907L
    constinit
#endif
        static const uint32_t table2[11] = {
            1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000, 0,
        };
    uint32_t digits = table1[countl_zero(n)];
    digits -= ((n - table2[digits]) >> 31);
    return digits;
}

/// @brief For @a `n` > 0 returns ⌊log_10(n)⌋. For @a `n` = 0 returns
/// (uint32_t)-1
/// @param[in] n
/// @return
ATTRIBUTE_CONST
#if defined(__cpp_constexpr) && __cpp_constexpr >= 202211L && defined(__GNUC__)
constexpr
#endif
    inline uint32_t
    log10_floor(uint64_t n) noexcept {
    /**
     * See Hackers Delight 11-4
     */
#if defined(__cpp_constexpr) && __cpp_constexpr >= 202211L && defined(__GNUC__)
    constexpr
#elif defined(__cpp_constinit) && __cpp_constinit >= 201907L
    constinit
#endif
        static const uint64_t table2[20] = {
            0ull,
            9ull,
            99ull,
            999ull,
            9999ull,
            99999ull,
            999999ull,
            9999999ull,
            99999999ull,
            999999999ull,
            9999999999ull,
            99999999999ull,
            999999999999ull,
            9999999999999ull,
            99999999999999ull,
            999999999999999ull,
            9999999999999999ull,
            99999999999999999ull,
            999999999999999999ull,
            9999999999999999999ull,
        };
    static_assert(countl_zero(uint64_t(0)) == 64, "countl_zero detail error");
    int32_t digits = (19 * (63 - int32_t(countl_zero(n)))) >> 6;
    ATTRIBUTE_ASSUME((-19 >> 6) <= digits && digits <= ((19 * 63) >> 6));
    digits += int32_t((table2[uint32_t(digits + 1)] - n) >> 63);
    return uint32_t(digits);
}

ATTRIBUTE_CONST
#if defined(__cpp_constexpr) && __cpp_constexpr >= 202211L && defined(__GNUC__)
constexpr
#endif
    inline uint32_t
    base_10_len(uint32_t n) noexcept {
    // log10_floor(0 | 1) = 0
    return log10_floor(n | 1) + 1;
}

ATTRIBUTE_CONST
#if defined(__cpp_constexpr) && __cpp_constexpr >= 202211L && defined(__GNUC__)
constexpr
#endif
    inline uint32_t
    base_10_len(uint64_t n) noexcept {
    // log10_floor(0 | 1) = 0
    return log10_floor(n | 1) + 1;
}

/// @brief Find q and r such n = q * (2 ^ r), q is odd if n != 0
/// @param[in] n
/// @return Pair of q and r
template <typename T>
#if CONFIG_HAS_AT_LEAST_CXX_20
    requires std::is_unsigned_v<T>
#if defined(INTEGERS_128_BIT_HPP)
             || std::is_same_v<T, uint128_t>
#endif
#endif
ATTRIBUTE_CONST constexpr std::pair<T, uint32_t> extract_pow2(T n) noexcept {
    uint32_t r = uint32_t(countr_zero(n));
    return {n >> r, r};
}

/// @brief Returns median of boolean variables x, y and z
/// @param x x
/// @param y y
/// @param z z
/// @return median of x, y and z
constexpr bool bool_median(bool x, bool y, bool z) noexcept {
    return (x | y) & (y | z) & (x | z);
}

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
#if CONFIG_HAS_AT_LEAST_CXX_20
    requires std::is_floating_point_v<FloatType>
#endif
ATTRIBUTE_CONST SumSinCos<FloatType> sum_of_sines_and_cosines(FloatType alpha, FloatType beta,
                                                              uint32_t n) noexcept {
    static_assert(std::is_floating_point_v<FloatType>, "Invalid type in sum_of_sines_and_cosines");
    const FloatType nf       = static_cast<FloatType>(n);
    const auto half_beta     = beta / 2;
    const auto half_beta_sin = std::sin(half_beta);
    if (unlikely(half_beta_sin == 0)) {
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
    // Make arg const and call sin and cos close to each
    //  other so that compiler can call sincos here.
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
ATTRIBUTE_CONST constexpr uint32_t max_number_of_prime_divisors(uint32_t n) noexcept {
    if (n >= 2 * 3 * 5 * 7 * 11) {
        if (n >= 2 * 3 * 5 * 7 * 11 * 13 * 17) {
            if (n >= 2 * 3 * 5 * 7 * 11 * 13 * 17 * 19) {
                if (n >= 2 * 3 * 5 * 7 * 11 * 13 * 17 * 19 * 23) {
                    return 9;
                } else {
                    return 8;
                }
            } else {
                return 7;
            }
        } else {
            if (n >= 2 * 3 * 5 * 7 * 11 * 13) {
                return 6;
            } else {
                return 5;
            }
        }
    } else {
        if (n >= 2 * 3 * 5) {
            if (n >= 2 * 3 * 5 * 7) {
                return 4;
            } else {
                return 3;
            }
        } else {
            if (n >= 2 * 3) {
                return 2;
            } else {
                return 1;
            }
        }
    }
}

}  // namespace detail

/// @brief
/// @param[in] n
/// @return vector of pairs { prime_div : power_of_prime_div },
///          sorted by prime_div.
inline std::vector<std::pair<uint32_t, uint32_t>> prime_divisors_to_vector(uint32_t n) {
    std::vector<std::pair<uint32_t, uint32_t>> divisors;
    divisors.reserve(detail::max_number_of_prime_divisors(n));
    if (n % 2 == 0 && n != 0) {
        // n = s * 2^pow_of_2, where s is odd
        auto [s, pow_of_2] = extract_pow2(n);
        n                  = s;
        divisors.emplace_back(uint32_t(2), pow_of_2);
    }

    for (uint32_t d = 3; d * d <= n; d += 2) {
        ATTRIBUTE_ASSUME(d != 0);
        if (n % d == 0) {
            uint32_t pow_of_d = 0;
            do {
                pow_of_d++;
                n /= d;
            } while (n % d == 0);
            divisors.emplace_back(d, pow_of_d);
        }
    }

    if (n != 1) {
        divisors.emplace_back(n, uint32_t(1));
    }

    return divisors;
}

/// @brief
/// @param[in] n
/// @return vector of pairs { prime_div : power_of_prime_div },
///          sorted by prime_div.
inline std::vector<std::pair<uint32_t, uint32_t>> prime_divisors_to_vector(uint64_t n) {
    std::vector<std::pair<uint32_t, uint32_t>> divisors;
    if (n % 2 == 0 && n != 0) {
        // n = s * 2^pow_of_2, where s is odd
        auto [s, pow_of_2] = extract_pow2(n);
        n                  = s;
        divisors.emplace_back(uint64_t(2), pow_of_2);
    }

    for (uint64_t d = 3; d * d <= n; d += 2) {
        ATTRIBUTE_ASSUME(d != 0);
        if (n % d == 0) {
            uint32_t pow_of_d = 0;
            do {
                pow_of_d++;
                n /= d;
            } while (n % d == 0);
            divisors.emplace_back(d, pow_of_d);
        }
    }

    if (n != 1) {
        divisors.emplace_back(n, uint32_t(1));
    }

    return divisors;
}

/// @brief
/// @param[in] n
/// @return
inline std::map<uint32_t, uint32_t> prime_divisors_to_map(uint32_t n) {
    std::map<uint32_t, uint32_t> divisors;

    if (n % 2 == 0 && n != 0) {
        // n = s * 2^pow_of_2, where s is odd
        auto [s, pow_of_2] = extract_pow2(n);
        n                  = s;
        divisors.emplace(uint32_t(2), pow_of_2);
    }

    for (uint32_t d = 3; d * d <= n; d += 2) {
        ATTRIBUTE_ASSUME(d != 0);
        if (n % d == 0) {
            uint32_t pow_of_d = 0;
            do {
                pow_of_d++;
                n /= d;
            } while (n % d == 0);
            divisors.emplace(d, pow_of_d);
        }
    }

    if (n != 1) {
        divisors.emplace(n, uint32_t(1));
    }

    return divisors;
}

/// @brief
/// @param[in] n
/// @param[out] divisors
inline void prime_divisors_to_map(uint32_t n, std::map<uint32_t, uint32_t>& divisors) {
    if (n % 2 == 0 && n != 0) {
        // n = s * 2^pow_of_2, where s is odd
        auto [s, pow_of_2] = extract_pow2(n);
        n                  = s;
        divisors[2] += pow_of_2;
    }

    for (uint32_t d = 3; d * d <= n; d += 2) {
        ATTRIBUTE_ASSUME(d != 0);
        if (n % d == 0) {
            uint32_t pow_of_d = 0;
            do {
                pow_of_d++;
                n /= d;
            } while (n % d == 0);
            divisors[d] += pow_of_d;
        }
    }

    if (n != 1) {
        divisors[n]++;
    }
}

inline void prime_divisors_to_map(uint64_t n, std::map<uint64_t, uint32_t>& divisors) {
    if (n % 2 == 0 && n != 0) {
        // n = s * 2^pow_of_2, where s is odd
        auto [s, pow_of_2] = extract_pow2(n);
        n                  = s;
        divisors[2] += pow_of_2;
    }

    for (uint64_t d = 3; d * d <= n; d += 2) {
        ATTRIBUTE_ASSUME(d != 0);
        if (n % d == 0) {
            uint32_t pow_of_d = 0;
            do {
                pow_of_d++;
                n /= d;
            } while (n % d == 0);
            divisors[d] += pow_of_d;
        }
    }

    if (n != 1) {
        divisors[n]++;
    }
}

inline std::map<uint64_t, uint32_t> prime_divisors_to_map(uint64_t n) {
    std::map<uint64_t, uint32_t> divisors;
    if (n % 2 == 0 && n != 0) {
        // n = s * 2^pow_of_2, where s is odd
        auto [s, pow_of_2] = extract_pow2(n);
        n                  = s;
        divisors.emplace(uint64_t(2), pow_of_2);
    }

    for (uint64_t d = 3; d * d <= n; d += 2) {
        ATTRIBUTE_ASSUME(d != 0);
        if (n % d == 0) {
            uint32_t pow_of_d = 0;
            do {
                pow_of_d++;
                n /= d;
            } while (n % d == 0);
            divisors.emplace(d, pow_of_d);
        }
    }

    if (n != 1) {
        divisors.emplace(n, uint32_t(1));
    }

    return divisors;
}

}  // namespace math_functions

#if defined(INTEGERS_128_BIT_HPP)

namespace std {

/// @brief Computes greaters common divisor of @a `a` and @a `b`
///         using Stein's algorithm (binary gcd). Here gcd(0, 0) = 0.
/// @param[in] a
/// @param[in] b
/// @return gcd(a, b)
ATTRIBUTE_CONST inline I128_CONSTEXPR uint128_t gcd(uint128_t a, uint128_t b) noexcept {
    if (unlikely(a == 0)) {
        return b;
    }
    if (unlikely(b == 0)) {
        return a;
    }

    uint32_t ra   = uint32_t(math_functions::countr_zero(a));
    uint32_t rb   = uint32_t(math_functions::countr_zero(b));
    uint32_t mult = std::min(ra, rb);
    a >>= ra;
    b >>= rb;
    while (true) {
        if (a < b) {
            uint128_t tmp = a;
            a             = b;
            b             = tmp;
        }

        a -= b;
        if (a == 0) {
            return b << mult;
        }

        a >>= math_functions::countr_zero(a);
    }
}

ATTRIBUTE_CONST inline I128_CONSTEXPR uint128_t gcd(uint64_t a, int128_t b) noexcept {
    uint128_t b0 = math_functions::uabs(b);
    if (unlikely(b0 == 0)) {
        return a;
    }

    // gcd(a, b) = gcd(a, b0) = gcd(b0, a % b0) = gcd(a1, b1)
    uint128_t a1 = b0;
    // b1 = a % b0
    uint64_t b1 = a < b0 ? a : a % uint64_t(b0);  // a < 2^64 => b1 < 2^64
    if (b1 == 0) {
        return a1;
    }
    // gcd(a1, b1) = gcd(b1, a1 % b1) = gcd(a2, b2)
    uint64_t a2 = b1;  // b1 < 2^64 => a2 < 2^64
    // b2 = a1 % b1
    // a1 = b0, b1 = a % b0 => b1 < a1
    uint64_t b2 = uint64_t(a1 % b1);  // b1 < 2^64 => b2 = a1 % b1 < 2^64
    return std::gcd(a2, b2);
}

}  // namespace std

#endif  // INTEGERS_128_BIT_HPP

#if MATH_FUNCTIONS_HPP_ENABLE_TARGET_OPTIONS
#if defined(__GNUC__)
#if !defined(__clang__)
#pragma GCC pop_options
#else
#pragma clang attribute pop
#endif  // !__clang__
#endif  // __GNUC__
#endif  // MATH_FUNCTIONS_HPP_ENABLE_TARGET_OPTIONS

#undef MATH_FUNCTIONS_HPP_ENABLE_TARGET_OPTIONS

#if defined(_MSC_VER)
#pragma warning(pop)
#endif  // _MSC_VER

#endif  // !MATH_FUNCTIONS_HPP
