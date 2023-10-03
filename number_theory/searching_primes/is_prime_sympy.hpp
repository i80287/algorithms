#include <cstdint>
#include <cmath>
#include <numeric>

#include "integers_128_bit.hpp"

template <typename T>
requires std::is_unsigned_v<T>
static constexpr T GCD(T a, T b) noexcept {
    if constexpr (std::is_same_v<T, uint128_t>) {
        uint128_t temp = 0;
        while (b) {
            temp = a;
            a = b;
            b = temp % b;
        }

        return a;
    }
    else {
        return std::gcd(a, b);   
    }
}

static constexpr uint128_t GCD(uint64_t a, int128_t b) noexcept {
    return GCD(static_cast<uint128_t>(a), static_cast<uint128_t>(b >= 0 ? b : -b));
}

/// @brief Calculate (n ^ p) % mod
/// @param n
/// @param p
/// @param mod
/// @return (n ^ p) % mod
static constexpr uint64_t BinPowMod(uint64_t n, uint64_t p, uint64_t mod) noexcept {
    if (mod == 0) {
        return 0;
    }

    uint128_t res = 1;
    uint128_t wdn_n = n % mod;

    do {
        if (p & 1) { /* p % 2 != 0 */
            res = (res * wdn_n) % mod;
        }

        wdn_n = (wdn_n * wdn_n) % mod;
        p >>= 1; // /= 2
    } while(p);

    return static_cast<uint64_t>(res);
}

/// @brief Find s and r such n = s * (2 ^ r), s odd
/// @param n n value.
/// @param r r value to find.
/// @return s.
template <typename T>
requires std::is_unsigned_v<T>
static constexpr T FindRS(T n, int32_t &r) noexcept {
    r = std::count_trailing_zeros(n);
    return n >> r;
}

template <typename Uint>
requires std::is_unsigned_v<Uint>
static constexpr int32_t JacobiSymbolUi(Uint a, Uint n) noexcept {
    int32_t t = 1;
    if (n % 2 == 0) {
        if (n == 0) {
            return a == 1;
        }

        int32_t p = std::count_trailing_zeros(n);
        n >>= p;

        switch (a % 8) {
            case 0:
            case 2:
            case 4:
            case 6:
                // a % 2 == 0
                return 0;
            case 3:
            case 5:
                // a === +-3 (mod 8)
                // t = (-1) ^ p
                t -= (int32_t(p % 2 != 0) << 1);
                break;
            case 1:
            case 7:
                // a === +-1 (mod 8)
                // t = 1
                break;
        }
    }

    //step 1
    a = a % n;
    Uint r = 0;
    //step 3
    while (a != 0) {
        //step 2
        while (a % 2 == 0) {
            a /= 2;
            r = n % 8;
            if (r == 3 || r == 5) {
                t = -t;
            }
        }

        //step 4
        r = n;
        n = a;
        a = r;
        if (a % 4 == 3 && n % 4 == 3) {
            t = -t;
        }
        a = a % n;
    }

    return (n == 1) ? t : 0;
}

template <typename Sint>
requires std::is_signed_v<Sint>
static constexpr int32_t JacobiSymbolSi(Sint a, Sint n) noexcept {
    bool carry = n < 0;
    if (carry) {
        n = -n;
        carry = a < 0;
    }

    int32_t t = 1;
    if (n % 2 == 0) {
        if (n == 0) {
            return a == 1 || a == -1;
        }

        int32_t p = std::count_trailing_zeros(static_cast<std::make_unsigned_t<Sint>>(n));
        n >>= p;

        switch (((a % 8) + 8) % 8) {
            case 0:
            case 2:
            case 4:
            case 6:
                // a % 2 == 0
                return 0;
            case 3:
            case 5:
                // a === +-3 (mod 8)
                // t = (-1) ^ p
                t -= (static_cast<int32_t>(p % 2 != 0) << 1);
                break;
            case 1:
            case 7:
                // a === +-1 (mod 8)
                // t = 1
                break;
        }
    }

    //step 1
    a = (a % n + n) % n;
    Sint r = 0;
    //step 3
    while (a != 0) {
        //step 2
        while (a % 2 == 0) {
            a /= 2;
            r = n % 8;
            if (r == 3 || r == 5) {
                t = -t;
            }
        }

        //step 4
        r = n;
        n = a;
        a = r;
        if ((a % 4 + 4) % 4 == 3 && n % 4 == 3) {
            t = -t;
        }
        a = (a % n + n) % n;
    }

    return (n == 1) ? (!carry ? t : -t) : 0;
}

