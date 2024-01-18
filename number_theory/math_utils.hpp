#if !defined(MATH_UTILS_HPP)
#define MATH_UTILS_HPP 1

/**
 * Define to 1 to if you want to compile some functions
 * with a bit faster instruction (see description below)
 */
#define MATH_UTILS_HPP_ENABLE_TARGET_OPTIONS 0

#if MATH_UTILS_HPP_ENABLE_TARGET_OPTIONS
/**
 * From lzcnt: bsr -> lzcnt (used in leading zeros count)
 * Used in log2_floor, log2_ceiled
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
#endif  // MATH_UTILS_HPP_ENABLE_TARGET_OPTIONS

#include <algorithm>  // std::min
#include <climits>    // CHAR_BIT
#include <cstdint>  // std::int32_t, std::int64_t, std::size_t, std::uint32_t, std::uint64_t
#include <numeric>      // std::gcd
#include <type_traits>  // std::is_unsigned_v, std::is_same_v
#include <utility>      // std::pair

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
#endif

namespace math_utils {

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
GCC_ATTRIBUTE_CONST static constexpr uint32_t bin_pow_mod(
    uint32_t n, uint32_t p, uint32_t mod) noexcept {
    uint64_t res = 1;
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

static_assert(bin_pow_mod(uint32_t(7), uint32_t(483), uint32_t(1000000007u)) ==
                  263145387u,
              "bin_pow_mod");
static_assert(bin_pow_mod(uint32_t(289), uint32_t(-1), uint32_t(2146514599u)) ==
                  1349294778u,
              "bin_pow_mod");
static_assert(bin_pow_mod(uint32_t(2146526839u), uint32_t(578423432u),
                          uint32_t(2147483629u)) == 281853233u,
              "bin_pow_mod");

#if defined(INTEGERS_128_BIT_HPP)

/// @brief Calculate (n ^ p) % mod
/// @param n
/// @param p
/// @param mod
/// @return (n ^ p) % mod
GCC_ATTRIBUTE_CONST
I128_CONSTEXPR
static inline uint64_t bin_pow_mod(uint64_t n, uint64_t p,
                                   uint64_t mod) noexcept {
    uint64_t res = 1;
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

#if HAS_I128_CONSTEXPR
static_assert(bin_pow_mod(uint64_t(119999999927ull),
                          uint64_t(18446744073709515329ull),
                          uint64_t(100000000000000003ull)) ==
                  85847679703545452ull,
              "bin_pow_mod");
static_assert(bin_pow_mod(uint64_t(72057594037927843ull),
                          uint64_t(18446744073709515329ull),
                          uint64_t(1000000000000000003ull)) ==
                  404835689235904145ull,
              "bin_pow_mod");
static_assert(bin_pow_mod(uint64_t(999999999999999487ull),
                          uint64_t(18446744073709551557ull),
                          uint64_t(1000000000000000009ull)) ==
                  802735487082721113ull,
              "bin_pow_mod");
#endif

#endif

GCC_ATTRIBUTE_CONST static constexpr uint32_t isqrt(uint32_t n) noexcept {
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

static_assert(isqrt(0u) == 0, "isqrt");
static_assert(isqrt(1u) == 1, "isqrt");
static_assert(isqrt(4u) == 2, "isqrt");
static_assert(isqrt(9u) == 3, "isqrt");
static_assert(isqrt(10u) == 3, "isqrt");
static_assert(isqrt(15u) == 3, "isqrt");
static_assert(isqrt(16u) == 4, "isqrt");
static_assert(isqrt(257u * 257u) == 257, "isqrt");
static_assert(isqrt(257u * 257u + 1) == 257, "isqrt");
static_assert(isqrt(258u * 258u - 1u) == 257, "isqrt");
static_assert(isqrt(1u << 12) == 1 << 6, "isqrt");
static_assert(isqrt(1u << 14) == 1 << 7, "isqrt");
static_assert(isqrt(1u << 16) == 1 << 8, "isqrt");
static_assert(isqrt(1u << 28) == 1 << 14, "isqrt");
static_assert(isqrt(1u << 30) == 1 << 15, "isqrt");
static_assert(isqrt(uint32_t(-1)) == (1u << 16) - 1, "isqrt");

GCC_ATTRIBUTE_CONST static constexpr uint32_t isqrt(uint64_t n) noexcept {
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
    ATTRIBUTE_ASSUME(l - 1 <= 0xFFFFFFFFu);
    return uint32_t(l - 1);
}

static_assert(isqrt(uint64_t(0)) == 0, "isqrt");
static_assert(isqrt(uint64_t(1)) == 1, "isqrt");
static_assert(isqrt(uint64_t(4)) == 2, "isqrt");
static_assert(isqrt(uint64_t(9)) == 3, "isqrt");
static_assert(isqrt(uint64_t(10)) == 3, "isqrt");
static_assert(isqrt(uint64_t(15)) == 3, "isqrt");
static_assert(isqrt(uint64_t(16)) == 4, "isqrt");
static_assert(isqrt(uint64_t(257 * 257)) == 257, "isqrt");
static_assert(isqrt(uint64_t(257 * 257 + 1)) == 257, "isqrt");
static_assert(isqrt(uint64_t(258 * 258 - 1)) == 257, "isqrt");
static_assert(isqrt(uint64_t(1 << 12)) == 1 << 6, "isqrt");
static_assert(isqrt(uint64_t(1 << 14)) == 1 << 7, "isqrt");
static_assert(isqrt(uint64_t(1 << 16)) == 1 << 8, "isqrt");
static_assert(isqrt(uint64_t(1 << 28)) == 1 << 14, "isqrt");
static_assert(isqrt(uint64_t(1 << 30)) == 1 << 15, "isqrt");
static_assert(isqrt(uint64_t(1) << 54) == uint64_t(1) << 27, "isqrt");
static_assert(isqrt(uint64_t(1) << 56) == uint64_t(1) << 28, "isqrt");
static_assert(isqrt(uint64_t(1) << 58) == uint64_t(1) << 29, "isqrt");
static_assert(isqrt(uint64_t(1) << 60) == uint64_t(1) << 30, "isqrt");
static_assert(isqrt(uint64_t(1) << 62) == uint64_t(1) << 31, "isqrt");
static_assert(isqrt(uint64_t(-1)) == 0xFFFFFFFFu, "isqrt");
static_assert(isqrt(uint64_t(1000000007) * 1000000007) == 1000000007u, "isqrt");

#if defined(INTEGERS_128_BIT_HPP)

GCC_ATTRIBUTE_CONST
I128_CONSTEXPR
static inline uint64_t isqrt(uint128_t n) noexcept {
    /**
     * See Hackers Delight Chapter 11.
     */
    uint64_t l = 0;
    uint128_t r_ = (n >> 6) + 16;
    uint64_t r = r_ > 0xFFFFFFFFFFFFFFFFull ? uint64_t(0xFFFFFFFFFFFFFFFFull)
                                            : uint64_t(r_);
    do {
        uint64_t m = uint64_t((uint128_t(l) + r + 1) >> 1);
        if (n >= uint128_t(m) * m) {
            l = m;
        } else {
            r = m - 1;
        }
    } while (r > l);
    return l;
}

#if HAS_I128_CONSTEXPR
static_assert(isqrt(uint128_t(0)) == 0, "isqrt");
static_assert(isqrt(uint128_t(1)) == 1, "isqrt");
static_assert(isqrt(uint128_t(4)) == 2, "isqrt");
static_assert(isqrt(uint128_t(9)) == 3, "isqrt");
static_assert(isqrt(uint128_t(10)) == 3, "isqrt");
static_assert(isqrt(uint128_t(15)) == 3, "isqrt");
static_assert(isqrt(uint128_t(16)) == 4, "isqrt");
static_assert(isqrt(uint128_t(257 * 257)) == 257, "isqrt");
static_assert(isqrt(uint128_t(257 * 257 + 1)) == 257, "isqrt");
static_assert(isqrt(uint128_t(258 * 258 - 1)) == 257, "isqrt");
static_assert(isqrt(uint128_t(1 << 12)) == 1 << 6, "isqrt");
static_assert(isqrt(uint128_t(1 << 14)) == 1 << 7, "isqrt");
static_assert(isqrt(uint128_t(1 << 16)) == 1 << 8, "isqrt");
static_assert(isqrt(uint128_t(1 << 28)) == 1 << 14, "isqrt");
static_assert(isqrt(uint128_t(1 << 30)) == 1 << 15, "isqrt");
static_assert(isqrt(uint128_t(1) << 54) == uint64_t(1) << 27, "isqrt");
static_assert(isqrt(uint128_t(1) << 56) == uint64_t(1) << 28, "isqrt");
static_assert(isqrt(uint128_t(1) << 58) == uint64_t(1) << 29, "isqrt");
static_assert(isqrt(uint128_t(1) << 60) == uint64_t(1) << 30, "isqrt");
static_assert(isqrt(uint128_t(1) << 62) == uint64_t(1) << 31, "isqrt");
static_assert(isqrt(uint128_t(uint64_t(-1))) == (uint64_t(1) << 32) - 1,
              "isqrt");
