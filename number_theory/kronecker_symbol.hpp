#ifndef KRONECKER_SYMBOL_HPP
#define KRONECKER_SYMBOL_HPP

#include <cstdint>
#include <cstdlib>
#include <type_traits>

#include "integers_128_bit.hpp"
#include "math_functions.hpp"

namespace math_functions {

namespace detail {

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

template <typename Uint>
#if CONFIG_HAS_AT_LEAST_CXX_20
    requires int128_traits::is_unsigned_v<Uint>
#endif
ATTRIBUTE_CONST constexpr std::int32_t kronecker_symbol_ui(Uint a, Uint n) noexcept {
    std::int32_t t = 1;

    if (n % 2 == 0) {
        // Work out the (a/2)

        if (unlikely(n == 0)) {
            return a == 1;
        }

        auto [q, p] = ::math_functions::extract_pow2(n);
        CONFIG_ASSUME_STATEMENT(q % 2 == 1);
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
                t -= static_cast<std::int32_t>((p % 2) * 2);
                CONFIG_ASSUME_STATEMENT(t == -1 || t == 1);
                break;
            case 1:
            case 7:
                // a === +-1 (mod 8)
                // t = 1
                break;
            default:
                // for the static analysers
                std::abort();
                break;
        }
    }

    CONFIG_ASSUME_STATEMENT(n % 2 == 1);
    // Redundant but still
    CONFIG_ASSUME_STATEMENT(n != 0);
    // step 1
    a %= n;
    Uint r = 0;
    // step 3
    while (a != 0) {
        // step 2
        if (a % 2 == 0) {
            auto [a_odd_part, a_exp] = ::math_functions::extract_pow2(a);
            a = a_odd_part;
            r = n % 8;
            CONFIG_ASSUME_STATEMENT(r <= 7);
            switch (r) {
                case 3:
                case 5:
                    t = a_exp % 2 == 0 ? t : -t;
                    break;
                default:
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

    CONFIG_ASSUME_STATEMENT(t == -1 || t == 1);
    return (n == 1) ? t : 0;
}

template <typename Sint>
#if CONFIG_HAS_AT_LEAST_CXX_20
    requires int128_traits::is_signed_v<Sint>
#endif
ATTRIBUTE_CONST constexpr std::int32_t kronecker_symbol_si(Sint a, Sint n) noexcept {
    const bool carry = n < 0 && a < 0;
    using Uint = typename int128_traits::make_unsigned_t<Sint>;
    Uint n_u = ::math_functions::uabs(n);

    std::int32_t t = 1;
    if (n_u % 2 == 0) {
        // Work out the (a/2)

        if (unlikely(n_u == 0)) {
            return a == 1 || a == -1;
        }

        auto [q, p] = ::math_functions::extract_pow2(n_u);
        CONFIG_ASSUME_STATEMENT(q % 2 == 1);
        n_u = q;

        switch (static_cast<std::uint32_t>((a % 8) + 8) % 8) {
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
                t -= static_cast<std::int32_t>((p % 2) * 2);
                CONFIG_ASSUME_STATEMENT(t == -1 || t == 1);
                break;
            case 1:
            case 7:
                // a === +-1 (mod 8)
                // t = 1
                break;
            default:
                // for the static analysers
                std::abort();
                break;
        }
    }

    CONFIG_ASSUME_STATEMENT(n_u % 2 == 1);
    // Redundant but still
    CONFIG_ASSUME_STATEMENT(n_u > 0);
    // step 1
    Uint a_u = (static_cast<Uint>(a % static_cast<Sint>(n_u)) + n_u) % n_u;
    Uint r = 0;
    // step 3
    while (a_u != 0) {
        // step 2
        if (a_u % 2 == 0) {
            auto [a_u_odd_part, a_u_exp] = ::math_functions::extract_pow2(a_u);
            a_u = a_u_odd_part;
            r = n_u % 8;
            CONFIG_ASSUME_STATEMENT(r <= 7);
            switch (r) {
                case 3:
                case 5:
                    t = a_u_exp % 2 == 0 ? t : -t;
                    break;
                default:
                    break;
            }
        }

        // step 4
        r = n_u;
        n_u = a_u;
        a_u = r;
        if (a_u % 4 == 3 && n_u % 4 == 3) {
            t = -t;
        }
        a_u %= n_u;
    }

    CONFIG_ASSUME_STATEMENT(t == -1 || t == 1);
    return (n_u == 1) ? (!carry ? t : -t) : 0;
}

template <class Sint, class Uint>
#if CONFIG_HAS_AT_LEAST_CXX_20
    requires int128_traits::is_signed_v<Sint> && int128_traits::is_unsigned_v<Uint>
#endif
ATTRIBUTE_CONST constexpr int32_t kronecker_symbol_si_ui(Sint a, Uint n) noexcept {
    std::int32_t t = 1;

    if (n % 2 == 0) {
        // Work out the (a/2)

        if (unlikely(n == 0)) {
            return a == 1 || a == -1;
        }

        auto [q, p] = ::math_functions::extract_pow2(n);
        CONFIG_ASSUME_STATEMENT(q % 2 == 1);
        n = q;

        switch (static_cast<std::uint32_t>((a % 8) + 8) % 8) {
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
                t -= static_cast<std::int32_t>((p % 2) * 2);
                CONFIG_ASSUME_STATEMENT(t == -1 || t == 1);
                break;
            case 1:
            case 7:
                // a === +-1 (mod 8)
                // t = 1
                break;
            default:
                // for the static analysers
                std::abort();
                break;
        }
    }

    CONFIG_ASSUME_STATEMENT(n % 2 == 1);
    // Redundant but still
    CONFIG_ASSUME_STATEMENT(n != 0);
    // step 1
    //  a_u = a mod n
    Uint a_u = (a >= 0 ? static_cast<Uint>(a) : (n - (-static_cast<Uint>(a)) % n)) % n;
    Uint r = 0;
    // step 3
    while (a_u != 0) {
        // step 2
        if (a_u % 2 == 0) {
            auto [a_u_odd_part, a_u_exp] = ::math_functions::extract_pow2(a_u);
            a_u = a_u_odd_part;
            r = n % 8;
            switch (r) {
                case 3:
                case 5:
                    t = a_u_exp % 2 == 0 ? t : -t;
                    break;
                default:
                    break;
            }
        }

        // step 4
        r = n;
        n = a_u;
        a_u = r;
        if (a_u % 4 == 3 && n % 4 == 3) {
            t = -t;
        }
        a_u %= n;
    }

    CONFIG_ASSUME_STATEMENT(t == -1 || t == 1);
    return (n == 1) ? t : 0;
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

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
ATTRIBUTE_CONST ATTRIBUTE_ALWAYS_INLINE constexpr int32_t kronecker_symbol(IntegerT1 a,
                                                                           IntegerT2 n) noexcept {
#if CONFIG_HAS_AT_LEAST_CXX_20
    using T1 = std::remove_cvref_t<IntegerT1>;
    using T2 = std::remove_cvref_t<IntegerT2>;
#else
    using T1 = std::remove_cv_t<typename std::remove_reference_t<IntegerT1> >;
    using T2 = std::remove_cv_t<typename std::remove_reference_t<IntegerT2> >;
#endif

    static_assert(int128_traits::is_integral_v<T1>, "First argument must be an integer");
    static_assert(int128_traits::is_integral_v<T2>, "Second argument must be an integer");
    static_assert(sizeof(T1) == sizeof(T2), "Both integers must have the same size");
    static_assert(sizeof(T1) >= sizeof(int), "Integers must be at least sizeof(int) in size");

    if constexpr (int128_traits::is_unsigned_v<T1>) {
        if constexpr (int128_traits::is_unsigned_v<T2>) {
            return ::math_functions::detail::kronecker_symbol_ui<T1>(a, static_cast<T1>(n));
        } else {
            return ::math_functions::detail::kronecker_symbol_ui<T1>(
                a, n >= 0 ? static_cast<T1>(n) : -static_cast<T1>(n));
        }
    } else {
        if constexpr (int128_traits::is_unsigned_v<T2>) {
            return ::math_functions::detail::kronecker_symbol_si_ui<T1, T2>(a, n);
        } else {
            return ::math_functions::detail::kronecker_symbol_si<T1>(a, static_cast<T1>(n));
        }
    }
}

}  // namespace math_functions

#endif  // !KRONECKER_SYMBOL_HPP