/// @brief Calculates Jacobi symbol; Source: https://en.wikipedia.org/wiki/Jacobi_symbol && https://en.wikipedia.org/wiki/Kronecker_symbol
/// @param a 
/// @param n 
/// @return Jacobi symbol (-1, 0 or 1)
template <typename IntegerT1, typename IntegerT2>
requires std::is_integral_v<IntegerT1> && std::is_integral_v<IntegerT2>
static constexpr int32_t JacobiSymbol(IntegerT1 a, IntegerT2 n) noexcept {
    using T1 = std::remove_cvref_t<IntegerT1>;
    using T2 = std::remove_cvref_t<IntegerT2>;

    static_assert(sizeof(T1) == sizeof(T2)
        && !std::is_same_v<T1, bool>
        && !std::is_same_v<T2, bool>);

    if constexpr (std::is_unsigned_v<T1>) {
        if constexpr (std::is_unsigned_v<T2>) {
            return JacobiSymbolUi<T1>(a, static_cast<T1>(n));
        }
        else {
            return JacobiSymbolUi<T1>(a, static_cast<T1>(n >= 0 ? n : -n));
        }
    }
    else {
        if constexpr (std::is_unsigned_v<T2>) {
            if constexpr (sizeof(T2) >= sizeof(int64_t)) {
                return JacobiSymbolSi<int128_t>(static_cast<int128_t>(a), static_cast<int128_t>(n));
            }
            else {
                return JacobiSymbolSi<int64_t>(static_cast<int64_t>(a), static_cast<int64_t>(n));
            }
        }
        else {
            return JacobiSymbolSi<T1>(a, static_cast<T1>(n));
        }
    }
}

static constexpr bool IsPerfectSquare(uint64_t n) noexcept {
    uint64_t root = static_cast<uint64_t>(std::sqrt(n));
    return root * root == n;
}

/* *********************************************************************************************
 * mpz_sprp: (also called a Miller-Rabin probable prime)
 * A "strong probable prime" to the base a is an odd composite n = (2^r)*s+1 with s odd such that
 * either a^s == 1 mod n, or a^((2^t)*s) == -1 mod n, for some integer t, with 0 <= t < r.
 * *********************************************************************************************/
static constexpr bool IsStrongPRP(uint64_t n, uint64_t a) noexcept {
    if (a < 2) {
        // throw std::domain_error("bool IsStrongPRP(uint64_t, uint64_t) requires 'a' greater than or equal to 2");
        return false;
    }

    if (n == 1) {
        return false;
    }

    if ((n & 1) == 0) {
        return n == 2;
    }

    if (GCD(n, a) != 1) {
        // throw std::domain_error("bool IsStrongPRP(uint64_t, uint64_t) requires gcd(n,a) == 1");
        return false;
    }

    const auto n_minus_1 = n - 1;
    /* Find s and r satisfying: n - 1 = s * (2^r), s odd */
    int32_t r = 0; uint64_t s = FindRS(n_minus_1, r);

    // n - 1 >= 2 => r >= 1

    /* Check a^((2^t)*s) mod n for 0 <= t < r */

    // Init test = ((a^s) mod n)
    uint64_t test = BinPowMod(a, s, n);
    if (test == 1 || test == n_minus_1) {
        return true;
    }

    // Since n is odd and n - 1 is even, initially r > 0.
    while (--r) {
        /* test = (test ^ 2) % n */
        uint128_t widen_test = test;
        test = static_cast<uint64_t>((widen_test * widen_test) % n);

        if (test == n_minus_1) {
            return true;
        }
    }

    return false;
}

constexpr int32_t SizeInBaseTwo(uint64_t n) noexcept {
    // " | 1" operation does not affect the answer for all numbers except n = 0
    // for n = 0 answer is 1
    return 64 - __builtin_clzll(n | 1);
}

/* *********************************************************************************************
 * mpz_stronglucas_prp:
 * A "strong Lucas probable prime" with parameters (P,Q) is a composite n = (2^r)*s+(D/n), where
 * s is odd, D=P^2-4Q, and gcd(n,2QD)=1 such that either U_s == 0 mod n or V_((2^t)*s) == 0 mod n
 * for some t, 0 <= t < r. [(D/n) is the Jacobi symbol]
 * *********************************************************************************************/
