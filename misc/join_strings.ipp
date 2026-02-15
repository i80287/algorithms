#ifndef JOIN_STRINGS_INCLUDING_IMPLEMENTATION
// cppcheck-suppress [preprocessorErrorDirective]
#error This header should not be used directly
#endif

#include <algorithm>
#include <bit>
#include <cassert>
#include <cctype>
#include <charconv>
#include <climits>
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
#include "ints_fmt.hpp"
#include "string_traits.hpp"

#if CONFIG_HAS_INCLUDE("../number_theory/integers_128_bit.hpp")
#include "../number_theory/integers_128_bit.hpp"
#endif

namespace misc {

using std::size_t;

namespace join_strings_detail {

template <class T>
inline constexpr bool is_filesystem_path_v = std::is_same_v<T, std::filesystem::path>;

template <class T>
inline constexpr bool is_integral_v =
#ifdef HAS_INT128_TYPEDEF
    std::is_same_v<T, int128_t> || std::is_same_v<T, uint128_t> ||
#endif
    std::is_integral_v<T>;

template <class T>
inline constexpr bool is_scalar_v = std::is_enum_v<T> || std::is_floating_point_v<T> || is_integral_v<T> ||
                                    std::is_pointer_v<T> || std::is_member_pointer_v<T> || std::is_null_pointer_v<T>;

template <class T>
inline constexpr bool is_formattable_pointer_v = std::is_same_v<T, const void *> || std::is_null_pointer_v<T>;

template <class T>
inline constexpr bool is_formattable_scalar_non_char_v =
    (std::is_enum_v<T> || std::is_floating_point_v<T> || is_integral_v<T> || is_formattable_pointer_v<T>) &&
    !is_char_v<T>;

template <typename T>
using IntegralValueToStringLikeRetType = ints_fmt::Formatter<T
#ifdef HAS_INT128_TYPEDEF
                                                             ,
                                                             int128_traits::make_unsigned_t<T>
#endif
                                                             >;

template <class T>
    requires is_integral_v<T>
[[nodiscard]] inline IntegralValueToStringLikeRetType<T> IntegralValueToStringLike(const T arg) noexcept {
    static_assert(std::is_nothrow_constructible_v<IntegralValueToStringLikeRetType<T>, T>);
    return IntegralValueToStringLikeRetType<T>{arg};
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
        overflowed |= __builtin_add_overflow(out_str.size(), max_bytes_to_convert, &out_str_new_conv_max_size);
        if (unlikely(overflowed)) {
            ThrowOnTooLongStringDuringConversionToNotUTF8(in_str.size(), maxlen, JSTR_FILE_LOCATION_STR(),
                                                          CONFIG_CURRENT_FUNCTION_NAME);
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

        join_strings_detail::ThrowOnFailedConversionToNotUTF8(conversion_succeeded, converted_bytes_count, str.size(),
                                                              JSTR_FILE_LOCATION_STR(), CONFIG_CURRENT_FUNCTION_NAME);
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
inline void ThrowOnFailedConversionToUTF8(std::string_view file_location, std::string_view function_name);

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

    join_strings_detail::ThrowOnFailedConversionToUTF8(JSTR_FILE_LOCATION_STR(), CONFIG_CURRENT_FUNCTION_NAME);
}

template <class T>
    requires std::is_enum_v<T>
[[nodiscard]] inline auto EnumToString(const T arg) {
    if constexpr (std::is_error_code_enum_v<T>) {
        return std::make_error_code(arg).message();
    } else if constexpr (std::is_error_condition_enum_v<T>) {
        return std::make_error_condition(arg).message();
    } else {
        static_assert(sizeof(T) < 0,
                      "Can't format enum value to string in join_strings() (no suitable converting function found)");
        return std::string_view{};
    }
}

class PtrStrBuffer {
private:
    static constexpr std::size_t kBitsPerHexSymbol = 4;
    static constexpr std::size_t kPtrHexValueMaxSize = sizeof(void *) * CHAR_BIT / kBitsPerHexSymbol;
    static constexpr std::string_view kPtrValueStrPrefix = "0x";
    static constexpr auto kMinFmtSize = uint32_t{kPtrValueStrPrefix.size() + 1};
    static constexpr auto kMaxCapacity = uint32_t{kPtrValueStrPrefix.size() + kPtrHexValueMaxSize};

    using storage_type = std::array<char, kMaxCapacity>;
    using size_type = std::uint8_t;
    static_assert(kMaxCapacity <= std::numeric_limits<size_type>::max());

public:
    using value_type = storage_type::value_type;

    ATTRIBUTE_ACCESS_NONE(2)
    explicit PtrStrBuffer(const void *const ptr) noexcept : storage_(), size_(write_ptr_to_buffer(ptr, storage_)) {}

    ATTRIBUTE_PURE [[nodiscard]] constexpr const char *data() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return storage_.data();
    }

    ATTRIBUTE_PURE [[nodiscard]] constexpr std::size_t size() const noexcept {
        const size_type fmt_size = size_;
        CONFIG_ASSUME_STATEMENT(kMinFmtSize <= fmt_size);
        CONFIG_ASSUME_STATEMENT(fmt_size <= kMaxCapacity);
        return fmt_size;
    }

    ATTRIBUTE_PURE [[nodiscard]] constexpr std::string_view as_string_view() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return std::string_view{data(), size()};
    }

