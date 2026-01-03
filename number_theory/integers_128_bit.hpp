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
#include "../misc/ints_fmt.hpp"

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

#ifdef HAS_INT128_TYPEDEF

#include <array>
#include <climits>
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

namespace ints_fmt {

using Int128Formatter = Formatter<int128_t, uint128_t>;
using UInt128Formatter = Formatter<uint128_t, uint128_t>;

}  // namespace ints_fmt

namespace int128_traits {

namespace detail {

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
struct is_integral : public int128_traits::detail::is_integral_helper<std::remove_cv_t<T>> {};

template <class T>
struct make_unsigned : public std::make_unsigned<T> {};

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
struct make_signed : public std::make_signed<T> {};

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
struct is_unsigned : public int128_traits::detail::is_unsigned_helper<std::remove_cv_t<T>> {};

template <class T>
struct is_signed : public int128_traits::detail::is_signed_helper<std::remove_cv_t<T>> {};

template <class T>
struct is_arithmetic : public int128_traits::detail::is_arithmetic_helper<std::remove_cv_t<T>> {};

template <class T>
inline constexpr bool is_integral_v = int128_traits::is_integral<T>::value;

template <class T>
inline constexpr bool is_unsigned_v = int128_traits::is_unsigned<T>::value;

template <class T>
inline constexpr bool is_signed_v = int128_traits::is_signed<T>::value;

template <class T>
inline constexpr bool is_arithmetic_v = int128_traits::is_arithmetic<T>::value;

template <typename T>
using make_unsigned_t = typename int128_traits::make_unsigned<T>::type;

template <typename T>
using make_signed_t = typename int128_traits::make_signed<T>::type;

#if CONFIG_HAS_CONCEPTS

template <class T>
concept integral = std::integral<T> || int128_traits::is_integral_v<T>;

template <class T>
concept signed_integral = std::signed_integral<T> || (int128_traits::integral<T> && int128_traits::is_signed_v<T>);

template <class T>
concept unsigned_integral =
    std::unsigned_integral<T> || (int128_traits::integral<T> && int128_traits::is_unsigned_v<T>);

#endif

}  // namespace int128_traits

inline std::ostream& operator<<(std::ostream& out ATTRIBUTE_LIFETIME_BOUND, const uint128_t number) {
    return out << ints_fmt::UInt128Formatter{number}.as_string_view();
}

inline std::ostream& operator<<(std::ostream& out ATTRIBUTE_LIFETIME_BOUND, const int128_t number) {
    return out << ints_fmt::Int128Formatter{number}.as_string_view();
}

[[nodiscard]] inline std::string to_string(const uint128_t number) {
    return ints_fmt::UInt128Formatter{number}.as_string();
}

[[nodiscard]] inline std::string to_string(const int128_t number) {
    return ints_fmt::Int128Formatter{number}.as_string();
}

[[nodiscard]] inline std::wstring to_wstring(const uint128_t number) {
    return ints_fmt::UInt128Formatter{number}.as_wstring();
}

[[nodiscard]] inline std::wstring to_wstring(const int128_t number) {
    return ints_fmt::Int128Formatter{number}.as_wstring();
}

#if defined(SPECIALIZE_STD_FORMAT)

template <class CharT>
struct std::formatter<uint128_t, CharT> {  // NOLINT(cert-dcl58-cpp)
    template <class ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <class FmtContext>
    typename FmtContext::iterator format(const uint128_t n, FmtContext& ctx) const {
        const ints_fmt::UInt128Formatter f{n};
        const std::string_view s = f.as_string_view();
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
        const ints_fmt::Int128Formatter f{n};
        const std::string_view s = f.as_string_view();
        return std::copy(s.begin(), s.end(), ctx.out());
    }
};

#undef SPECIALIZE_STD_FORMAT

#endif

#endif
