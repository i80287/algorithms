#ifndef IS_PRIME_SLOW_H
#define IS_PRIME_SLOW_H 1

#include <cmath>
#include <cstdint>

#include "math_utils.hpp"

static constexpr bool IsPrimeSlow(uint64_t n) noexcept {
	if (n % 2 == 0) {
        return n == 2;
    }
    if (n % 3 == 0) {
        return n == 3;
    }
    if (n % 5 == 0) {
        return n == 5;
    }
    if (n < 7 * 7) {
        return n != 1;
    }
	uint64_t i = 7;
	const uint64_t root = math_utils::isqrt(n);
	do {
		if (n % i == 0 || n % (i + 4) == 0 ||
			n % (i + 6) == 0 || n % (i + 10) == 0 ||
			n % (i + 12) == 0 || n % (i + 16) == 0 ||
			n % (i + 22) == 0 || n % (i + 24) == 0) {
			return false;
		}
		i += 30;
	} while (i <= root);
	return true;
}

#endif // !IS_PRIME_SLOW_H
