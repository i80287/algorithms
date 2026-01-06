#include <cstdint>
#include <iostream>
#include <vector>

using std::vector;

int main(void) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    size_t n1, n2, n3, n4, n5;
    std::cin >> n1 >> n2 >> n3 >> n4 >> n5;
    ++n1;
    ++n2;
    ++n3;
    ++n4;
    ++n5;

    vector<vector<vector<vector<vector<int64_t>>>>> dp(
        n1, vector<vector<vector<vector<int64_t>>>>(
                n2, vector<vector<vector<int64_t>>>(n3, vector<vector<int64_t>>(n4, vector<int64_t>(n5, 0)))));

    for (size_t i = 1; i != n1; ++i) {
        for (size_t j = 1; j != n2; ++j) {
            for (size_t k = 1; k != n3; ++k) {
                for (size_t l = 1; l != n4; ++l) {
                    for (size_t m = 1; m != n5; ++m) {
                        int a = 0;
                        std::cin >> a;
                        dp[i][j][k][l][m] =
                            a +
                            (dp[i - 1][j][k][l][m] + dp[i][j - 1][k][l][m] + dp[i][j][k - 1][l][m] +
                             dp[i][j][k][l - 1][m] + dp[i][j][k][l][m - 1]) -
                            (dp[i - 1][j - 1][k][l][m] + dp[i - 1][j][k - 1][l][m] + dp[i - 1][j][k][l - 1][m] +
                             dp[i - 1][j][k][l][m - 1] + dp[i][j - 1][k - 1][l][m] + dp[i][j - 1][k][l - 1][m] +
                             dp[i][j - 1][k][l][m - 1] + dp[i][j][k - 1][l - 1][m] + dp[i][j][k - 1][l][m - 1] +
                             dp[i][j][k][l - 1][m - 1]) +
                            (dp[i - 1][j - 1][k - 1][l][m] + dp[i - 1][j - 1][k][l - 1][m] +
                             dp[i - 1][j - 1][k][l][m - 1] + dp[i - 1][j][k - 1][l - 1][m] +
                             dp[i - 1][j][k - 1][l][m - 1] + dp[i - 1][j][k][l - 1][m - 1] +
                             dp[i][j - 1][k - 1][l - 1][m] + dp[i][j - 1][k - 1][l][m - 1] +
                             dp[i][j - 1][k][l - 1][m - 1] + dp[i][j][k - 1][l - 1][m - 1]) -
                            (dp[i][j - 1][k - 1][l - 1][m - 1] + dp[i - 1][j][k - 1][l - 1][m - 1] +
                             dp[i - 1][j - 1][k][l - 1][m - 1] + dp[i - 1][j - 1][k - 1][l][m - 1] +
                             dp[i - 1][j - 1][k - 1][l - 1][m]) +
                            (dp[i - 1][j - 1][k - 1][l - 1][m - 1]);
                    }  // m
                }  // l
            }  // k
        }  // j
    }  // i

    size_t q = 0;
    std::cin >> q;
    for (size_t i = 0; i != q; ++i) {
        size_t l1, l2, l3, l4, l5, r1, r2, r3, r4, r5;
        std::cin >> l1 >> l2 >> l3 >> l4 >> l5 >> r1 >> r2 >> r3 >> r4 >> r5;

        std::cout << ((dp[r1][r2][r3][r4][r5]) -
                      (dp[l1 - 1][r2][r3][r4][r5] + dp[r1][l2 - 1][r3][r4][r5] + dp[r1][r2][l3 - 1][r4][r5] +
                       dp[r1][r2][r3][l4 - 1][r5] + dp[r1][r2][r3][r4][l5 - 1]) +
                      (dp[l1 - 1][l2 - 1][r3][r4][r5] + dp[l1 - 1][r2][l3 - 1][r4][r5] +
                       dp[l1 - 1][r2][r3][l4 - 1][r5] + dp[l1 - 1][r2][r3][r4][l5 - 1] +
                       dp[r1][l2 - 1][l3 - 1][r4][r5] + dp[r1][l2 - 1][r3][l4 - 1][r5] +
                       dp[r1][l2 - 1][r3][r4][l5 - 1] + dp[r1][r2][l3 - 1][l4 - 1][r5] +
                       dp[r1][r2][l3 - 1][r4][l5 - 1] + dp[r1][r2][r3][l4 - 1][l5 - 1]) -
                      (dp[l1 - 1][l2 - 1][l3 - 1][r4][r5] + dp[l1 - 1][l2 - 1][r3][l4 - 1][r5] +
                       dp[l1 - 1][l2 - 1][r3][r4][l5 - 1] + dp[l1 - 1][r2][l3 - 1][l4 - 1][r5] +
                       dp[l1 - 1][r2][l3 - 1][r4][l5 - 1] + dp[l1 - 1][r2][r3][l4 - 1][l5 - 1] +
                       dp[r1][l2 - 1][l3 - 1][l4 - 1][r5] + dp[r1][l2 - 1][l3 - 1][r4][l5 - 1] +
                       dp[r1][l2 - 1][r3][l4 - 1][l5 - 1] + dp[r1][r2][l3 - 1][l4 - 1][l5 - 1]) +
                      (dp[r1][l2 - 1][l3 - 1][l4 - 1][l5 - 1] + dp[l1 - 1][r2][l3 - 1][l4 - 1][l5 - 1] +
                       dp[l1 - 1][l2 - 1][r3][l4 - 1][l5 - 1] + dp[l1 - 1][l2 - 1][l3 - 1][r4][l5 - 1] +
                       dp[l1 - 1][l2 - 1][l3 - 1][l4 - 1][r5]) -
                      (dp[l1 - 1][l2 - 1][l3 - 1][l4 - 1][l5 - 1]))
                  << '\n';
    }

    std::cout.flush();
}
