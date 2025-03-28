#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

using std::string, std::string_view, std::vector, std::cin, std::cout;

vector<uint32_t> z_function(string_view s) {
    const size_t n = s.length();
    vector<uint32_t> z(n);
    for (size_t i = 1, l = 0, r = 0; i < n; i++) {
        if (i <= r) {
            z[i] = std::min(z[i - l], uint32_t(r - i + 1));
        }
        while (i + z[i] < n && uint8_t(s[z[i]]) == uint8_t(s[i + z[i]])) {
            z[i]++;
        }
        size_t cur_r = i + z[i] - 1;
        if (cur_r > r) {
            r = cur_r;
            l = i;
        }
    }

    return z;
}

void find_z_func(string_view text, string_view substr) {
    string s;
    s.reserve(substr.size() + 1 + text.size());
    s.append(substr);
    s.push_back('#');
    s.append(text);
    vector<uint32_t> zf = z_function(s);
    cout << "Input: " << text << "\nSubstring to search: " << substr << '\n';
    auto it = zf.begin() + static_cast<std::ptrdiff_t>(substr.size() + 1);  // Skip "substr#"
    auto end = zf.end();
    for (size_t i = 0; it != end; ++it, ++i) {
        if (*it == substr.size()) {
            size_t end_index = i + substr.size() - 1;
            cout << "Substring " << text.substr(i, substr.size()) << " from " << i << " to "
                 << end_index << '\n';
        }
    }
}

int main() {
    constexpr std::string_view text = "abcdabcdddabcd";
    constexpr std::string_view substr = "abc";
    find_z_func(text, substr);
}
