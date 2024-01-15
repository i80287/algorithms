#include <cassert>

#include "fibonacci_fast.hpp"

int main() {
    static_assert(fibonacci_num(0) == 1);
    static_assert(fibonacci_num(1) == 1);
    constexpr uint32_t k = 1048576;
    uint64_t prev_prev_fib = 1;
    uint64_t prev_fib = 1;
    for (uint32_t i = 2; i < k; i++) {
        uint64_t current_fib = prev_prev_fib + prev_fib;
        assert(fibonacci_num(i) == current_fib);
        prev_prev_fib = prev_fib;
        prev_fib = current_fib;
    }
}
