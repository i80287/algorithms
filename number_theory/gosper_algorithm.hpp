#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>

#include "math_functions.hpp"

namespace math_functions {

struct loop_detect_result {
    int32_t mu_lower;
    uint32_t mu_upper;
    uint32_t lambda;
};

template <class Function>
#if __cplusplus >= 202002L
    requires requires(Function f) {
        { f(int32_t()) } -> std::same_as<int32_t>;
    }
#endif
static constexpr loop_detect_result loop_detection_Gosper(Function f, int32_t x0) noexcept {
    /**
     * See Hackers Delight 5-5.
     */
    int32_t f_values[33]
#if __cplusplus < 202002L
        = {};
#else
        ;
#endif
    f_values[0] = x0;
    int32_t xn  = x0;
    for (uint32_t n = 1;; n++) {
        xn            = f(xn);
        uint32_t kmax = log2_floor(n);
        for (uint32_t k = 0; k <= kmax; k++) {
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
                uint32_t m = ((((n >> k) - 1) | 1) << k) - 1;
                ATTRIBUTE_ASSUME(m < n);
                uint32_t lambda = n - m;
                ATTRIBUTE_ASSUME(lambda >= 1);
                uint32_t mu_upper = m;
                int32_t mu_lower  = int32_t(m - std::max(1u, lambda - 1) + 1);
                return {mu_lower, mu_upper, lambda};
            }
        }
        f_values[uint32_t(countr_zero(n + 1))] = xn;  // No match.
    }
}

}  // namespace math_functions
