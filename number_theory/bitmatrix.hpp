#pragma once

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "config_macros.hpp"

// clang-format off

/// @brief Transposes 8x8 matrix @a src and puts it in into @a dst
///        @a src may be equal to @a dst (inplace transposition)
/// @details See Hackers Delight for more info.
/// @tparam AgainstMinorDiagonal bool flag (see note below)
/// @param src source 8x8 matrix
/// @param dst destination 8x8 matrix
///
/// @note explanation of @a AgainstMinorDiagonal
///
/// Suppose we are given 8x8 matrix M =
/// {
///     0b00001111,
///     0b00000000,
///     0b00001111,
///     0b00000000,
///     0b00001111,
///     0b00000000,
///     0b00001111,
///     0b00000000,
/// }
///
/// If AgainstMinorDiagonal = false, M will become
/// {
///     0b00000000,
///     0b00000000,
///     0b00000000,
///     0b00000000,
///     0b10101010,
///     0b10101010,
///     0b10101010,
///     0b10101010,
/// }
///
/// Otherwise, M will become
/// {
///     0b01010101,
///     0b01010101,
///     0b01010101,
///     0b01010101,
///     0b00000000,
///     0b00000000,
///     0b00000000,
///     0b00000000,
/// }
template <bool AgainstMinorDiagonal = false>
ATTRIBUTE_ACCESS(read_only, 1)
ATTRIBUTE_ACCESS(write_only, 2)
ATTRIBUTE_NONNULL(1, 2)
constexpr void transpose8(const uint8_t src[8], uint8_t dst[8]) noexcept {
    uint64_t x = 0;

    /**
     * To unroll loops is important here (one can check it via godbolt)
     *
     * if AgainstMinorDiagonal == false, all 8 lines above can be done using 1
     * instruction: mov %register_for_x, QWORD PTR[%register_with_src]
     *
     *  movq %register_for_x %register_with_src
     *
     * Otherwise, if AgainstMinorDiagonal == true, it can be done using 2 instructions:
     *  mov %register_for_x, QWORD PTR[%register_with_src]
     *  bswap %register_for_x
     *
     * Or using 1 instruction if target has `movbe`:
     *  movbe %register_for_x, QWORD PTR[%register_with_src]
     */
    if constexpr (!AgainstMinorDiagonal) {
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

    if constexpr (!AgainstMinorDiagonal) {
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

/// @brief Transposes 8x8 matrix @a src inplace
/// @tparam AgainstMinorDiagonal 
/// @param src source 8x8 matrix
/// @note see @c transpose8(const uint8_t[8], uint8_t[8]) for the explanation of @a AgainstMinorDiagonal
template <bool AgainstMinorDiagonal = false>
ATTRIBUTE_ACCESS(read_write, 1)
ATTRIBUTE_NONNULL(1)
constexpr void transpose8(uint8_t src[8]) noexcept {
    transpose8<AgainstMinorDiagonal>(src, src);
}

/// @brief Transposes 32x32 matrix @a src inplace.
/// @details See Hackers Delight for more info
/// @tparam AgainstMinorDiagonal
/// @param src source 32x32 matrix
/// @note see @c transpose8(const uint8_t[8], uint8_t[8]) for the explanation of @a AgainstMinorDiagonal
template <bool AgainstMinorDiagonal = false>
ATTRIBUTE_ACCESS(read_write, 1)
ATTRIBUTE_NONNULL(1)
constexpr void transpose32(uint32_t src[32]) noexcept {
    uint32_t m = 0x0000FFFFU;
    /**
     * mask m values are {
     *  0x0000FFFF for j = 16
     *  0x00FF00FF for j = 8
     *  0x0F0F0F0F for j = 4
     *  0x33333333 for j = 2
     *  0x55555555 for j = 1
     * }
     */
    for (std::size_t j = 16; j != 0; j >>= 1, m ^= (m << j)) {
        for (std::size_t k = 0; k < 32; k = (k + j + 1) & ~j) {
            if constexpr (!AgainstMinorDiagonal) {
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

/// @brief Transposes 32x32 matrix @a src and puts it in into @a dst .
///        @a src and @a dst may overlap.
/// @tparam AgainstMinorDiagonal 
/// @param src
/// @param dst
/// @note see @c transpose8(const uint8_t[8], uint8_t[8]) for the explanation of @a AgainstMinorDiagonal
template <bool AgainstMinorDiagonal = false>
ATTRIBUTE_ACCESS(read_only, 1)
ATTRIBUTE_NONNULL(1, 2)
constexpr void transpose32(const uint32_t src[32], uint32_t dst[32]) noexcept {
    if (dst < src) {
        std::copy(&src[0], &src[32], &dst[0]);
    } else if (dst > src) {
        std::copy_backward(&src[0], &src[32], &dst[32]);
    }
    transpose32(dst);
}

/// @brief Transposes 64x64 matrix in `src` inplace.
/// @details See Hackers Delight for more info.
/// @tparam AgainstMinorDiagonal
/// @param src source 64x64 matrix
template <bool AgainstMinorDiagonal = false>
ATTRIBUTE_ACCESS(read_write, 1)
ATTRIBUTE_NONNULL(1)
constexpr void transpose64(uint64_t src[64]) noexcept {
    uint64_t m = 0x00000000FFFFFFFFULL;
    /**
     * mask m values are {
     *  0x00000000FFFFFFFF for j = 32
     *  0x0000FFFF0000FFFF for j = 16
     *  0x00FF00FF00FF00FF for j = 8
     *  0x0F0F0F0F0F0F0F0F for j = 4
     *  0x3333333333333333 for j = 2
     *  0x5555555555555555 for j = 1
     * }
     */
    for (std::size_t j = 32; j != 0; j >>= 1, m ^= (m << j)) {
        for (std::size_t k = 0; k < 64; k = (k + j + 1) & ~j) {
            if constexpr (!AgainstMinorDiagonal) {
                uint64_t t = (src[k + j] ^ (src[k] >> j)) & m;
                src[k + j] ^= t;
                src[k] ^= (t << j);
            } else {
                uint64_t t = (src[k] ^ (src[k + j] >> j)) & m;
                src[k] ^= t;
                src[k + j] ^= (t << j);
            }
        }
    }
}

// clang-format on

#include <cassert>

inline void transpose_4096(std::bitset<4096> (&m)[4096]) noexcept {
    assert(std::bit_cast<uintptr_t>(std::addressof(m[0])) % 8 == 0);
    assert(std::bit_cast<uintptr_t>(std::addressof(m[4095])) % 8 == 0);
    uint64_t tmp1[64]{};
    uint64_t tmp2[64]{};
    for (std::size_t i = 0; i < 64; i++) {
        for (std::size_t j = i; j < 64; j++) {
            for (std::size_t k = 0; k < 64; k++) {
                tmp1[k] = *(std::bit_cast<const uint64_t*>(std::addressof(m[i * 64 + k])) + j);
                tmp2[k] = *(std::bit_cast<const uint64_t*>(std::addressof(m[j * 64 + k])) + i);
            }
            transpose64(tmp1);
            transpose64(tmp2);
            for (std::size_t k = 0; k < 64; k++) {
                *(std::bit_cast<uint64_t*>(std::addressof(m[i * 64 + k])) + j) = tmp2[k];
                *(std::bit_cast<uint64_t*>(std::addressof(m[j * 64 + k])) + i) = tmp1[k];
            }
        }
    }
}

template <std::size_t N>
#if defined(__cpp_lib_constexpr_bitset) && __cpp_lib_constexpr_bitset >= 202207L
constexpr
#endif
    inline auto
    multiply_f2(const std::bitset<N> (&lhs)[N], const std::bitset<N> (&rhs)[N]) noexcept {
    std::array<std::bitset<N>, N> result{};
    for (std::size_t i = 0; i < N; i++) {
        for (std::size_t j = 0; j < N; j++) {
            if (lhs[i][j]) {
                result[i] ^= rhs[j];
            }
        }
    }
    return result;
}

#if defined(__cpp_lib_constexpr_bitset) && __cpp_lib_constexpr_bitset >= 202207L
#define CONSTEXPR_BITSET_CXX_23 constexpr
#else
#define CONSTEXPR_BITSET_CXX_23
#endif

template <std::size_t N>
class square_bitmatrix {
private:
    static constexpr std::size_t kAlignmentBits = 1 << 6;
    static constexpr std::size_t NBits          = (N + kAlignmentBits - 1) & ~kAlignmentBits;

public:
    using container_type         = typename std::array<std::bitset<NBits>, NBits>;
    using value_type             = typename container_type::value_type;
    using pointer                = typename container_type::pointer;
    using const_pointer          = typename container_type::const_pointer;
    using reference              = typename container_type::reference;
    using const_reference        = typename container_type::const_reference;
    using iterator               = typename container_type::iterator;
    using const_iterator         = typename container_type::const_iterator;
    using size_type              = typename container_type::size_type;
    using difference_type        = typename container_type::difference_type;
    using reverse_iterator       = typename container_type::reverse_iterator;
    using const_reverse_iterator = typename container_type::const_reverse_iterator;
    using row_type               = value_type;

    constexpr reference operator[](size_type index) noexcept {
        return data_[index];
    }
    constexpr const_reference operator[](size_type index) const noexcept {
        return data_[index];
    }
    CONSTEXPR_BITSET_CXX_23 auto operator[](std::pair<size_type, size_type> indexes) noexcept {
        return data_[indexes.first][indexes.second];
    }
    constexpr bool operator[](std::pair<size_type, size_type> indexes) const noexcept {
        return data_[indexes.first][indexes.second];
    }

    CONSTEXPR_BITSET_CXX_23 square_bitmatrix& operator*=(const square_bitmatrix& other) noexcept {
        for (auto& row : data_) {
            row_type row_mult(row);
            for (std::size_t j = 0; j < N; j++) {
                if (row[j]) {
                    row_mult ^= other.data_[j];
                }
            }
            row = row_mult;
        }
        return *this;
    }
    ATTRIBUTE_PURE CONSTEXPR_BITSET_CXX_23 square_bitmatrix operator*(const square_bitmatrix& other) const noexcept {
        square_bitmatrix copy(*this);
        copy *= other;
        return copy;
    }

private:
    container_type data_{};
};
