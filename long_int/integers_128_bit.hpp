/*
 * Small chunk of methods (like abs, std::ostream::operator<<, put_u128) and
 * template instantiations (like std::is_unsigned, std::make_signed)
 * for 128 bit width integers typedefed as uint128_t and int128_t.
 * 
 * This file is targeted for the g++ compiler. If your compiler supports
 * 128 bit integers but typedefes them differently from g++ (__uint128_t and __int128_t),
 * then you should change typedefs in the begining of this file.
 *
 */

#ifndef _INTEGERS_128_BIT_
#define _INTEGERS_128_BIT_ 1

#include <cstdint>
#include <cmath>
#include <type_traits>
#include <ostream>

typedef __uint128_t uint128_t;
typedef __int128_t int128_t;

namespace std {

template<>
struct make_unsigned<uint128_t> {
    typedef uint128_t type;
};

template<>
struct make_unsigned<int128_t> {
    typedef uint128_t type;
};

template<>
struct is_unsigned<uint128_t> {
    static constexpr bool value = true;
};

template<>
struct is_signed<int128_t> {
    static constexpr bool value = true;
};

static_assert(is_unsigned_v<uint128_t>);
static_assert(is_signed_v<int128_t>);
static_assert(is_same_v<make_unsigned_t<int128_t>, uint128_t>);

constexpr int128_t abs(int128_t x) noexcept {
    return x >= 0 ? x : -x;
}

/// @brief Count trailing zeros for n
/// @param n 
/// @return trailing zeros count (sizeof(n) * 8 for n = 0)
template <typename T>
#if __cplusplus >= 202002L
requires is_unsigned_v<T>
#endif
static inline constexpr int32_t count_trailing_zeros(T n) noexcept {
    if (n == 0) {
        return sizeof(n) * 8;
    }

    if constexpr (is_same_v<T, uint128_t>) {
        uint64_t low = static_cast<uint64_t>(n);
        if (low != 0) {
            return __builtin_ctzll(low);
        }

        uint64_t high = static_cast<uint64_t>(n >> 64);
        return __builtin_ctzll(high) + 64;
    }
    else if constexpr (is_same_v<T, unsigned long long>) {
        return __builtin_ctzll(n);
    }
    else if constexpr (is_same_v<T, unsigned long>) {
        return __builtin_ctzl(n);
    }
    else if constexpr (is_same_v<T, unsigned int>
        || is_same_v<T, unsigned short>
        || is_same_v<T, unsigned char>) {
        return __builtin_ctz(n);
    }
    else {
        static_assert(is_same_v<T, bool>, "unknown unsigned integer in int32_t count_trailing_zeros(T)");
        return 0;
    }
}

/// @brief Count leading zeros for n
/// @param n 
/// @return leading zeros count (sizeof(n) * 8 for n = 0)
template <typename T>
#if __cplusplus >= 202002L
requires is_unsigned_v<T>
#endif
static constexpr int32_t count_leading_zeros(T n) noexcept {
    if (n == 0) {
        return sizeof(n) * 8;
    }

    if constexpr (is_same_v<T, uint128_t>) {
        uint64_t hi = static_cast<uint64_t>(n >> 64);
        if (hi != 0) {
            return __builtin_clzll(hi);
        }

        uint64_t low = static_cast<uint64_t>(n);
        return 64 + __builtin_clzll(low);
    }
    else if constexpr (is_same_v<T, unsigned long long>) {
        return __builtin_clzll(n);
    }
    else if constexpr (is_same_v<T, unsigned long>) {
        return __builtin_clzl(n);
    }
    else if constexpr (is_same_v<T, unsigned int>
        || is_same_v<T, unsigned short>
        || is_same_v<T, unsigned char>) {
        return __builtin_clz(n);
    }
    else {
        static_assert(is_same_v<T, bool>, "unknown unsigned integer in int32_t count_trailing_zeros(T)");
        return 0;
    }
}

constexpr size_t NearestTwoPowGreaterEqual(size_t n) noexcept {
    if constexpr (sizeof(size_t) == sizeof(unsigned long long)) {
        return 1ull << (64 - __builtin_clzll(n | 1) - ((n & (n - 1)) == 0));
    }
    else if constexpr (sizeof(size_t) == sizeof(unsigned)) {
        return 1ull << (32 - __builtin_clz(n | 1) - ((n & (n - 1)) == 0));
    }
    else {
        __builtin_unreachable();
        return 0;
    }
}

constexpr bool IsDigit(int32_t c) noexcept {
    return static_cast<uint32_t>(c) - '0' <= '9' - '0';
}

constexpr uint32_t BaseTwoDigits(uint32_t n) noexcept {
    // " | 1" operation does not affect the answer for all numbers except n = 0
    // for n = 0 answer is 1
    return uint32_t(32 - __builtin_clz(n | 1));
}

constexpr uint32_t BaseTwoDigits(uint64_t n) noexcept {
    // " | 1" operation does not affect the answer for all numbers except n = 0
    // for n = 0 answer is 1
    return uint32_t(64 - __builtin_clzll(n | 1));
}

constexpr uint32_t BaseTenDigits(uint32_t n) noexcept {
    static constexpr unsigned char guess[33] = {
        0, 0, 0, 0, 1, 1, 1, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 5, 5, 5,
        6, 6, 6, 6, 7, 7, 7, 8, 8, 8,
        9, 9, 9
    };
    static constexpr uint32_t ten_to_the[10] = {
        1, 10, 100, 1000, 10000, 100000, 
        1000000, 10000000, 100000000, 1000000000,
    };
    uint32_t digits = guess[BaseTwoDigits(n)];
    // returns 1 for n = 0. If you want to return 0 for n = 0, remove | 1
    return digits + ((n | 1) >= ten_to_the[digits]);
}

constexpr uint32_t BaseTenDigits(uint64_t n) {
    uint32_t ans = 0;
    do {
        n /= 10;
        ans++;
    } while (n != 0);
    return ans;
}

constexpr uint32_t BaseTenDigits(uint128_t number) noexcept {
    uint32_t cnt = 0;
    do {
        number /= 10;
        cnt++;
    } while (number);

    return cnt;
}

// static_assert(BaseTenDigits(0u) == 1);
// static_assert(BaseTenDigits(1u) == 1);
// static_assert(BaseTenDigits(9u) == 1);
// static_assert(BaseTenDigits(10u) == 2);
// static_assert(BaseTenDigits(11u) == 2);
// static_assert(BaseTenDigits(99u) == 2);
// static_assert(BaseTenDigits(100u) == 3);
// static_assert(BaseTenDigits(101u) == 3);
// static_assert(BaseTenDigits(uint32_t(-1)) == 10);

static_assert(BaseTenDigits(0ull) == 1);
static_assert(BaseTenDigits(1ull) == 1);
static_assert(BaseTenDigits(9ull) == 1);
static_assert(BaseTenDigits(10ull) == 2);
static_assert(BaseTenDigits(11ull) == 2);
static_assert(BaseTenDigits(99ull) == 2);
static_assert(BaseTenDigits(100ull) == 3);
static_assert(BaseTenDigits(101ull) == 3);
static_assert(BaseTenDigits(uint64_t(-1)) == 20);

static_assert(BaseTenDigits(uint128_t(0)) == 1);
static_assert(BaseTenDigits(uint128_t(1)) == 1);
static_assert(BaseTenDigits(uint128_t(9)) == 1);
static_assert(BaseTenDigits(uint128_t(10)) == 2);
static_assert(BaseTenDigits(uint128_t(11)) == 2);
static_assert(BaseTenDigits(uint128_t(99)) == 2);
static_assert(BaseTenDigits(uint128_t(100)) == 3);
static_assert(BaseTenDigits(uint128_t(101)) == 3);
static_assert(BaseTenDigits(uint128_t(-1)) == 39);

inline std::ostream& operator<<(std::ostream& out, uint128_t number) {
    // 340282366920938463463374607431768211455 == 2^128 - 1
    // strlen("340282366920938463463374607431768211455") == 39;
    constexpr size_t max_number_digits_count = 39;
    static_assert(BaseTenDigits(static_cast<uint128_t>(-1)) == max_number_digits_count);

    char digits[max_number_digits_count + 1];
    digits[max_number_digits_count] = '\0';
    char* ptr = &digits[max_number_digits_count];
    size_t length = 0;
    do {
        auto r = number / 10;
        auto q = number - r * 10;
        *--ptr = static_cast<char>('0' + static_cast<uint64_t>(q));
        length++;
        number = r;
    } while (number);


#if __GNUC__ && !defined(__clang__)
    __ostream_insert(out, ptr, length);
#else
    out << std::string_view(ptr, length);
#endif
    return out;
}

inline int put_u128(uint128_t number) noexcept {
    // 340282366920938463463374607431768211455 == 2^128 - 1
    // strlen("340282366920938463463374607431768211455") == 39;
    constexpr size_t max_number_digits_count = 39;
    static_assert(BaseTenDigits(static_cast<uint128_t>(-1)) == max_number_digits_count);

    char digits[max_number_digits_count + 1];
    digits[max_number_digits_count] = '\0';
    char* ptr = &digits[max_number_digits_count];
    do {
        auto r = number / 10;
        auto q = number - r * 10;
        *--ptr = static_cast<char>('0' + static_cast<uint64_t>(q));
        number = r;
    } while (number);
    return fputs(ptr, stdout);
}

inline int put_u128_newline(uint128_t number) noexcept {
    // 340282366920938463463374607431768211455 == 2^128 - 1
    // strlen("340282366920938463463374607431768211455") == 39;
    constexpr size_t max_number_digits_count = 39;
    static_assert(BaseTenDigits(static_cast<uint128_t>(-1)) == max_number_digits_count);

    char digits[max_number_digits_count + 1];
    digits[max_number_digits_count] = '\0';
    char* ptr = &digits[max_number_digits_count];
    do {
        auto r = number / 10;
        auto q = number - r * 10;
        *--ptr = static_cast<char>('0' + static_cast<uint64_t>(q));
        number = r;
    } while (number);
    return puts(ptr);
}

inline string to_string(uint128_t number) {
    // See functions above
    constexpr size_t max_number_digits_count = 39;
    char digits[max_number_digits_count + 1];
    digits[max_number_digits_count] = '\0';
    char* ptr = &digits[max_number_digits_count];
    size_t length = 0;
    do {
        auto r = number / 10;
        auto q = number - r * 10;
        *--ptr = static_cast<char>('0' + static_cast<uint64_t>(q));
        length++;
        number = r;
    } while (number);

    return std::string(ptr, length);
}

};

#endif // !_INTEGERS_128_BIT_

