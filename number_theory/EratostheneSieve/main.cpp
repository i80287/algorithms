#include <vector>
#include <bitset>
#include <cstdint>
#include <cstddef>
#include <cmath>
#include <iostream>

/// @brief Print all prime numbers in [2; N)
/// @param N exclusive upper border
void print_sieve1(size_t N) {
    if (N <= 2) {
        return;
    }

    std::vector<bool> primes(N, 1);
    primes[0] = primes[1] = false;

    const size_t root = static_cast<size_t>(std::sqrt(N));
    for (size_t i = 2; i <= root; ++i) {
        if (primes[i]) {
            for (size_t j = i * i; j < N; j += i) {
                primes[j] = false;
            }
        }
    }

    for (size_t i = 2; i < N; i++) {
        if (primes[i])
            std::cout << i << ' ';
    }
}

/// @brief Print all prime numbers in [2; N)
/// @param N exclusive upper border
template <size_t N>
void print_sieve2() {
    if constexpr (N <= 2) {
        return;
    }

    std::bitset<N> primes;
    primes.set();
    primes[0] = primes[1] = false;

    constexpr size_t root = static_cast<size_t>(std::sqrt(N));
    for (size_t i = 0; i <= root; i++)
    {
        if (primes[i]) {
            for (size_t j = i * i; j < N; j += i) {
                primes[j] = false;
            }
        }
    }

    for (size_t i = 2; i < N; i++) {
        if (primes[i])
            std::cout << i << ' ';
    }
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    print_sieve1(100);
    std::cout << '\n';
    print_sieve2<100>();
    std::cout << std::endl;
}
