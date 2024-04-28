#include <cstdint>
#include <vector>
#include <string_view>
#include <limits>

using namespace std;

static vector<size_t> prefix_function(string_view s) {
	const size_t n = s.length();
	vector<size_t> pi(n);
	for (size_t i = 1; i < n; i++) {
		size_t j = pi[i - 1];
		uint32_t c_i = uint8_t(s[i]);
		while (j > 0 && c_i != uint8_t(s[j])) {
            j = pi[j - 1];
        }
		if (c_i == uint8_t(s[j])) {
			j++;
		}
		pi[i] = j;
	}

    for (size_t i = 0; i + 1 < n; i++) {
        if (pi[i] + 1 == pi[i + 1]) {
            pi[i] = 0;
        }
    }

	return pi;
}

/// @brief Knuth-Morris-Pratt algorithm
/// @param pattern 
/// @param text 
/// @return 
vector<size_t> find_substrings_bmh(const string_view pattern, const string_view text) {
    const auto pi = prefix_function(pattern);
    vector<size_t> ans;
    size_t p_i = 0;
    size_t t_i = 0;
    while (t_i < text.size()) {
        if (uint8_t(pattern[p_i]) == uint8_t(text[t_i])) {
            p_i++;
            t_i++;
            if (p_i == pattern.size()) {
                ans.push_back(t_i - p_i);
                p_i = pi[p_i - 1];
            }
        } else if (p_i == 0) {
            t_i++;
        } else {
            p_i = pi[p_i - 1];
        }
    }

    return ans;
}
