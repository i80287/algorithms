/*
 * Small chunk of functions (like std::ostream::operator<<, print_u128) and
 * template instantiations (like type_traits_helper_int128_t::is_unsigned,
 * type_traits_helper_int128_t::make_unsigned) for 128 bit width integers
 * typedefed as uint128_t and int128_t.
 *
 * This file is targeted for the g++ compiler. If your compiler supports
 * 128 bit integers but typedefes them differently from g++ (__uint128_t and
 * __int128_t), then you should change typedefs in the begining of this file.
 *
 */

#ifndef INTEGERS_128_BIT_HPP
#define INTEGERS_128_BIT_HPP 1

#include <cstdint>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#if __cplusplus >= 202002L && defined(__GNUC__) && !defined(__clang__)
#if __has_include("format")
#include <format>
#endif
#endif

#if defined(__GNUC__)
typedef __uint128_t uint128_t;
typedef __int128_t int128_t;
#elif defined(_MSC_VER)
#include <__msvc_int128.hpp>
typedef std::_Unsigned128 uint128_t;
typedef std::_Signed128 int128_t;
#else
#error "typedef 128-bit integer specific for your compiler"
#endif

#include "config_macros.hpp"

/**
 * Macro defined to 1 if current [u]int128_t supports constexpr
 * operations and 0 otherwise.
 */
#if (__cplusplus >= 201703L && defined(__GNUC__)) || (__cplusplus >= 202002L && defined(_MSC_VER))
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
#define I128_CONSTEXPR
#endif

namespace format_impl_uint128_t {

#if __cplusplus >= 202002L
// 340282366920938463463374607431768211455 == 2^128 - 1
static_assert(std::char_traits<char>::length("340282366920938463463374607431768211455") == 39, "");
//  170141183460469231731687303715884105727 ==  2^127 - 1
// -170141183460469231731687303715884105728 == -2^127
static_assert(std::char_traits<char>::length("-170141183460469231731687303715884105728") == 40, "");
#endif
inline constexpr size_t kMaxStringLengthU128 = 39;
inline constexpr size_t kMaxStringLengthI128 = 40;

/// @brief Realization taken from the gcc libstdc++ __to_chars_10_impl
/// @param number
/// @param buffer_ptr
/// @return
inline I128_CONSTEXPR char* uint128_t_format_fill_chars_buffer(uint128_t number,
                                                               char* buffer_ptr) noexcept {
    constexpr uint8_t remainders[201] =
        "0001020304050607080910111213141516171819"
        "2021222324252627282930313233343536373839"
        "4041424344454647484950515253545556575859"
        "6061626364656667686970717273747576777879"
        "8081828384858687888990919293949596979899";

    while (number >= 100) {
        const size_t remainder_index = size_t(number % 100) * 2;
        number /= 100;
        *--buffer_ptr = char(remainders[remainder_index + 1]);
        *--buffer_ptr = char(remainders[remainder_index]);
    }

    if (number >= 10) {
        const size_t remainder_index = size_t(number) * 2;
        *--buffer_ptr                = char(remainders[remainder_index + 1]);
        *--buffer_ptr                = char(remainders[remainder_index]);
    } else {
        *--buffer_ptr = char('0' + number);
    }

    return buffer_ptr;
}

}  // namespace format_impl_uint128_t

namespace type_traits_helper_int128_t {

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
    using type = std::make_unsigned_t<T>;
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
inline constexpr bool is_arithmetic_v = std::is_arithmetic_v<T>;

template <>
inline constexpr bool is_arithmetic_v<int128_t> = true;

template <>
inline constexpr bool is_arithmetic_v<uint128_t> = true;

template <class T>
inline constexpr bool is_integral_v = is_integral<T>::value;

template <class T>
inline constexpr bool is_default_constructible_v = std::is_default_constructible_v<T>;

template <>
inline constexpr bool is_default_constructible_v<uint128_t> = true;

template <>
inline constexpr bool is_default_constructible_v<int128_t> = true;

template <class T>
inline constexpr bool is_copy_constructible_v = std::is_copy_constructible_v<T>;

template <>
inline constexpr bool is_copy_constructible_v<uint128_t> = true;

template <>
inline constexpr bool is_copy_constructible_v<int128_t> = true;

template <class T>
inline constexpr bool is_move_constructible_v = std::is_move_constructible_v<T>;

template <>
inline constexpr bool is_move_constructible_v<int128_t> = true;

template <>
inline constexpr bool is_move_constructible_v<uint128_t> = true;

template <class T>
inline constexpr bool is_copy_assignable_v = std::is_copy_assignable_v<T>;

template <>
inline constexpr bool is_copy_assignable_v<int128_t> = true;

template <>
inline constexpr bool is_copy_assignable_v<uint128_t> = true;

template <class T>
inline constexpr bool is_move_assignable_v = std::is_move_assignable_v<T>;

template <>
inline constexpr bool is_move_assignable_v<int128_t> = true;

template <>
inline constexpr bool is_move_assignable_v<uint128_t> = true;

template <class T>
inline constexpr bool is_unsigned_v = is_unsigned<T>::value;

template <class T>
inline constexpr bool is_signed_v = is_signed<T>::value;

template <typename T>
using make_unsigned_t = typename make_unsigned<T>::type;

}  // namespace type_traits_helper_int128_t

