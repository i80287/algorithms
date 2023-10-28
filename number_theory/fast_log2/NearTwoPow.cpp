// #pragma GCC target("lzcnt,tune=native")

#include <cstdint>
#include <x86intrin.h>
#include <iostream>

inline uint64_t nearest_bin_pow_ge(uint64_t n) noexcept {
    return 1ull << static_cast<uint32_t>((63 ^ __builtin_clzll(n | 1)) + ((n & (n - 1)) != 0));
}

int main() {
    /*
     * For n = 0: 1
     * For n = 1: 1
     * For n = 2: 2
     * For n = 3: 4
     * For n = 4: 4
     * For n = 5: 8
     * For n = 6: 8
     * For n = 7: 8
     * For n = 8: 8
     * For n = 9: 16
     * For n = 10: 16
     * For n = 11: 16
     * For n = 12: 16
     * For n = 13: 16
     * For n = 14: 16
     * For n = 15: 16
     * For n = 16: 16
     * For n = 17: 32
     */
    for (uint64_t n = 0; n <= 17; ++n) {
        std::cout << "For n = " << n << ": " << nearest_bin_pow_ge(n) << '\n';
    }
}
