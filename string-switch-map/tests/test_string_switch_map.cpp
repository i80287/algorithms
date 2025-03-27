#include <algorithm>
#include <array>
#include <cassert>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <random>
#include <string_view>
#include <type_traits>

#include "../../misc/do_not_optimize_away.h"
#include "../StringMap.hpp"

// clang-format off
// NOLINTBEGIN(cert-dcl03-c, misc-static-assert, hicpp-static-assert, cppcoreguidelines-avoid-magic-numbers, cert-msc32-c, cert-msc51-cpp)
// clang-format on

namespace {

void test_string_match() {
    static constexpr auto sw = StringMatch<"abc", "def", "ghij", "foo", "bar", "baz", "qux",
                                           "abacaba", "ring", "ideal", "GLn(F)">();
    static_assert(sw("abc") == 0);
    static_assert(sw("def") == 1);
    static_assert(sw("ghij") == 2);
    static_assert(sw("foo") == 3);
    static_assert(sw("bar") == 4);
    static_assert(sw("baz") == 5);
    static_assert(sw("qux") == 6);
    static_assert(sw("abacaba") == 7);
    static_assert(sw("ring") == 8);
    static_assert(sw("ideal") == 9);
    static_assert(sw("GLn(F)") == 10);
    static_assert(sw.kDefaultValue == sw("GLn(F)") + 1);
    static_assert(sw.kDefaultValue == 11);
    static_assert(sw("not_in") == sw.kDefaultValue);
    static_assert(sw("") == sw.kDefaultValue);
    static_assert(sw("a") == sw.kDefaultValue);
    static_assert(sw("A") == sw.kDefaultValue);
    static_assert(sw("bc") == sw.kDefaultValue);
    static_assert(sw("de") == sw.kDefaultValue);
    // clang-format off
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
    constexpr const unsigned char kUString[] = "abc";
    // clang-format on
    static_assert(sw(std::data(kUString), std::size(kUString) - 1) == sw("abc"));

    assert(sw("abc") == 0);
    assert(sw("def") == 1);
    assert(sw("ghij") == 2);
    assert(sw("foo") == 3);
    assert(sw("bar") == 4);
    assert(sw("baz") == 5);
    assert(sw("qux") == 6);
    assert(sw("abacaba") == 7);
    assert(sw("ring") == 8);
    assert(sw("ideal") == 9);
    assert(sw("GLn(F)") == 10);
    assert(sw.kDefaultValue == sw("GLn(F)") + 1);
    assert(sw.kDefaultValue == 11);
    assert(sw("not_in") == sw.kDefaultValue);
    assert(sw("") == sw.kDefaultValue);
    assert(sw("a") == sw.kDefaultValue);
    assert(sw("A") == sw.kDefaultValue);
    assert(sw("bc") == sw.kDefaultValue);
    assert(sw("de") == sw.kDefaultValue);
    assert(sw(std::data(kUString), std::size(kUString) - 1) == sw("abc"));

    static constexpr auto match = StringMatch<"text1", "text2", "text3", "text4">();
    static_assert(match("text1") == 0);
    static_assert(match("text2") == 1);
    static_assert(match("text3") == 2);
    static_assert(match("text4") == 3);
    static_assert(match("not in") == match.kDefaultValue);
    static_assert(match.kDefaultValue == 4);

    assert(match("text1") == 0);
    assert(match("text2") == 1);
    assert(match("text3") == 2);
    assert(match("text4") == 3);
    assert(match("not in") == match.kDefaultValue);
    assert(match.kDefaultValue == 4);
}

void test_str_to_enum() {
    enum class SomeEnum {
        kText1,
        kText2,
        kText3,
        kText4,
        kNone,
    };
    using enum SomeEnum;
    static constexpr auto map =
        StringMap<StringMapKeys<"text1", "text2", "text3", "text4", "Text1", "Text3">,
                  StringMapValues{kText1, kText2, kText3, kText4, kText1, kText3}, kNone>();

    static_assert(map("text1") == kText1);
    static_assert(map("text2") == kText2);
    static_assert(map("text3") == kText3);
    static_assert(map("text4") == kText4);
    static_assert(map("Text1") == kText1);
    static_assert(map("Text3") == kText3);
    static_assert(map("something else") == kNone);
    static_assert(map.kDefaultValue == kNone);

    assert(map("text1") == kText1);
    assert(map("text2") == kText2);
    assert(map("text3") == kText3);
    assert(map("text4") == kText4);
    assert(map("Text1") == kText1);
    assert(map("Text3") == kText3);
    assert(map("something else") == kNone);
    assert(map.kDefaultValue == kNone);
}

void test_str_to_user_type() {
    using namespace std::string_view_literals;
    constexpr std::array kMyConstants{"abc"sv, "def"sv, "ghi"sv, "sneaky input"sv};

    struct MyTrivialType {
        std::array<int, 2> field1{};
        int field2{};

        constexpr MyTrivialType(int arg1, int arg2, int arg3) noexcept
            : field1{arg1, arg2}, field2(arg3) {}
        constexpr bool operator==(const MyTrivialType&) const noexcept = default;
    };

    static constexpr auto map =
        StringMap<StringMapKeys<kMyConstants[0], kMyConstants[1], kMyConstants[2]>,
                  StringMapValues{MyTrivialType(1, 2, 3), MyTrivialType(4, 5, 6),
                                  MyTrivialType(7, 8, 9)},
                  MyTrivialType(0, 0, 0)>();

    static_assert(map(kMyConstants[0]) == MyTrivialType(1, 2, 3));
    static_assert(map(kMyConstants[1]) == MyTrivialType(4, 5, 6));
    static_assert(map(kMyConstants[2]) == MyTrivialType(7, 8, 9));
    static_assert(map(kMyConstants[3]) == MyTrivialType(0, 0, 0));
    static_assert(map.kDefaultValue == MyTrivialType(0, 0, 0));

    assert(map(kMyConstants[0]) == MyTrivialType(1, 2, 3));
    assert(map(kMyConstants[1]) == MyTrivialType(4, 5, 6));
    assert(map(kMyConstants[2]) == MyTrivialType(7, 8, 9));
    assert(map(kMyConstants[3]) == MyTrivialType(0, 0, 0));
    assert(map.kDefaultValue == MyTrivialType(0, 0, 0));
}

#if (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 199309L) || \
    ((defined(__APPLE__) || defined(__linux__)) && (defined(__GNUG__) || defined(__clang__)))

#define CAN_RUN_BENCHMARK

using namespace std::string_view_literals;

// clang-format off
inline constexpr std::array kStrings{
    "abcdefghijklmnopqrstuvwxyz"sv,
    "bcdefghijklmnopqrstuvwxyz"sv,
    "cdefghijklmnopqrstuvwxyz"sv,
    "defghijklmnopqrstuvwxyz"sv,
    "efghijklmnopqrstuvwxyz"sv,
    "fghijklmnopqrstuvwxyz"sv,
    "ghijklmnopqrstuvwxyz"sv,
    "hijklmnopqrstuvwxyz"sv,
    "ijklmnopqrstuvwxyz"sv,
    "jklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzbcdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzcdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzfghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzhijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzjklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzbcdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzcdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzfghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzhijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzjklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzbcdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzcdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzfghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzhijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzjklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzbcdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzcdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzfghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzhijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzjklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzbcdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzcdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzdefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzefghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzfghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzghijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzhijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzijklmnopqrstuvwxyz"sv,
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzjklmnopqrstuvwxyz"sv,
};
// clang-format on

constexpr uint64_t operator-(const timespec& t2, const timespec& t1) noexcept {
    const uint64_t sec_passed = static_cast<uint64_t>(t2.tv_sec - t1.tv_sec);
    constexpr uint64_t kNanoSecondsPerSecond = 1'000'000'000;
    uint64_t nanoseconds_passed = sec_passed * kNanoSecondsPerSecond;
    using unsigned_nanoseconds_t = std::make_unsigned_t<decltype(timespec::tv_nsec)>;
    nanoseconds_passed += static_cast<unsigned_nanoseconds_t>(t2.tv_nsec);
    nanoseconds_passed -= static_cast<unsigned_nanoseconds_t>(t1.tv_nsec);
    return nanoseconds_passed;
}

void run_bench() {
    constexpr auto kMeasureLimit = 10000U;

    static constexpr auto sw = StringMatch<
        kStrings[0], kStrings[1], kStrings[2], kStrings[3], kStrings[4], kStrings[5], kStrings[6],
        kStrings[7], kStrings[8], kStrings[9], kStrings[10], kStrings[11], kStrings[12],
        kStrings[13], kStrings[14], kStrings[15], kStrings[16], kStrings[17], kStrings[18],
        kStrings[19], kStrings[20], kStrings[21], kStrings[22], kStrings[23], kStrings[24],
        kStrings[25], kStrings[26], kStrings[27], kStrings[28], kStrings[29], kStrings[30],
        kStrings[31], kStrings[32], kStrings[33], kStrings[34], kStrings[35], kStrings[36],
        kStrings[37], kStrings[38], kStrings[39], kStrings[40], kStrings[41], kStrings[42],
        kStrings[43], kStrings[44], kStrings[45], kStrings[46], kStrings[47], kStrings[48],
        kStrings[49], kStrings[50], kStrings[51], kStrings[52], kStrings[53], kStrings[54],
        kStrings[55], kStrings[56], kStrings[57], kStrings[58], kStrings[59]>();

    std::array<std::size_t, kMeasureLimit> indexes{};
    {
        constexpr std::uint32_t kSeed = 0x2383284;
        std::mt19937 rnd{kSeed};
        std::generate_n(indexes.begin(), kMeasureLimit,
                        [&]() noexcept { return rnd() % std::size(kStrings); });
    }

    for (const std::size_t ind : indexes) {
        auto ans = sw(kStrings[ind]);
        assert(ans == ind);
    }

    timespec t1{};
    // NOLINTNEXTLINE(misc-include-cleaner)
    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (const std::size_t ind : indexes) {
        auto ans = sw(kStrings[ind]);
        config::do_not_optimize_away(ans);
    }
    timespec t2{};
    // NOLINTNEXTLINE(misc-include-cleaner)
    clock_gettime(CLOCK_MONOTONIC, &t2);

    std::printf("%" PRIu64 " nanoseconds on average\n", (t2 - t1) / kMeasureLimit);
}

#endif

}  // namespace

// clang-format off
// NOLINTEND(cert-dcl03-c, misc-static-assert, hicpp-static-assert, cppcoreguidelines-avoid-magic-numbers, cert-msc32-c, cert-msc51-cpp)
// clang-format on

int main() {
    test_string_match();
    test_str_to_enum();
    test_str_to_user_type();
#ifdef CAN_RUN_BENCHMARK
    run_bench();
#endif
    return 0;
}
