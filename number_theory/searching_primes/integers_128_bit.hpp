#ifndef _INTEGERS_128_BIT_
#define _INTEGERS_128_BIT_ 1

#include <cstdint>
#include <cmath>
#include <type_traits>

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
requires std::is_unsigned_v<T>
static constexpr int32_t count_trailing_zeros(T n) noexcept {
    if constexpr (std::is_same_v<T, uint128_t>) {
        uint64_t high_or_low = static_cast<uint64_t>(n); // low

        if (high_or_low == 0)
        {// low part of n consist of zeros.
            if ((high_or_low = static_cast<uint64_t>(n >> 64)) != 0)
            {// high part of n does not consist of zeros.
                return __builtin_ctzll(high_or_low) + 64;
            }

            // else n is equal to 0.
            return 128;
        }

        // low part of n does not consist of zeros.
        return __builtin_ctzll(high_or_low);
    }
    else if constexpr (std::is_same_v<T, unsigned long long>) {
        return __builtin_ctzll(n);
    }
    else if constexpr (std::is_same_v<T, unsigned long>) {
        return __builtin_ctzl(n);
    }
    else if constexpr (std::is_same_v<T, unsigned int>
        || std::is_same_v<T, unsigned short>
        || std::is_same_v<T, unsigned char>) {
        return __builtin_ctz(n);
    }
    else {
        static_assert(std::is_same_v<T, bool>, "unknown unsigned integer in int32_t std::count_trailing_zeros(T)");
        return !n; // n ? 0 : 1;
    }
}

};

#endif // !_INTEGERS_128_BIT_