static_assert(isqrt(uint128_t(1) << 126) == uint64_t(1) << 63, "isqrt");
static_assert(isqrt(uint128_t(-1)) == (uint128_t(1) << 64) - 1, "isqrt");
static_assert(isqrt(uint128_t(1000000007) * 1000000007) == 1000000007, "isqrt");
static_assert(isqrt(uint128_t(1000000000000000003ull) *
                    1000000000000000003ull) == 1000000000000000003ull,
              "isqrt");
static_assert(isqrt(uint128_t(1000000000000000009ull) *
                    1000000000000000009ull) == 1000000000000000009ull,
              "isqrt");
static_assert(isqrt(uint128_t(18446744073709551521ull) *
                    18446744073709551521ull) == 18446744073709551521ull,
              "isqrt");
static_assert(isqrt(uint128_t(18446744073709551533ull) *
                    18446744073709551533ull) == 18446744073709551533ull,
              "isqrt");
static_assert(isqrt(uint128_t(18446744073709551557ull) *
                    18446744073709551557ull) == 18446744073709551557ull,
              "isqrt");
static_assert(isqrt(uint128_t(18446744073709551557ull) *
                        18446744073709551557ull +
                    1) == 18446744073709551557ull,
              "isqrt");
static_assert(isqrt(uint128_t(18446744073709551558ull) *
                        18446744073709551558ull -
                    1) == 18446744073709551557ull,
              "isqrt");
static_assert(isqrt(uint128_t(18446744073709551558ull) *
                    18446744073709551558ull) == 18446744073709551558ull,
              "isqrt");
#endif

#endif

GCC_ATTRIBUTE_CONST static constexpr uint32_t icbrt(uint32_t n) noexcept {
    /**
     * See Hackers Delight Chapter 11.
     */
    uint32_t y = 0;
    for (int32_t s = 30; s >= 0; s -= 3) {
        y <<= 1;
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

static_assert(icbrt(0u) == 0, "icbrt");
static_assert(icbrt(1u) == 1, "icbrt");
static_assert(icbrt(8u) == 2, "icbrt");
static_assert(icbrt(27u) == 3, "icbrt");
static_assert(icbrt(64u) == 4, "icbrt");
static_assert(icbrt(257u * 257u * 257u) == 257u, "icbrt");
static_assert(icbrt(257u * 257u * 257u + 1) == 257u, "icbrt");
static_assert(icbrt(258u * 258u * 258u - 1) == 257u, "icbrt");
static_assert(icbrt(258u * 258u * 258u) == 258u, "icbrt");
static_assert(icbrt(1u << 15) == 1u << 5, "icbrt");
static_assert(icbrt(1u << 18) == 1u << 6, "icbrt");
static_assert(icbrt(1u << 21) == 1u << 7, "icbrt");
static_assert(icbrt(1u << 24) == 1u << 8, "icbrt");
static_assert(icbrt(1u << 27) == 1u << 9, "icbrt");
static_assert(icbrt(1u << 30) == 1u << 10, "icbrt");
static_assert(icbrt(uint32_t(-1)) == 1625u, "icbrt");

GCC_ATTRIBUTE_CONST static constexpr uint64_t icbrt(uint64_t n) noexcept {
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
        y *= 2;
        uint64_t bs = (3 * y * (y + 1) | 1) << s;
        if (n >= bs) {
            n -= bs;
            y++;
        }
    }
    ATTRIBUTE_ASSUME(y <= 2642245u);
    return uint32_t(y);
}

static_assert(icbrt(uint64_t(0)) == 0, "icbrt");
static_assert(icbrt(uint64_t(1)) == 1, "icbrt");
static_assert(icbrt(uint64_t(8)) == 2, "icbrt");
static_assert(icbrt(uint64_t(27)) == 3, "icbrt");
static_assert(icbrt(uint64_t(64)) == 4, "icbrt");
static_assert(icbrt(uint64_t(65)) == 4, "icbrt");
static_assert(icbrt(uint64_t(124)) == 4, "icbrt");
static_assert(icbrt(uint64_t(125)) == 5, "icbrt");
static_assert(icbrt(uint64_t(289) * 289 * 289) == 289, "icbrt");
static_assert(icbrt(uint64_t(289) * 289 * 289 + 1) == 289, "icbrt");
static_assert(icbrt(uint64_t(290) * 290 * 290 - 1) == 289, "icbrt");
static_assert(icbrt(uint64_t(290) * 290 * 290) == 290, "icbrt");
static_assert(icbrt(uint64_t(1) << 30) == 1 << 10, "icbrt");
static_assert(icbrt(uint64_t(1) << 33) == 1 << 11, "icbrt");
static_assert(icbrt(uint64_t(1) << 36) == 1 << 12, "icbrt");
static_assert(icbrt(uint64_t(1) << 39) == 1 << 13, "icbrt");
static_assert(icbrt(uint64_t(1) << 42) == 1 << 14, "icbrt");
static_assert(icbrt(uint64_t(1) << 45) == 1 << 15, "icbrt");
static_assert(icbrt(uint64_t(1) << 48) == 1 << 16, "icbrt");
static_assert(icbrt(uint64_t(1) << 51) == 1 << 17, "icbrt");
static_assert(icbrt(uint64_t(1) << 54) == 1 << 18, "icbrt");
static_assert(icbrt(uint64_t(1) << 57) == 1 << 19, "icbrt");
static_assert(icbrt(uint64_t(1) << 60) == 1 << 20, "icbrt");
static_assert(icbrt(uint64_t(1) << 63) == 1 << 21, "icbrt");
static_assert(icbrt((uint64_t(1) << 63) | (uint64_t(1) << 32)) == 2097152,
              "icbrt");
static_assert(icbrt(uint64_t(1'367'631'000'000'000ull)) == 111'000, "icbrt");
static_assert(icbrt(uint64_t(1'000'000'000'000'000'000ull)) == 1'000'000,
              "icbrt");
static_assert(icbrt(uint64_t(1'331'000'000'000'000'000ull)) == 1'100'000,
              "icbrt");
static_assert(icbrt(uint64_t(8'000'000'000'000'000'000ull)) == 2'000'000,
              "icbrt");
static_assert(icbrt(uint64_t(15'625'000'000'000'000'000ull)) == 2'500'000,
              "icbrt");
static_assert(icbrt(uint64_t(-1)) == 2642245, "icbrt");

/// @brief Checks whether n is a perfect square or not
/// @param n
/// @return true if n is a perfect square and false otherwise
GCC_ATTRIBUTE_CONST static constexpr bool is_perfect_square(
    uint64_t n) noexcept {
    /**
     * +------------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     * |   n mod 16 |  0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 | 11
     * | 12 | 13 | 14 | 15 |
     * +------------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     * | n*n mod 16 |  0 |  1 |  4 |  9 |  0 |  9 |  4 |  1 |  0 |  1 |  4 |  9
     * |  0 |  9 |  4 |  1 |
     * +------------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     *
     * If we peek mod 32, then we should check only for n & 31 in { 0, 1, 4, 9,
     * 16, 17, 25 }, but switch statement could be less efficient in this case
     */
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
GCC_ATTRIBUTE_CONST static constexpr bool is_perfect_square(
    uint64_t n, uint32_t& root) noexcept {
    /**
     * +------------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     * |   n mod 16 |  0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 | 11
     * | 12 | 13 | 14 | 15 |
     * +------------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     * | n*n mod 16 |  0 |  1 |  4 |  9 |  0 |  9 |  4 |  1 |  0 |  1 |  4 |  9
     * |  0 |  9 |  4 |  1 |
     * +------------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     *
     * If we peek mod 32, then we should check only for n & 31 in { 0, 1, 4, 9,
     * 16, 17, 25 }, but switch statement could be less efficient in this case
     */
    switch (n & 15) {
        case 0:
        case 1:
        case 4:
        case 9: {
            root = isqrt(n);
            uint64_t r = root;
            return r * r == n;
        }
        default:
            return false;
    }
}

static_assert(is_perfect_square(uint64_t(0)), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(1)), "is_perfect_square");
static_assert(!is_perfect_square(uint64_t(2)), "is_perfect_square");
static_assert(!is_perfect_square(uint64_t(3)), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(4)), "is_perfect_square");
static_assert(!is_perfect_square(uint64_t(5)), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(9)), "is_perfect_square");
static_assert(!is_perfect_square(uint64_t(15)), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(16)), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(324)), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(1 << 16)), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(1 << 24)), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(1) << 32), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(1) << 40), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(1) << 48), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(1) << 56), "is_perfect_square");
static_assert(is_perfect_square(uint64_t(1) << 60), "is_perfect_square");

