#include <cstdint>
#include <string>
#include <string_view>
#include <iostream>
#include <vector>

using std::string, std::string_view, std::vector, std::cin, std::cout;

inline vector<uint32_t> prefix_function(const string& s) {
	size_t n = uint32_t(s.length());
	vector<uint32_t> pi(n);
	for (uint32_t i = 1; i < n; i++) {
		size_t j = pi[i - 1];
		uint32_t c_i = uint8_t(s[i]);
		while (j > 0 && c_i != uint8_t(s[j])) {
            j = pi[j - 1];
        }
		j += c_i == uint8_t(s[j]);
		pi[i] = uint32_t(j);
	}

	return pi;
}

inline vector<uint32_t> z_function(const string& s) {
	uint32_t n = uint32_t(s.length());
	vector<uint32_t> z(n);
	for (uint32_t i = 1, l = 0, r = 0; i < n; i++) {
		if (i <= r) {
            z[i] = std::min(z[i - l], r - i + 1);
        }
        while (i + z[i] < n && uint8_t(s[z[i]]) == uint8_t(s[i + z[i]])) {
            z[i]++;
        }
        uint32_t cur_r = i + z[i] - 1;
        if (cur_r > r) {
            r = cur_r;
            l = i;
        }
	}

	return z;
}

void find_p(string_view text, string_view substr) {
	string s;
	s.reserve(substr.size() + 1 + text.size());
	s.append(substr);
	s.push_back('#');
	s.append(text);

	vector<uint32_t> pref_func = prefix_function(s);

	cout << "Input: " << text << "\nSubstring to search: " << substr << '\n';
	auto it = pref_func.begin() + (substr.size() + 1); // Skip "substr#"
	auto end = pref_func.end();
	for (size_t i = 0; it != end; ++it, ++i) {
		if (*it == substr.size()) {
			size_t start_index = i + 1 - substr.size();
			cout << "Substring " << text.substr(start_index, substr.size())
				<< " from " << start_index << " to " << i << '\n';
		}
	}
}

void find_z(string_view text, string_view substr) {
	string s;
	s.reserve(substr.size() + 1 + text.size());
	s.append(substr);
	s.push_back('#');
	s.append(text);

	vector<uint32_t> zf = z_function(s);

	cout << "Input: " << text << "\nSubstring to search: " << substr << '\n';
	auto it = zf.begin() + (substr.size() + 1); // Skip "substr#"
	auto end = zf.end();
	for (size_t i = 0; it != end; ++it, ++i) {
		if (*it == substr.size()) {
			size_t end_index = i + substr.size() - 1;
			cout << "Substring " << text.substr(i, substr.size())
				<< " from " << i << " to " << end_index << '\n';
		}
	}
}

int main() {
	constexpr std::string_view text = "abcdabcdddabcd";
	constexpr std::string_view substr = "abc";
	find_p(text, substr);
	find_z(text, substr);
}
