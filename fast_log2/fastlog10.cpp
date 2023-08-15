#include <cstdint>
#include <iostream>

// Taken from https://stackoverflow.com/questions/25892665/performance-of-log10-function-returning-an-int

inline uint32_t baseTwoDigits(uint32_t x) {
    return x ? 32 - __builtin_clz(x) : 0;
}

inline uint32_t baseTenDigits(uint32_t x) {
    static const unsigned char guess[33] = {
        0, 0, 0, 0, 1, 1, 1, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 5, 5, 5,
        6, 6, 6, 6, 7, 7, 7, 8, 8, 8,
        9, 9, 9
    };
    static const uint32_t tenToThe[10] = {
        1, 10, 100, 1000, 10000, 100000, 
        1000000, 10000000, 100000000, 1000000000,
    };
    uint32_t digits = guess[baseTwoDigits(x)];
    return digits + (x >= tenToThe[digits]);
}

inline uint32_t log10_floor(uint32_t x) {
    return baseTenDigits(x);
}

int main() {
    std::cout << "Floor log10(10) = " << log10_floor(10) << '\n'; // 2
    std::cout << "Floor log10(99) = " << log10_floor(99) << '\n'; // 2
    std::cout << "Floor log10(100) = " << log10_floor(100) << '\n'; // 3
    std::cout << "Floor log10(101) = " << log10_floor(101) << '\n'; // 3
    return 0;
}