    ATTRIBUTE_PURE
    [[nodiscard]] explicit constexpr operator std::string_view() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return as_string_view();
    }

private:
    ATTRIBUTE_ACCESS_NONE(1)
    [[nodiscard]] static size_type write_ptr_to_buffer(const void *const ptr, storage_type &storage) noexcept {
        char *write_ptr = storage.data();
        std::size_t capacity = storage.size();
        std::char_traits<char>::copy(write_ptr, kPtrValueStrPrefix.data(), kPtrValueStrPrefix.size());
        write_ptr += kPtrValueStrPrefix.size();
        capacity -= kPtrValueStrPrefix.size();
        const std::to_chars_result ret =
            std::to_chars(write_ptr, write_ptr + capacity, std::bit_cast<std::uintptr_t>(ptr), 16);
        const std::ptrdiff_t total_size = ret.ptr - storage.data();
        CONFIG_ASSUME_STATEMENT(static_cast<std::ptrdiff_t>(kMinFmtSize) <= total_size);
        CONFIG_ASSUME_STATEMENT(total_size <= static_cast<std::ptrdiff_t>(kMaxCapacity));
        return static_cast<size_type>(total_size);
    }

    storage_type storage_;
    size_type size_;
};

[[nodiscard]]
ATTRIBUTE_ACCESS_NONE(1) ATTRIBUTE_ALWAYS_INLINE inline PtrStrBuffer PointerValueToStringLike(const void *const ptr) {
    return PtrStrBuffer{ptr};
}

