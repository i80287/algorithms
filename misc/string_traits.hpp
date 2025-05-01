#pragma once

#include <string>
#include <string_view>
#include <type_traits>

#include "config_macros.hpp"

namespace misc {

template <class T>
struct is_char : std::false_type {};

template <>
struct is_char<char> : std::true_type {};

template <>
struct is_char<wchar_t> : std::true_type {};

#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
template <>
struct is_char<char8_t> : std::true_type {};
#endif

template <>
struct is_char<char16_t> : std::true_type {};

template <>
struct is_char<char32_t> : std::true_type {};

template <class T>
inline constexpr bool is_char_v = misc::is_char<T>::value;

template <class T>
struct is_basic_string : std::false_type {};

template <class CharType>
struct is_basic_string<std::basic_string<CharType>> : std::true_type {};

template <class T>
inline constexpr bool is_basic_string_v = misc::is_basic_string<T>::value;

template <class T>
struct is_basic_string_view : std::false_type {};

template <class CharType>
struct is_basic_string_view<std::basic_string_view<CharType>> : std::true_type {};

template <class T>
inline constexpr bool is_basic_string_view_v = misc::is_basic_string_view<T>::value;

template <class T>
struct is_c_str : std::false_type {};

template <class T>
struct is_c_str<T *> : misc::is_char<std::remove_cv_t<T>> {};

template <class T>
struct is_c_str<T[]> : misc::is_char<std::remove_cv_t<T>> {};

template <class T>
inline constexpr bool is_c_str_v = misc::is_c_str<T>::value;

template <class T>
struct is_c_str_arr : std::false_type {};

template <class T, size_t N>
struct is_c_str_arr<T[N]> : misc::is_char<std::remove_cv_t<T>> {};

template <class T>
inline constexpr bool is_c_str_arr_v = misc::is_c_str_arr<T>::value;

template <class T>
inline constexpr bool is_string_like_v =
    misc::is_basic_string_v<T> || misc::is_basic_string_view_v<T> || misc::is_c_str_v<T> ||
    misc::is_c_str_arr_v<T>;

#if CONFIG_COMPILER_SUPPORTS_CONCEPTS

template <class T>
concept Char = misc::is_char_v<T>;

template <class T>
concept StringLike = misc::is_string_like_v<T>;

template <class T>
concept CharOrStringLike = misc::Char<T> || misc::StringLike<T>;

#endif

}  // namespace misc

#define STRING_TRAITS_INCLUDING_IMPLEMENTATION
#include "string_traits.ipp"
#undef STRING_TRAITS_INCLUDING_IMPLEMENTATION
