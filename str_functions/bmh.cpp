#include <cstdint>
#include <vector>
#include <array>
#include <string_view>
#include <limits>

using namespace std;

/// @brief Boyer-Moore-Horspool algorithm
/// @param pattern 
/// @param text 
/// @return 
vector<size_t> find_substrings_bmh(const string_view pattern, const string_view text) {
    // Just 256
    constexpr auto kSize = std::numeric_limits<uint8_t>::max() + 1;
    array<size_t, kSize> table{pattern.size()};
    for (size_t i = 0; i + 1 < pattern.size(); i++) {
        const auto c_ind = uint8_t(pattern[i]);
        table[c_ind] = pattern.size() - i - 1;
    }

    size_t t_i = pattern.size() - 1;
    vector<size_t> ans;
    while (t_i < text.size()) {
        const auto start_pos = t_i - (pattern.size() - 1);
        const string_view pt = text.substr(start_pos, pattern.size());
        if (pt == pattern) {
            ans.push_back(start_pos);
            t_i++;
        } else {
            t_i += table[uint8_t(text[t_i])];
        }
    }

    return ans;
}
