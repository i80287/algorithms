#include <fstream>
#include <cmath>
#include <cstdint>

static constexpr bool is_prime(uint64_t n) noexcept {
	if (n < 2ULL) {
		return false;
	}

	if (n < 4ULL) {
		return true;
	}

	if ((n % 2 == 0) | (n % 3 == 0)) {
		return false;
	}
	
	uint64_t root = static_cast<uint64_t>(std::sqrt(n));
	for (uint64_t i = 5; i <= root; i += 6) {
		if ((n % i == 0) | (n % (i + 2) == 0)) {
			return false;
		}
	}
	
	return true;
}

int main(void) {
	std::ofstream f("primes.txt", std::ios_base::app);
	for(uint64_t i = 72057594037927937ull; i != 1; i-=2) {
		if(is_prime(i)){
			f << i << '\n';
		}
	}

	f.close();
	return 0;
}
