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

#include <array>
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

namespace int128_traits {

namespace detail {

using std::size_t;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

#if CONFIG_HAS_AT_LEAST_CXX_20

// 340282366920938463463374607431768211455 == 2^128 - 1
inline constexpr size_t kMaxStringLengthU128 =
    std::char_traits<char>::length("340282366920938463463374607431768211455");
static_assert(kMaxStringLengthU128 == 39, "impl error");
//  170141183460469231731687303715884105727 ==  2^127 - 1
// -170141183460469231731687303715884105728 == -2^127
inline constexpr size_t kMaxStringLengthI128 =
    std::char_traits<char>::length("-170141183460469231731687303715884105728");
static_assert(kMaxStringLengthI128 == 40, "impl error");
#else
constexpr size_t kMaxStringLengthU128 = 39;
constexpr size_t kMaxStringLengthI128 = 40;
#endif

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

[[nodiscard]] ATTRIBUTE_CONST I128_CONSTEXPR uint128_t uabs128(const int128_t number) noexcept {
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    const uint128_t t = static_cast<uint128_t>(number >> 127U);
    return (static_cast<uint128_t>(number) ^ t) - t;
}

[[nodiscard]] ATTRIBUTE_CONST I128_CONSTEXPR uint128_t uabs128(const uint128_t number) noexcept {
    return number;
}

/// @brief Realization is taken from the gcc libstdc++ __to_chars_10_impl
/// @param number
/// @param buffer_ptr
/// @return
ATTRIBUTE_NONNULL_ALL_ARGS
ATTRIBUTE_RETURNS_NONNULL
[[nodiscard]] I128_CONSTEXPR char* uint128_t_format_fill_chars_buffer(
    uint128_t number, char* buffer_ptr ATTRIBUTE_LIFETIME_BOUND) noexcept {
    constexpr std::uint8_t remainders[201] =
        "0001020304050607080910111213141516171819"
        "2021222324252627282930313233343536373839"
        "4041424344454647484950515253545556575859"
        "6061626364656667686970717273747576777879"
        "8081828384858687888990919293949596979899";

    constexpr std::uint32_t kBase1 = 10;
    constexpr std::uint32_t kBase2 = kBase1 * kBase1;

    while (number >= kBase2) {
        const auto remainder_index = static_cast<size_t>(number % kBase2) * 2;
        number /= kBase2;
        *--buffer_ptr = static_cast<char>(remainders[remainder_index + 1]);
        *--buffer_ptr = static_cast<char>(remainders[remainder_index]);
    }

    if (number >= kBase1) {
        const auto remainder_index = static_cast<size_t>(number) * 2;
        *--buffer_ptr = static_cast<char>(remainders[remainder_index + 1]);
        *--buffer_ptr = static_cast<char>(remainders[remainder_index]);
    } else {
        *--buffer_ptr = static_cast<char>('0' + number);
    }

    return buffer_ptr;
}

struct FillBufferResult final {
    const char* written_str_begin;
    std::size_t written_str_size;

    [[nodiscard]] std::string to_string() const {
        return std::string{as_string_view()};
    }

    [[nodiscard]] std::wstring to_wstring() const {
        return std::wstring(written_str_begin, written_str_begin + written_str_size);
    }

