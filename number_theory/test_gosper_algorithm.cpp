#include <cassert>
#include <clocale>
#include <cstdio>
#include <cwchar>

#include "gosper_algorithm.hpp"

static constexpr int32_t f1(int32_t x) noexcept {
    return int32_t(uint32_t(x) + 1024);
}

static constexpr int32_t f2(int32_t x) noexcept {
    return int32_t(math_functions::uabs(x) + 1000);
}

static void show_to_console() noexcept {
    const int32_t x0 = 0;
    const auto [mu_lower, mu_upper, lambda] =
        math_functions::loop_detection_Gosper(f1, x0);
    const bool utf_letters = setlocale(LC_ALL, "el_GR.utf8") != nullptr ||
                             setlocale(LC_ALL, "el_GR") != nullptr ||
                             setlocale(LC_ALL, "greek") != nullptr ||
                             setlocale(LC_ALL, "en_US.utf8") != nullptr;
    if (utf_letters) {
        std::wprintf(L"start point μ0 in [%d %u] | period λ = %u\n", mu_lower,
                     mu_upper, lambda);
    } else {
        std::printf("start point mu_0 in [%d %u] | period lambda = %u\n",
                    mu_lower, mu_upper, lambda);
    }
}

static void test1() noexcept {
    const int32_t x0 = 0;
    const auto [mu_lower, mu_upper, lambda] =
        math_functions::loop_detection_Gosper(f2, x0);
    assert(lambda == 2);
    assert(uint32_t(mu_lower) == mu_upper);

    int32_t x = x0;
    for (uint32_t k = 0; k <= mu_upper; k++) x = f2(x);

    for (int i = 0; i < 1000; i++) {
        x = f2(x);
        int y = f2(f2(x));
        assert(x == y);
    }
}

static void test2() noexcept {
    const int32_t x0 = 0;

    for (int32_t p : {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 33}) {
        const auto [mu_lower, mu_upper, lambda] =
            math_functions::loop_detection_Gosper(
                [=](int32_t n) constexpr noexcept {
                    return ((n + 1) % p + p) % p;
                },
                x0);
        assert(lambda == uint32_t(p));
    }
}

int main() {
    test1();
    test2();
    show_to_console();
}
