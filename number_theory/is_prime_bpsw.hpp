#ifndef IS_PRIME_BPSW
#define IS_PRIME_BPSW 1

#include <cstdlib> // std::abs
#include <cstdint>
#include <numeric> // std::gcd

#include "config_macros.hpp"
#include "integers_128_bit.hpp"
#include "jacobi_symbol.hpp"
#include "math_utils.hpp"

/**********************************************************************************************
 * mpz_sprp: (also called a Miller-Rabin probable prime)
 * A "strong probable prime" to the base a is an odd composite n = (2^r)*s+1
 * with s odd such that either a^s == 1 mod n, or a^((2^t)*s) == -1 mod n, for
 * some integer t, with 0 <= t < r.
 **********************************************************************************************/
template <bool BasicChecks = true>
gcc_attribute_const static constexpr bool IsStrongPRP(uint64_t n,
                                                      uint64_t a) noexcept {
    if constexpr (BasicChecks) {
        if (unlikely(a < 2)) {
            // IsStrongPRP requires 'a' greater than or equal to 2
            a = 2;
        }

        if (unlikely(n == 1)) {
            return false;
        }

        if (unlikely(n % 2 == 0)) {
            return n == 2;
        }

        if (unlikely(std::gcd(n, a) != 1)) {
            // IsStrongPRP requires gcd(n,a) == 1
            return false;
        }
    }

    attribute_assume(a >= 2);
    attribute_assume(n % 2 == 1);
    attribute_assume(n >= 3);

    const uint64_t n_minus_1 = n - 1;
    /* Find q and r satisfying: n - 1 = q * (2^r), q odd */
    auto [q, r] = math_utils::extract_2pow(n_minus_1);
    // n - 1 >= 2 => r >= 1
    attribute_assume(r >= 1);
    attribute_assume(q % 2 == 1);
    // Redundant but still
    attribute_assume(q >= 1);

    /* Check a^((2^t)*q) mod n for 0 <= t < r */

    // Init test = ((a^q) mod n)
    uint64_t test = math_utils::bin_pow_mod(a, q, n);
    if (test == 1 || test == n_minus_1) {
        return true;
    }

    // Since n is odd and n - 1 is even >= 2, initially r > 0.
    while (--r) {
        /* test = (test ^ 2) % n */
        test = static_cast<uint64_t>((uint128_t(test) * test) % n);
        if (test == n_minus_1) {
            return true;
        }
    }

    return false;
}

/**********************************************************************************************
 * mpz_stronglucas_prp:
 * A "strong Lucas probable prime" with parameters (P,Q) is a composite n =
 * (2^r)*s+(D/n), where s is odd, D=P^2-4Q, and gcd(n,2QD)=1 such that either
 * U_s == 0 mod n or V_((2^t)*s) == 0 mod n for some t, 0 <= t < r. [(D/n) is
 * the Jacobi symbol]
 **********************************************************************************************/
