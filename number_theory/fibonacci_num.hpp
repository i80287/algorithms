#include <cstdint>
#include <type_traits>

#include "../misc/config_macros.hpp"

#if CONFIG_HAS_INCLUDE("integers_128_bit.hpp")
#include "integers_128_bit.hpp"
#endif

namespace math_functions {

using std::uint32_t;

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)

namespace detail {

template <typename T>
ATTRIBUTE_ACCESS(read_write, 1)
ATTRIBUTE_ACCESS(read_only, 2) constexpr void matrix_mul(T (&m1)[2][2], const T (&m2)[2][2]) noexcept {
#ifdef HAS_INT128_TYPEDEF
    static_assert(int128_traits::is_arithmetic_v<T>);
#else
    static_assert(std::is_arithmetic_v<T>);
#endif

    const T tmp[2][2] = {
        {m1[0][0] * m2[0][0] + m1[0][1] * m2[1][0], m1[0][0] * m2[0][1] + m1[0][1] * m2[1][1]},
        {m1[1][0] * m2[0][0] + m1[1][1] * m2[1][0], m1[1][0] * m2[0][1] + m1[1][1] * m2[1][1]},
    };
    m1[0][0] = tmp[0][0];
    m1[0][1] = tmp[0][1];
    m1[1][0] = tmp[1][0];
    m1[1][1] = tmp[1][1];
}

}  // namespace detail

template <typename T>
struct fibs_pair {
    using int_type = T;

    /// @brief F_{n - 1}
    T fib_n_1;
    /// @brief F_n
    T fib_n;
};

/// @brief Returns pair (F_{n - 1}, F_n), where F_n
///        is a n-th Fibonacci number modulo 2^64.
///        Here we suppose that F_{-1} = 0, F_0 = 1, F_1 = 1.
/// @tparam T
/// @param n
/// @return (F_{n - 1}, F_n)
template <typename T>
[[nodiscard]]
ATTRIBUTE_CONST constexpr fibs_pair<T> fibonacci_nums(uint32_t n) noexcept {
    T pow_matrix[2][2] = {
        {0, 1},
        {1, 1},
    };
    T fibmatrix[2][2] = {
        {1, 0},
        {0, 1},
    };

    while (true) {
        if (n % 2 != 0) {
            detail::matrix_mul(fibmatrix, pow_matrix);
        }

        n /= 2;
        if (n == 0) {
            break;
        }

        detail::matrix_mul(pow_matrix, pow_matrix);
    }

    return {fibmatrix[1][0], fibmatrix[1][1]};
}

/// @brief Returns F_n, where F_n is a n-th Fibonacci number modulo 2^64.
///        Here we suppose that F_{-1} = 0, F_0 = 1, F_1 = 1.
/// @tparam T
/// @param n
/// @return F_n
template <typename T>
[[nodiscard]]
ATTRIBUTE_CONST constexpr T nth_fibonacci_num(const uint32_t n) noexcept {
    return fibonacci_nums<T>(n).fib_n;
}

// NOLINTEND(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)

using fibs_pair_u64 = fibs_pair<std::uint64_t>;

[[nodiscard]] ATTRIBUTE_CONST constexpr fibs_pair_u64 fibonacci_nums_u64(const uint32_t n) noexcept {
    return fibonacci_nums<fibs_pair_u64::int_type>(n);
}

[[nodiscard]] ATTRIBUTE_CONST constexpr std::uint64_t nth_fibonacci_num_u64(const uint32_t n) noexcept {
    return nth_fibonacci_num<std::uint64_t>(n);
}

#ifdef HAS_INT128_TYPEDEF

using fibs_pair_u128 = fibs_pair<uint128_t>;

[[nodiscard]] ATTRIBUTE_CONST I128_CONSTEXPR fibs_pair_u128 fibonacci_nums_u128(const uint32_t n) noexcept {
    return fibonacci_nums<fibs_pair_u128::int_type>(n);
}

[[nodiscard]] ATTRIBUTE_CONST constexpr uint128_t nth_fibonacci_num_u128(const uint32_t n) noexcept {
    return nth_fibonacci_num<uint128_t>(n);
}

#endif

/// @brief Max n for which there is no overflow in the set
///        {
///            fibonacci_num_u64(0),
///            fibonacci_num_u64(1),
///            ...
///            fibonacci_num_u64(n)
///         }
///
///     fibonacci_num_u64(90) == 4660046610375530309
///     fibonacci_num_u64(91) == 7540113804746346429
///     fibonacci_num_u64(92) == 12200160415121876738
///     fibonacci_num_u64(93) == 1293530146158671551  <- overflow
inline constexpr uint32_t kMaxFibNonOverflowU64 = 92;

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

}  // namespace math_functions
