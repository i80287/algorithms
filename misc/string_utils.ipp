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
constexpr std::basic_string_view<CharType> trim_if(std::basic_string_view<CharType> str, const Predicate pred)
    noexcept(std::is_nothrow_invocable_v<Predicate, CharType>)
{
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

template <class CharType>
[[nodiscard]]
constexpr std::basic_string_view<CharType> TrimChar(const std::basic_string_view<CharType> str,
                                                    const CharType trim_char) noexcept {
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
    static constexpr bool kUseArrayMap =
        kMaxUTypeValue <= std::numeric_limits<std::uint16_t>::max();

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
}  // namespace detail

template <class StrType, class TrimStrType>
inline auto trim(const StrType &str, const TrimStrType &trim_chars) noexcept {
    if constexpr (std::is_base_of_v<trim_tag, TrimStrType>) {
        using CharType = misc::string_detail::determine_char_t<StrType>;
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
        using CharType = misc::string_detail::determine_char_t<StrType, TrimStrType>;
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
