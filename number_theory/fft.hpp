#pragma once

#include <cassert>
#include <cmath>
#include <complex>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

#include "../misc/assert.hpp"
#include "../misc/config_macros.hpp"

#if defined(__cpp_lib_math_constants) && __cpp_lib_math_constants >= 201907L && \
    CONFIG_HAS_INCLUDE(<numbers>)
#include <numbers>
#define FFT_HAS_NUMBERS
#endif

#if defined(__cpp_lib_span) && __cpp_lib_span >= 202002L && CONFIG_HAS_INCLUDE(<span>)
#include <span>
#define FFT_HAS_SPAN
#endif

namespace fft {

using f64 = double;
using complex = std::complex<f64>;

using std::size_t;

/// @brief Multiply polynomials @a p1 and @a p2 of size @a n
///         and store their product into @a p2
/// @param p1
/// @param p2
/// @param n
ATTRIBUTE_SIZED_ACCESS(read_write, 1, 3)
ATTRIBUTE_SIZED_ACCESS(read_write, 2, 3)
ATTRIBUTE_NONNULL_ALL_ARGS
inline void forward_backward_fft(complex* RESTRICT_QUALIFIER p1,
                                 complex* RESTRICT_QUALIFIER p2,
                                 size_t n) ATTRIBUTE_ALLOCATING_FUNCTION;

#ifdef FFT_HAS_SPAN

/// @brief See forward_backward_fft(complex*, complex*, size_t)
///         @a poly1 and @a poly2 should be of the same size
/// @note  @a poly1 and @a poly2 may point to the same array, but in this case
///         new memory will be allocated for the internal computation
/// @throws std::runtime_error if @a poly1.size() != @a poly2.size()
///         std::bad_alloc by the operator new if allocation needed for
///         the internal computations failed
/// @param poly1
/// @param poly2
inline void forward_backward_fft(std::span<complex> poly1,
                                 std::span<complex> poly2) ATTRIBUTE_ALLOCATING_FUNCTION;

#endif

namespace detail {

struct private_impl final {
private:
    /*
     * Save only e^{2pi*0/1}, e^{2pi*0/2}, e^{2pi*0/4}, e^{2pi*1/4}, e^{2pi*0/8},
     * e^{2pi*1/8}, e^{2pi*2/8}, e^{2pi*3/8}, ... because only low n / 2 roots are
     * used
     * (fft_roots[0] is never used btw, because in fft step >= 1,
     *  so fft_roots[0] can be initialized with anything)
     */
    static inline std::vector<complex> fft_roots = {
        complex{0, 0},
        complex{1, 0},
    };

    ATTRIBUTE_ALWAYS_INLINE ATTRIBUTE_CONST [[nodiscard]]
    static constexpr bool is_valid_polynomial_size(const size_t n) noexcept
        ATTRIBUTE_NONBLOCKING_FUNCTION {
        return n > 0 && (n & (n - 1)) == 0;
    }

    template <typename T>
    ATTRIBUTE_CONST [[nodiscard]]
    static bool are_distinct_non_empty_ranges(const T* const array_1_begin,
                                              const T* const array_2_begin,
                                              const size_t n) noexcept {
        const auto array_1_begin_int = reinterpret_cast<uintptr_t>(array_1_begin);
        const auto array_2_begin_int = reinterpret_cast<uintptr_t>(array_2_begin);
        const auto array_1_end_int = array_1_begin_int + n * sizeof(T);
        const auto array_2_end_int = array_2_begin_int + n * sizeof(T);

        const bool range_1_is_valid = array_1_begin_int != 0 && array_1_begin_int < array_1_end_int;
        const bool range_2_is_valid = array_2_begin_int != 0 && array_2_begin_int < array_2_end_int;

        return range_1_is_valid && range_2_is_valid &&
               ((array_1_end_int <= array_2_begin_int) ^ (array_2_end_int <= array_1_begin_int));
    }

