// clang-format off
#include "integers_128_bit.hpp"
// clang-format on

#include <cassert>
#include <cstdint>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

#include "../misc/config_macros.hpp"
#include "../misc/tests/test_tools.hpp"

namespace {

using std::int64_t;
using std::uint64_t;

// test traits for the uint128_t

static_assert(int128_traits::is_arithmetic_v<uint128_t>);
static_assert(int128_traits::is_integral_v<uint128_t>);
static_assert(int128_traits::is_unsigned_v<uint128_t>);
static_assert(!int128_traits::is_signed_v<uint128_t>);
static_assert(std::is_same_v<int128_traits::make_unsigned_t<uint128_t>, uint128_t>);
static_assert(std::is_same_v<int128_traits::make_signed_t<uint128_t>, int128_t>);

static_assert(int128_traits::is_arithmetic_v<const uint128_t>);
static_assert(int128_traits::is_integral_v<const uint128_t>);
static_assert(int128_traits::is_unsigned_v<const uint128_t>);
static_assert(!int128_traits::is_signed_v<const uint128_t>);
static_assert(std::is_same_v<int128_traits::make_unsigned_t<const uint128_t>, const uint128_t>);
static_assert(std::is_same_v<int128_traits::make_signed_t<const uint128_t>, const int128_t>);

static_assert(int128_traits::is_arithmetic_v<volatile uint128_t>);
static_assert(int128_traits::is_integral_v<volatile uint128_t>);
static_assert(int128_traits::is_unsigned_v<volatile uint128_t>);
static_assert(!int128_traits::is_signed_v<volatile uint128_t>);
static_assert(
    std::is_same_v<int128_traits::make_unsigned_t<volatile uint128_t>, volatile uint128_t>);
static_assert(std::is_same_v<int128_traits::make_signed_t<volatile uint128_t>, volatile int128_t>);

static_assert(int128_traits::is_arithmetic_v<const volatile uint128_t>);
static_assert(int128_traits::is_integral_v<const volatile uint128_t>);
static_assert(int128_traits::is_unsigned_v<const volatile uint128_t>);
static_assert(!int128_traits::is_signed_v<const volatile uint128_t>);
static_assert(std::is_same_v<int128_traits::make_unsigned_t<const volatile uint128_t>,
                             const volatile uint128_t>);
static_assert(std::is_same_v<int128_traits::make_signed_t<const volatile uint128_t>,
                             const volatile int128_t>);

// test traits for the int128_t

static_assert(int128_traits::is_arithmetic_v<int128_t>);
static_assert(int128_traits::is_integral_v<int128_t>);
static_assert(!int128_traits::is_unsigned_v<int128_t>);
static_assert(int128_traits::is_signed_v<int128_t>);
static_assert(std::is_same_v<int128_traits::make_unsigned_t<int128_t>, uint128_t>);
static_assert(std::is_same_v<int128_traits::make_signed_t<int128_t>, int128_t>);

static_assert(int128_traits::is_arithmetic_v<const int128_t>);
static_assert(int128_traits::is_integral_v<const int128_t>);
static_assert(!int128_traits::is_unsigned_v<const int128_t>);
static_assert(int128_traits::is_signed_v<const int128_t>);
static_assert(std::is_same_v<int128_traits::make_unsigned_t<const int128_t>, const uint128_t>);
static_assert(std::is_same_v<int128_traits::make_signed_t<const int128_t>, const int128_t>);

static_assert(int128_traits::is_arithmetic_v<volatile int128_t>);
static_assert(int128_traits::is_integral_v<volatile int128_t>);
static_assert(!int128_traits::is_unsigned_v<volatile int128_t>);
static_assert(int128_traits::is_signed_v<volatile int128_t>);
static_assert(
    std::is_same_v<int128_traits::make_unsigned_t<volatile int128_t>, volatile uint128_t>);
static_assert(std::is_same_v<int128_traits::make_signed_t<volatile int128_t>, volatile int128_t>);

static_assert(int128_traits::is_arithmetic_v<const volatile int128_t>);
static_assert(int128_traits::is_integral_v<const volatile int128_t>);
static_assert(!int128_traits::is_unsigned_v<const volatile int128_t>);
static_assert(int128_traits::is_signed_v<const volatile int128_t>);
static_assert(std::is_same_v<int128_traits::make_unsigned_t<const volatile int128_t>,
                             const volatile uint128_t>);
static_assert(
    std::is_same_v<int128_traits::make_signed_t<const volatile int128_t>, const volatile int128_t>);

// test traits for the uint64_t

static_assert(int128_traits::is_arithmetic_v<uint64_t>);
static_assert(int128_traits::is_integral_v<uint64_t>);
static_assert(int128_traits::is_unsigned_v<uint64_t>);
static_assert(!int128_traits::is_signed_v<uint64_t>);
static_assert(std::is_same_v<int128_traits::make_unsigned_t<uint64_t>, uint64_t>);
static_assert(std::is_same_v<int128_traits::make_signed_t<uint64_t>, int64_t>);

static_assert(int128_traits::is_arithmetic_v<const uint64_t>);
static_assert(int128_traits::is_integral_v<const uint64_t>);
static_assert(int128_traits::is_unsigned_v<const uint64_t>);
static_assert(!int128_traits::is_signed_v<const uint64_t>);
static_assert(std::is_same_v<int128_traits::make_unsigned_t<const uint64_t>, const uint64_t>);
static_assert(std::is_same_v<int128_traits::make_signed_t<const uint64_t>, const int64_t>);

static_assert(int128_traits::is_arithmetic_v<volatile uint64_t>);
static_assert(int128_traits::is_integral_v<volatile uint64_t>);
static_assert(int128_traits::is_unsigned_v<volatile uint64_t>);
static_assert(!int128_traits::is_signed_v<volatile uint64_t>);
static_assert(std::is_same_v<int128_traits::make_unsigned_t<volatile uint64_t>, volatile uint64_t>);
static_assert(std::is_same_v<int128_traits::make_signed_t<volatile uint64_t>, volatile int64_t>);

static_assert(int128_traits::is_arithmetic_v<const volatile uint64_t>);
static_assert(int128_traits::is_integral_v<const volatile uint64_t>);
static_assert(int128_traits::is_unsigned_v<const volatile uint64_t>);
static_assert(!int128_traits::is_signed_v<const volatile uint64_t>);
static_assert(std::is_same_v<int128_traits::make_unsigned_t<const volatile uint64_t>,
                             const volatile uint64_t>);
static_assert(
    std::is_same_v<int128_traits::make_signed_t<const volatile uint64_t>, const volatile int64_t>);

// test traits for the int64_t

static_assert(int128_traits::is_arithmetic_v<int64_t>);
static_assert(int128_traits::is_integral_v<int64_t>);
static_assert(!int128_traits::is_unsigned_v<int64_t>);
static_assert(int128_traits::is_signed_v<int64_t>);
static_assert(std::is_same_v<int128_traits::make_unsigned_t<int64_t>, uint64_t>);
static_assert(std::is_same_v<int128_traits::make_signed_t<int64_t>, int64_t>);

static_assert(int128_traits::is_arithmetic_v<volatile int64_t>);
static_assert(int128_traits::is_integral_v<volatile int64_t>);
static_assert(!int128_traits::is_unsigned_v<volatile int64_t>);
static_assert(int128_traits::is_signed_v<volatile int64_t>);
static_assert(std::is_same_v<int128_traits::make_unsigned_t<volatile int64_t>, volatile uint64_t>);
static_assert(std::is_same_v<int128_traits::make_signed_t<volatile int64_t>, volatile int64_t>);

static_assert(int128_traits::is_arithmetic_v<const int64_t>);
static_assert(int128_traits::is_integral_v<const int64_t>);
static_assert(!int128_traits::is_unsigned_v<const int64_t>);
static_assert(int128_traits::is_signed_v<const int64_t>);
static_assert(std::is_same_v<int128_traits::make_unsigned_t<const int64_t>, const uint64_t>);
static_assert(std::is_same_v<int128_traits::make_signed_t<const int64_t>, const int64_t>);

static_assert(int128_traits::is_arithmetic_v<const volatile int64_t>);
static_assert(int128_traits::is_integral_v<const volatile int64_t>);
static_assert(!int128_traits::is_unsigned_v<const volatile int64_t>);
static_assert(int128_traits::is_signed_v<const volatile int64_t>);
static_assert(std::is_same_v<int128_traits::make_unsigned_t<const volatile int64_t>,
                             const volatile uint64_t>);
static_assert(
    std::is_same_v<int128_traits::make_signed_t<const volatile int64_t>, const volatile int64_t>);

#if CONFIG_HAS_CONCEPTS

static_assert(int128_traits::integral<int128_t>);
static_assert(!int128_traits::unsigned_integral<int128_t>);
static_assert(int128_traits::signed_integral<int128_t>);

static_assert(int128_traits::integral<uint128_t>);
static_assert(int128_traits::unsigned_integral<uint128_t>);
static_assert(!int128_traits::signed_integral<uint128_t>);

#endif

template <class T>
[[nodiscard]] bool test_int128_to_string_test_case(const T value,
                                                   const std::string_view expected_str) {
    static_assert(std::is_same_v<T, int128_t> || std::is_same_v<T, uint128_t>);

    if (to_string(value) != expected_str) {
        return false;
    }

    return [value]() {
        std::ostringstream oss;
        oss << value;
        return std::move(oss).str();
    }() == expected_str;
}

void test_int128_to_string() {
    test_tools::log_tests_started();

    constexpr uint32_t k = 20000;
    for (uint64_t n = 0; n <= k; n++) {
        assert(test_int128_to_string_test_case(uint128_t{n}, std::to_string(n)));
        assert(test_int128_to_string_test_case(int128_t{n}, std::to_string(n)));
    }

    for (int64_t n = -int64_t{k}; n <= 0; n++) {
        assert(test_int128_to_string_test_case(int128_t{n}, std::to_string(n)));
    }

    for (uint64_t n = std::numeric_limits<uint64_t>::max();
         n >= std::numeric_limits<uint64_t>::max() - k; n--) {
        assert(test_int128_to_string_test_case(uint128_t{n}, std::to_string(n)));
        assert(test_int128_to_string_test_case(int128_t{n}, std::to_string(n)));
    }

    for (int64_t n = std::numeric_limits<int64_t>::min();
         n <= std::numeric_limits<int64_t>::min() + int64_t{k}; n++) {
        assert(test_int128_to_string_test_case(int128_t{n}, std::to_string(n)));
    }

    // GCC 14 false positive:
    // allocation of the 4611686018427387904 bytes via the operator new at:
    // clang-format off
    //  std::__new_allocator<char>::allocate(size_type, const void*)                      at /usr/include/c++/14/bits/new_allocator.h:151:55
    //  std::allocator<char>::allocate(size_type)                                         at /usr/include/c++/14/bits/allocator.h:196:40
    //  std::allocator_traits<std::allocator<char>>::allocate(allocator_type&, size_type) at /usr/include/c++/14/bits/alloc_traits.h:478:28
    //  std::string::_S_allocate(std::allocator<char>&, size_type)                        at /usr/include/c++/14/bits/basic_string.h:131:39
    //  std::string::_M_create(size_type&, size_type)                                     at /usr/include/c++/14/bits/basic_string.tcc:159:25
    //  std::string::_M_mutate(size_type, size_type, const char*, size_type)              at /usr/include/c++/14/bits/basic_string.tcc:332:30
    //  std::string::_M_replace(size_type, size_type, const char*, size_type)             at /usr/include/c++/14/bits/basic_string.tcc:548:17
    //  std::string::assign(char*, char*)                                                 at /usr/include/c++/14/bits/basic_string.h:1739:25
    // clang-format on
#if CONFIG_GNUC_AT_LEAST(14, 0) && !CONFIG_COMPILER_IS_ANY_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wno-alloc-size-larger-than"
#endif

    assert(test_int128_to_string_test_case(static_cast<uint128_t>(-1),
                                           "340282366920938463463374607431768211455"));
    assert(test_int128_to_string_test_case(uint128_t{1} << 127U,
                                           "170141183460469231731687303715884105728"));
    assert(test_int128_to_string_test_case(static_cast<int128_t>((uint128_t{1} << 127U) - 1),
                                           "170141183460469231731687303715884105727"));
    assert(test_int128_to_string_test_case(static_cast<int128_t>(uint128_t{1} << 127U),
                                           "-170141183460469231731687303715884105728"));
    constexpr int128_t kBigPrime = int128_t{55141608584989336ULL} * 10000 + 1159;
    constexpr std::string_view kBigPrimeStr = "551416085849893361159";
    assert(test_int128_to_string_test_case(static_cast<int128_t>(kBigPrime), kBigPrimeStr));
    assert(test_int128_to_string_test_case(static_cast<uint128_t>(kBigPrime), kBigPrimeStr));

#if CONFIG_GNUC_AT_LEAST(14, 0) && !CONFIG_COMPILER_IS_ANY_CLANG
#pragma GCC diagnostic pop
#endif
}

}  // namespace

int main() {
    test_int128_to_string();
}
