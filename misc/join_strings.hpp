#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>

#include "config_macros.hpp"

namespace misc {

namespace join_strings_detail {

template <class T>
inline constexpr bool is_char_v = std::is_same_v<T, char> || std::is_same_v<T, wchar_t> ||
#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
                                  std::is_same_v<T, char8_t> ||
#endif
                                  std::is_same_v<T, char16_t> || std::is_same_v<T, char32_t>;

template <bool UseWChar, class T>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE inline auto NumberToString(const T arg) {
    static_assert(std::is_arithmetic_v<T>);

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

    if constexpr (UseWChar) {
        return std::to_wstring(arg);
    } else {
        return std::to_string(arg);
    }
}

template <class CharType, class T, std::enable_if_t<is_char_v<CharType>, int> = 0>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE inline std::basic_string<CharType> ToStringOneArg(const T &arg) {
    if constexpr (!std::is_arithmetic_v<T>) {
        return std::basic_string<CharType>{std::basic_string_view<CharType>{arg}};
    } else if constexpr (std::is_same_v<T, CharType>) {
        return std::basic_string<CharType>(std::size_t{1}, arg);
    } else {
        static_assert(!is_char_v<T>, "implementation error");
        const auto str = NumberToString<std::is_same_v<CharType, wchar_t>>(arg);
        if constexpr (std::is_same_v<CharType, char> || std::is_same_v<CharType, wchar_t>) {
            return str;
        } else {
            return std::basic_string<CharType>(str.begin(), str.end());
        }
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

template <class CharType, size_t I, class... Args>
[[nodiscard]]
ATTRIBUTE_ACCESS(read_only, 1)
ATTRIBUTE_ALWAYS_INLINE
inline
std::enable_if_t<is_char_v<CharType>, std::basic_string<CharType>> JoinStringsConvArgsToStrViewImpl(const CharType* str, const Args&... args);

template <class CharType, size_t I, class T, class... Args>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE
inline
std::enable_if_t<is_char_v<CharType> && std::is_arithmetic_v<T>, std::basic_string<CharType>> JoinStringsConvArgsToStrViewImpl(T num, const Args&... args);

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

template <class CharType, size_t I, class... Args>
inline std::enable_if_t<is_char_v<CharType>, std::basic_string<CharType>>
JoinStringsConvArgsToStrViewImpl(const CharType *str, const Args &...args) {
    static_assert(I < 1 + sizeof...(args));
    return join_strings_detail::JoinStringsConvArgsToStrViewImpl<CharType, I + 1>(
        args..., str ? std::basic_string_view<CharType>{str} : std::basic_string_view<CharType>{});
}

template <class CharType, size_t I, class T, class... Args>
inline std::enable_if_t<is_char_v<CharType> && std::is_arithmetic_v<T>, std::basic_string<CharType>>
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

template <class T, class CharType>
struct same_char_types
    : std::conditional_t<std::is_arithmetic_v<T>, std::true_type, std::false_type> {};

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
struct determine_char_type final {
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
struct determine_char_type<FirstType, Types...> final {
    using selector = std::
        conditional_t<is_char_v<FirstType>, char_selector<FirstType>, recursion_selector<Types...>>;

    using type = typename selector::type;
};

template <class CharType, class... Types>
struct determine_char_type<std::basic_string_view<CharType>, Types...> final {
    using type = CharType;
};

template <class CharType, class... Types>
struct determine_char_type<std::basic_string<CharType>, Types...> final {
    using type = CharType;
};

template <class CharType, std::size_t N, class... Types>
struct determine_char_type<const CharType[N], Types...> final
    : std::enable_if_t<is_char_v<CharType>, dummy_base> {
    using type = CharType;
};

template <class CharType, std::size_t N, class... Types>
struct determine_char_type<CharType[N], Types...> final
    : std::enable_if_t<is_char_v<CharType>, dummy_base> {
    using type = CharType;
};

template <class CharType, class... Types>
struct determine_char_type<const CharType *, Types...> final
    : std::enable_if_t<is_char_v<CharType>, dummy_base> {
    using type = CharType;
};

template <class CharType, class... Types>
struct determine_char_type<CharType *, Types...> final
    : std::enable_if_t<is_char_v<CharType>, dummy_base> {
    using type = CharType;
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
