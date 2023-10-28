// For the real speed turn on at least lzcnt
// #pragma GCC target("bmi")
// #pragma GCC target("bmi2")
// #pragma GCC target("popcnt")
// #pragma GCC target("lzcnt")
// #pragma GCC target("tune=native")

#include <cstdint>
#include <cstdio>
#include <x86intrin.h>

inline uint32_t log2_floor_s(uint64_t) noexcept;
inline uint32_t log2_floor_x(uint64_t) noexcept;
inline uint32_t log2_ceil(uint64_t) noexcept;

int main() {
    /*
     * log2_floor_s(1) = 0
     * log2_floor_s(2) = 1
     * log2_floor_s(1023) = 9
     * log2_floor_s(1024) = 10
     * log2_floor_s(1025) = 10
     * log2_floor_s(2^32 - 1) = 31
     * log2_floor_s(2^64 - 1) = 63
     */
    printf("log2_floor_s(1) = %u\n", log2_floor_s(1ull));
    printf("log2_floor_s(2) = %u\n", log2_floor_s(2ull));
    printf("log2_floor_s(1023) = %u\n", log2_floor_s(1023ull));
    printf("log2_floor_s(1024) = %u\n", log2_floor_s(1024ull));
    printf("log2_floor_s(1025) = %u\n", log2_floor_s(1025ull));
    printf("log2_floor_s(2^32 - 1) = %u\n", log2_floor_s(UINT32_MAX));
    printf("log2_floor_s(2^64 - 1) = %u\n", log2_floor_s(UINT64_MAX));

    putchar('\n');

    /*
     * log2_floor_x(1) = 0
     * log2_floor_x(2) = 1
     * log2_floor_x(1023) = 9
     * log2_floor_x(1024) = 10
     * log2_floor_x(1025) = 10
     * log2_floor_x(2^32 - 1) = 31
     * log2_floor_x(2^64 - 1) = 63
     */
    printf("log2_floor_x(1) = %u\n", log2_floor_x(1ull));
    printf("log2_floor_x(2) = %u\n", log2_floor_x(2ull));
    printf("log2_floor_x(1023) = %u\n", log2_floor_x(1023ull));
    printf("log2_floor_x(1024) = %u\n", log2_floor_x(1024ull));
    printf("log2_floor_x(1025) = %u\n", log2_floor_x(1025ull));
    printf("log2_floor_x(2^32 - 1) = %u\n", log2_floor_x(UINT32_MAX));
    printf("log2_floor_x(2^64 - 1) = %u\n", log2_floor_x(UINT64_MAX));

    putchar('\n');

    /*
     * log2_ceil(1) = 0
     * log2_ceil(2) = 1
     * log2_ceil(1023) = 10
     * log2_ceil(1024) = 10
     * log2_ceil(1025) = 11
     * log2_ceil(2^32 - 1) = 32
     * log2_ceil(2^64 - 1) = 64
     */
    printf("log2_ceil(1) = %u\n", log2_ceil(1ull));
    printf("log2_ceil(2) = %u\n", log2_ceil(2ull));
    printf("log2_ceil(1023) = %u\n", log2_ceil(1023ull));
    printf("log2_ceil(1024) = %u\n", log2_ceil(1024ull));
    printf("log2_ceil(1025) = %u\n", log2_ceil(1025ull));
    printf("log2_ceil(2^32 - 1) = %u\n", log2_ceil(UINT32_MAX));
    printf("log2_ceil(2^64 - 1) = %u\n", log2_ceil(UINT64_MAX));
}

inline uint32_t log2_floor_s(uint64_t n) noexcept {
    // " | 1" does not affect ans for all n >= 1.
    return static_cast<uint32_t>(63 - __builtin_clzll(n | 1));
}

inline uint32_t log2_floor_x(uint64_t n) noexcept {
    // " | 1" does not affect ans for all n >= 1.
    return static_cast<uint32_t>(63 ^ __builtin_clzll(n | 1));
}

inline uint32_t log2_ceil(uint64_t n) noexcept {
    return log2_floor_x(n) + ((n & (n - 1)) != 0);
}

// Windows
// g++ -Wall -Wextra -O3 IntrinsicLog.cpp -o IntrinsicLog.exe && ./IntrinsicLog.exe
// Linux
// g++ -Wall -Wextra -O3 IntrinsicLog.cpp -o IntrinsicLog.out && ./IntrinsicLog.out
// Power shell
// & g++ -Wall -Wextra -O3 IntrinsicLog.cpp -o IntrinsicLog.exe ; ./IntrinsicLog.exe
