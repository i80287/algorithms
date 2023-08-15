#include <iostream>
#include <string>
#include <vector>

inline constexpr uint64_t Mod = 1000000007;
inline constexpr uint64_t Prime = 29;
inline constexpr int LowestChar = 'a' - 1;

int main() {
    using std::vector, std::string;

    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    string str;
    std::cin >> str;

    const size_t str_length = str.size();
    vector<uint64_t> str_prefix_hashes = vector<uint64_t>(str_length + 1);
    vector<uint64_t> pows = vector<uint64_t>(str_length + 1);

    str_prefix_hashes[0] = 0;
    pows[0] = 1;
    for (size_t i = 0; i < str_length; ++i) {
        str_prefix_hashes[i + 1] = (str_prefix_hashes[i] * Prime + (str[i] - LowestChar)) % Mod;
        pows[i + 1] = (pows[i] * Prime) % Mod;
    }

    for (size_t i = 1; i <= str_length; ++i) {
        // i is supposted to be a period's length.
        if (str_length % i != 0) {
            continue;
        }

        bool is_prefix_a_period = true;
        uint64_t current_prefix_hash = str_prefix_hashes[i];
        for (size_t j = i; j <= str_length - i; j += i) {
            uint64_t segment_hash = (str_prefix_hashes[j + i] - (str_prefix_hashes[j] * pows[i]) % Mod + Mod) % Mod;
            if (current_prefix_hash != segment_hash) {
                is_prefix_a_period = false;
                break;
            }
        }
        
        if (is_prefix_a_period) {
            std::cout << str_length / i << std::flush;
            return 0;
        }
    }
    
    return 0;
}
