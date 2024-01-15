#include <bitset>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>

#include "config_macros.hpp"

/// @brief Transposes 8x8 matrix in `src` and puts it in into `dst`. `src` may
/// be equal to `dst` (inplace transposition)
/// @details See Hackers Delight for more info.
/// @tparam kReversedBits
/// @param src source 8x8 matrix
/// @param dst destination 8x8 matrix
template <bool kReversedBits = false>
constexpr void transpose8(const uint8_t src[8], uint8_t dst[8]) noexcept {
    uint64_t x = 0;

    /**
     * To unroll loops is important here (one can check it via godbolt)
     *
     * if kReversedBits == false, all 8 lines above can be done using 1
     * instruction: mov %register_for_x, QWORD PTR[%register_with_src]
     *
     *  movq %register_for_x %register_with_src
     *
     * Otherwise, if kReversedBits == true, it can be done using 2 instructions:
     *  mov %register_for_x, QWORD PTR[%register_with_src]
     *  bswap %register_for_x
     *
     * Or using 1 instruction if target has `movbe`:
     *  movbe %register_for_x, QWORD PTR[%register_with_src]
     */
    if constexpr (!kReversedBits) {
        x = src[7];
        x = (x << 8) | src[6];
        x = (x << 8) | src[5];
        x = (x << 8) | src[4];
        x = (x << 8) | src[3];
        x = (x << 8) | src[2];
        x = (x << 8) | src[1];
        x = (x << 8) | src[0];
    } else {
        x = src[0];
        x = (x << 8) | src[1];
        x = (x << 8) | src[2];
        x = (x << 8) | src[3];
        x = (x << 8) | src[4];
        x = (x << 8) | src[5];
        x = (x << 8) | src[6];
        x = (x << 8) | src[7];
    }

    x = (x & 0xAA55AA55AA55AA55ULL) | ((x & 0x00AA00AA00AA00AAULL) << 7) |
        ((x >> 7) & 0x00AA00AA00AA00AAULL);
    x = (x & 0xCCCC3333CCCC3333ULL) | ((x & 0x0000CCCC0000CCCCULL) << 14) |
        ((x >> 14) & 0x0000CCCC0000CCCCULL);
    x = (x & 0xF0F0F0F00F0F0F0FULL) | ((x & 0x00000000F0F0F0F0ULL) << 28) |
        ((x >> 28) & 0x00000000F0F0F0F0ULL);

    if constexpr (!kReversedBits) {
        dst[0] = uint8_t(x);
        x >>= 8;
        dst[1] = uint8_t(x);
        x >>= 8;
        dst[2] = uint8_t(x);
        x >>= 8;
        dst[3] = uint8_t(x);
        x >>= 8;
        dst[4] = uint8_t(x);
        x >>= 8;
        dst[5] = uint8_t(x);
        x >>= 8;
        dst[6] = uint8_t(x);
        x >>= 8;
        dst[7] = uint8_t(x);
    } else {
        dst[7] = uint8_t(x);
        x >>= 8;
        dst[6] = uint8_t(x);
        x >>= 8;
        dst[5] = uint8_t(x);
        x >>= 8;
        dst[4] = uint8_t(x);
        x >>= 8;
        dst[3] = uint8_t(x);
        x >>= 8;
        dst[2] = uint8_t(x);
        x >>= 8;
        dst[1] = uint8_t(x);
        x >>= 8;
        dst[0] = uint8_t(x);
    }
}

/// @brief Transposes 32x32 matrix in `src` inplace.
/// @details See Hackers Delight for more info.
/// @tparam kReversedBits
/// @param src source 32x32 matrix
template <bool kReversedBits = false>
constexpr void transpose32(uint32_t src[32]) noexcept {
    uint32_t m = 0x0000FFFFU;
    for (size_t j = 16; j != 0; j >>= 1, m ^= (m << j)) {
        for (size_t k = 0; k < 32; k = (k + j + 1) & ~j) {
            if constexpr (!kReversedBits) {
                uint32_t t = (src[k + j] ^ (src[k] >> j)) & m;
                src[k + j] ^= t;
                src[k] ^= (t << j);
            } else {
                uint32_t t = (src[k] ^ (src[k + j] >> j)) & m;
                src[k] ^= t;
                src[k + j] ^= (t << j);
            }
        }
    }
}

/// @brief Transposes 32x32 matrix in `src` and puts it in into `dst`. `src` and
/// `dst` can not overlap (otherwise, behaviour is undefined)
/// @param src
/// @param dst
void transpose32(const uint32_t RESTRICT_QUALIFIER src[32],
                 uint32_t RESTRICT_QUALIFIER dst[32]) noexcept {
    if (likely(dst != src)) {
        // Let compiler vectorize this memcpy call
        memcpy(dst, src, sizeof(uint32_t) * 32);
    }
    transpose32(dst);
}

// template <size_t kSize>
// constexpr size_t kSize = 32;
// class bitmatrix {
//     using word_t = uint32_t;
//     static constexpr size_t kWordBits = sizeof(word_t) * __CHAR_BIT__;
//     static constexpr size_t kActualSize = (kSize + 31) & ~31;

// public:
//     uint32_t operator()(size_t i, size_t j) const noexcept {
//         words_[i / kWordSize]
//     }
// private:
//     word_t words_[kActualSize] = {};
// };

void test_8x8() {
    uint8_t a[8] = {
        0b00011000,
        0b00011000,
        0b11111111,
        0b01101110,
        0b01100111,
        0b11111111,
        0b00011000,
        0b00011000,
    };
    constexpr uint8_t b[8] = {
        0b00110100,
        0b00111100,
        0b00111100,
        0b11101111,
        0b11100111,
        0b00111100,
        0b00111100,
        0b00100100,
    };
    static_assert(sizeof(a) == sizeof(b));
    transpose8(a, a);
    assert(memcmp(a, b, 8) == 0);
}

void test_32x32() {
    uint32_t a[32] = {
        0b00011000000000000000000000000001U,
        0b00011000000000000000000000000010U,
        0b11111111000000000000000000001100U,
        0b01101110000000000000000000000000U,
        0b01100111000000000000000000000000U,
        0b11111111000000000000000000000000U,
        0b00011000000000000000000000000000U,
        0b00011000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
        0b00000000000000000000000000000000U,
    };
    for (uint32_t row : a) {
        std::cout << std::bitset<32>(row) << '\n';
    }

    transpose32(a);
    for (uint32_t row : a) {
        std::cout << std::bitset<32>(row) << '\n';
    }
}

int main() {
    test_8x8();
    test_32x32();
}
