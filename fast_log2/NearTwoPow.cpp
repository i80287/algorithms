#include <cstdint>
#include <x86intrin.h>
#include <iostream>

uint32_t nearest_bin_pow_ge(uint64_t n) {
    // __lzcnt64 can be used with -mlzcnt flag
    return 1u << static_cast<uint32_t>(64 - __builtin_clzll(n) - ((n & (n - 1)) == 0));
}

int main() {
    /*
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
    */
    for (uint64_t n = 1; n <= 10; ++n) {
        std::cout << "For n = " << n << ": " << nearest_bin_pow_ge(n) << '\n';
    }
}