#if defined(INTEGERS_128_BIT_HPP)

/// @brief Checks whether n is a perfect square or not
/// @param n
/// @return true if n is a perfect square and false otherwise
GCC_ATTRIBUTE_CONST
I128_CONSTEXPR
static inline bool is_perfect_square(uint128_t n) noexcept {
    /**
     * +------------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     * |   n mod 16 |  0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 | 11
     * | 12 | 13 | 14 | 15 |
     * +------------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     * | n*n mod 16 |  0 |  1 |  4 |  9 |  0 |  9 |  4 |  1 |  0 |  1 |  4 |  9
     * |  0 |  9 |  4 |  1 |
     * +------------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     *
     * If we peek mod 32, then we should check only for n & 31 in { 0, 1, 4, 9,
     * 16, 17, 25 }, but switch statement could be less efficient in this case
     */
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
GCC_ATTRIBUTE_CONST
I128_CONSTEXPR
static inline bool is_perfect_square(uint128_t n, uint64_t& root) noexcept {
    /**
     * +------------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     * |   n mod 16 |  0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 | 11
     * | 12 | 13 | 14 | 15 |
     * +------------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     * | n*n mod 16 |  0 |  1 |  4 |  9 |  0 |  9 |  4 |  1 |  0 |  1 |  4 |  9
     * |  0 |  9 |  4 |  1 |
     * +------------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     *
     * If we peek mod 32, then we should check only for n & 31 in { 0, 1, 4, 9,
     * 16, 17, 25 }, but switch statement could be less efficient in this case
     */
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

#if HAS_I128_CONSTEXPR
static_assert(is_perfect_square(uint128_t(0)), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(1)), "is_perfect_square");
static_assert(!is_perfect_square(uint128_t(2)), "is_perfect_square");
static_assert(!is_perfect_square(uint128_t(3)), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(4)), "is_perfect_square");
static_assert(!is_perfect_square(uint128_t(5)), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(9)), "is_perfect_square");
static_assert(!is_perfect_square(uint128_t(15)), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(16)), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(324)), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(1 << 16)), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(1 << 24)), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(1) << 32), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(1) << 40), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(1) << 48), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(1) << 56), "is_perfect_square");
static_assert(is_perfect_square(uint128_t(1) << 60), "is_perfect_square");
#endif

#endif

GCC_ATTRIBUTE_CONST static constexpr uint8_t bit_reverse(uint8_t b) noexcept {
    // See https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    return uint8_t(((b * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >>
                   32);
}

static_assert(bit_reverse(uint8_t(0b00000000)) == 0b00000000, "bit_reverse");
static_assert(bit_reverse(uint8_t(0b00000010)) == 0b01000000, "bit_reverse");
static_assert(bit_reverse(uint8_t(0b00001100)) == 0b00110000, "bit_reverse");
static_assert(bit_reverse(uint8_t(0b10101010)) == 0b01010101, "bit_reverse");
static_assert(bit_reverse(uint8_t(0b01010101)) == 0b10101010, "bit_reverse");
static_assert(bit_reverse(uint8_t(0b11111111)) == 0b11111111, "bit_reverse");

