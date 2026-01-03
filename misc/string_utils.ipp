#ifndef STRING_UTILS_INCLUDING_IMPLEMENTATION
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
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <utility>

#if CONFIG_HAS_CONCEPTS
#include <concepts>
#endif

#include "config_macros.hpp"
#include "string_traits.hpp"

namespace misc {
using std::size_t;
using std::uint32_t;

namespace detail {

template <typename CharType>
ATTRIBUTE_ALWAYS_INLINE constexpr void assert_type_is_char_or_wchar() noexcept {
    static_assert(std::is_same_v<CharType, char> || std::is_same_v<CharType, wchar_t>,
                  "char types other than char and wchar_t are not supported");
}

template <typename CharType>
[[nodiscard]] ATTRIBUTE_CONST constexpr uint32_t char_to_uint(const CharType c) noexcept {
    return uint32_t{static_cast<std::make_unsigned_t<CharType>>(c)};
}

template <typename CharType = char>
[[nodiscard]] ATTRIBUTE_CONST constexpr char uint_to_char(const uint32_t char_value) noexcept {
    return static_cast<CharType>(char_value);
}

template <typename CharType>
[[nodiscard]] ATTRIBUTE_CONST constexpr auto char_to_c_uint(const CharType c) noexcept {
    const auto uc = static_cast<std::make_unsigned_t<CharType>>(c);
    if constexpr (std::is_same_v<CharType, char>) {
        return static_cast<int>(static_cast<unsigned char>(c));
    } else if constexpr (std::is_same_v<CharType, wchar_t>) {
        return static_cast<wint_t>(uc);
    } else {
        return uc;
    }
}

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_char_between(const char value,
                                                             const char min,
                                                             const char max) noexcept {
    return detail::char_to_uint(value) - detail::char_to_uint(min) <=
           detail::char_to_uint(max) - detail::char_to_uint(min);
}

using alpha_digit_table_type = std::array<bool, static_cast<size_t>(std::numeric_limits<char>::max()) + 1>;

inline constexpr alpha_digit_table_type kAlphaDigitTable = []() constexpr {
    alpha_digit_table_type table{};
    for (size_t c = '0'; c <= '9'; c++) {
        table[c] = true;
    }
    for (size_t c = 'a'; c <= 'z'; c++) {
        table[c] = true;
    }
    for (size_t c = 'A'; c <= 'Z'; c++) {
        table[c] = true;
    }

    return table;
}();

inline constexpr alpha_digit_table_type kHexDigitTable = []() constexpr {
    alpha_digit_table_type table{};
    for (size_t c = '0'; c <= '9'; c++) {
        table[c] = true;
    }
    for (size_t c = 'a'; c <= 'f'; c++) {
        table[c] = true;
    }
    for (size_t c = 'A'; c <= 'F'; c++) {
        table[c] = true;
    }

    return table;
}();

[[nodiscard]] ATTRIBUTE_CONST constexpr bool is_whitespace_utf32(const char32_t c) noexcept {
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
    detail::assert_type_is_char_or_wchar<CharType>();
    if constexpr (std::is_same_v<CharType, char>) {
        return static_cast<bool>(std::isspace(detail::char_to_c_uint(c)));
    } else {
        return static_cast<bool>(std::iswspace(detail::char_to_c_uint(c)));
    }
}

template <class CharType>
inline bool is_alpha(const CharType c) noexcept {
    detail::assert_type_is_char_or_wchar<CharType>();
    if constexpr (std::is_same_v<CharType, char>) {
        return static_cast<bool>(std::isalpha(detail::char_to_c_uint(c)));
    } else {
        return static_cast<bool>(std::iswalpha(detail::char_to_c_uint(c)));
    }
}

template <class CharType>
inline bool is_alpha_digit(const CharType c) noexcept {
    detail::assert_type_is_char_or_wchar<CharType>();
    if constexpr (std::is_same_v<CharType, char>) {
        return static_cast<bool>(std::isalnum(detail::char_to_c_uint(c)));
    } else {
        return static_cast<bool>(std::iswalnum(detail::char_to_c_uint(c)));
    }
}

template <class CharType>
inline bool is_digit(const CharType c) noexcept {
    detail::assert_type_is_char_or_wchar<CharType>();
    if constexpr (std::is_same_v<CharType, char>) {
        return static_cast<bool>(std::isdigit(detail::char_to_c_uint(c)));
    } else {
        return static_cast<bool>(std::iswdigit(detail::char_to_c_uint(c)));
    }
}

template <class CharType>
inline bool is_hex_digit(const CharType c) noexcept {
    detail::assert_type_is_char_or_wchar<CharType>();
    if constexpr (std::is_same_v<CharType, char>) {
        return static_cast<bool>(std::isxdigit(detail::char_to_c_uint(c)));
    } else {
        return static_cast<bool>(std::iswxdigit(detail::char_to_c_uint(c)));
    }
}

template <class CharType>
inline bool is_upper(const CharType c) noexcept {
    detail::assert_type_is_char_or_wchar<CharType>();
    if constexpr (std::is_same_v<CharType, char>) {
        return static_cast<bool>(std::isupper(detail::char_to_c_uint(c)));
    } else {
        return static_cast<bool>(std::iswupper(detail::char_to_c_uint(c)));
    }
}

template <class CharType>
inline bool is_lower(const CharType c) noexcept {
    detail::assert_type_is_char_or_wchar<CharType>();
    if constexpr (std::is_same_v<CharType, char>) {
        return static_cast<bool>(std::islower(detail::char_to_c_uint(c)));
    } else {
        return static_cast<bool>(std::iswlower(detail::char_to_c_uint(c)));
    }
}

template <class CharType>
inline CharType to_upper(const CharType c) noexcept {
    detail::assert_type_is_char_or_wchar<CharType>();
    if constexpr (std::is_same_v<CharType, char>) {
        return static_cast<CharType>(std::toupper(detail::char_to_c_uint(c)));
    } else {
        return static_cast<CharType>(std::towupper(detail::char_to_c_uint(c)));
    }
}

template <class CharType>
inline CharType to_lower(const CharType c) noexcept {
    detail::assert_type_is_char_or_wchar<CharType>();
    if constexpr (std::is_same_v<CharType, char>) {
        return static_cast<CharType>(std::tolower(detail::char_to_c_uint(c)));
    } else {
        return static_cast<CharType>(std::towlower(detail::char_to_c_uint(c)));
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
        str[i] = misc::to_lower(str[i]);
    }
}

template <class CharType>
inline void to_lower_inplace(std::basic_string<CharType> &str) noexcept {
    misc::to_lower_inplace(str.data(), str.size());
}

template <class CharType>
inline std::basic_string<CharType> to_lower(const std::basic_string_view<CharType> str) {
    std::basic_string<CharType> ret(str);
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
        str[i] = misc::to_upper(str[i]);
    }
}

template <class CharType>
inline void to_upper_inplace(std::basic_string<CharType> &str) noexcept {
    misc::to_upper_inplace(str.data(), str.size());
}

template <class CharType>
inline std::basic_string<CharType> to_upper(const std::basic_string_view<CharType> str) {
    std::basic_string<CharType> ret(str);
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

namespace locale_indep {

constexpr bool is_whitespace(const char c) noexcept {
    switch (detail::char_to_uint(c)) {
        case '\t':
        case '\n':
        case '\v':
        case '\f':
        case '\r':
        case ' ': {
            return true;
        }
        default: {
            return false;
        }
    }
}

constexpr bool is_whitespace(char16_t c) noexcept {
    return locale_indep::is_whitespace(static_cast<char32_t>(c));
}

constexpr bool is_whitespace(char32_t c) noexcept {
    return detail::is_whitespace_utf32(c);
}

constexpr bool is_alpha(const char c) noexcept {
    return detail::is_char_between(c, 'A', 'Z') | detail::is_char_between(c, 'a', 'z');
}

constexpr bool is_alpha_digit(const char c) noexcept {
    return detail::kAlphaDigitTable[detail::char_to_uint(c)];
}

constexpr bool is_digit(const char c) noexcept {
    return detail::is_char_between(c, '0', '9');
}

constexpr bool is_hex_digit(const char c) noexcept {
    return detail::kHexDigitTable[detail::char_to_uint(c)];
}

constexpr bool is_upper(const char c) noexcept {
    return detail::is_char_between(c, 'A', 'Z');
}

constexpr bool is_lower(const char c) noexcept {
    return detail::is_char_between(c, 'a', 'z');
}

constexpr char to_upper(const char c) noexcept {
    return detail::uint_to_char(detail::char_to_uint(c) - (locale_indep::is_lower(c) * uint32_t{'a' - 'A'}));
}

constexpr char to_lower(const char c) noexcept {
    return detail::uint_to_char(detail::char_to_uint(c) + (locale_indep::is_upper(c) * uint32_t{'a' - 'A'}));
}

constexpr bool is_whitespace(const std::string_view str) noexcept {
    for (const char c : str) {
        if (!misc::locale_indep::is_whitespace(c)) {
            return false;
        }
    }

    return true;
}

inline bool is_whitespace(const std::string &str) noexcept {
    return misc::locale_indep::is_whitespace(std::string_view{str});
}

constexpr bool is_whitespace(const char *const str) noexcept {
    return misc::locale_indep::is_whitespace(std::string_view{str});
}

inline void to_lower_inplace(char *const str, size_t n) noexcept {
    for (size_t i = 0; i < n; i++) {
        str[i] = misc::locale_indep::to_lower(str[i]);
    }
}

inline void to_lower_inplace(std::string &str) noexcept {
    misc::locale_indep::to_lower_inplace(str.data(), str.size());
}

inline std::string to_lower(const std::string_view str) {
    std::string ret(str);
    misc::locale_indep::to_lower_inplace(ret);
    return ret;
}

inline std::string to_lower(const std::string &str) {
    return misc::locale_indep::to_lower(std::string_view{str});
}

inline std::string to_lower(const char *const str) {
    return misc::locale_indep::to_lower(std::string_view{str});
}

inline void to_upper_inplace(char *const str, size_t n) noexcept {
    for (size_t i = 0; i < n; i++) {
        str[i] = misc::locale_indep::to_upper(str[i]);
    }
}

inline void to_upper_inplace(std::string &str) noexcept {
    misc::locale_indep::to_upper_inplace(str.data(), str.size());
}

inline std::string to_upper(const std::string_view str) {
    std::string ret(str);
    misc::locale_indep::to_upper_inplace(ret);
    return ret;
}

inline std::string to_upper(const std::string &str) {
    return misc::locale_indep::to_upper(std::string_view{str});
}

inline std::string to_upper(const char *const str) {
    return misc::locale_indep::to_upper(std::string_view{str});
}

}  // namespace locale_indep

namespace detail {

// clang-format off
template <class CharType, class Predicate>
[[nodiscard]]
ATTRIBUTE_ALWAYS_INLINE
constexpr std::basic_string_view<CharType> trim_if(
    std::basic_string_view<CharType> str ATTRIBUTE_LIFETIME_BOUND,
    Predicate pred
) noexcept(std::is_nothrow_invocable_r_v<bool, Predicate, CharType>) {
    // clang-format on
    static_assert(std::is_invocable_r_v<bool, Predicate, CharType>, "predicate should accept CharType and return bool");

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
constexpr std::basic_string_view<CharType> TrimChar(
    const std::basic_string_view<CharType> str ATTRIBUTE_LIFETIME_BOUND,
    const CharType trim_char
) noexcept {
    // clang-format on
    return detail::trim_if(str, [trim_char](const CharType c) constexpr noexcept -> bool { return c == trim_char; });
}

// clang-format off
template <class CharType>
[[nodiscard]]
std::basic_string_view<CharType> TrimCharsImpl(const std::basic_string_view<CharType> str,
                                               const std::basic_string_view<CharType> trim_chars) {
    // clang-format on
    using UType = std::make_unsigned_t<CharType>;
    static constexpr size_t kMaxUTypeValue = static_cast<size_t>(std::numeric_limits<UType>::max());
    static constexpr bool kUseArrayMap = kMaxUTypeValue <= std::numeric_limits<std::uint16_t>::max();

    using MapType = std::conditional_t<kUseArrayMap, std::array<bool, kMaxUTypeValue + 1>, std::unordered_set<UType>>;
    MapType trim_chars_map{};
    for (const CharType c : trim_chars) {
        const auto key = static_cast<UType>(c);
        if constexpr (kUseArrayMap) {
            trim_chars_map[key] = true;
        } else {
            trim_chars_map.insert(key);
        }
    }

    return detail::trim_if(str, [&trim_chars_map = std::as_const(trim_chars_map)](const CharType c) noexcept -> bool {
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
                                                  const std::basic_string_view<CharType> trim_chars) {
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

}  // namespace detail

template <class StrType, class TrimStrType>
inline auto trim(const StrType &str, const TrimStrType &trim_chars) noexcept(std::is_base_of_v<trim_tag, TrimStrType>) {
    if constexpr (std::is_base_of_v<misc::trim_tag, TrimStrType>) {
        using CharType = misc::string_detail::determine_char_t<StrType>;
        static_assert(misc::is_char_v<CharType>, "string is expected in the trim with tag");

        if constexpr (std::is_base_of_v<misc::locale_indep::trim_tag, TrimStrType>) {
            static_assert(std::is_same_v<CharType, char>,
                          "Only char type is supported for locale independent trimming");
        }

        const std::basic_string_view<CharType> str_sv{str};

        if constexpr (std::is_same_v<TrimStrType, whitespace_tag>) {
            return detail::trim_if(str_sv, [](const CharType c) noexcept { return misc::is_whitespace<CharType>(c); });
        } else if constexpr (std::is_same_v<TrimStrType, alpha_tag>) {
            return detail::trim_if(str_sv, &misc::is_alpha<CharType>);
        } else if constexpr (std::is_same_v<TrimStrType, digit_tag>) {
            return detail::trim_if(str_sv, &misc::is_digit<CharType>);
        } else if constexpr (std::is_same_v<TrimStrType, alpha_digit_tag>) {
            return detail::trim_if(str_sv, &misc::is_alpha_digit<CharType>);
        } else if constexpr (std::is_same_v<TrimStrType, hex_digit_tag>) {
            return detail::trim_if(str_sv, &misc::is_hex_digit<CharType>);
        } else if constexpr (std::is_same_v<TrimStrType, locale_indep::whitespace_tag>) {
            return detail::trim_if(str_sv, [](const char c) noexcept { return misc::locale_indep::is_whitespace(c); });
        } else if constexpr (std::is_same_v<TrimStrType, locale_indep::alpha_tag>) {
            return detail::trim_if(str_sv, &misc::locale_indep::is_alpha);
        } else if constexpr (std::is_same_v<TrimStrType, locale_indep::digit_tag>) {
            return detail::trim_if(str_sv, &misc::locale_indep::is_digit);
        } else if constexpr (std::is_same_v<TrimStrType, locale_indep::alpha_digit_tag>) {
            return detail::trim_if(str_sv, &misc::locale_indep::is_alpha_digit);
        } else if constexpr (std::is_same_v<TrimStrType, locale_indep::hex_digit_tag>) {
            return detail::trim_if(str_sv, &misc::locale_indep::is_hex_digit);
        } else {
            static_assert([]() constexpr { return false; }(), "implementation error");
            return str;
        }
    } else {
        using CharType = misc::string_detail::determine_char_t<StrType, TrimStrType>;
        static_assert(misc::is_char_v<CharType>, "strings with the same char type are expected in the trim");

        if constexpr (misc::is_char_v<TrimStrType>) {
            return detail::TrimChar<CharType>(str, trim_chars);
        } else {
            return detail::TrimChars(std::basic_string_view<CharType>{str},
                                     std::basic_string_view<CharType>{trim_chars});
        }
    }
}

}  // namespace misc
