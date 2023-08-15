#include <cstdio>
#include <cstdint>

inline uint32_t de_bruijn_log2(uint32_t value);

inline uint32_t de_bruijn_log2(uint64_t value);

int main() {
    constexpr uint64_t pow2_to34 = 1ull << 34;
    printf("De Bruijn log2(511) = %u\n", de_bruijn_log2(511u));              // 8
    printf("De Bruijn log2(512) = %u\n", de_bruijn_log2(512u));              // 9
    printf("De Bruijn log2(513) = %u\n", de_bruijn_log2(513u));              // 9
    printf("De Bruijn log2(1023) = %u\n", de_bruijn_log2(1023u));            // 9
    printf("De Bruijn log2(1024) = %u\n", de_bruijn_log2(1024u));            // 10
    printf("De Bruijn log2(1025) = %u\n", de_bruijn_log2(1025u));            // 10
    printf("De Bruijn log2(2 ^ 34) = %u\n", de_bruijn_log2(pow2_to34));      // 34
    printf("De Bruijn log2(2 ^ 64 - 1) = %u\n", de_bruijn_log2(UINT64_MAX)); // 63
}

// Taken from .NET C# library

/*
 *  Returns the integer (floor) log of the specified value, base 2.
 *  Note that by convention, input value 0 returns 0 since log(0) is undefined.
 */
inline uint32_t de_bruijn_log2(uint64_t value) {
    uint32_t hi = static_cast<uint32_t>(value >> 32);
    return (hi != 0) ? (de_bruijn_log2(hi) + 32) : de_bruijn_log2(static_cast<uint32_t>(value));
}

// Taken from .NET C# library

/*
 *  Returns the integer (floor) log of the specified value, base 2.
 *  Note that by convention, input value 0 returns 0 since log(0) is undefined.
 */
inline uint32_t de_bruijn_log2(uint32_t value) {
    static const uint32_t MultiplyDeBruijnBitPosition[32] = 
    {
        0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
        8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
    };

    value |= (value >>  1); // first round down to one less than a power of 2 
    value |= (value >>  2);
    value |= (value >>  4);
    value |= (value >>  8);
    value |= (value >> 16);

    // Using deBruijn sequence, k=2, n=5 (2^5=32) : 0b_0000_0111_1100_0100_1010_1100_1101_1101
    return MultiplyDeBruijnBitPosition[((value * 0x07C4ACDDu) >> 27)];
}