GCC_ATTRIBUTE_CONST static constexpr uint32_t bit_reverse(uint32_t n) noexcept {
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

static_assert(bit_reverse(0b00000000'00000000'00000000'00000000u) ==
                  0b00000000'00000000'00000000'00000000u,
              "bit_reverse");
static_assert(bit_reverse(0b00000000'00000000'00000000'00000001u) ==
                  0b10000000'00000000'00000000'00000000u,
              "bit_reverse");
static_assert(bit_reverse(0b10000000'00000000'00000000'00000000u) ==
                  0b00000000'00000000'00000000'00000001u,
              "bit_reverse");
static_assert(bit_reverse(0b00000000'11111111'00000000'00000000u) ==
                  0b00000000'00000000'11111111'00000000u,
              "bit_reverse");
static_assert(bit_reverse(0b00000000'00000000'11111111'00000000u) ==
                  0b00000000'11111111'00000000'00000000u,
              "bit_reverse");
static_assert(bit_reverse(0b10101010'10101010'10101010'10101010u) ==
                  0b01010101'01010101'01010101'01010101u,
              "bit_reverse");
static_assert(bit_reverse(0b11111111'00000000'11111111'00000000u) ==
                  0b00000000'11111111'00000000'11111111u,
              "bit_reverse");

GCC_ATTRIBUTE_CONST static constexpr uint64_t bit_reverse(uint64_t n) noexcept {
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

static_assert(
    bit_reverse(uint64_t(
        0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL)) ==
        0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL,
    "bit_reverse");
static_assert(
    bit_reverse(uint64_t(
        0b10000001'00000000'10000001'00000000'10000001'00000000'10000001'00000000ULL)) ==
        0b00000000'10000001'00000000'10000001'00000000'10000001'00000000'10000001ULL,
    "bit_reverse");
static_assert(
    bit_reverse(uint64_t(
        0b00001111'00000000'11110000'00000000'10101010'00000000'00000000'00000000ULL)) ==
        0b00000000'00000000'00000000'01010101'00000000'00001111'00000000'11110000ULL,
    "bit_reverse");
static_assert(
    bit_reverse(uint64_t(
        0b00000000'00000000'00000000'10101010'10101010'00000000'00000000'00000000ULL)) ==
        0b00000000'00000000'00000000'01010101'01010101'00000000'00000000'00000000ULL,
    "bit_reverse");
static_assert(
    bit_reverse(uint64_t(
        0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL)) ==
        0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL,
    "bit_reverse");
static_assert(
    bit_reverse(uint64_t(
        0b11111111'00000000'11111111'00000000'11111111'00000000'11111111'00000000ULL)) ==
        0b00000000'11111111'00000000'11111111'00000000'11111111'00000000'11111111ULL,
    "bit_reverse");
static_assert(
    bit_reverse(uint64_t(
        0b11111111'11111111'11111111'11111111'00000000'00000000'00000000'00000000ULL)) ==
        0b00000000'00000000'00000000'00000000'11111111'11111111'11111111'11111111ULL,
    "bit_reverse");

#if defined(INTEGERS_128_BIT_HPP)

GCC_ATTRIBUTE_CONST I128_CONSTEXPR static uint128_t bit_reverse(
    uint128_t n) noexcept {
    uint128_t m = ~uint128_t(0);
    for (uint32_t s = sizeof(uint128_t) * CHAR_BIT; s >>= 1;) {
        m ^= m << s;
        n = ((n >> s) & m) | ((n << s) & ~m);
    }
    return n;
}

#if HAS_I128_CONSTEXPR

static_assert(bit_reverse(uint128_t(0)) == 0, "bit_reverse");
static_assert(bit_reverse(uint128_t(-1)) == uint128_t(-1), "bit_reverse");

#endif

#endif

GCC_ATTRIBUTE_CONST static constexpr uint32_t pop_count_software(
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

#if __cplusplus >= 202002L
static_assert(int(pop_count_software(0u)) == int(std::popcount(0u)),
              "pop_count_software");
static_assert(int(pop_count_software(1u)) == int(std::popcount(1u)),
              "pop_count_software");
static_assert(int(pop_count_software(2u)) == int(std::popcount(2u)),
              "pop_count_software");
static_assert(int(pop_count_software(3u)) == int(std::popcount(3u)),
              "pop_count_software");
static_assert(int(pop_count_software(4u)) == int(std::popcount(4u)),
              "pop_count_software");
static_assert(int(pop_count_software(0x4788743u)) ==
                  int(std::popcount(0x4788743u)),
              "pop_count_software");
static_assert(int(pop_count_software(0x2D425B23u)) ==
                  int(std::popcount(0x2D425B23u)),
              "pop_count_software");
static_assert(int(pop_count_software(0xFFFFFFFFu - 1)) ==
                  int(std::popcount(0xFFFFFFFFu - 1)),
              "pop_count_software");
static_assert(int(pop_count_software(0xFFFFFFFFu)) ==
                  int(std::popcount(0xFFFFFFFFu)),
              "pop_count_software");
#endif

GCC_ATTRIBUTE_CONST static constexpr uint64_t pop_count_software(
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

#if __cplusplus >= 202002L
static_assert(int(pop_count_software(uint64_t(0))) ==
                  int(std::popcount(uint64_t(0))),
              "pop_count_software");
static_assert(int(pop_count_software(uint64_t(1))) ==
                  int(std::popcount(uint64_t(1))),
              "pop_count_software");
static_assert(int(pop_count_software(uint64_t(2))) ==
                  int(std::popcount(uint64_t(2))),
              "pop_count_software");
static_assert(int(pop_count_software(uint64_t(3))) ==
                  int(std::popcount(uint64_t(3))),
              "pop_count_software");
static_assert(int(pop_count_software(uint64_t(4))) ==
                  int(std::popcount(uint64_t(4))),
              "pop_count_software");
static_assert(int(pop_count_software(uint64_t(0x4788743u))) ==
                  int(std::popcount(uint64_t(0x4788743u))),
              "pop_count_software");
static_assert(int(pop_count_software(uint64_t(0x2D425B23u))) ==
                  int(std::popcount(uint64_t(0x2D425B23u))),
              "pop_count_software");
static_assert(int(pop_count_software(uint64_t(0xFFFFFFFFu - 1))) ==
                  int(std::popcount(uint64_t(0xFFFFFFFFu - 1))),
              "pop_count_software");
static_assert(int(pop_count_software(uint64_t(0xFFFFFFFFu))) ==
                  int(std::popcount(uint64_t(0xFFFFFFFFu))),
              "pop_count_software");
static_assert(int(pop_count_software(uint64_t(0x5873485893484ull))) ==
                  int(std::popcount(uint64_t(0x5873485893484ull))),
              "pop_count_software");
static_assert(int(pop_count_software(uint64_t(0x85923489853245ull))) ==
                  int(std::popcount(uint64_t(0x85923489853245ull))),
              "pop_count_software");
static_assert(int(pop_count_software(uint64_t(0xFFFFFFFFFFFFFFFFull - 1))) ==
                  int(std::popcount(uint64_t(0xFFFFFFFFFFFFFFFFull - 1))),
              "pop_count_software");
static_assert(int(pop_count_software(uint64_t(0xFFFFFFFFFFFFFFFFull))) ==
                  int(std::popcount(uint64_t(0xFFFFFFFFFFFFFFFFull))),
              "pop_count_software");
#endif

GCC_ATTRIBUTE_CONST static constexpr int32_t pop_diff(uint32_t x,
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

#if __cplusplus >= 202002L
static_assert(std::popcount(0u) - std::popcount(0u) == pop_diff(0, 0));
static_assert(int(std::popcount(1u)) - int(std::popcount(0u)) ==
              pop_diff(1, 0));
static_assert(int(std::popcount(0u)) - int(std::popcount(1u)) ==
              pop_diff(0, 1));
static_assert(int(std::popcount(0xABCDEFu)) - int(std::popcount(4u)) ==
              pop_diff(0xABCDEF, 4));
static_assert(int(std::popcount(uint32_t(uint16_t(-1)))) -
                  int(std::popcount(314u)) ==
              pop_diff(uint16_t(-1), 314));
static_assert(int(std::popcount(uint32_t(-1))) - int(std::popcount(0u)) ==
              pop_diff(uint32_t(-1), 0));
static_assert(int(std::popcount(0u)) - int(std::popcount(uint32_t(-1))) ==
              pop_diff(0, uint32_t(-1)));
static_assert(int(std::popcount(uint32_t(-1))) -
                  int(std::popcount(uint32_t(-1))) ==
              pop_diff(uint32_t(-1), uint32_t(-1)));
#endif

GCC_ATTRIBUTE_CONST static constexpr int32_t sign(int x) noexcept {
    return int32_t(x > 0) - int32_t(x < 0);
}

GCC_ATTRIBUTE_CONST static constexpr int32_t sign(long x) noexcept {
    return int32_t(x > 0) - int32_t(x < 0);
}

GCC_ATTRIBUTE_CONST static constexpr int32_t sign(long long x) noexcept {
    return int32_t(x > 0) - int32_t(x < 0);
}

#if defined(INTEGERS_128_BIT_HPP)

GCC_ATTRIBUTE_CONST
I128_CONSTEXPR
static inline int32_t sign(int128_t x) noexcept {
    uint32_t sign_bit = uint32_t(uint128_t(x) >> 127);
    return int32_t(x != 0) - int32_t(2 * sign_bit);
}

#if HAS_I128_CONSTEXPR
static_assert(sign(int128_t(0)) == 0, "sign");
static_assert(sign(int128_t(1)) == 1, "sign");
static_assert(sign(int128_t(-1)) == -1, "sign");
static_assert(sign(int128_t(2)) == 1, "sign");
static_assert(sign(int128_t(-2)) == -1, "sign");
static_assert(sign(int128_t(18446744073709551615ull)) == 1, "sign");
static_assert(sign(-int128_t(18446744073709551615ull)) == -1, "sign");
static_assert(sign(int128_t(1) << 63) == 1, "sign");
static_assert(sign(-(int128_t(1) << 63)) == -1, "sign");
static_assert(sign(int128_t(1) << 126) == 1, "sign");
static_assert(sign(-(int128_t(1) << 126)) == -1, "sign");
static_assert(sign(int128_t((uint128_t(1) << 127) - 1)) == 1, "sign");
static_assert(sign(int128_t(-((uint128_t(1) << 127) - 1))) == -1, "sign");
static_assert(sign(int128_t(-(uint128_t(1) << 127))) == -1, "sign");
#endif

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
GCC_ATTRIBUTE_CONST static constexpr bool same_sign(int a, int b) noexcept {
    return (a ^ b) >= 0;
}

static_assert(same_sign(1, 1), "same_sign");
static_assert(same_sign(1, 0), "same_sign");
static_assert(!same_sign(1, -1), "same_sign");
static_assert(same_sign(0, 1), "same_sign");
static_assert(same_sign(0, 0), "same_sign");
static_assert(!same_sign(0, -1), "same_sign");
static_assert(!same_sign(-1, 1), "same_sign");
static_assert(!same_sign(-1, 0), "same_sign");
static_assert(same_sign(-1, -1), "same_sign");

/// @brief a >= 0 and b > 0 => true
///        a >= 0 and b = 0 => true
///        a >= 0 and b < 0 => false
///        a < 0 and b > 0 => false
///        a < 0 and b = 0 => false
///        a < 0 and b < 0 => true
/// @param a
/// @param b
/// @return
GCC_ATTRIBUTE_CONST static constexpr bool same_sign(long a, long b) noexcept {
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
GCC_ATTRIBUTE_CONST static constexpr bool same_sign(long long a,
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
GCC_ATTRIBUTE_CONST static constexpr bool same_sign_strict(int a,
                                                           int b) noexcept {
    return sign(a) == sign(b);
}

static_assert(same_sign_strict(1, 1), "same_sign_strict");
static_assert(!same_sign_strict(1, 0), "same_sign_strict");
static_assert(!same_sign_strict(1, -1), "same_sign_strict");
static_assert(!same_sign_strict(0, 1), "same_sign_strict");
static_assert(same_sign_strict(0, 0), "same_sign_strict");
static_assert(!same_sign_strict(0, -1), "same_sign_strict");
static_assert(!same_sign_strict(-1, 1), "same_sign_strict");
static_assert(!same_sign_strict(-1, 0), "same_sign_strict");
static_assert(same_sign_strict(-1, -1), "same_sign_strict");

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
GCC_ATTRIBUTE_CONST static constexpr bool same_sign_strict(long a,
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
GCC_ATTRIBUTE_CONST static constexpr bool same_sign_strict(
    long long a, long long b) noexcept {
    return sign(a) == sign(b);
}

GCC_ATTRIBUTE_CONST static constexpr uint32_t uabs(int n) noexcept {
    return n >= 0 ? static_cast<unsigned int>(n)
                  : -static_cast<unsigned int>(n);
}

GCC_ATTRIBUTE_CONST static constexpr unsigned long uabs(long n) noexcept {
    return n >= 0 ? static_cast<unsigned long>(n)
                  : -static_cast<unsigned long>(n);
}

GCC_ATTRIBUTE_CONST static constexpr unsigned long long uabs(
    long long n) noexcept {
    return n >= 0 ? static_cast<unsigned long long>(n)
                  : -static_cast<unsigned long long>(n);
}

#if defined(INTEGERS_128_BIT_HPP)

GCC_ATTRIBUTE_CONST
I128_CONSTEXPR
static inline uint128_t uabs(int128_t n) noexcept {
    uint128_t t = uint128_t(n >> 127);
    return (uint128_t(n) ^ t) - t;
}

#if HAS_I128_CONSTEXPR
static_assert(uabs(int128_t(0)) == 0, "uabs");
static_assert(uabs(int128_t(1)) == 1, "uabs");
static_assert(uabs(int128_t(-1)) == 1, "uabs");
static_assert(uabs(int128_t(4)) == 4, "uabs");
static_assert(uabs(int128_t(-4)) == 4, "uabs");
static_assert(uabs(int128_t(18446744073709551615ull)) ==
                  18446744073709551615ull,
              "uabs");
static_assert(uabs(-int128_t(18446744073709551615ull)) ==
                  18446744073709551615ull,
              "uabs");
static_assert(uabs(int128_t(1) << 126) == uint128_t(1) << 126, "uabs");
static_assert(uabs(-(int128_t(1) << 126)) == uint128_t(1) << 126, "uabs");
static_assert(uabs(int128_t((uint128_t(1) << 127) - 1)) ==
                  (uint128_t(1) << 127) - 1,
              "uabs");
static_assert(uabs(int128_t(-((uint128_t(1) << 127) - 1))) ==
                  (uint128_t(1) << 127) - 1,
              "uabs");
static_assert(uabs(int128_t(-(uint128_t(1) << 127))) == uint128_t(1) << 127,
              "uabs");
#endif

GCC_ATTRIBUTE_CONST static constexpr int32_t pop_cmp(uint32_t x,
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

#if __cplusplus >= 202002L

static_assert(sign(std::popcount(0u) - std::popcount(0u)) ==
                  sign(pop_cmp(0, 0)),
              "pop_cmp");
static_assert(sign(std::popcount(1u) - std::popcount(0u)) ==
                  sign(pop_cmp(1, 0)),
              "pop_cmp");
static_assert(sign(std::popcount(0u) - std::popcount(1u)) ==
                  sign(pop_cmp(0, 1)),
              "pop_cmp");
static_assert(sign(std::popcount(0xABCDEFu) - std::popcount(4u)) ==
                  pop_cmp(0xABCDEF, 4),
              "pop_cmp");
static_assert(sign(std::popcount(uint32_t(uint16_t(-1))) -
                   std::popcount(314u)) == sign(pop_cmp(uint16_t(-1), 314)),
              "pop_cmp");
static_assert(sign(std::popcount(uint32_t(-1)) - std::popcount(0u)) ==
                  sign(pop_cmp(uint32_t(-1), 0)),
              "pop_cmp");
static_assert(sign(std::popcount(0u) - std::popcount(uint32_t(-1))) ==
                  sign(pop_cmp(0, uint32_t(-1))),
              "pop_cmp");
static_assert(sign(std::popcount(uint32_t(-1)) - std::popcount(uint32_t(-1))) ==
                  sign(pop_cmp(uint32_t(-1), uint32_t(-1))),
              "pop_cmp");
#endif

#endif

GCC_ATTRIBUTE_CONST static constexpr uint32_t lz_count_32_software(
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

static_assert(lz_count_32_software(0) == 32, "lz_count_32_software");
static_assert(lz_count_32_software(1) == 31, "lz_count_32_software");
static_assert(lz_count_32_software(2) == 30, "lz_count_32_software");
static_assert(lz_count_32_software(4) == 29, "lz_count_32_software");
static_assert(lz_count_32_software(8) == 28, "lz_count_32_software");
static_assert(lz_count_32_software(12) == 28, "lz_count_32_software");
static_assert(lz_count_32_software(16) == 27, "lz_count_32_software");
static_assert(lz_count_32_software(32) == 26, "lz_count_32_software");
static_assert(lz_count_32_software(48) == 26, "lz_count_32_software");
static_assert(lz_count_32_software(uint32_t(1) << 30) == 1,
              "lz_count_32_software");
static_assert(lz_count_32_software(uint32_t(1) << 31) == 0,
              "lz_count_32_software");
static_assert(lz_count_32_software(~uint32_t(1)) == 0, "lz_count_32_software");

GCC_ATTRIBUTE_CONST static constexpr uint32_t lz_count_64_software(
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

static_assert(lz_count_64_software(0) == 64, "lz_count_64_software");
static_assert(lz_count_64_software(1) == 63, "lz_count_64_software");
static_assert(lz_count_64_software(2) == 62, "lz_count_64_software");
static_assert(lz_count_64_software(4) == 61, "lz_count_64_software");
static_assert(lz_count_64_software(8) == 60, "lz_count_64_software");
static_assert(lz_count_64_software(12) == 60, "lz_count_64_software");
static_assert(lz_count_64_software(16) == 59, "lz_count_64_software");
static_assert(lz_count_64_software(32) == 58, "lz_count_64_software");
static_assert(lz_count_64_software(48) == 58, "lz_count_64_software");
static_assert(lz_count_64_software(uint32_t(1) << 30) == 33,
              "lz_count_64_software");
static_assert(lz_count_64_software(uint32_t(1) << 31) == 32,
              "lz_count_64_software");
static_assert(lz_count_64_software(~uint32_t(1)) == 32, "lz_count_64_software");
static_assert(lz_count_64_software(uint64_t(1) << 62) == 1,
              "lz_count_64_software");
static_assert(lz_count_64_software(uint64_t(1) << 63) == 0,
              "lz_count_64_software");
static_assert(lz_count_64_software(uint64_t(-1)) == 0, "lz_count_64_software");

GCC_ATTRIBUTE_CONST static constexpr uint32_t tz_count_32_software(
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

static_assert(tz_count_32_software(0u) == 32, "tz_count_32_software");
static_assert(tz_count_32_software(1u) == 0, "tz_count_32_software");
static_assert(tz_count_32_software(2u) == 1, "tz_count_32_software");
static_assert(tz_count_32_software(4u) == 2, "tz_count_32_software");
static_assert(tz_count_32_software(8u) == 3, "tz_count_32_software");
static_assert(tz_count_32_software(12u) == 2, "tz_count_32_software");
static_assert(tz_count_32_software(16u) == 4, "tz_count_32_software");
static_assert(tz_count_32_software(32u) == 5, "tz_count_32_software");
static_assert(tz_count_32_software(48u) == 4, "tz_count_32_software");
static_assert(tz_count_32_software(1u << 30) == 30, "tz_count_32_software");
static_assert(tz_count_32_software(1u << 31) == 31, "tz_count_32_software");
static_assert(tz_count_32_software(~1u) == 1, "tz_count_32_software");
static_assert(tz_count_32_software(uint32_t(-1)) == 0, "tz_count_32_software");

GCC_ATTRIBUTE_CONST static constexpr uint32_t tz_count_64_software(
    uint64_t n) noexcept {
    uint32_t m = 0;
    for (n = ~n & (n - 1); n != 0; n >>= 1) {
        m++;
    }
    return m;
}

static_assert(tz_count_64_software(0u) == 64, "tz_count_64_software");
static_assert(tz_count_64_software(1u) == 0, "tz_count_64_software");
static_assert(tz_count_64_software(2u) == 1, "tz_count_64_software");
static_assert(tz_count_64_software(4u) == 2, "tz_count_64_software");
static_assert(tz_count_64_software(8u) == 3, "tz_count_64_software");
static_assert(tz_count_64_software(12u) == 2, "tz_count_64_software");
static_assert(tz_count_64_software(16u) == 4, "tz_count_64_software");
static_assert(tz_count_64_software(32u) == 5, "tz_count_64_software");
static_assert(tz_count_64_software(48u) == 4, "tz_count_64_software");
static_assert(tz_count_64_software(1u << 30) == 30, "tz_count_64_software");
static_assert(tz_count_64_software(1u << 31) == 31, "tz_count_64_software");
static_assert(tz_count_64_software(~1u) == 1, "tz_count_64_software");
static_assert(tz_count_64_software(uint32_t(-1)) == 0, "tz_count_64_software");

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
GCC_ATTRIBUTE_CONST static constexpr int32_t count_trailing_zeros(
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
            return static_cast<int32_t>(tz_count_64_software(low));
#endif
        }

        uint64_t high = static_cast<uint64_t>(n >> 64);
#if defined(__GNUC__)
        return __builtin_ctzll(high) + 64;
#else
        return static_cast<int32_t>(tz_count_64_software(high)) + 64;
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
        return static_cast<int32_t>(tz_count_64_software(n));
#endif
    } else if constexpr (std::is_same_v<T, unsigned long>) {
#if defined(__GNUC__)
        return __builtin_ctzl(n);
#else
        return static_cast<int32_t>(
            tz_count_64_software(static_cast<unsigned long long>(n)));
#endif
    } else {
        static_assert(std::is_same_v<T, unsigned int> ||
                          std::is_same_v<T, unsigned short> ||
                          std::is_same_v<T, unsigned char>,
                      "error in count_trailing_zeros");
#if defined(__GNUC__)
        return __builtin_ctz(n);
#else
        return static_cast<int32_t>(tz_count_32_software(n));
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
GCC_ATTRIBUTE_CONST static constexpr inline int32_t count_leading_zeros(
    T n) noexcept {
    if (unlikely(n == 0)) {
        return sizeof(n) * 8;
    }

#if defined(INTEGERS_128_BIT_HPP)
    if constexpr (std::is_same_v<T, uint128_t>) {
        uint64_t hi = static_cast<uint64_t>(n >> 64);
        if (hi != 0) {
#if defined(__GNUC__)
            return __builtin_clzll(hi);
#else
            return static_cast<int32_t>(lz_count_64_software(hi));
#endif
        }

        uint64_t low = static_cast<uint64_t>(n);
#if defined(__GNUC__)
        return 64 + __builtin_clzll(low);
#else
        return 64 + static_cast<int32_t>(lz_count_64_software(low));
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
        return static_cast<int32_t>(lz_count_64_software(n));
#endif
    } else if constexpr (std::is_same_v<T, unsigned long>) {
#if defined(__GNUC__)
        return __builtin_clzl(n);
#else
        return static_cast<int32_t>(lz_count_64_software(n));
#endif
    } else {
        static_assert(std::is_same_v<T, unsigned int> ||
                          std::is_same_v<T, unsigned short> ||
                          std::is_same_v<T, unsigned char>,
                      "error in count_leading_zeros");
#if defined(__GNUC__)
        return __builtin_clz(n);
#else
        return static_cast<int32_t>(lz_count_32_software(n));
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

static_assert(next_n_bits_permutation(0b0010011) == 0b0010101,
              "next_n_bits_permutation");
static_assert(next_n_bits_permutation(0b0010101) == 0b0010110,
              "next_n_bits_permutation");
static_assert(next_n_bits_permutation(0b0010110) == 0b0011001,
              "next_n_bits_permutation");
static_assert(next_n_bits_permutation(0b0011001) == 0b0011010,
              "next_n_bits_permutation");
static_assert(next_n_bits_permutation(0b0011010) == 0b0011100,
              "next_n_bits_permutation");
static_assert(next_n_bits_permutation(0b0011100) == 0b0100011,
              "next_n_bits_permutation");
static_assert(next_n_bits_permutation(0b0100011) == 0b0100101,
              "next_n_bits_permutation");

static_assert(next_n_bits_permutation(0b01) == 0b10, "next_n_bits_permutation");

static_assert(next_n_bits_permutation(0b1111111) == 0b10111111,
              "next_n_bits_permutation");

GCC_ATTRIBUTE_CONST static constexpr bool is_pow2(int n) noexcept {
    // Cast to unsigned to avoid potential overflow
    unsigned m = static_cast<unsigned>(n);
    // To check (m & (m - 1)) == 0 first is necessary
    return (m & (m - 1)) == 0 && n > 0;
}

GCC_ATTRIBUTE_CONST static constexpr bool is_pow2(long n) noexcept {
    // Cast to unsigned to avoid potential overflow
    unsigned long m = static_cast<unsigned long>(n);
    // To check (m & (m - 1)) == 0 first is necessary
    return (m & (m - 1)) == 0 && n > 0;
}

GCC_ATTRIBUTE_CONST static constexpr bool is_pow2(long long n) noexcept {
    // Cast to unsigned to avoid potential overflow
    unsigned long long m = static_cast<unsigned long long>(n);
    // To check (m & (m - 1)) == 0 first is necessary
    return (m & (m - 1)) == 0 && n > 0;
}

GCC_ATTRIBUTE_CONST static constexpr bool is_pow2(unsigned int n) noexcept {
    return (n & (n - 1)) == 0 && n != 0;
}

GCC_ATTRIBUTE_CONST static constexpr bool is_pow2(unsigned long n) noexcept {
    return (n & (n - 1)) == 0 && n != 0;
}

GCC_ATTRIBUTE_CONST static constexpr bool is_pow2(
    unsigned long long n) noexcept {
    return (n & (n - 1)) == 0 && n != 0;
}

static_assert(!is_pow2(0ull), "is_pow2");
static_assert(is_pow2(1ull << 0), "is_pow2");
static_assert(is_pow2(1ull << 1), "is_pow2");
static_assert(is_pow2(1ull << 2), "is_pow2");
static_assert(is_pow2(1ull << 3), "is_pow2");
static_assert(is_pow2(1ull << 4), "is_pow2");
static_assert(is_pow2(1ull << 5), "is_pow2");
static_assert(is_pow2(1ull << 6), "is_pow2");
static_assert(is_pow2(1ull << 7), "is_pow2");
static_assert(is_pow2(1ull << 8), "is_pow2");
static_assert(is_pow2(1ull << 9), "is_pow2");
static_assert(is_pow2(1ull << 60), "is_pow2");
static_assert(is_pow2(1ull << 61), "is_pow2");
static_assert(is_pow2(1ull << 62), "is_pow2");
static_assert(is_pow2(1ull << 63), "is_pow2");

GCC_ATTRIBUTE_CONST static constexpr uint64_t nearest_pow2_ge(
    uint32_t n) noexcept {
    constexpr uint32_t k = sizeof(uint32_t) * CHAR_BIT;
    return uint64_t(1ull) << (k - uint32_t(count_leading_zeros(n | 1)) -
                              ((n & (n - 1)) == 0));
}

GCC_ATTRIBUTE_CONST static constexpr uint64_t nearest_pow2_ge(
    uint64_t n) noexcept {
    constexpr uint32_t k = sizeof(uint64_t) * CHAR_BIT;
    return uint64_t(1ull) << (k - uint32_t(count_leading_zeros(n | 1)) -
                              ((n & (n - 1)) == 0));
}

static_assert(nearest_pow2_ge(uint32_t(0u)) == 1u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1u)) == 1u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(2u)) == 2u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(3u)) == 4u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(4u)) == 4u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(5u)) == 8u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(6u)) == 8u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(7u)) == 8u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(8u)) == 8u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(16u)) == 16u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(17u)) == 32u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(18u)) == 32u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(19u)) == 32u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(20u)) == 32u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(0x7FFFFFFFu)) == 0x80000000u,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(0x80000000u)) == 0x80000000u,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(0x80000001u)) == 0x100000000ull,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(0xFFFFFFFFu)) == 0x100000000ull,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 0) == uint32_t(1) << 0,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 1) == uint32_t(1) << 1,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 2) == uint32_t(1) << 2,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 3) == uint32_t(1) << 3,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 4) == uint32_t(1) << 4,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 5) == uint32_t(1) << 5,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 6) == uint32_t(1) << 6,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 7) == uint32_t(1) << 7,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 8) == uint32_t(1) << 8,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 9) == uint32_t(1) << 9,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 10) == uint32_t(1) << 10,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 11) == uint32_t(1) << 11,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 12) == uint32_t(1) << 12,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 13) == uint32_t(1) << 13,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 14) == uint32_t(1) << 14,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 15) == uint32_t(1) << 15,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 16) == uint32_t(1) << 16,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 17) == uint32_t(1) << 17,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 18) == uint32_t(1) << 18,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 19) == uint32_t(1) << 19,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 20) == uint32_t(1) << 20,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 21) == uint32_t(1) << 21,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 22) == uint32_t(1) << 22,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 23) == uint32_t(1) << 23,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 24) == uint32_t(1) << 24,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 25) == uint32_t(1) << 25,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 26) == uint32_t(1) << 26,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 27) == uint32_t(1) << 27,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 28) == uint32_t(1) << 28,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 29) == uint32_t(1) << 29,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 30) == uint32_t(1) << 30,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint32_t(1) << 31) == uint32_t(1) << 31,
              "nearest_pow2_ge");

