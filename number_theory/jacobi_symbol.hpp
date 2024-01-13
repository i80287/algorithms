#if !defined(JACOBI_SYMBOL_HPP)
#define JACOBI_SYMBOL_HPP 1

#include <type_traits>

#include "integers_128_bit.hpp"
#include "math_utils.hpp"

template <typename Uint>
#if __cplusplus >= 202002L
    requires type_traits_helper_int128_t::is_unsigned_v<Uint>
#endif
static constexpr int32_t JacobiSymbolUi(Uint a, Uint n) noexcept {
    int32_t t = 1;

    if (n % 2 == 0) {
        // Work out the (a/2)

        if (unlikely(n == 0)) {
            return a == 1;
        }

        auto [q, p] = math_utils::extract_2pow(n);
        n = q;

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
                t -= int32_t((p % 2) * 2);
                break;
            case 1:
            case 7:
                // a === +-1 (mod 8)
                // t = 1
                break;
        }
    }

    // step 1
    a = a % n;
    Uint r = 0;
    // step 3
    while (a != 0) {
        // step 2
        while (a % 2 == 0) {
            a /= 2;
            r = n % 8;
            if (r == 3 || r == 5) {
                t = -t;
            }
        }

        // step 4
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
#if __cplusplus >= 202002L
    requires type_traits_helper_int128_t::is_signed_v<Sint>
#endif
static constexpr int32_t JacobiSymbolSi(Sint a, Sint n) noexcept {
    bool carry = n < 0;
    if (carry) {
        n = -n;
        carry = a < 0;
    }

    using Uint = type_traits_helper_int128_t::make_unsigned_t<Sint>;
    Uint n_u = static_cast<Uint>(n);

    int32_t t = 1;
    if (n_u % 2 == 0) {
        // Work out the (a/2)

        if (unlikely(n_u == 0)) {
            return a == 1 || a == -1;
        }

        auto [q, p] = math_utils::extract_2pow(n_u);
        n_u = q;
        n >>= p;

        switch (uint32_t((a % 8) + 8) % 8) {
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
                t -= int32_t((p % 2) * 2);
                break;
            case 1:
            case 7:
                // a === +-1 (mod 8)
                // t = 1
                break;
        }
    }

    // step 1
    Uint a_u = static_cast<Uint>(a % n + n) % n_u;
    Uint r = 0;
    // step 3
    while (a_u != 0) {
        // step 2
        while (a_u % 2 == 0) {
            a_u /= 2;
            r = n_u % 8;
            if (r == 3 || r == 5) {
                t = -t;
            }
        }

        // step 4
        r = n_u;
        n_u = a_u;
        a_u = r;
        if (a_u % 4 == 3 && n_u % 4 == 3) {
            t = -t;
        }
        a_u = a_u % n_u;
    }

    return (n_u == 1) ? (!carry ? t : -t) : 0;
}

template <class Sint, class Uint>
#if __cplusplus >= 202002L
    requires type_traits_helper_int128_t::is_signed_v<Sint> &&
             type_traits_helper_int128_t::is_unsigned_v<Uint>
#endif
static constexpr int32_t JacobiSymbolSiUi(Sint a, Uint n) noexcept {
    int32_t t = 1;

    if (n % 2 == 0) {
        // Work out the (a/2)

        if (unlikely(n == 0)) {
            return a == 1 || a == -1;
        }

        auto [q, p] = math_utils::extract_2pow(n);
        n = q;

        switch (uint32_t((a % 8) + 8) % 8) {
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
                t -= int32_t((p % 2) * 2);
                break;
            case 1:
            case 7:
                // a === +-1 (mod 8)
                // t = 1
                break;
        }
    }

    //step 1
    // a_u = a mod n
    Uint a_u = a >= 0 ? Uint(a) % n : (n - (-Uint(a)) % n);
    Uint r = 0;
    //step 3
    while (a_u != 0) {
        //step 2
        while (a_u % 2 == 0) {
            a_u /= 2;
            r = n % 8;
            if (r == 3 || r == 5) {
                t = -t;
            }
        }

        //step 4
        r = n;
        n = a_u;
        a_u = r;
        if (a_u % 4 == 3 && n % 4 == 3) {
            t = -t;
        }
        a_u = a_u % n;
    }

    return (n == 1) ? t : 0;
}

/// @brief Calculates Jacobi symbol of (a/n)
/// Source:
///     https://en.wikipedia.org/wiki/Jacobi_symbol
///     https://en.wikipedia.org/wiki/Kronecker_symbol
/// @param a
/// @param n
/// @return Jacobi symbol of (a/n) (-1, 0 or 1)
template <typename IntegerT1, typename IntegerT2>
static constexpr int32_t JacobiSymbol(IntegerT1 a, IntegerT2 n) noexcept {
#if __cplusplus >= 202002L
    using T1 = std::remove_cvref_t<IntegerT1>;
    using T2 = std::remove_cvref_t<IntegerT2>;
#else
    using T1 = IntegerT1;
    using T2 = IntegerT2;
#endif

    static_assert(type_traits_helper_int128_t::is_integral_v<T1>);
    static_assert(type_traits_helper_int128_t::is_integral_v<T2>);
    static_assert(sizeof(T1) == sizeof(T2));
    static_assert(!std::is_same_v<T1, bool> && !std::is_same_v<T2, bool>);

    if constexpr (type_traits_helper_int128_t::is_unsigned_v<T1>) {
        if constexpr (type_traits_helper_int128_t::is_unsigned_v<T2>) {
            return JacobiSymbolUi<T1>(a, static_cast<T1>(n));
        } else {
            return JacobiSymbolUi<T1>(a, static_cast<T1>(n >= 0 ? n : -n));
        }
    } else {
        if constexpr (type_traits_helper_int128_t::is_unsigned_v<T2>) {
            return JacobiSymbolSiUi<T1, T2>(a, n);
        } else {
            return JacobiSymbolSi<T1>(a, static_cast<T1>(n));
        }
    }
}

#endif  // !JACOBI_SYMBOL_HPP