    template <bool IsBackwardFFT = false /* Forward of backward FFT */>
    ATTRIBUTE_SIZED_ACCESS(read_write, 1, 2)
    ATTRIBUTE_NONNULL(1) static void forward_or_backward_fft(
        complex* const p, const size_t k) noexcept ATTRIBUTE_NONBLOCKING_FUNCTION {
        CONFIG_ASSUME_STATEMENT(is_valid_polynomial_size(k));

        for (std::size_t i = 1, k_reversed_i = 0; i < k; i++) {
            // 'Increase' k_reversed_i by one
            std::size_t bit = k >> 1U;
            for (; k_reversed_i >= bit; bit >>= 1U) {
                k_reversed_i -= bit;
            }

            k_reversed_i += bit;
            if (i < k_reversed_i) {
                std::swap(p[i], p[k_reversed_i]);
            }
        }

        const complex* const points = fft_roots.data();

        // Unrolled loop for step = 1
        for (std::size_t block_start = 0; block_start < k; block_start += 2) {
            const complex p0_i = p[block_start];
            const complex w_j_p1_i = p[block_start + 1];
            p[block_start] = p0_i + w_j_p1_i;
            p[block_start + 1] = p0_i - w_j_p1_i;
        }

        for (std::size_t step = 2; step < k; step *= 2) {
            for (std::size_t block_start = 0; block_start < k;) {
                const std::size_t block_end = block_start + step;
                for (std::size_t pos_in_block = block_start, point_index = step;
                     pos_in_block < block_end; pos_in_block++, point_index++) {
                    const complex p0_i = p[pos_in_block];
                    complex w_j_p1_i{};
                    if constexpr (IsBackwardFFT) {
                        w_j_p1_i = std::conj(points[point_index]) * p[pos_in_block + step];
                    } else {
                        w_j_p1_i = points[point_index] * p[pos_in_block + step];
                    }
                    p[pos_in_block] = p0_i + w_j_p1_i;
                    p[pos_in_block + step] = p0_i - w_j_p1_i;
                }

                block_start = block_end + step;
            }
        }

        if constexpr (IsBackwardFFT) {
            const f64 one_kth = 1.0 / static_cast<f64>(k);
            for (complex *p_iter = p, *p_end = p_iter + k; p_iter != p_end; ++p_iter) {
                *p_iter *= one_kth;
            }
        }
    }

    static void ensure_roots_capacity(const size_t n) ATTRIBUTE_ALLOCATING_FUNCTION {
        ensure_roots_capacity_impl(n, fft_roots);
    }

    static void ensure_roots_capacity_impl(const size_t n, std::vector<complex>& roots)
        ATTRIBUTE_ALLOCATING_FUNCTION {
        CONFIG_ASSUME_STATEMENT(is_valid_polynomial_size(n));

        size_t current_len = roots.size();
        assert(current_len >= 2 && (current_len & (current_len - 1)) == 0);
        CONFIG_ASSUME_STATEMENT(current_len >= 2 && (current_len & (current_len - 1)) == 0);
        if (current_len >= n) {
            return;
        }

        roots.reserve(n);
        const auto add_point = [roots_data = roots.data(), &current_len,
                                &roots](const size_t i) noexcept {
#ifdef FFT_HAS_NUMBERS
            constexpr f64 kPi = std::numbers::pi_v<f64>;
#else
            const f64 kPi = std::acos(static_cast<f64>(-1));
#endif
            roots.emplace_back(roots_data[i]);
            // double phi = 2 * kPi * (2 * i - current_len + 1) / (2 * current_len);
            const f64 phi =
                kPi * static_cast<f64>(2 * i - current_len + 1) / static_cast<f64>(current_len);
            roots.emplace_back(std::cos(phi), std::sin(phi));
        };
        do {
            for (size_t i = current_len / 2; i != current_len; i++) {
                add_point(i);
            }
            current_len *= 2;
            assert(current_len == roots.size());
        } while (current_len < n);
        assert(current_len == n);
    }

