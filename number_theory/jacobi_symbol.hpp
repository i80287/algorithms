#if !defined(_JACOBI_SYMBOL_HPP_)
#define _JACOBI_SYMBOL_HPP_ 1

#include "integers_128_bit.hpp"
#include "math_utils.hpp"

#include <type_traits>

template <typename Uint>
#if __cplusplus >= 202002L
requires std::is_unsigned_v<Uint>
#endif
static constexpr int32_t JacobiSymbolUi(Uint a, Uint n) noexcept {
    int32_t t = 1;
    if (n % 2 == 0) {
        if (unlikely(n == 0)) {
            return a == 1;
        }

        uint32_t p = uint32_t(math_utils::count_trailing_zeros(n));
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
#if __cplusplus >= 202002L
requires std::is_signed_v<Sint>
#endif
static constexpr int32_t JacobiSymbolSi(Sint a, Sint n) noexcept {
    if (unlikely(n == 0)) {
        return a == 1 || a == -1;
    }

    bool carry = n < 0;
    if (carry) {
        n = -n;
        carry = a < 0;
    }

    using Uint = std::make_unsigned_t<Sint>;
    Uint n_u = static_cast<Uint>(n);

    int32_t t = 1;
    if (n_u % 2 == 0) {
        uint32_t p = uint32_t(math_utils::count_trailing_zeros(n_u));
        n_u >>= p;
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
                t -= static_cast<int32_t>(static_cast<uint32_t>(p % 2 != 0) * 2);
                break;
            case 1:
            case 7:
                // a === +-1 (mod 8)
                // t = 1
                break;
        }
    }

    //step 1
    Uint a_u = static_cast<Uint>(a % n + n) % n_u;
    Uint r = 0;
    //step 3
    while (a_u != 0) {
        //step 2
        while (a_u % 2 == 0) {
            a_u /= 2;
            r = n_u % 8;
            if (r == 3 || r == 5) {
                t = -t;
            }
        }

        //step 4
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

/// @brief Calculates Jacobi symbol of (a/n); Source: https://en.wikipedia.org/wiki/Jacobi_symbol && https://en.wikipedia.org/wiki/Kronecker_symbol
/// @param a 
/// @param n 
/// @return Jacobi symbol (-1, 0 or 1)
template <typename IntegerT1, typename IntegerT2>
#if __cplusplus >= 202002L
requires std::is_integral_v<IntegerT1> && std::is_integral_v<IntegerT2>
#endif
static constexpr int32_t JacobiSymbol(IntegerT1 a, IntegerT2 n) noexcept {
#if __cplusplus >= 202002L
    using T1 = std::remove_cvref_t<IntegerT1>;
    using T2 = std::remove_cvref_t<IntegerT2>;
#else
    using T1 = IntegerT1;
    using T2 = IntegerT2;
#endif
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

#endif // !_JACOBI_SYMBOL_HPP_ 
