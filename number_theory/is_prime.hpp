#ifndef IS_PRIME_BPSW_HPP
#define IS_PRIME_BPSW_HPP

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <numeric>
#include <stdexcept>

#include "../misc/config_macros.hpp"
#include "integers_128_bit.hpp"
#include "kronecker_symbol.hpp"
#include "math_functions.hpp"

namespace math_functions {

using std::int32_t;
using std::int64_t;
using std::uint32_t;
using std::uint64_t;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

namespace detail {

/**********************************************************************************************
 * mpz_sprp: (also called a Miller-Rabin probable prime)
 * A "strong probable prime" to the base a is an odd composite n = (2^r)*s+1
 * with s odd such that either a^s == 1 mod n, or a^((2^t)*s) == -1 mod n, for
 * some integer t, with 0 <= t < r.
 **********************************************************************************************/
template <bool DoBasicChecks = true>
ATTRIBUTE_CONST I128_CONSTEXPR bool is_strong_prp(const uint64_t n,
                                                  const uint64_t a) noexcept(!DoBasicChecks) {
    if constexpr (DoBasicChecks) {
        if (unlikely(a < 2)) {
            throw std::invalid_argument{"is_strong_prp requires 'a' greater than or equal to 2"};
        }

        if (unlikely(n == 1)) {
            return false;
        }

        if (unlikely(n % 2 == 0)) {
            return n == 2;
        }

        if (unlikely(std::gcd(n, a) != 1)) {
            throw std::invalid_argument{"is_strong_prp requires gcd(n, a) == 1"};
        }
    }

    CONFIG_ASSUME_STATEMENT(a >= 2);
    CONFIG_ASSUME_STATEMENT(n % 2 == 1);
    CONFIG_ASSUME_STATEMENT(n >= 3);

    const uint64_t n_minus_1 = n - 1;
    /* Find q and r satisfying: n - 1 = q * (2^r), q odd */
    auto [q, r] = math_functions::extract_pow2(n_minus_1);
    // n - 1 >= 2 => r >= 1
    CONFIG_ASSUME_STATEMENT(r >= 1);
    CONFIG_ASSUME_STATEMENT(q % 2 == 1);
    // Redundant but still
    CONFIG_ASSUME_STATEMENT(q >= 1);

    /* Check a^((2^t)*q) mod n for 0 <= t < r */

    // Init test = ((a^q) mod n)
    uint64_t test = math_functions::bin_pow_mod(a, q, n);
    CONFIG_ASSUME_STATEMENT(test < n);
    if (test == 1 || test == n_minus_1) {
        return true;
    }

    // Since n is odd and n - 1 is even >= 2, initially r > 0.
    while (--r) {
        /* test = (test ^ 2) % n */
        CONFIG_ASSUME_STATEMENT(test < n);
        test = static_cast<uint64_t>((uint128_t{test} * test) % n);
        CONFIG_ASSUME_STATEMENT(test < n);
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
template <bool DoBasicChecks = true>
ATTRIBUTE_CONST I128_CONSTEXPR bool is_strong_lucas_prp(const uint64_t n,
                                                        const uint16_t p,
                                                        const int32_t q) noexcept(!DoBasicChecks) {
    const uint32_t p2 = uint32_t{p} * uint32_t{p};
    const int64_t d = int64_t{p2} - int64_t{q} * 4;
    if constexpr (DoBasicChecks) {
        /* Check if p*p - 4*q == 0. */
        if (unlikely(d == 0)) {
            throw std::invalid_argument{"invalid values for p, q in is_strong_lucas_prp"};
        }

        if (unlikely(n == 1)) {
            return false;
        }

        if (unlikely(n % 2 == 0)) {
            return n == 2;
        }

        // NOLINTNEXTLINE(bugprone-implicit-widening-of-multiplication-result)
        const int128_t rhs = int128_t{int64_t{2} * int64_t{q}} * int128_t{d};
        if (unlikely(math_functions::gcd(n, rhs) != 1)) {
            throw std::invalid_argument{
                "is_strong_lucas_prp requires gcd(n, 2 * q * (p * p - 4 * q)) == 1"};
        }
    }

    CONFIG_ASSUME_STATEMENT(d != 0);
    CONFIG_ASSUME_STATEMENT(n % 2 == 1);
    CONFIG_ASSUME_STATEMENT(n >= 3);

    /* nmj = n - (D/n), where (D/n) is the Jacobi symbol */
    const uint64_t nmj = n - static_cast<uint64_t>(int64_t{math_functions::kronecker_symbol(d, n)});
    CONFIG_ASSUME_STATEMENT(nmj >= 2);

    /* Find s and r satisfying: nmj = s * (2 ^ r), s odd */
    const auto extraction_res = math_functions::extract_pow2(nmj);
    const uint64_t s = extraction_res.odd_part;
    const uint32_t r = extraction_res.power;
    CONFIG_ASSUME_STATEMENT(r >= 1);
    CONFIG_ASSUME_STATEMENT(s % 2 == 1);
    // Redundant but still
    CONFIG_ASSUME_STATEMENT(s >= 1);

    /**
     * make sure U_s == 0 mod n or V_((2^t)*s) == 0 mod n,
     * for some t, 0 <= t < r
     */
    uint64_t uh = 1;            // Initial value for U_1
    uint64_t vl = 2;            // Initial value for V_0
    uint64_t vh = uint64_t{p};  // Initial value for V_1
    uint64_t ql = 1;
    uint64_t qh = 1;
    // q mod n
    const uint64_t widen_q = (q >= 0 ? static_cast<uint32_t>(q)
                                     : (n - static_cast<uint64_t>(-static_cast<uint32_t>(q)) % n)) %
                             n;
    CONFIG_ASSUME_STATEMENT(widen_q < n);
    // n >= 3 => n - 1 >= 2 => n - 1 >= 1 => s >= 1
    for (uint32_t j = math_functions::log2_floor(s); j != 0; j--) {
        CONFIG_ASSUME_STATEMENT(ql < n);
        /* ql = ql*qh (mod n) */
        ql = static_cast<uint64_t>((uint128_t{ql} * qh) % n);
        CONFIG_ASSUME_STATEMENT(ql < n);
        if (s & (uint64_t{1} << j)) {
            CONFIG_ASSUME_STATEMENT(qh < n);
            /* qh = ql*q */
            qh = static_cast<uint64_t>((uint128_t{ql} * widen_q) % n);
            CONFIG_ASSUME_STATEMENT(qh < n);

            CONFIG_ASSUME_STATEMENT(uh < n);
            /* uh = uh*vh (mod n) */
            uh = static_cast<uint64_t>((uint128_t{uh} * vh) % n);
            CONFIG_ASSUME_STATEMENT(uh < n);

            CONFIG_ASSUME_STATEMENT(vl < n);
            /* vl = vh*vl - p*ql (mod n) */
            const uint64_t vh_vl = static_cast<uint64_t>((uint128_t{vh} * vl) % n);
            const uint64_t p_ql = static_cast<uint64_t>((p * uint128_t{ql}) % n);
            CONFIG_ASSUME_STATEMENT(vh_vl < n);
            CONFIG_ASSUME_STATEMENT(p_ql < n);
            const uint64_t tmp_vl = vh_vl - p_ql;
            vl = vh_vl >= p_ql ? tmp_vl : tmp_vl + n;
            CONFIG_ASSUME_STATEMENT(vl < n);

            CONFIG_ASSUME_STATEMENT(vh < n);
            /* vh = vh*vh - 2*qh (mod n) */
            const uint64_t vh_vh = static_cast<uint64_t>((uint128_t{vh} * vh) % n);
            CONFIG_ASSUME_STATEMENT(vh_vh < n);
            const uint128_t tmp_qh_2 = 2 * uint128_t{qh};
            CONFIG_ASSUME_STATEMENT(tmp_qh_2 <= 2 * uint128_t{n - 1});
            const uint64_t qh_2 = tmp_qh_2 < n ? static_cast<uint64_t>(tmp_qh_2)
                                               : static_cast<uint64_t>(tmp_qh_2 - n);
            CONFIG_ASSUME_STATEMENT(qh_2 < n);
            CONFIG_ASSUME_STATEMENT(qh_2 == (uint128_t{qh} * 2) % n);
            const uint64_t tmp_vh = vh_vh - qh_2;
            vh = vh_vh >= qh_2 ? tmp_vh : tmp_vh + n;
            CONFIG_ASSUME_STATEMENT(vh < n);
        } else {
            /* qh = ql */
            qh = ql;

            CONFIG_ASSUME_STATEMENT(uh < n);
            CONFIG_ASSUME_STATEMENT(vl < n);
            /* uh = uh*vl - ql (mod n) */
            const uint64_t uh_vl = static_cast<uint64_t>((uint128_t{uh} * vl) % n);
            CONFIG_ASSUME_STATEMENT(uh_vl < n);
            CONFIG_ASSUME_STATEMENT(ql < n);
            const uint64_t tmp_uh = uh_vl - ql;
            uh = uh_vl >= ql ? tmp_uh : tmp_uh + n;
            CONFIG_ASSUME_STATEMENT(uh < n);

            CONFIG_ASSUME_STATEMENT(vh < n);
            CONFIG_ASSUME_STATEMENT(vl < n);
            /* vh = vh*vl - p*ql (mod n) */
            const uint64_t vh_vl = static_cast<uint64_t>((uint128_t{vh} * vl) % n);
            const uint64_t p_ql = static_cast<uint64_t>((p * uint128_t{ql}) % n);
            CONFIG_ASSUME_STATEMENT(vh_vl < n);
            CONFIG_ASSUME_STATEMENT(p_ql < n);
            const uint64_t tmp_vh = vh_vl - p_ql;
            vh = vh_vl >= p_ql ? tmp_vh : tmp_vh + n;
            CONFIG_ASSUME_STATEMENT(vh < n);

            CONFIG_ASSUME_STATEMENT(vl < n);
            /* vl = vl*vl - 2*ql (mod n) */
            const uint64_t vl_vl = static_cast<uint64_t>((uint128_t{vl} * vl) % n);
            CONFIG_ASSUME_STATEMENT(vl_vl < n);
            const uint128_t tmp_ql_2 = 2 * uint128_t{ql};
            CONFIG_ASSUME_STATEMENT(tmp_ql_2 <= 2 * uint128_t{n - 1});
            const uint64_t ql_2 = tmp_ql_2 < n ? static_cast<uint64_t>(tmp_ql_2)
                                               : static_cast<uint64_t>(tmp_ql_2 - n);
            CONFIG_ASSUME_STATEMENT(ql_2 < n);
            CONFIG_ASSUME_STATEMENT(ql_2 == (uint128_t{ql} * 2) % n);
            const uint64_t tmp_vl = vl_vl - ql_2;
            vl = vl_vl >= ql_2 ? tmp_vl : tmp_vl + n;
            CONFIG_ASSUME_STATEMENT(vl < n);
        }
    }

    CONFIG_ASSUME_STATEMENT(ql < n);
    /* ql = ql*qh */
    ql = static_cast<uint64_t>((uint128_t{ql} * qh) % n);
    CONFIG_ASSUME_STATEMENT(ql < n);

    CONFIG_ASSUME_STATEMENT(qh < n);
    /* qh = ql*q */
    qh = static_cast<uint64_t>((uint128_t{ql} * widen_q) % n);
    CONFIG_ASSUME_STATEMENT(qh < n);

    CONFIG_ASSUME_STATEMENT(uh < n);
    /* uh = uh*vl - ql (mod n) */
    const uint64_t uh_vl = static_cast<uint64_t>((uint128_t{uh} * vl) % n);
    CONFIG_ASSUME_STATEMENT(uh_vl < n);
    CONFIG_ASSUME_STATEMENT(ql < n);
    const uint64_t tmp_uh = uh_vl - ql;
    uh = uh_vl >= ql ? tmp_uh : tmp_uh + n;
    CONFIG_ASSUME_STATEMENT(uh < n);
    CONFIG_ASSUME_STATEMENT(uh == (uint128_t{n} + uh_vl - ql) % n);

    /* uh contains LucasU_s */
    if (uh == 0) {
        return true;
    }

    CONFIG_ASSUME_STATEMENT(vl < n);
    /* vl = vh*vl - p*ql (mod n) */
    const uint64_t vh_vl = static_cast<uint64_t>((uint128_t{vh} * vl) % n);
    const uint64_t p_ql = static_cast<uint64_t>((p * uint128_t{ql}) % n);
    uint64_t tmp_vl = vh_vl - p_ql;
    vl = vh_vl >= p_ql ? tmp_vl : tmp_vl + n;
    CONFIG_ASSUME_STATEMENT(vl < n);
    CONFIG_ASSUME_STATEMENT(vl == (uint128_t{n} + vh_vl - p_ql) % n);

    /* uh contains LucasU_s and vl contains LucasV_s */
    if (vl == 0) {
        // || vl == n - 2 || vl == 2 for mpz_extrastronglucas_prp
        return true;
    }

    CONFIG_ASSUME_STATEMENT(ql < n);
    /* ql = ql*qh */
    ql = static_cast<uint64_t>((uint128_t{ql} * qh) % n);
    CONFIG_ASSUME_STATEMENT(ql < n);
    /* r - 1 for mpz_extrastronglucas_prp */
    for (uint32_t j = 1; j < r; j++) {
        CONFIG_ASSUME_STATEMENT(vl < n);
        /* vl = vl*vl - 2*ql (mod n) */
        const uint64_t vl_vl = static_cast<uint64_t>((uint128_t{vl} * vl) % n);
        CONFIG_ASSUME_STATEMENT(vl_vl < n);
        const uint128_t tmp_ql_2 = 2 * uint128_t{ql};
        CONFIG_ASSUME_STATEMENT(tmp_ql_2 <= 2 * uint128_t{n - 1});
        const uint64_t ql_2 =
            tmp_ql_2 < n ? static_cast<uint64_t>(tmp_ql_2) : static_cast<uint64_t>(tmp_ql_2 - n);
        CONFIG_ASSUME_STATEMENT(ql_2 < n);
        CONFIG_ASSUME_STATEMENT(ql_2 == (uint128_t{ql} * 2) % n);
        tmp_vl = vl_vl - ql_2;
        vl = vl_vl >= ql_2 ? tmp_vl : tmp_vl + n;
        CONFIG_ASSUME_STATEMENT(vl < n);
        CONFIG_ASSUME_STATEMENT(vl == (uint128_t{n} + vl_vl - ql_2) % n);

        if (vl == 0) {
            return true;
        }

        CONFIG_ASSUME_STATEMENT(ql < n);
        /* ql = ql*ql (mod n) */
        ql = static_cast<uint64_t>((uint128_t{ql} * ql) % n);
        CONFIG_ASSUME_STATEMENT(ql < n);
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
template <bool DoBasicChecks = true>
ATTRIBUTE_CONST I128_CONSTEXPR bool is_strong_selfridge_prp(const uint64_t n) noexcept {
    if constexpr (DoBasicChecks) {
        if (unlikely(n == 1)) {
            return false;
        }

        if (unlikely(n % 2 == 0)) {
            return n == 2;
        }
    }

    CONFIG_ASSUME_STATEMENT(n % 2 == 1);
    // Redundant but still
    CONFIG_ASSUME_STATEMENT(n >= 1);
    constexpr int32_t kStep = 2;
    for (int32_t d = 5;; d += (d > 0) ? kStep : -kStep, d = -d) {
        constexpr int32_t kMaxD = 999'997;
        // Calculate the Jacobi symbol (d/n)
        const int32_t jacobi = math_functions::kronecker_symbol(int64_t{d}, n);
        switch (jacobi) {
            /**
             * if jacobi == 0, d is a factor of n, therefore n is composite
             * if d == n, then n is either prime or 9
             */
            case 0: {
                return math_functions::uabs(d) == n && n != 9;
            }
            case 1: {
                /* if we get to the 5th d, make sure we aren't dealing with a
                 * square... */
                if (unlikely(d == 13 && math_functions::is_perfect_square(n))) {
                    return false;
                }

                if (unlikely(d > kMaxD)) {
                    // Appropriate value for D cannot be found in
                    // is_strong_selfridge_prp
                    return false;
                }
                break;
            }
            case -1: {
                CONFIG_ASSUME_STATEMENT(d <= kMaxD + kStep * 2);
                CONFIG_ASSUME_STATEMENT(-kMaxD - kStep <= d);
                CONFIG_ASSUME_STATEMENT((1 - d) % 4 == 0);
                const int32_t q = (1 - d) / 4;
                CONFIG_ASSUME_STATEMENT(1 - 4 * q == d);
                return math_functions::detail::is_strong_lucas_prp<false>(n, 1, q);
            }
            default: {
                // For the analysers
                std::abort();
            }
        }
    }
}

#if CONFIG_HAS_AT_LEAST_CXX_17
inline
#endif
    constexpr uint32_t kIsPrimeSqrtLoopStep = 30;

template <class T>
[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_prime_sqrt_impl(const T n) noexcept {
    if (n % 2 == 0) {
        return n == 2;
    }
    if (n % 3 == 0) {
        return n == 3;
    }
    if (n % 5 == 0) {
        return n == 5;
    }
    // NOLINTNEXTLINE(bugprone-implicit-widening-of-multiplication-result)
    if (unlikely(n < 7 * 7)) {
        return n != 1;
    }

    const auto root = math_functions::isqrt(n);
    using RootType = std::remove_const_t<decltype(root)>;
    static_assert(math_functions::is_unsigned_v<RootType>);

    using DivisorType = std::conditional_t<sizeof(T) >= sizeof(uint64_t), uint64_t, uint32_t>;
    // DivisorType == uint64_t when T == uint64_t because max uint32_t prime is 4294967291
    //  and if we make DivisorType == uint32_t, i will overflow when
    //  root >= 2^32 - kIsPrimeSqrtLoopStep
    //  (which is possible since 2^32 - kIsPrimeSqrtLoopStep < 4294967291)
    DivisorType i = 7u;
    CONFIG_ASSUME_STATEMENT(i <= root);

    const DivisorType max_i = [root]() -> DivisorType {
        if constexpr (sizeof(RootType) == sizeof(uint64_t)) {
            constexpr uint64_t kMaxUInt64Prime = 18446744073709551557ULL;
            static_assert(kMaxUInt64Prime < kMaxUInt64Prime + kIsPrimeSqrtLoopStep, "impl error");
            /**
             * There are no prime numbers on the segment
             * [kMaxUInt64Prime + 1; 2^64 - 1]
             *
             * We do this in order to avoid uint64_t overflow
             * (and endless cycle as a result) if root >= 2^64 - 30
             */
            return std::min(root, kMaxUInt64Prime);
        } else {
            return root;
        }
    }();

    do {
        CONFIG_ASSUME_STATEMENT(i <= max_i);
        if (n % i == 0 || n % (i + 4) == 0 || n % (i + 6) == 0 || n % (i + 10) == 0 ||
            n % (i + 12) == 0 || n % (i + 16) == 0 || n % (i + 22) == 0 || n % (i + 24) == 0) {
            return false;
        }
        i += kIsPrimeSqrtLoopStep;
    } while (i <= max_i);
    return true;
}

}  // namespace detail

/// @brief Complexity - O(log(n) ^ 2 * log(log(n))) ( O(log(n) ^ 3) bit
/// operations )
/// @param n number to test
/// @return true if n is prime and false otherwise
[[nodiscard]] ATTRIBUTE_CONST I128_CONSTEXPR bool is_prime_bpsw(uint64_t n) noexcept {
    if (n % 2 == 0) {
        return n == 2;
    }
    if (n % 3 == 0) {
        return n == 3;
    }
    if (n % 5 == 0) {
        return n == 5;
    }
    // NOLINTNEXTLINE(bugprone-implicit-widening-of-multiplication-result)
    if (unlikely(n < 7 * 7)) {
        return n != 1;
    }
    if ((n % 7) == 0 || (n % 11) == 0 || (n % 13) == 0 || (n % 17) == 0 || (n % 19) == 0 ||
        (n % 23) == 0 || (n % 29) == 0 || (n % 31) == 0 || (n % 37) == 0 || (n % 41) == 0 ||
        (n % 43) == 0 || (n % 47) == 0) {
        return false;
    }
    // NOLINTNEXTLINE(bugprone-implicit-widening-of-multiplication-result)
    if (unlikely(n < 53 * 53)) {
        return true;
    }

    return math_functions::detail::is_strong_prp<false>(n, 2) &&
           math_functions::detail::is_strong_selfridge_prp<false>(n);
}

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_prime_sqrt(const uint32_t n) noexcept {
    return detail::is_prime_sqrt_impl(n);
}

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_prime_sqrt(const uint64_t n) noexcept {
    return detail::is_prime_sqrt_impl(n);
}

[[nodiscard]] ATTRIBUTE_CONST I128_CONSTEXPR bool is_prime_sqrt(const uint128_t n) noexcept {
    return detail::is_prime_sqrt_impl(n);
}

/// @brief Funny realization that works in log(m)
/// @param m
/// @return true if m is prime and false otherwise
[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_prime_u16(const uint16_t m) noexcept {
    const uint32_t n = m;
    if (n % 2 == 0) {
        return n == 2;
    }
    if (n % 3 == 0) {
        return n == 3;
    }
    if (n % 5 == 0) {
        return n == 5;
    }
    // NOLINTNEXTLINE(bugprone-implicit-widening-of-multiplication-result)
    if (n < 7 * 7) {
        return n != 1;
    }
    if ((n % 7) == 0 || (n % 11) == 0 || (n % 13) == 0 || (n % 17) == 0 || (n % 19) == 0 ||
        (n % 23) == 0 || (n % 29) == 0 || (n % 31) == 0 || (n % 37) == 0 || (n % 41) == 0 ||
        (n % 43) == 0 || (n % 47) == 0) {
        return false;
    }
    // NOLINTNEXTLINE(bugprone-implicit-widening-of-multiplication-result)
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
        case 65281: {
            return false;
        }
        default: {
            return math_functions::bin_pow_mod(2u, n - 1, n) == 1;
        }
    }
}

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_mersenne_prime(const uint32_t n) noexcept {
    const auto [q, p] = math_functions::extract_pow2(n + 1);
    if (q != 1) {
        return false;
    }

    switch (p) {
        case 2:
        case 3:
        case 5:
        case 7:
        case 13:
        case 17:
        case 19:
        case 31:
        case 61:
            return true;
        default:
            return false;
    }
}

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_mersenne_prime(const uint64_t n) noexcept {
    const auto [q, p] = math_functions::extract_pow2(n + 1);
    if (q != 1) {
        return false;
    }

    switch (p) {
        case 2:
        case 3:
        case 5:
        case 7:
        case 13:
        case 17:
        case 19:
        case 31:
        case 61:
            return true;
        default:
            return false;
    }
}

[[nodiscard]] ATTRIBUTE_CONST I128_CONSTEXPR bool is_mersenne_prime(const uint128_t n) noexcept {
    const uint128_t np1 = n + 1;
    if (!math_functions::is_power_of_two(np1)) {
        return false;
    }
    const auto [q, p] = math_functions::extract_pow2(np1);
    CONFIG_ASSUME_STATEMENT(q == 1);

    switch (p) {
        case 2:
        case 3:
        case 5:
        case 7:
        case 13:
        case 17:
        case 19:
        case 31:
        case 61:
        case 89:
        case 107:
        case 127:
            return true;
        default:
            return false;
    }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

}  // namespace math_functions

#endif  // !IS_PRIME_BPSW_HPP
