#include <cstdint>
#include <iostream>
#include <popcntintrin.h>
#include <x86intrin.h>

#include <numeric>

#ifdef __SIZEOF_INT128__

typedef __uint128_t uint128_t;
typedef __uint128_t uint_64_128_t;

static inline int ctz_u128(uint128_t n) noexcept;

#else
typedef uint64_t uint_64_128_t;
#endif

static inline bool is_prime(uint_64_128_t n) noexcept;

static inline bool is_strong_prp(uint_64_128_t n, uint_64_128_t a) noexcept;

static inline bool is_strong_selfridge_prp(uint_64_128_t n) noexcept;

static constexpr uint_64_128_t two_pow_n_mod_n(uint_64_128_t n) noexcept;

inline uint_64_128_t gcd(uint_64_128_t a, uint_64_128_t b);

static inline uint_64_128_t find_r_s(uint_64_128_t n, int &r) noexcept;

static constexpr uint_64_128_t bin_pow_mod(uint_64_128_t base, uint_64_128_t exp, uint_64_128_t mod) noexcept;

int main() {
    //                18446744073709552157;
    uint_64_128_t p = 1844674407370;
    p =                       p * 10000000;
    p =                        p + 9552157;
    std::cout << std::boolalpha << is_prime(p);
    std::gcd(p, p);
}

inline uint_64_128_t gcd(uint_64_128_t a, uint_64_128_t b) {
    uint_64_128_t temp;
    while (b) {
        temp = a;
        a = b;
        b = temp % b;
    }

    return a;
}

static inline bool is_strong_prp(uint_64_128_t n, uint_64_128_t a) noexcept {
#ifndef NOCHECK
    if (a < 2)
    {
        return false;
    }

    if (n == 1) {
        return false;
    }
    
    if ((n & 1) == 0) {
        return n == 2;
    }

    if (gcd(n, a) != 1) {
        return false;
    }
#endif

    const uint_64_128_t n_minus_1 = n - 1;
    
    /* Find s and r satisfying: n - 1 = s * (2^r), s is odd */
    int r = 0; uint_64_128_t s = find_r_s(n_minus_1, r);

    /* Check a^((2^t)*s) mod n for 0 <= t < r */
    
    // Init test = ((a^s) mod n)
    uint_64_128_t test = bin_pow_mod(a, s, n);
    if (test == 0 || test == n_minus_1)
    {
        return true;
    }

    // Since n is odd and n - 1 is even, initially r > 0.
    while (--r)
    {
        /* test = (test ^ 2) % n */
        test = test * test % n;
        if (test == n_minus_1)
        {
            return true;
        }
    }

    return false;    
}

static inline bool is_strong_selfridge_prp(uint_64_128_t n) noexcept {
#ifndef NOCHECK
    if (n == 1) {
        return false;
    }
    
    if ((n & 1) == 0) {
        return n == 2;
    }
#endif

    // TODO:
    return true;
}

/// @brief Calculate (2 ^ n) % n
/// @param n
/// @return (2 ^ n) % n
static constexpr uint_64_128_t two_pow_n_mod_n(uint_64_128_t n) noexcept {
    if (n) {
        uint_64_128_t res = two_pow_n_mod_n(n >> 1);
        res = (res * res) % n;
        return (n & 1) ? ((res << 1) % n) : res;
    }

    return 1;
}

/// @brief Calculate (n ^ p) % mod
/// @param n 
/// @param p 
/// @param mod 
/// @return (n ^ p) % mod
static constexpr uint_64_128_t bin_pow_mod(uint_64_128_t n, uint_64_128_t p, uint_64_128_t mod) noexcept {
    uint_64_128_t res = 1;
    do {
        if (p & 1) { /* p % 2 != 0 */
            res = (res * n) % mod;
        }
        n = (n * n) % mod;
        p >>= 1; // /= 2
    } while(p);

    return res;
}

/// @brief Find s and r such n = s * (2^r)
/// @param n n value.
/// @param r r value to find.
/// @return s.
static inline uint_64_128_t find_r_s(uint_64_128_t n, int &r) noexcept {
#ifdef __SIZEOF_INT128__
    r = ctz_u128(n);
#else
    r = __builtin_ctzll(n);
#endif
    return n >> r;
}

#ifdef __SIZEOF_INT128__
static inline int ctz_u128(uint128_t n) noexcept {
    uint64_t high_or_low = (uint64_t)n;
    if (high_or_low == 0)
    {// low part of n consist of zeros.

        if ((high_or_low = (uint64_t)(n >> 64)) != 0)
        {// high part of n does not consist of zeros.
            return __builtin_ctzll(high_or_low) + 64;
        }

        // else n is equal to 0.
        return 128;
    }
    
    // low part of n does not consist of zeros.
    return __builtin_ctzll(high_or_low);
}
#endif

static inline bool is_prime(uint_64_128_t n) noexcept {
    if (n == 1) {
        return false;
    }

    if (n == 2 || n == 3 || n == 5) {
        return true;
    }

    if (((n & 1) == 0) || ((n % 3) == 0) || ((n % 5) == 0)){
        return false;
    }

    if (n < 49) {
        return true;
    }

    if ((n %  7) == 0 || (n % 11) == 0 || (n % 13) == 0 || (n % 17) == 0 ||
        (n % 19) == 0 || (n % 23) == 0 || (n % 29) == 0 || (n % 31) == 0 ||
        (n % 37) == 0 || (n % 41) == 0 || (n % 43) == 0 || (n % 47) == 0) {
        return  false;
    }
    
    if (n < 2809) {
        return true;
    }
    
    if (n < 31417) {
        return two_pow_n_mod_n(n) == 2;
    }

    return is_strong_prp(n, 2) && is_strong_selfridge_prp(n);
}
