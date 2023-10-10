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

inline constexpr int128_t abs(int128_t x) noexcept {
    return x >= 0 ? x : -x;
}

/// @brief Count trailing zeros for n
/// @param n 
/// @return trailing zeros count (128 for n = 0)
template <typename T>
requires is_unsigned_v<T>
static inline constexpr int32_t count_trailing_zeros(T n) noexcept {
    if (n == 0) {
        return sizeof(T) * 8;
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

#if defined(__cpp_consteval) && __cpp_consteval >= 201811L
consteval
#else
inline static constexpr
#endif
uint32_t digits_count(__uint128_t number) noexcept {
    uint32_t cnt = 0;
    while (number) {
        number /= 10;
        cnt++;
    }

    return cnt;
}

inline std::ostream& operator<<(std::ostream& out, __uint128_t number) {
    // 340282366920938463463374607431768211455 == 2^128 - 1
    // strlen("340282366920938463463374607431768211455") == 39;
    constexpr size_t max_number_digits_count = 39;
    static_assert(digits_count(static_cast<__uint128_t>(-1)) == max_number_digits_count);

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

#if __GNUC__
    __ostream_insert(out, ptr, length);
#else
    out << std::string_view(ptr, length);
#endif
    return out;
}

};

#endif // !_INTEGERS_128_BIT_
