#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <functional>
#include <memory>
#include <numeric>
#include <sstream>
#include <type_traits>
#include <utility>

#include "../misc/config_macros.hpp"

#if CONFIG_HAS_AT_LEAST_CXX_20 && CONFIG_HAS_INCLUDE(<concepts>)
#include <concepts>
#endif

// clang-format off
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
// clang-format on

using CStyleMatrix8x8 = uint8_t[8];
using Matrix8x8       = std::array<uint8_t, 8>;

using CStyleMatrix32x32 = uint32_t[32];
using Matrix32x32       = std::array<uint32_t, 32>;

using CStyleMatrix64x64 = uint64_t[64];
using Matrix64x64       = std::array<uint64_t, 64>;

namespace bitmatrix_detail {

template <class T>
inline constexpr bool kIsMatrix8x8 =
    std::is_same_v<T, Matrix8x8> || std::is_same_v<T, CStyleMatrix8x8>;

template <class T>
inline constexpr bool kIsMatrix32x32 =
    std::is_same_v<T, Matrix32x32> || std::is_same_v<T, CStyleMatrix32x32>;

template <class T>
inline constexpr bool kIsMatrix64x64 =
    std::is_same_v<T, Matrix64x64> || std::is_same_v<T, CStyleMatrix64x64>;

}  // namespace bitmatrix_detail

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
/// If AgainstMinorDiagonal = true, M will become
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
template <bool AgainstMinorDiagonal = false, class SrcMatrixType, class DstMatrixType>
ATTRIBUTE_ACCESS(read_only, 1)
ATTRIBUTE_ACCESS(write_only, 2)
constexpr void transpose8(const SrcMatrixType& src, DstMatrixType& dst) noexcept {
    static_assert(bitmatrix_detail::kIsMatrix8x8<SrcMatrixType>, "8x8 matrix as std::array or c-style array (both of uint8_t) was expected");
    static_assert(bitmatrix_detail::kIsMatrix8x8<DstMatrixType>, "8x8 matrix as std::array or c-style array (both of uint8_t) was expected");
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
        x = (x << 8U) | src[6];
        x = (x << 8U) | src[5];
        x = (x << 8U) | src[4];
        x = (x << 8U) | src[3];
        x = (x << 8U) | src[2];
        x = (x << 8U) | src[1];
        x = (x << 8U) | src[0];
    } else {
        x = src[0];
        x = (x << 8U) | src[1];
        x = (x << 8U) | src[2];
        x = (x << 8U) | src[3];
        x = (x << 8U) | src[4];
        x = (x << 8U) | src[5];
        x = (x << 8U) | src[6];
        x = (x << 8U) | src[7];
    }

    x = (x & 0xAA55AA55AA55AA55ULL) | ((x & 0x00AA00AA00AA00AAULL) << 7U) |
        ((x >> 7U) & 0x00AA00AA00AA00AAULL);
    x = (x & 0xCCCC3333CCCC3333ULL) | ((x & 0x0000CCCC0000CCCCULL) << 14U) |
        ((x >> 14U) & 0x0000CCCC0000CCCCULL);
    x = (x & 0xF0F0F0F00F0F0F0FULL) | ((x & 0x00000000F0F0F0F0ULL) << 28U) |
        ((x >> 28U) & 0x00000000F0F0F0F0ULL);

    if constexpr (!AgainstMinorDiagonal) {
        dst[0] = static_cast<uint8_t>(x);
        x >>= 8U;
        dst[1] = static_cast<uint8_t>(x);
        x >>= 8U;
        dst[2] = static_cast<uint8_t>(x);
        x >>= 8U;
        dst[3] = static_cast<uint8_t>(x);
        x >>= 8U;
        dst[4] = static_cast<uint8_t>(x);
        x >>= 8U;
        dst[5] = static_cast<uint8_t>(x);
        x >>= 8U;
        dst[6] = static_cast<uint8_t>(x);
        x >>= 8U;
        dst[7] = static_cast<uint8_t>(x);
    } else {
        dst[7] = static_cast<uint8_t>(x);
        x >>= 8U;
        dst[6] = static_cast<uint8_t>(x);
        x >>= 8U;
        dst[5] = static_cast<uint8_t>(x);
        x >>= 8U;
        dst[4] = static_cast<uint8_t>(x);
        x >>= 8U;
        dst[3] = static_cast<uint8_t>(x);
        x >>= 8U;
        dst[2] = static_cast<uint8_t>(x);
        x >>= 8U;
        dst[1] = static_cast<uint8_t>(x);
        x >>= 8U;
        dst[0] = static_cast<uint8_t>(x);
    }
}