static_assert(nearest_pow2_ge(uint64_t(0u)) == 1u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1u)) == 1u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(2u)) == 2u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(3u)) == 4u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(4u)) == 4u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(5u)) == 8u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(6u)) == 8u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(7u)) == 8u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(8u)) == 8u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(16u)) == 16u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(17u)) == 32u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(18u)) == 32u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(19u)) == 32u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(20u)) == 32u, "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(0x7FFFFFFFu)) == 0x80000000u,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(0x80000000u)) == 0x80000000u,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(0x80000001u)) == 0x100000000ull,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(0xFFFFFFFFu)) == 0x100000000ull,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(0x7FFFFFFFFFFFFFFFull)) ==
                  0x8000000000000000ull,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(0x8000000000000000ull)) ==
                  0x8000000000000000ull,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 0) == uint64_t(1) << 0,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 1) == uint64_t(1) << 1,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 2) == uint64_t(1) << 2,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 3) == uint64_t(1) << 3,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 4) == uint64_t(1) << 4,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 5) == uint64_t(1) << 5,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 6) == uint64_t(1) << 6,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 7) == uint64_t(1) << 7,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 8) == uint64_t(1) << 8,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 9) == uint64_t(1) << 9,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 10) == uint64_t(1) << 10,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 11) == uint64_t(1) << 11,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 12) == uint64_t(1) << 12,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 13) == uint64_t(1) << 13,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 14) == uint64_t(1) << 14,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 15) == uint64_t(1) << 15,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 16) == uint64_t(1) << 16,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 17) == uint64_t(1) << 17,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 18) == uint64_t(1) << 18,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 19) == uint64_t(1) << 19,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 20) == uint64_t(1) << 20,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 21) == uint64_t(1) << 21,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 22) == uint64_t(1) << 22,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 23) == uint64_t(1) << 23,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 24) == uint64_t(1) << 24,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 25) == uint64_t(1) << 25,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 26) == uint64_t(1) << 26,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 27) == uint64_t(1) << 27,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 28) == uint64_t(1) << 28,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 29) == uint64_t(1) << 29,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 30) == uint64_t(1) << 30,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 31) == uint64_t(1) << 31,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 32) == uint64_t(1) << 32,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 33) == uint64_t(1) << 33,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 34) == uint64_t(1) << 34,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 35) == uint64_t(1) << 35,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 36) == uint64_t(1) << 36,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 37) == uint64_t(1) << 37,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 38) == uint64_t(1) << 38,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 39) == uint64_t(1) << 39,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 40) == uint64_t(1) << 40,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 41) == uint64_t(1) << 41,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 42) == uint64_t(1) << 42,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 43) == uint64_t(1) << 43,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 44) == uint64_t(1) << 44,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 45) == uint64_t(1) << 45,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 46) == uint64_t(1) << 46,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 47) == uint64_t(1) << 47,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 48) == uint64_t(1) << 48,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 49) == uint64_t(1) << 49,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 50) == uint64_t(1) << 50,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 51) == uint64_t(1) << 51,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 52) == uint64_t(1) << 52,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 53) == uint64_t(1) << 53,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 54) == uint64_t(1) << 54,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 55) == uint64_t(1) << 55,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 56) == uint64_t(1) << 56,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 57) == uint64_t(1) << 57,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 58) == uint64_t(1) << 58,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 59) == uint64_t(1) << 59,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 60) == uint64_t(1) << 60,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 61) == uint64_t(1) << 61,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 62) == uint64_t(1) << 62,
              "nearest_pow2_ge");