template <bool BasicChecks = true>
gcc_attribute_const static constexpr bool IsStrongLucasPRP(uint64_t n,
                                                           uint32_t p,
                                                           int32_t q) noexcept {
    int64_t d = int64_t(uint64_t(p) * p) - static_cast<int64_t>(q) * 4;
    if constexpr (BasicChecks) {
        /* Check if p*p - 4*q == 0. */
        if (unlikely(d == 0)) {
            // Invalid values for p,q in IsStrongLucasPRP
            return false;
        }

        if (unlikely(n == 1)) {
            return false;
        }

        if (unlikely(n % 2 == 0)) {
            return n == 2;
        }

        if (unlikely(std::gcd(n, 2 * q * static_cast<int128_t>(d)) != 1)) {
            // IsStrongLucasPRP requires gcd(n, 2 * q * (p * p - 4 * q)) = 1
            return false;
        }
    }

    attribute_assume(d != 0);
    attribute_assume(n % 2 == 1);
    attribute_assume(n >= 3);

    /* nmj = n - (D/n), where (D/n) is the Jacobi symbol */
    uint64_t nmj = n - uint64_t(int64_t(JacobiSymbol(d, n)));
    attribute_assume(nmj >= 2);

    /* Find s and r satisfying: nmj = s * (2 ^ r), s odd */
    auto [s, r] = math_utils::extract_2pow(nmj);
    attribute_assume(r >= 1);
    attribute_assume(s % 2 == 1);
    // Redundant but still
    attribute_assume(s >= 1);

    /* make sure U_s == 0 mod n or V_((2^t)*s) == 0 mod n, for some t, 0 <= t <
     * r */
    uint128_t uh = 1;
    uint128_t vl = 2;
    uint128_t vh = p;
    uint128_t ql = 1;
    uint128_t qh = 1;
    const uint64_t widen_q =
        uint64_t(q >= 0 ? uint32_t(q) : (n - uint32_t(-q)) % n);
    // n >= 3 => n - 1 >= 2 => n - 1 >= 1 => s >= 1
    for (uint32_t j = math_utils::log2_floor(s); j != 0; j--) {
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
        } else {
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

    // Unrolled loop for j = 0 (s & (1 << j) = s % 2 = 1)

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

    for (uint32_t j = 1; j < r; j++) {
        /* vl = vl*vl - 2*ql (mod n) */
        auto vl_vl = vl * vl;
        auto ql_2 = 2 * ql;
        vl = vl_vl - ql_2;
        if (vl_vl < ql_2) {
            vl += n * ((ql_2 - vl_vl + n - 1) / n);
        }
        vl = vl % n;

        if (vl == 0) {
            return true;
        }

        /* ql = ql*ql (mod n) */
        ql = (ql * ql) % n;
    }

    return false;
}

/**********************************************************************************************************
 * mpz_strongselfridge_prp:
 * A "strong Lucas-Selfridge probable prime" n is a "strong Lucas probable
 *prime" using Selfridge parameters of: Find the first element D in the sequence
 * {5, -7, 9, -11, 13, ...} such that Jacobi(D,n) = -1 Then use P=1 and
 * Q=(1-D)/4 in the strong Lucas probable prime test. Make sure n is not a
 * perfect square, otherwise the search for D will only stop when D=n.
 ***********************************************************************************************************/
template <bool BasicChecks = true>
gcc_attribute_const static constexpr bool IsStrongSelfridgePRP(
    uint64_t n) noexcept {
    if constexpr (BasicChecks) {
        if (unlikely(n == 1)) {
            return false;
        }

        if (unlikely(n % 2 == 0)) {
            return n == 2;
        }
    }

    attribute_assume(n % 2 == 1);
    // Redundant but still
    attribute_assume(n >= 1);
    for (int32_t d = 5;; d += (d > 0) ? 2 : -2, d = -d) {
        // Calculate the Jacobi symbol (d/n)
        const int32_t jacobi = JacobiSymbol(int64_t(d), n);
        attribute_assume(jacobi == -1 || jacobi == 0 || jacobi == 1);
        switch (jacobi) {
            /**
             * if jacobi == 0, d is a factor of n, therefore n is composite
             * if d == n, then n is either prime or 9
             */
            case 0:
                return uint32_t(std::abs(d)) == n && n != 9;
            case 1:
                /* if we get to the 5th d, make sure we aren't dealing with a
                 * square... */
                if (unlikely(d == 13 && math_utils::is_perfect_square(n))) {
                    return false;
                }

                if (unlikely(d >= 1000000)) {
                    // Appropriate value for D cannot be found in
                    // IsStrongSelfridgePRP
                    return false;
                }
                break;
            case -1: {
                attribute_assume((1 - d) % 4 == 0);
                int32_t q = (1 - d) / 4;
                attribute_assume(1 - 4 * q == d);
                return IsStrongLucasPRP<false>(n, 1, q);
            }
        }
    }
}

/// @brief Complexity - O(log(n) ^ 2 * log(log(n))) ( O(log(n) ^ 3) bit
/// operations )
/// @param n number to test
/// @return true if n is prime and false otherwise
gcc_attribute_const static constexpr bool IsPrime(uint64_t n) noexcept {
    if (n % 2 == 0) {
        return n == 2;
    }

    if (n % 3 == 0) {
        return n == 3;
    }
    if (n % 5 == 0) {
        return n == 5;
    }
    if (unlikely(n < 7 * 7)) {
        return n != 1;
    }
    if ((n % 7) == 0 || (n % 11) == 0 || (n % 13) == 0 || (n % 17) == 0 ||
        (n % 19) == 0 || (n % 23) == 0 || (n % 29) == 0 || (n % 31) == 0 ||
        (n % 37) == 0 || (n % 41) == 0 || (n % 43) == 0 || (n % 47) == 0) {
        return false;
    }
    if (unlikely(n < 53 * 53)) {
        return true;
    }
    if (n < 31417) {
        switch (n) {
            case 7957:
            case 8321:
            case 13747:
            case 18721:
            case 19951:
            case 23377:
                return false;
            default:
                return math_utils::bin_pow_mod(2, uint32_t(n - 1),
                                               uint32_t(n)) == 1;
        }
    }

    return IsStrongPRP<false>(n, 2) && IsStrongSelfridgePRP<false>(n);
}

/// @brief Funny realization that works in log(n)
/// @param m
/// @return true if n is prime and false otherwise
gcc_attribute_const static constexpr bool IsPrimeSmallN(uint16_t m) noexcept {
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
    if ((n % 7) == 0 || (n % 11) == 0 || (n % 13) == 0 || (n % 17) == 0 ||
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
    switch (n) {
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

#endif  // !IS_PRIME_BPSW
