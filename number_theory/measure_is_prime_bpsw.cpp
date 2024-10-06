#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "is_prime.hpp"
#include "test_tools.hpp"

using namespace test_tools;

static std::vector<uint64_t> read_primes() {
    std::vector<uint64_t> nums;
    nums.reserve(1065000zu);
    for (FilePtr fin("u64-primes.txt", "r");;) {
        switch (uint64_t n = 0; std::fscanf(fin, "%" PRIu64, &n)) {
            [[likely]] case 1:
                nums.push_back(n);
                break;
            [[unlikely]] case std::char_traits<char>::eof():
                return nums;
            [[unlikely]] default:
                perror("fscanf");
                throw std::runtime_error("fscanf");
        }
    }
}

static volatile bool side_effect_ensurer{};

static std::chrono::nanoseconds run_measurements(const std::vector<uint64_t>& primes) {
    const auto start = std::chrono::high_resolution_clock::now();
    for (uint64_t prime : primes) {
        side_effect_ensurer = math_functions::is_prime_bpsw(prime);
    }
    const auto end = std::chrono::high_resolution_clock::now();
    return end - start;
}

int main() {
    const auto primes = read_primes();
    for (uint64_t prime : primes) {
        side_effect_ensurer = prime != 0;
    }

    for (auto iter = 4zu; iter != 0; iter--) {
        const std::chrono::nanoseconds time_ms = run_measurements(primes);
        const std::uint64_t ns                 = static_cast<std::uint64_t>(time_ms.count());
        const std::uint64_t ns_per_primes      = ns / primes.size();
        printf("%" PRIu64
               " nano seconds\n"
               "%" PRIu64 " nano seconds per prime on average\n",
               ns, ns_per_primes);
    }
}
