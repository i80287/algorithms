#pragma once

#include <cstdint>
#include <functional>
#include <limits>
#include <type_traits>
#include <utility>

#include "config_macros.hpp"
#include "math_functions.hpp"

namespace math_functions {

struct LoopDetectResult {
    std::uint32_t cycle_start_lower_bound;
    std::uint32_t cycle_start_upper_bound;
    std::uint32_t cycle_period;
};

template <class Function>
#if CONFIG_HAS_AT_LEAST_CXX_20
    requires std::is_invocable_r_v<std::int32_t, Function, std::int32_t>
#endif
[[nodiscard]]
constexpr LoopDetectResult loop_detection_Gosper(Function f, std::int32_t x0) noexcept(
    std::is_nothrow_invocable_r_v<std::int32_t, Function, std::int32_t>) {
    /**
     * See Hackers Delight 5-5.
     */
    // clang-format off
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays, cppcoreguidelines-avoid-magic-numbers)
    std::int32_t f_values[33]
#if CONFIG_HAS_AT_LEAST_CXX_20
        ;
#else
        = {};
#endif
    // clang-format on

    f_values[0]     = x0;
    std::int32_t xn = x0;
    for (std::uint32_t n = 1;;) {
        xn                       = std::invoke(f, std::int32_t{xn});
        const std::uint32_t kmax = ::math_functions::log2_floor(n);
        for (std::uint32_t k = 0; k <= kmax; k++) {
            if (unlikely(xn == f_values[k])) {
                /**
                 * let ctz be a function for counting
                 * trailing zeros in the binary notation
                 *
                 * Compute m = max{i | i < n and ctz(i + 1) = k}
                 * j' := r * 2^k, r = n >> k
                 * r' := (r - 1) | 1
                 * r' = r - 1 if r is even and r otherwise
                 * j := r' << k, ctz(j) == k
                 * m = j - 1
                 */
                const std::uint32_t m = ((((n >> k) - 1U) | 1U) << k) - 1;
                CONFIG_ASSUME_STATEMENT(m < n);
                const std::uint32_t lambda = n - m;
                CONFIG_ASSUME_STATEMENT(lambda >= 1);
                const auto mu_upper = m;
                const auto gap      = std::max(1U, lambda - 1) - 1;
                const auto mu_lower = mu_upper >= gap ? mu_upper - gap : 0;
                CONFIG_ASSUME_STATEMENT(mu_lower <= mu_upper);
                return {mu_lower, mu_upper, lambda};
            }
        }

        if (unlikely(++n == 0)) {
            break;
        }
        f_values[static_cast<std::uint32_t>(::math_functions::countr_zero(n))] = xn;  // No match.
    }

    std::terminate();
    return {};
}

}  // namespace math_functions
