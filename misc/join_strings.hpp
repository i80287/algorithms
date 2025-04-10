#pragma once

#include <string>
#include <string_view>
#include <type_traits>

#include "config_macros.hpp"

#if CONFIG_HAS_AT_LEAST_CXX_20 && CONFIG_HAS_CONCEPTS && CONFIG_HAS_INCLUDE(<concepts>)

#define JOIN_STRINGS_SUPPORTS_CUSTOM_TO_STRING

#define JOIN_STRINGS_SUPPORTS_CUSTOM_OSTRINGSTREAM

#endif

#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_lib_ranges) && __cpp_lib_ranges >= 201911L && \
    CONFIG_HAS_INCLUDE(<ranges>)

#include <ranges>
#define JOIN_STRINGS_SUPPORTS_JOIN_STRINGS_COLLECTION

#endif

#if CONFIG_HAS_AT_LEAST_CXX_17 && defined(__cpp_lib_filesystem) && \
    __cpp_lib_filesystem >= 201703L && CONFIG_HAS_INCLUDE(<filesystem>)

#define JOIN_STRINGS_SUPPORTS_FILESYSTEM_PATH

#endif

namespace misc {

template <class T>
inline constexpr bool is_char_v = std::is_same_v<T, char> || std::is_same_v<T, wchar_t> ||
#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
                                  std::is_same_v<T, char8_t> ||
#endif
                                  std::is_same_v<T, char16_t> || std::is_same_v<T, char32_t>;

#ifdef JOIN_STRINGS_SUPPORTS_JOIN_STRINGS_COLLECTION

template <class T>
concept Char = is_char_v<T>;

template <class T>
struct is_basic_string : std::false_type {};

template <class CharType>
struct is_basic_string<std::basic_string<CharType>> : std::true_type {};

template <class T>
inline constexpr bool is_basic_string_v = is_basic_string<T>::value;

template <class T>
struct is_basic_string_view : std::false_type {};

template <class CharType>
struct is_basic_string_view<std::basic_string_view<CharType>> : std::true_type {};

template <class T>
inline constexpr bool is_basic_string_view_v = is_basic_string_view<T>::value;

template <class T>
struct is_c_str : std::false_type {};

template <Char T>
struct is_c_str<const T *> : std::true_type {};

template <Char T>
struct is_c_str<T *> : std::true_type {};

template <Char T>
struct is_c_str<const T[]> : std::true_type {};

template <Char T>
struct is_c_str<T[]> : std::true_type {};

template <class T>
inline constexpr bool is_c_str_v = is_c_str<T>::value;

template <class T>
struct is_c_str_arr : std::false_type {};

template <Char T, size_t N>
struct is_c_str_arr<const T[N]> : std::true_type {};

template <Char T, size_t N>
struct is_c_str_arr<T[N]> : std::true_type {};

template <class T>
inline constexpr bool is_c_str_arr_v = is_c_str_arr<T>::value;

template <class T>
concept CharOrStringLike =
    misc::Char<T> || misc::is_basic_string_v<T> || misc::is_basic_string_view_v<T> ||
    misc::is_c_str_v<T> || misc::is_c_str_arr_v<T>;

#endif

/// @brief Join arguments @a args (converting to string if necessary)
/// @tparam HintCharType hint char type (default: `char`).
///         Can be passed if, for instance, join_strings(1, 2, 3.0)
///         should be std::wstring: join_strings<wchar_t>(1, 2, 3.0)
/// @tparam Args Types of the arguments to join, e.g. char types, pointers to them,
///         basic_string, basic_string_view, integral/floating point values
/// @param args arguments to join
/// @return joined args as a string of type std::basic_string<CharType>
///         where CharType is deducted by the @a Args... or HintCharType
///         if @a Args is pack of numeric types
template <class HintCharType = char, class... Args>
[[nodiscard]] ATTRIBUTE_ALWAYS_INLINE inline auto join_strings(const Args &...args);

#ifdef JOIN_STRINGS_SUPPORTS_JOIN_STRINGS_COLLECTION

template <misc::CharOrStringLike Sep, std::ranges::forward_range Container>
[[nodiscard]] auto join_strings_collection(const Sep &sep, const Container &strings);

template <std::ranges::forward_range Container>
[[nodiscard]] auto join_strings_collection(const Container &strings);

#endif

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

#define JOIN_STRINGS_INCLUDING_IMPLEMENTATION
#include "join_strings.ipp"
#undef JOIN_STRINGS_INCLUDING_IMPLEMENTATION
