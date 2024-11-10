#include <cassert>
#include <cstdint>

#include "fibonacci_num.hpp"
#include "test_tools.hpp"

// NOLINTBEGIN(cert-dcl03-c, misc-static-assert, hicpp-static-assert,
// cppcoreguidelines-avoid-magic-numbers)

namespace {

using namespace test_tools;

template <uint32_t k>
void test_fib_u64() noexcept {
    log_tests_started();

    static_assert(math_functions::fibonacci_num(0) == 1);
    static_assert(math_functions::fibonacci_num(1) == 1);
    uint64_t prev_prev_fib = 1;
    uint64_t prev_fib      = 1;
    for (uint32_t n = 2; n < k; n++) {
        const uint64_t current_fib = prev_prev_fib + prev_fib;
        const auto [f_n_1, f_n]    = math_functions::fibonacci_nums(n);
        assert(f_n_1 == prev_fib);
        assert(f_n == current_fib);
        assert(f_n == math_functions::fibonacci_num(n));
        prev_prev_fib = prev_fib;
        prev_fib      = current_fib;
    }
}

template <uint32_t k>
void test_fib_u128() noexcept {
    log_tests_started();
#if defined(HAS_I128_CONSTEXPR)
    static_assert(math_functions::fibonacci_num_u128(0) == 1);
    static_assert(math_functions::fibonacci_num_u128(1) == 1);
#endif
    assert(math_functions::fibonacci_num_u128(0) == 1);
    assert(math_functions::fibonacci_num_u128(1) == 1);
    uint128_t prev_prev_fib = 1;
    uint128_t prev_fib      = 1;
    for (uint32_t n = 2; n < k; n++) {
        const uint128_t current_fib = prev_prev_fib + prev_fib;
        const auto [f_n_1, f_n]     = math_functions::fibonacci_nums_u128(n);
        assert(f_n_1 == prev_fib);
        assert(f_n == current_fib);
        assert(f_n == math_functions::fibonacci_num_u128(n));
        prev_prev_fib = prev_fib;
        prev_fib      = current_fib;
    }
}

}  // namespace

// NOLINTEND(cert-dcl03-c, misc-static-assert, hicpp-static-assert,
// cppcoreguidelines-avoid-magic-numbers)

int main() {
    constexpr uint32_t k = 1U << 24U;
    test_fib_u64<k>();
    test_fib_u128<k>();
}
