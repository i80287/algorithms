#include <cstdint>
#include <cstdio>
#include <popcntintrin.h>
#include <x86intrin.h>

inline uint32_t lzcnt_log2_floor(uint64_t);
inline uint32_t lzcnt_log2_ceil(uint64_t);

inline uint32_t clz_log2_ceil_(uint64_t);
inline uint32_t clz_log2_floor(uint64_t);
inline uint32_t clz_log2_ceil(uint64_t);

int main() {
    /*
     * floor log2(1) = 0        
     * floor log2(2) = 1        
     * floor log2(1023) = 9     
     * floor log2(1024) = 10    
     * floor log2(1025) = 10    
     * floor log2(2^32 - 1) = 31
     * floor log2(2^64 - 1) = 63
     */
    printf("floor log2(1) = %u\n", lzcnt_log2_floor(1ull));
    printf("floor log2(2) = %u\n", lzcnt_log2_floor(2ull));
    printf("floor log2(1023) = %u\n", lzcnt_log2_floor(1023ull));
    printf("floor log2(1024) = %u\n", lzcnt_log2_floor(1024ull));
    printf("floor log2(1025) = %u\n", lzcnt_log2_floor(1025ull));
    printf("floor log2(2^32 - 1) = %u\n", lzcnt_log2_floor(UINT32_MAX));
    printf("floor log2(2^64 - 1) = %u\n", lzcnt_log2_floor(UINT64_MAX)); // 63

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
    printf("log2_ceil(1) = %u\n", lzcnt_log2_ceil(1ull));
    printf("log2_ceil(2) = %u\n", lzcnt_log2_ceil(2ull));
    printf("log2_ceil(1023) = %u\n", lzcnt_log2_ceil(1023ull));
    printf("log2_ceil(1024) = %u\n", lzcnt_log2_ceil(1024ull));
    printf("log2_ceil(1025) = %u\n", lzcnt_log2_ceil(1025ull));
    printf("log2_ceil(2^32 - 1) = %u\n", lzcnt_log2_ceil(UINT32_MAX));
    printf("log2_ceil(2^64 - 1) = %u\n", lzcnt_log2_ceil(UINT64_MAX)); // 64

    putchar('\n');

    /*
     * clz_log2_ceil_(1) = 0   
     * clz_log2_ceil_(2) = 1   
     * clz_log2_ceil_(1023) = 10    
     * clz_log2_ceil_(1024) = 10    
     * clz_log2_ceil_(1025) = 11    
     * clz_log2_ceil_(2^32 - 1) = 32
     * clz_log2_ceil_(2^64 - 1) = 64
     */
    printf("clz_log2_ceil_(1) = %u\n", clz_log2_ceil_(1ull));
    printf("clz_log2_ceil_(2) = %u\n", clz_log2_ceil_(2ull));
    printf("clz_log2_ceil_(1023) = %u\n", clz_log2_ceil_(1023ull));
    printf("clz_log2_ceil_(1024) = %u\n", clz_log2_ceil_(1024ull));
    printf("clz_log2_ceil_(1025) = %u\n", clz_log2_ceil_(1025ull));
    printf("clz_log2_ceil_(2^32 - 1) = %u\n", clz_log2_ceil_(UINT32_MAX));
    printf("clz_log2_ceil_(2^64 - 1) = %u\n", clz_log2_ceil_(UINT64_MAX)); // 64

    putchar('\n');

    /*
     * clz_log2_floor(1) = 0
     * clz_log2_floor(2) = 1
     * clz_log2_floor(1023) = 9
     * clz_log2_floor(1024) = 10
     * clz_log2_floor(1025) = 10
     * clz_log2_floor(2^32 - 1) = 31
     * clz_log2_floor(2^64 - 1) = 63
     */
    printf("clz_log2_floor(1) = %u\n", clz_log2_floor(1ull));
    printf("clz_log2_floor(2) = %u\n", clz_log2_floor(2ull));
    printf("clz_log2_floor(1023) = %u\n", clz_log2_floor(1023ull));
    printf("clz_log2_floor(1024) = %u\n", clz_log2_floor(1024ull));
    printf("clz_log2_floor(1025) = %u\n", clz_log2_floor(1025ull));
    printf("clz_log2_floor(2^32 - 1) = %u\n", clz_log2_floor(UINT32_MAX));
    printf("clz_log2_floor(2^64 - 1) = %u\n", clz_log2_floor(UINT64_MAX)); // 63

    putchar('\n');

    /*
     * clz_log2_ceil(1) = 0
     * clz_log2_ceil(2) = 1
     * clz_log2_ceil(1023) = 10
     * clz_log2_ceil(1024) = 10
     * clz_log2_ceil(1025) = 11
     * clz_log2_ceil(2^32 - 1) = 32
     * clz_log2_ceil(2^64 - 1) = 64
     */
    printf("clz_log2_ceil(1) = %u\n", clz_log2_ceil(1ull));
    printf("clz_log2_ceil(2) = %u\n", clz_log2_ceil(2ull));
    printf("clz_log2_ceil(1023) = %u\n", clz_log2_ceil(1023ull));
    printf("clz_log2_ceil(1024) = %u\n", clz_log2_ceil(1024ull));
    printf("clz_log2_ceil(1025) = %u\n", clz_log2_ceil(1025ull));
    printf("clz_log2_ceil(2^32 - 1) = %u\n", clz_log2_ceil(UINT32_MAX));
    printf("clz_log2_ceil(2^64 - 1) = %u\n", clz_log2_ceil(UINT64_MAX)); // 64
}

inline uint32_t clz_log2_ceil_(uint64_t x) {
    return static_cast<uint32_t>(64 - __builtin_clzll(x | 1) - ((x & (x - 1)) == 0));
}

inline uint32_t clz_log2_floor(uint64_t value) {
    return static_cast<uint32_t>(__builtin_clzll(value | 1) ^ 63);
}

inline uint32_t clz_log2_ceil(uint64_t value) {
    return clz_log2_floor(value) + ((value & (value - 1)) != 0);
}

inline uint32_t lzcnt_log2_floor(uint64_t value) {
    // | 1 does not affect ans for values >= 1.
    return (static_cast<uint32_t>(_lzcnt_u64(value | 1) ^ 63));
}

inline uint32_t lzcnt_log2_ceil(uint64_t value) {
    return lzcnt_log2_floor(value) + ((value & (value - 1)) != 0);
}

// Windows
// g++ -Wall -Wextra -O3 -mlzcnt IntrinsicLog.cpp -o IntrinsicLog.exe && ./IntrinsicLog.exe
// Linux
// g++ -Wall -Wextra -O3 -mlzcnt IntrinsicLog.cpp -o IntrinsicLog.out && ./IntrinsicLog.out

// Power shell
// & g++ -Wall -Wextra -O3 -mlzcnt IntrinsicLog.cpp -o IntrinsicLog.exe ; ./IntrinsicLog.exe
