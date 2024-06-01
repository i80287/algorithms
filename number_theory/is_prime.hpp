#ifndef IS_PRIME_BPSW_HPP
#define IS_PRIME_BPSW_HPP 1

#include <cstdint>  // std::uint32_t, std::uint64_t
#include <cstdlib>  // std::abs
#include <numeric>  // std::gcd

#include "config_macros.hpp"
#include "integers_128_bit.hpp"
#include "kronecker_symbol.hpp"
#include "math_functions.hpp"

namespace math_functions {

using std::uint32_t;
using std::uint64_t;

/**********************************************************************************************
 * mpz_sprp: (also called a Miller-Rabin probable prime)
 * A "strong probable prime" to the base a is an odd composite n = (2^r)*s+1
 * with s odd such that either a^s == 1 mod n, or a^((2^t)*s) == -1 mod n, for
 * some integer t, with 0 <= t < r.
 **********************************************************************************************/
template <bool BasicChecks = true>
ATTRIBUTE_CONST static constexpr bool is_strong_prp(uint64_t n, uint64_t a) noexcept {
    if constexpr (BasicChecks) {
        if (unlikely(a < 2)) {
            // is_strong_prp requires 'a' greater than or equal to 2
            a = 2;
        }

        if (unlikely(n == 1)) {
            return false;
        }

        if (unlikely(n % 2 == 0)) {
            return n == 2;
        }

        if (unlikely(std::gcd(n, a) != 1)) {
            // is_strong_prp requires gcd(n,a) == 1
            return false;
        }
    }

    ATTRIBUTE_ASSUME(a >= 2);
    ATTRIBUTE_ASSUME(n % 2 == 1);
    ATTRIBUTE_ASSUME(n >= 3);

    const uint64_t n_minus_1 = n - 1;
    /* Find q and r satisfying: n - 1 = q * (2^r), q odd */
    auto [q, r] = math_functions::extract_pow2(n_minus_1);
    // n - 1 >= 2 => r >= 1
    ATTRIBUTE_ASSUME(r >= 1);
    ATTRIBUTE_ASSUME(q % 2 == 1);
    // Redundant but still
    ATTRIBUTE_ASSUME(q >= 1);

    /* Check a^((2^t)*q) mod n for 0 <= t < r */

    // Init test = ((a^q) mod n)
    uint64_t test = math_functions::bin_pow_mod(a, q, n);
    ATTRIBUTE_ASSUME(test < n);
    if (test == 1 || test == n_minus_1) {
        return true;
    }

    // Since n is odd and n - 1 is even >= 2, initially r > 0.
    while (--r) {
        /* test = (test ^ 2) % n */
        ATTRIBUTE_ASSUME(test < n);
        test = uint64_t((uint128_t(test) * test) % n);
        ATTRIBUTE_ASSUME(test < n);
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
ATTRIBUTE_CONST static constexpr bool is_strong_lucas_prp(uint64_t n, uint32_t p,
                                                          int32_t q) noexcept {
    int64_t d = int64_t(uint64_t(p) * p) - int64_t(q) * 4;
    if constexpr (BasicChecks) {
        /* Check if p*p - 4*q == 0. */
        if (unlikely(d == 0)) {
            // Invalid values for p,q in is_strong_lucas_prp
            return false;
        }

        if (unlikely(n == 1)) {
            return false;
        }

        if (unlikely(n % 2 == 0)) {
            return n == 2;
        }

        if (unlikely(std::gcd(n, 2 * q * static_cast<int128_t>(d)) != 1)) {
            // is_strong_lucas_prp requires gcd(n, 2 * q * (p * p - 4 * q)) = 1
            return false;
        }
    }

    ATTRIBUTE_ASSUME(d != 0);
    ATTRIBUTE_ASSUME(n % 2 == 1);
    ATTRIBUTE_ASSUME(n >= 3);

    /* nmj = n - (D/n), where (D/n) is the Jacobi symbol */
    uint64_t nmj = n - uint64_t(int64_t(kronecker_symbol(d, n)));
    ATTRIBUTE_ASSUME(nmj >= 2);

    /* Find s and r satisfying: nmj = s * (2 ^ r), s odd */
    const auto [s, r] = math_functions::extract_pow2(nmj);
    ATTRIBUTE_ASSUME(r >= 1);
    ATTRIBUTE_ASSUME(s % 2 == 1);
    // Redundant but still
    ATTRIBUTE_ASSUME(s >= 1);

    /**
     * make sure U_s == 0 mod n or V_((2^t)*s) == 0 mod n,
     * for some t, 0 <= t < r
     */
    uint64_t uh = 1;  // Initial value for U_1
    uint64_t vl = 2;  // Initial value for V_0
    uint64_t vh = p;  // Initial value for V_1
    uint64_t ql = 1;
    uint64_t qh = 1;
    // q mod n
    const uint64_t widen_q = (q >= 0 ? uint32_t(q) : (n - (uint64_t(-uint32_t(q)) % n))) % n;
    ATTRIBUTE_ASSUME(widen_q < n);
    // n >= 3 => n - 1 >= 2 => n - 1 >= 1 => s >= 1
    for (uint32_t j = math_functions::log2_floor(s); j != 0; j--) {
        ATTRIBUTE_ASSUME(ql < n);
        /* ql = ql*qh (mod n) */
        ql = uint64_t((uint128_t(ql) * qh) % n);
        ATTRIBUTE_ASSUME(ql < n);
        if (s & (uint64_t(1) << j)) {
            ATTRIBUTE_ASSUME(qh < n);
            /* qh = ql*q */
            qh = uint64_t((uint128_t(ql) * widen_q) % n);
            ATTRIBUTE_ASSUME(qh < n);

            ATTRIBUTE_ASSUME(uh < n);
            /* uh = uh*vh (mod n) */
            uh = uint64_t((uint128_t(uh) * vh) % n);
            ATTRIBUTE_ASSUME(uh < n);

            ATTRIBUTE_ASSUME(vl < n);
            /* vl = vh*vl - p*ql (mod n) */
            uint64_t vh_vl = uint64_t((uint128_t(vh) * vl) % n);
            uint64_t p_ql  = uint64_t((p * uint128_t(ql)) % n);
            ATTRIBUTE_ASSUME(vh_vl < n);
            ATTRIBUTE_ASSUME(p_ql < n);
            uint64_t tmp_vl = vh_vl - p_ql;
            vl              = vh_vl >= p_ql ? tmp_vl : tmp_vl + n;
            ATTRIBUTE_ASSUME(vl < n);

            ATTRIBUTE_ASSUME(vh < n);
            /* vh = vh*vh - 2*qh (mod n) */
            uint64_t vh_vh = uint64_t((uint128_t(vh) * vh) % n);
            ATTRIBUTE_ASSUME(vh_vh < n);
            uint128_t tmp_qh_2 = 2 * uint128_t(qh);
            ATTRIBUTE_ASSUME(tmp_qh_2 <= 2 * uint128_t(n - 1));
            uint64_t qh_2 = tmp_qh_2 < n ? uint64_t(tmp_qh_2) : uint64_t(tmp_qh_2 - n);
            ATTRIBUTE_ASSUME(qh_2 < n);
            ATTRIBUTE_ASSUME(qh_2 == (uint128_t(qh) * 2) % n);
            uint64_t tmp_vh = vh_vh - qh_2;
            vh              = vh_vh >= qh_2 ? tmp_vh : tmp_vh + n;
            ATTRIBUTE_ASSUME(vh < n);
        } else {
            /* qh = ql */
            qh = ql;

            ATTRIBUTE_ASSUME(uh < n);
            ATTRIBUTE_ASSUME(vl < n);
            /* uh = uh*vl - ql (mod n) */
            uint64_t uh_vl = uint64_t((uint128_t(uh) * vl) % n);
            ATTRIBUTE_ASSUME(uh_vl < n);
            ATTRIBUTE_ASSUME(ql < n);
            uint64_t tmp_uh = uh_vl - ql;
            uh              = uh_vl >= ql ? tmp_uh : tmp_uh + n;
            ATTRIBUTE_ASSUME(uh < n);

            ATTRIBUTE_ASSUME(vh < n);
            ATTRIBUTE_ASSUME(vl < n);
            /* vh = vh*vl - p*ql (mod n) */
            uint64_t vh_vl = uint64_t((uint128_t(vh) * vl) % n);
            uint64_t p_ql  = uint64_t((p * uint128_t(ql)) % n);
            ATTRIBUTE_ASSUME(vh_vl < n);
            ATTRIBUTE_ASSUME(p_ql < n);
            uint64_t tmp_vh = vh_vl - p_ql;
            vh              = vh_vl >= p_ql ? tmp_vh : tmp_vh + n;
            ATTRIBUTE_ASSUME(vh < n);

            ATTRIBUTE_ASSUME(vl < n);
            /* vl = vl*vl - 2*ql (mod n) */
            uint64_t vl_vl = uint64_t((uint128_t(vl) * vl) % n);
            ATTRIBUTE_ASSUME(vl_vl < n);
            uint128_t tmp_ql_2 = 2 * uint128_t(ql);
            ATTRIBUTE_ASSUME(tmp_ql_2 <= 2 * uint128_t(n - 1));
            uint64_t ql_2 = tmp_ql_2 < n ? uint64_t(tmp_ql_2) : uint64_t(tmp_ql_2 - n);
            ATTRIBUTE_ASSUME(ql_2 < n);
            ATTRIBUTE_ASSUME(ql_2 == (uint128_t(ql) * 2) % n);
            uint64_t tmp_vl = vl_vl - ql_2;
            vl              = vl_vl >= ql_2 ? tmp_vl : tmp_vl + n;
            ATTRIBUTE_ASSUME(vl < n);
        }
    }

    ATTRIBUTE_ASSUME(ql < n);
    /* ql = ql*qh */
    ql = uint64_t((uint128_t(ql) * qh) % n);
    ATTRIBUTE_ASSUME(ql < n);

    ATTRIBUTE_ASSUME(qh < n);
    /* qh = ql*q */
    qh = uint64_t((uint128_t(ql) * widen_q) % n);
    ATTRIBUTE_ASSUME(qh < n);

    ATTRIBUTE_ASSUME(uh < n);
    /* uh = uh*vl - ql (mod n) */
    uint64_t uh_vl = uint64_t((uint128_t(uh) * vl) % n);
    ATTRIBUTE_ASSUME(uh_vl < n);
    ATTRIBUTE_ASSUME(ql < n);
    uint64_t tmp_uh = uh_vl - ql;
    uh              = uh_vl >= ql ? tmp_uh : tmp_uh + n;
    ATTRIBUTE_ASSUME(uh < n);
    ATTRIBUTE_ASSUME(uh == (uint128_t(n) + uh_vl - ql) % n);

    /* uh contains LucasU_s */
    if (uh == 0) {
        return true;
    }

    ATTRIBUTE_ASSUME(vl < n);
    /* vl = vh*vl - p*ql (mod n) */
    uint64_t vh_vl  = uint64_t((uint128_t(vh) * vl) % n);
    uint64_t p_ql   = uint64_t((p * uint128_t(ql)) % n);
    uint64_t tmp_vl = vh_vl - p_ql;
    vl              = vh_vl >= p_ql ? tmp_vl : tmp_vl + n;
    ATTRIBUTE_ASSUME(vl < n);
    ATTRIBUTE_ASSUME(vl == (uint128_t(n) + vh_vl - p_ql) % n);

    /* uh contains LucasU_s and vl contains LucasV_s */
    if (vl == 0) {
        // || vl == n - 2 || vl == 2 for mpz_extrastronglucas_prp
        return true;
    }

    ATTRIBUTE_ASSUME(ql < n);
    /* ql = ql*qh */
    ql = uint64_t((uint128_t(ql) * qh) % n);
    ATTRIBUTE_ASSUME(ql < n);

    for (uint32_t j = 1; j < r /* r - 1 for mpz_extrastronglucas_prp */; j++) {
        ATTRIBUTE_ASSUME(vl < n);
        /* vl = vl*vl - 2*ql (mod n) */
        uint64_t vl_vl = uint64_t((uint128_t(vl) * vl) % n);
        ATTRIBUTE_ASSUME(vl_vl < n);
        uint128_t tmp_ql_2 = 2 * uint128_t(ql);
        ATTRIBUTE_ASSUME(tmp_ql_2 <= 2 * uint128_t(n - 1));
        uint64_t ql_2 = tmp_ql_2 < n ? uint64_t(tmp_ql_2) : uint64_t(tmp_ql_2 - n);
        ATTRIBUTE_ASSUME(ql_2 < n);
        ATTRIBUTE_ASSUME(ql_2 == (uint128_t(ql) * 2) % n);
        tmp_vl = vl_vl - ql_2;
        vl     = vl_vl >= ql_2 ? tmp_vl : tmp_vl + n;
        ATTRIBUTE_ASSUME(vl < n);
        ATTRIBUTE_ASSUME(vl == (uint128_t(n) + vl_vl - ql_2) % n);

        if (vl == 0) {
            return true;
        }

        ATTRIBUTE_ASSUME(ql < n);
        /* ql = ql*ql (mod n) */
        ql = uint64_t((uint128_t(ql) * ql) % n);
        ATTRIBUTE_ASSUME(ql < n);
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
ATTRIBUTE_CONST static constexpr bool is_strong_selfridge_prp(uint64_t n) noexcept {
    if constexpr (BasicChecks) {
        if (unlikely(n == 1)) {
            return false;
        }

        if (unlikely(n % 2 == 0)) {
            return n == 2;
        }
    }

    ATTRIBUTE_ASSUME(n % 2 == 1);
    // Redundant but still
    ATTRIBUTE_ASSUME(n >= 1);
    for (int32_t d = 5;; d += (d > 0) ? 2 : -2, d = -d) {
        // Calculate the Jacobi symbol (d/n)
        const int32_t jacobi = kronecker_symbol(int64_t(d), n);
        ATTRIBUTE_ASSUME(jacobi == -1 || jacobi == 0 || jacobi == 1);
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
                if (unlikely(d == 13 && math_functions::is_perfect_square(n))) {
                    return false;
                }

                if (unlikely(d >= 1000000)) {
                    // Appropriate value for D cannot be found in
                    // is_strong_selfridge_prp
                    return false;
                }
                break;
            case -1: {
                ATTRIBUTE_ASSUME((1 - d) % 4 == 0);
                int32_t q = (1 - d) / 4;
                ATTRIBUTE_ASSUME(1 - 4 * q == d);
                return is_strong_lucas_prp<false>(n, 1, q);
            }
        }
    }
}

/// @brief Complexity - O(log(n) ^ 2 * log(log(n))) ( O(log(n) ^ 3) bit
/// operations )
/// @param n number to test
/// @return true if n is prime and false otherwise
ATTRIBUTE_CONST static constexpr bool is_prime_bpsw(uint64_t n) noexcept {
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
    if ((n % 7) == 0 || (n % 11) == 0 || (n % 13) == 0 || (n % 17) == 0 || (n % 19) == 0 ||
        (n % 23) == 0 || (n % 29) == 0 || (n % 31) == 0 || (n % 37) == 0 || (n % 41) == 0 ||
        (n % 43) == 0 || (n % 47) == 0) {
        return false;
    }
    if (unlikely(n < 53 * 53)) {
        return true;
    }

    return is_strong_prp<false>(n, 2) && is_strong_selfridge_prp<false>(n);
}

ATTRIBUTE_CONST static constexpr bool is_prime_sqrt(uint32_t n) noexcept {
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
    uint32_t i          = 7;
    const uint32_t root = math_functions::isqrt(n);
    do {
        if (n % i == 0 || n % (i + 4) == 0 || n % (i + 6) == 0 || n % (i + 10) == 0 ||
            n % (i + 12) == 0 || n % (i + 16) == 0 || n % (i + 22) == 0 || n % (i + 24) == 0) {
            return false;
        }
        i += 30;
    } while (i <= root);
    return true;
}

ATTRIBUTE_CONST static constexpr bool is_prime_sqrt(uint64_t n) noexcept {
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
    uint64_t i          = 7;
    const uint64_t root = math_functions::isqrt(n);
    do {
        if (n % i == 0 || n % (i + 4) == 0 || n % (i + 6) == 0 || n % (i + 10) == 0 ||
            n % (i + 12) == 0 || n % (i + 16) == 0 || n % (i + 22) == 0 || n % (i + 24) == 0) {
            return false;
        }
        i += 30;
    } while (i <= root);
    return true;
}

ATTRIBUTE_CONST static constexpr bool is_prime_sqrt(uint128_t n) noexcept {
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
    uint64_t i                   = 7;
    constexpr uint64_t kMaxPrime = 18446744073709551557ull;
    static_assert(kMaxPrime < kMaxPrime + 30, "");
    /**
     * There are no prime numbers on the segment
     * [18446744073709551558; 2^64 - 1]
     *
     * We do this in order to avoid uint64_t overflow
     * (and endless cycle as a result) if root >= 2^64 - 30
     */
    const uint64_t root = std::min(math_functions::isqrt(n), kMaxPrime);

    do {
        if (n % i == 0 || n % (i + 4) == 0 || n % (i + 6) == 0 || n % (i + 10) == 0 ||
            n % (i + 12) == 0 || n % (i + 16) == 0 || n % (i + 22) == 0 || n % (i + 24) == 0) {
            return false;
        }
        i += 30;
    } while (i <= root);

    return true;
}

/// @brief Funny realization that works in log(n)
/// @param m
/// @return true if n is prime and false otherwise
ATTRIBUTE_CONST static constexpr bool is_prime_u16(uint16_t m) noexcept {
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
    if ((n % 7) == 0 || (n % 11) == 0 || (n % 13) == 0 || (n % 17) == 0 || (n % 19) == 0 ||
        (n % 23) == 0 || (n % 29) == 0 || (n % 31) == 0 || (n % 37) == 0 || (n % 41) == 0 ||
        (n % 43) == 0 || (n % 47) == 0) {
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
            return math_functions::bin_pow_mod(2, n - 1, n) == 1;
    }
}

}  // namespace math_functions

#endif  // !IS_PRIME_BPSW_HPP
