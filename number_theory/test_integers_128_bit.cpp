#include <cassert>
#include <cstdint>

#include "integers_128_bit.hpp"

namespace i128_helper = type_traits_helper_int128_t;

static_assert(i128_helper::is_arithmetic_v<int128_t>);
static_assert(i128_helper::is_integral_v<int128_t>);
static_assert(i128_helper::is_arithmetic_v<uint128_t>);
static_assert(i128_helper::is_integral_v<uint128_t>);
static_assert(i128_helper::is_unsigned_v<uint128_t>);
static_assert(!i128_helper::is_unsigned_v<int128_t>);
static_assert(i128_helper::is_signed_v<int128_t>);
static_assert(!i128_helper::is_signed_v<uint128_t>);
static_assert(std::is_same_v<i128_helper::make_unsigned_t<int128_t>, uint128_t>);
static_assert(std::is_same_v<i128_helper::make_unsigned_t<uint128_t>, uint128_t>);

static_assert(i128_helper::is_arithmetic_v<int64_t>);
static_assert(i128_helper::is_integral_v<int64_t>);
static_assert(!i128_helper::is_unsigned_v<int64_t>);
static_assert(i128_helper::is_signed_v<int64_t>);
static_assert(std::is_same_v<i128_helper::make_unsigned_t<int64_t>, uint64_t>);

static_assert(i128_helper::is_arithmetic_v<uint64_t>);
static_assert(i128_helper::is_integral_v<uint64_t>);
static_assert(i128_helper::is_unsigned_v<uint64_t>);
static_assert(!i128_helper::is_signed_v<uint64_t>);
static_assert(std::is_same_v<i128_helper::make_unsigned_t<uint64_t>, uint64_t>);

int main() {
    constexpr uint64_t k = 20000;
    for (uint64_t n = 0; n <= k; n++) {
        assert(std::to_string(uint128_t(n)) == std::to_string(n));
        assert(std::to_string(int128_t(n)) == std::to_string(n));
    }

    for (int64_t n = -int64_t(k); n <= 0; n++) {
        assert(std::to_string(int128_t(n)) == std::to_string(n));
    }

    for (uint64_t n = UINT64_MAX; n >= UINT64_MAX - k; n--) {
        assert(std::to_string(uint128_t(n)) == std::to_string(n));
        assert(std::to_string(int128_t(n)) == std::to_string(n));
    }

    for (int64_t n = INT64_MIN; n <= INT64_MIN + int64_t(k); n++) {
        assert(std::to_string(int128_t(n)) == std::to_string(n));
    }

    assert(std::to_string(uint128_t(-1)) == "340282366920938463463374607431768211455");
    assert(std::to_string(uint128_t(1) << 127) == "170141183460469231731687303715884105728");
    assert(std::to_string(int128_t((uint128_t(1) << 127) - 1)) ==
           "170141183460469231731687303715884105727");
    assert(std::to_string(int128_t(uint128_t(1) << 127)) ==
           "-170141183460469231731687303715884105728");
}
