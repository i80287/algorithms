#include <cassert>
#include "eratosthene_sieve.hpp"

int main() {
    constexpr size_t N                  = 100;
    std::vector<bool> primes_as_bvector = math_functions::primes_sieve_as_bvector(N);
    std::bitset<N>& primes_bset         = math_functions::primes_sieve_as_bitset<N>();
    constexpr uint32_t primes[] = {2,  3,  5,  7,   11,  13,  17,  19,  23, 29, 31,
                                   37, 41, 43, 47,  53,  59,  61,  67,  71, 73, 79,
                                   83, 89, 97, 101, 103, 107, 109, 113, 127};
    size_t i                    = 0;
    for (uint32_t n = 0; n < N; n++) {
        if (primes[i] < n) {
            i++;
        }
        bool is_prime = primes[i] == n;
        assert(primes_as_bvector[n] == is_prime);
        assert(primes_bset[n] == is_prime);
    }
}
