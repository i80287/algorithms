#include <cstdio>
#include <cstdint>

// #pragma GCC optimize("unroll-loops")

uint32_t ceiled_log2(uint64_t x) {
    static const uint64_t t[6] = {
        0xFFFFFFFF00000000ull,
        0x00000000FFFF0000ull,
        0x000000000000FF00ull,
        0x00000000000000F0ull,
        0x000000000000000Cull,
        0x0000000000000002ull
    };

    uint32_t y = ((x & (x - 1)) != 0);
    uint32_t j = 32;
    
    for (size_t i = 0; i != 6; ++i) {
        uint32_t k = (((x & t[i]) == 0) ? 0 : j);
        y += k;
        x >>= k;

        j >>= 1;
    }

    return y;
}

int main() {
    /*
     * ceiled_log2(511) = 9
     * ceiled_log2(512) = 9
     * ceiled_log2(513) = 10
     * ceiled_log2(1023) = 10
     * ceiled_log2(1024) = 10
     * ceiled_log2(1025) = 11
     */
    printf("ceiled_log2(511) = %u\n", ceiled_log2(511));
    printf("ceiled_log2(512) = %u\n", ceiled_log2(512));
    printf("ceiled_log2(513) = %u\n", ceiled_log2(513));
    printf("ceiled_log2(1023) = %u\n", ceiled_log2(1023));
    printf("ceiled_log2(1024) = %u\n", ceiled_log2(1024));
    printf("ceiled_log2(1025) = %u\n", ceiled_log2(1025));
} 
