#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "../misc/do_not_optimize_away.h"
#include "../misc/tests/test_tools.hpp"
#include "is_prime.hpp"

namespace {

std::vector<uint64_t> read_primes() {
    using test_tools::FilePtr;

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
                std::perror("fscanf");
                throw std::runtime_error("fscanf");
        }
    }
}

std::chrono::nanoseconds run_measurements(const std::vector<uint64_t>& primes) {
    const auto start = std::chrono::high_resolution_clock::now();
    for (uint64_t prime : primes) {
        config::do_not_optimize_away(math_functions::is_prime_bpsw(prime));
    }
    const auto end = std::chrono::high_resolution_clock::now();
    return end - start;
}

}  // namespace

int main() {
    const auto primes = read_primes();
    for (uint64_t prime : primes) {
        config::do_not_optimize_away(prime != 0);
    }

    for (auto iter = 4zu; iter != 0; iter--) {
        const std::chrono::nanoseconds time_ms = run_measurements(primes);
        const std::uint64_t ns = static_cast<std::uint64_t>(time_ms.count());
        const std::uint64_t ns_per_primes = ns / primes.size();
        std::printf("%" PRIu64
                    " nano seconds\n"
                    "%" PRIu64 " nano seconds per prime on average\n",
                    ns, ns_per_primes);
    }
}
