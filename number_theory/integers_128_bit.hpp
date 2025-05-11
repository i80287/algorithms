#pragma once

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

#include "../misc/config_macros.hpp"

#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && defined(__SIZEOF_INT128__)

typedef __uint128_t uint128_t;
typedef __int128_t int128_t;

#define HAS_INT128_TYPEDEF

#define INT128_IS_BUILTIN_TYPE 1

#elif CONFIG_COMPILER_IS_MSVC && CONFIG_HAS_INCLUDE(<__msvc_int128.hpp>)

#include <__msvc_int128.hpp>
typedef std::_Unsigned128 uint128_t;
typedef std::_Signed128 int128_t;

#define HAS_INT128_TYPEDEF

#define INT128_IS_BUILTIN_TYPE 0

#else

#if defined(INTEGERS_128_BIT_HPP_WARN_IF_UNSUPPORED)
#if CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG
// cppcheck-suppress [preprocessorErrorDirective]
#warning "Unsupported compiler, typedef 128-bit integer specific for your compiler"
#elif CONFIG_COMPILER_IS_MSVC
// cppcheck-suppress [preprocessorErrorDirective]
#pragma message WARN("Unsupported compiler, typedef 128-bit integer specific for your compiler")
#else
#error "Unknown compiler, could not warn about unsupported compiler for the 128-bit integer"
#endif
#endif

#endif

#if defined(HAS_INT128_TYPEDEF)

#ifndef INTEGERS_128_BIT_HPP
#define INTEGERS_128_BIT_HPP

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>

#if CONFIG_HAS_AT_LEAST_CXX_20 && !defined(__APPLE__) && CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG && \
    CONFIG_HAS_INCLUDE(<format>)
#define SPECIALIZE_STD_FORMAT
#include <format>
#endif

#if CONFIG_HAS_CONCEPTS
#include <concepts>
#endif

/**
 * This macro is defined if current [u]int128_t supports constexpr
 * operations.
 */
#if (CONFIG_HAS_AT_LEAST_CXX_17 && CONFIG_COMPILER_IS_GCC_OR_ANY_CLANG) || \
    (CONFIG_HAS_AT_LEAST_CXX_20 && defined(_MSC_VER))
#define HAS_I128_CONSTEXPR
#endif

/**
 * This macro is defined to `constexpr` if current [u]int128_t supports constexpr
 * operations. Defined to `inline` otherwise.
 */
#if defined(HAS_I128_CONSTEXPR)
#define I128_CONSTEXPR constexpr
#else
#define I128_CONSTEXPR inline
#endif

// clang-format off
// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
// clang-format on

namespace int128_detail {

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

#if CONFIG_HAS_AT_LEAST_CXX_20

// 340282366920938463463374607431768211455 == 2^128 - 1
inline constexpr std::size_t kMaxStringLengthU128 =
    std::char_traits<char>::length("340282366920938463463374607431768211455");
static_assert(kMaxStringLengthU128 == 39, "impl error");
//  170141183460469231731687303715884105727 ==  2^127 - 1
// -170141183460469231731687303715884105728 == -2^127
inline constexpr std::size_t kMaxStringLengthI128 =
    std::char_traits<char>::length("-170141183460469231731687303715884105728");
static_assert(kMaxStringLengthI128 == 40, "impl error");
#else
constexpr std::size_t kMaxStringLengthU128 = 39;
constexpr std::size_t kMaxStringLengthI128 = 40;
#endif

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

/// @brief Realization is taken from the gcc libstdc++ __to_chars_10_impl
/// @param number
/// @param buffer_ptr
/// @return
[[nodiscard]] I128_CONSTEXPR char* uint128_t_format_fill_chars_buffer(
    uint128_t number, char* buffer_ptr ATTRIBUTE_LIFETIME_BOUND) noexcept {
    constexpr std::uint8_t remainders[201] =
        "0001020304050607080910111213141516171819"
        "2021222324252627282930313233343536373839"
        "4041424344454647484950515253545556575859"
        "6061626364656667686970717273747576777879"
        "8081828384858687888990919293949596979899";

    constexpr uint32_t kBase1 = 10;
    constexpr uint32_t kBase2 = kBase1 * kBase1;

    while (number >= kBase2) {
        const auto remainder_index = static_cast<std::size_t>(number % kBase2) * 2;
        number /= kBase2;
        *--buffer_ptr = static_cast<char>(remainders[remainder_index + 1]);
        *--buffer_ptr = static_cast<char>(remainders[remainder_index]);
    }

    if (number >= kBase1) {
        const auto remainder_index = static_cast<std::size_t>(number) * 2;
        *--buffer_ptr = static_cast<char>(remainders[remainder_index + 1]);
        *--buffer_ptr = static_cast<char>(remainders[remainder_index]);
    } else {
        *--buffer_ptr = static_cast<char>('0' + number);
    }

    return buffer_ptr;
}

[[nodiscard]] ATTRIBUTE_CONST I128_CONSTEXPR uint128_t uabs128(const int128_t number) noexcept {
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    const uint128_t t = static_cast<uint128_t>(number >> 127U);
    return (static_cast<uint128_t>(number) ^ t) - t;
}

}  // namespace int128_detail

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

