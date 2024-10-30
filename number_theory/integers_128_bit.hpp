/*
 * Small chunk of functions (like std::ostream::operator<<, print_u128) and
 * template instantiations (like int128_traits::is_unsigned,
 * int128_traits::make_unsigned) for 128 bit width integers
 * typedefed as uint128_t and int128_t.
 *
 * This file is targeted for the g++ compiler. If your compiler supports
 * 128 bit integers but typedefes them differently from g++ (__uint128_t and
 * __int128_t), then you should change typedefs in the begining of this file.
 *
 */

#include "config_macros.hpp"

#if (defined(__clang__) || defined(__GNUC__)) && defined(__SIZEOF_INT128__)

typedef __uint128_t uint128_t;
typedef __int128_t int128_t;

#define HAS_INT128_TYPEDEF 1

#elif defined(_MSC_VER) && CONFIG_HAS_INCLUDE(<__msvc_int128.hpp>)

#include <__msvc_int128.hpp>
typedef std::_Unsigned128 uint128_t;
typedef std::_Signed128 int128_t;

#define HAS_INT128_TYPEDEF 1

#else

#if defined(INTEGERS_128_BIT_HPP_WARN_IF_UNSUPPORED) && INTEGERS_128_BIT_HPP_WARN_IF_UNSUPPORED

#if defined(__clang__) || defined(__GNUC__)
// cppcheck-suppress [preprocessorErrorDirective]
#warning "Unsupported compiler, typedef 128-bit integer specific for your compiler"
#elif defined(_MSC_VER)
// cppcheck-suppress [preprocessorErrorDirective]
#pragma message WARN("your warning message here")
#endif

#endif

#define HAS_INT128_TYPEDEF 0

#endif

#if HAS_INT128_TYPEDEF

#ifndef INTEGERS_128_BIT_HPP
#define INTEGERS_128_BIT_HPP 1

#include <cstddef>
#include <cstdint>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>

#if CONFIG_HAS_AT_LEAST_CXX_20 && !defined(__APPLE__) && \
    (defined(__GNUC__) || defined(__clang__)) && CONFIG_HAS_INCLUDE(<format>)
#define SPECIALIZE_STD_FORMAT 1
#include <format>
#else
#define SPECIALIZE_STD_FORMAT 0
#endif

/**
 * Macro defined to 1 if current [u]int128_t supports constexpr
 * operations and 0 otherwise.
 */
#if (CONFIG_HAS_AT_LEAST_CXX_17 && (defined(__GNUG__) || defined(__clang__))) || \
    (CONFIG_HAS_AT_LEAST_CXX_20 && defined(_MSC_VER))
#define HAS_I128_CONSTEXPR 1
#else
#define HAS_I128_CONSTEXPR 0
#endif

/**
 * Macro defined to `constexpr` if current [u]int128_t supports constexpr
 * operations. Defined to empty otherwise.
 */
#if HAS_I128_CONSTEXPR
#define I128_CONSTEXPR constexpr
#else
#define I128_CONSTEXPR inline
#endif

namespace format_impl_uint128_t {

#if CONFIG_HAS_AT_LEAST_CXX_20
// 340282366920938463463374607431768211455 == 2^128 - 1
inline constexpr std::size_t kMaxStringLengthU128 =
    std::char_traits<char>::length("340282366920938463463374607431768211455");
static_assert(kMaxStringLengthU128 == 39, "");
//  170141183460469231731687303715884105727 ==  2^127 - 1
// -170141183460469231731687303715884105728 == -2^127
inline constexpr std::size_t kMaxStringLengthI128 =
    std::char_traits<char>::length("-170141183460469231731687303715884105728");
static_assert(kMaxStringLengthI128 == 40, "");
#else
constexpr std::size_t kMaxStringLengthU128 = 39;
constexpr std::size_t kMaxStringLengthI128 = 40;
#endif

/// @brief Realization taken from the gcc libstdc++ __to_chars_10_impl
/// @param number
/// @param buffer_ptr
/// @return
I128_CONSTEXPR char* uint128_t_format_fill_chars_buffer(uint128_t number,
                                                        char* buffer_ptr) noexcept {
    constexpr std::uint8_t remainders[201] =
        "0001020304050607080910111213141516171819"
        "2021222324252627282930313233343536373839"
        "4041424344454647484950515253545556575859"
        "6061626364656667686970717273747576777879"
        "8081828384858687888990919293949596979899";

    while (number >= 100) {
        const auto remainder_index = std::size_t(number % 100) * 2;
        number /= 100;
        *--buffer_ptr = char(remainders[remainder_index + 1]);
        *--buffer_ptr = char(remainders[remainder_index]);
    }

    if (number >= 10) {
        const auto remainder_index = std::size_t(number) * 2;
        *--buffer_ptr              = char(remainders[remainder_index + 1]);
        *--buffer_ptr              = char(remainders[remainder_index]);
    } else {
        *--buffer_ptr = char('0' + number);
    }

    return buffer_ptr;
}

}  // namespace format_impl_uint128_t

