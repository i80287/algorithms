#ifndef IS_PRIME_SQRT_HPP
#define IS_PRIME_SQRT_HPP 1

#include <cmath>
#include <cstdint>

#include "config_macros.hpp"
#include "math_utils.hpp"

GCC_ATTRIBUTE_CONST static constexpr bool IsPrimeSqrt(uint64_t n) noexcept {
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
    uint64_t i = 7;
    const uint64_t root = math_utils::isqrt(n);
    do {
        if (n % i == 0 || n % (i + 4) == 0 || n % (i + 6) == 0 ||
            n % (i + 10) == 0 || n % (i + 12) == 0 || n % (i + 16) == 0 ||
            n % (i + 22) == 0 || n % (i + 24) == 0) {
            return false;
        }
        i += 30;
    } while (i <= root);
    return true;
}

#endif  // !IS_PRIME_SQRT_HPP