    friend inline void fft::forward_backward_fft(complex* RESTRICT_QUALIFIER p1,
                                                 complex* RESTRICT_QUALIFIER p2,
                                                 size_t n) ATTRIBUTE_ALLOCATING_FUNCTION;

#ifdef FFT_HAS_SPAN
    friend inline void fft::forward_backward_fft(std::span<complex> poly1, std::span<complex> poly2)
        ATTRIBUTE_ALLOCATING_FUNCTION;
#endif
};

}  // namespace detail

inline void forward_backward_fft(complex* const RESTRICT_QUALIFIER p1,
                                 complex* const RESTRICT_QUALIFIER p2,
                                 const size_t n) ATTRIBUTE_ALLOCATING_FUNCTION {
    if (unlikely(n == 0)) {
        return;
    }

    THROW_IF_NOT(fft::detail::private_impl::is_valid_polynomial_size(n));
    THROW_IF_NOT(fft::detail::private_impl::are_distinct_non_empty_ranges(p1, p2, n));

    fft::detail::private_impl::ensure_roots_capacity(n);
    fft::detail::private_impl::forward_or_backward_fft</*IsBackwardFFT = */ false>(p1, n);

    // clang-format off
    /*
     * A(w^j) = a_0 + a_1 * w^j + a_2 * w^{2 j} + ... + a_{n - 1} * w^{(n - 1) j}
     * B(w^j) = b_0 + b_1 * w^j + b_2 * w^{2 j} + ... + b_{n - 1} * w^{(n - 1) j}
     *
     * P = A + B * i = [ A(w^0) + B(w^0) * i, A(w^1) + B(w^1) * i, ... A(w^(n - 1)) + B(w^(n - 1)) * i ]
     *
     * P(w^j) + conj(P(w^{n - j})) =
     * = A(w^j) + B(w^j) * i + conj(A(w^{n - j}) + B(w^{n - j}) * i) =
     * = \sum_{k=0}^{n-1} (a_k + b_k * i) * w^{j k} + \sum_{k=0}^{n-1} conj((a_k + b_k * i) * w^{(n - j) k}) =
     * = \sum_{k=0}^{n-1} (a_k + b_k * i) * w^{j k} + \sum_{k=0}^{n-1} conj((a_k + b_k * i) * w^{-j k}) =
     * = \sum_{k=0}^{n-1} (a_k + b_k * i) * w^{j k} + conj((a_k + b_k * i) * w^{-j k}) =
     * = \sum_{k=0}^{n-1} (a_k + b_k * i) * w^{j k} + conj(a_k + b_k * i) * conj(w^{-j k}) =
     * = \sum_{k=0}^{n-1} (a_k + b_k * i) * w^{j k} + (a_k - b_k * i) * w^{j k} =
     * = \sum_{k=0}^{n-1} 2 a_k * w^{j k} = 2 * A(w^j)
     *
     * \implies A(w^j) = (P(w^j) + conj(P(w^{n - j}))) / 2
     *
     * By analogy it is can be proved that
     * B(w^j) = (P(w^j) - conj(P(w^{n - j}))) / (2 * i)
     *
     * C(w^j) = A(w^j) * B(w^j) \implies C(w^j) =
     * = (P(w^j) + conj(P(w^{n - j}))) / 2 * (P(w^j) - conj(P(w^{n - j}))) / (2 * i) =
     * = (P(w^j) + conj(P(w^{n - j}))) * (P(w^j) - conj(P(w^{n - j}))) / (4 * i)
     */
    // clang-format on

    constexpr complex one_over_four_i(f64{0}, f64{-0.25});  // 1 / (4 * i) == -i / 4
    for (std::size_t j = 0; j < n; j++) {
        const std::size_t n_j = (n - j) & (n - 1);  // <=> mod n because n is power of two
        const complex p_w_j = p1[j];
        const complex p_w_n_j = std::conj(p1[n_j]);
        p2[j] = (p_w_j + p_w_n_j) * (p_w_j - p_w_n_j) * one_over_four_i;
    }
    fft::detail::private_impl::forward_or_backward_fft</*IsBackwardFFT = */ true>(p2, n);
}

#ifdef FFT_HAS_SPAN

inline void forward_backward_fft(const std::span<complex> poly1,
                                 const std::span<complex> poly2) ATTRIBUTE_ALLOCATING_FUNCTION {
    THROW_IF(poly1.size() != poly2.size());

    if (unlikely(poly1.empty())) {
        return;
    }

    const bool allocate_memory = !fft::detail::private_impl::are_distinct_non_empty_ranges(
        poly1.data(), poly2.data(), poly2.size());

    using TemporaryStorage = std::vector<complex>;
    TemporaryStorage poly1_storage = [&]() {
        if (allocate_memory) {
            return TemporaryStorage(poly1.size());
        } else {
            return TemporaryStorage();
        }
    }();

    forward_backward_fft(allocate_memory ? poly1_storage.data() : poly1.data(), poly2.data(),
                         poly1.size());
}

#endif

}  // namespace fft

#undef FFT_HAS_SPAN
#undef FFT_HAS_NUMBERS
