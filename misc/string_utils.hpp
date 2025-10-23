#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>

#include "config_macros.hpp"
#include "string_traits.hpp"

namespace misc {
using std::size_t;

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

template <class CharType>
[[nodiscard]] inline bool is_upper(const CharType c) noexcept;

template <class CharType>
[[nodiscard]] inline bool is_lower(const CharType c) noexcept;

template <class CharType>
[[nodiscard]] inline CharType to_upper(const CharType c) noexcept;

template <class CharType>
[[nodiscard]] inline CharType to_lower(const CharType c) noexcept;

template <class CharType>
[[nodiscard]]
inline bool is_whitespace(std::basic_string_view<CharType> str) noexcept;

template <class CharType>
[[nodiscard]]
inline bool is_whitespace(const std::basic_string<CharType> &str) noexcept;

template <class CharType>
ATTRIBUTE_NONNULL_ALL_ARGS ATTRIBUTE_ACCESS(read_only, 1) [[nodiscard]]
inline bool is_whitespace(const CharType *str) noexcept;

template <class CharType>
ATTRIBUTE_SIZED_ACCESS(read_write, 1, 2)
inline void to_lower_inplace(CharType *str, size_t n) noexcept;

template <class CharType>
inline void to_lower_inplace(std::basic_string<CharType> &str) noexcept;

template <class CharType>
[[nodiscard]]
inline std::basic_string<CharType> to_lower(std::basic_string_view<CharType> str);

template <class CharType>
[[nodiscard]] inline std::basic_string<CharType> to_lower(const std::basic_string<CharType> &str);

template <class CharType>
ATTRIBUTE_NONNULL_ALL_ARGS ATTRIBUTE_ACCESS(read_only, 1)
    [[nodiscard]] inline std::basic_string<CharType> to_lower(const CharType *str);

template <class CharType>
ATTRIBUTE_SIZED_ACCESS(read_write, 1, 2)
inline void to_upper_inplace(CharType *str, size_t n) noexcept;

template <class CharType>
inline void to_upper_inplace(std::basic_string<CharType> &str) noexcept;

template <class CharType>
[[nodiscard]]
inline std::basic_string<CharType> to_upper(std::basic_string_view<CharType> str);

template <class CharType>
[[nodiscard]] inline std::basic_string<CharType> to_upper(const std::basic_string<CharType> &str);

template <class CharType>
ATTRIBUTE_NONNULL_ALL_ARGS ATTRIBUTE_ACCESS(read_only, 1) [[nodiscard]]
inline std::basic_string<CharType> to_upper(const CharType *str);

namespace locale_indep {

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_whitespace(char c) noexcept;

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_whitespace(char16_t c) noexcept;

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_whitespace(char32_t c) noexcept;

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_whitespace(char c) noexcept;

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_alpha(char c) noexcept;

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_alpha_digit(char c) noexcept;

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_digit(char c) noexcept;

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_hex_digit(char c) noexcept;

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_upper(char c) noexcept;

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_lower(char c) noexcept;

[[nodiscard]] ATTRIBUTE_CONST constexpr char to_upper(char c) noexcept;

[[nodiscard]] ATTRIBUTE_CONST constexpr char to_lower(char c) noexcept;

[[nodiscard]] constexpr bool is_whitespace(std::string_view str) noexcept;

[[nodiscard]] inline bool is_whitespace(const std::string &str) noexcept;

ATTRIBUTE_NONNULL_ALL_ARGS ATTRIBUTE_ACCESS(read_only, 1) [[nodiscard]]
constexpr bool is_whitespace(const char *str) noexcept;

ATTRIBUTE_SIZED_ACCESS(read_write, 1, 2)
inline void to_lower_inplace(char *str, size_t n) noexcept;

inline void to_lower_inplace(std::string &str) noexcept;

[[nodiscard]] inline std::string to_lower(std::string_view str);

[[nodiscard]] inline std::string to_lower(const std::string &str);

ATTRIBUTE_NONNULL_ALL_ARGS ATTRIBUTE_ACCESS(read_only, 1) [[nodiscard]]
inline std::string to_lower(const char *str);

ATTRIBUTE_SIZED_ACCESS(read_write, 1, 2)
inline void to_upper_inplace(char *str, size_t n) noexcept;

inline void to_upper_inplace(std::string &str) noexcept;

[[nodiscard]] inline std::string to_upper(std::string_view str);

[[nodiscard]] inline std::string to_upper(const std::string &str);

ATTRIBUTE_NONNULL_ALL_ARGS ATTRIBUTE_ACCESS(read_only, 1) [[nodiscard]]
inline std::string to_upper(const char *str);

}  // namespace locale_indep

struct trim_tag {};

struct whitespace_tag final : trim_tag {};

struct alpha_tag final : trim_tag {};

struct digit_tag final : trim_tag {};

struct alpha_digit_tag final : trim_tag {};

struct hex_digit_tag final : trim_tag {};

namespace locale_indep {

struct trim_tag : misc::trim_tag {};

struct whitespace_tag final : trim_tag {};

struct alpha_tag final : trim_tag {};

struct digit_tag final : trim_tag {};

struct alpha_digit_tag final : trim_tag {};

struct hex_digit_tag final : trim_tag {};

}  // namespace locale_indep

template <class StrType, class TrimStrType = whitespace_tag>
ATTRIBUTE_ALWAYS_INLINE [[nodiscard]]
inline auto trim(
    const StrType &str ATTRIBUTE_LIFETIME_BOUND,
    const TrimStrType &trim_chars = {}) noexcept(std::is_base_of_v<trim_tag, TrimStrType>);

}  // namespace misc

#define STRING_UTILS_INCLUDING_IMPLEMENTATION
#include "string_utils.ipp"
#undef STRING_UTILS_INCLUDING_IMPLEMENTATION
