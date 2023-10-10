#include <cassert>
#include <cstdint>
#include <bitset>
#include <cmath>
#include <iostream>
#include <numeric>
#include <string>
#include <type_traits>
#include <tuple>
#include <utility>
#include <vector>

static constexpr uint64_t bin_pow(uint64_t n, uint64_t p) noexcept {
    uint64_t res = 1;
    do {
        if (p & 1) {
            res *= n;
        }
        n *= n;
        p >>= 1;
    } while(p);

    return res;
}

int main() {
    constexpr size_t N = 1e7 + 1;
    static std::bitset<N> primes;
    primes.set();
    primes[0] = primes[1] = false;

    constexpr size_t root = static_cast<size_t>(std::sqrt(N));
    for (size_t i = 0; i <= root; i++) {
        if (primes[i]) {
            for (size_t j = i * i; j < N; j += i) {
                primes[j] = false;
            }
        }
    }

    uint32_t n = 0;
    std::cin >> n;
    std::vector<uint32_t> primes_vec;
    std::vector<uint64_t> divs_sum(n + 1);
    divs_sum[1] = 1;
    std::vector<uint32_t> divs_count(n + 1);
    divs_count[1] = 1;
    std::vector<uint32_t> euler_func(n + 1);
    euler_func[1] = 1;
    primes_vec.reserve(n);
    for (uint32_t i = 2; i <= n; i++) {
        if (primes[i]) {
            divs_sum[i] = i + 1;
            divs_count[i] = 2;
            euler_func[i] = i - 1;
            primes_vec.push_back(i);
        }
    }

    /*
     * 1 <= n <= 1e7
     * Time limit: 5 seconds
     * Memory limit: 512 mb
     * 
     * Task J. Rucode festival
     * 
     * d(n) - minimal divisor of n that is greater then 1. d(1) := 0
     * s_0(n) - count of unique divisors of n
     * s_1(n) - the sum of all divisors of n
     * phi(n) - Euler function
     * 
     * sum1 = \sum{k=0}{n} d(n)
     * sum2 = \sum{k=0}{n} s_0(n)
     * sum3 = \sum{k=0}{n} s_1(n)
     * sum4 = \sum{k=0}{n} phi(n)
    */

    uint64_t sum1 = 0;
    uint64_t sum2 = 1; // for 1
    uint64_t sum3 = divs_sum[1];
    uint64_t sum4 = euler_func[1];
    for (uint32_t k = 2; k <= n; k++) {
        if (primes[k]) {
            sum1 += k;
            sum2 += 2;
            sum3 += k + 1;
            sum4 += k - 1;
            continue;
        }

        for (uint32_t prime_number : primes_vec) {
            assert(k > prime_number);
            if (k % prime_number == 0) {
                sum1 += prime_number;

                uint32_t prime_power = 0;
                uint32_t current_k = k;
                do {
                    current_k /= prime_number;
                    prime_power++;
                } while (current_k % prime_number == 0);

                uint64_t p_m1 = bin_pow(prime_number, prime_power - 1);

                uint64_t k_divs_sum = divs_sum[current_k] * ((p_m1 * prime_number * prime_number - 1) / (prime_number - 1));
                sum3 += k_divs_sum;
                divs_sum[k] = k_divs_sum;

                uint32_t k_divs_count = divs_count[current_k] * (prime_power + 1);
                sum2 += k_divs_count;
                divs_count[k] = k_divs_count;

                uint32_t k_euler_func = uint32_t(euler_func[current_k] * (p_m1 * (prime_number - 1)));
                sum4 += k_euler_func;
                euler_func[k] = k_euler_func;
                break;
            }
        }
    }

    std::cout << sum1 << ' ' << sum2 << ' ' << sum3 << ' ' << sum4 << '\n';
}
