/*
 * Small chunk of functions (like std::ostream::operator<<, print_u128) and
 * template instantiations (like std::is_unsigned, std::make_unsigned)
 * for 128 bit width integers typedefed as uint128_t and int128_t.
 *
 * This file is targeted for the g++ compiler. If your compiler supports
 * 128 bit integers but typedefes them differently from g++ (__uint128_t and
 * __int128_t), then you should change typedefs in the begining of this file.
 *
 */

#ifndef INTEGERS_128_BIT
#define INTEGERS_128_BIT 1

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

#if defined(__GNUC__)
#if defined(likely)
#undef likely
#endif
#define likely(x) __builtin_expect(static_cast<bool>(x), true)
#if defined(unlikely)
#undef unlikely
#endif
#define unlikely(x) __builtin_expect(static_cast<bool>(x), false)
#else
#if !defined(likely)
#define likely(x) static_cast<bool>(x)
#endif
#if !defined(unlikely)
#define unlikely(x) static_cast<bool>(x)
#endif
#endif


namespace format_impl_uint128_t {

#if __cplusplus >= 202207L && defined(__GNUC__) && !defined(__clang__)
constexpr
#endif
    static inline char*
    uint128_t_format_fill_chars_buffer(uint128_t number, char* buffer_ptr) noexcept {
    do { /**
          * let compiler optimize it like "q = number - r * 10"
          * or whatever (maybe / 10 and % 10 will be
          * calculated at once for uint128_t too)
          */
        auto r = number / 10;
        auto q = number % 10;
        *--buffer_ptr = static_cast<char>('0' + static_cast<uint64_t>(q));
        number = r;
    } while (number);
    return buffer_ptr;
}

}  // namespace format_impl_uint128_t

namespace std {

template <>
struct is_integral<int128_t> {
    static constexpr bool value = true;
};

template <>
struct make_unsigned<uint128_t> {
    typedef uint128_t type;
};

template <>
struct make_unsigned<int128_t> {
    typedef uint128_t type;
};

template <>
struct is_unsigned<uint128_t> {
    static constexpr bool value = true;
};

template <>
struct is_signed<int128_t> {
    static constexpr bool value = true;
};

template <>
inline constexpr bool is_arithmetic_v<int128_t> = true;

template <>
inline constexpr bool is_arithmetic_v<uint128_t> = true;

// mingw-64 g++ CE
// template <>
// inline constexpr bool is_integral_v<int128_t> = true;
// template <>
// inline constexpr bool is_integral_v<uint128_t> = true;

template <>
inline constexpr bool is_default_constructible_v<int128_t> = true;

template <>
inline constexpr bool is_copy_constructible_v<int128_t> = true;

template <>
inline constexpr bool is_move_constructible_v<int128_t> = true;

template <>
inline constexpr bool is_copy_assignable_v<int128_t> = true;

template <>
inline constexpr bool is_move_assignable_v<int128_t> = true;

template <>
inline constexpr bool is_default_constructible_v<uint128_t> = true;

template <>
inline constexpr bool is_copy_constructible_v<uint128_t> = true;

template <>
inline constexpr bool is_move_constructible_v<uint128_t> = true;

template <>
inline constexpr bool is_copy_assignable_v<uint128_t> = true;

template <>
inline constexpr bool is_move_assignable_v<uint128_t> = true;

template <>
inline constexpr bool is_unsigned_v<uint128_t> = true;

template <>
inline constexpr bool is_signed_v<int128_t> = true;

static_assert(is_arithmetic_v<int128_t>);
// static_assert(is_integral_v<int128_t>);
static_assert(is_arithmetic_v<uint128_t>);
static_assert(is_unsigned_v<uint128_t>);
static_assert(is_signed_v<int128_t>);
static_assert(is_same_v<make_unsigned_t<int128_t>, uint128_t>);

inline ostream& operator<<(ostream& out, uint128_t number) {
    // 340282366920938463463374607431768211455 == 2^128 - 1
    // strlen("340282366920938463463374607431768211455") == 39;
    constexpr size_t max_number_digits_count = 39;

    char digits[max_number_digits_count + 1];
    digits[max_number_digits_count] = '\0';
    const char* ptr = format_impl_uint128_t::uint128_t_format_fill_chars_buffer(
        number, &digits[max_number_digits_count]);
    size_t length = static_cast<size_t>(&digits[max_number_digits_count] - ptr);
    return out << string_view(ptr, length);
}

inline int print_u128(uint128_t number) noexcept {
    // 340282366920938463463374607431768211455 == 2^128 - 1
    // strlen("340282366920938463463374607431768211455") == 39;
    constexpr size_t max_number_digits_count = 39;
    char digits[max_number_digits_count + 1];
    digits[max_number_digits_count] = '\0';
    const char* ptr = format_impl_uint128_t::uint128_t_format_fill_chars_buffer(
        number, &digits[max_number_digits_count]);
    return fputs(ptr, stdout);
}

inline int print_u128_newline(uint128_t number) noexcept {
    // 340282366920938463463374607431768211455 == 2^128 - 1
    // strlen("340282366920938463463374607431768211455") == 39;
    constexpr size_t max_number_digits_count = 39;
    char digits[max_number_digits_count + 1];
    digits[max_number_digits_count] = '\0';
    const char* ptr = format_impl_uint128_t::uint128_t_format_fill_chars_buffer(
        number, &digits[max_number_digits_count]);
    return puts(ptr);
}

inline string to_string(uint128_t number) {
    // 340282366920938463463374607431768211455 == 2^128 - 1
    // strlen("340282366920938463463374607431768211455") == 39;
    constexpr size_t max_number_digits_count = 39;
    char digits[max_number_digits_count + 1];
    digits[max_number_digits_count] = '\0';

    const char* ptr = format_impl_uint128_t::uint128_t_format_fill_chars_buffer(
        number, &digits[max_number_digits_count]);
    size_t length = static_cast<size_t>(&digits[max_number_digits_count] - ptr);

    return string(ptr, length);
}

inline string to_string(int128_t number) {
    // 340282366920938463463374607431768211455 == 2^128 - 1
    // strlen("340282366920938463463374607431768211455") == 39;
    constexpr size_t max_number_digits_count = 39;
    // + 1 for sign
    char digits[max_number_digits_count + 1 + 1];
    digits[max_number_digits_count + 1] = '\0';

    bool negative = number < 0;
    if (negative) {
        number = -number;
    }

    char* ptr = format_impl_uint128_t::uint128_t_format_fill_chars_buffer(
        uint128_t(number), &digits[max_number_digits_count + 1]);
    if (negative) {
        *--ptr = '-';
    }
    size_t length =
        static_cast<size_t>(&digits[max_number_digits_count + 1] - ptr);

    return string(ptr, length);
}

inline ostream& operator<<(ostream& out, int128_t number) {
    // 340282366920938463463374607431768211455 == 2^128 - 1
    // strlen("340282366920938463463374607431768211455") == 39;
    constexpr size_t max_number_digits_count = 39;
    // + 1 for sign
    char digits[max_number_digits_count + 1 + 1];
    digits[max_number_digits_count + 1] = '\0';

    bool negative = number < 0;
    if (negative) {
        number = -number;
    }

    char* ptr = format_impl_uint128_t::uint128_t_format_fill_chars_buffer(
        uint128_t(number), &digits[max_number_digits_count + 1]);
    if (negative) {
        *--ptr = '-';
    }
    size_t length =
        static_cast<size_t>(&digits[max_number_digits_count + 1] - ptr);

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

#endif  // !INTEGERS_128_BIT
