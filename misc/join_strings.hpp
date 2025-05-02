#pragma once

#include "config_macros.hpp"

#if CONFIG_HAS_AT_LEAST_CXX_17 && defined(__cpp_lib_filesystem) && \
    __cpp_lib_filesystem >= 201703L && CONFIG_HAS_INCLUDE(<filesystem>)

#define JOIN_STRINGS_SUPPORTS_FILESYSTEM_PATH

#endif

#if CONFIG_HAS_CONCEPTS

#define JOIN_STRINGS_SUPPORTS_CUSTOM_TO_STRING
#define JOIN_STRINGS_SUPPORTS_CUSTOM_OSTRINGSTREAM

#endif

#if CONFIG_HAS_CONCEPTS && defined(__cpp_lib_ranges) && __cpp_lib_ranges >= 201911L && \
    CONFIG_HAS_INCLUDE(<ranges>)

#define JOIN_STRINGS_SUPPORTS_JOIN_STRINGS_COLLECTION

#include <ranges>

#include "string_traits.hpp"

#endif

namespace misc {

/// @brief Join arguments @a args (converting to string if necessary)
/// @tparam HintCharType hint char type (default: `char`).
///         Can be passed if, for instance, join_strings(1, 2, 3.0)
///         should be std::wstring: join_strings<wchar_t>(1, 2, 3.0)
/// @tparam Args Types of the arguments to join, e.g. char types, basic_string,
///         basic_string_view, char pointer and C-style array (treated like strings),
///         integral/floating point values, enums, std::filesystem::path, types with
///         method to_string (which returns std::string) and types for which ADL can
///         find to_string(const T&), to_wstring(const T&), to_basic_string<CharType>(const T&)
///         or operator<<(std::basic_ostream<CharType>&, const T&)
/// @param args arguments to join
/// @return joined args as a string of type std::basic_string<CharType>
///         where CharType is deducted from the @a Args... or HintCharType
///         if @a Args... is pack of types without associated char type
template <class HintCharType = char, class... Args>
[[nodiscard]] ATTRIBUTE_ALWAYS_INLINE inline auto join_strings(const Args &...args);

#ifdef JOIN_STRINGS_SUPPORTS_JOIN_STRINGS_COLLECTION

template <misc::CharOrStringLike Sep, std::ranges::forward_range Container>
[[nodiscard]] auto join_strings_collection(const Sep &sep, const Container &strings);

template <std::ranges::forward_range Container>
[[nodiscard]] auto join_strings_collection(const Container &strings);

#endif

}  // namespace misc

#define JOIN_STRINGS_INCLUDING_IMPLEMENTATION
#include "join_strings.ipp"
#undef JOIN_STRINGS_INCLUDING_IMPLEMENTATION