static constexpr bool IsStrongLucasPRP(uint64_t n, uint32_t p, int32_t q) noexcept {
    /* Check if p*p - 4*q == 0. */
    int64_t d = static_cast<int64_t>(p) * p - static_cast<int64_t>(q) * 4;
    if (d == 0) {
        // throw std::domain_error("Invalid values for p,q in bool IsStrongLucasPRP(uint64_t, uint64_t, uint64_t)");
        return false;
    }

    if (n == 1) {
        return false;
    }

    if ((n & 1) == 0) {
        return n == 2;
    }

    uint128_t res = GCD(n, 2 * q * static_cast<int128_t>(d));
    if (res != n && res != 1) {
        // throw std::domain_error("is_strong_lucas_prp() requires gcd(n, 2 * q * (p * p - 4 * q)) == 1");
        return false;
    }

    /* nmj = n - (D/n), where (D/n) is the Jacobi symbol */
    uint64_t nmj = n - JacobiSymbol(d, n);

    /* Find s and r satisfying: nmj = (2 ^ r) * s, s odd */
    int32_t r = 0; uint64_t s = FindRS(nmj, r);

    /* make sure U_s == 0 mod n or V_((2^t)*s) == 0 mod n, for some t, 0 <= t < r */
    uint128_t uh = 1;
    uint128_t vl = 2;
    uint128_t vh = p;
    uint128_t ql = 1;
    uint128_t qh = 1;
    const uint128_t widen_n = n;
    const uint64_t widen_q = static_cast<uint64_t>(q >= 0 ? q : n + q);

    for (int32_t j = SizeInBaseTwo(s) - 1; j >= 1; j--) {
        /* ql = ql*qh (mod n) */
        ql = ((ql * qh) % widen_n);
        if (s & (1ull << j)) {
            /* qh = ql*q */
            qh = ((ql * widen_q) % widen_n);

            /* uh = uh*vh (mod n) */
            uh = ((uh * vh) % widen_n);

            /* vl = vh*vl - p*ql (mod n) */
            uint128_t vh_vl = vh * vl;
            uint128_t p_ql = p * ql;
            vl = vh_vl - p_ql;
            if (vh_vl < p_ql) {
                vl += widen_n * ((p_ql - vh_vl + widen_n - 1) / widen_n);
            }

            vl = (vl % widen_n);

            /* vh = vh*vh - 2*qh (mod n) */
            uint128_t vh_vh = vh * vh;
            uint128_t qh_2 = 2 * qh;
            vh = vh_vh - qh_2;
            if (vh_vh < qh_2) {
                vh += widen_n * ((qh_2 - vh_vh + widen_n - 1) / widen_n);
            }

            vh = (vh % widen_n);
        }
        else {
            /* qh = ql */
            qh = ql;

            /* uh = uh*vl - ql (mod n) */
            uint128_t uh_vl = uh * vl;
            uh = uh_vl - ql;
            if (uh_vl < ql) {
                uh += widen_n * ((ql - uh_vl + widen_n - 1) / widen_n);
            }

            uh = (uh % widen_n);

            /* vh = vh*vl - p*ql (mod n) */
            uint128_t vh_vl = vh * vl;
            uint128_t p_ql = p * ql;
            vh = vh_vl - p_ql;
            if (vh_vl < p_ql) {
                vh += widen_n * ((p_ql - vh_vl + widen_n - 1) / widen_n);
            }

            vh = (vh % widen_n);

            /* vl = vl*vl - 2*ql (mod n) */
            uint128_t vl_vl = vl * vl;
            uint128_t ql_2 = 2 * ql;
            vl = vl_vl - ql_2;
            if (vl_vl < ql_2) {
                vl += widen_n * ((ql_2 - vl_vl + widen_n - 1) / widen_n);
            }

            vl = (vl % widen_n);
        }
    }

    /* ql = ql*qh */
    ql = ((ql * qh) % widen_n);

    /* qh = ql*q */
    qh = ((ql * widen_q) % widen_n);

    /* uh = uh*vl - ql (mod n) */
    uint128_t uh_vl = uh * vl;
    uh = uh_vl - ql;
    if (uh_vl < ql) {
        uh += widen_n * ((ql - uh_vl + widen_n - 1) / widen_n);
    }

    uh = (uh % widen_n);

    /* vl = vh*vl - p*ql (mod n) */
    uint128_t vh_vl = vh * vl;
    uint128_t p_ql = p * ql;
    vl = vh_vl - p_ql;
    if (vh_vl < p_ql) {
        vl += widen_n * ((p_ql - vh_vl + widen_n - 1) / widen_n);
    }

    vl = (vl % widen_n);

    /* ql = ql*qh */
    ql = ((ql * qh) % widen_n);

    /* uh contains LucasU_s and vl contains LucasV_s */
    if (uh == 0 || vl == 0) {
        // || vl == n - 2 || vl == 2 for mpz_extrastronglucas_prp
        return true;
    }

    for (int32_t j = 1; j < r; j++) {
        /* vl = vl*vl - 2*ql (mod n) */
        auto vl_vl = vl * vl;
        auto ql_2 = 2 * ql;
        vl = vl_vl - ql_2;
        if (vl_vl < ql_2) {
            vl += widen_n * ((ql_2 - vl_vl + widen_n - 1) / widen_n);
        }
        vl = vl % widen_n;

        /* ql = ql*ql (mod n) */
        ql = (ql * ql) % widen_n;

        if (vl == 0) {
            return true;
        }
    }

    return false;
}

