#include <cstdint>
#include <cmath>
#include <numeric>

#include "integers_128_bit.hpp"
#include "jacobi_symbol.hpp"
#include "math_utils.hpp"

/* *********************************************************************************************
 * mpz_sprp: (also called a Miller-Rabin probable prime)
 * A "strong probable prime" to the base a is an odd composite n = (2^r)*s+1 with s odd such that
 * either a^s == 1 mod n, or a^((2^t)*s) == -1 mod n, for some integer t, with 0 <= t < r.
 * *********************************************************************************************/
static constexpr bool IsStrongPRP(uint64_t n, uint64_t a) noexcept {
    if (unlikely(a < 2)) {
        // throw std::domain_error("bool IsStrongPRP(uint64_t, uint64_t) requires 'a' greater than or equal to 2");
        return false;
    }

    if (unlikely(n == 1)) {
        return false;
    }

    if (unlikely(n % 2 == 0)) {
        return n == 2;
    }

    if (unlikely(std::gcd(n, a) != 1)) {
        // throw std::domain_error("bool IsStrongPRP(uint64_t, uint64_t) requires gcd(n,a) == 1");
        return false;
    }

    const auto n_minus_1 = n - 1;
    /* Find s and r satisfying: n - 1 = s * (2^r), s odd */
    int32_t r = 0; uint64_t s = math_utils::find_rs(n_minus_1, r);

    // n - 1 >= 2 => r >= 1

    /* Check a^((2^t)*s) mod n for 0 <= t < r */

    // Init test = ((a^s) mod n)
    uint64_t test = math_utils::bin_pow_mod(a, s, n);
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

/* *********************************************************************************************
 * mpz_stronglucas_prp:
 * A "strong Lucas probable prime" with parameters (P,Q) is a composite n = (2^r)*s+(D/n), where
 * s is odd, D=P^2-4Q, and gcd(n,2QD)=1 such that either U_s == 0 mod n or V_((2^t)*s) == 0 mod n
 * for some t, 0 <= t < r. [(D/n) is the Jacobi symbol]
 * *********************************************************************************************/
static constexpr bool IsStrongLucasPRP(uint64_t n, uint32_t p, int32_t q) noexcept {
    /* Check if p*p - 4*q == 0. */
    int64_t d = static_cast<int64_t>(p) * p - static_cast<int64_t>(q) * 4;
    if (unlikely(d == 0)) {
        // throw std::domain_error("Invalid values for p,q in bool IsStrongLucasPRP(uint64_t, uint32_t, int32_t)");
        return false;
    }

    if (unlikely(n == 1)) {
        return false;
    }

    if (unlikely(n % 2 == 0)) {
        return n == 2;
    }

    uint128_t res = std::gcd(n, 2 * q * static_cast<int128_t>(d));
    if (unlikely(res != n && res != 1)) {
        // throw std::domain_error("IsStrongLucasPRP(uint64_t, uint32_t, int32_t) requires gcd(n, 2 * q * (p * p - 4 * q)) == 1");
        return false;
    }

    /* nmj = n - (D/n), where (D/n) is the Jacobi symbol */
    uint64_t nmj = n - JacobiSymbol(d, n);

    /* Find s and r satisfying: nmj = (2 ^ r) * s, s odd */
    int32_t r = 0; uint64_t s = math_utils::find_rs(nmj, r);

    /* make sure U_s == 0 mod n or V_((2^t)*s) == 0 mod n, for some t, 0 <= t < r */
    uint128_t uh = 1;
    uint128_t vl = 2;
    uint128_t vh = p;
    uint128_t ql = 1;
    uint128_t qh = 1;
    const uint64_t widen_q = static_cast<uint64_t>(q >= 0 ? q : (n - static_cast<uint64_t>(-q)) % n);
    // n >= 3 => n - 1 >= 2 => n - 1 >= 1 => s >= 1 => base_2_digits(s) >= 1
    for (uint32_t j = math_utils::base_2_digits(s) - 1; j != 0; j--) {
        /* ql = ql*qh (mod n) */
        ql = ((ql * qh) % n);
        if (s & (1ull << j)) {
            /* qh = ql*q */
            qh = ((ql * widen_q) % n);

            /* uh = uh*vh (mod n) */
            uh = ((uh * vh) % n);

            /* vl = vh*vl - p*ql (mod n) */
            uint128_t vh_vl = vh * vl;
            uint128_t p_ql = p * ql;
            vl = vh_vl - p_ql;
            if (vh_vl < p_ql) {
                vl += n * ((p_ql - vh_vl + n - 1) / n);
            }

            vl = (vl % n);

            /* vh = vh*vh - 2*qh (mod n) */
            uint128_t vh_vh = vh * vh;
            uint128_t qh_2 = 2 * qh;
            vh = vh_vh - qh_2;
            if (vh_vh < qh_2) {
                vh += n * ((qh_2 - vh_vh + n - 1) / n);
            }

            vh = (vh % n);
        }
        else {
            /* qh = ql */
            qh = ql;

            /* uh = uh*vl - ql (mod n) */
            uint128_t uh_vl = uh * vl;
            uh = uh_vl - ql;
            if (uh_vl < ql) {
                uh += n * ((ql - uh_vl + n - 1) / n);
            }

            uh = (uh % n);

            /* vh = vh*vl - p*ql (mod n) */
            uint128_t vh_vl = vh * vl;
            uint128_t p_ql = p * ql;
            vh = vh_vl - p_ql;
            if (vh_vl < p_ql) {
                vh += n * ((p_ql - vh_vl + n - 1) / n);
            }

            vh = (vh % n);

            /* vl = vl*vl - 2*ql (mod n) */
            uint128_t vl_vl = vl * vl;
            uint128_t ql_2 = 2 * ql;
            vl = vl_vl - ql_2;
            if (vl_vl < ql_2) {
                vl += n * ((ql_2 - vl_vl + n - 1) / n);
            }

            vl = (vl % n);
        }
    }

    /* ql = ql*qh */
    ql = ((ql * qh) % n);

    /* qh = ql*q */
    qh = ((ql * widen_q) % n);

    /* uh = uh*vl - ql (mod n) */
    uint128_t uh_vl = uh * vl;
    uh = uh_vl - ql;
    if (uh_vl < ql) {
        uh += n * ((ql - uh_vl + n - 1) / n);
    }

    uh = (uh % n);

    /* vl = vh*vl - p*ql (mod n) */
    uint128_t vh_vl = vh * vl;
    uint128_t p_ql = p * ql;
    vl = vh_vl - p_ql;
    if (vh_vl < p_ql) {
        vl += n * ((p_ql - vh_vl + n - 1) / n);
    }

    vl = (vl % n);

    /* ql = ql*qh */
    ql = ((ql * qh) % n);

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
            vl += n * ((ql_2 - vl_vl + n - 1) / n);
        }
        vl = vl % n;

        /* ql = ql*ql (mod n) */
        ql = (ql * ql) % n;

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
    if (unlikely(n == 1)) {
        return false;
    }
    
    if (unlikely(n % 2 == 0)) {
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
                if (d == 13 && math_utils::is_perfect_square(n)) {
                    return false;
                }

                d += (d > 0) ? 2 : -2;
                d = -d;

                if (unlikely(d >= MAX_D)) {
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
/// @return true if n is prime and false otherwise
static constexpr bool IsPrime(uint64_t n) noexcept {
    if (n % 2 == 0) {
        return n == 2;
    }
    if (n % 3 == 0) {
        return n == 3;
    }
    if (n % 5 == 0) {
        return n == 5;
    }
    if (n < 7 * 7) {
        return n != 1;
    }
    if ((n %  7) == 0 || (n % 11) == 0 || (n % 13) == 0 || (n % 17) == 0 ||
        (n % 19) == 0 || (n % 23) == 0 || (n % 29) == 0 || (n % 31) == 0 ||
        (n % 37) == 0 || (n % 41) == 0 || (n % 43) == 0 || (n % 47) == 0) {
        return false;
    }
    if (n < 53 * 53) {
        return true;
    }
    if (n < 31417) {
        switch(n) {
            case 7957:
            case 8321:
            case 13747:
            case 18721:
            case 19951:
            case 23377:
                return false;
            default:
                return math_utils::bin_pow_mod(2, uint32_t(n - 1), uint32_t(n)) == 1;
        }
    }

    return IsStrongPRP(n, 2) && IsStrongSelfridgePRP(n);
}

/// @brief Funny realization that works in log(n)
/// @param m 
/// @return true if n is prime and false otherwise
static constexpr bool IsPrimeSmallN(uint16_t m) noexcept {
    uint32_t n = m;
    if (n % 2 == 0) {
        return n == 2;
    }
    if (n % 3 == 0) {
        return n == 3;
    }
    if (n % 5 == 0) {
        return n == 5;
    }
    if (n < 7 * 7) {
        return n != 1;
    }
    if ((n %  7) == 0 || (n % 11) == 0 || (n % 13) == 0 || (n % 17) == 0 ||
        (n % 19) == 0 || (n % 23) == 0 || (n % 29) == 0 || (n % 31) == 0 ||
        (n % 37) == 0 || (n % 41) == 0 || (n % 43) == 0 || (n % 47) == 0) {
        return false;
    }
    if (n < 53 * 53) {
        return true;
    }
    if (n % 53 == 0 || n % 59 == 0) {
        return false;
    }
    switch(n) {
        case 7957:
        case 18721:
        case 19951:
        case 23377:
        case 31417:
        case 31609:
        case 31621:
        case 35333:
        case 42799:
        case 49141:
        case 49981:
        case 60701:
        case 60787:
        case 65281:
            return false;
        default:
            return math_utils::bin_pow_mod(2, n - 1, n) == 1;
    }
}
