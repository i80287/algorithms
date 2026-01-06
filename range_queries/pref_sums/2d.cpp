#include <cstdint>
#include <iostream>
#include <vector>

using std::vector;

int main(void) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    size_t n = 0, m = 0;
    std::cin >> n >> m;
    ++n;
    ++m;

    vector<vector<int64_t>> dp(n, vector<int64_t>(m, 0));
    for (size_t i = 1; i != n; ++i) {
        for (size_t j = 1; j != m; ++j) {
            int a;
            std::cin >> a;
            dp[i][j] = a + dp[i - 1][j] + dp[i][j - 1] - dp[i - 1][j - 1];
        }
    }

    size_t q = 0;
    std::cin >> q;
    for (size_t i = 0; i != q; ++i) {
        size_t lx = 0, ly = 0, rx = 0, ry = 0;
        std::cin >> lx >> ly >> rx >> ry;
        std::cout << dp[rx][ry] - dp[lx - 1][ry] - dp[rx][ly - 1] + dp[lx - 1][ly - 1] << '\n';
    }

    std::cout.flush();
}