namespace std {

inline ostream& operator<<(ostream& out, uint128_t number) {
    using namespace format_impl_uint128_t;

    // + 1 for '\0'
    constexpr auto buffer_size = kMaxStringLengthU128 + 1;

    char digits[buffer_size];
    digits[buffer_size - 1] = '\0';
    const char* ptr         = uint128_t_format_fill_chars_buffer(number, &digits[buffer_size - 1]);
    size_t length           = static_cast<size_t>(&digits[buffer_size - 1] - ptr);
    return out << string_view(ptr, length);
}

inline int fprint_u128(uint128_t number, FILE* filestream) noexcept {
    using namespace format_impl_uint128_t;

    // + 1 for '\0'
    constexpr auto buffer_size = kMaxStringLengthU128 + 1;
    char digits[buffer_size];
    digits[buffer_size - 1] = '\0';
    const char* ptr         = uint128_t_format_fill_chars_buffer(number, &digits[buffer_size - 1]);
    return fputs(ptr, filestream);
}

inline int print_u128(uint128_t number) noexcept {
    return fprint_u128(number, stdout);
}

inline int fprint_u128_newline(uint128_t number, FILE* filestream) noexcept {
    using namespace format_impl_uint128_t;

    // + 1 for '\0', + 1 for '\n'
    constexpr auto buffer_size = kMaxStringLengthU128 + 1 + 1;
    char digits[buffer_size];
    digits[buffer_size - 2] = '\n';
    digits[buffer_size - 1] = '\0';
    const char* ptr         = uint128_t_format_fill_chars_buffer(number, &digits[buffer_size - 2]);
    return fputs(ptr, filestream);
}

inline int print_u128_newline(uint128_t number) noexcept {
    using namespace format_impl_uint128_t;

    // + 1 for '\0'
    constexpr auto buffer_size = kMaxStringLengthU128 + 1;
    char digits[buffer_size];
    digits[buffer_size - 1] = '\0';
    const char* ptr         = uint128_t_format_fill_chars_buffer(number, &digits[buffer_size - 1]);
    return puts(ptr);
}

inline string to_string(uint128_t number) {
    using namespace format_impl_uint128_t;

    // + 1 for '\0'
    constexpr auto buffer_size = kMaxStringLengthU128 + 1;
    char digits[buffer_size];
    digits[buffer_size - 1] = '\0';

    const char* ptr = uint128_t_format_fill_chars_buffer(number, &digits[buffer_size - 1]);
    size_t length   = static_cast<size_t>(&digits[buffer_size - 1] - ptr);

    return string(ptr, length);
}

inline string to_string(int128_t number) {
    using namespace format_impl_uint128_t;

    // + 1 for '\0'
    constexpr auto buffer_size = kMaxStringLengthI128 + 1;
    char digits[buffer_size];
    digits[buffer_size - 1] = '\0';

    uint128_t number_m = number >= 0 ? uint128_t(number) : -uint128_t(number);
    char* ptr          = uint128_t_format_fill_chars_buffer(number_m, &digits[buffer_size - 1]);
    if (number < 0) {
        *--ptr = '-';
    }
    size_t length = static_cast<size_t>(&digits[buffer_size - 1] - ptr);

    return string(ptr, length);
}

inline ostream& operator<<(ostream& out, int128_t number) {
    using namespace format_impl_uint128_t;

    // + 1 for '\0'
    constexpr auto buffer_size = kMaxStringLengthI128 + 1;
    char digits[buffer_size];
    digits[buffer_size - 1] = '\0';

    const bool negative = number < 0;
    if (negative) {
        number = -number;
    }

    char* ptr = uint128_t_format_fill_chars_buffer(uint128_t(number), &digits[buffer_size - 1]);
    if (negative) {
        *--ptr = '-';
    }
    size_t length = static_cast<size_t>(&digits[buffer_size - 1] - ptr);

    return out << string_view(ptr, length);
}

#if __cplusplus >= 202002L && defined(__GNUC__) && !defined(__clang__)
#if __has_include("format")
template <class CharT>
struct formatter<uint128_t, CharT> {
    template <class ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <class FmtContext>
    typename FmtContext::iterator format(uint128_t n, FmtContext& ctx) const {
        string s = to_string(n);
        return copy(s.begin(), s.end(), ctx.out());
    }
};
template <class CharT>
struct formatter<int128_t, CharT> {
    template <class ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <class FmtContext>
    typename FmtContext::iterator format(int128_t n, FmtContext& ctx) const {
        string s = to_string(n);
        return copy(s.begin(), s.end(), ctx.out());
    }
};
#endif
#endif

}  // namespace std

#endif  // !INTEGERS_128_BIT_HPP