static_assert(nearest_pow2_ge(uint64_t(1) << 63) == uint64_t(1) << 63,
              "nearest_pow2_ge");

/* Just constexpr version of isdigit from ctype.h */
GCC_ATTRIBUTE_CONST static constexpr bool is_digit(int32_t c) noexcept {
    return static_cast<uint32_t>(c) - '0' <= '9' - '0';
}

GCC_ATTRIBUTE_CONST static constexpr uint32_t base_2_digits(
    uint32_t n) noexcept {
    // " | 1" operation does not affect the answer for all numbers except n = 0
    // for n = 0 answer is 1
    return 32 - uint32_t(count_leading_zeros(n | 1));
}

GCC_ATTRIBUTE_CONST static constexpr uint32_t base_2_digits(
    uint64_t n) noexcept {
    // " | 1" operation does not affect the answer for all numbers except n = 0
    // for n = 0 answer is 1
    return 64 - uint32_t(count_leading_zeros(n | 1));
}

GCC_ATTRIBUTE_CONST
#if __cpp_constexpr >= 202211L && defined(__GNUC__)
constexpr
#endif
    static inline uint32_t
    base_10_digits(uint32_t n) noexcept {
#if __cpp_constexpr >= 202211L && defined(__GNUC__)
    constexpr
#endif
        static const uint8_t guess[33] = {0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3,
                                          3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6,
                                          6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9};
#if __cpp_constexpr >= 202211L && defined(__GNUC__)
    constexpr
#endif
        static const uint32_t ten_to_the[10] = {
            1,      10,      100,      1000,      10000,
            100000, 1000000, 10000000, 100000000, 1000000000,
        };
    uint32_t digits = guess[base_2_digits(n)];
    // returns 1 for n = 0. If you want to return 0 for n = 0, remove | 1
    return digits + ((n | 1) >= ten_to_the[digits]);
}

