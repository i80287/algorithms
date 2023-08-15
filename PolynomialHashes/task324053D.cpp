#include <iostream>
#include <vector>
#include <string>

inline constexpr uint64_t Prime = 29;
inline constexpr uint64_t Mod = 1e9 + 7;
inline constexpr int FirstChar = 'a' - 1;

int main() {
    using std::string, std::vector, std::cin, std::cout;

    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    string str;
    cin >> str;

    const size_t length = str.length() + 1;
    if (length <= 3) {
        cout << "Just a legend\n" << std::flush;
        return 0;
    }

    vector<uint64_t> str_pref_hashes(length);
    vector<uint64_t> prime_pows(length);
    str_pref_hashes[0] = 0;
    prime_pows[0] = 1;
    for (size_t i = 1; i < length; ++i) {
        str_pref_hashes[i] = (str_pref_hashes[i - 1] * Prime + str[i - 1] - FirstChar) % Mod;
        prime_pows[i] = (prime_pows[i - 1] * Prime) % Mod;
    }

    size_t ans = 0;
    const size_t prefix_max_length = length - 3;
    const uint64_t full_str_hash = str_pref_hashes[length - 1];

    // i is a length of prefix 
    for (size_t i = 1; i <= prefix_max_length; i++) {

        // hash[1 ... i], because hash[0] = 0, hash[1] = str[0] - 'a' + 1, etc
        uint64_t prefix_hash = str_pref_hashes[i];

        // hash[length-i ... length-1]
        uint64_t suffix_hash = (full_str_hash - (str_pref_hashes[length - i - 1] * prime_pows[i] % Mod) + Mod) % Mod;
        if (prefix_hash != suffix_hash) {
            continue;
        }

        bool is_prefix_contains_inside = false;

        // Iterate through hash[j ... j + i - 1], j from 2 to 
        // segment length is i, last index is j + i - 1 and j + i - 1 <= length - 2
        const size_t max_segment_index = length - 1 - i;
        for (size_t j = 2; j <= max_segment_index; ++j) {
            uint64_t segment_hash = (str_pref_hashes[j + i - 1] - (str_pref_hashes[j - 1] * prime_pows[i] % Mod) + Mod) % Mod;
            if (segment_hash == prefix_hash) {
                is_prefix_contains_inside = true;
                break;
            }
        }

        if (is_prefix_contains_inside) {
            ans = i;
        }
    }

    if (ans) {
        std::string_view sv = str;
        cout << sv.substr(0, ans);
    } else {
        cout << "Just a legend\n" << std::flush;
    }
    return 0;
}
