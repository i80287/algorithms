#include <cmath>
#include <cstdio>
#include <cstdint>

static constexpr uint64_t bin_pow_slow_recursion(uint64_t n, uint32_t p) noexcept {
    if (p) {
        uint64_t res = bin_pow_slow_recursion(n, p >> 1);
        res *= res;
        return p & 1 ? n * res : res;
    }
    return 1ull;
}

static constexpr uint64_t bin_pow_slow_recursion(uint64_t n, uint32_t p, uint64_t mod) noexcept {
    if (p) {
        uint64_t res = bin_pow_slow_recursion(n, p >> 1, mod);
        res = (res * res) % mod;
        return p & 1 ? (n * res) % mod : res;
    }
    return 1ull;
}

template <typename T>
static constexpr T bin_pow(T n, uint64_t p) noexcept {
    T res = 1;
    do {
        if (p & 1) {
            res *= n;
        }
        n *= n;
        p >>= 1;
    } while(p);

    return res;
}

static constexpr uint64_t bin_pow(uint64_t n, uint64_t p, uint64_t mod) noexcept {
    uint64_t res = 1;
    do {
        if (p & 1) {
            res = (res * n) % mod;
        }
        n = (n * n) % mod;
        p >>= 1;
    } while(p);

    return res;
}

int main() {
    printf("%llu\n", bin_pow_slow_recursion(3, 19)); // 1162261467
    printf("%llu\n", bin_pow(3ull, 19)); // 1162261467
    printf("%llu\n", bin_pow_slow_recursion(3, 19, 1e9 + 7)); // 162261460
    printf("%llu\n", bin_pow(3, 19, 1e9 + 7)); // 162261460
    return (0);
}
