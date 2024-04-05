#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "is_prime.hpp"

static std::vector<uint64_t> read_primes() {
    struct Wrapper final {
        FILE* const file;
        Wrapper(const char* fname, const char* mode) noexcept
            : file(fopen(fname, mode)) {}
        ~Wrapper() {
            if (file) {
                fclose(file);
            }
        }
    };

    std::vector<uint64_t> nums;
    nums.reserve(1065000zu);
    Wrapper fin("u64-primes.txt", "r");
    if (fin.file == nullptr) [[unlikely]] {
        throw std::runtime_error("fopen");
    }
    while (true) {
        uint64_t n = 0;
        switch (fscanf(fin.file, "%" PRIu64, &n)) {
            [[likely]] case 1:
                nums.push_back(n);
                break;
            [[unlikely]] case -1:
                return nums;
            [[unlikely]] default:
                throw std::runtime_error("fscanf");
        }
    }
}

static volatile bool side_effect_ensurer{};

static auto run_measurements(const std::vector<uint64_t>& primes) {
    const auto start = std::chrono::high_resolution_clock::now();
    for (uint64_t prime : primes) {
        side_effect_ensurer = math_functions::is_prime_bpsw(prime);
    }
    const auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                 start);
}

int main() {
    const auto primes = read_primes();
    for (uint64_t prime : primes) {
        side_effect_ensurer = prime != 0;
    }

    for (auto iter = 4zu; iter != 0; iter--) {
        std::cout << run_measurements(primes) << '\n';
    }
}
