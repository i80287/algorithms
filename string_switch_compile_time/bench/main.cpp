#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cstdint>
#include <ctime>
#include <random>
#include <ranges>

#include "../StringMatch.hpp"

// clang-format off
inline constexpr std::string_view kStrings[] = {
    "abcdefghijklmnopqrstuvwxyz",
    "bcdefghijklmnopqrstuvwxyz",
    "cdefghijklmnopqrstuvwxyz",
    "defghijklmnopqrstuvwxyz",
    "efghijklmnopqrstuvwxyz",
    "fghijklmnopqrstuvwxyz",
    "ghijklmnopqrstuvwxyz",
    "hijklmnopqrstuvwxyz",
    "ijklmnopqrstuvwxyz",
    "jklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzbcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzfghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzhijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzjklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzbcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzfghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzhijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzjklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzbcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzcdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzdefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzefghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzfghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzghijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzhijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzijklmnopqrstuvwxyz",
    "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzjklmnopqrstuvwxyz",
};
// clang-format on

constexpr auto operator-(const timespec& t2, const timespec& t1) noexcept {
    const auto sec_passed = static_cast<uint64_t>(t2.tv_sec - t1.tv_sec);
    using unsigned_nsec_t = std::make_unsigned_t<decltype(t2.tv_nsec)>;
    auto nsec_passed      = sec_passed * 1'000'000'000;
    nsec_passed += static_cast<unsigned_nsec_t>(t2.tv_nsec);
    nsec_passed -= static_cast<unsigned_nsec_t>(t1.tv_nsec);
    return nsec_passed;
}

int main() {
    constexpr auto kMeasureLimit = 20u;
    std::mt19937 rnd;
    uint64_t total_time = 0;

    static constexpr auto sw =
        StringMatch<kStrings[0], kStrings[1], kStrings[2], kStrings[3], kStrings[4],
                    kStrings[5], kStrings[6], kStrings[7], kStrings[8], kStrings[9],
                    kStrings[10], kStrings[11], kStrings[12], kStrings[13], kStrings[14],
                    kStrings[15], kStrings[16], kStrings[17], kStrings[18], kStrings[19],
                    kStrings[20], kStrings[21], kStrings[22], kStrings[23], kStrings[24],
                    kStrings[25], kStrings[26], kStrings[27], kStrings[28], kStrings[29],
                    kStrings[30], kStrings[31], kStrings[32], kStrings[33], kStrings[34],
                    kStrings[35], kStrings[36], kStrings[37], kStrings[38], kStrings[39]>();

    for (size_t i = 0; i < kMeasureLimit; i++) {
        timespec t1{};
        timespec t2{};
        const auto ind = rnd() % std::size(kStrings);
        clock_gettime(CLOCK_REALTIME, &t1);
        auto ans = sw(kStrings[ind]);
        clock_gettime(CLOCK_REALTIME, &t2);
        assert(ans == ind);
        assert(t2.tv_sec >= t1.tv_sec);
        assert(t2.tv_nsec >= 0);
        assert(t1.tv_nsec >= 0);
        total_time += t2 - t1;
    }

    printf("%" PRIu64 "\n", total_time / kMeasureLimit);
}
