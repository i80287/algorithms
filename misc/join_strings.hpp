#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>

#include "config_macros.hpp"

#if CONFIG_HAS_AT_LEAST_CXX_20 && CONFIG_HAS_CONCEPTS && CONFIG_HAS_INCLUDE(<concepts>)

#include <concepts>
#define JOIN_STRINGS_SUPPORTS_CUSTOM_ENUM_TO_STRING

#endif

namespace misc {

namespace join_strings_detail {

template <class T>
inline constexpr bool is_char_v = std::is_same_v<T, char> || std::is_same_v<T, wchar_t> ||
#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
                                  std::is_same_v<T, char8_t> ||
#endif
                                  std::is_same_v<T, char16_t> || std::is_same_v<T, char32_t>;

namespace type_traits_detail {

template <class T>
struct is_pointer_to_char : std::false_type {};

template <class T>
struct is_pointer_to_char<T *>
    : std::conditional_t<is_char_v<std::remove_cv_t<T>>, std::true_type, std::false_type> {};

}  // namespace type_traits_detail

template <class T>
inline constexpr bool is_pointer_to_char_v = type_traits_detail::is_pointer_to_char<T>::value;

template <bool UseWChar, class T>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE inline auto ArithmeticToStringImpl(const T arg) {
    static_assert(std::is_arithmetic_v<T>, "implementation error");

    if constexpr (std::is_integral_v<T>) {
        if (config::is_constant_evaluated() || config::is_gcc_constant_p(arg)) {
            if (arg == 0) {
                if constexpr (UseWChar) {
                    return std::wstring{L"0"};
                } else {
                    return std::string{"0"};
                }
            } else if constexpr (sizeof(T) > sizeof(int)) {
                if constexpr (std::is_unsigned_v<T>) {
                    if (arg <= std::numeric_limits<unsigned>::max()) {
                        const unsigned comp_arg = static_cast<unsigned>(arg);
                        if constexpr (UseWChar) {
                            return std::to_wstring(comp_arg);
                        } else {
                            return std::to_string(comp_arg);
                        }
                    }
                } else {
                    if (arg >= std::numeric_limits<int>::min() &&
                        arg <= std::numeric_limits<int>::max()) {
                        const int comp_arg = static_cast<int>(arg);
                        if constexpr (UseWChar) {
                            return std::to_wstring(comp_arg);
                        } else {
                            return std::to_string(comp_arg);
                        }
                    }
                }
            }
        }
    }

    constexpr bool kShortIntegralType = std::is_integral_v<T> && sizeof(T) < sizeof(int);

    const auto ext_arg =
        kShortIntegralType
            ? static_cast<std::conditional_t<std::is_unsigned_v<T>, unsigned, int>>(arg)
            : arg;

    if constexpr (UseWChar) {
        return std::to_wstring(ext_arg);
    } else {
        return std::to_string(ext_arg);
    }
}

template <class CharType, class T>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE inline std::basic_string<CharType> ArithmeticToString(const T arg) {
    static_assert(is_char_v<CharType>, "implementation error");
    static_assert(!is_char_v<T>, "implementation error");
    static_assert(std::is_arithmetic_v<T>, "implementation error");

    const auto str = ArithmeticToStringImpl<std::is_same_v<CharType, wchar_t>>(arg);
    if constexpr (std::is_same_v<CharType, char> || std::is_same_v<CharType, wchar_t>) {
        return str;
    } else {
        return std::basic_string<CharType>(str.begin(), str.end());
    }
}

template <class CharType, class T>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE inline std::basic_string<CharType> EnumToString(const T arg) {
    static_assert(std::is_enum_v<T>, "implementation error");

    if constexpr (std::is_error_code_enum_v<T>) {
        const std::string str = std::make_error_code(arg).message();
        if constexpr (std::is_same_v<CharType, char>) {
            return str;
        } else {
            return std::basic_string<CharType>(str.begin(), str.end());
        }
    } else if constexpr (std::is_error_condition_enum_v<T>) {
        const std::string str = std::make_error_condition(arg).message();
        if constexpr (std::is_same_v<CharType, char>) {
            return str;
        } else {
            return std::basic_string<CharType>(str.begin(), str.end());
        }
    } else
#ifdef JOIN_STRINGS_SUPPORTS_CUSTOM_ENUM_TO_STRING
        if constexpr (requires(const T &enum_value) {
                          {
                              to_basic_string<CharType>(enum_value)
                          } -> std::same_as<std::basic_string<CharType>>;
                      }) {
        return to_basic_string<CharType>(arg);
    } else if constexpr (requires(const T &enum_value) {
                             { to_wstring(enum_value) } -> std::same_as<std::wstring>;
                         } && std::is_same_v<CharType, wchar_t>) {
    } else if constexpr (requires(const T &enum_value) {
                             { to_string(enum_value) } -> std::same_as<std::string>;
                         }) {
        const std::string str = to_string(arg);
        if constexpr (std::is_same_v<CharType, char>) {
            return str;
        } else {
            return std::basic_string<CharType>(str.begin(), str.end());
        }
    } else
#endif
    {
        return ArithmeticToString<CharType>(static_cast<std::underlying_type_t<T>>(arg));
    }
}

template <class CharType, class T>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE inline std::basic_string<CharType> PointerTypeToString(const T arg) {
    static_assert(is_char_v<CharType>, "implementation error");
    static_assert(std::is_pointer_v<T> || std::is_member_pointer_v<T> || std::is_null_pointer_v<T>,
                  "implementation error");

    const std::uintptr_t ptr_num = reinterpret_cast<std::uintptr_t>(arg);
    if (std::is_null_pointer_v<T> || ptr_num == 0) {
        if constexpr (std::is_same_v<CharType, char>) {
            return "null";
        } else if constexpr (std::is_same_v<CharType, wchar_t>) {
            return L"null";
#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
        } else if constexpr (std::is_same_v<CharType, char8_t>) {
            return u8"null";
#endif
        } else if constexpr (std::is_same_v<CharType, char16_t>) {
            return u"null";
        } else if constexpr (std::is_same_v<CharType, char32_t>) {
            return U"null";
        } else {
            static_assert([]() constexpr { return false; }(), "implementation error");
            return {};
        }
    } else {
        return ArithmeticToString<CharType>(ptr_num);
    }
}

template <class CharType, class T>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE inline std::basic_string<CharType> ToStringScalarArg(const T arg) {
    static_assert(is_char_v<CharType>, "implementation error");
    static_assert(std::is_scalar_v<T>, "implementation error");

    if constexpr (std::is_enum_v<T>) {
        return EnumToString<CharType>(arg);
    } else if constexpr (std::is_same_v<T, CharType>) {
        return std::basic_string<CharType>(std::size_t{1}, arg);
    } else if constexpr (std::is_arithmetic_v<T>) {
        return ArithmeticToString<CharType>(arg);
    } else {
        return PointerTypeToString<CharType>(arg);
    }
}

template <class CharType, class T>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE inline std::basic_string<CharType> ToStringOneArg(const T &arg) {
    static_assert(is_char_v<CharType>, "implementation error");

    if constexpr (std::is_scalar_v<T>) {
        return ToStringScalarArg<CharType>(arg);
    } else {
        return std::basic_string<CharType>{arg};
    }
}

// clang-format off

template <class CharType, class... Args>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE
constexpr std::enable_if_t<is_char_v<CharType>, std::size_t> CalculateStringArgsSize(CharType /*c*/, Args... args) noexcept;

template <class CharType, class... Args>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE
constexpr std::enable_if_t<is_char_v<CharType>, std::size_t> CalculateStringArgsSize(std::basic_string_view<CharType> s, Args... args) noexcept;

// clang-format on

template <class CharType, class... Args>
constexpr std::enable_if_t<is_char_v<CharType>, std::size_t> CalculateStringArgsSize(
    CharType /*c*/, Args... args) noexcept {
    size_t size = 1;
    if constexpr (sizeof...(args) > 0) {
        size += join_strings_detail::CalculateStringArgsSize<CharType>(args...);
    }
    return size;
}

template <class CharType, class... Args>
constexpr std::enable_if_t<is_char_v<CharType>, std::size_t> CalculateStringArgsSize(
    std::basic_string_view<CharType> s, Args... args) noexcept {
    size_t size = s.size();
    if constexpr (sizeof...(args) > 0) {
        size += join_strings_detail::CalculateStringArgsSize<CharType>(args...);
    }
    return size;
}

// clang-format off

template <class CharType, class... Args>
ATTRIBUTE_NONNULL_ALL_ARGS
ATTRIBUTE_ACCESS(write_only, 1)
ATTRIBUTE_ALWAYS_INLINE
constexpr std::enable_if_t<is_char_v<CharType>, void> WriteStringsInplace(CharType* result, CharType c, Args... args) noexcept;

template <class CharType, class... Args>
ATTRIBUTE_NONNULL_ALL_ARGS
ATTRIBUTE_ACCESS(write_only, 1)
ATTRIBUTE_ALWAYS_INLINE
constexpr std::enable_if_t<is_char_v<CharType>, void> WriteStringsInplace(CharType* result, std::basic_string_view<CharType> s, Args... args) noexcept;

// clang-format on

template <class CharType, class... Args>
constexpr std::enable_if_t<is_char_v<CharType>, void> WriteStringsInplace(CharType *result,
                                                                          CharType c,
                                                                          Args... args) noexcept {
    *result = c;
    if constexpr (sizeof...(args) > 0) {
        join_strings_detail::WriteStringsInplace<CharType>(result + 1, args...);
    }
}

template <class CharType, class... Args>
constexpr std::enable_if_t<is_char_v<CharType>, void> WriteStringsInplace(
    CharType *result, std::basic_string_view<CharType> s, Args... args) noexcept {
    std::char_traits<CharType>::copy(result, s.data(), s.size());
    if constexpr (sizeof...(args) > 0) {
        join_strings_detail::WriteStringsInplace<CharType>(result + s.size(), args...);
    }
}

// clang-format off

template <class CharType, class... Args>
ATTRIBUTE_NONNULL_ALL_ARGS
ATTRIBUTE_SIZED_ACCESS(write_only, 1, 2)
ATTRIBUTE_ALWAYS_INLINE
constexpr std::enable_if_t<is_char_v<CharType>, void> WriteStringToBuffer(CharType* buffer, size_t /*buffer_size*/, Args... args) noexcept {
    join_strings_detail::WriteStringsInplace<CharType>(buffer, args...);
}

// clang-format on

template <class CharType, class... Args>
[[nodiscard]] inline std::enable_if_t<is_char_v<CharType>, std::basic_string<CharType>>
JoinStringsImpl(Args... args) {
    if constexpr (sizeof...(args) >= 2) {
        const std::size_t size = CalculateStringArgsSize<CharType>(args...);
        std::basic_string<CharType> result(size, CharType{});
        join_strings_detail::WriteStringToBuffer<CharType>(result.data(), result.size(), args...);
        return result;
    } else {
        return join_strings_detail::ToStringOneArg<CharType>(args...);
    }
}

// clang-format off

template <class CharType, size_t I, class... Args>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE
inline
std::enable_if_t<is_char_v<CharType>, std::basic_string<CharType>> JoinStringsConvArgsToStrViewImpl(std::basic_string_view<CharType> str, const Args&... args);

template <class CharType, size_t I, class T, class... Args>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE
inline
std::enable_if_t<is_char_v<CharType> && std::is_scalar_v<T> && !is_pointer_to_char_v<T>, std::basic_string<CharType>>
JoinStringsConvArgsToStrViewImpl(T num, const Args&... args);

// clang-format on

template <class CharType, size_t I, class... Args>
inline std::enable_if_t<is_char_v<CharType>, std::basic_string<CharType>>
JoinStringsConvArgsToStrViewImpl(std::basic_string_view<CharType> str, const Args &...args) {
    if constexpr (I == 1 + sizeof...(args)) {
        return join_strings_detail::JoinStringsImpl<CharType>(str, args...);
    } else {
        return join_strings_detail::JoinStringsConvArgsToStrViewImpl<CharType, I + 1>(args..., str);
    }
}

template <class CharType, size_t I, class T, class... Args>
inline std::enable_if_t<is_char_v<CharType> && std::is_scalar_v<T> && !is_pointer_to_char_v<T>,
                        std::basic_string<CharType>>
JoinStringsConvArgsToStrViewImpl(T num, const Args &...args) {
    if constexpr (std::is_same_v<T, CharType>) {
        if constexpr (I == 1 + sizeof...(args)) {
            return join_strings_detail::JoinStringsImpl<CharType>(num, args...);
        } else {
            return join_strings_detail::JoinStringsConvArgsToStrViewImpl<CharType, I + 1>(args...,
                                                                                          num);
        }
    } else {
        static_assert(I < 1 + sizeof...(args));
        return join_strings_detail::JoinStringsConvArgsToStrViewImpl<CharType, I + 1>(
            args..., std::basic_string_view<CharType>{ToStringOneArg<CharType>(num)});
    }
}

struct dummy_base {};

template <class T>
struct dummy_base_with_type {
    using type = T;
};

template <class T, class CharType>
struct same_char_types
    : std::conditional_t<std::is_scalar_v<T>, std::true_type, std::false_type> {};

template <class CharType>
struct same_char_types<std::basic_string<CharType>, CharType> : std::true_type {};

template <class CharType>
struct same_char_types<std::basic_string_view<CharType>, CharType> : std::true_type {};

template <class CharType, std::size_t N>
struct same_char_types<const CharType[N], CharType> : std::true_type {};

template <class CharType, std::size_t N>
struct same_char_types<CharType[N], CharType> : std::true_type {};

template <class CharType>
struct same_char_types<const CharType *, CharType> : std::true_type {};

template <class CharType>
struct same_char_types<CharType *, CharType> : std::true_type {};

template <class CharType>
struct same_char_types<CharType, CharType> : std::true_type {};

template <class... Types>
struct determine_char_type {
    using type = void;
};

template <class... Types>
struct recursion_selector final {
    using type = typename determine_char_type<Types...>::type;
};

template <class CharType>
struct char_selector final {
    using type = CharType;
};

template <class FirstType, class... Types>
struct determine_char_type<FirstType, Types...> {
    using selector = std::
        conditional_t<is_char_v<FirstType>, char_selector<FirstType>, recursion_selector<Types...>>;

