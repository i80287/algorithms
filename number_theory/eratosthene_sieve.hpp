#ifndef ERATOSTHENE_SIEVE_HPP
#define ERATOSTHENE_SIEVE_HPP

#include <bitset>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "math_functions.hpp"

namespace math_functions {

/// @brief Find all prime numbers in [2; n)
/// @param N exclusive upper bound
/// @return vector<bool>, for which bvec[n] == true \iff n is prime
std::vector<bool> primes_sieve_as_bvector(size_t n) {
    std::vector<bool> primes(n, true);
    if (likely(n >= 2)) {
        primes[0] = false;
        primes[1] = false;
    }
    const size_t root = math_functions::isqrt(n);
    for (size_t i = 2; i <= root; ++i) {
        if (primes[i]) {
            for (size_t j = uint32_t(i) * uint32_t(i); j < n; j += i) {
                primes[j] = false;
            }
        }
    }

    return primes;
}

/// @brief Find all prime numbers in [2; N)
/// @tparam N exclusive upper bound
/// @return bitset, for which bset[n] == true \iff n is prime
template <size_t N>
# if __cplusplus >= 202100L
constexpr
#endif
std::bitset<N>& primes_sieve_as_bitset() noexcept {
    static constinit std::bitset<N> primes;
    if constexpr (N > 2) {
        primes.set();
        primes[0]             = false;
        primes[1]             = false;
        constexpr size_t root = math_functions::isqrt(N);
        for (size_t i = 2; i <= root; i++) {
            if (primes[i]) {
                for (size_t j = uint32_t(i) * uint32_t(i); j < N; j += i) {
                    primes[j] = false;
                }
            }
        }
    }

    return primes;
}

}  // namespace math_functions

#endif