namespace int128_traits {

template <class T>
struct is_integral {
    static constexpr bool value = std::is_integral_v<T>;
};

template <>
struct is_integral<uint128_t> {
    static constexpr bool value = true;
};

template <>
struct is_integral<int128_t> {
    static constexpr bool value = true;
};

template <class T>
struct make_unsigned {
    using type = typename std::make_unsigned_t<T>;
};

template <>
struct make_unsigned<uint128_t> {
    using type = uint128_t;
};

template <>
struct make_unsigned<int128_t> {
    using type = uint128_t;
};

template <class T>
struct make_signed {
    using type = typename std::make_signed_t<T>;
};

template <>
struct make_signed<uint128_t> {
    using type = int128_t;
};

template <>
struct make_signed<int128_t> {
    using type = int128_t;
};

template <class T>
struct is_unsigned {
    static constexpr bool value = std::is_unsigned_v<T>;
};

template <>
struct is_unsigned<uint128_t> {
    static constexpr bool value = true;
};

template <>
struct is_unsigned<int128_t> {
    static constexpr bool value = false;
};

template <class T>
struct is_signed {
    static constexpr bool value = std::is_signed_v<T>;
};

template <>
struct is_signed<uint128_t> {
    static constexpr bool value = false;
};

template <>
struct is_signed<int128_t> {
    static constexpr bool value = true;
};

template <class T>
struct is_arithmetic {
    static constexpr bool value = std::is_arithmetic_v<T>;
};

template <>
struct is_arithmetic<uint128_t> {
    static constexpr bool value = true;
};

template <>
struct is_arithmetic<int128_t> {
    static constexpr bool value = true;
};

template <class T>
inline constexpr bool is_integral_v = ::int128_traits::is_integral<T>::value;

template <class T>
inline constexpr bool is_unsigned_v = ::int128_traits::is_unsigned<T>::value;

template <class T>
inline constexpr bool is_signed_v = ::int128_traits::is_signed<T>::value;

template <class T>
inline constexpr bool is_arithmetic_v = ::int128_traits::is_arithmetic<T>::value;

template <typename T>
using make_unsigned_t = typename ::int128_traits::make_unsigned<T>::type;

template <typename T>
using make_signed_t = typename ::int128_traits::make_signed<T>::type;

#if CONFIG_HAS_CONCEPTS

template <class T>
concept integral = std::integral<T> || int128_traits::is_integral_v<T>;

template <class T>
concept signed_integral =
    std::signed_integral<T> || (int128_traits::integral<T> && int128_traits::is_signed_v<T>);

template <class T>
concept unsigned_integral =
    std::unsigned_integral<T> || (int128_traits::integral<T> && !int128_traits::signed_integral<T>);

#endif

}  // namespace int128_traits

inline std::ostream& operator<<(std::ostream& out, uint128_t number) {
    // + 1 for '\0'
    constexpr auto buffer_size = ::format_impl_uint128_t::kMaxStringLengthU128 + 1;

    char digits[buffer_size];
    digits[buffer_size - 1] = '\0';
    const char* ptr         = ::format_impl_uint128_t::uint128_t_format_fill_chars_buffer(
        number, &digits[buffer_size - 1]);
    const auto length = static_cast<std::size_t>(&digits[buffer_size - 1] - ptr);
    return out << std::string_view(ptr, length);
}

