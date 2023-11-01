#include <cassert>
#include <cstdint>
#include <cmath>
#include <map>
#include <iostream>

inline void factorize(uint32_t k, std::map<uint32_t, uint32_t>& prime_divisors) {
    if (k == 0) {
        return;
    }

    if (k % 2 == 0) { // we dont need to optimize it like (k & 1) == 0, compiler will do it anyway
        // k = s * 2^pow_of_2, where s is odd
        uint32_t pow_of_2 = uint32_t(__builtin_ctz(k));
        k >>= pow_of_2;
        prime_divisors.emplace(2u, pow_of_2);
    }

    for (uint32_t d = 3; d * d <= k; d += 2) {
        if (k % d == 0) {
            uint32_t pow_of_d = 0;
            do {
                pow_of_d++;
                k /= d;
            } while (k % d == 0);
            prime_divisors.emplace(d, pow_of_d);
        }
    }

    if (k != 1) {
        prime_divisors.emplace(k, 1u);
    }
}

/// @brief Returns max q, such that n! % k^q == 0, where k > 1 and n >= 1
/// @param n 
/// @param k 
/// @return q (if k = 1, returns 4294967295)
uint32_t div_k_n_fact(uint32_t n, uint32_t k) {
    // let k  = p_1^a_1 * p_2^a_2 * ... * p_q^a_q
    // let n! = p_1^b_1 * p_2^b_2 * ...
    // then b_i = n / p_i + n / p_i^2 + n / p_i^3 + ...
    // q = min{ b_i / a_i | 1 <= i <= q }

    std::map<uint32_t, uint32_t> k_prime_divisors;
    factorize(k, k_prime_divisors);
    uint32_t ans = uint32_t(-1);
    for (const auto& p : k_prime_divisors) {
        uint32_t p_i = p.first;
        uint64_t pow_of_p_i = p_i;
        // max b_i can be reached if n = 4294967295 and p_i = 2
        // then b_i = 4294967295 / 2 + 4294967295 / 4 + 4294967295 / 8 + ... + 4294967295 / 2147483648 + 4294967295 / 4294967296 =
        // = 4294967295 * (1 - 1/(2^31)) = 4294967295 - 4294967295/(2^31) < 4294967295 - 2147483648/(2^31) = 4294967294
        uint32_t b_i = 0;
        uint32_t count;
        while ((count = uint32_t(n / pow_of_p_i)) != 0) {
            pow_of_p_i *= p_i;
            b_i += count;
        }

        uint32_t a_i = p.second;
        ans = std::min(ans, b_i / a_i);
    }

    return ans;
}

int main() {
    assert(div_k_n_fact(12, 6) == 5);
    assert(div_k_n_fact(6, 3) == 2);
    assert(div_k_n_fact(6, 12) == 2);
    assert(div_k_n_fact(0xffffffffu, 2) == 4294967263u);
}
