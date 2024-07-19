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

/// @brief Transposes 32x32 matrix @a src inplace
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

/// @brief Transposes 32x32 matrix @a src and puts it in into @a dst
///        @a src and @a dst may overlap
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

/// @brief Transposes 64x64 matrix in @a src inplace
/// @details See Hackers Delight for more info
/// @tparam AgainstMinorDiagonal
/// @param src source 64x64 matrix
/// @note see @c transpose8(const uint8_t[8], uint8_t[8]) for the explanation of @a AgainstMinorDiagonal
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

#if defined(__cpp_lib_constexpr_bitset) && __cpp_lib_constexpr_bitset >= 202207L
#define CONSTEXPR_BITSET_OPS constexpr
#else
#define CONSTEXPR_BITSET_OPS
#endif
#if defined(__cpp_constexpr) && __cpp_constexpr >= 202306L
#define CONSTEXPR_POINTER_CAST constexpr
#else
#define CONSTEXPR_POINTER_CAST
#endif

namespace detail::square_matrix_detail {

inline constexpr std::size_t kAlignmentBits = 64;

}  // namespace detail::square_matrix_detail

template <std::size_t N>
class alignas(detail::square_matrix_detail::kAlignmentBits) square_bitmatrix {
private:
    static constexpr std::size_t kAlignmentBits = detail::square_matrix_detail::kAlignmentBits;
    static constexpr std::size_t kBits          = (N + kAlignmentBits - 1) & ~kAlignmentBits;

public:
    using matrix_type            = typename std::array<std::bitset<kBits>, kBits>;
    using value_type             = typename matrix_type::value_type;
    using pointer                = typename matrix_type::pointer;
    using const_pointer          = typename matrix_type::const_pointer;
    using reference              = typename matrix_type::reference;
    using const_reference        = typename matrix_type::const_reference;
    using iterator               = typename matrix_type::iterator;
    using const_iterator         = typename matrix_type::const_iterator;
    using size_type              = typename matrix_type::size_type;
    using difference_type        = typename matrix_type::difference_type;
    using reverse_iterator       = typename matrix_type::reverse_iterator;
    using const_reverse_iterator = typename matrix_type::const_reverse_iterator;
    using row_type               = value_type;

    [[nodiscard]] constexpr reference operator[](size_type index) noexcept {
        return data_[index];
    }
    [[nodiscard]] constexpr const_reference operator[](size_type index) const noexcept {
        return data_[index];
    }
    [[nodiscard]] CONSTEXPR_BITSET_OPS auto operator[](
        std::pair<size_type, size_type> indexes) noexcept {
        return data_[indexes.first][indexes.second];
    }
    [[nodiscard]] constexpr bool operator[](
        std::pair<size_type, size_type> indexes) const noexcept {
        return data_[indexes.first][indexes.second];
    }

    CONSTEXPR_BITSET_OPS square_bitmatrix& operator*=(const square_bitmatrix& other) noexcept {
        do_multiply_lhs_inplace(data_, other.data_);
        return *this;
    }
    [[nodiscard]] ATTRIBUTE_PURE CONSTEXPR_BITSET_OPS square_bitmatrix
    operator*(const square_bitmatrix& other) const noexcept {
        square_bitmatrix copy(*this);
        copy *= other;
        return copy;
    }
    CONSTEXPR_POINTER_CAST square_bitmatrix& transpose_inplace() noexcept {
        do_transpose_inplace(data_);
        return *this;
    }
    [[nodiscard]] ATTRIBUTE_PURE CONSTEXPR_POINTER_CAST square_bitmatrix
    transpose() const noexcept {
        square_bitmatrix copy(*this);
        copy.transpose_inplace();
        return copy;
    }
    [[nodiscard]] ATTRIBUTE_PURE CONSTEXPR_POINTER_CAST square_bitmatrix T() const noexcept {
        return transpose();
    }

private:
    CONSTEXPR_BITSET_OPS void do_multiply_lhs_inplace(matrix_type& lhs,
                                                      const matrix_type& rhs) noexcept {
        auto iter           = lhs.begin();
        const auto iter_end = iter + N;
        for (; iter != iter_end; ++iter) {
            const row_type& row = *iter;
            row_type row_mult(row);
            for (std::size_t j = 0; j < N; j++) {
                if (row[j]) {
                    row_mult ^= rhs[j];
                }
            }
            *iter = row_mult;
        }
    }
    CONSTEXPR_POINTER_CAST void do_transpose_inplace(matrix_type& matrix) noexcept {
        static_assert(kAlignmentBits == 64);
        uint64_t tmp1[kAlignmentBits]{};
        uint64_t tmp2[kAlignmentBits]{};

        for (std::size_t i = 0; i < kBits / kAlignmentBits; i++) {
            for (std::size_t j = i; j < kBits / kAlignmentBits; j++) {
                for (std::size_t k = 0; k < kAlignmentBits; k++) {
                    tmp1[k] = std::bit_cast<const uint64_t*>(
                        std::addressof(matrix[i * kAlignmentBits + k]))[j];
                    tmp2[k] = std::bit_cast<const uint64_t*>(
                        std::addressof(matrix[j * kAlignmentBits + k]))[i];
                }
                transpose64(tmp1);
                transpose64(tmp2);
                for (std::size_t k = 0; k < 64; k++) {
                    std::bit_cast<uint64_t*>(std::addressof(matrix[i * kAlignmentBits + k]))[j] =
                        tmp2[k];
                    std::bit_cast<uint64_t*>(std::addressof(matrix[j * kAlignmentBits + k]))[i] =
                        tmp1[k];
                }
            }
        }
    }

    matrix_type data_{};
};

#undef CONSTEXPR_POINTER_CAST
#undef CONSTEXPR_BITSET_OPS