    using type = typename selector::type;
};

template <class CharType, class... Types>
struct determine_char_type<std::basic_string_view<CharType>, Types...> {
    using type = CharType;
};

template <class CharType, class... Types>
struct determine_char_type<std::basic_string<CharType>, Types...> {
    using type = CharType;
};

template <class CharType, std::size_t N, class... Types>
struct determine_char_type<const CharType[N], Types...>
    : std::enable_if_t<is_char_v<CharType>, dummy_base> {
    using type = CharType;
};

template <class CharType, std::size_t N, class... Types>
struct determine_char_type<CharType[N], Types...>
    : std::enable_if_t<is_char_v<CharType>, dummy_base> {
    using type = CharType;
};

template <class CharType, class... Types>
struct determine_char_type<const CharType *, Types...>
    : std::conditional_t<is_char_v<CharType>,
                         dummy_base_with_type<CharType>,
                         determine_char_type<Types...>> {
    using Base = std::conditional_t<is_char_v<CharType>,
                                    dummy_base_with_type<CharType>,
                                    determine_char_type<Types...>>;
    using type = typename Base::type;
};

template <class CharType, class... Types>
struct determine_char_type<CharType *, Types...>
    : std::conditional_t<is_char_v<CharType>,
                         dummy_base_with_type<CharType>,
                         determine_char_type<Types...>> {
    using Base = std::conditional_t<is_char_v<CharType>,
                                    dummy_base_with_type<CharType>,
                                    determine_char_type<Types...>>;
    using type = typename Base::type;
};

template <class... Args>
using determine_char_t = typename determine_char_type<Args...>::type;

}  // namespace join_strings_detail

// clang-format off

/// @brief Join arguments @a args... (converting to string if necessary)
/// @tparam ...Args Types of the arguments to join, e.g. char types, pointers to them,
///         basic_string, basic_string_view, integral/floating point values
/// @tparam HintCharType hint char type (default: `char`).
///         Can be passed if, for instance, JoinStrings(1, 2, 3.0)
///         should be std::wstring: JoinStrings<wchar_t>(1, 2, 3.0)
/// @param ...args arguments to join
/// @return joined args as a string of type std::basic_string<CharType>
///         where CharType is deducted by the @a Args... or HintCharType
///         if @a Args... is pack of numeric types
template <class HintCharType = char, class... Args>
[[nodiscard]] ATTRIBUTE_ALWAYS_INLINE inline auto JoinStrings(const Args&... args) {
    static_assert(sizeof...(args) >= 1, "Empty input is explicitly prohibited");

    using DeducedCharType = join_strings_detail::determine_char_t<Args...>;

    static_assert(join_strings_detail::is_char_v<HintCharType>, "Hint type should be char, wchar_t, char8_t, char16_t or char32_t");
    using CharType = std::conditional_t<join_strings_detail::is_char_v<DeducedCharType>, DeducedCharType, HintCharType>;

    constexpr bool kAllCharTypesAreSame = std::conjunction_v<join_strings_detail::same_char_types<Args, CharType>...>;
    static_assert(
        kAllCharTypesAreSame,
        "Hint:\n"
        "    Some non integral/float arguments have different char types\n"
        "    For example, both std::string and std::wstring might have been passed to the JoinStrings\n");

    if constexpr (kAllCharTypesAreSame) {
        return join_strings_detail::JoinStringsConvArgsToStrViewImpl<CharType, 0>(args...);
    } else {
        // Do not make more unreadable CEs when kAllCharTypesAreSame == false, static assertion has already failed
        return std::basic_string<CharType>{};
    }
}

// clang-format on

}  // namespace misc