template <class T>
    requires is_formattable_scalar_non_char_v<T>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE inline auto ScalarValueToStringLike(const T arg) {
    if constexpr (std::is_enum_v<T>) {
        return EnumToString(arg);
    } else if constexpr (is_formattable_pointer_v<T>) {
        return PointerValueToStringLike(arg);
    } else if constexpr (std::is_floating_point_v<T>) {
        return std::to_string(arg);
    } else {
        return IntegralValueToStringLike(arg);
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
concept WriteableViaBasicOStringStream = requires(const T &arg, std::basic_ostringstream<CharType> &oss) {
    { oss << arg };
};

template <misc::Char CharType, WriteableViaBasicOStringStream<CharType> T>
[[nodiscard]] inline std::basic_string<CharType> ToStringOneArgViaOStringStream(const T &arg) {
    std::basic_ostringstream<CharType> oss;
    // not std::ignore because user defined operator<< may return void
    static_cast<void>(oss << arg);
    return std::move(oss).str();
}

template <typename T>
inline constexpr bool is_formattable_to_static_ascii_buffer = is_formattable_pointer_v<T> || is_integral_v<T>;

template <class T>
    requires(!is_char_v<T>)
ATTRIBUTE_ALWAYS_INLINE [[nodiscard]]
inline auto ValueToStringLike(const T &arg) {
    if constexpr (is_formattable_to_static_ascii_buffer<T>) {
        return ScalarValueToStringLike(arg);
    } else if constexpr (requires(const T &test_arg) {
                             { to_string(test_arg) } -> std::same_as<std::string>;
                         }) {
        return to_string(arg);
    } else if constexpr (requires(const T &test_arg) {
                             { test_arg.to_string() } -> std::same_as<std::string>;
                         }) {
        return arg.to_string();
    } else if constexpr (requires(const T &test_arg) {
                             { to_string_view(test_arg) } -> std::same_as<std::string_view>;
                         }) {
        return to_string_view(arg);
    } else if constexpr (requires(const T &test_arg) {
                             { test_arg.to_string_view() } -> std::same_as<std::string_view>;
                         }) {
        return arg.to_string_view();
    } else if constexpr (is_scalar_v<T>) {
        static_assert(is_formattable_scalar_non_char_v<T>,
                      "Can't format pointer type at join_strings() to string. Cast to const void* if pointer-like "
                      "formatting is desired");
        return ScalarValueToStringLike(arg);
    } else if constexpr (WriteableViaBasicOStringStream<T, char>) {
        return ToStringOneArgViaOStringStream<char>(arg);
    } else {
        static_assert(sizeof(T) < 0, "Can't convert argument in the join_strings() to string");
        return std::string_view{};
    }
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
ATTRIBUTE_ALWAYS_INLINE [[nodiscard]]
inline auto ValueToGenericStringLike(const T &arg ATTRIBUTE_LIFETIME_BOUND) {
    if constexpr (is_filesystem_path_v<T>) {
        return FilesystemPathToString<CharType>(arg);
    } else if constexpr (is_string_like_v<T>) {
        return std::basic_string_view<CharType>{arg};
    } else if constexpr (is_char_v<T>) {
        static_assert(std::is_same_v<CharType, T>);
        return std::basic_string_view<CharType>(&arg, 1);
    } else if constexpr (is_formattable_to_static_ascii_buffer<T>) {
        return ValueToStringLike(arg);
    } else if constexpr (requires(const T &test_arg) {
                             { to_basic_string<CharType>(test_arg) } -> std::same_as<std::basic_string<CharType>>;
                         }) {
        return to_basic_string<CharType>(arg);
    } else if constexpr (requires(const T &test_arg) {
                             {
                                 to_basic_string_view<CharType>(test_arg)
                             } -> std::same_as<std::basic_string_view<CharType>>;
                         }) {
        return to_basic_string_view<CharType>(arg);
    } else if constexpr (WriteableViaBasicOStringStream<T, CharType> && !is_scalar_v<T>) {
        return ToStringOneArgViaOStringStream<CharType>(arg);
    } else if constexpr (std::is_same_v<CharType, char>) {
        return ValueToStringLike(arg);
    } else {
        return ConvertBytesTo<CharType>(ValueToStringLike(arg));
    }
}

template <typename... Args>
ATTRIBUTE_ALWAYS_INLINE [[nodiscard]]
constexpr size_t CalculateStringArgsSizeImpl(const size_t first_s_size, const Args... sizes) noexcept {
    size_t total_size = first_s_size;
    if constexpr (sizeof...(sizes) > 0) {
        const size_t other_args_size = join_strings_detail::CalculateStringArgsSizeImpl(sizes...);
        total_size += other_args_size;
        const bool overflow_occured = total_size < other_args_size;
        if (unlikely(overflow_occured)) {
            return std::numeric_limits<size_t>::max();
        }
    }

    return total_size;
}

template <typename... Args>
ATTRIBUTE_ALWAYS_INLINE [[nodiscard]]
constexpr size_t CalculateStringArgsSize(const Args &...args) noexcept {
    return CalculateStringArgsSizeImpl(args.size()...);
}

// clang-format off

template <misc::Char CharType, typename T, typename... Args>
ATTRIBUTE_NONNULL_ALL_ARGS
ATTRIBUTE_ACCESS(write_only, 1)
constexpr void WriteStringsInplace(CharType *const result, const T& s, const Args&... args) noexcept {
    // clang-format on

    if constexpr (std::is_same_v<typename T::value_type, CharType>) {
        std::char_traits<CharType>::copy(result, s.data(), s.size());
    } else {
        const std::string_view ascii_buffer_view = s.as_string_view();
        for (size_t i = 0; i < ascii_buffer_view.size(); i++) {
            result[i] = static_cast<CharType>(ascii_buffer_view[i]);
        }
    }
    if constexpr (sizeof...(args) > 0) {
        join_strings_detail::WriteStringsInplace<CharType>(result + s.size(), args...);
    }
}

// clang-format off

template <misc::Char CharType, typename... Args>
ATTRIBUTE_NONNULL_ALL_ARGS
ATTRIBUTE_SIZED_ACCESS(write_only, 1, 2)
constexpr void WriteStringToBuffer(CharType* const buffer, size_t /*buffer_size*/, const Args&... args) noexcept {
    // clang-format on
    join_strings_detail::WriteStringsInplace<CharType>(buffer, args...);
}

template <misc::Char CharType, typename... Args>
    requires(sizeof...(Args) >= 2)
[[nodiscard]] inline std::basic_string<CharType> JoinStringsImpl(const Args &...args) {
    const size_t size = CalculateStringArgsSize(args...);
    std::basic_string<CharType> result(size, CharType{});
    join_strings_detail::WriteStringToBuffer<CharType>(result.data(), result.size(), args...);
    return result;
}

template <misc::Char CharType, typename... Args>
    requires(sizeof...(Args) >= 2)
[[nodiscard]] inline std::basic_string<CharType> JoinStringsImpl(std::basic_string<CharType> &&result,
                                                                 const Args &...args) {
    const size_t size = CalculateStringArgsSize(std::basic_string_view<CharType>(result), args...);
    const size_t initial_size = result.size();
    CONFIG_ASSUME_STATEMENT(initial_size <= size);
    result.resize(size);
    join_strings_detail::WriteStringToBuffer<CharType>(result.data() + initial_size, result.size(), args...);
    return std::move(result);
}

template <misc::Char CharType, size_t I, class T, class... Args>
    requires(sizeof...(Args) >= 1)
[[nodiscard]] inline std::basic_string<CharType> JoinStringsConvArgsToStrViewImpl(T &&value, Args &&...args) {
    if constexpr (I == 1 + sizeof...(args)) {
        return join_strings_detail::JoinStringsImpl<CharType>(std::forward<T>(value), args...);
    } else if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::basic_string<CharType>>) {
        return join_strings_detail::JoinStringsConvArgsToStrViewImpl<CharType, I + 1>(std::forward<Args>(args)...,
                                                                                      std::forward<T>(value));
    } else {
        const auto arg_as_str_like = ValueToGenericStringLike<CharType>(value);
        return join_strings_detail::JoinStringsConvArgsToStrViewImpl<CharType, I + 1>(std::forward<Args>(args)...,
                                                                                      arg_as_str_like);
    }
}

template <misc::Char CharType, class T>
[[nodiscard]] inline std::basic_string<CharType> NonStringValueToString(const T &value) {
    using generic_string_like_type = decltype(ValueToGenericStringLike<CharType>(std::declval<const T &>()));

    if constexpr (std::is_same_v<generic_string_like_type, std::basic_string<CharType>>) {
        return ValueToGenericStringLike<CharType>(value);
    } else {
        generic_string_like_type str_like = ValueToGenericStringLike<CharType>(value);
        if constexpr (std::is_same_v<typename generic_string_like_type::value_type, CharType>) {
            return std::basic_string<CharType>{std::basic_string_view<CharType>(str_like)};
        } else {
            const std::string_view ascii_buf = str_like.as_string_view();
            return std::basic_string<CharType>(ascii_buf.data(), ascii_buf.data() + ascii_buf.size());
        }
    }
}

template <misc::Char CharType, class T>
[[nodiscard]] inline std::basic_string<CharType> ValueToString(T &&value) {
    if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::basic_string<CharType>>) {
        return std::forward<T>(value);
    } else {
        return NonStringValueToString<CharType>(value);
    }
}

}  // namespace join_strings_detail

