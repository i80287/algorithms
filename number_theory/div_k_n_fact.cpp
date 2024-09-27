#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <map>
#include <limits>

#include "math_functions.hpp"

/// @brief Return max q, such that n! % k^q == 0, where k > 1 and n >= 0
///        Using math notation: n! ≡ 0 mod(k^q)
/// @param n
/// @param k
/// @return q if k > 1 and std::numeric_limits<std::uint32_t>::max() otherwise
std::uint32_t div_k_n_fact(std::uint32_t n, std::uint32_t k) {
    // let k  = p_1^a_1 * p_2^a_2 * ... * p_m^a_m
    // let n! = p_1^b_1 * p_2^b_2 * ...
    // then b_i = n / p_i + n / p_i^2 + n / p_i^3 + ...
    // q = min{ b_i / a_i | 1 <= i <= m }

    auto ans = std::numeric_limits<std::uint32_t>::max();
    for (const auto [p_i, prime_factor_power] : ::math_functions::prime_factors_as_vector(k)) {
        uint64_t pow_of_p_i = p_i;
        // max b_i can be reached if n = 4294967295 and p_i = 2
        // then b_i = 4294967295 / 2 + 4294967295 / 4 + 4294967295 / 8 + ... + 4294967295 /
        // 2147483648 + 4294967295 / 4294967296 = = 4294967295 * (1 - 1/(2^31)) = 4294967295 -
        // 4294967295/(2^31) < 4294967295 - 2147483648/(2^31) = 4294967294
        std::uint32_t b_i = 0;
        for (std::uint32_t count{}; (count = std::uint32_t(n / pow_of_p_i)) != 0;) {
            pow_of_p_i *= p_i;
            b_i += count;
        }

        std::uint32_t a_i = prime_factor_power;
        ans               = std::min(ans, b_i / a_i);
    }

    return ans;
}

int main() {
    assert(div_k_n_fact(12, 6) == 5);
    assert(div_k_n_fact(6, 3) == 2);
    assert(div_k_n_fact(6, 12) == 2);
    assert(div_k_n_fact(0xffffffffu, 2) == 4294967263u);
}