#if __cplusplus >= 202207L && __cpp_constexpr >= 202211L && defined(__GNUC__)
static_assert(base_10_digits(0u) == 1, "base_10_digits");
static_assert(base_10_digits(1u) == 1, "base_10_digits");
static_assert(base_10_digits(9u) == 1, "base_10_digits");
static_assert(base_10_digits(10u) == 2, "base_10_digits");
static_assert(base_10_digits(11u) == 2, "base_10_digits");
static_assert(base_10_digits(99u) == 2, "base_10_digits");
static_assert(base_10_digits(100u) == 3, "base_10_digits");
static_assert(base_10_digits(101u) == 3, "base_10_digits");
static_assert(base_10_digits(uint32_t(-1)) == 10, "base_10_digits");
#endif

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
GCC_ATTRIBUTE_CONST static constexpr uint32_t base_10_len(T value) noexcept {
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

#if __cplusplus >= 201703L
static_assert(base_10_len(0ull) == 1, "base_10_len");
static_assert(base_10_len(1ull) == 1, "base_10_len");
static_assert(base_10_len(9ull) == 1, "base_10_len");
static_assert(base_10_len(10ull) == 2, "base_10_len");
static_assert(base_10_len(11ull) == 2, "base_10_len");
static_assert(base_10_len(99ull) == 2, "base_10_len");
static_assert(base_10_len(100ull) == 3, "base_10_len");
static_assert(base_10_len(101ull) == 3, "base_10_len");
static_assert(base_10_len(uint64_t(-1)) == 20, "base_10_len");

#if defined(INTEGERS_128_BIT_HPP)
static_assert(base_10_len(uint128_t(0)) == 1, "base_10_len");
static_assert(base_10_len(uint128_t(1)) == 1, "base_10_len");
static_assert(base_10_len(uint128_t(9)) == 1, "base_10_len");
static_assert(base_10_len(uint128_t(10)) == 2, "base_10_len");
static_assert(base_10_len(uint128_t(11)) == 2, "base_10_len");
static_assert(base_10_len(uint128_t(99)) == 2, "base_10_len");
static_assert(base_10_len(uint128_t(100)) == 3, "base_10_len");
static_assert(base_10_len(uint128_t(101)) == 3, "base_10_len");
static_assert(base_10_len(uint128_t(-1)) == 39, "base_10_len");
#endif

#endif

GCC_ATTRIBUTE_CONST static constexpr uint32_t log2_floor(uint64_t n) noexcept {
    // " | 1" does not affect ans for all n >= 1.
    return 63 ^ uint32_t(count_leading_zeros(n | 1));
}

GCC_ATTRIBUTE_CONST static constexpr uint32_t log2_ceil(uint64_t n) noexcept {
    return log2_floor(n) + ((n & (n - 1)) != 0);
}

#if defined(INTEGERS_128_BIT_HPP)
GCC_ATTRIBUTE_CONST static constexpr uint32_t log2_floor(uint128_t n) noexcept {
    // " | 1" does not affect ans for all n >= 1.
    uint64_t hi = uint64_t(n >> 64);
    return hi != 0 ? (127 ^ uint32_t(count_leading_zeros(hi)))
                   : (63 ^ uint32_t(count_leading_zeros(uint64_t(n) | 1)));
}

GCC_ATTRIBUTE_CONST static constexpr uint32_t log2_ceil(uint128_t n) noexcept {
    return log2_floor(n) + ((n & (n - 1)) != 0);
}

#endif

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
GCC_ATTRIBUTE_CONST static constexpr std::pair<T, uint32_t> extract_2pow(
    T n) noexcept {
    uint32_t r = uint32_t(count_trailing_zeros(n));
    return {n >> r, r};
}

}  // namespace math_utils

