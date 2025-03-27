#pragma once

#include <algorithm>
#include <cstddef>
#include <string_view>
#include <vector>

#include "../misc/config_macros.hpp"

namespace str_tools {

using std::size_t;

#if CONFIG_HAS_AT_LEAST_CXX_20 && !defined(_GLIBCXX_DEBUG) && !defined(_GLIBCXX_ASSERTIONS)
#define CONSTEXPR_VECTOR_CXX_20 constexpr
#else
#define CONSTEXPR_VECTOR_CXX_20 inline
#endif

template <size_t ReplacementCost = 1, size_t DeletionCost = 1, size_t InsertionCost = 1>
[[nodiscard]] CONSTEXPR_VECTOR_CXX_20 size_t levenshtein_distance(std::string_view s1,
                                                                  std::string_view s2) {
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
            const bool replace_char = s1[i - 1] != s2[j - 1];
            const size_t total_cost_with_replacement =
                dp1[j - 1] + size_t{replace_char} * ReplacementCost;

            const size_t total_cost_with_deletion = dp1[j] + DeletionCost;
            const size_t total_cost_with_insertion = dp2[j - 1] + InsertionCost;

            dp2[j] = std::min(total_cost_with_replacement,
                              std::min(total_cost_with_deletion, total_cost_with_insertion));
        }
        dp1.swap(dp2);
    }

    return dp1[s2.size()];
}

}  // namespace str_tools

#undef CONSTEXPR_VECTOR_CXX_20
