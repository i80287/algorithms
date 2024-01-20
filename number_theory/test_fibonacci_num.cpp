#include <cassert>

#include "fibonacci_num.hpp"

using math_functions::fibonacci_num;
using math_functions::fibonacci_num_u128;

template <uint32_t k>
static void test_fib_u64() noexcept {
    static_assert(fibonacci_num(0) == 1);
    static_assert(fibonacci_num(1) == 1);
    uint64_t prev_prev_fib = 1;
    uint64_t prev_fib = 1;
    for (uint32_t i = 2; i < k; i++) {
        uint64_t current_fib = prev_prev_fib + prev_fib;
        assert(fibonacci_num(i) == current_fib);
        prev_prev_fib = prev_fib;
        prev_fib = current_fib;
    }
}

template <uint32_t k>
static void test_fib_u128() noexcept {
    static_assert(fibonacci_num_u128(0) == 1);
    static_assert(fibonacci_num_u128(1) == 1);
    uint128_t prev_prev_fib = 1;
    uint128_t prev_fib = 1;
    for (uint32_t i = 2; i < k; i++) {
        uint128_t current_fib = prev_prev_fib + prev_fib;
        assert(fibonacci_num_u128(i) == current_fib);
        prev_prev_fib = prev_fib;
        prev_fib = current_fib;
    }
}

int main() {
    constexpr uint32_t k = 1u << 24;
    test_fib_u64<k>();
    test_fib_u128<k>();
}