inline std::ostream& operator<<(std::ostream& out ATTRIBUTE_LIFETIME_BOUND,
                                const uint128_t number) {
    constexpr auto kBufferSize = int128_detail::kMaxStringLengthU128;

    char digits[kBufferSize];
    char* const buffer_end_ptr = digits + kBufferSize;
    const char* const ptr =
        int128_detail::uint128_t_format_fill_chars_buffer(number, buffer_end_ptr);
    const auto length = static_cast<std::size_t>(buffer_end_ptr - ptr);
    return out << std::string_view(ptr, length);
}

inline std::ostream& operator<<(std::ostream& out ATTRIBUTE_LIFETIME_BOUND, const int128_t number) {
    constexpr auto kBufferSize = int128_detail::kMaxStringLengthI128;
    char digits[kBufferSize];

    const uint128_t number_abs = int128_detail::uabs128(number);

    char* const buffer_end_ptr = digits + kBufferSize;
    char* ptr = int128_detail::uint128_t_format_fill_chars_buffer(number_abs, buffer_end_ptr);
    if (number < 0) {
        *--ptr = '-';
    }
    const auto length = static_cast<std::size_t>(buffer_end_ptr - ptr);
    CONFIG_ASSUME_STATEMENT(length <= kBufferSize);
    return out << std::string_view(ptr, length);
}

ATTRIBUTE_NONNULL(2)
inline int fprint_u128(const uint128_t number, std::FILE* const filestream) {
    // + 1 for '\0'
    constexpr auto kBufferSize = int128_detail::kMaxStringLengthU128 + 1;
    char digits[kBufferSize];
    digits[kBufferSize - 1] = '\0';
    const char* const ptr =
        int128_detail::uint128_t_format_fill_chars_buffer(number, &digits[kBufferSize - 1]);
    return std::fputs(ptr, filestream);
}

inline int print_u128(const uint128_t number) {
    return ::fprint_u128(number, stdout);
}

ATTRIBUTE_NONNULL(2)
inline int fprint_u128_newline(const uint128_t number, std::FILE* const filestream) {
    // + 1 for '\n', + 1 for '\0'
    constexpr auto kBufferSize = int128_detail::kMaxStringLengthU128 + 1 + 1;
    char digits[kBufferSize];
    digits[kBufferSize - 2] = '\n';
    digits[kBufferSize - 1] = '\0';
    const char* const ptr =
        int128_detail::uint128_t_format_fill_chars_buffer(number, &digits[kBufferSize - 2]);
    return std::fputs(ptr, filestream);
}

inline int print_u128_newline(const uint128_t number) {
    return ::fprint_u128_newline(number, stdout);
}

[[nodiscard]] inline std::string to_string(const uint128_t number) {
    constexpr auto kBufferSize = int128_detail::kMaxStringLengthU128;
    char digits[kBufferSize];

    char* const buffer_end_ptr = digits + kBufferSize;
    const char* const ptr =
        int128_detail::uint128_t_format_fill_chars_buffer(number, buffer_end_ptr);
    const auto length = static_cast<std::size_t>(buffer_end_ptr - ptr);
    CONFIG_ASSUME_STATEMENT(length <= kBufferSize);
    return std::string(ptr, length);
}

[[nodiscard]] inline std::string to_string(const int128_t number) {
    constexpr auto kBufferSize = int128_detail::kMaxStringLengthI128;
    char digits[kBufferSize];

    const uint128_t number_abs = int128_detail::uabs128(number);

    char* const buffer_end_ptr = digits + kBufferSize;
    char* ptr = int128_detail::uint128_t_format_fill_chars_buffer(number_abs, buffer_end_ptr);
    if (number < 0) {
        *--ptr = '-';
    }
    const auto length = static_cast<std::size_t>(buffer_end_ptr - ptr);
    CONFIG_ASSUME_STATEMENT(length <= kBufferSize);
    return std::string(ptr, length);
}

// clang-format off
// NOLINTEND(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
// clang-format on

#if defined(SPECIALIZE_STD_FORMAT)

template <class CharT>
struct std::formatter<uint128_t, CharT> {  // NOLINT(cert-dcl58-cpp)
    template <class ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <class FmtContext>
    typename FmtContext::iterator format(const uint128_t n, FmtContext& ctx) const {
        std::string s = to_string(n);
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
    typename FmtContext::iterator format(const int128_t n, FmtContext& ctx) const {
        std::string s = to_string(n);
        return std::copy(s.begin(), s.end(), ctx.out());
    }
};

#undef SPECIALIZE_STD_FORMAT

#endif

#endif  // !INTEGERS_128_BIT_HPP

#undef HAS_INT128_TYPEDEF

#endif  // HAS_INT128_TYPEDEF
