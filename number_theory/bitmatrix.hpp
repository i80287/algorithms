#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <climits>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <initializer_list>
#include <memory>
#include <sstream>
#include <type_traits>
#include <utility>

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
constexpr void transpose8(const uint8_t (&src)[8], uint8_t (&dst)[8]) noexcept {
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
constexpr void transpose8(uint8_t (&src)[8]) noexcept {
    transpose8<AgainstMinorDiagonal>(src, src);
}

/// @brief Transposes 32x32 matrix @a src inplace
/// @details See Hackers Delight for more info
/// @tparam AgainstMinorDiagonal
/// @param src source 32x32 matrix
/// @note see @c transpose8(const uint8_t[8], uint8_t[8]) for the explanation of @a AgainstMinorDiagonal
template <bool AgainstMinorDiagonal = false>
ATTRIBUTE_ACCESS(read_write, 1)
constexpr void transpose32(uint32_t (&src)[32]) noexcept {
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
constexpr void transpose32(const uint32_t (&src)[32], uint32_t (&dst)[32]) noexcept {
    if (std::addressof(dst) < std::addressof(src)) {
        std::copy(&src[0], &src[32], &dst[0]);
    } else if (std::addressof(dst) > std::addressof(src)) {
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
constexpr void transpose64(uint64_t (&src)[64]) noexcept {
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

namespace detail {
struct square_bitmatrix_helper {
private:
    static constexpr bool kUseUInt64 =
        false && sizeof(std::bitset<64 + 1>) == sizeof(std::bitset<64 + 64>);
    static constexpr bool kUseUInt32 =
        false && !kUseUInt64 && sizeof(std::bitset<32 + 1>) == sizeof(std::bitset<32 + 32>);
    static constexpr bool kUseUInt8 = !kUseUInt64 && !kUseUInt32;
    static_assert(!kUseUInt8 || sizeof(std::bitset<8 + 1>) == sizeof(std::bitset<8 + 8>));

public:
    using word_type =
        std::conditional_t<kUseUInt64, std::uint64_t,
                           std::conditional_t<kUseUInt32, std::uint32_t, std::uint8_t> >;
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

}  // namespace detail

template <std::size_t N, class word_type = typename detail::square_bitmatrix_helper::word_type>
#if CONFIG_HAS_AT_LEAST_CXX_20
    requires(N > 0)
#endif
struct alignas(std::uint64_t) alignas(word_type) square_bitmatrix {
private:
    static_assert(N > 0);
    static_assert(CHAR_BIT == 8, "Platform not supported");
    static constexpr std::size_t kAlignmentBits = sizeof(word_type) * CHAR_BIT;
    static constexpr std::size_t kBits          = (N + kAlignmentBits - 1) & ~(kAlignmentBits - 1);
    static_assert(sizeof(std::bitset<N>) == sizeof(std::bitset<kBits>));

    template <class TFunction>
    static constexpr bool is_noexcept_index_fn() noexcept {
        return noexcept(std::declval<TFunction>()(size_type{}));
    }
    template <class TFunction>
    static constexpr bool is_noexcept_coords_fn() noexcept {
        return noexcept(std::declval<TFunction>()(size_type{}, size_type{}));
    }
    template <class TFunction>
    static constexpr bool is_noexcept_bit_reference_fn() noexcept {
        return noexcept(std::declval<TFunction>()(std::declval<bit_reference>()));
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
    // initializers like {0b00, 0b10, ...}
    matrix_type data_{};

    [[nodiscard]] ATTRIBUTE_CONST static CONSTEXPR_BITSET_OPS square_bitmatrix identity() noexcept {
        if constexpr (N <= kAlignmentBits) {
            if constexpr (kAlignmentBits == 64) {
                return {
                    0b0000000000000000000000000000000000000000000000000000000000000001ull,
                    0b0000000000000000000000000000000000000000000000000000000000000010ull,
                    0b0000000000000000000000000000000000000000000000000000000000000100ull,
                    0b0000000000000000000000000000000000000000000000000000000000001000ull,
                    0b0000000000000000000000000000000000000000000000000000000000010000ull,
                    0b0000000000000000000000000000000000000000000000000000000000100000ull,
                    0b0000000000000000000000000000000000000000000000000000000001000000ull,
                    0b0000000000000000000000000000000000000000000000000000000010000000ull,
                    0b0000000000000000000000000000000000000000000000000000000100000000ull,
                    0b0000000000000000000000000000000000000000000000000000001000000000ull,
                    0b0000000000000000000000000000000000000000000000000000010000000000ull,
                    0b0000000000000000000000000000000000000000000000000000100000000000ull,
                    0b0000000000000000000000000000000000000000000000000001000000000000ull,
                    0b0000000000000000000000000000000000000000000000000010000000000000ull,
                    0b0000000000000000000000000000000000000000000000000100000000000000ull,
                    0b0000000000000000000000000000000000000000000000001000000000000000ull,
                    0b0000000000000000000000000000000000000000000000010000000000000000ull,
                    0b0000000000000000000000000000000000000000000000100000000000000000ull,
                    0b0000000000000000000000000000000000000000000001000000000000000000ull,
                    0b0000000000000000000000000000000000000000000010000000000000000000ull,
                    0b0000000000000000000000000000000000000000000100000000000000000000ull,
                    0b0000000000000000000000000000000000000000001000000000000000000000ull,
                    0b0000000000000000000000000000000000000000010000000000000000000000ull,
                    0b0000000000000000000000000000000000000000100000000000000000000000ull,
                    0b0000000000000000000000000000000000000001000000000000000000000000ull,
                    0b0000000000000000000000000000000000000010000000000000000000000000ull,
                    0b0000000000000000000000000000000000000100000000000000000000000000ull,
                    0b0000000000000000000000000000000000001000000000000000000000000000ull,
                    0b0000000000000000000000000000000000010000000000000000000000000000ull,
                    0b0000000000000000000000000000000000100000000000000000000000000000ull,
                    0b0000000000000000000000000000000001000000000000000000000000000000ull,
                    0b0000000000000000000000000000000010000000000000000000000000000000ull,
                    0b0000000000000000000000000000000100000000000000000000000000000000ull,
                    0b0000000000000000000000000000001000000000000000000000000000000000ull,
                    0b0000000000000000000000000000010000000000000000000000000000000000ull,
                    0b0000000000000000000000000000100000000000000000000000000000000000ull,
                    0b0000000000000000000000000001000000000000000000000000000000000000ull,
                    0b0000000000000000000000000010000000000000000000000000000000000000ull,
                    0b0000000000000000000000000100000000000000000000000000000000000000ull,
                    0b0000000000000000000000001000000000000000000000000000000000000000ull,
                    0b0000000000000000000000010000000000000000000000000000000000000000ull,
                    0b0000000000000000000000100000000000000000000000000000000000000000ull,
                    0b0000000000000000000001000000000000000000000000000000000000000000ull,
                    0b0000000000000000000010000000000000000000000000000000000000000000ull,
                    0b0000000000000000000100000000000000000000000000000000000000000000ull,
                    0b0000000000000000001000000000000000000000000000000000000000000000ull,
                    0b0000000000000000010000000000000000000000000000000000000000000000ull,
                    0b0000000000000000100000000000000000000000000000000000000000000000ull,
                    0b0000000000000001000000000000000000000000000000000000000000000000ull,
                    0b0000000000000010000000000000000000000000000000000000000000000000ull,
                    0b0000000000000100000000000000000000000000000000000000000000000000ull,
                    0b0000000000001000000000000000000000000000000000000000000000000000ull,
                    0b0000000000010000000000000000000000000000000000000000000000000000ull,
                    0b0000000000100000000000000000000000000000000000000000000000000000ull,
                    0b0000000001000000000000000000000000000000000000000000000000000000ull,
                    0b0000000010000000000000000000000000000000000000000000000000000000ull,
                    0b0000000100000000000000000000000000000000000000000000000000000000ull,
                    0b0000001000000000000000000000000000000000000000000000000000000000ull,
                    0b0000010000000000000000000000000000000000000000000000000000000000ull,
                    0b0000100000000000000000000000000000000000000000000000000000000000ull,
                    0b0001000000000000000000000000000000000000000000000000000000000000ull,
                    0b0010000000000000000000000000000000000000000000000000000000000000ull,
                    0b0100000000000000000000000000000000000000000000000000000000000000ull,
                    0b1000000000000000000000000000000000000000000000000000000000000000ull,
                };
            } else if constexpr (kAlignmentBits == 32) {
                return {
                    0b00000000000000000000000000000001u, 0b00000000000000000000000000000010u,
                    0b00000000000000000000000000000100u, 0b00000000000000000000000000001000u,
                    0b00000000000000000000000000010000u, 0b00000000000000000000000000100000u,
                    0b00000000000000000000000001000000u, 0b00000000000000000000000010000000u,
                    0b00000000000000000000000100000000u, 0b00000000000000000000001000000000u,
                    0b00000000000000000000010000000000u, 0b00000000000000000000100000000000u,
                    0b00000000000000000001000000000000u, 0b00000000000000000010000000000000u,
                    0b00000000000000000100000000000000u, 0b00000000000000001000000000000000u,
                    0b00000000000000010000000000000000u, 0b00000000000000100000000000000000u,
                    0b00000000000001000000000000000000u, 0b00000000000010000000000000000000u,
                    0b00000000000100000000000000000000u, 0b00000000001000000000000000000000u,
                    0b00000000010000000000000000000000u, 0b00000000100000000000000000000000u,
                    0b00000001000000000000000000000000u, 0b00000010000000000000000000000000u,
                    0b00000100000000000000000000000000u, 0b00001000000000000000000000000000u,
                    0b00010000000000000000000000000000u, 0b00100000000000000000000000000000u,
                    0b01000000000000000000000000000000u, 0b10000000000000000000000000000000u,
                };
            } else if constexpr (kAlignmentBits == 8) {
                return {
                    0b00000001u, 0b00000010u, 0b00000100u, 0b00001000u,
                    0b00010000u, 0b00100000u, 0b01000000u, 0b10000000u,
                };
            }
        }

        square_bitmatrix m{};
        for (size_type i = 0; i < N; ++i) {
            m[i][i] = true;
        }
        return m;
    }
    [[nodiscard]] ATTRIBUTE_CONST static constexpr square_bitmatrix allzeros() noexcept {
        return {};
    }
    [[nodiscard]] ATTRIBUTE_CONST static CONSTEXPR_BITSET_OPS square_bitmatrix allones() noexcept {
        square_bitmatrix m{};
        for (row_type& row : m) {
            row.set();
        }
        return m;
    }

    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE constexpr iterator begin() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return data_.begin();
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE constexpr iterator end() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return begin() + N;
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE constexpr const_iterator begin() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return data_.begin();
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE constexpr const_iterator end() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return begin() + N;
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE constexpr const_iterator cbegin() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return data_.cbegin();
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE constexpr const_iterator cend() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return cbegin() + N;
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE constexpr pointer data() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return data_.data();
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE constexpr const_pointer data() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return data_.data();
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE constexpr reference operator[](size_type index) noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return data_[index];
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE constexpr const_reference operator[](
        size_type index) const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return data_[index];
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE CONSTEXPR_BITSET_OPS bit_reference
    operator[](std::pair<size_type, size_type> indexes) noexcept ATTRIBUTE_LIFETIME_BOUND {
        return data_[indexes.first][indexes.second];
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE constexpr bool operator[](
        std::pair<size_type, size_type> indexes) const noexcept {
        return data_[indexes.first][indexes.second];
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE constexpr bool get(std::size_t i,
                                                             std::size_t j) const noexcept {
        return data_[i][j];
    }
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE constexpr bool get(
        std::pair<size_type, size_type> indexes) const noexcept {
        return get(indexes.first, indexes.second);
    }
    ATTRIBUTE_ALWAYS_INLINE CONSTEXPR_BITSET_OPS void set(std::size_t i, std::size_t j,
                                                          bool value = true) noexcept {
        data_[i][j] = value;
    }
    ATTRIBUTE_ALWAYS_INLINE CONSTEXPR_BITSET_OPS void set(std::pair<size_type, size_type> indexes,
                                                          bool value = true) noexcept {
        set(indexes.first, indexes.second, value);
    }

    CONSTEXPR_BITSET_OPS square_bitmatrix& operator|=(const square_bitmatrix& other) noexcept {
        do_bitwise_or(other.data());
        return *this;
    }
    [[nodiscard]] ATTRIBUTE_PURE CONSTEXPR_BITSET_OPS square_bitmatrix
    operator|(const square_bitmatrix& other) const noexcept {
        square_bitmatrix copy(*this);
        copy |= other;
        return copy;
    }
    CONSTEXPR_BITSET_OPS square_bitmatrix& operator&=(const square_bitmatrix& other) noexcept {
        do_bitwise_and(other.data());
        return *this;
    }
    [[nodiscard]] ATTRIBUTE_PURE CONSTEXPR_BITSET_OPS square_bitmatrix
    operator&(const square_bitmatrix& other) const noexcept {
        square_bitmatrix copy(*this);
        copy &= other;
        return copy;
    }
    CONSTEXPR_BITSET_OPS square_bitmatrix& operator^=(const square_bitmatrix& other) noexcept {
        do_bitwise_xor(other.data());
        return *this;
    }
    [[nodiscard]] ATTRIBUTE_PURE CONSTEXPR_BITSET_OPS square_bitmatrix
    operator^(const square_bitmatrix& other) const noexcept {
        square_bitmatrix copy(*this);
        copy ^= other;
        return copy;
    }
    CONSTEXPR_BITSET_OPS square_bitmatrix& operator*=(const square_bitmatrix& other) noexcept {
        do_multiply_over_z2(other.data());
        return *this;
    }
    [[nodiscard]] ATTRIBUTE_PURE CONSTEXPR_BITSET_OPS square_bitmatrix
    operator*(const square_bitmatrix& other) const noexcept {
        square_bitmatrix copy(*this);
        copy *= other;
        return copy;
    }
    [[nodiscard]] ATTRIBUTE_PURE CONSTEXPR_BITSET_OPS square_bitmatrix
    operator*(const row_type& vector) const noexcept {
        return do_multiply_over_z2(vector);
    }
#if CONFIG_HAS_AT_LEAST_CXX_20
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
#endif
    [[nodiscard]] ATTRIBUTE_PURE CONSTEXPR_BITSET_OPS size_type count() const noexcept {
        return std::accumulate(
            begin(), end(), size_type(0),
            [](size_type set_bits_count, const row_type& row)
                CONSTEXPR_BITSET_OPS noexcept { return set_bits_count + row.count(); });
    }
    [[nodiscard]] ATTRIBUTE_PURE CONSTEXPR_BITSET_OPS bool none() const noexcept {
        return !any();
    }
    [[nodiscard]] ATTRIBUTE_PURE CONSTEXPR_BITSET_OPS bool any() const noexcept {
        return std::any_of(begin(), end(), [](const row_type& row) CONSTEXPR_BITSET_OPS noexcept {
            return row.any();
        });
    }
    [[nodiscard]] ATTRIBUTE_PURE CONSTEXPR_BITSET_OPS bool all() const noexcept {
        return std::all_of(begin(), end(), [](const row_type& row) CONSTEXPR_BITSET_OPS noexcept {
            return row.all();
        });
    }
    CONSTEXPR_BITSET_OPS void reset() noexcept {
        if (config_is_constant_evaluated()) {
            for (row_type& row : data_) {
                row.reset();
            }
        } else {
            std::memset(reinterpret_cast<void*>(std::addressof(data_)), 0, sizeof(data_));
        }
    }
    [[nodiscard]] ATTRIBUTE_PURE CONSTEXPR_BITSET_OPS bool operator==(
        const square_bitmatrix& other) const noexcept {
        return std::equal(begin(), end(), other.begin());
    }
#if !defined(__cpp_impl_three_way_comparison) || __cpp_impl_three_way_comparison < 201907L
    [[nodiscard]] ATTRIBUTE_PURE CONSTEXPR_BITSET_OPS bool operator!=(
        const square_bitmatrix& other) const noexcept {
        return !(*this == other);
    }
#endif
    template <class TFunction>
#if CONFIG_HAS_AT_LEAST_CXX_20
        requires requires(TFunction fn, std::size_t i, std::size_t j) {
            { fn(i, j) };
        }
#endif
    constexpr void for_each_set_bit(TFunction fn) const
        noexcept(is_noexcept_coords_fn<TFunction>()) {
        auto&& iter     = begin();
        auto&& iter_end = end();
        for (size_type i{}; iter != iter_end; ++iter, ++i) {
            for_each_row_set_bit(*iter, [&](size_type j) { fn(i, j); });
        }
    }
    friend std::ostream& operator<<(std::ostream& out, const square_bitmatrix& matrix) {
        std::ostringstream str;
        for (const row_type& row : matrix) {
            str << row << '\n';
        }
        return out << std::move(str).str();
    }

private:
    static constexpr bool kUseSGIExtension =
#if CONFIG_HAS_AT_LEAST_CXX_20
        requires(const row_type row) {
            { row._Find_first() } -> std::convertible_to<size_type>;
            { row._Find_next(size_type()) } -> std::convertible_to<size_type>;
        };
#else
        false;
#endif
    template <class TFunction>
    ATTRIBUTE_ALWAYS_INLINE static constexpr void for_each_row_set_bit(
        const row_type& row, TFunction fn) noexcept(is_noexcept_index_fn<TFunction>()) {
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

    CONSTEXPR_BITSET_OPS void do_bitwise_or(const_pointer other_begin) noexcept {
        std::for_each(begin(), end(),
                      [other_iter = other_begin](
                          row_type& row_reference) mutable CONSTEXPR_BITSET_OPS noexcept {
                          row_reference |= *other_iter;
                          ++other_iter;
                      });
    }
    CONSTEXPR_BITSET_OPS void do_bitwise_and(const_pointer other_begin) noexcept {
        std::for_each(begin(), end(),
                      [other_iter = other_begin](
                          row_type& row_reference) mutable CONSTEXPR_BITSET_OPS noexcept {
                          row_reference &= *other_iter;
                          ++other_iter;
                      });
    }
    CONSTEXPR_BITSET_OPS void do_bitwise_xor(const_pointer other_begin) noexcept {
        std::for_each(begin(), end(),
                      [other_iter = other_begin](
                          row_type& row_reference) mutable CONSTEXPR_BITSET_OPS noexcept {
                          row_reference ^= *other_iter;
                          ++other_iter;
                      });
    }
    CONSTEXPR_BITSET_OPS void do_multiply_over_z2(const_pointer other_begin) noexcept {
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
    ATTRIBUTE_PURE CONSTEXPR_BITSET_OPS row_type
    do_multiply_over_z2(const row_type& vector) const noexcept {
        row_type result_vector{};
        for (size_type i = 0; i < N; i++) {
            result_vector[i] = static_cast<bool>((data_[i] ^ vector).count() % 2);
        }
        return result_vector;
    }
#if CONFIG_HAS_AT_LEAST_CXX_20
    CONSTEXPR_POINTER_CAST void do_transpose_inplace(matrix_type& matrix) noexcept {
        word_type tmp1[kAlignmentBits]{};
        word_type tmp2[kAlignmentBits]{};

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
    }
    template <class TWordType, std::size_t Size>
    static constexpr void transpose_block_helper(TWordType (&m)[Size]) noexcept {
        if constexpr (detail::Transposable64<TWordType, Size>) {
            transpose64(m);
        } else if constexpr (detail::Transposable32<TWordType, Size>) {
            transpose32(m);
        } else if constexpr (detail::Transposable8<TWordType, Size>) {
            transpose8(m);
        } else {
            static_assert([]() constexpr { return false; },
                          "Invalid call to transpose_block_helper");
        }
    }
#endif
};

template <std::size_t N>
using packed_square_bitmatrix = square_bitmatrix<N, std::uint8_t>;

#undef CONSTEXPR_POINTER_CAST
#undef CONSTEXPR_BITSET_OPS
