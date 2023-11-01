#include <ext/pb_ds/assoc_container.hpp>
#include <iostream>

template <typename K, typename V>
using unordered_map = __gnu_pbds::gp_hash_table<K, V>;

int main() {
    uint32_t n = 0;
    std::cin >> n;
    unordered_map<uint32_t, uint64_t> divisors;

    for (uint32_t i = n; i != 0; i--) {
        uint32_t a = 0;
        std::cin >> a;

        if (a % 2 == 0) { // we dont need to optimize it like (k & 1) == 0, compiler will do it anyway
            if (a == 0) {
                continue;
            }

            // k = s * 2^pow_of_2, where s is odd
            uint32_t pow_of_2 = uint32_t(__builtin_ctz(a));
            a >>= pow_of_2;
            divisors[2] += pow_of_2;
        }

        for (uint32_t d = 3; d * d <= a; d += 2) {
            if (a % d == 0) {
                uint32_t pow_of_d = 0;
                do {
                    pow_of_d++;
                    a /= d;
                } while (a % d == 0);
                divisors[d] += pow_of_d;
            }
        }

        if (a != 1) {
            divisors[a]++;
        }
    }

    for (const auto& pair : divisors) {
        std::cout << pair.first << ": " << pair.second << '\n';
    }
}