/// @brief Transposes 8x8 matrix @a src inplace
/// @tparam AgainstMinorDiagonal 
/// @param src source 8x8 matrix
/// @note see @c transpose8(const uint8_t[8], uint8_t[8]) for the explanation of @a AgainstMinorDiagonal
template <bool AgainstMinorDiagonal = false, class MatrixType>
ATTRIBUTE_ACCESS(read_write, 1)
constexpr void transpose8(MatrixType& src) noexcept {
    transpose8<AgainstMinorDiagonal>(src, src);
}

/// @brief Transposes 32x32 matrix @a src inplace
/// @details See Hackers Delight for more info
/// @tparam AgainstMinorDiagonal
/// @param src source 32x32 matrix
/// @note see @c transpose8(const uint8_t[8], uint8_t[8]) for the explanation of @a AgainstMinorDiagonal
template <bool AgainstMinorDiagonal = false, class MatrixType>
ATTRIBUTE_ACCESS(read_write, 1)
constexpr void transpose32(MatrixType& src) noexcept {
    static_assert(bitmatrix_detail::kIsMatrix32x32<MatrixType>, "32x32 matrix as std::array or c-style array (both of uint32_t) was expected");

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
    for (std::size_t j = 16; j != 0; j >>= 1U, m ^= (m << j)) {
        for (std::size_t k = 0; k < 32; k = (k + j + 1) & ~j) {
            if constexpr (!AgainstMinorDiagonal) {
                const uint32_t t = (src[k + j] ^ (src[k] >> j)) & m;
                src[k + j] ^= t;
                src[k] ^= (t << j);
            } else {
                const uint32_t t = (src[k] ^ (src[k + j] >> j)) & m;
                src[k] ^= t;
                src[k + j] ^= (t << j);
            }
        }
    }
}

