#include <cstdint>

constexpr void matrix_mul(uint64_t m1[2][2], const uint64_t m2[2][2]) noexcept {
    const uint64_t tmp[2][2] = {
        m1[0][0] * m2[0][0] + m1[0][1] * m2[1][0],
        m1[0][0] * m2[0][1] + m1[0][1] * m2[1][1],
        m1[1][0] * m2[0][0] + m1[1][1] * m2[1][0],
        m1[1][0] * m2[0][1] + m1[1][1] * m2[1][1],
    };
    m1[0][0] = tmp[0][0];
    m1[0][1] = tmp[0][1];
    m1[1][0] = tmp[1][0];
    m1[1][1] = tmp[1][1];
}

constexpr uint64_t fibonacci_num(uint32_t n) noexcept {
    uint64_t p[2][2] = {
        0, 1,
        1, 1,
    };
    uint64_t fibmatrix[2][2] = {
        1, 0,
        0, 1,
    };

    while (true) {
        if (n % 2 != 0) {
            matrix_mul(fibmatrix, p);
        }

        n /= 2;
        if (n == 0) {
            break;
        }

        matrix_mul(p, p);
    }

    return fibmatrix[0][0] + fibmatrix[1][0];
}
