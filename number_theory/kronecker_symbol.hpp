#ifndef KRONECKER_SYMBOL_HPP
#define KRONECKER_SYMBOL_HPP

#include <type_traits>

#include "integers_128_bit.hpp"
#include "math_functions.hpp"
namespace math_functions {

namespace detail {

template <typename Uint>
#if __cplusplus >= 202002L
    requires type_traits_helper_int128_t::is_unsigned_v<Uint>
#endif
ATTRIBUTE_CONST static constexpr int32_t kronecker_symb_ui(Uint a, Uint n) noexcept {
    int32_t t = 1;

    if (n % 2 == 0) {
        // Work out the (a/2)

        if (unlikely(n == 0)) {
            return a == 1;
        }

        auto [q, p] = extract_pow2(n);
        ATTRIBUTE_ASSUME(q % 2 == 1);
        n = q;

        switch (a % 8) {
            case 0:
            case 2:
            case 4:
            case 6:
                // a % 2 == 0
                // t = 0, but we return t or 0 so we can return 0 right here
                return 0;
            case 3:
            case 5:
                // a === +-3 (mod 8)
                // t = (-1) ^ p
                t -= int32_t((p % 2) * 2);
                ATTRIBUTE_ASSUME(t == -1 || t == 1);
                break;
            case 1:
            case 7:
                // a === +-1 (mod 8)
                // t = 1
                break;
        }
    }

    ATTRIBUTE_ASSUME(n % 2 == 1);
    // Redundant but still
    ATTRIBUTE_ASSUME(n != 0);
    // step 1
    a %= n;
    Uint r = 0;
    // step 3
    while (a != 0) {
        // step 2
        if (a % 2 == 0) {
            auto [a_odd_part, a_exp] = math_functions::extract_pow2(a);
            a                        = a_odd_part;
            r                        = n % 8;
            ATTRIBUTE_ASSUME(r <= 7);
            switch (r) {
                case 3:
                case 5:
                    t = a_exp % 2 == 0 ? t : -t;
                    break;
            }
        }

        // step 4
        r = n;
        n = a;
        a = r;
        if (a % 4 == 3 && n % 4 == 3) {
            t = -t;
        }
        a %= n;
    }

    ATTRIBUTE_ASSUME(t == -1 || t == 1);
    return (n == 1) ? t : 0;
}

template <typename Sint>
#if __cplusplus >= 202002L
    requires type_traits_helper_int128_t::is_signed_v<Sint>
#endif
ATTRIBUTE_CONST static constexpr int32_t kronecker_symbol_si(Sint a, Sint n) noexcept {
    bool carry = n < 0 && a < 0;
    using Uint = type_traits_helper_int128_t::make_unsigned_t<Sint>;
    Uint n_u   = static_cast<Uint>(uabs(n));

    int32_t t = 1;
    if (n_u % 2 == 0) {
        // Work out the (a/2)

        if (unlikely(n_u == 0)) {
            return a == 1 || a == -1;
        }

        auto [q, p] = extract_pow2(n_u);
        ATTRIBUTE_ASSUME(q % 2 == 1);
        n_u = q;

        switch (uint32_t((a % 8) + 8) % 8) {
            case 0:
            case 2:
            case 4:
            case 6:
                // a % 2 == 0
                // t = 0, but we return t or 0 so we can return 0 right here
                return 0;
            case 3:
            case 5:
                // a === +-3 (mod 8)
                // t = (-1) ^ p
                t -= int32_t((p % 2) * 2);
                ATTRIBUTE_ASSUME(t == -1 || t == 1);
                break;
            case 1:
            case 7:
                // a === +-1 (mod 8)
                // t = 1
                break;
        }
    }

    ATTRIBUTE_ASSUME(n_u % 2 == 1);
    // Redundant but still
    ATTRIBUTE_ASSUME(n_u > 0);
    // step 1
    Uint a_u = (static_cast<Uint>(a % Sint(n_u)) + n_u) % n_u;
    Uint r   = 0;
    // step 3
    while (a_u != 0) {
        // step 2
        if (a_u % 2 == 0) {
            auto [a_u_odd_part, a_u_exp] = math_functions::extract_pow2(a_u);
            a_u                          = a_u_odd_part;
            r                            = n_u % 8;
            ATTRIBUTE_ASSUME(r <= 7);
            switch (r) {
                case 3:
                case 5:
                    t = a_u_exp % 2 == 0 ? t : -t;
                    break;
            }
        }

        // step 4
        r   = n_u;
        n_u = a_u;
        a_u = r;
        if (a_u % 4 == 3 && n_u % 4 == 3) {
            t = -t;
        }
        a_u %= n_u;
    }

    ATTRIBUTE_ASSUME(t == -1 || t == 1);
    return (n_u == 1) ? (!carry ? t : -t) : 0;
}

template <class Sint, class Uint>
#if __cplusplus >= 202002L
    requires type_traits_helper_int128_t::is_signed_v<Sint> &&
             type_traits_helper_int128_t::is_unsigned_v<Uint>
#endif
ATTRIBUTE_CONST static constexpr int32_t kronecker_symbol_ui(Sint a, Uint n) noexcept {
    int32_t t = 1;

    if (n % 2 == 0) {
        // Work out the (a/2)

        if (unlikely(n == 0)) {
            return a == 1 || a == -1;
        }

        auto [q, p] = extract_pow2(n);
        ATTRIBUTE_ASSUME(q % 2 == 1);
        n = q;

        switch (uint32_t((a % 8) + 8) % 8) {
            case 0:
            case 2:
            case 4:
            case 6:
                // a % 2 == 0
                // t = 0, but we return t or 0 so we can return 0 right here
                return 0;
            case 3:
            case 5:
                // a === +-3 (mod 8)
                // t = (-1) ^ p
                t -= int32_t((p % 2) * 2);
                ATTRIBUTE_ASSUME(t == -1 || t == 1);
                break;
            case 1:
            case 7:
                // a === +-1 (mod 8)
                // t = 1
                break;
        }
    }

    ATTRIBUTE_ASSUME(n % 2 == 1);
    // Redundant but still
    ATTRIBUTE_ASSUME(n != 0);
    // step 1
    //  a_u = a mod n
    Uint a_u = (a >= 0 ? Uint(a) : (n - (-Uint(a)) % n)) % n;
    Uint r   = 0;
    // step 3
    while (a_u != 0) {
        // step 2
        if (a_u % 2 == 0) {
            auto [a_u_odd_part, a_u_exp] = math_functions::extract_pow2(a_u);
            a_u                          = a_u_odd_part;
            r                            = n % 8;
            ATTRIBUTE_ASSUME(r <= 7);
            switch (r) {
                case 3:
                case 5:
                    t = a_u_exp % 2 == 0 ? t : -t;
                    break;
            }
        }

        // step 4
        r   = n;
        n   = a_u;
        a_u = r;
        if (a_u % 4 == 3 && n % 4 == 3) {
            t = -t;
        }
        a_u %= n;
    }

    ATTRIBUTE_ASSUME(t == -1 || t == 1);
    return (n == 1) ? t : 0;
}

}  // namespace detail

/// @brief Calculates Kronecker symbol of (a/n)
/// Source:
///     https://en.wikipedia.org/wiki/Legendre_symbol
///     https://en.wikipedia.org/wiki/Jacobi_symbol
///     https://en.wikipedia.org/wiki/Kronecker_symbol
/// @param a
/// @param n
/// @return Kronecker symbol of (a/n) (-1, 0 or 1)
template <typename IntegerT1, typename IntegerT2>
ATTRIBUTE_CONST static constexpr int32_t kronecker_symbol(IntegerT1 a, IntegerT2 n) noexcept {
#if __cplusplus >= 202002L
    using T1 = std::remove_cvref_t<IntegerT1>;
    using T2 = std::remove_cvref_t<IntegerT2>;
#else
    using T1 = IntegerT1;
    using T2 = IntegerT2;
#endif

    static_assert(type_traits_helper_int128_t::is_integral_v<T1>, "");
    static_assert(type_traits_helper_int128_t::is_integral_v<T2>, "");
    static_assert(sizeof(T1) == sizeof(T2), "");
    static_assert(sizeof(T1) >= sizeof(int), "");

    if constexpr (type_traits_helper_int128_t::is_unsigned_v<T1>) {
        if constexpr (type_traits_helper_int128_t::is_unsigned_v<T2>) {
            return detail::kronecker_symb_ui<T1>(a, static_cast<T1>(n));
        } else {
            return detail::kronecker_symb_ui<T1>(a,
                                                 n >= 0 ? static_cast<T1>(n) : -static_cast<T1>(n));
        }
    } else {
        if constexpr (type_traits_helper_int128_t::is_unsigned_v<T2>) {
            return detail::kronecker_symbol_ui<T1, T2>(a, n);
        } else {
            return detail::kronecker_symbol_si<T1>(a, static_cast<T1>(n));
        }
    }
}

}  // namespace math_functions

#endif  // !KRONECKER_SYMBOL_HPP
