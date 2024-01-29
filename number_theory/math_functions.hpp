#if !defined(MATH_FUNCTIONS_HPP)
#define MATH_FUNCTIONS_HPP 1

/**
 * Define to 1 to if you want to compile some functions
 * with a bit faster instruction (see description below)
 */
#define MATH_FUNCTIONS_HPP_ENABLE_TARGET_OPTIONS 0

#if MATH_FUNCTIONS_HPP_ENABLE_TARGET_OPTIONS
/**
 * From lzcnt: bsr -> lzcnt (used in leading zeros count)
 * Used in log2_floor, log2_ceil
 *
 * From bmi: bsf -> tzcnt (used in trailing zeros count)
 * Used in extract_2pow
 */
#if defined(__GNUC__)
#if !defined(__clang__)
#pragma GCC push_options
#pragma GCC target("lzcnt,bmi")
#else
#pragma clang attribute push(__attribute__((target("lzcnt,bmi"))), \
                             apply_to = function)
#endif  // !__clang__
#endif  // __GNUC__
#endif  // MATH_FUNCTIONS_HPP_ENABLE_TARGET_OPTIONS

#include <algorithm>  // std::min
#include <climits>    // CHAR_BIT
#include <cstdint>  // std::int32_t, std::int64_t, std::size_t, std::uint32_t, std::uint64_t
#include <map>          // std::map
#include <numeric>      // std::gcd
#include <type_traits>  // std::is_unsigned_v, std::is_same_v
#include <utility>      // std::pair
#include <vector>       // std::vector

#if __cplusplus >= 202002L
#include <bit>  // std::popcount, std::countr_zero, std::countl_zero
#endif

#if defined(__has_include) && __has_include("integers_128_bit.hpp")
#include "integers_128_bit.hpp"
#endif

#include "config_macros.hpp"

// Visual C++ thinks that unary minus on unsigned is an error :clown:
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4146)
#endif  // _MSC_VER

