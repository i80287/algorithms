#include <array>
#include <cassert>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <numeric>

#include "config_macros.hpp"
#include "fibonacci_num.hpp"
#include "kronecker_symbol.hpp"
#include "math_functions.hpp"
#include "test_tools.hpp"

using math_functions::fibonacci_num;
using math_functions::kronecker_symbol;

#if CONFIG_HAS_INCLUDE(<gmpxx.h>)
#include <gmp.h>
#include <gmpxx.h>
#define HAS_GMPXX_DURING_TESTING
#else
#if defined(__linux__) && !defined(__MINGW32__)
#error "gmpxx tests should be available on linux"
#endif
#endif

// clang-format off
// NOLINTBEGIN(cert-dcl03-c, misc-static-assert, hicpp-static-assert, cppcoreguidelines-avoid-magic-numbers)
// clang-format on

namespace {

using std::int32_t;
using std::int64_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

/// @brief Kronecker symbols (n/k) for 1 <= n <= 30 and 1 <= k <= 30
///        Here (n/k) = krnk[n - 1][k - 1] (using zero indexing notation)
inline constexpr std::array<std::array<int32_t, 30>, 30> krnk = {{
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, -1, 0,  -1, 0,  1, 0, 1, 0, -1, 0,  -1, 0,  1,
     0, 1, 0,  -1, 0,  -1, 0, 1, 0, 1, 0,  -1, 0,  -1, 0},
    {1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0,
     1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0},
    {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
    {1, -1, -1, 1, 0, 1, -1, -1, 1, 0, 1, -1, -1, 1, 0,
     1, -1, -1, 1, 0, 1, -1, -1, 1, 0, 1, -1, -1, 1, 0},
    {1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, -1, 0, 0, 0, -1, 0, -1, 0, 0, 0, -1, 0, 1, 0, 0, 0, 1, 0},
    {1, 1,  -1, 1,  -1, -1, 0, 1, 1,  -1, 1,  -1, -1, 0, 1,
     1, -1, 1,  -1, -1, 0,  1, 1, -1, 1,  -1, -1, 0,  1, 1},
    {1, 0, -1, 0,  -1, 0,  1, 0, 1, 0, -1, 0,  -1, 0,  1,
     0, 1, 0,  -1, 0,  -1, 0, 1, 0, 1, 0,  -1, 0,  -1, 0},
    {1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0},
    {1, 0,  1, 0,  0, 0,  -1, 0,  1, 0, -1, 0, 1, 0,  0,
     0, -1, 0, -1, 0, -1, 0,  -1, 0, 0, 0,  1, 0, -1, 0},
    {1, -1, 1,  1,  1, -1, -1, -1, 1,  -1, 0, 1, -1, 1,  1,
     1, -1, -1, -1, 1, -1, 0,  1,  -1, 1,  1, 1, -1, -1, -1},
    {1, 0, 0, 0, -1, 0, 1, 0, 0, 0, -1, 0, 1, 0, 0, 0, -1, 0, 1, 0, 0, 0, -1, 0, 1, 0, 0, 0, -1, 0},
    {1, -1, 1,  1,  -1, -1, -1, -1, 1,  1, -1, 1, 0,  1, -1,
     1, 1,  -1, -1, -1, -1, 1,  1,  -1, 1, 0,  1, -1, 1, 1},
    {1, 0, 1, 0, 1, 0, 0, 0, 1, 0, -1, 0, 1, 0, 1, 0, -1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, -1, 0},
    {1, 1, 0, 1, 0, 0, -1, 1, 0, 0, -1, 0, -1, -1, 0,
     1, 1, 0, 1, 0, 0, -1, 1, 0, 0, -1, 0, -1, -1, 0},
    {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
    {1, 1, -1, 1, -1, -1, -1, 1,  1,  -1, -1, -1, 1,  -1, 1,
     1, 0, 1,  1, -1, 1,  -1, -1, -1, 1,  1,  -1, -1, -1, 1},
    {1, 0, 0, 0, -1, 0, 1, 0, 0, 0, -1, 0, -1, 0, 0, 0, 1, 0, -1, 0, 0, 0, 1, 0, 1, 0, 0, 0, -1, 0},
    {1, -1, -1, 1, 1, 1,  1,  -1, 1, -1, 1, -1, -1, -1, -1,
     1, 1,  -1, 0, 1, -1, -1, 1,  1, 1,  1, -1, 1,  -1, 1},
    {1, 0,  -1, 0, 0, 0, -1, 0,  1, 0, 1, 0,  -1, 0, 0,
     0, -1, 0,  1, 0, 1, 0,  -1, 0, 0, 0, -1, 0,  1, 0},
    {1, -1, 0, 1,  1, 0, 0, -1, 0, -1, -1, 0, -1, 0,  0,
     1, 1,  0, -1, 1, 0, 1, -1, 0, 1,  1,  0, 0,  -1, 0},
    {1, 0, -1, 0, -1, 0, -1, 0, 1, 0, 0, 0, 1, 0, 1, 0, -1, 0, 1, 0, 1, 0, 1, 0, 1, 0, -1, 0, 1, 0},
    {1, 1,  1, 1,  -1, 1,  -1, 1, 1, -1, -1, 1, 1,  -1, -1,
     1, -1, 1, -1, -1, -1, -1, 0, 1, 1,  1,  1, -1, 1,  -1},
    {1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, -1, 0, 0, 0, -1, 0, -1, 0, 0, 0, -1, 0, 1, 0, 0, 0, 1, 0},
    {1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0},
    {1, 0, -1, 0, 1, 0, -1, 0, 1, 0, 1, 0, 0, 0, -1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, -1, 0, -1, 0},
    {1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0,
     1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0},
    {1, 0,  -1, 0,  -1, 0, 0, 0, 1, 0, 1, 0,  -1, 0, 1,
     0, -1, 0,  -1, 0,  0, 0, 1, 0, 1, 0, -1, 0,  1, 0},
    {1, -1, -1, 1,  1, 1,  1, -1, 1, -1, -1, -1, 1, -1, -1,
     1, -1, -1, -1, 1, -1, 1, 1,  1, 1,  -1, -1, 1, 0,  1},
    {1, 0, 0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0},
}};

/// @brief First 30 odd prime numbers
inline constexpr std::array<uint32_t, 30> odd_primes = {
    3,  5,  7,  11, 13, 17, 19, 23, 29, 31,  37,  41,  43,  47,  53,
    59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127,
};

/// @brief Legendre symbols (a/p) for 1 <= a <= 30 and 3 <= p <= 127, p is prime
inline constexpr std::array<std::array<int32_t, 30>, 30> lgnr = {{
    {1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0,
     1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0, 1, -1, 0},
    {1, -1, -1, 1, 0, 1, -1, -1, 1, 0, 1, -1, -1, 1, 0,
     1, -1, -1, 1, 0, 1, -1, -1, 1, 0, 1, -1, -1, 1, 0},
    {1, 1,  -1, 1,  -1, -1, 0, 1, 1,  -1, 1,  -1, -1, 0, 1,
     1, -1, 1,  -1, -1, 0,  1, 1, -1, 1,  -1, -1, 0,  1, 1},
    {1, -1, 1,  1,  1, -1, -1, -1, 1,  -1, 0, 1, -1, 1,  1,
     1, -1, -1, -1, 1, -1, 0,  1,  -1, 1,  1, 1, -1, -1, -1},
    {1, -1, 1,  1,  -1, -1, -1, -1, 1,  1, -1, 1, 0,  1, -1,
     1, 1,  -1, -1, -1, -1, 1,  1,  -1, 1, 0,  1, -1, 1, 1},
    {1, 1, -1, 1, -1, -1, -1, 1,  1,  -1, -1, -1, 1,  -1, 1,
     1, 0, 1,  1, -1, 1,  -1, -1, -1, 1,  1,  -1, -1, -1, 1},
    {1, -1, -1, 1, 1, 1,  1,  -1, 1, -1, 1, -1, -1, -1, -1,
     1, 1,  -1, 0, 1, -1, -1, 1,  1, 1,  1, -1, 1,  -1, 1},
    {1, 1,  1, 1,  -1, 1,  -1, 1, 1, -1, -1, 1, 1,  -1, -1,
     1, -1, 1, -1, -1, -1, -1, 0, 1, 1,  1,  1, -1, 1,  -1},
    {1, -1, -1, 1,  1, 1,  1, -1, 1, -1, -1, -1, 1, -1, -1,
     1, -1, -1, -1, 1, -1, 1, 1,  1, 1,  -1, -1, 1, 0,  1},
    {1, 1,  -1, 1, 1, -1, 1,  1,  1,  1, -1, -1, -1, 1,  -1,
     1, -1, 1,  1, 1, -1, -1, -1, -1, 1, -1, -1, 1,  -1, -1},
    {1, -1, 1,  1,  -1, -1, 1,  -1, 1,  1, 1, 1, -1, -1, -1,
     1, -1, -1, -1, -1, 1,  -1, -1, -1, 1, 1, 1, 1,  -1, 1},
    {1, 1,  -1, 1,  1, -1, -1, 1, 1,  1, -1, -1, -1, -1, -1,
     1, -1, 1,  -1, 1, 1,  -1, 1, -1, 1, -1, -1, -1, -1, -1},
    {1, -1, -1, 1,  -1, 1, -1, -1, 1, 1, 1,  -1, 1,  1,  1,
     1, 1,  -1, -1, -1, 1, -1, 1,  1, 1, -1, -1, -1, -1, -1},
    {1, 1, 1, 1,  -1, 1, 1,  1,  1, -1, -1, 1, -1, 1,  -1,
     1, 1, 1, -1, -1, 1, -1, -1, 1, 1,  -1, 1, 1,  -1, -1},
    {1, -1, -1, 1,  -1, 1,  1,  -1, 1, 1, 1,  -1, 1, -1, 1,
     1, 1,  -1, -1, -1, -1, -1, -1, 1, 1, -1, -1, 1, 1,  -1},
    {1, -1, 1,  1, 1, -1, 1, -1, 1,  -1, -1, 1, -1, -1, 1,
     1, 1,  -1, 1, 1, 1,  1, -1, -1, 1,  1,  1, 1,  1,  -1},
    {1, -1, 1,  1, 1, -1, -1, -1, 1,  -1, -1, 1, 1,  1,  1,
     1, -1, -1, 1, 1, -1, 1,  -1, -1, 1,  -1, 1, -1, -1, -1},
    {1, -1, -1, 1, -1, 1, -1, -1, 1, 1, -1, -1, -1, 1, 1,
     1, 1,  -1, 1, -1, 1, 1,  1,  1, 1, 1,  -1, -1, 1, -1},
    {1, 1,  1, 1, 1, 1,  -1, 1,  1, 1, -1, 1, -1, -1, 1,
     1, -1, 1, 1, 1, -1, -1, -1, 1, 1, -1, 1, -1, 1,  1},
    {1, 1,  1, 1, -1, 1,  -1, 1, 1, -1, -1, 1, -1, -1, -1,
     1, -1, 1, 1, -1, -1, -1, 1, 1, 1,  -1, 1, -1, -1, -1},
    {1, 1,  -1, 1, 1, -1, -1, 1, 1,  1, 1, -1, 1,  -1, -1,
     1, -1, 1,  1, 1, 1,  1,  1, -1, 1, 1, -1, -1, -1, -1},
    {1, -1, 1,  1,  -1, -1, 1,  -1, 1,  1, 1, 1, -1, -1, -1,
     1, 1,  -1, -1, -1, 1,  -1, 1,  -1, 1, 1, 1, 1,  1,  1},
    {1, 1, -1, 1,  1, -1, -1, 1,  1,  1, 1,  -1, -1, -1, -1,
     1, 1, 1,  -1, 1, 1,  1,  -1, -1, 1, -1, -1, -1, -1, -1},
    {1, 1,  1, 1,  -1, 1,  -1, 1,  1, -1, 1,  1, -1, -1, -1,
     1, -1, 1, -1, -1, -1, 1,  -1, 1, 1,  -1, 1, -1, -1, -1},
    {1, -1, -1, 1, 1, 1, -1, -1, 1, -1, -1, -1, 1,  1,  -1,
     1, 1,  -1, 1, 1, 1, 1,  1,  1, 1,  -1, -1, -1, -1, 1},
    {1, 1, -1, 1, -1, -1, 1,  1, 1,  -1, -1, -1, 1, 1, 1,
     1, 1, 1,  1, -1, -1, -1, 1, -1, 1,  1,  -1, 1, 1, 1},
    {1, -1, 1,  1, -1, -1, -1, -1, 1,  1, 1,  1, 1,  1, -1,
     1, -1, -1, 1, -1, -1, -1, 1,  -1, 1, -1, 1, -1, 1, 1},
    {1, -1, 1,  1,  1, -1, 1, -1, 1,  -1, -1, 1, -1, -1, 1,
     1, -1, -1, -1, 1, 1,  1, -1, -1, 1,  1,  1, 1,  1,  -1},
    {1, 1,  -1, 1,  -1, -1, 1, 1,  1,  -1, 1, -1, 1, 1,  1,
     1, -1, 1,  -1, -1, -1, 1, -1, -1, 1,  1, -1, 1, -1, 1},
    {1, 1, -1, 1, -1, -1, -1, 1,  1,  -1, 1, -1, 1,  -1, 1,
     1, 1, 1,  1, -1, 1,  1,  -1, -1, 1,  1, -1, -1, -1, 1},
}};

void CheckJacobi(uint32_t a, uint32_t n, int32_t real_jacobi) noexcept {
    // Special checker for a < 2^31 and n < 2^31
    assert(kronecker_symbol(a, n) == real_jacobi);
    assert(kronecker_symbol(static_cast<int32_t>(a), n) == real_jacobi);
    assert(kronecker_symbol(a, static_cast<int32_t>(n)) == real_jacobi);
    assert(kronecker_symbol(static_cast<int32_t>(a), static_cast<int32_t>(n)) == real_jacobi);

    assert(kronecker_symbol(uint64_t{a}, uint64_t{n}) == real_jacobi);
    assert(kronecker_symbol(uint64_t{a}, int64_t{n}) == real_jacobi);
    assert(kronecker_symbol(int64_t{a}, uint64_t{n}) == real_jacobi);
    assert(kronecker_symbol(int64_t{a}, int64_t{n}) == real_jacobi);

    // a >= 0 => (a/-1) = 1 => (a/n) = (a/-n)
    assert(kronecker_symbol(a, -static_cast<int32_t>(n)) == real_jacobi);
    assert(kronecker_symbol(static_cast<int32_t>(a), -static_cast<int32_t>(n)) == real_jacobi);
    assert(kronecker_symbol(int64_t{a}, -int64_t{n}) == real_jacobi);
    assert(kronecker_symbol(uint64_t{a}, -int64_t{n}) == real_jacobi);
}

template <uint16_t kLen>
void CheckJacobiBasic() noexcept {
    test_tools::log_tests_started();

    for (uint32_t n = 1; n <= 30; n++) {
        for (uint32_t k = 1; k <= 30; k++) {
            CheckJacobi(k, n, krnk[n - 1][k - 1]);
        }
    }

    for (int32_t k = -int32_t{kLen}; k <= int32_t{kLen}; k++) {
        const int32_t b = k == 1 || k == -1 ? 1 : 0;
        assert(kronecker_symbol(k, int32_t{0}) == b);
        assert(kronecker_symbol(k, uint32_t{0}) == b);
        assert(kronecker_symbol(int64_t{k}, int64_t{0}) == b);
        assert(kronecker_symbol(int64_t{k}, uint64_t{0}) == b);
    }

    for (size_t i = 0; i < 30; i++) {
        const uint32_t p = odd_primes[i];
        for (uint32_t a = 1; a <= 30; a++) {
            CheckJacobi(a, p, lgnr[i][a - 1]);
        }
    }

    // Check some properties of the Legendre/Kronecker/Jacobi symbol
    for (int32_t a = -int32_t{kLen}; a <= kLen; a++) {
        for (int32_t n = -int32_t{kLen}; n <= kLen; n++) {
            const uint32_t j_abs = math_functions::uabs(kronecker_symbol(a, n));
            assert(j_abs == uint32_t{std::gcd(a, n) == 1});
        }
    }

    for (const uint32_t p : odd_primes) {
        assert(kronecker_symbol(p, p) == 0);
        for (const uint32_t q : odd_primes) {
            if (unlikely(p == q)) {
                continue;
            }
            const int32_t j_p_q        = kronecker_symbol(p, q);
            const int32_t j_q_p        = kronecker_symbol(q, p);
            const uint32_t p12_q12_pow = ((p - 1) / 2) * ((q - 1) / 2);
            // (-1)^{ ((p - 1) / 2) * ((q - 1) / 2) }
            const int32_t to_p12_q12_pow = 1 - static_cast<int32_t>((p12_q12_pow % 2) * 2);
            assert(j_p_q * j_q_p == to_p12_q12_pow);
        }

        const int32_t j_m1_p = kronecker_symbol(-1, p);
        const int32_t j_2_p  = kronecker_symbol(2U, p);
        switch (p % 8) {
            case 1:
                assert(j_m1_p == 1);
                assert(j_2_p == 1);
                break;
            case 3:
                assert(j_m1_p == -1);
                assert(j_2_p == -1);
                break;
            case 5:
                assert(j_m1_p == 1);
                assert(j_2_p == -1);
                break;
            case 7:
                assert(j_m1_p == -1);
                assert(j_2_p == 1);
                break;
            default:
                assert(false && "Not an odd prime number");
                break;
        }

        const int32_t j_3_p = kronecker_symbol(3U, p);
        switch (p % 12) {
            case 1:
            case 11:
                assert(j_3_p == 1);
                break;
            case 5:
            case 7:
                assert(j_3_p == -1);
                break;
            default:
                break;
        }

        const int32_t j_5_p  = kronecker_symbol(5U, p);
        uint32_t j_5_p_mod_p = static_cast<uint32_t>(j_5_p);
        switch (p % 5) {
            case 1:
            case 4:
                assert(j_5_p == 1);
                break;
            case 2:
            case 3:
                assert(j_5_p == -1);
                j_5_p_mod_p = p - 1;
                break;
            case 0:
                assert(p == 5);
                assert(j_5_p == 0);
                break;
            default:
                // For the analysers
                std::abort();
        }

        if (p <= math_functions::kMaxFibNonOverflowU64) {
            if constexpr (fibonacci_num(1) == 1 && fibonacci_num(2) == 1) {
                assert(fibonacci_num(p) % p == j_5_p_mod_p);
            } else {
                assert(fibonacci_num(p - 1) % p == j_5_p_mod_p);
            }
        }

        for (uint32_t a = 0; a <= kLen; a++) {
            const int32_t j_a_p = kronecker_symbol(a, p);
            assert(j_a_p == -1 || j_a_p == 0 || j_a_p == 1);
            const uint32_t j_a_p_mod_p = j_a_p == -1 ? p - 1 : static_cast<uint32_t>(j_a_p);
            const uint32_t a_p12       = math_functions::bin_pow_mod(a, (p - 1) / 2, p);
            assert(j_a_p_mod_p == a_p12);
            for (uint32_t b = 0; b <= kLen; b++) {
                const int32_t j_b_p = kronecker_symbol(b, p);
                assert(a % p != b % p || j_a_p == j_b_p);
                const int32_t j_ab_p = kronecker_symbol(a * b, p);
                assert(j_ab_p == j_a_p * j_b_p);
            }
        }
    }
}

#if defined(HAS_GMPXX_DURING_TESTING)

void CheckJacobi(int32_t i, int32_t j, const mpz_class& n1, const mpz_class& n2) noexcept {
    const int32_t func_jac = kronecker_symbol(i, j);
    const int real_jac     = mpz_jacobi(n1.get_mpz_t(), n2.get_mpz_t());
    if (func_jac != real_jac) {
        std::printf("Error at (%" PRId32 ", %" PRId32 "): given J = %d, correct J = %d\n", i, j,
                    func_jac, real_jac);
    }
}

void CheckJacobi(int64_t i, int64_t j, const mpz_class& n1, const mpz_class& n2) noexcept {
    const int32_t func_jac = kronecker_symbol(i, j);
    const int real_jac     = mpz_jacobi(n1.get_mpz_t(), n2.get_mpz_t());
    if (func_jac != real_jac) {
        std::printf("Error at (%" PRId64 ", %" PRId64 "): given J = %d, correct J = %d\n", i, j,
                    func_jac, real_jac);
    }
}

void CheckJacobi(uint32_t i, uint32_t j, const mpz_class& n1, const mpz_class& n2) noexcept {
    const int32_t func_jac = kronecker_symbol(i, j);
    const int real_jac     = mpz_jacobi(n1.get_mpz_t(), n2.get_mpz_t());
    if (func_jac != real_jac) {
        std::printf("Error at (%" PRIu32 ", %" PRIu32 "): given J = %d, correct J = %d\n", i, j,
                    func_jac, real_jac);
    }
}

void CheckJacobi(uint64_t i, uint64_t j, const mpz_class& n1, const mpz_class& n2) noexcept {
    const int32_t func_jac = kronecker_symbol(i, j);
    const int real_jac     = mpz_jacobi(n1.get_mpz_t(), n2.get_mpz_t());
    if (func_jac != real_jac) {
        std::printf("Error at (%" PRIu64 ", %" PRIu64 "): given J = %d, correct J = %d\n", i, j,
                    func_jac, real_jac);
    }
}

void CheckJacobi(uint32_t i, int32_t j, const mpz_class& n1, const mpz_class& n2) noexcept {
    const int32_t func_jac = kronecker_symbol(i, j);
    const int real_jac     = mpz_jacobi(n1.get_mpz_t(), n2.get_mpz_t());
    if (func_jac != real_jac) {
        std::printf("Error at (%" PRIu32 ", %" PRId32 "): given J = %d, correct J = %d\n", i, j,
                    func_jac, real_jac);
    }
}

void CheckJacobi(int32_t i, uint32_t j, const mpz_class& n1, const mpz_class& n2) noexcept {
    const int32_t func_jac = kronecker_symbol(i, j);
    const int real_jac     = mpz_jacobi(n1.get_mpz_t(), n2.get_mpz_t());
    if (func_jac != real_jac) {
        std::printf("Error at (%" PRId32 ", %" PRIu32 "): given J = %d, correct J = %d\n", i, j,
                    func_jac, real_jac);
    }
}

void CheckJacobi(uint64_t i, int64_t j, const mpz_class& n1, const mpz_class& n2) noexcept {
    const int32_t func_jac = kronecker_symbol(i, j);
    const int real_jac     = mpz_jacobi(n1.get_mpz_t(), n2.get_mpz_t());
    if (func_jac != real_jac) {
        std::printf("Error at (%" PRIu64 ", %" PRId64 "): given J = %d, correct J = %d\n", i, j,
                    func_jac, real_jac);
    }
}

void CheckJacobi(int64_t i, uint64_t j, const mpz_class& n1, const mpz_class& n2) noexcept {
    const int32_t func_jac = kronecker_symbol(i, j);
    const int real_jac     = mpz_jacobi(n1.get_mpz_t(), n2.get_mpz_t());
    if (func_jac != real_jac) {
        std::printf("Error at (%" PRId64 ", %" PRIu64 "): given J = %d, correct J = %d\n", i, j,
                    func_jac, real_jac);
    }
}

template <uint16_t kLen>
void GMPCheckJacobiI32() {
    test_tools::log_tests_started();
    mpz_class n1;
    mpz_class n2;

    for (int32_t i = std::numeric_limits<int32_t>::min();
         i <= int32_t{kLen} + std::numeric_limits<int32_t>::min(); i++) {
        n1 = i;
        for (int32_t j = std::numeric_limits<int32_t>::min();
             j <= int32_t{kLen} + std::numeric_limits<int32_t>::min(); j++) {
            n2 = j;
            CheckJacobi(i, j, n1, n2);
        }
    }

    for (int32_t i = std::numeric_limits<int32_t>::max();
         i >= std::numeric_limits<int32_t>::max() - int32_t{kLen}; i--) {
        n1 = i;
        for (int32_t j = std::numeric_limits<int32_t>::max();
             j >= std::numeric_limits<int32_t>::max() - int32_t{kLen}; j--) {
            n2 = j;
            CheckJacobi(i, j, n1, n2);
        }
    }
}

template <uint16_t kLen>
void GMPCheckJacobiI64() {
    test_tools::log_tests_started();
    mpz_class n1;
    mpz_class n2;

    n1.set_str("-9223372036854775808", 10);
    for (int64_t i = std::numeric_limits<int64_t>::min();
         i <= std::numeric_limits<int64_t>::min() + int64_t{kLen}; ++i, ++n1) {
        n2.set_str("-9223372036854775808", 10);
        for (int64_t j = std::numeric_limits<int64_t>::min();
             j <= std::numeric_limits<int64_t>::min() + int64_t{kLen}; ++j, ++n2) {
            CheckJacobi(i, j, n1, n2);
        }
    }

    n1.set_str("9223372036854775807", 10);
    for (int64_t i = std::numeric_limits<int64_t>::max();
         i >= std::numeric_limits<int64_t>::max() - int64_t{kLen}; --i, --n1) {
        n2.set_str("9223372036854775807", 10);
        for (int64_t j = std::numeric_limits<int64_t>::max();
             j >= std::numeric_limits<int64_t>::max() - int64_t{kLen}; --j, --n2) {
            CheckJacobi(i, j, n1, n2);
        }
    }
}

template <uint16_t kLen>
void GMPCheckJacobiU32() {
    test_tools::log_tests_started();
    mpz_class n1;
    mpz_class n2;

    for (uint32_t i = 0; i <= uint32_t{kLen}; i++) {
        n1 = i;
        for (uint32_t j = 0; j <= uint32_t{kLen}; j++) {
            n2 = j;
            CheckJacobi(i, j, n1, n2);
        }
    }

    for (uint32_t i = std::numeric_limits<int32_t>::max();
         i >= std::numeric_limits<int32_t>::max() - uint32_t{kLen}; i--) {
        n1 = i;
        for (uint32_t j = std::numeric_limits<int32_t>::max();
             j >= std::numeric_limits<int32_t>::max() - uint32_t{kLen}; j--) {
            n2 = j;
            CheckJacobi(i, j, n1, n2);
        }
    }
}

template <uint16_t kLen>
void GMPCheckJacobiU64() {
    test_tools::log_tests_started();
    mpz_class n1;
    mpz_class n2;

    n1 = 0U;
    for (uint64_t i = 0; i <= uint64_t{kLen}; ++i, ++n1) {
        n2 = 0U;
        for (uint64_t j = 0; j <= uint64_t{kLen}; ++j, ++n2) {
            CheckJacobi(i, j, n1, n2);
        }
    }

    n1.set_str("18446744073709551615", 10);
    for (uint64_t i = std::numeric_limits<int64_t>::max();
         i >= std::numeric_limits<int64_t>::max() - uint64_t{kLen}; --i, --n1) {
        n2.set_str("18446744073709551615", 10);
        for (uint64_t j = std::numeric_limits<int64_t>::max();
             j >= std::numeric_limits<int64_t>::max() - uint64_t{kLen}; --j, --n2) {
            CheckJacobi(i, j, n1, n2);
        }
    }
}

template <uint16_t kLen>
void GMPCheckJacobiU32I32() {
    test_tools::log_tests_started();
    mpz_class n1;
    mpz_class n2;

    for (int32_t i = std::numeric_limits<int32_t>::min();
         i <= int32_t{kLen} + std::numeric_limits<int32_t>::min(); i++) {
        n1 = i;
        for (uint32_t j = 0; j <= uint32_t{kLen}; j++) {
            n2 = j;
            CheckJacobi(i, j, n1, n2);
            CheckJacobi(j, i, n2, n1);
        }
        for (uint32_t j = std::numeric_limits<int32_t>::max();
             j >= std::numeric_limits<int32_t>::max() - uint32_t{kLen}; j--) {
            n2 = j;
            CheckJacobi(i, j, n1, n2);
            CheckJacobi(j, i, n2, n1);
        }
    }

    for (int32_t i = std::numeric_limits<int32_t>::max();
         i >= std::numeric_limits<int32_t>::max() - int32_t{kLen}; i--) {
        n1 = i;
        for (uint32_t j = 0; j <= uint32_t{kLen}; j++) {
            n2 = j;
            CheckJacobi(i, j, n1, n2);
            CheckJacobi(j, i, n2, n1);
        }
        for (uint32_t j = std::numeric_limits<int32_t>::max();
             j >= std::numeric_limits<int32_t>::max() - uint32_t{kLen}; j--) {
            n2 = j;
            CheckJacobi(i, j, n1, n2);
            CheckJacobi(j, i, n2, n1);
        }
    }
}

template <uint16_t kLen>
void GMPCheckJacobiU64I64() {
    test_tools::log_tests_started();
    mpz_class n1;
    mpz_class n2;

    n1.set_str("-9223372036854775808", 10);
    for (int64_t i = std::numeric_limits<int64_t>::min();
         i <= std::numeric_limits<int64_t>::min() + int64_t{kLen}; ++i, ++n1) {
        n2 = 0U;
        for (uint64_t j = 0; j <= uint64_t{kLen}; ++j, ++n2) {
            CheckJacobi(i, j, n1, n2);
            CheckJacobi(j, i, n2, n1);
        }

        n2.set_str("18446744073709551615", 10);
        for (uint64_t j = std::numeric_limits<int64_t>::max();
             j >= std::numeric_limits<int64_t>::max() - uint64_t{kLen}; --j, --n2) {
            CheckJacobi(i, j, n1, n2);
            CheckJacobi(j, i, n2, n1);
        }
    }

    n1.set_str("9223372036854775807", 10);
    for (int64_t i = std::numeric_limits<int64_t>::max();
         i >= std::numeric_limits<int64_t>::max() - int64_t{kLen}; --i, --n1) {
        n2 = 0U;
        for (uint64_t j = 0; j <= uint64_t{kLen}; ++j, ++n2) {
            CheckJacobi(i, j, n1, n2);
            CheckJacobi(j, i, n2, n1);
        }

        n2.set_str("18446744073709551615", 10);
        for (uint64_t j = std::numeric_limits<int64_t>::max();
             j >= std::numeric_limits<int64_t>::max() - uint64_t{kLen}; --j, --n2) {
            CheckJacobi(i, j, n1, n2);
            CheckJacobi(j, i, n2, n1);
        }
    }
}

#endif

}  // namespace

// clang-format off
// NOLINTEND(cert-dcl03-c, misc-static-assert, hicpp-static-assert, cppcoreguidelines-avoid-magic-numbers)
// clang-format on

int main() {
    constexpr uint16_t kLen = 2000;
    CheckJacobiBasic<kLen>();
#if defined(HAS_GMPXX_DURING_TESTING)
    GMPCheckJacobiI32<kLen>();
    GMPCheckJacobiI64<kLen>();
    GMPCheckJacobiU32<kLen>();
    GMPCheckJacobiU64<kLen>();
    GMPCheckJacobiU32I32<kLen>();
    GMPCheckJacobiU64I64<kLen>();
#endif
}