// clang-format off

template <misc::Char HintCharType, class... Args>
inline auto join_strings(Args&&... args) {
    static_assert(sizeof...(args) >= 1, "Empty input is explicitly prohibited");

    using DeducedCharType = misc::string_detail::determine_char_t<std::remove_cvref_t<Args>...>;
    using CharType = std::conditional_t<misc::is_char_v<DeducedCharType>, DeducedCharType, HintCharType>;

    constexpr bool kAllCharTypesAreSame = std::conjunction_v<misc::string_detail::same_char_types<std::remove_cvref_t<Args>, CharType>...>;
    static_assert(
        kAllCharTypesAreSame,
        "Hint:\n"
        "    Some non integral/float arguments have different char types\n"
        "    For example, both std::string and std::wstring might have been passed to the join_strings\n");

    if constexpr (kAllCharTypesAreSame) {
        if constexpr (sizeof...(args) == 1) {
            return join_strings_detail::ValueToString<CharType>(std::forward<Args>(args)...);
        } else {
            return join_strings_detail::JoinStringsConvArgsToStrViewImpl<CharType, 0>(std::forward<Args>(args)...);
        }
    } else {
        // Do not make more unreadable CEs when kAllCharTypesAreSame == false, static assertion has already failed
        return std::basic_string<CharType>{};
    }
}

