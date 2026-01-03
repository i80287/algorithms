#ifndef JOIN_STRINGS_INCLUDING_IMPLEMENTATION
// cppcheck-suppress [preprocessorErrorDirective]
#error This header should not be used directly
#endif

#include <algorithm>
#include <cassert>
#include <cctype>
#include <charconv>
#include <codecvt>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cuchar>
#include <cwchar>
#include <cwctype>
#include <filesystem>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>

#include "config_macros.hpp"
#include "string_traits.hpp"

#if CONFIG_HAS_INCLUDE("../number_theory/integers_128_bit.hpp")
#include "../number_theory/integers_128_bit.hpp"
#endif

namespace misc {

using std::size_t;

namespace join_strings_detail {

template <class T>
inline constexpr bool is_filesystem_path_v = std::is_same_v<T, std::filesystem::path>;

template <misc::Char CharType, class T>
ATTRIBUTE_ALWAYS_INLINE [[nodiscard]]
inline std::basic_string<CharType> ArithmeticToStringImpl(const T arg) {
    if constexpr (std::is_integral_v<T> && sizeof(T) > sizeof(int)) {
        if (config::is_constant_evaluated() || config::is_gcc_constant_p(arg)) {
            using CompressedIntType = std::conditional_t<std::is_unsigned_v<T>, unsigned, int>;

            if (arg >= std::numeric_limits<CompressedIntType>::min() &&
                arg <= std::numeric_limits<CompressedIntType>::max()) {
                return join_strings_detail::ArithmeticToStringImpl<CharType>(
                    static_cast<CompressedIntType>(arg));
            }
        }
    }

    constexpr bool kShortIntegralType = std::is_integral_v<T> && sizeof(T) < sizeof(int);
    using ToStringableType =
        std::conditional_t<kShortIntegralType,
                           std::conditional_t<std::is_unsigned_v<T>, unsigned, int>, T>;

    const ToStringableType extended_arg = static_cast<ToStringableType>(arg);

    using namespace std;
    if constexpr (std::is_same_v<CharType, wchar_t>) {
        return to_wstring(extended_arg);
    } else {
        return to_string(extended_arg);
    }
}

#define JSTR_STRINGIFY_IMPL(expr) #expr
#define JSTR_STRINGIFY(expr)      JSTR_STRINGIFY_IMPL(expr)
#define JSTR_FILE_LOCATION_STR()  __FILE__ ":" JSTR_STRINGIFY(__LINE__)

ATTRIBUTE_COLD
[[noreturn]]
inline void ThrowOnNegativeMaxLengthDuringConversionToNotUTF8(int32_t max_char_conv_len,
                                                              std::string_view file_location,
                                                              std::string_view function_name);

ATTRIBUTE_COLD
[[noreturn]]
inline void ThrowOnTooLongStringDuringConversionToNotUTF8(size_t input_str_size,
                                                          size_t max_char_conv_len,
                                                          std::string_view file_location,
                                                          std::string_view function_name);

#if CONFIG_COMPILER_IS_ANY_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated"
#elif CONFIG_COMPILER_IS_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif CONFIG_COMPILER_IS_MSVC
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

/// @brief See libstdc++ __do_str_codecvt
template <misc::Char OutCharType, misc::Char InCharType, class CodecvtState = std::mbstate_t>
[[nodiscard]]
bool DoStrCodecvt(const std::basic_string_view<InCharType> in_str,
                  std::basic_string<OutCharType> &out_str,
                  const std::codecvt<OutCharType, InCharType, CodecvtState> &cvt,
                  size_t &count) {
    const InCharType *const first = in_str.data();
    const InCharType *const last = first + in_str.size();

    if (unlikely(first == last)) {
        out_str.clear();
        count = 0;
        return true;
    }

    size_t outchars = 0;
    const InCharType *next = first;
    const int maxlen_signed = cvt.max_length() + 1;
    // __do_str_codecvt doesn't check it but this function does just in case
    if (unlikely(maxlen_signed <= 0)) {
        ThrowOnNegativeMaxLengthDuringConversionToNotUTF8(maxlen_signed, JSTR_FILE_LOCATION_STR(),
                                                          CONFIG_CURRENT_FUNCTION_NAME);
    }
    const auto maxlen = size_t{static_cast<unsigned>(maxlen_signed)};

    using std::codecvt_base;

    CodecvtState state{};
    codecvt_base::result result;
    do {
        const auto bytes_to_convert = static_cast<size_t>(last - next);
#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG
        size_t max_bytes_to_convert = 0;
        bool overflowed = false;
        overflowed |= __builtin_mul_overflow(bytes_to_convert, maxlen, &max_bytes_to_convert);
        size_t out_str_new_conv_max_size = 0;
        overflowed |= __builtin_add_overflow(out_str.size(), max_bytes_to_convert,
                                             &out_str_new_conv_max_size);
        if (unlikely(overflowed)) {
            ThrowOnTooLongStringDuringConversionToNotUTF8(
                in_str.size(), maxlen, JSTR_FILE_LOCATION_STR(), CONFIG_CURRENT_FUNCTION_NAME);
        }
#else
        const size_t max_bytes_to_convert = bytes_to_convert * maxlen;
        const size_t out_str_new_conv_max_size = out_str.size() + max_bytes_to_convert;
#endif
        out_str.resize(out_str_new_conv_max_size);
        OutCharType *outnext = &out_str.front() + outchars;
        OutCharType *const outlast = &out_str.back() + 1;
        result = cvt.in(state, next, last, next, outnext, outlast, outnext);
        outchars = static_cast<size_t>(outnext - &out_str.front());
    } while (result == codecvt_base::partial && next != last &&
             static_cast<std::ptrdiff_t>(out_str.size() - outchars) < maxlen_signed);

    if (result == codecvt_base::error) {
        count = static_cast<size_t>(next - first);
        return false;
    }

    // The codecvt facet will only return noconv when the types are
    // the same, so avoid instantiating basic_string::assign otherwise
    if constexpr (std::is_same_v<OutCharType, InCharType>) {
        if (result == codecvt_base::noconv) {
            out_str.assign(first, last);
            count = static_cast<size_t>(last - first);
            return true;
        }
    }

    out_str.resize(outchars);
    count = static_cast<size_t>(next - first);
    return true;
}

ATTRIBUTE_COLD
[[noreturn]]
inline void ThrowOnFailedConversionToNotUTF8(bool conversion_succeeded,
                                             size_t converted_bytes_count,
                                             size_t string_size,
                                             std::string_view file_location,
                                             std::string_view function_name);

// utility wrapper to adapt locale-bound facets
template <class Facet>
class deletable_facet final : public Facet {
private:
    using Base = Facet;

public:
    using Base::Base;

