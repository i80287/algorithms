#if !defined(MATH_UTILS_HPP)
#define MATH_UTILS_HPP 1

#if defined(__GNUC__)
// optionally for a bit faster log2 (lzcnt instead of bsr may be used)
// #pragma GCC target("lzcnt")
#endif

#include <cmath>
#include <cstdint>
#include <numeric>
#include <type_traits>

#if __cplusplus >= 202002L
#include <bit>
#endif

#if __has_include("integers_128_bit.hpp")
#include "integers_128_bit.hpp"
#endif

#if defined(__GNUC__)
#if defined(likely)
#undef likely
#endif
#define likely(x) __builtin_expect(static_cast<bool>(x), true)
#if defined(unlikely)
#undef unlikely
#endif
#define unlikely(x) __builtin_expect(static_cast<bool>(x), false)
#else
#if !defined(likely)
#define likely(x) static_cast<bool>(x)
#endif
#if !defined(unlikely)
#define unlikely(x) static_cast<bool>(x)
#endif
#endif

namespace math_utils {

/// @brief Calculates T ^ p
/// @tparam T
/// @param n
/// @param p
/// @return T ^ p
template <class T>
static constexpr T bin_pow(T n, size_t p) noexcept {
    T res(1u);
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
static constexpr uint32_t bin_pow_mod(uint32_t n, uint32_t p,
                                      uint32_t mod) noexcept {
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
              263145387u);
static_assert(bin_pow_mod(uint32_t(289), uint32_t(-1), uint32_t(2146514599u)) ==
              1349294778u);
static_assert(bin_pow_mod(uint32_t(2146526839u), uint32_t(578423432u),
                          uint32_t(2147483629u)) == 281853233u);

#if defined(INTEGERS_128_BIT)

/// @brief Calculate (n ^ p) % mod
/// @param n
/// @param p
/// @param mod
/// @return (n ^ p) % mod
#if __cplusplus >= 202002L && defined(__GNUC__)
constexpr
#else
inline
#endif
    static uint64_t
    bin_pow_mod(uint64_t n, uint64_t p, uint64_t mod) noexcept {
    uint64_t res = 1;
    while (true) {
        if (p & 1) {
            res = uint64_t((uint128_t(res) * n) % mod);
        }
        p >>= 1;
        if (p == 0) {
            return static_cast<uint64_t>(res);
        }
        n = uint64_t((uint128_t(n) * n) % mod);
    }
}

#if __cplusplus >= 202002L && defined(__GNUC__)
static_assert(bin_pow_mod(uint64_t(119999999927ull),
                          uint64_t(18446744073709515329ull),
                          uint64_t(100000000000000003ull)) ==
              85847679703545452ull);
static_assert(bin_pow_mod(uint64_t(72057594037927843ull),
                          uint64_t(18446744073709515329ull),
                          uint64_t(1000000000000000003ull)) ==
              404835689235904145ull);
static_assert(bin_pow_mod(uint64_t(999999999999999487ull),
                          uint64_t(18446744073709551557ull),
                          uint64_t(1000000000000000009ull)) ==
              802735487082721113ull);
#endif

#endif

static constexpr uint32_t isqrt(uint32_t n) noexcept {
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
    return y;
}

static_assert(isqrt(0u) == 0);
static_assert(isqrt(1u) == 1);
static_assert(isqrt(4u) == 2);
static_assert(isqrt(9u) == 3);
static_assert(isqrt(10u) == 3);
static_assert(isqrt(15u) == 3);
static_assert(isqrt(16u) == 4);
static_assert(isqrt(257u * 257u) == 257);
static_assert(isqrt(257u * 257u + 1) == 257);
static_assert(isqrt(258u * 258u - 1u) == 257);
static_assert(isqrt(1u << 12) == 1 << 6);
static_assert(isqrt(1u << 14) == 1 << 7);
static_assert(isqrt(1u << 16) == 1 << 8);
static_assert(isqrt(1u << 28) == 1 << 14);
static_assert(isqrt(1u << 30) == 1 << 15);
static_assert(isqrt(uint32_t(-1)) == (1 << 16) - 1);

static constexpr uint32_t isqrt(uint64_t n) noexcept {
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
    return uint32_t(l - 1);
}

static_assert(isqrt(uint64_t(0)) == 0);
static_assert(isqrt(uint64_t(1)) == 1);
static_assert(isqrt(uint64_t(4)) == 2);
static_assert(isqrt(uint64_t(9)) == 3);
static_assert(isqrt(uint64_t(10)) == 3);
static_assert(isqrt(uint64_t(15)) == 3);
static_assert(isqrt(uint64_t(16)) == 4);
static_assert(isqrt(uint64_t(257 * 257)) == 257);
static_assert(isqrt(uint64_t(257 * 257 + 1)) == 257);
static_assert(isqrt(uint64_t(258 * 258 - 1)) == 257);
static_assert(isqrt(uint64_t(1 << 12)) == 1 << 6);
static_assert(isqrt(uint64_t(1 << 14)) == 1 << 7);
static_assert(isqrt(uint64_t(1 << 16)) == 1 << 8);
static_assert(isqrt(uint64_t(1 << 28)) == 1 << 14);
static_assert(isqrt(uint64_t(1 << 30)) == 1 << 15);
static_assert(isqrt(uint64_t(1) << 54) == uint64_t(1) << 27);
static_assert(isqrt(uint64_t(1) << 56) == uint64_t(1) << 28);
static_assert(isqrt(uint64_t(1) << 58) == uint64_t(1) << 29);
static_assert(isqrt(uint64_t(1) << 60) == uint64_t(1) << 30);
static_assert(isqrt(uint64_t(1) << 62) == uint64_t(1) << 31);
static_assert(isqrt(uint64_t(-1)) == 0xFFFFFFFFu);
static_assert(isqrt(uint64_t(1000000007) * 1000000007) == 1000000007u);

#if defined(INTEGERS_128_BIT)

#if __cplusplus >= 202002L && defined(__GNUC__)
constexpr
#else
inline
#endif
    static uint64_t
    isqrt(uint128_t n) noexcept {
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

#if __cplusplus >= 202002L && defined(__GNUC__)
static_assert(isqrt(uint128_t(0)) == 0);
static_assert(isqrt(uint128_t(1)) == 1);
static_assert(isqrt(uint128_t(4)) == 2);
static_assert(isqrt(uint128_t(9)) == 3);
static_assert(isqrt(uint128_t(10)) == 3);
static_assert(isqrt(uint128_t(15)) == 3);
static_assert(isqrt(uint128_t(16)) == 4);
static_assert(isqrt(uint128_t(257 * 257)) == 257);
static_assert(isqrt(uint128_t(257 * 257 + 1)) == 257);
static_assert(isqrt(uint128_t(258 * 258 - 1)) == 257);
static_assert(isqrt(uint128_t(1 << 12)) == 1 << 6);
static_assert(isqrt(uint128_t(1 << 14)) == 1 << 7);
static_assert(isqrt(uint128_t(1 << 16)) == 1 << 8);
static_assert(isqrt(uint128_t(1 << 28)) == 1 << 14);
static_assert(isqrt(uint128_t(1 << 30)) == 1 << 15);
static_assert(isqrt(uint128_t(1) << 54) == uint64_t(1) << 27);
static_assert(isqrt(uint128_t(1) << 56) == uint64_t(1) << 28);
static_assert(isqrt(uint128_t(1) << 58) == uint64_t(1) << 29);
static_assert(isqrt(uint128_t(1) << 60) == uint64_t(1) << 30);
static_assert(isqrt(uint128_t(1) << 62) == uint64_t(1) << 31);
static_assert(isqrt(uint128_t(uint64_t(-1))) == (uint64_t(1) << 32) - 1);
static_assert(isqrt(uint128_t(1) << 126) == uint64_t(1) << 63);
static_assert(isqrt(uint128_t(-1)) == (uint128_t(1) << 64) - 1);
static_assert(isqrt(uint128_t(1000000007) * 1000000007) == 1000000007);
static_assert(isqrt(uint128_t(1000000000000000003ull) *
                    1000000000000000003ull) == 1000000000000000003ull);
static_assert(isqrt(uint128_t(1000000000000000009ull) *
                    1000000000000000009ull) == 1000000000000000009ull);
static_assert(isqrt(uint128_t(18446744073709551521ull) *
                    18446744073709551521ull) == 18446744073709551521ull);
static_assert(isqrt(uint128_t(18446744073709551533ull) *
                    18446744073709551533ull) == 18446744073709551533ull);
static_assert(isqrt(uint128_t(18446744073709551557ull) *
                    18446744073709551557ull) == 18446744073709551557ull);
static_assert(isqrt(uint128_t(18446744073709551557ull) *
                        18446744073709551557ull +
                    1) == 18446744073709551557ull);
static_assert(isqrt(uint128_t(18446744073709551558ull) *
                        18446744073709551558ull -
                    1) == 18446744073709551557ull);
static_assert(isqrt(uint128_t(18446744073709551558ull) *
                    18446744073709551558ull) == 18446744073709551558ull);
#endif

#endif

static constexpr uint32_t icbrt(uint32_t n) noexcept {
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
    return y;
}

static_assert(icbrt(0u) == 0);
static_assert(icbrt(1u) == 1);
static_assert(icbrt(8u) == 2);
static_assert(icbrt(27u) == 3);
static_assert(icbrt(64u) == 4);
static_assert(icbrt(257u * 257u * 257u) == 257u);
static_assert(icbrt(257u * 257u * 257u + 1) == 257u);
static_assert(icbrt(258u * 258u * 258u - 1) == 257u);
static_assert(icbrt(258u * 258u * 258u) == 258u);
static_assert(icbrt(1u << 15) == 1u << 5);
static_assert(icbrt(1u << 18) == 1u << 6);
static_assert(icbrt(1u << 21) == 1u << 7);
static_assert(icbrt(1u << 24) == 1u << 8);
static_assert(icbrt(1u << 27) == 1u << 9);
static_assert(icbrt(1u << 30) == 1u << 10);

static constexpr uint64_t icbrt(uint64_t n) noexcept {
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
    return uint32_t(y);
}

static_assert(icbrt(uint64_t(0)) == 0);
static_assert(icbrt(uint64_t(1)) == 1);
static_assert(icbrt(uint64_t(8)) == 2);
static_assert(icbrt(uint64_t(27)) == 3);
static_assert(icbrt(uint64_t(64)) == 4);
static_assert(icbrt(uint64_t(65)) == 4);
static_assert(icbrt(uint64_t(124)) == 4);
static_assert(icbrt(uint64_t(125)) == 5);
static_assert(icbrt(uint64_t(289) * 289 * 289) == 289);
static_assert(icbrt(uint64_t(289) * 289 * 289 + 1) == 289);
static_assert(icbrt(uint64_t(290) * 290 * 290 - 1) == 289);
static_assert(icbrt(uint64_t(290) * 290 * 290) == 290);
static_assert(icbrt(uint64_t(1) << 30) == 1 << 10);
static_assert(icbrt(uint64_t(1) << 33) == 1 << 11);
static_assert(icbrt(uint64_t(1) << 36) == 1 << 12);
static_assert(icbrt(uint64_t(1) << 39) == 1 << 13);
static_assert(icbrt(uint64_t(1) << 42) == 1 << 14);
static_assert(icbrt(uint64_t(1) << 45) == 1 << 15);
static_assert(icbrt(uint64_t(1) << 48) == 1 << 16);
static_assert(icbrt(uint64_t(1) << 51) == 1 << 17);
static_assert(icbrt(uint64_t(1) << 54) == 1 << 18);
static_assert(icbrt(uint64_t(1) << 57) == 1 << 19);
static_assert(icbrt(uint64_t(1) << 60) == 1 << 20);
static_assert(icbrt(uint64_t(1) << 63) == 1 << 21);
static_assert(icbrt((uint64_t(1) << 63) | (uint64_t(1) << 32)) == 2097152);
static_assert(icbrt(uint64_t(1'367'631'000'000'000ull)) == 111'000);
static_assert(icbrt(uint64_t(1'000'000'000'000'000'000ull)) == 1'000'000);
static_assert(icbrt(uint64_t(1'331'000'000'000'000'000ull)) == 1'100'000);
static_assert(icbrt(uint64_t(8'000'000'000'000'000'000ull)) == 2'000'000);
static_assert(icbrt(uint64_t(15'625'000'000'000'000'000ull)) == 2'500'000);
static_assert(icbrt(uint64_t(-1)) == 2642245);

/// @brief Checks whether n is a perfect square or not
/// @param n
/// @return true if n is a perfect square and false otherwise
static constexpr bool is_perfect_square(uint64_t n) noexcept {
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
static constexpr bool is_perfect_square(uint64_t n, uint32_t& root) noexcept {
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

static_assert(is_perfect_square(uint64_t(0)));
static_assert(is_perfect_square(uint64_t(1)));
static_assert(!is_perfect_square(uint64_t(2)));
static_assert(!is_perfect_square(uint64_t(3)));
static_assert(is_perfect_square(uint64_t(4)));
static_assert(!is_perfect_square(uint64_t(5)));
static_assert(is_perfect_square(uint64_t(9)));
static_assert(!is_perfect_square(uint64_t(15)));
static_assert(is_perfect_square(uint64_t(16)));
static_assert(is_perfect_square(uint64_t(324)));
static_assert(is_perfect_square(uint64_t(1 << 16)));
static_assert(is_perfect_square(uint64_t(1 << 24)));
static_assert(is_perfect_square(uint64_t(1) << 32));
static_assert(is_perfect_square(uint64_t(1) << 40));
static_assert(is_perfect_square(uint64_t(1) << 48));
static_assert(is_perfect_square(uint64_t(1) << 56));
static_assert(is_perfect_square(uint64_t(1) << 60));

#if defined(INTEGERS_128_BIT)

/// @brief Checks whether n is a perfect square or not
/// @param n
/// @return true if n is a perfect square and false otherwise
#if __cplusplus >= 202002L && defined(__GNUC__)
constexpr
#else
inline
#endif
    static bool
    is_perfect_square(uint128_t n) noexcept {
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
#if __cplusplus >= 202002L && defined(__GNUC__)
constexpr
#else
inline
#endif
    static bool
    is_perfect_square(uint128_t n, uint64_t& root) noexcept {
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

#if __cplusplus >= 202002L && defined(__GNUC__)
static_assert(is_perfect_square(uint128_t(0)));
static_assert(is_perfect_square(uint128_t(1)));
static_assert(!is_perfect_square(uint128_t(2)));
static_assert(!is_perfect_square(uint128_t(3)));
static_assert(is_perfect_square(uint128_t(4)));
static_assert(!is_perfect_square(uint128_t(5)));
static_assert(is_perfect_square(uint128_t(9)));
static_assert(!is_perfect_square(uint128_t(15)));
static_assert(is_perfect_square(uint128_t(16)));
static_assert(is_perfect_square(uint128_t(324)));
static_assert(is_perfect_square(uint128_t(1 << 16)));
static_assert(is_perfect_square(uint128_t(1 << 24)));
static_assert(is_perfect_square(uint128_t(1) << 32));
static_assert(is_perfect_square(uint128_t(1) << 40));
static_assert(is_perfect_square(uint128_t(1) << 48));
static_assert(is_perfect_square(uint128_t(1) << 56));
static_assert(is_perfect_square(uint128_t(1) << 60));
#endif

#endif

static constexpr uint32_t bit_reverse(uint32_t n) noexcept {
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
              0b00000000'00000000'00000000'00000000u);
static_assert(bit_reverse(0b00000000'00000000'00000000'00000001u) ==
              0b10000000'00000000'00000000'00000000u);
static_assert(bit_reverse(0b10000000'00000000'00000000'00000000u) ==
              0b00000000'00000000'00000000'00000001u);
static_assert(bit_reverse(0b00000000'11111111'00000000'00000000u) ==
              0b00000000'00000000'11111111'00000000u);
static_assert(bit_reverse(0b00000000'00000000'11111111'00000000u) ==
              0b00000000'11111111'00000000'00000000u);
static_assert(bit_reverse(0b10101010'10101010'10101010'10101010u) ==
              0b01010101'01010101'01010101'01010101u);
static_assert(bit_reverse(0b11111111'00000000'11111111'00000000u) ==
              0b00000000'11111111'00000000'11111111u);

static constexpr uint64_t bit_reverse(uint64_t n) noexcept {
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
    0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL);
static_assert(
    bit_reverse(uint64_t(
        0b10000001'00000000'10000001'00000000'10000001'00000000'10000001'00000000ULL)) ==
    0b00000000'10000001'00000000'10000001'00000000'10000001'00000000'10000001ULL);
static_assert(
    bit_reverse(uint64_t(
        0b00001111'00000000'11110000'00000000'10101010'00000000'00000000'00000000ULL)) ==
    0b00000000'00000000'00000000'01010101'00000000'00001111'00000000'11110000ULL);
static_assert(
    bit_reverse(uint64_t(
        0b00000000'00000000'00000000'10101010'10101010'00000000'00000000'00000000ULL)) ==
    0b00000000'00000000'00000000'01010101'01010101'00000000'00000000'00000000ULL);
static_assert(
    bit_reverse(uint64_t(
        0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL)) ==
    0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL);
static_assert(
    bit_reverse(uint64_t(
        0b11111111'00000000'11111111'00000000'11111111'00000000'11111111'00000000ULL)) ==
    0b00000000'11111111'00000000'11111111'00000000'11111111'00000000'11111111ULL);
static_assert(
    bit_reverse(uint64_t(
        0b11111111'11111111'11111111'11111111'00000000'00000000'00000000'00000000ULL)) ==
    0b00000000'00000000'00000000'00000000'11111111'11111111'11111111'11111111ULL);

static constexpr uint32_t pop_count_software(uint32_t n) noexcept {
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
static_assert(int64_t(pop_count_software(0u)) == int64_t(std::popcount(0u)));
static_assert(int64_t(pop_count_software(1u)) == int64_t(std::popcount(1u)));
static_assert(int64_t(pop_count_software(2u)) == int64_t(std::popcount(2u)));
static_assert(int64_t(pop_count_software(3u)) == int64_t(std::popcount(3u)));
static_assert(int64_t(pop_count_software(4u)) == int64_t(std::popcount(4u)));
static_assert(int64_t(pop_count_software(0x4788743u)) ==
              int64_t(std::popcount(0x4788743u)));
static_assert(int64_t(pop_count_software(0x2D425B23u)) ==
              int64_t(std::popcount(0x2D425B23u)));
static_assert(int64_t(pop_count_software(0xFFFFFFFFu - 1)) ==
              int64_t(std::popcount(0xFFFFFFFFu - 1)));
static_assert(int64_t(pop_count_software(0xFFFFFFFFu)) ==
              int64_t(std::popcount(0xFFFFFFFFu)));
#endif

static constexpr uint64_t pop_count_software(uint64_t n) noexcept {
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
static_assert(int64_t(pop_count_software(uint64_t(0))) ==
              int64_t(std::popcount(uint64_t(0))));
static_assert(int64_t(pop_count_software(uint64_t(1))) ==
              int64_t(std::popcount(uint64_t(1))));
static_assert(int64_t(pop_count_software(uint64_t(2))) ==
              int64_t(std::popcount(uint64_t(2))));
static_assert(int64_t(pop_count_software(uint64_t(3))) ==
              int64_t(std::popcount(uint64_t(3))));
static_assert(int64_t(pop_count_software(uint64_t(4))) ==
              int64_t(std::popcount(uint64_t(4))));
static_assert(int64_t(pop_count_software(uint64_t(0x4788743u))) ==
              int64_t(std::popcount(uint64_t(0x4788743u))));
static_assert(int64_t(pop_count_software(uint64_t(0x2D425B23u))) ==
              int64_t(std::popcount(uint64_t(0x2D425B23u))));
static_assert(int64_t(pop_count_software(uint64_t(0xFFFFFFFFu - 1))) ==
              int64_t(std::popcount(uint64_t(0xFFFFFFFFu - 1))));
static_assert(int64_t(pop_count_software(uint64_t(0xFFFFFFFFu))) ==
              int64_t(std::popcount(uint64_t(0xFFFFFFFFu))));
static_assert(int64_t(pop_count_software(uint64_t(0x5873485893484ull))) ==
              int64_t(std::popcount(uint64_t(0x5873485893484ull))));
static_assert(int64_t(pop_count_software(uint64_t(0x85923489853245ull))) ==
              int64_t(std::popcount(uint64_t(0x85923489853245ull))));
static_assert(int64_t(pop_count_software(uint64_t(0xFFFFFFFFFFFFFFFFull -
                                                  1))) ==
              int64_t(std::popcount(uint64_t(0xFFFFFFFFFFFFFFFFull - 1))));
static_assert(int64_t(pop_count_software(uint64_t(0xFFFFFFFFFFFFFFFFull))) ==
              int64_t(std::popcount(uint64_t(0xFFFFFFFFFFFFFFFFull))));
#endif

static constexpr int32_t pop_diff(uint32_t x, uint32_t y) noexcept {
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
static_assert(int64_t(std::popcount(0u)) - int64_t(std::popcount(0u)) ==
              int64_t(pop_diff(0, 0)));
static_assert(int64_t(std::popcount(1u)) - int64_t(std::popcount(0u)) ==
              int64_t(pop_diff(1, 0)));
static_assert(int64_t(std::popcount(0u)) - int64_t(std::popcount(1u)) ==
              int64_t(pop_diff(0, 1)));
static_assert(int64_t(std::popcount(0xABCDEFu)) - int64_t(std::popcount(4u)) ==
              int64_t(pop_diff(0xABCDEF, 4)));
static_assert(int64_t(std::popcount(uint32_t(uint16_t(-1)))) -
                  int64_t(std::popcount(314u)) ==
              int64_t(pop_diff(uint16_t(-1), 314)));
static_assert(int64_t(std::popcount(uint32_t(-1))) -
                  int64_t(std::popcount(0u)) ==
              int64_t(pop_diff(uint32_t(-1), 0)));
static_assert(int64_t(std::popcount(0u)) -
                  int64_t(std::popcount(uint32_t(-1))) ==
              int64_t(pop_diff(0, uint32_t(-1))));
static_assert(int64_t(std::popcount(uint32_t(-1))) -
                  int64_t(std::popcount(uint32_t(-1))) ==
              int64_t(pop_diff(uint32_t(-1), uint32_t(-1))));
#endif

static constexpr int32_t sign(int x) noexcept {
    return int32_t(x > 0) - int32_t(x < 0);
}

static constexpr int32_t sign(long x) noexcept {
    return int32_t(x > 0) - int32_t(x < 0);
}

static constexpr int32_t sign(long long x) noexcept {
    return int32_t(x > 0) - int32_t(x < 0);
}

#if defined(INTEGERS_128_BIT)

#if __cplusplus >= 202002L && defined(__GNUC__)
constexpr
#else
inline
#endif
    static int32_t
    sign(int128_t x) noexcept {
    uint32_t sign_bit = uint32_t(uint128_t(x) >> 127);
    return int32_t(x != 0) - int32_t(2 * sign_bit);
}

#if __cplusplus >= 202002L && defined(__GNUC__)
static_assert(sign(int128_t(0)) == 0);
static_assert(sign(int128_t(1)) == 1);
static_assert(sign(int128_t(-1)) == -1);
static_assert(sign(int128_t(2)) == 1);
static_assert(sign(int128_t(-2)) == -1);
static_assert(sign(int128_t(18446744073709551615ull)) == 1);
static_assert(sign(-int128_t(18446744073709551615ull)) == -1);
static_assert(sign(int128_t(1) << 63) == 1);
static_assert(sign(-(int128_t(1) << 63)) == -1);
static_assert(sign(int128_t(1) << 126) == 1);
static_assert(sign(-(int128_t(1) << 126)) == -1);
static_assert(sign(int128_t((uint128_t(1) << 127) - 1)) == 1);
static_assert(sign(int128_t(-((uint128_t(1) << 127) - 1))) == -1);
static_assert(sign(int128_t(-(uint128_t(1) << 127))) == -1);
#endif

#if __cplusplus >= 202002L && defined(__GNUC__)
constexpr
#else
inline
#endif
    static uint128_t
    uabs(int128_t n) noexcept {
    uint128_t t = uint128_t(n >> 127);
    return (uint128_t(n) ^ t) - t;
}

#if __cplusplus >= 202002L && defined(__GNUC__)
static_assert(uabs(int128_t(0)) == 0);
static_assert(uabs(int128_t(1)) == 1);
static_assert(uabs(int128_t(-1)) == 1);
static_assert(uabs(int128_t(4)) == 4);
static_assert(uabs(int128_t(-4)) == 4);
static_assert(uabs(int128_t(18446744073709551615ull)) ==
              18446744073709551615ull);
static_assert(uabs(-int128_t(18446744073709551615ull)) ==
              18446744073709551615ull);
static_assert(uabs(int128_t(1) << 126) == uint128_t(1) << 126);
static_assert(uabs(-(int128_t(1) << 126)) == uint128_t(1) << 126);
static_assert(uabs(int128_t((uint128_t(1) << 127) - 1)) ==
              (uint128_t(1) << 127) - 1);
static_assert(uabs(int128_t(-((uint128_t(1) << 127) - 1))) ==
              (uint128_t(1) << 127) - 1);
static_assert(uabs(int128_t(-(uint128_t(1) << 127))) == uint128_t(1) << 127);
#endif

#endif

// Visual C++ thinks that unary minus on uint32_t is an error :clown:
#if !defined(_MSC_VER)

static constexpr int32_t pop_cmp(uint32_t x, uint32_t y) noexcept {
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

static_assert(sign(int64_t(std::popcount(0u)) - int64_t(std::popcount(0u))) ==
              sign(int64_t(pop_cmp(0, 0))));
static_assert(sign(int64_t(std::popcount(1u)) - int64_t(std::popcount(0u))) ==
              sign(int64_t(pop_cmp(1, 0))));
static_assert(sign(int64_t(std::popcount(0u)) - int64_t(std::popcount(1u))) ==
              sign(int64_t(pop_cmp(0, 1))));
static_assert(sign(int64_t(std::popcount(0xABCDEFu)) -
                   int64_t(std::popcount(4u))) ==
              int64_t(pop_cmp(0xABCDEF, 4)));
static_assert(sign(int64_t(std::popcount(uint32_t(uint16_t(-1)))) -
                   int64_t(std::popcount(314u))) ==
              sign(int64_t(pop_cmp(uint16_t(-1), 314))));
static_assert(sign(int64_t(std::popcount(uint32_t(-1))) -
                   int64_t(std::popcount(0u))) ==
              sign(int64_t(pop_cmp(uint32_t(-1), 0))));
static_assert(sign(int64_t(std::popcount(0u)) -
                   int64_t(std::popcount(uint32_t(-1)))) ==
              sign(int64_t(pop_cmp(0, uint32_t(-1)))));
static_assert(sign(int64_t(std::popcount(uint32_t(-1))) -
                   int64_t(std::popcount(uint32_t(-1)))) ==
              sign(int64_t(pop_cmp(uint32_t(-1), uint32_t(-1)))));
#endif

#endif

static constexpr uint32_t lz_count_32_software(uint32_t n) noexcept {
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

static_assert(lz_count_32_software(0) == 32);
static_assert(lz_count_32_software(1) == 31);
static_assert(lz_count_32_software(2) == 30);
static_assert(lz_count_32_software(4) == 29);
static_assert(lz_count_32_software(8) == 28);
static_assert(lz_count_32_software(12) == 28);
static_assert(lz_count_32_software(16) == 27);
static_assert(lz_count_32_software(32) == 26);
static_assert(lz_count_32_software(48) == 26);
static_assert(lz_count_32_software(uint32_t(1) << 30) == 1);
static_assert(lz_count_32_software(uint32_t(1) << 31) == 0);
static_assert(lz_count_32_software(~uint32_t(1)) == 0);

static constexpr uint32_t lz_count_64_software(uint64_t n) noexcept {
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

static_assert(lz_count_64_software(0) == 64);
static_assert(lz_count_64_software(1) == 63);
static_assert(lz_count_64_software(2) == 62);
static_assert(lz_count_64_software(4) == 61);
static_assert(lz_count_64_software(8) == 60);
static_assert(lz_count_64_software(12) == 60);
static_assert(lz_count_64_software(16) == 59);
static_assert(lz_count_64_software(32) == 58);
static_assert(lz_count_64_software(48) == 58);
static_assert(lz_count_64_software(uint32_t(1) << 30) == 33);
static_assert(lz_count_64_software(uint32_t(1) << 31) == 32);
static_assert(lz_count_64_software(~uint32_t(1)) == 32);
static_assert(lz_count_64_software(uint64_t(1) << 62) == 1);
static_assert(lz_count_64_software(uint64_t(1) << 63) == 0);
static_assert(lz_count_64_software(uint64_t(-1)) == 0);

static constexpr uint32_t tz_count_32_software(uint32_t n) noexcept {
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

static_assert(tz_count_32_software(0u) == 32);
static_assert(tz_count_32_software(1u) == 0);
static_assert(tz_count_32_software(2u) == 1);
static_assert(tz_count_32_software(4u) == 2);
static_assert(tz_count_32_software(8u) == 3);
static_assert(tz_count_32_software(12u) == 2);
static_assert(tz_count_32_software(16u) == 4);
static_assert(tz_count_32_software(32u) == 5);
static_assert(tz_count_32_software(48u) == 4);
static_assert(tz_count_32_software(1u << 30) == 30);
static_assert(tz_count_32_software(1u << 31) == 31);
static_assert(tz_count_32_software(~1u) == 1);
static_assert(tz_count_32_software(uint32_t(-1)) == 0);

static constexpr uint32_t tz_count_64_software(uint64_t n) noexcept {
    uint32_t m = 0;
    for (n = ~n & (n - 1); n != 0; n >>= 1) {
        m++;
    }
    return m;
}

static_assert(tz_count_64_software(0u) == 64);
static_assert(tz_count_64_software(1u) == 0);
static_assert(tz_count_64_software(2u) == 1);
static_assert(tz_count_64_software(4u) == 2);
static_assert(tz_count_64_software(8u) == 3);
static_assert(tz_count_64_software(12u) == 2);
static_assert(tz_count_64_software(16u) == 4);
static_assert(tz_count_64_software(32u) == 5);
static_assert(tz_count_64_software(48u) == 4);
static_assert(tz_count_64_software(1u << 30) == 30);
static_assert(tz_count_64_software(1u << 31) == 31);
static_assert(tz_count_64_software(~1u) == 1);
static_assert(tz_count_64_software(uint32_t(-1)) == 0);

/// @brief Count trailing zeros for n
/// @param n
/// @return trailing zeros count (sizeof(n) * 8 for n = 0)
template <typename T>
#if __cplusplus >= 202002L
    requires std::is_unsigned_v<T>
#if defined(INTEGERS_128_BIT)
             || std::is_same_v<T, uint128_t>
#endif
#endif
static constexpr int32_t count_trailing_zeros(T n) noexcept {
    if (unlikely(n == 0)) {
        return sizeof(n) * 8;
    }

#if defined(INTEGERS_128_BIT)
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
                      std::is_same_v<T, unsigned char>);
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
#if defined(INTEGERS_128_BIT)
             || std::is_same_v<T, uint128_t>
#endif
#endif
static constexpr inline int32_t count_leading_zeros(T n) noexcept {
    if (unlikely(n == 0)) {
        return sizeof(n) * 8;
    }

#if defined(INTEGERS_128_BIT)
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
                      std::is_same_v<T, unsigned char>);
#if defined(__GNUC__)
        return __builtin_clz(n);
#else
        return static_cast<int32_t>(lz_count_32_software(n));
#endif
    }
#endif
}

static constexpr size_t nearest_2_pow_greater_equal(size_t n) noexcept {
    return size_t(1u) << (64 - uint32_t(count_leading_zeros(n | 1)) -
                          ((n & (n - 1)) == 0));
}

/* Just constexpr version of isdigit from ctype.h */
static constexpr bool is_digit(int32_t c) noexcept {
    return static_cast<uint32_t>(c) - '0' <= '9' - '0';
}

static constexpr uint32_t base_2_digits(uint32_t n) noexcept {
    // " | 1" operation does not affect the answer for all numbers except n = 0
    // for n = 0 answer is 1
    return 32 - uint32_t(count_leading_zeros(n | 1));
}

static constexpr uint32_t base_2_digits(uint64_t n) noexcept {
    // " | 1" operation does not affect the answer for all numbers except n = 0
    // for n = 0 answer is 1
    return 64 - uint32_t(count_leading_zeros(n | 1));
}

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
static_assert(base_10_digits(0u) == 1);
static_assert(base_10_digits(1u) == 1);
static_assert(base_10_digits(9u) == 1);
static_assert(base_10_digits(10u) == 2);
static_assert(base_10_digits(11u) == 2);
static_assert(base_10_digits(99u) == 2);
static_assert(base_10_digits(100u) == 3);
static_assert(base_10_digits(101u) == 3);
static_assert(base_10_digits(uint32_t(-1)) == 10);
#endif

/// @brief Realization taken from the gcc libstdc++ __to_chars_len
/// @tparam T
/// @param value
/// @return
template <typename T>
#if __cplusplus >= 202002L
    requires std::is_unsigned_v<T>
#if defined(INTEGERS_128_BIT)
             || std::is_same_v<T, uint128_t>
#endif
#endif
static constexpr uint32_t base_10_len(T value) noexcept {
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
static_assert(base_10_len(0ull) == 1);
static_assert(base_10_len(1ull) == 1);
static_assert(base_10_len(9ull) == 1);
static_assert(base_10_len(10ull) == 2);
static_assert(base_10_len(11ull) == 2);
static_assert(base_10_len(99ull) == 2);
static_assert(base_10_len(100ull) == 3);
static_assert(base_10_len(101ull) == 3);
static_assert(base_10_len(uint64_t(-1)) == 20);

#if defined(INTEGERS_128_BIT)
static_assert(base_10_len(uint128_t(0)) == 1);
static_assert(base_10_len(uint128_t(1)) == 1);
static_assert(base_10_len(uint128_t(9)) == 1);
static_assert(base_10_len(uint128_t(10)) == 2);
static_assert(base_10_len(uint128_t(11)) == 2);
static_assert(base_10_len(uint128_t(99)) == 2);
static_assert(base_10_len(uint128_t(100)) == 3);
static_assert(base_10_len(uint128_t(101)) == 3);
static_assert(base_10_len(uint128_t(-1)) == 39);
#endif

#endif

static constexpr uint32_t log2_floor(uint64_t n) noexcept {
    // " | 1" does not affect ans for all n >= 1.
    return 63 ^ uint32_t(count_leading_zeros(n | 1));
}

static constexpr uint32_t log2_ceil(uint64_t n) noexcept {
    return log2_floor(n) + ((n & (n - 1)) != 0);
}

#if defined(INTEGERS_128_BIT)
static constexpr uint32_t log2_floor(uint128_t n) noexcept {
    // " | 1" does not affect ans for all n >= 1.
    uint64_t hi = uint64_t(n >> 64);
    return hi != 0 ? (127 ^ uint32_t(count_leading_zeros(hi)))
                   : (63 ^ uint32_t(count_leading_zeros(uint64_t(n) | 1)));
}

static constexpr uint32_t log2_ceil(uint128_t n) noexcept {
    return log2_floor(n) + ((n & (n - 1)) != 0);
}

#endif

/// @brief Find q and r such n = q * (2 ^ r), q odd
/// @param n n value.
/// @param r r value to find.
/// @return q.
template <typename T>
#if __cplusplus >= 202002L
    requires std::is_unsigned_v<T>
#if defined(INTEGERS_128_BIT)
             || std::is_same_v<T, uint128_t>
#endif
#endif
static constexpr T extract_2pow(T n, uint32_t& r) noexcept {
    r = uint32_t(count_trailing_zeros(n));
    return n >> r;
}

}  // namespace math_utils

#if defined(INTEGERS_128_BIT)

namespace std {

#if __cplusplus >= 202002L && defined(__GNUC__)
constexpr
#endif
    static uint128_t
    gcd(uint128_t a, uint128_t b) noexcept {
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
            auto tmp = std::move(a);
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

#if __cplusplus >= 202002L && defined(__GNUC__)
static_assert(gcd(uint128_t(1), uint128_t(1)) == 1);
static_assert(gcd(uint128_t(3), uint128_t(7)) == 1);
static_assert(gcd(uint128_t(0), uint128_t(112378432)) == 112378432);
static_assert(gcd(uint128_t(112378432), uint128_t(0)) == 112378432);
static_assert(gcd(uint128_t(429384832), uint128_t(324884)) == 4);
static_assert(gcd(uint128_t(18446744073709551521ull),
                  uint128_t(18446744073709551533ull)) == 1);
static_assert(gcd(uint128_t(18446744073709551521ull) * 18446744073709551521ull,
                  uint128_t(18446744073709551521ull)) ==
              18446744073709551521ull);
static_assert(gcd(uint128_t(23999993441ull) * 23999993377ull,
                  uint128_t(23999992931ull) * 23999539633ull) == 1);
static_assert(gcd(uint128_t(2146514599u) * 2146514603u * 2146514611u,
                  uint128_t(2146514611u) * 2146514621u * 2146514647u) ==
              2146514611ull);
static_assert(gcd(uint128_t(2146514599u) * 2146514603u * 2146514611u * 2,
                  uint128_t(2146514599u) * 2146514603u * 2146514611u * 3) ==
              uint128_t(2146514599u) * 2146514603u * 2146514611u);
static_assert(gcd(uint128_t(100000000000000003ull) * 1000000000000000003ull,
                  uint128_t(1000000000000000003ull) * 1000000000000000009ull) ==
              1000000000000000003ull);
static_assert(gcd(uint128_t(3 * 2 * 5 * 7 * 11 * 13 * 17 * 19),
                  uint128_t(18446744073709551557ull) * 3) == 3);
static_assert(gcd(uint128_t(1000000000000000009ull),
                  uint128_t(1000000000000000009ull) * 1000000000000000009ull) ==
              1000000000000000009ull);
static_assert(gcd(uint128_t(0),
                  uint128_t(1000000000000000009ull) * 1000000000000000009ull) ==
              uint128_t(1000000000000000009ull) * 1000000000000000009ull);
static_assert(gcd(uint128_t(18446744073709551557ull), uint128_t(0)) ==
              18446744073709551557ull);
#endif

#if __cplusplus >= 202002L && defined(__GNUC__)
constexpr
#endif
    static uint128_t gcd(uint64_t a, int128_t b) noexcept {
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
    uint64_t a2 = b1;                 // b1 < 2^64 => a2 < 2^64
    // b2 = a1 % b1
    // a1 = b0, b1 = a % b0 => b1 < a1
    uint64_t b2 = uint64_t(a1 % b1);  // b1 < 2^64 => b2 = a1 % b1 < 2^64
    return std::gcd(a2, b2);
}

#if __cplusplus >= 202002L && defined(__GNUC__)
static_assert(gcd(uint64_t(2), int128_t(4)) == 2);
static_assert(gcd(uint64_t(2), int128_t(-4)) == 2);
static_assert(gcd(uint64_t(3), int128_t(7)) == 1);
static_assert(gcd(uint64_t(3), int128_t(-7)) == 1);
static_assert(gcd(uint64_t(3), int128_t(18446744073709551557ull) * 3) == 3);
static_assert(gcd(uint64_t(3), int128_t(18446744073709551557ull) * (-3)) == 3);
static_assert(gcd(uint64_t(3) * 2 * 5 * 7 * 11 * 13 * 17 * 19,
                  int128_t(18446744073709551557ull) * 3) == 3);
static_assert(gcd(uint64_t(1000000000000000009ull),
                  int128_t(1000000000000000009ll) * 1000000000000000009ll) ==
              1000000000000000009ull);
static_assert(gcd(uint64_t(0),
                  int128_t(1000000000000000009ll) * 1000000000000000009ll) ==
              uint128_t(1000000000000000009ll) * 1000000000000000009ull);
static_assert(gcd(uint64_t(18446744073709551557ull), int128_t(0)) ==
              18446744073709551557ull);
#endif

}  // namespace std

#endif

#endif  // !MATH_UTILS_HPP
