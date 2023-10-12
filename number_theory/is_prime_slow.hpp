#ifndef _IS_PRIME_SLOW_H_
#define _IS_PRIME_SLOW_H_ 1

#include <cmath>
#include <cstdint>

static constexpr bool IsPrimeSlow(uint64_t n) noexcept {
	if (n == 1) {
		return false;
	}

	if (n % 2 == 0 || n % 3 == 0) {
		return n == 2 || n == 3;
	}

	uint64_t root = static_cast<uint64_t>(std::sqrt(n));
	for (uint64_t i = 5; i <= root; i += 6) {
		if (n % i == 0 || n % (i + 2) == 0) {
			return false;
		}
	}

	return true;
}

#endif // !_IS_PRIME_SLOW_H_