    [[nodiscard]] constexpr std::string_view as_string_view() const noexcept {
        return std::string_view{written_str_begin, written_str_size};
    }
};

template <typename T>
using ArrayBufferForFormatting =
    std::conditional_t<std::is_same_v<T, uint128_t>,
                       std::array<char, int128_traits::detail::kMaxStringLengthU128>,
                       std::array<char, int128_traits::detail::kMaxStringLengthI128>>;

template <typename T>
[[nodiscard]]
I128_CONSTEXPR FillBufferResult
fill_buffer(const T number, ArrayBufferForFormatting<T>& buffer ATTRIBUTE_LIFETIME_BOUND) noexcept {
    char* const buffer_end_ptr = buffer.data() + buffer.size();

    char* ptr = int128_traits::detail::uint128_t_format_fill_chars_buffer(
        int128_traits::detail::uabs128(number), buffer_end_ptr);
    if constexpr (std::is_same_v<T, int128_t>) {
        if (number < 0) {
            *--ptr = '-';
        }
    }

    const auto length = static_cast<size_t>(buffer_end_ptr - ptr);
    if constexpr (std::is_same_v<T, int128_t>) {
        CONFIG_ASSUME_STATEMENT(2 <= length);
    } else {
        CONFIG_ASSUME_STATEMENT(1 <= length);
    }
    const size_t buffer_size = buffer.size();
    CONFIG_ASSUME_STATEMENT(length <= buffer_size);
    return {ptr, length};
}

template <class T>
struct is_integral_helper : public std::is_integral<T> {};

template <>
struct is_integral_helper<uint128_t> : public std::true_type {};

template <>
struct is_integral_helper<int128_t> : public std::true_type {};

template <class T>
struct is_signed_helper : public std::is_signed<T> {};

template <>
struct is_signed_helper<uint128_t> : public std::false_type {};

template <>
struct is_signed_helper<int128_t> : public std::true_type {};

template <class T>
struct is_unsigned_helper : public std::is_unsigned<T> {};

template <>
struct is_unsigned_helper<uint128_t> : public std::true_type {};

template <>
struct is_unsigned_helper<int128_t> : public std::false_type {};

template <class T>
struct is_arithmetic_helper : public std::is_arithmetic<T> {};

template <>
struct is_arithmetic_helper<uint128_t> : public std::true_type {};

template <>
struct is_arithmetic_helper<int128_t> : public std::true_type {};

}  // namespace detail

template <class T>
struct is_integral : public ::int128_traits::detail::is_integral_helper<std::remove_cv_t<T>> {};

template <class T>
struct make_unsigned {
    using type = typename std::make_unsigned_t<T>;
};

template <>
struct make_unsigned<uint128_t> {
    using type = uint128_t;
};

template <>
struct make_unsigned<const uint128_t> {
    using type = const uint128_t;
};

template <>
struct make_unsigned<volatile uint128_t> {
    using type = volatile uint128_t;
};

template <>
struct make_unsigned<const volatile uint128_t> {
    using type = const volatile uint128_t;
};

template <>
struct make_unsigned<int128_t> {
    using type = uint128_t;
};

template <>
struct make_unsigned<const int128_t> {
    using type = const uint128_t;
};

template <>
struct make_unsigned<volatile int128_t> {
    using type = volatile uint128_t;
};

template <>
struct make_unsigned<const volatile int128_t> {
    using type = const volatile uint128_t;
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
struct make_signed<const uint128_t> {
    using type = const int128_t;
};

template <>
struct make_signed<volatile uint128_t> {
    using type = volatile int128_t;
};

template <>
struct make_signed<const volatile uint128_t> {
    using type = const volatile int128_t;
};

template <>
struct make_signed<int128_t> {
    using type = int128_t;
};

template <>
struct make_signed<const int128_t> {
    using type = const int128_t;
};

template <>
struct make_signed<volatile int128_t> {
    using type = volatile int128_t;
};

template <>
struct make_signed<const volatile int128_t> {
    using type = const volatile int128_t;
};

template <class T>
struct is_unsigned : public ::int128_traits::detail::is_unsigned_helper<std::remove_cv_t<T>> {};

template <class T>
struct is_signed : public ::int128_traits::detail::is_signed_helper<std::remove_cv_t<T>> {};

template <class T>
struct is_arithmetic : public ::int128_traits::detail::is_arithmetic_helper<std::remove_cv_t<T>> {};

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
    int128_traits::detail::ArrayBufferForFormatting<uint128_t> digits{};
    return out << int128_traits::detail::fill_buffer(number, digits).as_string_view();
}

inline std::ostream& operator<<(std::ostream& out ATTRIBUTE_LIFETIME_BOUND, const int128_t number) {
    int128_traits::detail::ArrayBufferForFormatting<int128_t> digits{};
    return out << int128_traits::detail::fill_buffer(number, digits).as_string_view();
}

[[nodiscard]] inline std::string to_string(const uint128_t number) {
    int128_traits::detail::ArrayBufferForFormatting<uint128_t> digits{};
    return int128_traits::detail::fill_buffer(number, digits).to_string();
}

[[nodiscard]] inline std::string to_string(const int128_t number) {
    int128_traits::detail::ArrayBufferForFormatting<int128_t> digits{};
    return int128_traits::detail::fill_buffer(number, digits).to_string();
}

[[nodiscard]] inline std::wstring to_wstring(const uint128_t number) {
    int128_traits::detail::ArrayBufferForFormatting<uint128_t> digits{};
    return int128_traits::detail::fill_buffer(number, digits).to_wstring();
}

[[nodiscard]] inline std::wstring to_wstring(const int128_t number) {
    int128_traits::detail::ArrayBufferForFormatting<int128_t> digits{};
    return int128_traits::detail::fill_buffer(number, digits).to_wstring();
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
    typename FmtContext::iterator format(const int128_t n, FmtContext& ctx) const {
        std::string s = ::to_string(n);
        return std::copy(s.begin(), s.end(), ctx.out());
    }
};

#undef SPECIALIZE_STD_FORMAT

#endif

#endif  // !INTEGERS_128_BIT_HPP

#undef HAS_INT128_TYPEDEF

#endif  // HAS_INT128_TYPEDEF
