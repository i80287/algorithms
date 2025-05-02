#include <iostream>
#include <string>
#include <vector>

inline constexpr int64_t Prime = 59;
inline constexpr int64_t Mod = 1000000007;

int main(void) {
    using std::string, std::cin, std::cout, std::vector;

    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    string str_p;
    cin >> str_p;
    const size_t p_length = str_p.length();

    string str_t;
    str_t.reserve(p_length);
    std::cin >> str_t;

    const size_t t_length = str_t.length();
    if (p_length > t_length) {
        cout << '0' << std::endl;
        return 0;
    }

    vector<int64_t> pows = vector<int64_t>(t_length + 1);
    pows[0] = 1;
    for (size_t i = 1; i < t_length + 1; ++i) {
        pows[i] = (pows[i - 1] * Prime) % Mod;
    }

    int64_t p_hash = 0;
    for (size_t i = 0; i < p_length; ++i) {
        p_hash = (p_hash * Prime + str_p[i] - 'A' + 1) % Mod;
    }

    vector<int64_t> t_hashes = vector<int64_t>(t_length + 1);
    t_hashes[0] = 0;
    for (size_t i = 0; i < t_length; ++i) {
        t_hashes[i + 1] = (t_hashes[i] * Prime + str_t[i] - 'A' + 1) % Mod;
    }

    vector<int64_t> ans;
    size_t end = t_length - p_length;
    const int64_t MaxPstrPow = pows[p_length];
    for (size_t i = 0; i <= end; ++i) {
        if (p_hash == (t_hashes[i + p_length] - (t_hashes[i] * MaxPstrPow) % Mod + Mod) % Mod) {
            ans.push_back(i + 1);
        }
    }

    cout << ans.size() << '\n';
    for (size_t index : ans) {
        cout << index << ' ';
    }
    cout.flush();

    return 0;
}
