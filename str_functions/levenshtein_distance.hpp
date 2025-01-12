#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

using std::size_t;
using std::uint32_t;

template <size_t ReplacementCost = 1, size_t DeletionCost = 1, size_t InsertionCost = 1>
[[nodiscard]] constexpr size_t levenshtein_distance(std::string_view s1, std::string_view s2) {
    if (s1.size() < s2.size()) {
        s1.swap(s2);
    }

    std::vector<size_t> dp1(s2.size() + 1);
    std::vector<size_t> dp2(s2.size() + 1);
    for (size_t j = 0; j <= s2.size(); j++) {
        dp1[j] = j;
    }

    for (size_t i = 1; i <= s1.size(); i++) {
        dp2[0] = i;
        for (size_t j = 1; j <= s2.size(); j++) {
            dp2[j] = std::min(dp1[j - 1] + size_t{s1[i - 1] != s2[j - 1]} * ReplacementCost,
                              std::min(dp1[j] + DeletionCost, dp2[j - 1] + InsertionCost));
        }
        dp1.swap(dp2);
    }

    return dp1[s2.size()];
}
