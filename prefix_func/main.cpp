#include <cstdint>
#include <string>
#include <iostream>
#include <vector>

constexpr std::vector<uint32_t> prefix_function(const std::string& s) {
	size_t n = s.length();
	std::vector<uint32_t> pi(n);

	for (size_t i = 1; i < n; ++i) {
		size_t j = pi[i - 1];
		int c_i = s[i];
		while (j > 0 && c_i != s[j]) {
            j = pi[j - 1];
        }

		if (c_i == s[j]) {
            ++j;
        }

		pi[i] = static_cast<uint32_t>(j);
	}

	return pi;
}

void find(std::string_view text, std::string_view substr) {
	std::string s;
	s.reserve(substr.size() + 1 + text.size());
	s.append(substr);
	s.push_back('#');
	s.append(text);

	std::vector<uint32_t> pref_func = prefix_function(s);

	std::cout << "Input: " << text << "\nSubstring to search: " << substr << '\n';
	auto it = pref_func.begin() + substr.size() + 1; // Skip "substr#"
	auto end = pref_func.end();
	for (size_t i = 0; it != end; ++it, ++i) {
		if (*it == substr.size()) {
			size_t start_index = i + 1 - substr.size();
			std::cout << "Substring " << text.substr(start_index, substr.size())
				<< " from " << start_index << " to " << i << '\n';
		}
	}
}

int main() {
	constexpr std::string_view text = "abcdabcd";
	constexpr std::string_view substr = "abc";
	find(text, substr);
}