#if defined(INTEGERS_128_BIT_HPP)

namespace std {

GCC_ATTRIBUTE_CONST
I128_CONSTEXPR
static uint128_t gcd(uint128_t a, uint128_t b) noexcept {
    if (unlikely(a == 0)) {
        return b;
    }
    if (unlikely(b == 0)) {
        return a;
    }

    uint32_t ra = uint32_t(math_utils::count_trailing_zeros(a));
    uint32_t rb = uint32_t(math_utils::count_trailing_zeros(b));
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

        a >>= math_utils::count_trailing_zeros(a);
    }
}

#if HAS_I128_CONSTEXPR
static_assert(gcd(uint128_t(1), uint128_t(1)) == 1, "gcd");
static_assert(gcd(uint128_t(3), uint128_t(7)) == 1, "gcd");
static_assert(gcd(uint128_t(0), uint128_t(112378432)) == 112378432, "gcd");
static_assert(gcd(uint128_t(112378432), uint128_t(0)) == 112378432, "gcd");
static_assert(gcd(uint128_t(429384832), uint128_t(324884)) == 4, "gcd");
static_assert(gcd(uint128_t(18446744073709551521ull),
                  uint128_t(18446744073709551533ull)) == 1,
              "gcd");
static_assert(gcd(uint128_t(18446744073709551521ull) * 18446744073709551521ull,
                  uint128_t(18446744073709551521ull)) ==
                  18446744073709551521ull,
              "gcd");
static_assert(gcd(uint128_t(23999993441ull) * 23999993377ull,
                  uint128_t(23999992931ull) * 23999539633ull) == 1,
              "gcd");
static_assert(gcd(uint128_t(2146514599u) * 2146514603u * 2146514611u,
                  uint128_t(2146514611u) * 2146514621u * 2146514647u) ==
                  2146514611ull,
              "gcd");
static_assert(gcd(uint128_t(2146514599u) * 2146514603u * 2146514611u * 2,
                  uint128_t(2146514599u) * 2146514603u * 2146514611u * 3) ==
                  uint128_t(2146514599u) * 2146514603u * 2146514611u,
              "gcd");
static_assert(gcd(uint128_t(100000000000000003ull) * 1000000000000000003ull,
                  uint128_t(1000000000000000003ull) * 1000000000000000009ull) ==
                  1000000000000000003ull,
              "gcd");
static_assert(gcd(uint128_t(3 * 2 * 5 * 7 * 11 * 13 * 17 * 19),
                  uint128_t(18446744073709551557ull) * 3) == 3,
              "gcd");
static_assert(gcd(uint128_t(1000000000000000009ull),
                  uint128_t(1000000000000000009ull) * 1000000000000000009ull) ==
                  1000000000000000009ull,
              "gcd");
static_assert(gcd(uint128_t(0),
                  uint128_t(1000000000000000009ull) * 1000000000000000009ull) ==
                  uint128_t(1000000000000000009ull) * 1000000000000000009ull,
              "gcd");
static_assert(gcd(uint128_t(18446744073709551557ull), uint128_t(0)) ==
                  18446744073709551557ull,
              "gcd");
#endif

GCC_ATTRIBUTE_CONST I128_CONSTEXPR static uint128_t gcd(uint64_t a,
                                                        int128_t b) noexcept {
    uint128_t b0 = math_utils::uabs(b);
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

#if HAS_I128_CONSTEXPR
static_assert(gcd(uint64_t(2), int128_t(4)) == 2, "gcd");
static_assert(gcd(uint64_t(2), int128_t(-4)) == 2, "gcd");
static_assert(gcd(uint64_t(3), int128_t(7)) == 1, "gcd");
static_assert(gcd(uint64_t(3), int128_t(-7)) == 1, "gcd");
static_assert(gcd(uint64_t(3), int128_t(18446744073709551557ull) * 3) == 3,
              "gcd");
static_assert(gcd(uint64_t(3), int128_t(18446744073709551557ull) * (-3)) == 3,
              "gcd");
static_assert(gcd(uint64_t(3) * 2 * 5 * 7 * 11 * 13 * 17 * 19,
                  int128_t(18446744073709551557ull) * 3) == 3,
              "gcd");
static_assert(gcd(uint64_t(1000000000000000009ull),
                  int128_t(1000000000000000009ll) * 1000000000000000009ll) ==
                  1000000000000000009ull,
              "gcd");
static_assert(gcd(uint64_t(0),
                  int128_t(1000000000000000009ll) * 1000000000000000009ll) ==
                  uint128_t(1000000000000000009ll) * 1000000000000000009ull,
              "gcd");
static_assert(gcd(uint64_t(18446744073709551557ull), int128_t(0)) ==
                  18446744073709551557ull,
              "gcd");
#endif

}  // namespace std

#endif

#if defined(_MSC_VER)
#pragma warning(pop)
#endif(_MSC_VER)

#if MATH_UTILS_HPP_ENABLE_TARGET_OPTIONS
#if defined(__GNUC__)
#if !defined(__clang__)
#pragma GCC pop_options
#else
#pragma clang attribute pop
#endif  // !__clang__
#endif  // __GNUC__
#endif  // MATH_UTILS_HPP_ENABLE_TARGET_OPTIONS

#undef MATH_UTILS_HPP_ENABLE_TARGET_OPTIONS

#endif  // !MATH_UTILS_HPP
