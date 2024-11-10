#include <cassert>
#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>

#include "config_macros.hpp"
#include "integers_128_bit.hpp"
#include "test_tools.hpp"

namespace {

using std::int64_t;
using std::uint64_t;

static_assert(int128_traits::is_arithmetic_v<uint128_t>);
static_assert(int128_traits::is_integral_v<uint128_t>);
static_assert(int128_traits::is_unsigned_v<uint128_t>);
static_assert(!int128_traits::is_signed_v<uint128_t>);
static_assert(std::is_same_v<int128_traits::make_unsigned_t<uint128_t>, uint128_t>);
static_assert(std::is_same_v<int128_traits::make_signed_t<uint128_t>, int128_t>);

static_assert(int128_traits::is_arithmetic_v<int128_t>);
static_assert(int128_traits::is_integral_v<int128_t>);
static_assert(!int128_traits::is_unsigned_v<int128_t>);
static_assert(int128_traits::is_signed_v<int128_t>);
static_assert(std::is_same_v<int128_traits::make_unsigned_t<int128_t>, uint128_t>);
static_assert(std::is_same_v<int128_traits::make_signed_t<int128_t>, int128_t>);

static_assert(int128_traits::is_arithmetic_v<uint64_t>);
static_assert(int128_traits::is_integral_v<uint64_t>);
static_assert(int128_traits::is_unsigned_v<uint64_t>);
static_assert(!int128_traits::is_signed_v<uint64_t>);
static_assert(std::is_same_v<int128_traits::make_unsigned_t<uint64_t>, uint64_t>);
static_assert(std::is_same_v<int128_traits::make_signed_t<uint64_t>, int64_t>);

static_assert(int128_traits::is_arithmetic_v<int64_t>);
static_assert(int128_traits::is_integral_v<int64_t>);
static_assert(!int128_traits::is_unsigned_v<int64_t>);
static_assert(int128_traits::is_signed_v<int64_t>);
static_assert(std::is_same_v<int128_traits::make_unsigned_t<int64_t>, uint64_t>);
static_assert(std::is_same_v<int128_traits::make_signed_t<int64_t>, int64_t>);

#if CONFIG_HAS_CONCEPTS

static_assert(int128_traits::integral<int128_t>);
static_assert(!int128_traits::unsigned_integral<int128_t>);
static_assert(int128_traits::signed_integral<int128_t>);

static_assert(int128_traits::integral<uint128_t>);
static_assert(int128_traits::unsigned_integral<uint128_t>);
static_assert(!int128_traits::signed_integral<uint128_t>);

#endif

void test_int128_to_string() {
    test_tools::log_tests_started();
    constexpr uint32_t k = 20000;
    for (uint64_t n = 0; n <= k; n++) {
        assert(to_string(uint128_t{n}) == std::to_string(n));
        assert(to_string(int128_t{n}) == std::to_string(n));
    }

    for (int64_t n = -int64_t{k}; n <= 0; n++) {
        assert(to_string(int128_t{n}) == std::to_string(n));
    }

    for (uint64_t n = std::numeric_limits<uint64_t>::max();
         n >= std::numeric_limits<uint64_t>::max() - k; n--) {
        assert(to_string(uint128_t{n}) == std::to_string(n));
        assert(to_string(int128_t{n}) == std::to_string(n));
    }

    for (int64_t n = std::numeric_limits<int64_t>::min();
         n <= std::numeric_limits<int64_t>::min() + int64_t{k}; n++) {
        assert(to_string(int128_t{n}) == std::to_string(n));
    }

    assert(to_string(static_cast<uint128_t>(-1)) == "340282366920938463463374607431768211455");
    assert(to_string(uint128_t{1} << 127U) == "170141183460469231731687303715884105728");
    assert(to_string(static_cast<int128_t>((uint128_t{1} << 127U) - 1)) ==
           "170141183460469231731687303715884105727");
    assert(to_string(static_cast<int128_t>(uint128_t{1} << 127U)) ==
           "-170141183460469231731687303715884105728");
}

}  // namespace

int main() {
    test_int128_to_string();
}