inline std::ostream& operator<<(std::ostream& out, int128_t number) {
    // + 1 for '\0'
    constexpr auto buffer_size = ::format_impl_uint128_t::kMaxStringLengthI128 + 1;
    char digits[buffer_size];
    digits[buffer_size - 1] = '\0';

    const bool negative = number < 0;
    if (negative) {
        number = -number;
    }

    char* ptr = ::format_impl_uint128_t::uint128_t_format_fill_chars_buffer(
        static_cast<uint128_t>(number), &digits[buffer_size - 1]);
    if (negative) {
        *--ptr = '-';
    }
    const auto length = static_cast<std::size_t>(&digits[buffer_size - 1] - ptr);
    if (length > buffer_size) {
        CONFIG_UNREACHABLE();
    }

    return out << std::string_view(ptr, length);
}

ATTRIBUTE_NONNULL(2)
inline int fprint_u128(uint128_t number, std::FILE* filestream) noexcept {
    // + 1 for '\0'
    constexpr auto buffer_size = ::format_impl_uint128_t::kMaxStringLengthU128 + 1;
    char digits[buffer_size];
    digits[buffer_size - 1] = '\0';
    const char* ptr         = ::format_impl_uint128_t::uint128_t_format_fill_chars_buffer(
        number, &digits[buffer_size - 1]);
    return std::fputs(ptr, filestream);
}

inline int print_u128(uint128_t number) noexcept {
    return ::fprint_u128(number, stdout);
}

ATTRIBUTE_NONNULL(2)
inline int fprint_u128_newline(uint128_t number, std::FILE* filestream) noexcept {
    // + 1 for '\0', + 1 for '\n'
    constexpr auto buffer_size = ::format_impl_uint128_t::kMaxStringLengthU128 + 1 + 1;
    char digits[buffer_size];
    digits[buffer_size - 2] = '\n';
    digits[buffer_size - 1] = '\0';
    const char* ptr         = ::format_impl_uint128_t::uint128_t_format_fill_chars_buffer(
        number, &digits[buffer_size - 2]);
    return std::fputs(ptr, filestream);
}

inline int print_u128_newline(uint128_t number) noexcept {
    return ::fprint_u128_newline(number, stdout);
}

[[nodiscard]] inline std::string to_string(uint128_t number) {
    // + 1 for '\0'
    constexpr auto buffer_size = ::format_impl_uint128_t::kMaxStringLengthU128 + 1;
    char digits[buffer_size];
    digits[buffer_size - 1] = '\0';

    const char* ptr = ::format_impl_uint128_t::uint128_t_format_fill_chars_buffer(
        number, &digits[buffer_size - 1]);
    const auto length = static_cast<std::size_t>(&digits[buffer_size - 1] - ptr);
    if (length > buffer_size) {
        CONFIG_UNREACHABLE();
    }

    return std::string(ptr, length);
}

[[nodiscard]] inline std::string to_string(int128_t number) {
    // + 1 for '\0'
    constexpr auto buffer_size = ::format_impl_uint128_t::kMaxStringLengthI128 + 1;
    char digits[buffer_size];
    digits[buffer_size - 1] = '\0';

    const uint128_t t          = static_cast<uint128_t>(number >> 127);
    const uint128_t number_abs = (static_cast<uint128_t>(number) ^ t) - t;

    char* ptr = ::format_impl_uint128_t::uint128_t_format_fill_chars_buffer(
        number_abs, &digits[buffer_size - 1]);
    if (number < 0) {
        *--ptr = '-';
    }
    const auto length = static_cast<std::size_t>(&digits[buffer_size - 1] - ptr);
    if (length > buffer_size) {
        CONFIG_UNREACHABLE();
    }

    return std::string(ptr, length);
}

#if SPECIALIZE_STD_FORMAT

template <class CharT>
struct std::formatter<uint128_t, CharT> {  // NOLINT(cert-dcl58-cpp)
    template <class ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <class FmtContext>
    typename FmtContext::iterator format(uint128_t n, FmtContext& ctx) const {
        std::string s = ::to_string(n);
        return std::copy(s.begin(), s.end(), ctx.out());
    }
};

template <class CharT>
struct std::formatter<int128_t, CharT> {  // NOLINT(cert-dcl58-cpp)
    template <class ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <class FmtContext>
    typename FmtContext::iterator format(int128_t n, FmtContext& ctx) const {
        std::string s = ::to_string(n);
        return std::copy(s.begin(), s.end(), ctx.out());
    }
};

#endif

#undef SPECIALIZE_STD_FORMAT

#endif  // !INTEGERS_128_BIT_HPP

#endif  // HAS_INT128_TYPEDEF

#undef HAS_INT128_TYPEDEF
