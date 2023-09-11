#ifndef _IS_PRIME_SLOW_H_
#define _IS_PRIME_SLOW_H_ 1

#include <cmath>
#include <cstdint>

static constexpr bool IsPrimeSlow(uint64_t n) noexcept {
	switch(n) {
		case 0:
		case 1:
			return false;
		case 2:
		case 3:
			return true;
	}

	if (n % 2 == 0 || n % 3 == 0) {
		return false;
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
