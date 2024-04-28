#include <cstdint>
#include <string>
#include <string_view>
#include <iostream>
#include <vector>

using std::string, std::string_view, std::vector, std::cin, std::cout;

vector<uint32_t> prefix_function(string_view s) {
	const size_t n = s.length();
	vector<uint32_t> pi(n);
	for (size_t i = 1; i < n; i++) {
		size_t j = pi[i - 1];
		uint32_t c_i = uint8_t(s[i]);
		while (j > 0 && c_i != uint8_t(s[j])) {
            j = pi[j - 1];
        }
		if (c_i == uint8_t(s[j])) {
			j++;
		}
		pi[i] = uint32_t(j);
	}

	return pi;
}

void find_pref_func(string_view text, string_view substr) {
	string s;
	s.reserve(substr.size() + 1 + text.size());
	s.append(substr);
	s.push_back('#');
	s.append(text);
	vector<uint32_t> pref_func = prefix_function(s);
	cout << "Input: " << text << "\nSubstring to search: " << substr << '\n';
	auto it = pref_func.begin() + static_cast<std::ptrdiff_t>(substr.size() + 1); // Skip "substr#"
	auto end = pref_func.end();
	for (size_t i = 0; it != end; ++it, ++i) {
		if (*it == substr.size()) {
			size_t start_index = i + 1 - substr.size();
			cout << "Substring " << text.substr(start_index, substr.size())
				<< " from " << start_index << " to " << i << '\n';
		}
	}
}

int main() {
	constexpr std::string_view text = "abcdabcdddabcd";
	constexpr std::string_view substr = "abc";
	find_pref_func(text, substr);
}
