#ifndef JOIN_STRINGS_INCLUDING_IMPLEMENTATION
// cppcheck-suppress [preprocessorErrorDirective]
#error This header should not be used directly
#endif

#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cwchar>
#include <cwctype>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include "config_macros.hpp"

namespace misc {

namespace join_strings_detail {

using std::size_t;

template <class T>
struct is_pointer_to_char : std::false_type {};

template <class T>
struct is_pointer_to_char<T *>
    : std::conditional_t<is_char_v<std::remove_cv_t<T>>, std::true_type, std::false_type> {};

template <class T>
inline constexpr bool is_pointer_to_char_v = is_pointer_to_char<T>::value;

#ifdef JOIN_STRINGS_SUPPORTS_FILESYSTEM_PATH

template <class T>
inline constexpr bool is_filesystem_path_v = std::is_same_v<T, std::filesystem::path>;

#else

template <class>
constexpr bool is_filesystem_path_v = false;

#endif

template <bool UseWChar, class T>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE inline auto ArithmeticToStringImpl(const T arg) {
    static_assert(std::is_arithmetic_v<T>, "implementation error");

    if constexpr (std::is_integral_v<T>) {
        if (config::is_constant_evaluated() || config::is_gcc_constant_p(arg)) {
            if constexpr (sizeof(T) > sizeof(int)) {
                using CompressedIntType = std::conditional_t<std::is_unsigned_v<T>, unsigned, int>;

                if (arg >= std::numeric_limits<CompressedIntType>::min() &&
                    arg <= std::numeric_limits<CompressedIntType>::max()) {
                    return join_strings_detail::ArithmeticToStringImpl<UseWChar>(
                        static_cast<CompressedIntType>(arg));
                }
            }
        }
    }

    constexpr bool kShortIntegralType = std::is_integral_v<T> && sizeof(T) < sizeof(int);

    const auto extended_arg =
        kShortIntegralType
            ? static_cast<std::conditional_t<std::is_unsigned_v<T>, unsigned, int>>(arg)
            : arg;

    if constexpr (UseWChar) {
        return std::to_wstring(extended_arg);
    } else {
        return std::to_string(extended_arg);
    }
}

template <class ToCharType>
[[nodiscard]] inline std::basic_string<ToCharType> ConvertString(const std::string_view str) {
    static_assert(!std::is_same_v<ToCharType, char>, "implementation error");

#if CONFIG_HAS_AT_LEAST_CXX_20 && defined(__cpp_char8_t) && __cpp_char8_t >= 201811L
    if constexpr (std::is_same_v<ToCharType, char8_t>) {
        return std::basic_string<ToCharType>(str.begin(), str.end());
    } else
#endif
    {
        return std::wstring_convert<std::codecvt_utf8_utf16<ToCharType>, ToCharType>{}.from_bytes(
            str.data(), str.data() + str.size());
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
        return ConvertString<CharType>(str);
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
            return ConvertString<CharType>(str);
        }
    } else if constexpr (std::is_error_condition_enum_v<T>) {
        const std::string str = std::make_error_condition(arg).message();
        if constexpr (std::is_same_v<CharType, char>) {
            return str;
        } else {
            return ConvertString<CharType>(str);
        }
    } else {
        return ArithmeticToString<CharType>(static_cast<std::underlying_type_t<T>>(arg));
    }
}

template <class CharType>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE inline std::basic_string<CharType> NullPtrToString() {
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
}

template <class CharType, class T>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE inline std::basic_string<CharType> PointerTypeToString(const T arg) {
    static_assert(is_char_v<CharType>, "implementation error");
    static_assert(std::is_pointer_v<T> || std::is_member_pointer_v<T> || std::is_null_pointer_v<T>,
                  "implementation error");

    const auto ptr_num = reinterpret_cast<std::uintptr_t>(arg);
    if constexpr (std::is_null_pointer_v<T>) {
        return NullPtrToString<CharType>();
    } else if (ptr_num == 0) {
        return NullPtrToString<CharType>();
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
        return std::basic_string<CharType>(size_t{1}, arg);
    } else if constexpr (std::is_arithmetic_v<T>) {
        return ArithmeticToString<CharType>(arg);
    } else {
        return PointerTypeToString<CharType>(arg);
    }
}

#ifdef JOIN_STRINGS_SUPPORTS_FILESYSTEM_PATH

// clang-format off
template <class CharType>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE
inline std::basic_string<CharType> FilesystemPathToString(const std::filesystem::path &path) {
    // clang-format on

    static_assert(is_char_v<CharType>, "implementation error");
    return path.template generic_string<CharType>();
}

#else

template <class CharType, class T>
std::basic_string<CharType> FilesystemPathToString(const T &) = delete;

#endif

#ifdef JOIN_STRINGS_SUPPORTS_CUSTOM_OSTRINGSTREAM

template <class T, class CharType>
concept WriteableViaOStringStream =
    requires(const T &arg, std::basic_ostringstream<CharType> &oss) {
        { oss << arg };
    };

template <class T, class CharType>
concept DirectlyConvertableToString =
    std::constructible_from<std::basic_string<CharType>, const T &>;

template <class CharType, class T>
[[nodiscard]] inline std::basic_string<CharType> ToStringOneArgViaOStringStream(const T &arg) {
    std::basic_ostringstream<CharType> oss;
    // not std::ignore because user defined operator<< may return void
    static_cast<void>(oss << arg);
    return std::move(oss).str();
}

#endif

template <class CharType, class T>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE inline std::basic_string<CharType> ToStringOneArg(const T &arg) {
    static_assert(is_char_v<CharType>, "implementation error");

#ifdef JOIN_STRINGS_SUPPORTS_CUSTOM_TO_STRING
    if constexpr (requires(const T &test_arg) {
                      {
                          to_basic_string<CharType>(test_arg)
                      } -> std::same_as<std::basic_string<CharType>>;
                  }) {
        return to_basic_string<CharType>(arg);
    } else if constexpr (requires(const T &test_arg) {
                             { to_wstring(test_arg) } -> std::same_as<std::wstring>;
                         } && std::is_same_v<CharType, wchar_t>) {
        return to_wstring(arg);
    } else if constexpr (requires(const T &test_arg) {
                             { to_string(test_arg) } -> std::same_as<std::string>;
                         }) {
        const std::string str = to_string(arg);
        if constexpr (std::is_same_v<CharType, char>) {
            return str;
        } else {
            return ConvertString<CharType>(str);
        }
    } else if constexpr (requires(const T &test_arg) {
                             { test_arg.to_string() } -> std::same_as<std::string>;
                         }) {
        const std::string str = arg.to_string();
        if constexpr (std::is_same_v<CharType, char>) {
            return str;
        } else {
            return ConvertString<CharType>(str);
        }
    } else
#endif
        if constexpr (std::is_scalar_v<T>) {
        return ToStringScalarArg<CharType>(arg);
    } else if constexpr (is_filesystem_path_v<T>) {
        return FilesystemPathToString<CharType>(arg);
#ifdef JOIN_STRINGS_SUPPORTS_CUSTOM_OSTRINGSTREAM
    } else if constexpr (WriteableViaOStringStream<T, CharType> &&
                         !DirectlyConvertableToString<T, CharType>) {
        return ToStringOneArgViaOStringStream<CharType>(arg);
#endif
    } else {
        return std::basic_string<CharType>{arg};
    }
}

// clang-format off

template <class CharType, class... Args>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE
constexpr size_t CalculateStringArgsSize(CharType chr, Args... args) noexcept;

template <class CharType, class... Args>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE
constexpr size_t CalculateStringArgsSize(std::basic_string_view<CharType> s, Args... args) noexcept;

// clang-format on

template <class CharType, class... Args>
constexpr size_t CalculateStringArgsSize(CharType /*chr*/, Args... args) noexcept {
    size_t size = 1;
    if constexpr (sizeof...(args) > 0) {
        const size_t other_args_size =
            join_strings_detail::CalculateStringArgsSize<CharType>(args...);
        const bool will_overflow = other_args_size == std::numeric_limits<size_t>::max();
        if (unlikely(will_overflow)) {
            return other_args_size;
        }

        size += other_args_size;
    }

    return size;
}

template <class CharType, class... Args>
constexpr size_t CalculateStringArgsSize(const std::basic_string_view<CharType> s,
                                         Args... args) noexcept {
    size_t size = s.size();
    if constexpr (sizeof...(args) > 0) {
        const size_t other_args_size =
            join_strings_detail::CalculateStringArgsSize<CharType>(args...);
        size += other_args_size;
        const bool overflow_occured = size < other_args_size;
        if (unlikely(overflow_occured)) {
            return std::numeric_limits<size_t>::max();
        }
    }

    return size;
}

// clang-format off

template <class CharType, class... Args>
ATTRIBUTE_NONNULL_ALL_ARGS
ATTRIBUTE_ACCESS(write_only, 1)
ATTRIBUTE_ALWAYS_INLINE
constexpr void WriteStringsInplace(CharType* result, CharType c, Args... args) noexcept;

template <class CharType, class... Args>
ATTRIBUTE_NONNULL_ALL_ARGS
ATTRIBUTE_ACCESS(write_only, 1)
ATTRIBUTE_ALWAYS_INLINE
constexpr void WriteStringsInplace(CharType* result, std::basic_string_view<CharType> s, Args... args) noexcept;

// clang-format on

template <class CharType, class... Args>
constexpr void WriteStringsInplace(CharType *const result,
                                   const CharType c,
                                   Args... args) noexcept {
    *result = c;
    if constexpr (sizeof...(args) > 0) {
        join_strings_detail::WriteStringsInplace<CharType>(result + 1, args...);
    }
}

template <class CharType, class... Args>
constexpr void WriteStringsInplace(CharType *const result,
                                   const std::basic_string_view<CharType> s,
                                   Args... args) noexcept {
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
constexpr void WriteStringToBuffer(CharType* const buffer, size_t /*buffer_size*/, Args... args) noexcept {
    join_strings_detail::WriteStringsInplace<CharType>(buffer, args...);
}

// clang-format on

template <class CharType, class... Args>
[[nodiscard]] inline std::basic_string<CharType> JoinStringsImpl(Args... args) {
    if constexpr (sizeof...(args) >= 2) {
        const size_t size = CalculateStringArgsSize<CharType>(args...);
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
inline std::basic_string<CharType> JoinStringsConvArgsToStrViewImpl(std::basic_string_view<CharType> str, const Args&... args);

template <class CharType, size_t I, class T, class... Args>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE
inline std::enable_if_t<!is_pointer_to_char_v<T>, std::basic_string<CharType>> JoinStringsConvArgsToStrViewImpl(const T& value, const Args&... args);

// clang-format on

template <class CharType, size_t I, class... Args>
inline std::basic_string<CharType> JoinStringsConvArgsToStrViewImpl(
    const std::basic_string_view<CharType> str, const Args &...args) {
    if constexpr (I == 1 + sizeof...(args)) {
        return join_strings_detail::JoinStringsImpl<CharType>(str, args...);
    } else {
        return join_strings_detail::JoinStringsConvArgsToStrViewImpl<CharType, I + 1>(args..., str);
    }
}

template <class CharType, size_t I, class T, class... Args>
inline std::enable_if_t<!is_pointer_to_char_v<T>, std::basic_string<CharType>>
JoinStringsConvArgsToStrViewImpl(const T &value, const Args &...args) {
    if constexpr (std::is_same_v<T, CharType>) {
        if constexpr (I == 1 + sizeof...(args)) {
            return join_strings_detail::JoinStringsImpl<CharType>(value, args...);
        } else {
            return join_strings_detail::JoinStringsConvArgsToStrViewImpl<CharType, I + 1>(args...,
                                                                                          value);
        }
    } else {
        static_assert(I < 1 + sizeof...(args));
        return join_strings_detail::JoinStringsConvArgsToStrViewImpl<CharType, I + 1>(
            args..., std::basic_string_view<CharType>{ToStringOneArg<CharType>(value)});
    }
}

template <class T, class CharType>
struct same_char_types
    : std::conditional_t<misc::is_char_v<T>, std::is_same<T, CharType>, std::true_type> {};

template <class StrCharType, class CharType>
struct same_char_types<std::basic_string<StrCharType>, CharType>
    : std::is_same<StrCharType, CharType> {};

template <class StrCharType, class CharType>
struct same_char_types<std::basic_string_view<StrCharType>, CharType>
    : std::is_same<StrCharType, CharType> {};

template <class StrCharType, class CharType, size_t N>
struct same_char_types<const StrCharType[N], CharType> : std::is_same<StrCharType, CharType> {};

template <class StrCharType, class CharType, size_t N>
struct same_char_types<StrCharType[N], CharType> : std::is_same<StrCharType, CharType> {};

template <class StrCharType, class CharType>
struct same_char_types<const StrCharType *, CharType>
    : std::conditional_t<misc::is_char_v<StrCharType>,
                         std::is_same<StrCharType, CharType>,
                         std::true_type> {};

template <class StrCharType, class CharType>
struct same_char_types<StrCharType *, CharType>
    : std::conditional_t<misc::is_char_v<StrCharType>,
                         std::is_same<StrCharType, CharType>,
                         std::true_type> {};

struct dummy_base {};

template <class T>
struct dummy_base_with_type {
    using type = T;
};

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

template <class CharType, size_t N, class... Types>
struct determine_char_type<const CharType[N], Types...>
    : std::enable_if_t<is_char_v<CharType>, dummy_base> {
    using type = CharType;
};

template <class CharType, size_t N, class... Types>
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
template <class HintCharType, class... Args>
inline auto join_strings(const Args&... args) {
    static_assert(sizeof...(args) >= 1, "Empty input is explicitly prohibited");

    using DeducedCharType = join_strings_detail::determine_char_t<Args...>;

    static_assert(misc::is_char_v<HintCharType>, "Hint type should be char, wchar_t, char8_t, char16_t or char32_t");
    using CharType = std::conditional_t<misc::is_char_v<DeducedCharType>, DeducedCharType, HintCharType>;

    constexpr bool kAllCharTypesAreSame = std::conjunction_v<join_strings_detail::same_char_types<Args, CharType>...>;
    static_assert(
        kAllCharTypesAreSame,
        "Hint:\n"
        "    Some non integral/float arguments have different char types\n"
        "    For example, both std::string and std::wstring might have been passed to the join_strings\n");

    if constexpr (kAllCharTypesAreSame) {
        return join_strings_detail::JoinStringsConvArgsToStrViewImpl<CharType, 0>(args...);
    } else {
        // Do not make more unreadable CEs when kAllCharTypesAreSame == false, static assertion has already failed
        return std::basic_string<CharType>{};
    }
}

template <class ToCharType>
inline std::basic_string<ToCharType> convert_bytes_to(const std::string_view bytes_str) {
    return join_strings_detail::ConvertString<ToCharType>(bytes_str);
}

// clang-format on

#ifdef JOIN_STRINGS_SUPPORTS_JOIN_STRINGS_COLLECTION

namespace join_strings_detail {

template <class StringType,
          bool has_value_type =
              misc::is_basic_string_v<StringType> || misc::is_basic_string_view_v<StringType>,
          bool is_c_str = misc::is_c_str_v<StringType> || misc::is_c_str_arr_v<StringType>>
struct string_char_selector {
    using type = void;
};

template <class StringType>
struct string_char_selector<StringType, true, false> {
    using type = typename StringType::value_type;
};

template <class StringType>
struct string_char_selector<StringType, false, true> {
    using type = typename std::iter_value_t<StringType>;
};

template <class StringType>
using string_char_t = typename string_char_selector<StringType>::type;

[[noreturn]] ATTRIBUTE_COLD inline void ThrowOnStringsTotalSizeOverflow() {
    constexpr const char kMessage[] =
        "join_strings_collection(): total strings length exceded max size_t value";
    throw std::length_error{kMessage};
}

inline constexpr size_t kSizeOnOverflow = std::numeric_limits<size_t>::max();

template <std::ranges::forward_range Container>
[[nodiscard]] size_t StringsTotalSizeImpl(const Container &strings) noexcept {
    size_t total_size = 0;
    bool overflow = false;
    for (const auto &elem : strings) {
        const size_t elem_size = elem.size();
        const size_t new_size = total_size + elem_size;
        overflow |= total_size > new_size || elem_size > new_size;
        total_size = new_size;
    }

    return overflow ? kSizeOnOverflow : total_size;
}

template <std::ranges::forward_range Container>
[[nodiscard]] size_t StringsTotalSize(const Container &strings) {
    const size_t strings_total_size = join_strings_detail::StringsTotalSizeImpl(strings);
    const bool total_size_overflow = strings_total_size == kSizeOnOverflow;

    if (unlikely(total_size_overflow)) {
        ThrowOnStringsTotalSizeOverflow();
    }

    return strings_total_size;
}

template <std::ranges::forward_range Container>
[[nodiscard]] size_t StringsTotalSizeWithCharSep(const Container &strings) {
    const size_t strings_total_size = join_strings_detail::StringsTotalSizeImpl(strings);
    const size_t seps_total_size = strings.size();
    const bool strings_size_overflow = strings_total_size == kSizeOnOverflow;
    const size_t total_size_with_seps = strings_total_size + seps_total_size;
    const bool total_size_overflow =
        strings_total_size > total_size_with_seps || seps_total_size > total_size_with_seps;

    if (unlikely(strings_size_overflow || total_size_overflow)) {
        ThrowOnStringsTotalSizeOverflow();
    }

    return total_size_with_seps;
}

// clang-format off
template <misc::Char T, std::ranges::forward_range Container>
[[nodiscard]] size_t StringsTotalSizeWithAtLeast2SvSep(
    const std::basic_string_view<T> sep,
    const Container &strings
) {
    // clang-format on

    const size_t strings_total_size = join_strings_detail::StringsTotalSizeImpl(strings);
    const size_t container_size = strings.size();
    const size_t sep_size = sep.size();
    CONFIG_ASSUME_STATEMENT(sep_size >= 2);
    const size_t seps_total_size =
        container_size == 0 ? size_t{0} : sep_size * (container_size - 1);
    const size_t total_size_with_seps = strings_total_size + seps_total_size;

    const bool strings_size_overflow = strings_total_size == kSizeOnOverflow;
    const bool seps_size_overflow =
        container_size >= 2 && (sep_size > seps_total_size || container_size - 1 > seps_total_size);
    const bool total_size_overflow =
        strings_total_size > total_size_with_seps || seps_total_size > total_size_with_seps;

    if (unlikely(strings_size_overflow || seps_size_overflow || total_size_overflow)) {
        ThrowOnStringsTotalSizeOverflow();
    }

    return total_size_with_seps;
}

template <misc::Char T, std::ranges::forward_range Container>
[[nodiscard]] std::basic_string<T> JoinStringsCollectionWithEmptySep(const Container &strings) {
    const size_t total_size = join_strings_detail::StringsTotalSize(strings);
    std::basic_string<T> result(total_size, '\0');
    T *write_ptr = result.data();
    for (const auto &elem : strings) {
        std::char_traits<T>::copy(write_ptr, elem.data(), elem.size());
        write_ptr += elem.size();
    }

    return result;
}

template <misc::Char T, std::ranges::forward_range Container>
[[nodiscard]] std::basic_string<T> JoinStringsCollectionByChar(const T sep,
                                                               const Container &strings) {
    const size_t total_size = join_strings_detail::StringsTotalSizeWithCharSep(strings);
    std::basic_string<T> result(total_size, '\0');
    T *write_ptr = result.data();
    for (const auto &elem : strings) {
        std::char_traits<T>::copy(write_ptr, elem.data(), elem.size());
        write_ptr += elem.size();
        *write_ptr = sep;
        ++write_ptr;
    }
    if (!result.empty()) {
        result.pop_back();
    }

    return result;
}

// clang-format off
template <misc::Char T, std::ranges::forward_range Container>
[[nodiscard]] std::basic_string<T> JoinStringsCollectionBySvAtLeast2(
    const std::basic_string_view<T> sep,
    const Container &strings
) {
    // clang-format on
    const size_t total_size = join_strings_detail::StringsTotalSizeWithAtLeast2SvSep(sep, strings);
    std::basic_string<T> result(total_size, '\0');

    auto iter = std::begin(strings);
    const auto end_iter = std::end(strings);
    if (unlikely(iter == end_iter)) {
        return result;
    }

    T *write_ptr = result.data();
    {
        const auto &elem = *iter;
        std::char_traits<T>::copy(write_ptr, elem.data(), elem.size());
        write_ptr += elem.size();
    }

    for (++iter; iter != end_iter; ++iter) {
        const size_t sep_size = sep.size();
        CONFIG_ASSUME_STATEMENT(sep_size >= 2);
        std::char_traits<T>::copy(write_ptr, sep.data(), sep_size);
        write_ptr += sep_size;
        const auto &elem = *iter;
        std::char_traits<T>::copy(write_ptr, elem.data(), elem.size());
        write_ptr += elem.size();
    }

    return result;
}

template <misc::Char T, std::ranges::forward_range Container>
[[nodiscard]] std::basic_string<T> JoinStringsCollectionBySv(const std::basic_string_view<T> sep,
                                                             const Container &strings) {
    switch (sep.size()) {
        case 0: {
            return join_strings_detail::JoinStringsCollectionWithEmptySep<T, Container>(strings);
        }
        case 1: {
            return join_strings_detail::JoinStringsCollectionByChar<T, Container>(sep.front(),
                                                                                  strings);
        }
        default: {
            return join_strings_detail::JoinStringsCollectionBySvAtLeast2<T>(sep, strings);
        }
    }
}

}  // namespace join_strings_detail

template <misc::CharOrStringLike Sep, std::ranges::forward_range Container>
auto join_strings_collection(const Sep &sep, const Container &strings) {
    using StringType = std::ranges::range_value_t<Container>;

    static_assert(misc::is_basic_string_v<StringType> || misc::is_basic_string_view_v<StringType>,
                  "strings should be container of std::basic_string or std::basic_string_view");

    using CharType = typename StringType::value_type;
    static_assert(misc::is_char_v<CharType>, "char type is expected");

    if constexpr (misc::is_char_v<Sep>) {
        static_assert(std::is_same_v<Sep, CharType>,
                      "char type of the separator and char type of the strings should be the same");
        return join_strings_detail::JoinStringsCollectionByChar<CharType, Container>(sep, strings);
    } else {
        using SepCharType = typename join_strings_detail::string_char_t<Sep>;

        static_assert(std::is_same_v<SepCharType, CharType>,
                      "char type of the separator and char type of the strings should be the same");

        return join_strings_detail::JoinStringsCollectionBySv<CharType, Container>(
            std::basic_string_view<CharType>{sep}, strings);
    }
}

template <std::ranges::forward_range Container>
auto join_strings_collection(const Container &strings) {
    using StringType = std::ranges::range_value_t<Container>;

    static_assert(misc::is_basic_string_v<StringType> || misc::is_basic_string_view_v<StringType>,
                  "strings should be container of std::basic_string or std::basic_string_view");

    using CharType = typename StringType::value_type;
    static_assert(misc::is_char_v<CharType>, "char type is expected");

    return join_strings_detail::JoinStringsCollectionWithEmptySep<CharType, Container>(strings);
}

#endif

namespace detail {

[[nodiscard]] constexpr bool IsWhitespaceUTF32(const char32_t c) noexcept {
    switch (c) {
        case U'\u0009':
        case U'\u000A':
        case U'\u000B':
        case U'\u000C':
        case U'\u000D':
        case U'\u0020':
        case U'\u0085':
        case U'\u00A0':
        case U'\u1680':
        case U'\u2000':
        case U'\u2001':
        case U'\u2002':
        case U'\u2003':
        case U'\u2004':
        case U'\u2005':
        case U'\u2006':
        case U'\u2007':
        case U'\u2008':
        case U'\u2009':
        case U'\u200A':
        case U'\u2028':
        case U'\u2029':
        case U'\u202F':
        case U'\u205F':
        case U'\u3000': {
            return true;
        }
        default: {
            return false;
        }
    }
}

}  // namespace detail

template <class CharType>
inline bool is_whitespace(const CharType c) noexcept {
    if constexpr (std::is_same_v<CharType, char>) {
        return static_cast<bool>(std::isspace(static_cast<unsigned char>(c)));
    } else if constexpr (std::is_same_v<CharType, wchar_t>) {
        return static_cast<bool>(std::iswspace(static_cast<wint_t>(c)));
    } else {
        static_assert(
            std::is_same_v<CharType, char16_t> || std::is_same_v<CharType, char32_t>,
            "char types other than char, wchar_t, char16_t and char32_t are not supported");
        return detail::IsWhitespaceUTF32(static_cast<char32_t>(c));
    }
}

template <class CharType>
inline bool is_alpha(const CharType c) noexcept {
    if constexpr (std::is_same_v<CharType, char>) {
        return static_cast<bool>(std::isalpha(static_cast<unsigned char>(c)));
    } else {
        static_assert(std::is_same_v<CharType, wchar_t>,
                      "char types other than char and wchar_t are not supported");
        return static_cast<bool>(std::iswalpha(static_cast<wint_t>(c)));
    }
}

template <class CharType>
inline bool is_alpha_digit(const CharType c) noexcept {
    if constexpr (std::is_same_v<CharType, char>) {
        return static_cast<bool>(std::isalnum(static_cast<unsigned char>(c)));
    } else {
        static_assert(std::is_same_v<CharType, wchar_t>,
                      "char types other than char and wchar_t are not supported");
        return static_cast<bool>(std::iswalnum(static_cast<wint_t>(c)));
    }
}

template <class CharType>
inline bool is_digit(const CharType c) noexcept {
    if constexpr (std::is_same_v<CharType, char>) {
        return static_cast<bool>(std::isdigit(static_cast<unsigned char>(c)));
    } else {
        static_assert(std::is_same_v<CharType, wchar_t>,
                      "char types other than char and wchar_t are not supported");
        return static_cast<bool>(std::iswdigit(static_cast<wint_t>(c)));
    }
}

template <class CharType>
inline bool is_hex_digit(const CharType c) noexcept {
    if constexpr (std::is_same_v<CharType, char>) {
        return static_cast<bool>(std::isxdigit(static_cast<unsigned char>(c)));
    } else {
        static_assert(std::is_same_v<CharType, wchar_t>,
                      "char types other than char and wchar_t are not supported");
        return static_cast<bool>(std::iswxdigit(static_cast<wint_t>(c)));
    }
}

namespace detail {

template <class CharType>
[[nodiscard]] CharType to_lower(const CharType c) noexcept {
    if constexpr (std::is_same_v<CharType, char>) {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    } else {
        static_assert(std::is_same_v<CharType, wchar_t>,
                      "char types other than char and wchar_t are not supported");
        return static_cast<CharType>(std::towlower(static_cast<wint_t>(c)));
    }
}

template <class CharType>
[[nodiscard]] CharType to_upper(const CharType c) noexcept {
    if constexpr (std::is_same_v<CharType, char>) {
        return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    } else {
        static_assert(std::is_same_v<CharType, wchar_t>,
                      "char types other than char and wchar_t are not supported");
        return static_cast<CharType>(std::towupper(static_cast<wint_t>(c)));
    }
}

// clang-format off
template <class CharType, class Predicate>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE
inline std::basic_string_view<CharType> trim_if(std::basic_string_view<CharType> str, Predicate pred) noexcept(std::is_nothrow_invocable_v<Predicate, CharType>) {
    // clang-format on
    static_assert(std::is_invocable_r_v<bool, Predicate, CharType>,
                  "predicate should accept CharType and return bool");

#if CONFIG_HAS_CONCEPTS
    static_assert(std::predicate<Predicate, CharType>);
#endif

    while (!str.empty() && pred(str.front())) {
        str.remove_prefix(1);
    }
    while (!str.empty() && pred(str.back())) {
        str.remove_suffix(1);
    }

    return str;
}

// clang-format off
template <class CharType>
[[nodiscard]]
constexpr std::basic_string_view<CharType> TrimChar(const std::basic_string_view<CharType> str, const CharType trim_char) noexcept {
    // clang-format on
    return detail::trim_if(
        str, [trim_char](const CharType c) constexpr noexcept -> bool { return c == trim_char; });
}

// clang-format off
template <class CharType>
[[nodiscard]]
std::basic_string_view<CharType> TrimCharsImpl(const std::basic_string_view<CharType> str,
                                               const std::basic_string_view<CharType> trim_chars) noexcept {
    // clang-format on
    using UType = std::make_unsigned_t<CharType>;
    static constexpr size_t kMaxUTypeValue = static_cast<size_t>(std::numeric_limits<UType>::max());
    static constexpr bool kUseArrayMap = kMaxUTypeValue <= std::numeric_limits<uint16_t>::max();

    using MapType = std::conditional_t<kUseArrayMap, std::array<bool, kMaxUTypeValue + 1>,
                                       std::unordered_set<UType>>;
    MapType trim_chars_map{};
    for (const CharType c : trim_chars) {
        const auto key = static_cast<UType>(c);
        if constexpr (kUseArrayMap) {
            trim_chars_map[key] = true;
        } else {
            trim_chars_map.insert(key);
        }
    }

    return detail::trim_if(
        str, [&trim_chars_map = std::as_const(trim_chars_map)](const CharType c) noexcept -> bool {
            const auto key = static_cast<UType>(c);
            if constexpr (kUseArrayMap) {
                return trim_chars_map[key];
            } else {
                return trim_chars_map.find(key) != trim_chars_map.end();
            }
        });
}

// clang-format off
template <class CharType>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE
inline std::basic_string_view<CharType> TrimChars(const std::basic_string_view<CharType> str,
                                                  const std::basic_string_view<CharType> trim_chars) noexcept {
    // clang-format on
    const size_t trim_size = trim_chars.size();
    if (config::is_constant_evaluated() || config::is_gcc_constant_p(trim_size)) {
        if (trim_size == 0) {
            return str;
        }

        if (trim_size == 1) {
            return detail::TrimChar<CharType>(str, trim_chars.front());
        }
    }

    return detail::TrimCharsImpl<CharType>(str, trim_chars);
}

template <class T>
struct str_char {
    using type = void;
};

template <class T>
struct str_char<const T *> {
    using type = T;
};

template <class T>
struct str_char<T *> {
    using type = T;
};

template <class T>
struct str_char<const T[]> {
    using type = T;
};

template <class T>
struct str_char<T[]> {
    using type = T;
};

template <class T, size_t N>
struct str_char<const T[N]> {
    using type = T;
};

template <class T, size_t N>
struct str_char<T[N]> {
    using type = T;
};

template <class T>
struct str_char<std::basic_string<T>> {
    using type = T;
};

template <class T>
struct str_char<std::basic_string_view<T>> {
    using type = T;
};

template <class T>
using str_char_t = typename str_char<std::remove_cv_t<std::remove_reference_t<T>>>::type;

}  // namespace detail

template <class StrType, class TrimStrType>
inline auto trim(const StrType &str, const TrimStrType &trim_chars) noexcept {
    if constexpr (std::is_base_of_v<trim_tag, TrimStrType>) {
        using CharType = join_strings_detail::determine_char_t<StrType>;
        static_assert(misc::is_char_v<CharType>, "string is expected in the trim with tag");

        const std::basic_string_view<CharType> str_sv{str};

        if constexpr (std::is_same_v<TrimStrType, whitespace_tag>) {
            return detail::trim_if(str_sv, [](const CharType c) constexpr noexcept {
                return misc::is_whitespace<CharType>(c);
            });
        } else if constexpr (std::is_same_v<TrimStrType, alpha_tag>) {
            return detail::trim_if(str_sv, misc::is_alpha<CharType>);
        } else if constexpr (std::is_same_v<TrimStrType, digit_tag>) {
            return detail::trim_if(str_sv, misc::is_digit<CharType>);
        } else if constexpr (std::is_same_v<TrimStrType, alpha_digit_tag>) {
            return detail::trim_if(str_sv, misc::is_alpha_digit<CharType>);
        } else if constexpr (std::is_same_v<TrimStrType, hex_digit_tag>) {
            return detail::trim_if(str_sv, misc::is_hex_digit<CharType>);
        } else {
            static_assert([]() constexpr { return false; }, "implementation error");
            return str;
        }
    } else {
        using CharType = join_strings_detail::determine_char_t<StrType, TrimStrType>;
        static_assert(misc::is_char_v<CharType>,
                      "strings with the same char type are expected in the trim");

        return detail::TrimChars(std::basic_string_view<CharType>{str},
                                 std::basic_string_view<CharType>{trim_chars});
    }
}

template <class CharType>
inline bool is_whitespace(const std::basic_string_view<CharType> str) noexcept {
    for (const CharType c : str) {
        if (!misc::is_whitespace(c)) {
            return false;
        }
    }

    return true;
}

template <class CharType>
inline bool is_whitespace(const std::basic_string<CharType> &str) noexcept {
    return misc::is_whitespace(std::basic_string_view<CharType>{str});
}

template <class CharType>
inline bool is_whitespace(const CharType *const str) noexcept {
    return misc::is_whitespace(std::basic_string_view<CharType>{str});
}

template <class CharType>
inline void to_lower_inplace(CharType *const str, const size_t n) noexcept {
    for (size_t i = 0; i < n; i++) {
        str[i] = detail::to_lower(str[i]);
    }
}

template <class CharType>
inline void to_lower_inplace(std::basic_string<CharType> &str) noexcept {
    misc::to_lower_inplace(str.data(), str.size());
}

template <class CharType>
inline std::basic_string<CharType> to_lower(const std::basic_string_view<CharType> str) {
    std::basic_string<CharType> ret{str};
    misc::to_lower_inplace(ret);
    return ret;
}

template <class CharType>
inline std::basic_string<CharType> to_lower(const std::basic_string<CharType> &str) {
    return misc::to_lower(std::basic_string_view<CharType>{str});
}

template <class CharType>
inline std::basic_string<CharType> to_lower(const CharType *const str) {
    return misc::to_lower(std::basic_string_view<CharType>{str});
}

template <class CharType>
inline void to_upper_inplace(CharType *const str, const size_t n) noexcept {
    for (size_t i = 0; i < n; i++) {
        str[i] = detail::to_upper(str[i]);
    }
}

template <class CharType>
inline void to_upper_inplace(std::basic_string<CharType> &str) noexcept {
    misc::to_upper_inplace(str.data(), str.size());
}

template <class CharType>
inline std::basic_string<CharType> to_upper(const std::basic_string_view<CharType> str) {
    std::basic_string<CharType> ret{str};
    misc::to_upper_inplace(ret);
    return ret;
}

template <class CharType>
inline std::basic_string<CharType> to_upper(const std::basic_string<CharType> &str) {
    return misc::to_upper(std::basic_string_view<CharType>{str});
}

template <class CharType>
inline std::basic_string<CharType> to_upper(const CharType *const str) {
    return misc::to_upper(std::basic_string_view<CharType>{str});
}

}  // namespace misc
