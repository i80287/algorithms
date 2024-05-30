#include <cstdint>

#if defined(__has_include) && __has_include("integers_128_bit.hpp")
#include "integers_128_bit.hpp"
#endif

namespace math_functions {

namespace detail {

static constexpr void matrix_mul(uint64_t (&m1)[2][2], const uint64_t (&m2)[2][2]) noexcept {
    const uint64_t tmp[2][2] = {
        {m1[0][0] * m2[0][0] + m1[0][1] * m2[1][0], m1[0][0] * m2[0][1] + m1[0][1] * m2[1][1]},
        {m1[1][0] * m2[0][0] + m1[1][1] * m2[1][0], m1[1][0] * m2[0][1] + m1[1][1] * m2[1][1]},
    };
    m1[0][0] = tmp[0][0];
    m1[0][1] = tmp[0][1];
    m1[1][0] = tmp[1][0];
    m1[1][1] = tmp[1][1];
}

}  // namespace detail

struct fibs_pair {
    /// @brief F_{n - 1}
    uint64_t fib_n_1;
    /// @brief F_n
    uint64_t fib_n;
};

/// @brief Returns pair (F_{n - 1}, F_n), where F_n
///        is a n-th Fibonacci number modulo 2^64.
///        Here we suppose that F_{-1} = 0, F_0 = 1, F_1 = 1.
/// @param n
/// @return (F_{n - 1}, F_n)
ATTRIBUTE_CONST constexpr fibs_pair fibonacci_nums(uint32_t n) noexcept {
    uint64_t p[2][2] = {
        {0, 1},
        {1, 1},
    };
    uint64_t fibmatrix[2][2] = {
        {1, 0},
        {0, 1},
    };

    while (true) {
        if (n % 2 != 0) {
            detail::matrix_mul(fibmatrix, p);
        }

        n /= 2;
        if (n == 0) {
            break;
        }

        detail::matrix_mul(p, p);
    }

    return {fibmatrix[1][0], fibmatrix[1][1]};
}

/// @brief Returns F_n, where F_n is a n-th Fibonacci number modulo 2^64.
///        Here we suppose that F_{-1} = 0, F_0 = 1, F_1 = 1.
/// @param n
/// @return F_n
ATTRIBUTE_CONST constexpr uint64_t fibonacci_num(uint32_t n) noexcept {
    return fibonacci_nums(n).fib_n;
}

/// @brief Max n for which there is no overflow in the set
///        {
///            fibonacci_num(0),
///            fibonacci_num(1),
///            ...
///            fibonacci_num(n)
///         }
///
///     fibonacci_num(90) == 4660046610375530309
///     fibonacci_num(91) == 7540113804746346429
///     fibonacci_num(92) == 12200160415121876738
///     fibonacci_num(93) == 1293530146158671551  <- overflow
inline constexpr uint32_t kMaxFibNonOverflowU64 = 92;

#if defined(INTEGERS_128_BIT_HPP)

/// @brief Helper namespace in order not to pollute math_functions namespace
namespace detail {

static inline I128_CONSTEXPR void matrix_mul(uint128_t m1[2][2],
                                             const uint128_t m2[2][2]) noexcept {
    const uint128_t tmp[2][2] = {
        {m1[0][0] * m2[0][0] + m1[0][1] * m2[1][0], m1[0][0] * m2[0][1] + m1[0][1] * m2[1][1]},
        {m1[1][0] * m2[0][0] + m1[1][1] * m2[1][0], m1[1][0] * m2[0][1] + m1[1][1] * m2[1][1]},
    };
    m1[0][0] = tmp[0][0];
    m1[0][1] = tmp[0][1];
    m1[1][0] = tmp[1][0];
    m1[1][1] = tmp[1][1];
}

}  // namespace detail

struct fibs_pair_u128 {
    /// @brief F_{n - 1}
    uint128_t fib_n_1;
    /// @brief F_n
    uint128_t fib_n;
};

/// @brief Returns pair (F_{n - 1}, F_n), where F_n
///        is a n-th Fibonacci number modulo 2^128.
///        Here we suppose that F_{-1} = 0, F_0 = 1, F_1 = 1.
/// @param n
/// @return (F_{n - 1}, F_n)
ATTRIBUTE_CONST inline I128_CONSTEXPR fibs_pair_u128 fibonacci_nums_u128(uint32_t n) noexcept {
    uint128_t p[2][2] = {
        {0, 1},
        {1, 1},
    };
    uint128_t fibmatrix[2][2] = {
        {1, 0},
        {0, 1},
    };

    while (true) {
        if (n % 2 != 0) {
            detail::matrix_mul(fibmatrix, p);
        }

        n /= 2;
        if (n == 0) {
            break;
        }

        detail::matrix_mul(p, p);
    }

    return {fibmatrix[1][0], fibmatrix[1][1]};
}

/// @brief Returns F_n, where F_n is a n-th Fibonacci number modulo 2^128.
///        Here we suppose that F_{-1} = 0, F_0 = 1, F_1 = 1.
/// @param n
/// @return F_n
ATTRIBUTE_CONST inline I128_CONSTEXPR uint128_t fibonacci_num_u128(uint32_t n) noexcept {
    return fibonacci_nums_u128(n).fib_n;
}

/// @brief Max n for which there is no overflow in the set
///        {
///            fibonacci_num_u128(0),
///            fibonacci_num_u128(1),
///            ...
///            fibonacci_num_u128(n)
///         }
///
///     fibonacci_num_u128(183) == 127127879743834334146972278486287885163
///     fibonacci_num_u128(184) == 205697230343233228174223751303346572685
///     fibonacci_num_u128(185) == 332825110087067562321196029789634457848
///     fibonacci_num_u128(186) == 198239973509362327032045173661212819077
///                                  ^
///                               overflow
inline constexpr uint32_t kMaxFibNonOverflowU128 = 185;

#endif  // INTEGERS_128_BIT_HPP

}  // namespace math_functions
