#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>

#include "../misc/config_macros.hpp"

#if CONFIG_HAS_AT_LEAST_CXX_20
#define CONSTEXPR_CXX_20 constexpr
#else
#define CONSTEXPR_CXX_20 inline
#endif

namespace str_tools {

namespace detail {
using std::size_t;
using std::uint8_t;

ATTRIBUTE_NODISCARD
ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
ATTRIBUTE_SIZED_ACCESS(read_only, 3, 4)
ATTRIBUTE_PURE
CONSTEXPR_CXX_20 bool is_permutation_of_impl(const char* lhs_ptr,
                                             const size_t lhs_size,
                                             const char* rhs_ptr,
                                             const size_t rhs_size) noexcept {
    using MapType = std::array<size_t, size_t{std::numeric_limits<uint8_t>::max()} + 1>;

    MapType lhs_cnt_map{};
    for (size_t i = 0; i < lhs_size; i++) {
        lhs_cnt_map[static_cast<uint8_t>(lhs_ptr[i])]++;
    }
    MapType rhs_cnt_map{};
    for (size_t i = 0; i < rhs_size; i++) {
        rhs_cnt_map[static_cast<uint8_t>(rhs_ptr[i])]++;
    }

    return lhs_cnt_map == rhs_cnt_map;
}

ATTRIBUTE_NODISCARD
ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
ATTRIBUTE_PURE constexpr size_t unique_chars_count_impl(const char* str_ptr,
                                                        const size_t str_size) noexcept {
    constexpr size_t kMapSize = size_t{std::numeric_limits<uint8_t>::max()} + 1;
    using MapType             = std::array<bool, kMapSize>;

    MapType has_char{};
    for (size_t i = 0; i < str_size; i++) {
        has_char[static_cast<uint8_t>(str_ptr[i])] = true;
    }

    size_t count = 0;
    for (const bool has_i_pos : has_char) {
        count += size_t{has_i_pos};
    }

    CONFIG_ASSUME_STATEMENT(count <= str_size);
    CONFIG_ASSUME_STATEMENT(count <= kMapSize);

    return count;
}

ATTRIBUTE_NODISCARD
ATTRIBUTE_SIZED_ACCESS(read_only, 1, 2)
inline std::string sorted_unique_chars_of_impl(const char* str_ptr, const size_t str_size) {
    constexpr size_t kMapSize = size_t{std::numeric_limits<uint8_t>::max()} + 1;
    using MapType             = std::array<bool, kMapSize>;

    MapType has_char{};
    for (size_t i = 0; i < str_size; i++) {
        has_char[static_cast<uint8_t>(str_ptr[i])] = true;
    }

    size_t count = 0;
    for (const bool has_i_pos : has_char) {
        count += size_t{has_i_pos};
    }

    CONFIG_ASSUME_STATEMENT(count <= str_size);
    CONFIG_ASSUME_STATEMENT(count <= kMapSize);

    std::string str(count, '\0');
    for (size_t chr = 0, i = 0; chr < has_char.size(); chr++) {
        if (has_char[chr]) {
            str[i++] = static_cast<char>(static_cast<uint8_t>(chr));
        }
    }

    return str;
}

}  // namespace detail

ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_PURE
[[nodiscard]]
CONSTEXPR_CXX_20 bool is_permutation_of(const std::string_view lhs,
                                        const std::string_view rhs) noexcept {
    return str_tools::detail::is_permutation_of_impl(lhs.data(), lhs.size(), rhs.data(),
                                                     rhs.size());
}

ATTRIBUTE_ALWAYS_INLINE
ATTRIBUTE_PURE
[[nodiscard]] constexpr std::size_t unique_chars_count(const std::string_view str) noexcept {
    return str_tools::detail::unique_chars_count_impl(str.data(), str.size());
}

ATTRIBUTE_ALWAYS_INLINE
[[nodiscard]] inline std::string sorted_unique_chars_of(const std::string_view str) {
    return str_tools::detail::sorted_unique_chars_of_impl(str.data(), str.size());
}

}  // namespace str_tools

#undef CONSTEXPR_CXX_20
