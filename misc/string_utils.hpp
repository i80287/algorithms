#pragma once

#include <string>
#include <string_view>

#include "config_macros.hpp"
#include "string_traits.hpp"

namespace misc {

template <class CharType>
[[nodiscard]] inline bool is_whitespace(const CharType c) noexcept;

template <class CharType>
[[nodiscard]] inline bool is_alpha(const CharType c) noexcept;

template <class CharType>
[[nodiscard]] inline bool is_alpha_digit(const CharType c) noexcept;

template <class CharType>
[[nodiscard]] inline bool is_digit(const CharType c) noexcept;

template <class CharType>
[[nodiscard]] inline bool is_hex_digit(const CharType c) noexcept;

struct trim_tag {};

struct whitespace_tag final : trim_tag {};

struct alpha_tag final : trim_tag {};

struct digit_tag final : trim_tag {};

struct alpha_digit_tag final : trim_tag {};

struct hex_digit_tag final : trim_tag {};

template <class StrType, class TrimStrType = whitespace_tag>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE inline auto trim(const StrType &str ATTRIBUTE_LIFETIME_BOUND,
                                         const TrimStrType &trim_chars = {}) noexcept;

template <class CharType>
[[nodiscard]] inline bool is_whitespace(const std::basic_string_view<CharType> str) noexcept;

template <class CharType>
[[nodiscard]] inline bool is_whitespace(const std::basic_string<CharType> &str) noexcept;

template <class CharType>
[[nodiscard]]
ATTRIBUTE_NONNULL_ALL_ARGS inline bool is_whitespace(const CharType *const str) noexcept;

template <class CharType>
ATTRIBUTE_SIZED_ACCESS(read_write, 1, 2)
inline void to_lower_inplace(CharType *const str, const size_t n) noexcept;

template <class CharType>
inline void to_lower_inplace(std::basic_string<CharType> &str) noexcept;

template <class CharType>
[[nodiscard]]
inline std::basic_string<CharType> to_lower(const std::basic_string_view<CharType> str);

template <class CharType>
[[nodiscard]] inline std::basic_string<CharType> to_lower(const std::basic_string<CharType> &str);

template <class CharType>
[[nodiscard]] inline std::basic_string<CharType> to_lower(const CharType *const str);

template <class CharType>
ATTRIBUTE_SIZED_ACCESS(read_write, 1, 2)
inline void to_upper_inplace(CharType *const str, const size_t n) noexcept;

template <class CharType>
inline void to_upper_inplace(std::basic_string<CharType> &str) noexcept;

template <class CharType>
[[nodiscard]]
inline std::basic_string<CharType> to_upper(const std::basic_string_view<CharType> str);

template <class CharType>
[[nodiscard]] inline std::basic_string<CharType> to_upper(const std::basic_string<CharType> &str);

template <class CharType>
[[nodiscard]]
ATTRIBUTE_NONNULL_ALL_ARGS inline std::basic_string<CharType> to_upper(const CharType *const str);

}  // namespace misc

#define STRING_UTILS_INCLUDING_IMPLEMENTATION
#include "string_utils.ipp"
#undef STRING_UTILS_INCLUDING_IMPLEMENTATION