/* *********************************************************************************************************
 * mpz_strongselfridge_prp:
 * A "strong Lucas-Selfridge probable prime" n is a "strong Lucas probable prime" using Selfridge parameters of:
 * Find the first element D in the sequence {5, -7, 9, -11, 13, ...} such that Jacobi(D,n) = -1
 * Then use P=1 and Q=(1-D)/4 in the strong Lucas probable prime test.
 * Make sure n is not a perfect square, otherwise the search for D will only stop when D=n.
 * **********************************************************************************************************/
static constexpr bool IsStrongSelfridgePRP(uint64_t n) noexcept {
    if (n == 1) {
        return false;
    }
    
    if (n % 2 == 0) {
        return n == 2;
    }

    int32_t d = 5;
    constexpr int32_t MAX_D = 1000000;
    while (true) {
        // Calculate the Jacobi symbol (a/p).
        int32_t jacobi = JacobiSymbol(static_cast<int64_t>(d), n);
        switch (jacobi) {
            /* if jacobi == 0, d is a factor of n, therefore n is composite... */
            /* if d == n, then either n is either prime or 9... */
            case 0:
                return static_cast<uint64_t>(std::abs(d)) == n && n != 9;
            case 1:
                /* if we get to the 5th d, make sure we aren't dealing with a square... */
                if (d == 13 && IsPerfectSquare(n)) {
                    return false;
                }

                d += (d > 0) ? 2 : -2;
                d = -d;

                if (d >= MAX_D) {
                    // throw std::domain_error("Appropriate value for D cannot be found in bool IsStrongSelfridgePRP(uint64_t)");
                    return false;
                }
                break;
            case -1:
                int32_t q = (1 - d) / 4;
                return IsStrongLucasPRP(n, 1, q);
        }
    }
}

/// @brief Realization taken from the sympy and gmpy2 C source code (IsStrongPRP(uint64_t, uint64_t) and IsStrongSelfridgePRP(uint64_t))
///        complexity - O(log(n) ^ 2 * log(log(n)))
/// @param n number to test
/// @return true if is is prime and false otherwise
static constexpr bool IsPrime(uint64_t n) noexcept {
    if ((n % 2 == 0) || (n % 3 == 0) || (n % 5 == 0)){
        return n == 2 || n == 3 || n == 5;
    }

    if (n < 49) {
        return n != 1;
    }

    if ((n %  7) == 0 || (n % 11) == 0 || (n % 13) == 0 || (n % 17) == 0 ||
        (n % 19) == 0 || (n % 23) == 0 || (n % 29) == 0 || (n % 31) == 0 ||
        (n % 37) == 0 || (n % 41) == 0 || (n % 43) == 0 || (n % 47) == 0) {
        return false;
    }

    if (n < 2809) {
        return true;
    }

    if (n < 31417) {
        if (BinPowMod(2, n, n) != 2) {
            return false;
        }

        switch(n) {
            case 7957:
            case 8321:
            case 13747:
            case 18721:
            case 19951:
            case 23377:
                return false;
            default:
                return true;
        }
    }

    return IsStrongPRP(n, 2) && IsStrongSelfridgePRP(n);
}
