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
std::vector<bool> primes_sieve_as_bvector(uint32_t n) {
    std::vector<bool> primes(n, true);
    if (likely(n >= 2)) {
        primes[0] = false;
        primes[1] = false;
    }
    const uint32_t root = math_functions::isqrt(n);
    for (uint32_t i = 2; i <= root; ++i) {
        if (primes[i]) {
            for (uint32_t j = uint32_t(i) * uint32_t(i); j < n; j += i) {
                primes[j] = false;
            }
        }
    }

    return primes;
}

/// @brief Find all prime numbers in [2; N)
/// @tparam N exclusive upper bound
/// @return bitset, for which bset[n] == true \iff n is prime
template <uint32_t N>
#if __cplusplus >= 202100L
constexpr
#endif
    std::bitset<N>&
    primes_sieve_as_bitset() noexcept {
#if defined(__cpp_constinit) && __cpp_constinit >= 201907L
    constinit
#endif
        static std::bitset<N>
            primes{};
    if constexpr (N > 2) {
        primes.set();
        primes[0]               = false;
        primes[1]               = false;
        constexpr uint32_t root = math_functions::isqrt(uint64_t(N));
        for (uint32_t i = 2; i <= root; i++) {
            if (primes[i]) {
                for (uint32_t j = i * i; j < N; j += i) {
                    primes[j] = false;
                }
            }
        }
    }

    return primes;
}

}  // namespace math_functions

#endif