/// @brief Transposes 64x64 matrix in @a src inplace
/// @details See Hackers Delight for more info
/// @tparam AgainstMinorDiagonal
/// @param src source 64x64 matrix
/// @note see @c transpose8(const uint8_t[8], uint8_t[8]) for the explanation of @a AgainstMinorDiagonal
template <bool AgainstMinorDiagonal = false, class MatrixType>
ATTRIBUTE_ACCESS(read_write, 1)
constexpr void transpose64(MatrixType& src) noexcept {
    static_assert(bitmatrix_detail::kIsMatrix64x64<MatrixType>, "64x64 matrix as std::array or c-style array (both of uint64_t) was expected");

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
    for (std::size_t j = 32; j != 0; j >>= 1U, m ^= (m << j)) {
        for (std::size_t k = 0; k < 64; k = (k + j + 1) & ~j) {
            if constexpr (!AgainstMinorDiagonal) {
                const uint64_t t = (src[k + j] ^ (src[k] >> j)) & m;
                src[k + j] ^= t;
                src[k] ^= (t << j);
            } else {
                const uint64_t t = (src[k] ^ (src[k + j] >> j)) & m;
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

namespace bitmatrix_detail {

struct square_bitmatrix_helper {
private:
    static constexpr bool kCanUseUInt64 =
        sizeof(std::bitset<64 + 1>) == sizeof(std::bitset<64 + 64>);
    static constexpr bool kCanUseUInt32 =
        !kCanUseUInt64 && sizeof(std::bitset<32 + 1>) == sizeof(std::bitset<32 + 32>);
    static constexpr bool kUseUInt8 = !kCanUseUInt64 && !kCanUseUInt32;
    static_assert(!kUseUInt8 || sizeof(std::bitset<8 + 1>) == sizeof(std::bitset<8 + 8>),
                  "Unsupported platform");

public:
    template <std::size_t N>
    using word_type = std::conditional_t<
        (N > 32) && kCanUseUInt64,
        std::uint64_t,
        std::conditional_t<(N > 8) && kCanUseUInt32, std::uint32_t, std::uint8_t>>;
};

#if CONFIG_HAS_AT_LEAST_CXX_20

template <class WordType, std::size_t Size>
concept Transposable64 = Size == 64 && requires(WordType (&m)[64]) {
    { transpose64(m) };
};
template <class WordType, std::size_t Size>
concept Transposable32 = Size == 32 && requires(WordType (&m)[32]) {
    { transpose32(m) };
};
template <class WordType, std::size_t Size>
concept Transposable8 = Size == 8 && requires(WordType (&m)[8]) {
    { transpose8(m) };
};

#endif

}  // namespace bitmatrix_detail

template <std::size_t N,
          class word_type = typename bitmatrix_detail::square_bitmatrix_helper::word_type<N>>
#if CONFIG_HAS_AT_LEAST_CXX_20
    requires(N > 0)
#endif
struct alignas(std::uint64_t) alignas(word_type) square_bitmatrix {
private:
    static_assert(N > 0, "Matrix can't have zero size");
    static_assert(CHAR_BIT == 8, "Platform not supported");
    static_assert(std::is_unsigned_v<word_type>, "word_type should be unsigned integral type");

    static constexpr std::size_t kAlignmentBits = sizeof(word_type) * CHAR_BIT;
    static constexpr std::size_t kBits          = (N + kAlignmentBits - 1) & ~(kAlignmentBits - 1);
    static_assert(sizeof(std::bitset<N>) == sizeof(std::bitset<kBits>),
                  "Invalid word_type passed to the square_bitmatrix");

    template <class Function>
    static constexpr bool is_noexcept_index_fn() noexcept {
        return noexcept(std::declval<Function>()(size_type{}));
    }
    template <class Function>
    static constexpr bool is_noexcept_coords_fn() noexcept {
        return noexcept(std::declval<Function>()(size_type{}, size_type{}));
    }
    template <class Function>
    static constexpr bool is_noexcept_bit_reference_fn() noexcept {
        return noexcept(std::declval<Function>()(std::declval<bit_reference>()));
    }

public:
    using matrix_type            = typename std::array<std::bitset<N>, kBits>;
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
    using bit_reference          = typename value_type::reference;

    // Made public for the simple initialization from the
    // initializers like {0b00, 0b10, ...} (like in the std::array)
    matrix_type data_{};

    [[nodiscard]] ATTRIBUTE_CONST static CONSTEXPR_BITSET_OPS square_bitmatrix identity() noexcept {
        if constexpr (N <= kAlignmentBits) {
            if constexpr (kAlignmentBits == 64) {
                return {
                    0b0000000000000000000000000000000000000000000000000000000000000001ULL,
                    0b0000000000000000000000000000000000000000000000000000000000000010ULL,
                    0b0000000000000000000000000000000000000000000000000000000000000100ULL,
                    0b0000000000000000000000000000000000000000000000000000000000001000ULL,
                    0b0000000000000000000000000000000000000000000000000000000000010000ULL,
                    0b0000000000000000000000000000000000000000000000000000000000100000ULL,
                    0b0000000000000000000000000000000000000000000000000000000001000000ULL,
                    0b0000000000000000000000000000000000000000000000000000000010000000ULL,
                    0b0000000000000000000000000000000000000000000000000000000100000000ULL,
                    0b0000000000000000000000000000000000000000000000000000001000000000ULL,
                    0b0000000000000000000000000000000000000000000000000000010000000000ULL,
                    0b0000000000000000000000000000000000000000000000000000100000000000ULL,
                    0b0000000000000000000000000000000000000000000000000001000000000000ULL,
                    0b0000000000000000000000000000000000000000000000000010000000000000ULL,
                    0b0000000000000000000000000000000000000000000000000100000000000000ULL,
                    0b0000000000000000000000000000000000000000000000001000000000000000ULL,
                    0b0000000000000000000000000000000000000000000000010000000000000000ULL,
                    0b0000000000000000000000000000000000000000000000100000000000000000ULL,
                    0b0000000000000000000000000000000000000000000001000000000000000000ULL,
                    0b0000000000000000000000000000000000000000000010000000000000000000ULL,
                    0b0000000000000000000000000000000000000000000100000000000000000000ULL,
                    0b0000000000000000000000000000000000000000001000000000000000000000ULL,
                    0b0000000000000000000000000000000000000000010000000000000000000000ULL,
                    0b0000000000000000000000000000000000000000100000000000000000000000ULL,
                    0b0000000000000000000000000000000000000001000000000000000000000000ULL,
                    0b0000000000000000000000000000000000000010000000000000000000000000ULL,
                    0b0000000000000000000000000000000000000100000000000000000000000000ULL,
                    0b0000000000000000000000000000000000001000000000000000000000000000ULL,
                    0b0000000000000000000000000000000000010000000000000000000000000000ULL,
                    0b0000000000000000000000000000000000100000000000000000000000000000ULL,
                    0b0000000000000000000000000000000001000000000000000000000000000000ULL,
                    0b0000000000000000000000000000000010000000000000000000000000000000ULL,
                    0b0000000000000000000000000000000100000000000000000000000000000000ULL,
                    0b0000000000000000000000000000001000000000000000000000000000000000ULL,
                    0b0000000000000000000000000000010000000000000000000000000000000000ULL,
                    0b0000000000000000000000000000100000000000000000000000000000000000ULL,
                    0b0000000000000000000000000001000000000000000000000000000000000000ULL,
                    0b0000000000000000000000000010000000000000000000000000000000000000ULL,
                    0b0000000000000000000000000100000000000000000000000000000000000000ULL,
                    0b0000000000000000000000001000000000000000000000000000000000000000ULL,
                    0b0000000000000000000000010000000000000000000000000000000000000000ULL,
                    0b0000000000000000000000100000000000000000000000000000000000000000ULL,
                    0b0000000000000000000001000000000000000000000000000000000000000000ULL,
                    0b0000000000000000000010000000000000000000000000000000000000000000ULL,
                    0b0000000000000000000100000000000000000000000000000000000000000000ULL,
                    0b0000000000000000001000000000000000000000000000000000000000000000ULL,
                    0b0000000000000000010000000000000000000000000000000000000000000000ULL,
                    0b0000000000000000100000000000000000000000000000000000000000000000ULL,
                    0b0000000000000001000000000000000000000000000000000000000000000000ULL,
                    0b0000000000000010000000000000000000000000000000000000000000000000ULL,
                    0b0000000000000100000000000000000000000000000000000000000000000000ULL,
                    0b0000000000001000000000000000000000000000000000000000000000000000ULL,
                    0b0000000000010000000000000000000000000000000000000000000000000000ULL,
                    0b0000000000100000000000000000000000000000000000000000000000000000ULL,
                    0b0000000001000000000000000000000000000000000000000000000000000000ULL,
                    0b0000000010000000000000000000000000000000000000000000000000000000ULL,
                    0b0000000100000000000000000000000000000000000000000000000000000000ULL,
                    0b0000001000000000000000000000000000000000000000000000000000000000ULL,
                    0b0000010000000000000000000000000000000000000000000000000000000000ULL,
                    0b0000100000000000000000000000000000000000000000000000000000000000ULL,
                    0b0001000000000000000000000000000000000000000000000000000000000000ULL,
                    0b0010000000000000000000000000000000000000000000000000000000000000ULL,
                    0b0100000000000000000000000000000000000000000000000000000000000000ULL,
                    0b1000000000000000000000000000000000000000000000000000000000000000ULL,
                };
            } else if constexpr (kAlignmentBits == 32) {
                return {
                    0b00000000000000000000000000000001U, 0b00000000000000000000000000000010U,
                    0b00000000000000000000000000000100U, 0b00000000000000000000000000001000U,
                    0b00000000000000000000000000010000U, 0b00000000000000000000000000100000U,
                    0b00000000000000000000000001000000U, 0b00000000000000000000000010000000U,
                    0b00000000000000000000000100000000U, 0b00000000000000000000001000000000U,
                    0b00000000000000000000010000000000U, 0b00000000000000000000100000000000U,
                    0b00000000000000000001000000000000U, 0b00000000000000000010000000000000U,
                    0b00000000000000000100000000000000U, 0b00000000000000001000000000000000U,
                    0b00000000000000010000000000000000U, 0b00000000000000100000000000000000U,
                    0b00000000000001000000000000000000U, 0b00000000000010000000000000000000U,
                    0b00000000000100000000000000000000U, 0b00000000001000000000000000000000U,
                    0b00000000010000000000000000000000U, 0b00000000100000000000000000000000U,
                    0b00000001000000000000000000000000U, 0b00000010000000000000000000000000U,
                    0b00000100000000000000000000000000U, 0b00001000000000000000000000000000U,
                    0b00010000000000000000000000000000U, 0b00100000000000000000000000000000U,
                    0b01000000000000000000000000000000U, 0b10000000000000000000000000000000U,
                };
            } else if constexpr (kAlignmentBits == 8) {
                return {
                    0b00000001U, 0b00000010U, 0b00000100U, 0b00001000U,
                    0b00010000U, 0b00100000U, 0b01000000U, 0b10000000U,
                };
            } else {
                return create_identity_matrix_impl();
            }
        } else {
            return create_identity_matrix_impl();
        }
    }
    [[nodiscard]] ATTRIBUTE_CONST static constexpr square_bitmatrix allzeros() noexcept {
        return square_bitmatrix{};
    }
    [[nodiscard]] ATTRIBUTE_CONST static CONSTEXPR_BITSET_OPS square_bitmatrix allones() noexcept {
        square_bitmatrix m{};
        for (row_type& row : m) {
            row.set();
        }
        return m;
    }

    // clang-format off

    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    constexpr iterator begin() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return data_.begin();
    }
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    constexpr iterator end() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return begin() + N;
    }
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    constexpr const_iterator begin() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return data_.begin();
    }
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    constexpr const_iterator end() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return begin() + N;
    }
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    constexpr const_iterator cbegin() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return data_.cbegin();
    }
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    constexpr const_iterator cend() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return cbegin() + N;
    }
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    constexpr pointer data() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return data_.data();
    }
    [[nodiscard]]
    ATTRIBUTE_PURE
    ATTRIBUTE_ALWAYS_INLINE
    constexpr const_pointer data() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return data_.data();
    }

    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_CONST
    constexpr size_type size() const noexcept {
        return N;
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_CONST
    constexpr size_type rows() const noexcept {
        return N;
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_CONST
    constexpr size_type columns() const noexcept {
        return N;
    }
    struct matrix_shape {
        size_type rows;
        size_type columns;
    };
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_CONST
    constexpr matrix_shape shape() const noexcept {
#if CONFIG_HAS_AT_LEAST_CXX_20
        return {
            .rows    = rows(),
            .columns = columns(),
        };
#else
        return {rows(), columns()};
#endif
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_CONST
    constexpr size_type flat_size() const noexcept {
        return rows() * columns();
    }
    ATTRIBUTE_REINITIALIZES constexpr void clear() noexcept {
        data_ = matrix_type{};
    }

    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE
    constexpr reference operator[](size_type index) noexcept ATTRIBUTE_LIFETIME_BOUND {
        return data_[index];
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE
    constexpr const_reference operator[](size_type index) const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return data_[index];
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE
    CONSTEXPR_BITSET_OPS
    bit_reference operator[](std::pair<size_type, size_type> indexes) noexcept ATTRIBUTE_LIFETIME_BOUND {
        return data_[indexes.first][indexes.second];
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE
    constexpr bool operator[](std::pair<size_type, size_type> indexes) const noexcept {
        return data_[indexes.first][indexes.second];
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE
    constexpr bool get_unchecked(std::size_t i, std::size_t j) const noexcept {
        return data_[i][j];
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE
    constexpr bool get_unchecked(std::pair<size_type, size_type> indexes) const noexcept {
        return get_unchecked(indexes.first, indexes.second);
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE
    constexpr bool get_checked(std::size_t i, std::size_t j) const {
        return data_[i].test(j);
    }
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE
    constexpr bool get_checked(std::pair<size_type, size_type> indexes) const {
        return get_checked(indexes.first, indexes.second);
    }
    ATTRIBUTE_ALWAYS_INLINE
    CONSTEXPR_BITSET_OPS
    void set_unchecked(std::size_t i, std::size_t j, bool value = true) noexcept {
        data_[i][j] = value;
    }
    ATTRIBUTE_ALWAYS_INLINE
    CONSTEXPR_BITSET_OPS
    void set_unchecked(std::pair<size_type, size_type> indexes, bool value = true) noexcept {
        set_unchecked(indexes.first, indexes.second, value);
    }
    ATTRIBUTE_ALWAYS_INLINE
    CONSTEXPR_BITSET_OPS
    void set_checked(std::size_t i, std::size_t j, bool value = true) noexcept {
        data_[i].set(j, value);
    }
    ATTRIBUTE_ALWAYS_INLINE
    CONSTEXPR_BITSET_OPS
    void set_checked(std::pair<size_type, size_type> indexes, bool value = true) noexcept {
        set_checked(indexes.first, indexes.second, value);
    }

    CONSTEXPR_BITSET_OPS
    square_bitmatrix& operator|=(const square_bitmatrix& other) noexcept ATTRIBUTE_LIFETIME_BOUND {
        do_bitwise_or(other.data());
        return *this;
    }
    [[nodiscard]]
    ATTRIBUTE_PURE
    CONSTEXPR_BITSET_OPS
    square_bitmatrix operator|(const square_bitmatrix& other) const noexcept {
        square_bitmatrix copy(*this);
        copy |= other;
        return copy;
    }
    CONSTEXPR_BITSET_OPS
    square_bitmatrix& operator&=(const square_bitmatrix& other) noexcept ATTRIBUTE_LIFETIME_BOUND {
        do_bitwise_and(other.data());
        return *this;
    }
    [[nodiscard]]
    ATTRIBUTE_PURE
    CONSTEXPR_BITSET_OPS
    square_bitmatrix operator&(const square_bitmatrix& other) const noexcept {
        square_bitmatrix copy(*this);
        copy &= other;
        return copy;
    }
    CONSTEXPR_BITSET_OPS
    square_bitmatrix& operator^=(const square_bitmatrix& other) noexcept ATTRIBUTE_LIFETIME_BOUND {
        do_bitwise_xor(other.data());
        return *this;
    }
    [[nodiscard]]
    ATTRIBUTE_PURE
    CONSTEXPR_BITSET_OPS
    square_bitmatrix operator^(const square_bitmatrix& other) const noexcept {
        square_bitmatrix copy(*this);
        copy ^= other;
        return copy;
    }
    CONSTEXPR_BITSET_OPS
    square_bitmatrix& operator*=(const square_bitmatrix& other) noexcept ATTRIBUTE_LIFETIME_BOUND {
        if (unlikely(std::addressof(*this) == std::addressof(other))) {
            square_bitmatrix copy(*this);
            do_multiply_over_z2(copy.data());
        } else {
            do_multiply_over_z2(other.data());
        }
        return *this;
    }
    [[nodiscard]]
    ATTRIBUTE_PURE
    CONSTEXPR_BITSET_OPS
    square_bitmatrix operator*(const square_bitmatrix& other) const noexcept {
        square_bitmatrix copy(*this);
        copy *= other;
        return copy;
    }
    [[nodiscard]]
    ATTRIBUTE_PURE
    CONSTEXPR_BITSET_OPS
    square_bitmatrix operator*(const row_type& vector) const noexcept {
        return do_multiply_over_z2(vector);
    }
    CONSTEXPR_BITSET_OPS
    square_bitmatrix& flip() noexcept ATTRIBUTE_LIFETIME_BOUND {
        do_flip_inplace(data_);
        return *this;
    }
    ATTRIBUTE_NODISCARD_WITH_MESSAGE("This method is not inplace. Use flip() for this purpose")
    ATTRIBUTE_PURE
    CONSTEXPR_BITSET_OPS
    square_bitmatrix operator~() const noexcept {
        square_bitmatrix copy(*this);
        copy.flip();
        return copy;
    }
    CONSTEXPR_BITSET_OPS
    square_bitmatrix& flip_row(size_type row_index) noexcept ATTRIBUTE_LIFETIME_BOUND {
        do_flip_row_inplace(row_index);
        return *this;
    }
    CONSTEXPR_BITSET_OPS
    square_bitmatrix& flip_column(size_type column_index) noexcept ATTRIBUTE_LIFETIME_BOUND {
        do_flip_column_inplace(column_index);
        return *this;
    }
    CONSTEXPR_POINTER_CAST
    square_bitmatrix& transpose() noexcept ATTRIBUTE_LIFETIME_BOUND {
        transpose_matrix(data_);
        return *this;
    }
    ATTRIBUTE_NODISCARD_WITH_MESSAGE("This method is not inplace. Use transpose() for this purpose")
    ATTRIBUTE_PURE
    CONSTEXPR_POINTER_CAST
    square_bitmatrix T() const noexcept {
        square_bitmatrix copy(*this);
        copy.transpose();
        return copy;
    }
    [[nodiscard]]
    ATTRIBUTE_PURE
    CONSTEXPR_BITSET_OPS
    size_type count() const noexcept {
        return std::accumulate(
            begin(), end(), size_type(0),
            [](size_type set_bits_count, const row_type& row)
                CONSTEXPR_BITSET_OPS noexcept { return set_bits_count + row.count(); });
    }
    [[nodiscard]]
    ATTRIBUTE_PURE
    CONSTEXPR_BITSET_OPS
    bool none() const noexcept {
        return !any();
    }
    [[nodiscard]]
    ATTRIBUTE_PURE
    CONSTEXPR_BITSET_OPS
    bool any() const noexcept {
        return std::any_of(begin(), end(), [](const row_type& row) CONSTEXPR_BITSET_OPS noexcept {
            return row.any();
        });
    }
    [[nodiscard]]
    ATTRIBUTE_PURE
    CONSTEXPR_BITSET_OPS
    bool all() const noexcept {
        return std::all_of(begin(), end(), [](const row_type& row) CONSTEXPR_BITSET_OPS noexcept {
            return row.all();
        });
    }
    ATTRIBUTE_REINITIALIZES constexpr void reset() noexcept {
        this->clear();
    }
    [[nodiscard]]
    ATTRIBUTE_PURE
    CONSTEXPR_BITSET_OPS
    bool operator==(const square_bitmatrix& other) const noexcept {
        return std::equal(begin(), end(), other.begin());
    }
#if !defined(__cpp_impl_three_way_comparison) || __cpp_impl_three_way_comparison < 201907L
    [[nodiscard]]
    ATTRIBUTE_PURE
    CONSTEXPR_BITSET_OPS
    bool operator!=(const square_bitmatrix& other) const noexcept {
        return !(*this == other);
    }
#endif

    template <class Function>
#if CONFIG_HAS_AT_LEAST_CXX_20
        requires requires(Function fn, std::size_t i, std::size_t j) {
            { fn(i, j) };
        }
#endif
    constexpr void for_each_set_bit(Function fn) const noexcept(is_noexcept_coords_fn<Function>()) {
        const_iterator iter     = begin();
        const_iterator iter_end = end();
        for (size_type i{}; iter != iter_end; ++iter, ++i) {
            for_each_row_set_bit(*iter, [&](size_type j) { fn(i, j); });
        }
    }
    friend std::ostream& operator<<(std::ostream& out ATTRIBUTE_LIFETIME_BOUND, const square_bitmatrix& matrix) {
        std::ostringstream str;
        for (const row_type& row : matrix) {
            str << row << '\n';
        }
        return out << std::move(str).str();
    }

private:
    static constexpr bool kUseSGIExtension =
#if CONFIG_HAS_AT_LEAST_CXX_20
#if CONFIG_HAS_INCLUDE(<concepts>)
        requires(const row_type row) {
            { row._Find_first() } -> std::convertible_to<size_type>;
            { row._Find_next(size_type()) } -> std::convertible_to<size_type>;
        };
#else
        requires(const row_type row, size_type i, size_type j) {
            i = row._Find_first();
            j = row._Find_next(size_type());
        };
#endif
#else
        false;
#endif

    template <class Function>
    ATTRIBUTE_ALWAYS_INLINE
    static constexpr void for_each_row_set_bit(const row_type& row, Function fn) noexcept(is_noexcept_index_fn<Function>()) {
        if constexpr (kUseSGIExtension) {
            for (size_type j = row._Find_first(); j < N; j = row._Find_next(j)) {
                fn(j);
            }
        } else {
            for (size_type j = 0; j < N; ++j) {
                if (row[j]) {
                    fn(j);
                }
            }
        }
    }

    CONSTEXPR_BITSET_OPS
    void do_bitwise_or(const_pointer other_begin) noexcept {
        std::for_each(begin(), end(),
                      [other_iter = other_begin](
                          row_type& row_reference) mutable CONSTEXPR_BITSET_OPS noexcept {
                          row_reference |= *other_iter;
                          ++other_iter;
                      });
    }
    CONSTEXPR_BITSET_OPS
    void do_bitwise_and(const_pointer other_begin) noexcept {
        std::for_each(begin(), end(),
                      [other_iter = other_begin](
                          row_type& row_reference) mutable CONSTEXPR_BITSET_OPS noexcept {
                          row_reference &= *other_iter;
                          ++other_iter;
                      });
    }
    CONSTEXPR_BITSET_OPS
    void do_bitwise_xor(const_pointer other_begin) noexcept {
        std::for_each(begin(), end(),
                      [other_iter = other_begin](
                          row_type& row_reference) mutable CONSTEXPR_BITSET_OPS noexcept {
                          row_reference ^= *other_iter;
                          ++other_iter;
                      });
    }
    CONSTEXPR_BITSET_OPS
    void do_multiply_over_z2(const_pointer other_begin) noexcept {
        std::for_each(
            begin(), end(),
            [other_begin = other_begin](row_type& row_reference) CONSTEXPR_BITSET_OPS noexcept {
                row_type row_mult{};
                for_each_row_set_bit(row_reference, [&](size_type j) CONSTEXPR_BITSET_OPS noexcept {
                    row_mult ^= other_begin[j];
                });
                row_reference = row_mult;
            });
    }
    ATTRIBUTE_NODISCARD_WITH_MESSAGE("impl error")
    ATTRIBUTE_PURE
    CONSTEXPR_BITSET_OPS
    row_type do_multiply_over_z2(const row_type& vector) const noexcept {
        row_type result_vector{};
        for (size_type i = 0; i < N; i++) {
            result_vector[i] = static_cast<bool>((data_[i] ^ vector).count() % 2);
        }
        return result_vector;
    }
    CONSTEXPR_BITSET_OPS
    void do_flip_inplace() noexcept {
        std::for_each(begin(), end(), [](row_type& row_reference) CONSTEXPR_BITSET_OPS noexcept {
            row_reference.flip();
        });
    }
    CONSTEXPR_BITSET_OPS
    void do_flip_row_inplace(size_type row_index) noexcept {
        data_[row_index].flip();
    }
    CONSTEXPR_BITSET_OPS
    void do_flip_column_inplace(size_type column_index) noexcept {
        if (unlikely(column_index >= columns())) {
            // std::bitset<>::flip() will throw if column_index >= std::bitset<>::size()
            return;
        }
        std::for_each(begin(), end(), [=](row_type& row_reference) CONSTEXPR_BITSET_OPS noexcept {
            row_reference.flip(column_index);
        });
    }
    static CONSTEXPR_POINTER_CAST void transpose_matrix(matrix_type& matrix) noexcept {
#if CONFIG_HAS_AT_LEAST_CXX_20
        word_type tmp1[kAlignmentBits]{};
        word_type tmp2[kAlignmentBits]{};

        // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)

        for (size_type i = 0; i < kBits / kAlignmentBits; ++i) {
            for (size_type j = i; j < kBits / kAlignmentBits; ++j) {
                for (size_type k = 0; k < kAlignmentBits; ++k) {
                    tmp1[k] = reinterpret_cast<const word_type*>(
                        std::addressof(matrix[i * kAlignmentBits + k]))[j];
                    tmp2[k] = reinterpret_cast<const word_type*>(
                        std::addressof(matrix[j * kAlignmentBits + k]))[i];
                }
                transpose_block_helper(tmp1);
                transpose_block_helper(tmp2);
                for (size_type k = 0; k < kAlignmentBits; ++k) {
                    reinterpret_cast<word_type*>(
                        std::addressof(matrix[i * kAlignmentBits + k]))[j] = tmp2[k];
                    reinterpret_cast<word_type*>(
                        std::addressof(matrix[j * kAlignmentBits + k]))[i] = tmp1[k];
                }
            }
        }

        // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
#else
        if constexpr (sizeof(std::bitset<8 + 1>) == sizeof(std::bitset<8 + 8>)) {
            transpose_8_fallback<>(matrix);
        } else {
            transpose_slow_fallback<>(matrix);
        }
#endif
    }
#if CONFIG_HAS_AT_LEAST_CXX_20
    template <class WordType, std::size_t Size>
    ATTRIBUTE_ALWAYS_INLINE
    static constexpr void transpose_block_helper(WordType (&m)[Size]) noexcept {
        if constexpr (bitmatrix_detail::Transposable64<WordType, Size>) {
            transpose64(m);
        } else if constexpr (bitmatrix_detail::Transposable32<WordType, Size>) {
            transpose32(m);
        } else if constexpr (bitmatrix_detail::Transposable8<WordType, Size>) {
            transpose8(m);
        } else {
            static_assert([]() constexpr { return false; }(),
                          "Invalid call to transpose_block_helper");
        }
    }
#else
    template <class = void>
    ATTRIBUTE_ALWAYS_INLINE
    static constexpr void transpose_8_fallback(matrix_type& matrix) noexcept {
        Matrix8x8 tmp1{};
        Matrix8x8 tmp2{};

        // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)

        for (size_type i = 0; i < kBits / 8; ++i) {
            for (size_type j = i; j < kBits / 8; ++j) {
                for (size_type k = 0; k < 8; ++k) {
                    tmp1[k] =
                        reinterpret_cast<const std::uint8_t*>(std::addressof(matrix[i * 8 + k]))[j];
                    tmp2[k] =
                        reinterpret_cast<const std::uint8_t*>(std::addressof(matrix[j * 8 + k]))[i];
                }
                transpose8(tmp1);
                transpose8(tmp2);
                for (size_type k = 0; k < 8; ++k) {
                    reinterpret_cast<std::uint8_t*>(std::addressof(matrix[i * 8 + k]))[j] = tmp2[k];
                    reinterpret_cast<std::uint8_t*>(std::addressof(matrix[j * 8 + k]))[i] = tmp1[k];
                }
            }
        }

        // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    }
    template <class = void>
    ATTRIBUTE_ALWAYS_INLINE
    static constexpr void transpose_slow_fallback(matrix_type& matrix) noexcept {
        for (size_type i = 0; i < matrix.size(); ++i) {
            for (size_type j = i + 1; j < matrix[i].size(); ++j) {
                std::swap(matrix[i][j], matrix[j][i]);
            }
        }
    }
#endif

    // clang-format on

    static constexpr square_bitmatrix create_identity_matrix_impl() noexcept {
        square_bitmatrix m{};
        for (size_type i = 0; i < N; ++i) {
            m.set_unchecked(i, i, true);
        }
        return m;
    }
};

#undef CONSTEXPR_POINTER_CAST
#undef CONSTEXPR_BITSET_OPS

// clang-format off
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
// clang-format on
