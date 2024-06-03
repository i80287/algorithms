#pragma once

#include <cassert>
#include <cmath>
#include <complex>
#include <utility>
#include <vector>

#if defined(__cpp_lib_math_constants) && __cpp_lib_math_constants >= 201907L
#include <numbers>
#endif

#include "config_macros.hpp"

namespace fft {

using f64     = double;
using complex = std::complex<f64>;

namespace fft_detail {

/*
 * Save only e^{2pi*0/1}, e^{2pi*0/2}, e^{2pi*0/4}, e^{2pi*1/4}, e^{2pi*0/8},
 * e^{2pi*1/8}, e^{2pi*2/8}, e^{2pi*3/8}, ... because only low n / 2 roots are
 * used (fft_roots[0] never used btw, because in fft step >= 1, so it can be
 * anything)
 */
static std::vector<complex> fft_roots = {complex(0, 0), complex(1, 0)};

template <bool IsBackwardFFT = false /* Forward of backward FFT */>
#if CONFIG_HAS_AT_LEAST_CXX_20 && !defined(_GLIBCXX_DEBUG)
constexpr
#endif
    inline void
    forward_or_backward_fft(complex* p, const size_t k) noexcept {
    ATTRIBUTE_ASSUME(k > 0 && (k & (k - 1)) == 0);

    for (size_t i = 1, k_reversed_i = 0; i < k; i++) {
        // 'Increase' k_reversed_i by one
        size_t bit = k >> 1;
        for (; k_reversed_i >= bit; bit >>= 1) {
            k_reversed_i -= bit;
        }

        k_reversed_i += bit;
        if (i < k_reversed_i) {
            std::swap(p[i], p[k_reversed_i]);
        }
    }

    const complex* const points = fft_roots.data();

    // Unrolled loop for step = 1
    for (size_t block_start = 0; block_start < k; block_start += 2) {
        const complex p0_i = p[block_start];
        complex w_j_p1_i   = p[block_start + 1];
        p[block_start]     = p0_i + w_j_p1_i;
        p[block_start + 1] = p0_i - w_j_p1_i;
    }

    for (size_t step = 2; step < k; step *= 2) {
        for (size_t block_start = 0; block_start < k;) {
            size_t block_end = block_start + step;
            for (size_t pos_in_block = block_start, point_index = step; pos_in_block < block_end;
                 pos_in_block++, point_index++) {
                const complex p0_i = p[pos_in_block];
                complex w_j_p1_i{};
                if constexpr (IsBackwardFFT) {
                    w_j_p1_i = std::conj(points[point_index]) * p[pos_in_block + step];
                } else {
                    w_j_p1_i = points[point_index] * p[pos_in_block + step];
                }
                p[pos_in_block]        = p0_i + w_j_p1_i;
                p[pos_in_block + step] = p0_i - w_j_p1_i;
            }

            block_start = block_end + step;
        }
    }

    if constexpr (IsBackwardFFT) {
        f64 one_kth = 1.0 / f64(k);
        for (complex *p_iter = p, *p_end = p_iter + k; p_iter != p_end; ++p_iter) {
            *p_iter *= one_kth;
        }
    }
}

};  // namespace fft_detail

inline void ensure_roots_capacity(const size_t n) {
    assert((n & (n - 1)) == 0);
    ATTRIBUTE_ASSUME((n & (n - 1)) == 0);

    using fft_detail::fft_roots;

    size_t current_len = fft_roots.size();
    assert(current_len >= 2 && (current_len & (current_len - 1)) == 0);
    ATTRIBUTE_ASSUME(current_len >= 2 && (current_len & (current_len - 1)) == 0);
    if (current_len >= n) {
        return;
    }

    assert(n >= 4);
    ATTRIBUTE_ASSUME(n >= 4);

    fft_roots.reserve(n);
#if defined(__cpp_lib_math_constants) && __cpp_lib_math_constants >= 201907L
    constexpr f64 kPi = std::numbers::pi_v<f64>;
#else
    const f64 kPi = std::acos(f64(-1));
#endif

    auto add_point = [&](size_t i) noexcept {
        assert(fft_roots.size() < fft_roots.capacity());
        ATTRIBUTE_ASSUME(fft_roots.size() < fft_roots.capacity());
        fft_roots.emplace_back(fft_roots[i]);
        // double phi = 2 * kPi * (2 * i - current_len + 1) / (2 * current_len);
        f64 phi = kPi * f64(2 * i - current_len + 1) / f64(current_len);
        assert(fft_roots.size() < fft_roots.capacity());
        ATTRIBUTE_ASSUME(fft_roots.size() < fft_roots.capacity());
        fft_roots.emplace_back(std::cos(phi), std::sin(phi));
    };
    do {
        for (size_t i = current_len / 2; i != current_len; i++) {
            add_point(i);
        }
        current_len *= 2;
    } while (current_len < n);
    assert(fft_roots.size() == current_len);
    assert(current_len == n);
}

inline void forward_backward_fft(complex* p1, complex* p2, const size_t n) {
    assert(n > 0 && (n & (n - 1)) == 0);
    ATTRIBUTE_ASSUME(n > 0 && (n & (n - 1)) == 0);

    ensure_roots_capacity(n);
    fft_detail::forward_or_backward_fft</*IsBackwardFFT = */ false>(p1, n);
    /*
     * A(w^j) = a_0 + a_1 * w^j + a_2 * w^{2 j} + ... + a_{n - 1} * w^{(n - 1)
     * j} B(w^j) = b_0 + b_1 * w^j + b_2 * w^{2 j} + ... + b_{n - 1} * w^{(n -
     * 1) j}
     *
     * P = A + B * i = [ A(w^0) + B(w^0) * i, A(w^1) + B(w^1) * i, ... A(w^(n -
     * 1)) + B(w^(n - 1)) * i ]
     *
     * P(w^j) + conj(P(w^{n - j})) =
     * = A(w^j) + B(w^j) * i + conj(A(w^{n - j}) + B(w^{n - j}) * i) =
     * = \sum{k=0}{n-1} (a_k + b_k * i) * w^{jk} + \sum{k=0}{n-1} conj((a_k +
     * b_k * i) * w^{(n - j)k}) = = \sum{k=0}{n-1} (a_k + b_k * i) * w^{jk} +
     * \sum{k=0}{n-1} conj((a_k + b_k * i) * w^{-jk}) = = \sum{k=0}{n-1} (a_k +
     * b_k * i) * w^{jk} + conj((a_k + b_k * i) * w^{-jk}) = = \sum{k=0}{n-1}
     * (a_k + b_k * i) * w^{jk} + conj(a_k + b_k * i) * conj(w^{-jk}) = =
     * \sum{k=0}{n-1} (a_k + b_k * i) * w^{jk} + (a_k - b_k * i) * w^{jk} = =
     * \sum{k=0}{n-1} 2 a_k * w^{jk} = 2 * A(w^j)
     *
     * \implies A(w^j) = (P(w^j) + conj(P(w^{n - j}))) / 2
     *
     * By analogy it is can be proved that A(w^j) = (P(w^j) - conj(P(w^{n -
     * j}))) / (2 * i)
     *
     * C(w^j) = A(w^j) * B(w^j) \implies C(w^j) =
     * = (P(w^j) + conj(P(w^{n - j}))) / 2 * (P(w^j) - conj(P(w^{n - j}))) / (2
     * * i) = = (P(w^j) + conj(P(w^{n - j}))) * (P(w^j) - conj(P(w^{n - j}))) /
     * (4 * i) = = (P(w^j) + conj(P(w^{n - j}))) * (P(w^j) - conj(P(w^{n - j})))
     * / (4 * i) =
     */
    constexpr complex one_quat_i = complex(0, -0.25);  // 1 / (4 * i) == -i / 4
    for (size_t j = 0; j < n; j++) {
        size_t n_j      = (n - j) & (n - 1);  // <=> mod n because n is power of two
        complex p_w_j   = p1[j];
        complex p_w_n_j = std::conj(p1[n_j]);
        p2[j]           = (p_w_j + p_w_n_j) * (p_w_j - p_w_n_j) * one_quat_i;
    }
    fft_detail::forward_or_backward_fft</*IsBackwardFFT = */ true>(p2, n);
}

}  // namespace fft
