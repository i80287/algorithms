#include <numeric>
#include <cstdint>
#include <iostream>

uint64_t solve(uint64_t k, uint64_t c, uint64_t m);

int main() {
    using std::cout, std::cin;

    uint64_t k = 0, c = 0, m = 0;
    while (true) {
        cout << "k\n> ";
        cin >> k;
        cout << "c\n> ";
        cin >> c;
        cout << "m\n> ";
        cin >> m;

        if (std::gcd(c, m) != 1) {
            cout << "c and m should be coprime\n";
            continue;
        }

        if ((m & 1) == 0) {
            cout << "m should odd (m % 2 = 1)\n";
            continue;
        }

        uint64_t x = solve(k, c, m);
        cout << "x = " << x << '\n';
    }
}

/*
 * Fast solution finder for
 * 2^k * x ≡ c (mod m),
 * where GCD(c, m) = 1 && m ≡ 1 (mod 2)
 * 
 * Works in O(k)
 * 
 * The algorithm can be modified by checking:
 * if c % 4 = 1, then if m % 4 = 3, c += m, else c -= m
 * or
 * if c % 4 = 3, then if m % 4 = 3, c -= m, else c += m
 * then
 * k -= 2, c /= 4
 */
uint64_t solve(uint64_t k, uint64_t c, uint64_t m) {
    while (k != 0) {
        if (c & 1) {
            c += m;
        }

        c >>= 1;
        --k;
    }

    return c;
}
