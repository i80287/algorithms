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
        const uint32_t a_initial = a;
        for (uint32_t d = 2; d * d <= a_initial; d++) {
            if (a % d == 0) {
                uint32_t prime_div_power = 0;
                do {
                    prime_div_power++;
                    a /= d;
                } while (a % d == 0);
                divisors[d] += prime_div_power;
            }
        }

        if (a > 1) {
            divisors[a]++;
        }
    }

    for (const auto& pair : divisors) {
        std::cout << pair.first << ": " << pair.second << '\n';
    }
}
