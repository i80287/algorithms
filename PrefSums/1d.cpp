#include <cstdint>
#include <cstddef>
#include <iostream>
#include <vector>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    uint32_t n = 0, q = 0;
    std::cin >> n >> q;
    std::vector<uint64_t> prefsums(n + 1);
    prefsums[0] = 0;
    for (size_t i = 1; i <= n; ++i)
    {
        uint32_t a = 0;
        std::cin >> a;
        prefsums[i] = prefsums[i - 1] + a;
    }

    while (q--) {
        uint32_t l = 0, r = 0;
        std::cin >> l >> r;
        std::cout << prefsums[r] - prefsums[l - 1] << '\n';
    }

    std::cout.flush();
}