    ~deletable_facet() = default;
};

template <misc::Char ToCharType>
    requires(!std::is_same_v<ToCharType, char>)
[[nodiscard]]
inline std::basic_string<ToCharType> ConvertBytesToNotUTF8(const std::string_view str) {
    if constexpr (false
#if defined(CONFIG_HAS_AT_LEAST_CXX_26) && CONFIG_HAS_AT_LEAST_CXX_26
                  || true
#endif
    ) {
        const deletable_facet<std::codecvt<ToCharType, char, std::mbstate_t>> cvt;
        std::basic_string<ToCharType> out_str{};
        size_t converted_bytes_count{};
        const bool conversion_succeeded = DoStrCodecvt(str, out_str, cvt, converted_bytes_count);
        if (likely(conversion_succeeded && converted_bytes_count == str.size())) {
            return out_str;
        }

        join_strings_detail::ThrowOnFailedConversionToNotUTF8(
            conversion_succeeded, converted_bytes_count, str.size(), JSTR_FILE_LOCATION_STR(),
            CONFIG_CURRENT_FUNCTION_NAME);
    } else {
        return std::wstring_convert<std::codecvt_utf8_utf16<ToCharType>, ToCharType>{}.from_bytes(
            str.data(), str.data() + str.size());
    }
}

#if CONFIG_COMPILER_IS_ANY_CLANG
#pragma clang diagnostic pop
#elif CONFIG_COMPILER_IS_GCC
#pragma GCC diagnostic pop
#elif CONFIG_COMPILER_IS_MSVC
#pragma warning(pop)
#endif

ATTRIBUTE_COLD
[[noreturn]]
inline void ThrowOnFailedConversionToUTF8(std::string_view file_location,
                                          std::string_view function_name);

[[nodiscard]] inline std::u8string ConvertBytesToUTF8(const std::string_view bytes) {
    const auto is_ascii_nonzero_char = [](const char chr) constexpr noexcept {
        const uint32_t u32_chr = static_cast<unsigned char>(chr);
        constexpr uint32_t kMinChar = 1;
        constexpr uint32_t kMaxAsciiCharCode = 127;
        return u32_chr - kMinChar <= kMaxAsciiCharCode - kMinChar;
    };

    static_assert(sizeof(char) == sizeof(char8_t));
    static_assert(alignof(char) == alignof(char8_t));
    if (likely(std::all_of(bytes.begin(), bytes.end(), is_ascii_nonzero_char))) {
        return std::u8string(reinterpret_cast<const char8_t *>(bytes.data()), bytes.size());
    }

    join_strings_detail::ThrowOnFailedConversionToUTF8(JSTR_FILE_LOCATION_STR(),
                                                       CONFIG_CURRENT_FUNCTION_NAME);
}

template <misc::Char ToCharType>
[[nodiscard]] inline std::basic_string<ToCharType> ConvertBytesTo(const std::string_view str) {
    if constexpr (std::is_same_v<ToCharType, char8_t>) {
        return ConvertBytesToUTF8(str);
    } else {
        return ConvertBytesToNotUTF8<ToCharType>(str);
    }
}

template <misc::Char CharType, class T>
    requires(!misc::is_char_v<T>)
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE inline std::basic_string<CharType> ArithmeticToString(const T arg) {
    using FmtChar = std::conditional_t<std::is_same_v<CharType, wchar_t>, wchar_t, char>;
    // For some reasons clang can't use nrvo if ArithmeticToStringImpl<FmtChar>(arg) called before
    // `if constexpr`
    if constexpr (std::is_same_v<FmtChar, CharType>) {
        return ArithmeticToStringImpl<FmtChar>(arg);
    } else {
        return ConvertBytesTo<CharType>(ArithmeticToStringImpl<FmtChar>(arg));
    }
}

template <misc::Char CharType, class T>
    requires std::is_enum_v<T>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE inline std::basic_string<CharType> EnumToString(const T arg) {
    if constexpr (std::is_error_code_enum_v<T>) {
        if constexpr (std::is_same_v<CharType, char>) {
            return std::make_error_condition(arg).message();
        } else {
            return ConvertBytesTo<CharType>(std::make_error_condition(arg).message());
        }
    } else if constexpr (std::is_error_condition_enum_v<T>) {
        if constexpr (std::is_same_v<CharType, char>) {
            return std::make_error_condition(arg).message();
        } else {
            return ConvertBytesTo<CharType>(std::make_error_condition(arg).message());
        }
    } else {
        return ArithmeticToString<CharType>(static_cast<std::underlying_type_t<T>>(arg));
    }
}

#define JS_PP_RETURN_STR_LITERAL(CharType, StrLiteral)         \
    if constexpr (std::is_same_v<CharType, char>) {            \
        return StrLiteral;                                     \
    } else if constexpr (std::is_same_v<CharType, wchar_t>) {  \
        return L##StrLiteral;                                  \
    } else if constexpr (std::is_same_v<CharType, char8_t>) {  \
        return u8##StrLiteral;                                 \
    } else if constexpr (std::is_same_v<CharType, char16_t>) { \
        return u##StrLiteral;                                  \
    } else if constexpr (std::is_same_v<CharType, char32_t>) { \
        return U##StrLiteral;                                  \
    } else                                                     \
        static_assert([]() { return false; }())

#define JS_PP_STR_LITERAL(CharType, StrLiteral)                   \
    []<class = CharType>() constexpr noexcept -> decltype(auto) { \
        JS_PP_RETURN_STR_LITERAL(CharType, StrLiteral);           \
    }()

template <misc::Char CharType>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE inline std::basic_string<CharType> NullPtrToString() {
    JS_PP_RETURN_STR_LITERAL(CharType, "null");
}

#undef JS_PP_STR_LITERAL
#undef JS_PP_RETURN_STR_LITERAL

template <misc::Char CharType, class T>
    requires(std::is_pointer_v<T> || std::is_member_pointer_v<T> || std::is_null_pointer_v<T>)
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE inline std::basic_string<CharType> PointerTypeToString(const T arg) {
    const auto ptr_num = reinterpret_cast<std::uintptr_t>(arg);
    if constexpr (std::is_null_pointer_v<T>) {
        return NullPtrToString<CharType>();
    } else if (ptr_num == 0) {
        return NullPtrToString<CharType>();
    } else {
        return ArithmeticToString<CharType>(ptr_num);
    }
}

template <misc::Char CharType, class T>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE inline std::basic_string<CharType> ToStringScalarArg(const T arg) {
    if constexpr (std::is_enum_v<T>) {
        return EnumToString<CharType>(arg);
    } else if constexpr (std::is_same_v<T, CharType>) {
        return std::basic_string<CharType>(size_t{1}, arg);
    } else if constexpr (std::is_pointer_v<T> || std::is_member_pointer_v<T> ||
                         std::is_null_pointer_v<T>) {
        return PointerTypeToString<CharType>(arg);
    } else {
        return ArithmeticToString<CharType>(arg);
    }
}

// clang-format off
template <misc::Char CharType>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE
inline std::basic_string<CharType> FilesystemPathToString(const std::filesystem::path& path) {
    // clang-format on
    return path.template generic_string<CharType>();
}

template <class T, class CharType>
concept WriteableViaBasicOStringStream =
    requires(const T &arg, std::basic_ostringstream<CharType> &oss) {
        { oss << arg };
    };

template <class T>
concept WriteableViaOStringStream = WriteableViaBasicOStringStream<T, char>;

template <class T, class CharType>
concept DirectlyConvertableToBasicString =
    std::constructible_from<std::basic_string<CharType>, const T &>;

template <class T>
concept DirectlyConvertableToString = DirectlyConvertableToBasicString<T, char>;

template <misc::Char CharType, class T>
[[nodiscard]] inline std::basic_string<CharType> ToStringOneArgViaOStringStream(const T &arg) {
    std::basic_ostringstream<CharType> oss;
    // not std::ignore because user defined operator<< may return void
    static_cast<void>(oss << arg);
    return std::move(oss).str();
}

template <misc::Char CharType, class T>
ATTRIBUTE_ALWAYS_INLINE [[nodiscard]]
inline std::basic_string<CharType> ToStringOneArg(const T &arg) {
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
        if constexpr (std::is_same_v<CharType, char>) {
            return to_string(arg);
        } else {
            return ConvertBytesTo<CharType>(to_string(arg));
        }
    } else if constexpr (requires(const T &test_arg) {
                             { test_arg.to_string() } -> std::same_as<std::string>;
                         }) {
        if constexpr (std::is_same_v<CharType, char>) {
            return arg.to_string();
        } else {
            return ConvertBytesTo<CharType>(arg.to_string());
        }
    } else if constexpr (requires(const T &test_arg) {
                             { to_string_view(test_arg) } -> std::same_as<std::string_view>;
                         }) {
        if constexpr (std::is_same_v<CharType, char>) {
            return std::string{to_string_view(arg)};
        } else {
            return ConvertBytesTo<CharType>(to_string_view(arg));
        }
    } else if constexpr (requires(const T &test_arg) {
                             { test_arg.to_string_view() } -> std::same_as<std::string_view>;
                         }) {
        if constexpr (std::is_same_v<CharType, char>) {
            return std::string{arg.to_string_view()};
        } else {
            return ConvertBytesTo<CharType>(arg.to_string_view());
        }
    } else if constexpr (std::is_scalar_v<T>
#ifdef HAS_INT128_TYPEDEF
                         || std::is_same_v<T, int128_t> || std::is_same_v<T, uint128_t>
#endif
    ) {
        return ToStringScalarArg<CharType>(arg);
    } else if constexpr (is_filesystem_path_v<T>) {
        return FilesystemPathToString<CharType>(arg);
    } else if constexpr (DirectlyConvertableToBasicString<T, CharType>) {
        return std::basic_string<CharType>{arg};
    } else if constexpr (WriteableViaBasicOStringStream<T, CharType>) {
        return ToStringOneArgViaOStringStream<CharType>(arg);
    } else if constexpr (DirectlyConvertableToString<T>) {
        return ConvertBytesTo<CharType>(std::string{arg});
    } else if constexpr (WriteableViaOStringStream<T>) {
        return ConvertBytesTo<CharType>(ToStringOneArgViaOStringStream<char>(arg));
    } else {
        return std::basic_string<CharType>{arg};
    }
}

// clang-format off

template <misc::Char CharType, class... Args>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE
constexpr size_t CalculateStringArgsSize(CharType chr, Args... args) noexcept;

template <misc::Char CharType, class... Args>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE
constexpr size_t CalculateStringArgsSize(std::basic_string_view<CharType> s, Args... args) noexcept;

// clang-format on

template <misc::Char CharType, class... Args>
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

template <misc::Char CharType, class... Args>
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

template <misc::Char CharType, class... Args>
ATTRIBUTE_NONNULL_ALL_ARGS
ATTRIBUTE_ACCESS(write_only, 1)
ATTRIBUTE_ALWAYS_INLINE
constexpr void WriteStringsInplace(CharType* result, CharType c, Args... args) noexcept;

template <misc::Char CharType, class... Args>
ATTRIBUTE_NONNULL_ALL_ARGS
ATTRIBUTE_ACCESS(write_only, 1)
ATTRIBUTE_ALWAYS_INLINE
constexpr void WriteStringsInplace(CharType* result, std::basic_string_view<CharType> s, Args... args) noexcept;

// clang-format on

template <misc::Char CharType, class... Args>
constexpr void WriteStringsInplace(CharType *const result,
                                   const CharType c,
                                   Args... args) noexcept {
    *result = c;
    if constexpr (sizeof...(args) > 0) {
        join_strings_detail::WriteStringsInplace<CharType>(result + 1, args...);
    }
}

template <misc::Char CharType, class... Args>
constexpr void WriteStringsInplace(CharType *const result,
                                   const std::basic_string_view<CharType> s,
                                   Args... args) noexcept {
    std::char_traits<CharType>::copy(result, s.data(), s.size());
    if constexpr (sizeof...(args) > 0) {
        join_strings_detail::WriteStringsInplace<CharType>(result + s.size(), args...);
    }
}

// clang-format off

template <misc::Char CharType, class... Args>
ATTRIBUTE_NONNULL_ALL_ARGS
ATTRIBUTE_SIZED_ACCESS(write_only, 1, 2)
ATTRIBUTE_ALWAYS_INLINE
constexpr void WriteStringToBuffer(CharType* const buffer, size_t /*buffer_size*/, Args... args) noexcept {
    join_strings_detail::WriteStringsInplace<CharType>(buffer, args...);
}

// clang-format on

template <misc::Char CharType, class... Args>
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

template <misc::Char CharType, size_t I, class... Args>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE
inline std::basic_string<CharType> JoinStringsConvArgsToStrViewImpl(std::basic_string_view<CharType> str, const Args&... args);

template <misc::Char CharType, size_t I, class T, class... Args>
    requires (!is_string_like_v<T>)
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE
inline std::basic_string<CharType> JoinStringsConvArgsToStrViewImpl(const T& value, const Args&... args);

// clang-format on

template <misc::Char CharType, size_t I, class... Args>
inline std::basic_string<CharType> JoinStringsConvArgsToStrViewImpl(
    const std::basic_string_view<CharType> str, const Args &...args) {
    if constexpr (I == 1 + sizeof...(args)) {
        return join_strings_detail::JoinStringsImpl<CharType>(str, args...);
    } else {
        return join_strings_detail::JoinStringsConvArgsToStrViewImpl<CharType, I + 1>(args..., str);
    }
}

template <misc::Char CharType, size_t I, class T, class... Args>
    requires(!is_string_like_v<T>)
inline std::basic_string<CharType> JoinStringsConvArgsToStrViewImpl(const T &value,
                                                                    const Args &...args) {
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
}  // namespace join_strings_detail

// clang-format off

template <misc::Char HintCharType, class... Args>
inline auto join_strings(const Args&... args) {
    static_assert(sizeof...(args) >= 1, "Empty input is explicitly prohibited");

    using DeducedCharType = misc::string_detail::determine_char_t<Args...>;

    using CharType = std::conditional_t<misc::is_char_v<DeducedCharType>, DeducedCharType, HintCharType>;

    constexpr bool kAllCharTypesAreSame = std::conjunction_v<misc::string_detail::same_char_types<Args, CharType>...>;
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

// clang-format on

namespace join_strings_detail {

inline void ThrowOnNegativeMaxLengthDuringConversionToNotUTF8(
    const int32_t max_char_conv_len,
    const std::string_view file_location,
    const std::string_view function_name) {
    throw std::runtime_error{misc::join_strings("codecvt::max_length() returned negative value ",
                                                max_char_conv_len, " at ", file_location, ' ',
                                                function_name)};
}

inline void ThrowOnTooLongStringDuringConversionToNotUTF8(const size_t input_str_size,
                                                          const size_t max_char_conv_len,
                                                          const std::string_view file_location,
                                                          const std::string_view function_name

) {
    throw std::runtime_error{misc::join_strings(
        "DoStrCodecvt(): size of the converted string is too large. Input string size: ",
        input_str_size, ", max size of the converted character: ", max_char_conv_len, " at ",
        file_location, ' ', function_name)};
}

inline void ThrowOnFailedConversionToNotUTF8(const bool conversion_succeeded,
                                             const size_t converted_bytes_count,
                                             const size_t string_size,
                                             const std::string_view file_location,
                                             const std::string_view function_name) {
    using namespace std::string_view_literals;

    const std::string_view error_str =
        !conversion_succeeded ? "Could not convert"sv : "Only partially converted"sv;
    throw std::runtime_error{misc::join_strings(
        error_str, " multibyte string type to another string type (total bytes converted: "sv,
        converted_bytes_count, " of "sv, string_size, ") at "sv, file_location, ' ',
        function_name)};
}

inline void ThrowOnFailedConversionToUTF8(const std::string_view file_location,
                                          const std::string_view function_name) {
    using namespace std::string_view_literals;

    throw std::runtime_error{
        misc::join_strings("Unsupported conversion from multibyte string type to utf-8 string type "
                           "(only ascii to utf-8 is supported) at "sv,
                           file_location, ':', function_name)};
}

#undef JSTR_FILE_LOCATION_STR
#undef JSTR_STRINGIFY
#undef JSTR_STRINGIFY_IMPL

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
        "join_strings_range(): total strings length exceeded max size_t value";
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
[[nodiscard]] std::basic_string<T> JoinStringsRangeWithEmptySep(const Container &strings) {
    const size_t total_size = join_strings_detail::StringsTotalSize(strings);
    std::basic_string<T> result(total_size, '\0');
    T *write_ptr = result.data();
    for (const std::basic_string_view<T> elem : strings) {
        std::char_traits<T>::copy(write_ptr, elem.data(), elem.size());
        write_ptr += elem.size();
    }

    return result;
}

template <misc::Char T, std::ranges::forward_range Container>
[[nodiscard]] std::basic_string<T> JoinStringsRangeByChar(const T sep, const Container &strings) {
    const size_t total_size = join_strings_detail::StringsTotalSizeWithCharSep(strings);
    std::basic_string<T> result(total_size, '\0');
    T *write_ptr = result.data();
    for (const std::basic_string_view<T> elem : strings) {
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
[[nodiscard]] std::basic_string<T> JoinStringsRangeBySvAtLeast2(
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
        const std::basic_string_view<T> elem = *iter;
        std::char_traits<T>::copy(write_ptr, elem.data(), elem.size());
        write_ptr += elem.size();
    }

    for (++iter; iter != end_iter; ++iter) {
        const size_t sep_size = sep.size();
        CONFIG_ASSUME_STATEMENT(sep_size >= 2);
        std::char_traits<T>::copy(write_ptr, sep.data(), sep_size);
        write_ptr += sep_size;
        const std::basic_string_view<T> elem = *iter;
        std::char_traits<T>::copy(write_ptr, elem.data(), elem.size());
        write_ptr += elem.size();
    }

    return result;
}

template <misc::Char T, std::ranges::forward_range Container>
[[nodiscard]] std::basic_string<T> JoinStringsRangeBySv(const std::basic_string_view<T> sep,
                                                        const Container &strings) {
    switch (sep.size()) {
        case 0: {
            return join_strings_detail::JoinStringsRangeWithEmptySep<T, Container>(strings);
        }
        case 1: {
            return join_strings_detail::JoinStringsRangeByChar<T, Container>(sep.front(), strings);
        }
        default: {
            return join_strings_detail::JoinStringsRangeBySvAtLeast2<T>(sep, strings);
        }
    }
}

}  // namespace join_strings_detail

template <misc::CharOrStringLike Sep, std::ranges::forward_range Container>
inline auto join_strings_range(const Sep &sep, const Container &strings) {
    using StringType = std::ranges::range_value_t<Container>;

    static_assert(misc::is_basic_string_v<StringType> || misc::is_basic_string_view_v<StringType>,
                  "strings should be container of std::basic_string or std::basic_string_view");

    using CharType = typename StringType::value_type;
    static_assert(misc::is_char_v<CharType>, "char type is expected");

    if constexpr (misc::is_char_v<Sep>) {
        static_assert(std::is_same_v<Sep, CharType>,
                      "char type of the separator and char type of the strings should be the same");
        return join_strings_detail::JoinStringsRangeByChar<CharType, Container>(sep, strings);
    } else {
        using SepCharType = join_strings_detail::string_char_t<Sep>;

        static_assert(std::is_same_v<SepCharType, CharType>,
                      "char type of the separator and char type of the strings should be the same");

        return join_strings_detail::JoinStringsRangeBySv<CharType, Container>(
            std::basic_string_view<CharType>{sep}, strings);
    }
}

template <std::ranges::forward_range Container>
inline auto join_strings_range(const Container &strings) {
    using StringType = std::ranges::range_value_t<Container>;

    static_assert(misc::is_basic_string_v<StringType> || misc::is_basic_string_view_v<StringType>,
                  "join_strings_range accepts only a range of std::basic_string or "
                  "std::basic_string_view");

    using CharType = typename StringType::value_type;
    static_assert(misc::is_char_v<CharType>,
                  "join_strings_range expects a range of strings (with correct char type)");

    return join_strings_detail::JoinStringsRangeWithEmptySep<CharType, Container>(strings);
}

}  // namespace misc
