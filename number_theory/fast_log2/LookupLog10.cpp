#include <cstdint>
#include <cstdio>

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
    /*
     * Floor log10(1) = 1  
     * Floor log10(9) = 1  
     * Floor log10(10) = 2 
     * Floor log10(11) = 2 
     * Floor log10(99) = 2 
     * Floor log10(100) = 3
     * Floor log10(101) = 3
     */
    printf(
        "Floor log10(1) = %u\n"
        "Floor log10(9) = %u\n"
        "Floor log10(10) = %u\n"
        "Floor log10(11) = %u\n"
        "Floor log10(99) = %u\n"
        "Floor log10(100) = %u\n"
        "Floor log10(101) = %u\n",
        log10_floor(1),
        log10_floor(9),
        log10_floor(10),
        log10_floor(11),
        log10_floor(99),
        log10_floor(100),
        log10_floor(101));
    return 0;
}
