#include <array>
#include <cassert>
#include <clocale>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cwchar>

#include "../misc/config_macros.hpp"
#include "../misc/test_tools.hpp"
#include "gosper_algorithm.hpp"
#include "math_functions.hpp"

// NOLINTBEGIN(cert-dcl03-c, misc-static-assert, hicpp-static-assert,
// cppcoreguidelines-avoid-magic-numbers)

namespace {

using std::int32_t;
using std::size_t;
using std::uint32_t;

[[nodiscard]] ATTRIBUTE_CONST constexpr int32_t f2(int32_t x) noexcept {
    return static_cast<int32_t>(math_functions::uabs(x) + 1000);
}

[[nodiscard]] ATTRIBUTE_CONST constexpr int32_t f3(int32_t x) noexcept {
    constexpr int32_t m = 19;
    constexpr int32_t c = 100;
    x %= m;
    return (x * x + c) % m;
}

void test1() noexcept {
    test_tools::log_tests_started();
    const int32_t x0 = 0;
    const auto [mu_lower, mu_upper, lambda] = math_functions::loop_detection_Gosper(f2, x0);
    assert(lambda == 2);
    assert(mu_lower == mu_upper);

    int32_t x = x0;
    for (size_t k = 0; k <= mu_upper; k++) {
        x = f2(x);
    }

    for (int i = 0; i < 1000; i++) {
        x = f2(x);
        assert(x == f2(f2(x)));
    }
}

void test2() noexcept {
    test_tools::log_tests_started();
    constexpr int32_t x0 = 0;
    for (const int32_t p : {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 33}) {
        const auto [mu_lower, mu_upper, lambda] = math_functions::loop_detection_Gosper(
            [p](const int32_t n) constexpr noexcept { return ((n + 1) % p + p) % p; }, x0);
        assert(lambda == static_cast<uint32_t>(p));
        assert(mu_lower <= mu_upper);
    }
}

void test3() noexcept {
    test_tools::log_tests_started();
    const int32_t x0 = 0;
    const auto [mu_lower, mu_upper, lambda] = math_functions::loop_detection_Gosper(f3, x0);
    assert(mu_lower <= mu_upper);
    int xi = x0;
    for (size_t i = 1; i <= mu_upper; i++) {
        xi = f3(xi);
    }
    constexpr size_t kPeriod = 4;
    assert(lambda == kPeriod);
    using PeriodicValues = std::array<int32_t, kPeriod>;
    auto fillarray = [](PeriodicValues& mem, int32_t x_start) constexpr noexcept {
        mem[0] = x_start;
        for (size_t i = 1; i < mem.size(); i++) {
            mem[i] = f3(mem[i - 1]);
        }
    };
    PeriodicValues mem{};
    fillarray(mem, xi);
    for (size_t iter = 100; iter > 0; iter--) {
        PeriodicValues next_mem{};
        fillarray(next_mem, f3(mem.back()));
        assert(mem == next_mem);
        mem = next_mem;
    }
}

}  // namespace

// NOLINTEND(cert-dcl03-c, misc-static-assert, hicpp-static-assert,
// cppcoreguidelines-avoid-magic-numbers)

int main() {
    test1();
    test2();
    test3();
}