// clang-format on

namespace join_strings_detail {

inline void ThrowOnNegativeMaxLengthDuringConversionToNotUTF8(const int32_t max_char_conv_len,
                                                              const std::string_view file_location,
                                                              const std::string_view function_name) {
    throw std::runtime_error{misc::join_strings("codecvt::max_length() returned negative value ", max_char_conv_len,
                                                " at ", file_location, ' ', function_name)};
}

inline void ThrowOnTooLongStringDuringConversionToNotUTF8(const size_t input_str_size,
                                                          const size_t max_char_conv_len,
                                                          const std::string_view file_location,
                                                          const std::string_view function_name

) {
    throw std::runtime_error{misc::join_strings(
        "DoStrCodecvt(): size of the converted string is too large. Input string size: ", input_str_size,
        ", max size of the converted character: ", max_char_conv_len, " at ", file_location, ' ', function_name)};
}

inline void ThrowOnFailedConversionToNotUTF8(const bool conversion_succeeded,
                                             const size_t converted_bytes_count,
                                             const size_t string_size,
                                             const std::string_view file_location,
                                             const std::string_view function_name) {
    using namespace std::string_view_literals;

    const std::string_view error_str = !conversion_succeeded ? "Could not convert"sv : "Only partially converted"sv;
    throw std::runtime_error{
        misc::join_strings(error_str, " multibyte string type to another string type (total bytes converted: "sv,
                           converted_bytes_count, " of "sv, string_size, ") at "sv, file_location, ' ', function_name)};
}

inline void ThrowOnFailedConversionToUTF8(const std::string_view file_location, const std::string_view function_name) {
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
          bool has_value_type = misc::is_basic_string_v<StringType> || misc::is_basic_string_view_v<StringType>,
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
    constexpr const char kMessage[] = "join_strings_range(): total strings length exceeded max size_t value";
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
    const size_t seps_total_size = container_size == 0 ? size_t{0} : sep_size * (container_size - 1);
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
[[nodiscard]] std::basic_string<T> JoinStringsRangeBySv(const std::basic_string_view<T> sep, const Container &strings) {
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

        return join_strings_detail::JoinStringsRangeBySv<CharType, Container>(std::basic_string_view<CharType>{sep},
                                                                              strings);
    }
}

template <std::ranges::forward_range Container>
inline auto join_strings_range(const Container &strings) {
    using StringType = std::ranges::range_value_t<Container>;

    static_assert(misc::is_basic_string_v<StringType> || misc::is_basic_string_view_v<StringType>,
                  "join_strings_range accepts only a range of std::basic_string or "
                  "std::basic_string_view");

    using CharType = typename StringType::value_type;
    static_assert(misc::is_char_v<CharType>, "join_strings_range expects a range of strings (with correct char type)");

    return join_strings_detail::JoinStringsRangeWithEmptySep<CharType, Container>(strings);
}

}  // namespace misc
