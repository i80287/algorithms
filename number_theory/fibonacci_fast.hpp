#include <cstdint>
#include <type_traits>

/// @brief Helper namespace in order not to pollute globale namespace
namespace fibonacci_helper_namespace {

static constexpr void matrix_mul(uint64_t m1[2][2],
                                 const uint64_t m2[2][2]) noexcept {
    const uint64_t tmp[2][2] = {
        {m1[0][0] * m2[0][0] + m1[0][1] * m2[1][0],
         m1[0][0] * m2[0][1] + m1[0][1] * m2[1][1]},
        {m1[1][0] * m2[0][0] + m1[1][1] * m2[1][0],
         m1[1][0] * m2[0][1] + m1[1][1] * m2[1][1]},
    };
    m1[0][0] = tmp[0][0];
    m1[0][1] = tmp[0][1];
    m1[1][0] = tmp[1][0];
    m1[1][1] = tmp[1][1];
}

}  // namespace fibonacci_helper_namespace

constexpr uint64_t fibonacci_num(uint32_t n) noexcept {
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
            fibonacci_helper_namespace::matrix_mul(fibmatrix, p);
        }

        n /= 2;
        if (n == 0) {
            break;
        }

        fibonacci_helper_namespace::matrix_mul(p, p);
    }

    return fibmatrix[0][0] + fibmatrix[1][0];
}

/// @brief Helper namespace in order not to pollute globale namespace
namespace fibonacci_helper_namespace {

#if __cplusplus >= 202002L
consteval
#else
static constexpr
#endif
    uint32_t
    max_nonoverflow_helper() noexcept {
    uint32_t n = 0;
    do {
        n++;
    } while (fibonacci_num(n) < fibonacci_num(n + 1));
    return n;
}

}  // namespace fibonacci_helper_namespace

/// @brief Max n for which there is not ovewflow in
///        {
///            uint64_t(fibonacci_num(0)),
///            uint64_t(fibonacci_num(1)),
///            ...
///            uint64_t(fibonacci_num(n))
///         }
///
///     fibonacci_num(90) == 4660046610375530309
///     fibonacci_num(91) == 7540113804746346429
///     fibonacci_num(92) == 12200160415121876738
///     fibonacci_num(93) == 1293530146158671551
inline constexpr uint32_t kMaxFibonacciNonOverflowN =
    fibonacci_helper_namespace::max_nonoverflow_helper();
