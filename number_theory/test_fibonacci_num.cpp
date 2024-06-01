#include <cassert>

#include "fibonacci_num.hpp"
#include "test_tools.hpp"

template <uint32_t k>
static void test_fib_u64() noexcept {
    log_tests_started();

    static_assert(math_functions::fibonacci_num(0) == 1);
    static_assert(math_functions::fibonacci_num(1) == 1);
    uint64_t prev_prev_fib = 1;
    uint64_t prev_fib      = 1;
    for (uint32_t i = 2; i < k; i++) {
        uint64_t current_fib = prev_prev_fib + prev_fib;
        auto [f_n_1, f_n]    = math_functions::fibonacci_nums(i);
        assert(f_n_1 == prev_fib);
        assert(f_n == current_fib);
        f_n = math_functions::fibonacci_num(i);
        assert(f_n == current_fib);
        prev_prev_fib = prev_fib;
        prev_fib      = current_fib;
    }
}

template <uint32_t k>
static void test_fib_u128() noexcept {
    log_tests_started();

    static_assert(math_functions::fibonacci_num_u128(0) == 1);
    static_assert(math_functions::fibonacci_num_u128(1) == 1);
    uint128_t prev_prev_fib = 1;
    uint128_t prev_fib      = 1;
    for (uint32_t i = 2; i < k; i++) {
        uint128_t current_fib = prev_prev_fib + prev_fib;
        auto [f_n_1, f_n]     = math_functions::fibonacci_nums_u128(i);
        assert(f_n_1 == prev_fib);
        assert(f_n == current_fib);
        f_n = math_functions::fibonacci_num_u128(i);
        assert(f_n == current_fib);
        prev_prev_fib = prev_fib;
        prev_fib      = current_fib;
    }
}

int main() {
    constexpr uint32_t k = 1u << 24;
    test_fib_u64<k>();
    test_fib_u128<k>();
}
