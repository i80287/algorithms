#if !defined(_MATH_UTILS_HPP_)
#define _MATH_UTILS_HPP_ 1

#include "integers_128_bit.hpp"

namespace std {

constexpr int128_t abs(int128_t x) noexcept {
    return x >= 0 ? x : -x;
}

constexpr uint128_t gcd(uint128_t a, uint128_t b) noexcept {
    uint128_t temp = 0;
    while (b != 0) {
        temp = a;
        a = b;
        b = temp % b;
    }

    return a;
}

constexpr uint128_t gcd(uint64_t a, int128_t b) noexcept {
    uint128_t b_ = static_cast<uint128_t>(b >= 0 ? b : -b);
    if (unlikely(b == 0)) {
        return a;
    }

    uint128_t a_ = b_;
    b_ = a % b_;    // Now b_ < 2^64
    if (b_ == 0) {
        return a_;
    }

    uint128_t temp = a_;
    a_ = b_;        // Now a_ < 2^64
    b_ = temp % b_; // And still b_ < 2^64

    return gcd(uint64_t(a_), uint64_t(b_));
}

#if __cplusplus >= 202002L
static_assert(gcd(uint64_t(2), int128_t(4)) == 2);
static_assert(gcd(uint64_t(2), int128_t(-4)) == 2);
static_assert(gcd(uint64_t(3), int128_t(7)) == 1);
static_assert(gcd(uint64_t(3), int128_t(-7)) == 1);
static_assert(gcd(uint64_t(3), int128_t(18446744073709551557ull) * 3) == 3);
static_assert(gcd(uint64_t(3), int128_t(18446744073709551557ull) * (-3)) == 3);
static_assert(gcd(uint64_t(1000000000000000009ull), int128_t(1000000000000000009ll) * 1000000000000000009ll) == 1000000000000000009ull);
static_assert(gcd(uint64_t(0), int128_t(1000000000000000009ll) * 1000000000000000009ll) == uint128_t(1000000000000000009ll) * 1000000000000000009ull);
static_assert(gcd(uint64_t(18446744073709551557ull), int128_t(0)) == 18446744073709551557ull);
#endif

} // namespace std

/// @brief Calculate (n ^ p) % mod
/// @param n
/// @param p
/// @param mod
/// @return (n ^ p) % mod
static constexpr uint32_t BinPowMod(uint32_t n, uint32_t p, uint32_t mod) noexcept {
    uint64_t res = 1;
    uint64_t wdn_n = n;
    while (true) {
        if (p & 1) {
            res = (res * wdn_n) % mod;
        }
        p >>= 1;
        if (p == 0) {
            break;
        }
        wdn_n = (wdn_n * wdn_n) % mod;
    }

    return static_cast<uint32_t>(res);
}

/// @brief Calculate (n ^ p) % mod
/// @param n
/// @param p
/// @param mod
/// @return (n ^ p) % mod
static constexpr uint64_t BinPowMod(uint64_t n, uint64_t p, uint64_t mod) noexcept {
    uint64_t res = 1;
    while (true) {
        if (p & 1) {
            res = uint64_t((uint128_t(res) * uint128_t(n)) % mod);
        }
        p >>= 1;
        if (p == 0) {
            break;
        }
        n = uint64_t((uint128_t(n) * uint128_t(n)) % mod);
    }

    return static_cast<uint64_t>(res);
}

/// @brief Find s and r such n = s * (2 ^ r), s odd
/// @param n n value.
/// @param r r value to find.
/// @return s.
template <typename T>
#if __cplusplus >= 202002L
requires std::is_unsigned_v<T>
#endif
static constexpr T FindRS(T n, int32_t &r) noexcept {
    r = std::count_trailing_zeros(n);
    return n >> r;
}

static constexpr bool IsPerfectSquare(uint64_t n) noexcept {
    /*
     * +------------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     * |   n mod 16 |  1  |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 | 11 | 12 | 13 | 14 | 15 |
     * +------------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     * | n*n mod 16 |  1  |  4 |  9 |  0 |  9 |  4 |  1 |  0 |  1 |  4 |  9 |  0 |  9 |  4 |  1 |
     * +------------+-----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
     * 
     * If we peek mod 32, then we should check only for n & 31 in { 0, 1, 4, 9, 16, 17, 25 }
     */
    switch (n & 15) {
        case 0:
        case 1:
        case 4:
        case 9: {
            uint64_t root = static_cast<uint64_t>(std::sqrt(n));
            return root * root == n;
        }
        default:
            return false;
    }
}

#endif // !_MATH_UTILS_HPP_