namespace math_functions {

using std::int32_t;
using std::int64_t;
using std::size_t;
using std::uint32_t;
using std::uint64_t;

/// @brief Calculates T ^ p
/// @tparam T
/// @param n
/// @param p
/// @return T ^ p
template <class T>
static constexpr T bin_pow(T n, size_t p) noexcept {
    T res = 1u;
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

/// @brief Calculate (n ^ p) % mod
/// @param n
/// @param p
/// @param mod
/// @return (n ^ p) % mod
ATTRIBUTE_CONST static constexpr uint32_t bin_pow_mod(
    uint32_t n, uint32_t p, uint32_t mod) noexcept {
    uint64_t res = mod != 1;
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
/// @param n
/// @param p
/// @param mod
/// @return (n ^ p) % mod
ATTRIBUTE_CONST static inline I128_CONSTEXPR uint64_t
bin_pow_mod(uint64_t n, uint64_t p, uint64_t mod) noexcept {
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

ATTRIBUTE_CONST static constexpr uint32_t isqrt(uint32_t n) noexcept {
    /**
     * See Hackers Delight Chapter 11.
     */
    uint32_t y = 0;
    for (uint32_t m = 0x40000000; m != 0; m >>= 2) {
        uint32_t b = y | m;
        y >>= 1;
        if (n >= b) {
            n -= b;
            y |= m;
        }
    }
    ATTRIBUTE_ASSUME(y < (1u << 16));
    return y;
}

ATTRIBUTE_CONST static constexpr uint32_t isqrt(uint64_t n) noexcept {
    /**
     * See Hackers Delight Chapter 11.
     */
    uint64_t l = 1;
    uint64_t r = (n >> 5) + 8;
    if (r > 0xFFFFFFFFull) {
        r = 0xFFFFFFFFull;
    }
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

ATTRIBUTE_CONST static inline I128_CONSTEXPR uint64_t
isqrt(uint128_t n) noexcept {
    /**
     * See Hackers Delight Chapter 11.
     */
    uint64_t l = 0;
    uint128_t r_ = (n >> 6) + 16;
    uint64_t r = r_ > 0xFFFFFFFFFFFFFFFFull ? uint64_t(0xFFFFFFFFFFFFFFFFull)
                                            : uint64_t(r_);
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

ATTRIBUTE_CONST static constexpr uint32_t icbrt(uint32_t n) noexcept {
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

ATTRIBUTE_CONST static constexpr uint32_t icbrt(uint64_t n) noexcept {
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
/// @param n
/// @return
ATTRIBUTE_CONST static constexpr uint32_t ifrrt(uint64_t n) noexcept {
    return isqrt(isqrt(n));
}

/// @brief Return integer part of the fourth root of n, that is ⌊n^0.25⌋
///         It can be shown that ⌊n^0.25⌋ = ⌊⌊n^0.5⌋^0.5⌋
/// @param n
/// @return
ATTRIBUTE_CONST static constexpr uint32_t ifrrt(uint128_t n) noexcept {
    return isqrt(isqrt(n));
}

/// @brief Checks whether n is a perfect square or not
/// @param n
/// @return true if n is a perfect square and false otherwise
ATTRIBUTE_CONST static constexpr bool is_perfect_square(
    uint64_t n) noexcept {
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

/// @brief Checks whether n is a perfect square or not. If it is, stores it to
/// the root argument
/// @param n
/// @param root
/// @return true if n is a perfect square and false otherwise
ATTRIBUTE_CONST static constexpr bool is_perfect_square(
    uint64_t n, uint32_t& root) noexcept {
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

/// @brief Checks whether n is a perfect square or not
/// @param n
/// @return true if n is a perfect square and false otherwise
ATTRIBUTE_CONST static inline I128_CONSTEXPR bool is_perfect_square(
    uint128_t n) noexcept {
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

/// @brief Checks whether n is a perfect square or not. If it is, stores it to
/// the root argument
/// @param n
/// @param root
/// @return true if n is a perfect square and false otherwise
ATTRIBUTE_CONST static inline I128_CONSTEXPR bool is_perfect_square(
    uint128_t n, uint64_t& root) noexcept {
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

ATTRIBUTE_CONST static constexpr uint8_t bit_reverse(uint8_t b) noexcept {
    // See https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    return uint8_t(((b * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >>
                   32);
}

ATTRIBUTE_CONST static constexpr uint32_t bit_reverse(uint32_t n) noexcept {
    /**
     * See Hackers Delight 7.1
     */
    n = ((n & 0x55555555u) << 1) | ((n >> 1) & 0x55555555u);
    n = ((n & 0x33333333u) << 2) | ((n >> 2) & 0x33333333u);
    n = ((n & 0x0F0F0F0Fu) << 4) | ((n >> 4) & 0x0F0F0F0Fu);
    // n = ((n & 0x00FF00FFu) << 8) | ((n >> 8) & 0x00FF00FFu);
    // n = ((n & 0x0000FFFFu) << 16) | ((n & 0xFFFF0000u) >> 16);
    n = (n << 24) | ((n & 0xFF00u) << 8) | ((n >> 8) & 0xFF00u) | (n >> 24);
    return n;
}

ATTRIBUTE_CONST static constexpr uint64_t bit_reverse(uint64_t n) noexcept {
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
    return n;
}

#if defined(INTEGERS_128_BIT_HPP)

ATTRIBUTE_CONST static inline I128_CONSTEXPR uint128_t
bit_reverse(uint128_t n) noexcept {
    uint128_t m = ~uint128_t(0);
    for (uint32_t s = sizeof(uint128_t) * CHAR_BIT; s >>= 1;) {
        m ^= m << s;
        n = ((n >> s) & m) | ((n << s) & ~m);
    }
    return n;
}

#endif

namespace impl {

ATTRIBUTE_CONST static uint32_t log2_floor_software(uint64_t n) {
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

ATTRIBUTE_CONST static inline uint32_t log2_ceil_software(uint64_t n) {
    return log2_floor_software(n) + ((n & (n - 1)) != 0);
}

/**
 *  Returns the integer (floor) log of the specified value, base 2.
 *  Note that by convention, input value 0 returns 0 since log(0) is undefined.
 */
ATTRIBUTE_CONST static inline uint32_t de_bruijn_log2(uint32_t value) {
    // See https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2

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

/**
 * Taken from .NET C# library
 *  Returns the integer (floor) log of the specified value, base 2.
 *  Note that by convention, input value 0 returns 0 since log(0) is undefined.
 */
ATTRIBUTE_CONST static inline uint32_t de_bruijn_log2(uint64_t value) {
    uint32_t hi = static_cast<uint32_t>(value >> 32);
    return (hi != 0) ? (de_bruijn_log2(hi) + 32)
                     : de_bruijn_log2(static_cast<uint32_t>(value));
}

ATTRIBUTE_CONST static constexpr uint32_t pop_count_software(
    uint32_t n) noexcept {
    /**
     * See Hackers Delight Chapter 5.
     */
    n = (n & 0x55555555) + ((n >> 1) & 0x55555555);
    n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
    n = (n & 0x0F0F0F0F) + ((n >> 4) & 0x0F0F0F0F);
    n = (n & 0x00FF00FF) + ((n >> 8) & 0x00FF00FF);
    n = (n & 0x0000FFFF) + ((n >> 16) & 0x0000FFFF);
    return n;
}

ATTRIBUTE_CONST static constexpr uint64_t pop_count_software(
    uint64_t n) noexcept {
    /**
     * See Hackers Delight Chapter 5.
     */
    n = (n & 0x5555555555555555ull) + ((n >> 1) & 0x5555555555555555ull);
    n = (n & 0x3333333333333333ull) + ((n >> 2) & 0x3333333333333333ull);
    n = (n & 0x0F0F0F0F0F0F0F0Full) + ((n >> 4) & 0x0F0F0F0F0F0F0F0Full);
    n = (n & 0x00FF00FF00FF00FFull) + ((n >> 8) & 0x00FF00FF00FF00FFull);
    n = (n & 0x0000FFFF0000FFFFull) + ((n >> 16) & 0x0000FFFF0000FFFFull);
    n = (n & 0x00000000FFFFFFFFull) + ((n >> 32) & 0x00000000FFFFFFFFull);
    return n;
}

}  // namespace impl

ATTRIBUTE_CONST static constexpr int32_t pop_diff(uint32_t x,
                                                      uint32_t y) noexcept {
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

ATTRIBUTE_CONST static constexpr int32_t sign(int x) noexcept {
    return int32_t(x > 0) - int32_t(x < 0);
}

ATTRIBUTE_CONST static constexpr int32_t sign(long x) noexcept {
    return int32_t(x > 0) - int32_t(x < 0);
}

ATTRIBUTE_CONST static constexpr int32_t sign(long long x) noexcept {
    return int32_t(x > 0) - int32_t(x < 0);
}

#if defined(INTEGERS_128_BIT_HPP)

ATTRIBUTE_CONST static inline I128_CONSTEXPR int32_t
sign(int128_t x) noexcept {
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
/// @param a
/// @param b
/// @return
ATTRIBUTE_CONST static constexpr bool same_sign(int a, int b) noexcept {
    return (a ^ b) >= 0;
}

/// @brief a >= 0 and b > 0 => true
///        a >= 0 and b = 0 => true
///        a >= 0 and b < 0 => false
///        a < 0 and b > 0 => false
///        a < 0 and b = 0 => false
///        a < 0 and b < 0 => true
/// @param a
/// @param b
/// @return
ATTRIBUTE_CONST static constexpr bool same_sign(long a, long b) noexcept {
    return (a ^ b) >= 0;
}

/// @brief a >= 0 and b > 0 => true
///        a >= 0 and b = 0 => true
///        a >= 0 and b < 0 => false
///        a < 0 and b > 0 => false
///        a < 0 and b = 0 => false
///        a < 0 and b < 0 => true
/// @param a
/// @param b
/// @return
ATTRIBUTE_CONST static constexpr bool same_sign(long long a,
                                                    long long b) noexcept {
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
/// @param a
/// @param b
/// @return
ATTRIBUTE_CONST static constexpr bool same_sign_strict(int a,
                                                           int b) noexcept {
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
/// @param a
/// @param b
/// @return
ATTRIBUTE_CONST static constexpr bool same_sign_strict(long a,
                                                           long b) noexcept {
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
/// @param a
/// @param b
/// @return
ATTRIBUTE_CONST static constexpr bool same_sign_strict(
    long long a, long long b) noexcept {
    return sign(a) == sign(b);
}

ATTRIBUTE_CONST static constexpr unsigned uabs(int n) noexcept {
    return n >= 0 ? static_cast<unsigned>(n) : -static_cast<unsigned>(n);
}

ATTRIBUTE_CONST static constexpr unsigned long uabs(long n) noexcept {
    return n >= 0 ? static_cast<unsigned long>(n)
                  : -static_cast<unsigned long>(n);
}

ATTRIBUTE_CONST static constexpr unsigned long long uabs(
    long long n) noexcept {
    return n >= 0 ? static_cast<unsigned long long>(n)
                  : -static_cast<unsigned long long>(n);
}

#if defined(INTEGERS_128_BIT_HPP)

ATTRIBUTE_CONST static inline I128_CONSTEXPR uint128_t
uabs(int128_t n) noexcept {
    uint128_t t = uint128_t(n >> 127);
    return (uint128_t(n) ^ t) - t;
}

#endif

ATTRIBUTE_CONST static constexpr int32_t pop_cmp(uint32_t x,
                                                     uint32_t y) noexcept {
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

namespace impl {

ATTRIBUTE_CONST static constexpr uint32_t lz_count_32_software(
    uint32_t n) noexcept {
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
    return m;
}

ATTRIBUTE_CONST static constexpr uint32_t lz_count_64_software(
    uint64_t n) noexcept {
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
    return m;
}

ATTRIBUTE_CONST static constexpr uint32_t tz_count_32_software(
    uint32_t n) noexcept {
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
    return m - (n & 1);
}

ATTRIBUTE_CONST static constexpr uint32_t tz_count_64_software(
    uint64_t n) noexcept {
    uint32_t m = 0;
    for (n = ~n & (n - 1); n != 0; n >>= 1) {
        m++;
    }
    return m;
}

}  // namespace impl

/// @brief Count trailing zeros for n
/// @param n
/// @return trailing zeros count (sizeof(n) * 8 for n = 0)
template <typename T>
#if __cplusplus >= 202002L
    requires std::is_unsigned_v<T>
#if defined(INTEGERS_128_BIT_HPP)
             || std::is_same_v<T, uint128_t>
#endif
#endif
ATTRIBUTE_CONST static constexpr int32_t count_trailing_zeros(
    T n) noexcept {
    if (unlikely(n == 0)) {
        return sizeof(n) * 8;
    }

#if defined(INTEGERS_128_BIT_HPP)
    if constexpr (std::is_same_v<T, uint128_t>) {
        uint64_t low = static_cast<uint64_t>(n);
        if (low != 0) {
#if defined(__GNUC__)
            return __builtin_ctzll(low);
#else
            return static_cast<int32_t>(impl::tz_count_64_software(low));
#endif
        }

        uint64_t high = static_cast<uint64_t>(n >> 64);
        ATTRIBUTE_ASSUME(high != 0);
#if defined(__GNUC__)
        return __builtin_ctzll(high) + 64;
#else
        return static_cast<int32_t>(impl::tz_count_64_software(high)) + 64;
#endif
    } else
#endif

#if __cplusplus >= 202002L
        return std::countr_zero(n);
#else
    if constexpr (std::is_same_v<T, unsigned long long>) {
#if defined(__GNUC__)
        return __builtin_ctzll(n);
#else
        return static_cast<int32_t>(impl::tz_count_64_software(n));
#endif
    } else if constexpr (std::is_same_v<T, unsigned long>) {
#if defined(__GNUC__)
        return __builtin_ctzl(n);
#else
        return static_cast<int32_t>(
            impl::tz_count_64_software(static_cast<unsigned long long>(n)));
#endif
    } else {
        static_assert(std::is_same_v<T, unsigned int> ||
                          std::is_same_v<T, unsigned short> ||
                          std::is_same_v<T, unsigned char>,
                      "error in count_trailing_zeros");
#if defined(__GNUC__)
        return __builtin_ctz(n);
#else
        return static_cast<int32_t>(impl::tz_count_32_software(n));
#endif
    }
#endif
}

/// @brief Count leading zeros for n
/// @param n
/// @return leading zeros count (sizeof(n) * 8 for n = 0)
template <typename T>
#if __cplusplus >= 202002L
    requires std::is_unsigned_v<T>
#if defined(INTEGERS_128_BIT_HPP)
             || std::is_same_v<T, uint128_t>
#endif
#endif
ATTRIBUTE_CONST static constexpr int32_t count_leading_zeros(T n) noexcept {
    if (unlikely(n == 0)) {
        return sizeof(n) * 8;
    }

#if defined(INTEGERS_128_BIT_HPP)
    if constexpr (std::is_same_v<T, uint128_t>) {
        uint64_t hi = static_cast<uint64_t>(n >> 64);
        if (hi != 0) {
            // Avoid recursive call to count_leading_zeros<uint64_t>
#if __cplusplus >= 202002L
            return std::countl_zero(hi);
#elif defined(__GNUC__)
            return __builtin_clzll(hi);
#else
            return static_cast<int32_t>(impl::lz_count_64_software(hi));
#endif
        }

        uint64_t low = static_cast<uint64_t>(n);
        ATTRIBUTE_ASSUME(low != 0);
        // Avoid recursive call to count_leading_zeros<uint64_t>
#if __cplusplus >= 202002L
        return 64 + std::countl_zero(low);
#elif defined(__GNUC__)
        return 64 + __builtin_clzll(low);
#else
        return 64 + static_cast<int32_t>(impl::lz_count_64_software(low));
#endif
    } else
#endif

#if __cplusplus >= 202002L
        return std::countl_zero(n);
#else
    if constexpr (std::is_same_v<T, unsigned long long>) {
#if defined(__GNUC__)
        return __builtin_clzll(n);
#else
        return static_cast<int32_t>(impl::lz_count_64_software(n));
#endif
    } else if constexpr (std::is_same_v<T, unsigned long>) {
#if defined(__GNUC__)
        return __builtin_clzl(n);
#else
        return static_cast<int32_t>(impl::lz_count_64_software(n));
#endif
    } else {
        static_assert(std::is_same_v<T, unsigned int> ||
                          std::is_same_v<T, unsigned short> ||
                          std::is_same_v<T, unsigned char>,
                      "error in count_leading_zeros");
#if defined(__GNUC__)
        return __builtin_clz(n);
#else
        return static_cast<int32_t>(impl::lz_count_32_software(n));
#endif
    }
#endif
}

/// @brief 0b0010011 -> 0b0010101 -> 0b0010110 -> 0b0011001 -> 0b0011010 ->
/// 0b0011100 -> 0b0100011 -> etc.
/// x = 0 => undefined behaviour (shift by 33 bits)
/// @param x
/// @return
constexpr uint32_t next_n_bits_permutation(uint32_t x) noexcept {
    // See https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2

    // t gets x's least significant 0 bits set to 1
    uint32_t t = x | (x - 1);
    // Next set to 1 the most significant bit to change,
    // set to 0 the least significant ones, and add the necessary 1 bits.
    return (t + 1) | (((~t & -~t) - 1) >> (count_trailing_zeros(x) + 1));
}

ATTRIBUTE_CONST static constexpr bool is_pow2(int n) noexcept {
    // Cast to unsigned to avoid potential overflow
    unsigned m = static_cast<unsigned>(n);
    // To check (m & (m - 1)) == 0 first is necessary
    return (m & (m - 1)) == 0 && n > 0;
}

ATTRIBUTE_CONST static constexpr bool is_pow2(long n) noexcept {
    // Cast to unsigned to avoid potential overflow
    unsigned long m = static_cast<unsigned long>(n);
    // To check (m & (m - 1)) == 0 first is necessary
    return (m & (m - 1)) == 0 && n > 0;
}

ATTRIBUTE_CONST static constexpr bool is_pow2(long long n) noexcept {
    // Cast to unsigned to avoid potential overflow
    unsigned long long m = static_cast<unsigned long long>(n);
    // To check (m & (m - 1)) == 0 first is necessary
    return (m & (m - 1)) == 0 && n > 0;
}

ATTRIBUTE_CONST static constexpr bool is_pow2(unsigned int n) noexcept {
    return (n & (n - 1)) == 0 && n != 0;
}

ATTRIBUTE_CONST static constexpr bool is_pow2(unsigned long n) noexcept {
    return (n & (n - 1)) == 0 && n != 0;
}

ATTRIBUTE_CONST static constexpr bool is_pow2(
    unsigned long long n) noexcept {
    return (n & (n - 1)) == 0 && n != 0;
}

#if defined(INTEGERS_128_BIT_HPP)

ATTRIBUTE_CONST static inline I128_CONSTEXPR bool is_pow2(
    int128_t n) noexcept {
    return (n & (n - 1)) == 0 && n > 0;
}

ATTRIBUTE_CONST static inline I128_CONSTEXPR bool is_pow2(
    uint128_t n) noexcept {
    return (n & (n - 1)) == 0 && n != 0;
}

#endif

ATTRIBUTE_CONST static constexpr uint64_t nearest_pow2_ge(
    uint32_t n) noexcept {
    constexpr uint32_t k = sizeof(uint32_t) * CHAR_BIT;
    return uint64_t(1ull) << (k - uint32_t(count_leading_zeros(n | 1)) -
                              ((n & (n - 1)) == 0));
}

ATTRIBUTE_CONST static constexpr uint64_t nearest_pow2_ge(
    uint64_t n) noexcept {
    constexpr uint32_t k = sizeof(uint64_t) * CHAR_BIT;
    return uint64_t(1ull) << (k - uint32_t(count_leading_zeros(n | 1)) -
                              ((n & (n - 1)) == 0));
}

/* Just constexpr version of isdigit from ctype.h */
ATTRIBUTE_CONST static constexpr bool is_digit(int32_t c) noexcept {
    return static_cast<uint32_t>(c) - '0' <= '9' - '0';
}

ATTRIBUTE_CONST static constexpr uint32_t base_2_digits(
    uint32_t n) noexcept {
    // " | 1" operation does not affect the answer for all numbers except n = 0
    // for n = 0 answer is 1
    return 32 - uint32_t(count_leading_zeros(n | 1));
}

ATTRIBUTE_CONST static constexpr uint32_t base_2_digits(
    uint64_t n) noexcept {
    // " | 1" operation does not affect the answer for all numbers except n = 0
    // for n = 0 answer is 1
    return 64 - uint32_t(count_leading_zeros(n | 1));
}

/// @brief Realization taken from the gcc libstdc++ __to_chars_len
/// @tparam T
/// @param value
/// @return
template <typename T>
#if __cplusplus >= 202002L
    requires std::is_unsigned_v<T>
#if defined(INTEGERS_128_BIT_HPP)
             || std::is_same_v<T, uint128_t>
#endif
#endif
ATTRIBUTE_CONST static constexpr uint32_t base_10_len(T value) noexcept {
    const uint32_t base = 10;
    const uint32_t b2 = base * base;
    const uint32_t b3 = b2 * base;
    const uint32_t b4 = b3 * base;
    for (uint32_t n = 1;;) {
        if (value < base) {
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
/// @param n
/// @return
ATTRIBUTE_CONST static constexpr uint32_t log2_floor(uint32_t n) noexcept {
    return 31 - uint32_t(count_leading_zeros(n));
}

/// @brief For n > 0 returns ⌈log_2(n)⌉. For n = 0 returns (uint32_t)-1
/// @param n
/// @return
ATTRIBUTE_CONST static constexpr uint32_t log2_ceil(uint32_t n) noexcept {
    return log2_floor(n) + ((n & (n - 1)) != 0);
}

/// @brief For n > 0 returns ⌊log_2(n)⌋. For n = 0 returns (uint32_t)-1
/// @param n
/// @return
ATTRIBUTE_CONST static constexpr uint32_t log2_floor(uint64_t n) noexcept {
    return 63 - uint32_t(count_leading_zeros(n));
}

/// @brief For n > 0 returns ⌈log_2(n)⌉. For n = 0 returns (uint32_t)-1
/// @param n
/// @return
ATTRIBUTE_CONST static constexpr uint32_t log2_ceil(uint64_t n) noexcept {
    return log2_floor(n) + ((n & (n - 1)) != 0);
}

#if defined(INTEGERS_128_BIT_HPP)
/// @brief For n > 0 returns ⌊log_2(n)⌋. For n = 0 returns (uint32_t)-1
/// @param n
/// @return
ATTRIBUTE_CONST static constexpr uint32_t log2_floor(uint128_t n) noexcept {
    uint64_t hi = uint64_t(n >> 64);
    return hi != 0 ? (127 - uint32_t(count_leading_zeros(hi)))
                   : (log2_floor(uint64_t(n)));
}

/// @brief For n > 0 returns ⌈log_2(n)⌉. For n = 0 returns (uint32_t)-1
/// @param n
/// @return
ATTRIBUTE_CONST static constexpr uint32_t log2_ceil(uint128_t n) noexcept {
    return log2_floor(n) + ((n & (n - 1)) != 0);
}

#endif

/// @brief For n > 0 returns ⌈log_10(n)⌉. For n = 0 returns (uint32_t)-1
/// @param n
/// @return
ATTRIBUTE_CONST
#if __cpp_constexpr >= 202211L && defined(__GNUC__)
constexpr
#endif
    static inline uint32_t
    log10_floor(uint32_t n) noexcept {
    /**
     * See Hackers Delight 11-4
     */
#if __cpp_constexpr >= 202211L && defined(__GNUC__)
    constexpr
#endif
        static const uint8_t table1[33] = {10, 9, 9, 8, 8, 8, 7, 7, 7, 6, 6,
                                           6,  6, 5, 5, 5, 4, 4, 4, 3, 3, 3,
                                           3,  2, 2, 2, 1, 1, 1, 0, 0, 0, 0};
#if __cpp_constexpr >= 202211L && defined(__GNUC__)
    constexpr
#endif
        static const uint32_t table2[11] = {
            1,       10,       100,       1000,       10000, 100000,
            1000000, 10000000, 100000000, 1000000000, 0};
    uint32_t digits = table1[count_leading_zeros(n)];
    digits -= ((n - table2[digits]) >> 31);
    return digits;
}

ATTRIBUTE_CONST
#if __cpp_constexpr >= 202211L && defined(__GNUC__)
constexpr
#endif
    static inline uint32_t
    log10_floor(uint64_t n) noexcept {
    /**
     * See Hackers Delight 11-4
     */
#if __cpp_constexpr >= 202211L && defined(__GNUC__)
    constexpr
#endif
        static const uint64_t table2[20] = {0ull,
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
                                            9999999999999999999ull};
    int32_t digits = (19 * (63 - int32_t(count_leading_zeros(n)))) >> 6;
    digits += int32_t((table2[digits + 1] - n) >> 63);
    return uint32_t(digits);
}

ATTRIBUTE_CONST
#if __cpp_constexpr >= 202211L && defined(__GNUC__)
constexpr
#endif
    static inline uint32_t
    base_10_digits(uint32_t n) noexcept {
    // log10_floor(0 | 1) = 0
    return log10_floor(n | 1) + 1;
}

ATTRIBUTE_CONST
#if __cpp_constexpr >= 202211L && defined(__GNUC__)
constexpr
#endif
    static inline uint32_t
    base_10_digits(uint64_t n) noexcept {
    // log10_floor(0 | 1) = 0
    return log10_floor(n | 1) + 1;
}

/// @brief Find q and r such n = q * (2 ^ r), q is odd if n != 0
/// @param n n value.
/// @param r r value to find.
/// @return Pair of q and r
template <typename T>
#if __cplusplus >= 202002L
    requires std::is_unsigned_v<T>
#if defined(INTEGERS_128_BIT_HPP)
             || std::is_same_v<T, uint128_t>
#endif
#endif
ATTRIBUTE_CONST static constexpr std::pair<T, uint32_t> extract_2pow(
    T n) noexcept {
    uint32_t r = uint32_t(count_trailing_zeros(n));
    return {n >> r, r};
}

ATTRIBUTE_CONST
#if __cplusplus >= 202002L
constexpr
#endif
    static inline std::vector<std::pair<uint32_t, uint32_t>>
    prime_divisors_as_vector(uint32_t n) {
    std::vector<std::pair<uint32_t, uint32_t>> divisors;
    if (n % 2 == 0 && n != 0) {
        // k = s * 2^pow_of_2, where s is odd
        auto [s, pow_of_2] = extract_2pow(n);
        n = s;
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

ATTRIBUTE_CONST static inline std::map<uint32_t, uint32_t>
prime_divisors_as_map(uint32_t n) {
    std::map<uint32_t, uint32_t> divisors;
    if (n % 2 == 0 && n != 0) {
        // k = s * 2^pow_of_2, where s is odd
        auto [s, pow_of_2] = extract_2pow(n);
        n = s;
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

}  // namespace math_functions

#if defined(INTEGERS_128_BIT_HPP)

namespace std {

ATTRIBUTE_CONST static inline I128_CONSTEXPR uint128_t
gcd(uint128_t a, uint128_t b) noexcept {
    if (unlikely(a == 0)) {
        return b;
    }
    if (unlikely(b == 0)) {
        return a;
    }

    uint32_t ra = uint32_t(math_functions::count_trailing_zeros(a));
    uint32_t rb = uint32_t(math_functions::count_trailing_zeros(b));
    uint32_t mult = std::min(ra, rb);
    a >>= ra;
    b >>= rb;
    while (true) {
        if (a < b) {
            uint128_t tmp = a;
            a = b;
            b = tmp;
        }

        a -= b;
        if (a == 0) {
            return b << mult;
        }

        a >>= math_functions::count_trailing_zeros(a);
    }
}

ATTRIBUTE_CONST static inline I128_CONSTEXPR uint128_t
gcd(uint64_t a, int128_t b) noexcept {
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

#endif

#if defined(_MSC_VER)
#pragma warning(pop)
#endif  // _MSC_VER

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

#endif  // !MATH_FUNCTIONS_HPP
