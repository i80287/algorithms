#include <algorithm>
#include <atomic>
#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>
#include <list>
#include <numeric>
#include <random>
#include <set>
#include <thread>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include "config_macros.hpp"
#include "math_functions.hpp"
#include "test_tools.hpp"

#if CONFIG_HAS_INCLUDE(<mpfr.h>)
#include <mpfr.h>
#define HAS_MPFR_DURING_TESTING 1
#else
#define HAS_MPFR_DURING_TESTING 0
#if defined(__linux__) && !defined(__MINGW32__)
#error "mpfr tests should be available on linux"
#endif
#endif

using namespace math_functions;
using namespace test_tools;
using std::gcd;
using std::size_t;
using std::uint32_t;
using std::uint64_t;

namespace {

void test_isqrt() {
    log_tests_started();

    constexpr uint32_t kIters = 2'000'000;

    constexpr auto test_sqrts = [](uint32_t n, uint32_t n_squared) {
        assert(n == isqrt(n_squared));
        assert(n == isqrt(uint64_t{n_squared}));
        assert(n == isqrt(uint128_t{n_squared}));
    };

    constexpr uint32_t kProbes = 100'000;
    for (uint32_t i = 0; i <= std::min(kProbes, 65535u); i++) {
        uint32_t i_square      = i * i;
        uint32_t next_i_square = (i + 1) * (i + 1);
        for (uint32_t j = i_square; j != next_i_square; j++) {
            test_sqrts(i, j);
        }
    }

    for (uint32_t r = std::numeric_limits<uint32_t>::max() - kIters; r != 0; r++) {
        uint64_t rs = uint64_t(r) * r;
        assert(r - 1 == isqrt(rs - 1));
        assert(r == isqrt(rs));
        assert(r == isqrt(rs + 1));
        assert(r - 1 == isqrt(uint128_t{rs - 1}));
        assert(r == isqrt(uint128_t{rs}));
        assert(r == isqrt(uint128_t{rs + 1}));
    }

    for (uint64_t r = std::numeric_limits<uint64_t>::max() - kIters; r != 0; r++) {
        uint128_t rs = uint128_t{r} * r;
        assert(r - 1 == isqrt(uint128_t{rs - 1}));
        assert(r == isqrt(rs));
        assert(r == isqrt(rs + 1));
    }

    constexpr std::pair<uint64_t, uint128_t> root_with_square[] = {
        {uint64_t{std::numeric_limits<uint8_t>::max()},
         uint128_t{std::numeric_limits<uint16_t>::max()}},
        {uint64_t{std::numeric_limits<uint16_t>::max()},
         uint128_t{std::numeric_limits<uint32_t>::max()}},
        {uint64_t{std::numeric_limits<uint32_t>::max()},
         uint128_t{std::numeric_limits<uint64_t>::max()}},
        {uint64_t{8347849ull}, uint128_t{8347849ull} * 8347849ull},
        {uint64_t{23896778463ull}, uint128_t{23896778463ull} * 23896778463ull},
        {uint64_t{26900711288786ull},
         uint128_t{72364826784263874ull} * 10'000'000'000ull + 2'638'723'478},
        {uint64_t{3748237487274238478ull},
         uint128_t{3748237487274238478ull} * 3748237487274238478ull},
        {uint64_t{9472294799293ull}, uint128_t{8972436876473126137ull} * 10'000'000ull + 7'236'478},
        {uint64_t{18015752134763552034ull},
         (uint128_t{17594829943123320651ull} << 64) | 2622055845271657274ull},
        {std::numeric_limits<uint64_t>::max(), static_cast<uint128_t>(-1)},
    };
    for (const auto& [root, square] : root_with_square) {
        if (static_cast<uint32_t>(square) == square) {
            assert(root == isqrt(static_cast<uint32_t>(square)));
        }
        if (static_cast<uint64_t>(square) == square) {
            assert(root == isqrt(static_cast<uint64_t>(square)));
        }
        assert(root == isqrt(square));
    }
}

void test_icbrt() noexcept {
    log_tests_started();

    for (uint32_t n = 1; n < 1625; n++) {
        const uint32_t tr               = n * n * n;
        const uint32_t n_cube_minus_one = tr + 3 * n * n + 3 * n;
        assert(icbrt(tr) == n);
        assert(icbrt(uint64_t{tr}) == n);
        assert(icbrt(n_cube_minus_one) == n);
        assert(icbrt(uint64_t{n_cube_minus_one}) == n);
        assert(icbrt(n_cube_minus_one + 1) == n + 1);
        assert(icbrt(uint64_t{n_cube_minus_one + 1}) == n + 1);
    }
    assert(icbrt(1625u * 1625u * 1625u) == 1625);
    assert(icbrt(std::numeric_limits<uint32_t>::max()) == 1625);

    for (uint64_t n = 1625; n < 2642245; n++) {
        const uint64_t tr               = n * n * n;
        const uint64_t n_cube_minus_one = tr + 3 * n * n + 3 * n;
        assert(icbrt(tr) == n);
        assert(icbrt(n_cube_minus_one) == n);
        assert(icbrt(n_cube_minus_one + 1) == n + 1);
    }
    assert(icbrt(uint64_t{2642245} * 2642245 * 2642245) == 2642245);
    assert(icbrt(std::numeric_limits<uint64_t>::max()) == 2642245);
}

void test_log2() noexcept {
    log_tests_started();

    for (uint32_t k = 0; k < sizeof(uint32_t) * CHAR_BIT; k++) {
        uint32_t pw = uint32_t{1} << k;
        assert(log2_floor(pw) == k);
        assert(log2_ceil(pw) == k);
        if (!is_power_of_two(pw + 1)) {
            assert(log2_floor(pw + 1) == k);
            assert(log2_ceil(pw + 1) == k + 1);
        }
    }

    for (uint32_t k = 0; k < sizeof(uint64_t) * CHAR_BIT; k++) {
        uint64_t pw = uint64_t{1} << k;
        assert(log2_floor(pw) == k);
        assert(log2_ceil(pw) == k);
        if (!is_power_of_two(pw + 1)) {
            assert(log2_floor(pw + 1) == k);
            assert(log2_ceil(pw + 1) == k + 1);
        }
    }

    for (uint32_t k = 0; k < sizeof(uint128_t) * CHAR_BIT; k++) {
        uint128_t pw = uint128_t{1} << k;
        assert(log2_floor(pw) == k);
        assert(log2_ceil(pw) == k);
        if (!is_power_of_two(pw + 1)) {
            assert(log2_floor(pw + 1) == k);
            assert(log2_ceil(pw + 1) == k + 1);
        }
    }

    assert(log2_floor(uint32_t{0}) == static_cast<uint32_t>(-1));
    assert(log2_ceil(uint32_t{0}) == static_cast<uint32_t>(-1));
    assert(log2_floor(uint64_t{0}) == static_cast<uint32_t>(-1));
    assert(log2_ceil(uint64_t{0}) == static_cast<uint32_t>(-1));
    assert(log2_floor(uint128_t{0}) == static_cast<uint32_t>(-1));
    assert(log2_ceil(uint128_t{0}) == static_cast<uint32_t>(-1));
}

void test_bit_reverse() noexcept {
    log_tests_started();

    for (uint32_t n = 0; n < 256; n++) {
        assert(bit_reverse(static_cast<uint8_t>(n)) == (bit_reverse(n) >> 24));
    }

    constexpr uint32_t shifts[32] = {
        2,  3,  5,  7,  11, 13, 17, 19, 23, 29,  31,  37,  41,  43,  47,  53,
        59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131,
    };
    uint128_t n = std::numeric_limits<uint64_t>::max();
    for (auto k = static_cast<uint32_t>(1e7); k > 0; k--) {
        const auto hi_64  = static_cast<uint64_t>(n >> 64);
        const auto low_64 = static_cast<uint64_t>(n);
        uint128_t b       = (uint128_t{bit_reverse(low_64)} << 64) | bit_reverse(hi_64);
        assert(bit_reverse(n) == b);
        n += shifts[k % 32];
    }
}

#if defined(HAS_MPFR_DURING_TESTING) && HAS_MPFR_DURING_TESTING

const mpfr_rnd_t kRoundMode = mpfr_get_default_rounding_mode();

template <class FloatType>
SumSinCos<FloatType> call_sum_of_sines_and_cosines(mpfr_t alpha, mpfr_t beta, uint32_t n) noexcept {
    if constexpr (std::is_same_v<FloatType, float>) {
        return sum_of_sines_and_cosines(mpfr_get_flt(alpha, kRoundMode),
                                        mpfr_get_flt(beta, kRoundMode), n);
    } else if constexpr (std::is_same_v<FloatType, double>) {
        return sum_of_sines_and_cosines(mpfr_get_d(alpha, kRoundMode), mpfr_get_d(beta, kRoundMode),
                                        n);
    } else {
        return sum_of_sines_and_cosines(mpfr_get_ld(alpha, kRoundMode),
                                        mpfr_get_ld(beta, kRoundMode), n);
    }
}

template <class FloatType>
std::pair<bool, bool> check_sums_correctness(mpfr_t c_sines_sum, FloatType sines_sum,
                                             mpfr_t c_cosines_sum, FloatType cosines_sum,
                                             FloatType eps) noexcept {
    auto cmp_lambda = [](mpfr_t c_sum, FloatType sum, FloatType lambda_eps) noexcept {
        if constexpr (std::is_same_v<FloatType, float> || std::is_same_v<FloatType, double>) {
            mpfr_sub_d(c_sum, c_sum, static_cast<double>(sum), kRoundMode);
            mpfr_abs(c_sum, c_sum, kRoundMode);
            return mpfr_cmp_d(c_sum, static_cast<double>(lambda_eps)) <= 0;
        } else {
            const long double upper_bound = sum + lambda_eps;
            const long double lower_bound = sum - lambda_eps;
            return 0 <= mpfr_cmp_ld(c_sum, lower_bound) && mpfr_cmp_ld(c_sum, upper_bound) <= 0;
        }
    };

    return {
        cmp_lambda(c_sines_sum, sines_sum, eps),
        cmp_lambda(c_cosines_sum, cosines_sum, eps),
    };
}

template <class FloatType>
void test_sin_cos_sum_generic() noexcept {
    log_tests_started();

    constexpr uint32_t kMaxN       = 1e2;
    constexpr int32_t k            = 5;
    constexpr uint32_t angle_scale = 10;
    constexpr double angle_start   = bin_pow(double(angle_scale), -ptrdiff_t{k});

    constexpr FloatType kSumEps = []() constexpr noexcept -> FloatType {
        if constexpr (std::is_same_v<FloatType, float>) {
            return 0.4f;
        } else if constexpr (std::is_same_v<FloatType, double>) {
            return 0.0000001;
        } else {
            return 0.0000001L;
        }
    }();

    mpfr_t alpha;
    mpfr_t beta;
    mpfr_t angle;
    mpfr_t c_sines_sum;
    mpfr_t c_cosines_sum;
    mpfr_t angle_sin;
    mpfr_t angle_cos;
    mpfr_init(alpha);
    mpfr_init(beta);
    mpfr_init(angle);
    mpfr_init(c_sines_sum);
    mpfr_init(c_cosines_sum);
    mpfr_init(angle_sin);
    mpfr_init(angle_cos);
    for (uint32_t n = 0; n < kMaxN; n++) {
        mpfr_set_d(alpha, angle_start, kRoundMode);
        for (int32_t alpha_power = -k; alpha_power <= k; alpha_power++) {
            mpfr_set_d(beta, angle_start, kRoundMode);
            for (int32_t beta_power = -k; beta_power <= k; beta_power++) {
                const auto [sines_sum, cosines_sum] =
                    call_sum_of_sines_and_cosines<FloatType>(alpha, beta, n);
#if CONFIG_HAS_AT_LEAST_CXX_20
                static_assert(
                    std::is_same_v<std::remove_cvref_t<decltype(sines_sum)>, FloatType> &&
                        std::is_same_v<std::remove_cvref_t<decltype(cosines_sum)>, FloatType>,
                    "sum_of_sines_and_cosines return type error");
#endif

                mpfr_set_zero(c_sines_sum, 0);
                mpfr_set_zero(c_cosines_sum, 0);
                mpfr_set(angle, alpha, kRoundMode);
                for (uint32_t i = 0; i < n; i++) {
                    mpfr_sin_cos(angle_sin, angle_cos, angle, kRoundMode);
                    mpfr_add(c_sines_sum, c_sines_sum, angle_sin, kRoundMode);
                    mpfr_add(c_cosines_sum, c_cosines_sum, angle_cos, kRoundMode);
                    mpfr_add(angle, angle, beta, kRoundMode);
                }
                const auto [sin_sum_correct, cos_sum_correct] = check_sums_correctness(
                    c_sines_sum, sines_sum, c_cosines_sum, cosines_sum, kSumEps);
                assert(sin_sum_correct);
                assert(cos_sum_correct);

                mpfr_mul_ui(beta, beta, angle_scale, kRoundMode);
            }

            mpfr_mul_ui(alpha, alpha, angle_scale, kRoundMode);
        }
    }
    mpfr_clear(angle_cos);
    mpfr_clear(angle_sin);
    mpfr_clear(c_cosines_sum);
    mpfr_clear(c_sines_sum);
    mpfr_clear(angle);
    mpfr_clear(beta);
    mpfr_clear(alpha);
}

void test_sin_cos_sum() noexcept {
    log_tests_started();

    test_sin_cos_sum_generic<float>();
    test_sin_cos_sum_generic<double>();
    test_sin_cos_sum_generic<long double>();
}

#endif

void test_visit_all_submasks() noexcept {
    log_tests_started();

    std::vector<uint64_t> vec;
    vec.reserve(128);
    visit_all_submasks(0b10100, [&](uint64_t m) { vec.push_back(m); });
    assert((vec == std::vector<uint64_t>{0b10100, 0b10000, 0b00100}));

    vec.clear();
    visit_all_submasks(0, [&](uint64_t m) { vec.push_back(m); });
    assert(vec.size() == 1 && vec[0] == 0);

    vec.clear();
    visit_all_submasks(0b111, [&](uint64_t m) { vec.push_back(m); });
    assert((vec == std::vector<uint64_t>{0b111, 0b110, 0b101, 0b100, 0b011, 0b010, 0b001}));
}

void test_prime_bitarrays() {
    log_tests_started();

    constexpr size_t N                    = 1000;
    const std::vector primes_as_bvector   = math_functions::dynamic_primes_sieve(N);
    const std::bitset<N + 1>& primes_bset = math_functions::fixed_primes_sieve<N>();
    constexpr uint32_t primes[]           = {
        2,   3,   5,   7,   11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,  53,   59,
        61,  67,  71,  73,  79,  83,  89,  97,  101, 103, 107, 109, 113, 127, 131, 137,  139,
        149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229,  233,
        239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331,  337,
        347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433,  439,
        443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547,  557,
        563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647,  653,
        659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761,  769,
        773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881,  883,
        887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997, 1009,
    };
    static_assert(primes[std::size(primes) - 1] >= N, "");
#if CONFIG_HAS_AT_LEAST_CXX_20
    static_assert(std::ranges::is_sorted(primes));
#endif
    size_t i = 0;
    assert(primes_as_bvector.size() == N + 1);
    for (uint32_t n = 0; n <= N; n++) {
        if (primes[i] < n) {
            i++;
        }
        bool is_prime = primes[i] == n;
        assert(primes_as_bvector[n] == is_prime);
        assert(primes_bset[n] == is_prime);
    }
}

void test_factorizer() {
    log_tests_started();

    constexpr auto N = static_cast<uint32_t>(1e7);
    Factorizer fact(N);
    {
        const auto is_prime = dynamic_primes_sieve(N);
        assert(is_prime.size() == N + 1);
        for (std::uint32_t i = 0; i <= N; i++) {
            assert(is_prime[i] == fact.is_prime(i));
        }
    }

    for (std::uint32_t i = 0; i <= N; i++) {
        auto pfs = fact.prime_factors(i);
        assert(pfs.size() == fact.number_of_unique_prime_factors(i));
        auto pfs_v = prime_factors_as_vector(i);
        assert(pfs.size() == pfs_v.size());
        assert(std::equal(
            pfs.begin(), pfs.end(), pfs_v.begin(), [](auto pf1, auto pf2) constexpr noexcept {
                return pf1.factor == pf2.factor && pf1.factor_power == pf2.factor_power;
            }));
    }
}

#if defined(INTEGERS_128_BIT_HPP)

template <class IntType>
bool test_extended_euclid_algorithm_a_b(std::size_t thread_id, IntType a, IntType b) noexcept {
    const auto [u, v, a_b_gcd] = math_functions::extended_euclid_algorithm(a, b);
    using HighIntType = std::conditional_t<sizeof(IntType) == sizeof(uint32_t), int64_t, int128_t>;
    const HighIntType hi_a = a;
    const HighIntType hi_b = b;
    const HighIntType hi_u = u;
    const HighIntType hi_v = v;
    if (unlikely(hi_a * hi_u + hi_b * hi_v != static_cast<HighIntType>(a_b_gcd))) {
        std::cerr << "In thread " << thread_id
                  << " a * u + b * v != gcd(a, b) when "
                     "a = "
                  << a
                  << ", "
                     "b = "
                  << b
                  << ", "
                     "u = "
                  << u
                  << ", "
                     "v = "
                  << v
                  << ", "
                     "gcd = "
                  << a_b_gcd << std::endl;
        return false;
    }

    bool valid_u_range = (b == 0 && u == ::math_functions::sign(a)) ^
                         (::math_functions::uabs(u) <= ::math_functions::uabs(b));
    if (unlikely(!valid_u_range)) {
        std::cerr << "In thread " << thread_id
                  << " not b == 0 && u == sign(a) xor -|b| <= u && u <= |b| when "
                     "a = "
                  << a
                  << ", "
                     "b = "
                  << b
                  << ", "
                     "u = "
                  << u
                  << ", "
                     "v = "
                  << v
                  << ", "
                     "gcd = "
                  << a_b_gcd << std::endl;
        return false;
    }

    bool valid_v_range = (a == 0 && v == ::math_functions::sign(b)) ^
                         (::math_functions::uabs(v) <= ::math_functions::uabs(a));
    if (unlikely(!valid_v_range)) {
        std::cerr << "In thread " << thread_id
                  << " not a == 0 && v == sign(b) xor -|a| <= v && v <= |a| when "
                     "a = "
                  << a
                  << ", "
                     "b = "
                  << b
                  << ", "
                     "u = "
                  << u
                  << ", "
                     "v = "
                  << v
                  << ", "
                     "gcd = "
                  << a_b_gcd << std::endl;
        return false;
    }

    const auto real_gcd = std::gcd(math_functions::uabs(a), math_functions::uabs(b));
    if (unlikely(a_b_gcd != real_gcd)) {
        std::cerr << "In thread " << thread_id
                  << " calculated gcd != std::gcd(a, b) when "
                     "a = "
                  << a
                  << ", "
                     "b = "
                  << b
                  << ", "
                     "u = "
                  << u
                  << ", "
                     "v = "
                  << v
                  << ", "
                     "wrong gcd = "
                  << a_b_gcd
                  << ", "
                     "true gcd = "
                  << real_gcd << std::endl;
        return false;
    }

    return true;
}

template <class IntType>
[[nodiscard]] bool multi_thread_test_extended_euclid_algorithm() {
    log_tests_started();

    constexpr std::size_t kTotalTests     = 1ull << 30;
    constexpr std::size_t kTotalThreads   = 4;
    constexpr std::size_t kTestsPerThread = kTotalTests / kTotalThreads;

    std::vector<std::thread> threads;
    threads.reserve(kTotalThreads);
    std::atomic_flag result = ATOMIC_FLAG_INIT;
    for (size_t i = 0; i < kTotalThreads; ++i) {
        threads.emplace_back([thread_id = i, &result
#if defined(_MSC_VER)
                              ,
                              kTestsPerThread
#endif
        ]() {
            const std::size_t seed = thread_id * 3'829'234'734ul + 27'273'489;
            std::cout << "Thread " << thread_id << " started, seed = " << seed << std::endl;

            static_assert(sizeof(IntType) == sizeof(std::uint32_t) ||
                          sizeof(IntType) == sizeof(std::uint64_t));
            using rnt_t = std::conditional_t<sizeof(IntType) == sizeof(std::uint32_t), std::mt19937,
                                             std::mt19937_64>;
            rnt_t mrs_rnd(static_cast<typename rnt_t::result_type>(seed));

            for (size_t test_iter = kTestsPerThread; test_iter != 0; --test_iter) {
                const auto a = static_cast<IntType>(mrs_rnd());
                const auto b = static_cast<IntType>(mrs_rnd());
                if (unlikely(!test_extended_euclid_algorithm_a_b(thread_id, a, b))) {
                    result.test_and_set();
                    break;
                }
            }
        });
    }

    for (auto&& thread : threads) {
        thread.join();
    }

    return !result.test_and_set();
}

#endif  // INTEGERS_128_BIT_HPP

void test_extended_euclid_algorithm() {
    constexpr uint64_t a     = std::numeric_limits<uint64_t>::max();
    constexpr uint64_t b     = 0;
    constexpr auto q         = math_functions::extended_euclid_algorithm(a, b);
    constexpr auto u_value   = q.u_value;
    constexpr auto v_value   = q.v_value;
    constexpr auto gcd_value = q.gcd_value;
    static_assert(gcd_value == std::gcd(a, b));
    static_assert(u_value * a + v_value * b == gcd_value);

#if defined(INTEGERS_128_BIT_HPP)
    assert(multi_thread_test_extended_euclid_algorithm<std::int32_t>());
    assert(multi_thread_test_extended_euclid_algorithm<std::uint32_t>());
    assert(multi_thread_test_extended_euclid_algorithm<std::int64_t>());
    assert(multi_thread_test_extended_euclid_algorithm<std::uint64_t>());
#endif
}

void test_solve_congruence_modulo_m_all_roots() {
    log_tests_started();

    const auto seed = std::ranlux24(static_cast<uint32_t>(std::time(nullptr)))();
    printf("Seed: %" PRIuFAST32 "\n", seed);
    std::mt19937 rnd_32(seed);

    constexpr auto kTotalTests = size_t{1} << 25;
    for (auto test_iter = kTotalTests; test_iter > 0; --test_iter) {
        const auto m = static_cast<std::uint32_t>(rnd_32());
        if (unlikely(m == 0)) {
            continue;
        }
        const auto a       = static_cast<std::uint32_t>(rnd_32());
        const auto c       = static_cast<std::uint32_t>(rnd_32());
        const auto roots   = math_functions::solve_congruence_modulo_m_all_roots(a, c, m);
        const auto gcd_a_m = std::gcd(a, m);
        assert((c % gcd_a_m == 0) == !roots.empty());
        if (!roots.empty()) {
            const auto c_mod_m = c % m;
            const auto step    = m / gcd_a_m;
            auto expected_x    = roots[0];
            for (const std::uint32_t x : roots) {
                assert(x < m);
                assert((uint64_t{a} * uint64_t{x}) % m == c_mod_m);
                assert(x == expected_x);
                expected_x += step;
            }
        }
    }
}

void test_solve_binary_congruence_modulo_m() {
    log_tests_started();

    const auto seed = std::ranlux24(std::uint32_t(std::time(nullptr)))();
    printf("Seed: %" PRIuFAST32 "\n", seed);
    std::mt19937 rnd_32(seed);

    constexpr auto kTotalTests = size_t{1} << 25;
    for (auto test_iter = kTotalTests; test_iter > 0; --test_iter) {
        const auto m = static_cast<std::uint32_t>(rnd_32());
        if (unlikely(m == 0)) {
            continue;
        }

        const auto k = std::uniform_int_distribution<uint32_t>(
            0, std::numeric_limits<uint16_t>::max())(rnd_32);
        const auto c      = static_cast<uint32_t>(rnd_32());
        const auto x      = solve_binary_congruence_modulo_m(k, c, m);
        const auto [r, s] = math_functions::extract_pow2(m);
        assert(r % 2 == 1 && r << s == m);
        const auto gcd_2k_m = uint32_t{1} << std::min(k, s);
        if (c % gcd_2k_m != 0) {
            assert(x == math_functions::kNoCongruenceSolution);
        } else {
            assert(x < m);
            // (2^{k} * x) % m
            const auto prod_of_2k_x = (uint64_t{bin_pow_mod(uint32_t{2}, k, m)} * uint64_t{x}) % m;
            assert(prod_of_2k_x == c % m);
        }

        assert(x == solve_congruence_modulo_m(bin_pow_mod(2u, k, m), c, m));
    }
}

void test_inv_mod_m() {
    log_tests_started();

    constexpr uint32_t first_prime_nums[] = {
        2,   3,   5,   7,   11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,  53,
        59,  61,  67,  71,  73,  79,  83,  89,  97,  101, 103, 107, 109, 113, 127, 131,
        137, 139, 157, 149, 151, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223,
        227, 229, 233, 239, 241, 257, 251, 263, 269, 271, 277, 281, 283, 293, 307, 311,
    };

    for (uint32_t m : first_prime_nums) {
        for (uint32_t a = 1; a < m; a++) {
            const auto a_inv = math_functions::inv_mod_m(a, m);
            assert(a_inv < m);
            assert((a * uint64_t(a_inv)) % m == 1);
        }
    }

    constexpr uint32_t big_prime_nums[] = {
        2147483489u, 2147483497u, 2147483543u, 2147483549u, 2147483563u,
        2147483579u, 2147483587u, 2147483629u, 2147483647u,
    };
    constexpr uint32_t kLimit = 100;
    for (uint32_t m : big_prime_nums) {
        for (uint32_t a = 1; a < std::min(m, kLimit); a++) {
            const auto a_inv = math_functions::inv_mod_m(a, m);
            assert(a_inv < m);
            assert((a * uint64_t(a_inv)) % m == 1);
        }
        for (uint32_t a = m - kLimit; a < m; a++) {
            const auto a_inv = math_functions::inv_mod_m(a, m);
            assert(a_inv < m);
            assert((a * uint64_t(a_inv)) % m == 1);
        }
    }

    for (uint32_t m = 3; m <= 1000; m++) {
        auto n_pfs = math_functions::prime_factors_as_vector(m);
        for (uint32_t a = 1; a < m; a++) {
            bool are_coprime = std::gcd(a, m) == 1;
            assert(are_coprime ==
                   std::all_of(n_pfs.begin(), n_pfs.end(),
                               [a](auto pf) constexpr noexcept { return a % pf.factor != 0; }));
            const auto a_inv = math_functions::inv_mod_m(a, m);
            if (!are_coprime) {
                assert(a_inv == math_functions::kNoCongruenceSolution);
                continue;
            }

            assert(a_inv < m);
            assert(a_inv != math_functions::kNoCongruenceSolution);
            assert((a * uint64_t(a_inv)) % m == 1);
        }
    }

    auto make_rbtree_set = [](const std::vector<std::uint64_t>& nums) {
        return std::set<std::uint64_t>(nums.begin(), nums.end());
    };
    auto make_hash_set = [](const std::vector<std::uint64_t>& nums) {
        return std::unordered_set<std::uint64_t>(nums.begin(), nums.end());
    };
    auto make_unique_vector = [](std::vector<std::uint64_t> nums) {
        std::sort(nums.begin(), nums.end());
        const auto iter = std::unique(nums.begin(), nums.end());
        nums.erase(iter, nums.end());
        return nums;
    };
    auto make_unique_list = [](std::vector<std::uint64_t> nums) {
        std::sort(nums.begin(), nums.end());
        const auto iter = std::unique(nums.begin(), nums.end());
        nums.erase(iter, nums.end());
        return std::list<std::uint64_t>(nums.begin(), nums.end());
    };

    constexpr std::mt19937_64::result_type kSeed = 372'134'058;
    std::mt19937_64 rnd_gen(kSeed);
    constexpr std::size_t n = 25'000;
    std::vector<uint64_t> nums(n);

    for (const std::uint32_t m : first_prime_nums) {
        std::generate(nums.begin(), nums.end(), [&rnd_gen, m]() noexcept {
            const auto rnd_num = rnd_gen();
            return rnd_num % m != 0 ? rnd_num : rnd_num + 1;
        });
        const auto [nums_mod_m, inv_nums] = math_functions::inv_mod_m(nums, m);
        assert(nums_mod_m.size() == n);
        assert(inv_nums.size() == n);
        for (std::size_t i = 0; i < n; i++) {
            assert(nums_mod_m[i] == nums[i] % m);
            assert(inv_nums[i] < m);
            assert((std::uint64_t{nums_mod_m[i]} * inv_nums[i]) % m == 1);
        }

        {
            auto [c1, c2] = math_functions::inv_mod_m(make_unique_vector(nums), m);
            auto [c3, c4] = math_functions::inv_mod_m(make_rbtree_set(nums), m);
            assert(c1 == c3);
            assert(c2 == c4);

            auto [c5, c6] = math_functions::inv_mod_m(make_hash_set(nums), m);
            std::sort(c1.begin(), c1.end());
            std::sort(c5.begin(), c5.end());
            assert(c1 == c5);
            std::sort(c2.begin(), c2.end());
            std::sort(c6.begin(), c6.end());
            assert(c2 == c6);

            auto [c7, c8] = math_functions::inv_mod_m(make_unique_list(nums), m);
            std::sort(c7.begin(), c7.end());
            assert(c1 == c7);
            std::sort(c8.begin(), c8.end());
            assert(c2 == c8);
        }
    }
}

void test_powers_sum() noexcept {
    log_tests_started();

    constexpr size_t kMaxM = 6;
    for (size_t m = 0; m <= kMaxM; m++) {
        static_assert(kMaxM <= 6);
        const auto max_n = [m]() noexcept -> std::uint32_t {
            switch (m) {
                case 6:
                    return 564;
                case 5:
                    return 1500;
                case 4:
                    return 9500;
                case 3:
                    return 91'000;
                case 2:
                    return 1'715'000;
                case 1:
                case 0:
                    return 1'000'000'000;
                default:
                    break;
            }
            assert(false);
            std::terminate();
        }();
        constexpr uint32_t kOffset = 50;
        assert(max_n >= kOffset);
        const uint32_t start_n = max_n - kOffset;
        uint64_t s             = 0;
        for (size_t i = 1; i < start_n; i++) {
            s += math_functions::bin_pow(i, m);
        }
        for (uint32_t n = start_n; n <= max_n; n++) {
            s += math_functions::bin_pow(uint64_t{n}, m);
            switch (m) {
                case 0:
                    assert(math_functions::powers_sum_u64<0>(n) == s);
                    assert(math_functions::powers_sum_u128<0>(n) == s);
                    break;
                case 1:
                    assert(math_functions::powers_sum_u64<1>(n) == s);
                    assert(math_functions::powers_sum_u128<1>(n) == s);
                    break;
                case 2:
                    assert(math_functions::powers_sum_u64<2>(n) == s);
                    assert(math_functions::powers_sum_u128<2>(n) == s);
                    break;
                case 3:
                    assert(math_functions::powers_sum_u64<3>(n) == s);
                    assert(math_functions::powers_sum_u128<3>(n) == s);
                    break;
                case 4:
                    assert(math_functions::powers_sum_u64<4>(n) == s);
                    assert(math_functions::powers_sum_u128<4>(n) == s);
                    break;
                case 5:
                    assert(math_functions::powers_sum_u64<5>(n) == s);
                    assert(math_functions::powers_sum_u128<5>(n) == s);
                    break;
                case 6:
                    assert(math_functions::powers_sum_u64<6>(n) == s);
                    assert(math_functions::powers_sum_u128<6>(n) == s);
                    break;
                default:
                    assert(false);
            }
        }
    }
}

void test_solve_factorial_congruence() noexcept {
    log_tests_started();

    assert(solve_factorial_congruence(12, 6) == 5);
    assert(solve_factorial_congruence(6, 3) == 2);
    assert(solve_factorial_congruence(6, 12) == 2);
    assert(solve_factorial_congruence(0xffffffffu, 2) == 4294967263u);

    static_assert(1ull * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9 * 10 * 11 * 12 <=
                  std::numeric_limits<uint32_t>::max());
    static_assert(1ull * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9 * 10 * 11 * 12 * 13 >
                  std::numeric_limits<uint32_t>::max());

    uint32_t n_fact = 1;
    for (uint32_t n = 1; n <= 12; n++) {
        n_fact *= n;
        uint32_t k = n_fact + 1;
        assert(solve_factorial_congruence(n, k) == 0);
    }
}

void test_general_asserts() {
#define STRINGIFY(expr) #expr
#define ASSERT_THAT(expr)                       \
    do {                                        \
        assert(expr);                           \
        static_assert((expr), STRINGIFY(expr)); \
    } while (false)

#if defined(__cpp_constexpr) && __cpp_constexpr >= 202211L && defined(__GNUG__)
#define LOG10_ASSERT_THAT(expr)                 \
    do {                                        \
        assert(expr);                           \
        static_assert((expr), STRINGIFY(expr)); \
    } while (false)
#else
#define LOG10_ASSERT_THAT(expr) \
    do {                        \
        assert(expr);           \
    } while (false)
#endif

    ASSERT_THAT(bin_pow_mod(uint32_t{7}, uint32_t{483}, uint32_t{1000000007u}) == 263145387u);
    ASSERT_THAT(bin_pow_mod(uint32_t{289}, std::numeric_limits<uint32_t>::max(),
                            uint32_t{2146514599u}) == 1349294778u);
    ASSERT_THAT(bin_pow_mod(uint32_t{2146526839u}, uint32_t{578423432u}, uint32_t{2147483629u}) ==
                281853233u);

#if defined(HAS_I128_CONSTEXPR) && HAS_I128_CONSTEXPR
    ASSERT_THAT(bin_pow_mod(uint64_t{7}, uint64_t{483}, uint64_t{1000000007u}) == 263145387u);
    ASSERT_THAT(bin_pow_mod(uint64_t{289}, uint64_t{std::numeric_limits<uint32_t>::max()},
                            uint64_t{2146514599u}) == 1349294778u);
    ASSERT_THAT(bin_pow_mod(uint64_t{2146526839u}, uint64_t{578423432u}, uint64_t{2147483629u}) ==
                281853233u);
    ASSERT_THAT(bin_pow_mod(uint64_t{119999999927ull}, uint64_t{18446744073709515329ull},
                            uint64_t{100000000000000003ull}) == 85847679703545452ull);
    ASSERT_THAT(bin_pow_mod(uint64_t{72057594037927843ull}, uint64_t{18446744073709515329ull},
                            uint64_t{1000000000000000003ull}) == 404835689235904145ull);
    ASSERT_THAT(bin_pow_mod(uint64_t{999999999999999487ull}, uint64_t{18446744073709551557ull},
                            uint64_t{1000000000000000009ull}) == 802735487082721113ull);

    ASSERT_THAT(bin_pow_mod(uint64_t{2}, uint64_t{18446744073709551427ull} - 1,
                            uint64_t{18446744073709551427ull}) == 1);
    ASSERT_THAT(bin_pow_mod(uint64_t{3}, uint64_t{18446744073709551427ull} - 1,
                            uint64_t{18446744073709551427ull}) == 1);
    ASSERT_THAT(bin_pow_mod(uint64_t{1238873}, uint64_t{18446744073709551427ull} - 1,
                            uint64_t{18446744073709551427ull}) == 1);
#endif

#if CONFIG_HAS_AT_LEAST_CXX_20

    ASSERT_THAT(isqrt(0u) == 0);
    ASSERT_THAT(isqrt(1u) == 1);
    ASSERT_THAT(isqrt(4u) == 2);
    ASSERT_THAT(isqrt(9u) == 3);
    ASSERT_THAT(isqrt(10u) == 3);
    ASSERT_THAT(isqrt(15u) == 3);
    ASSERT_THAT(isqrt(16u) == 4);
    ASSERT_THAT(isqrt(257u * 257u) == 257);
    ASSERT_THAT(isqrt(257u * 257u + 1) == 257);
    ASSERT_THAT(isqrt(258u * 258u - 1u) == 257);
    ASSERT_THAT(isqrt(1u << 12) == 1 << 6);
    ASSERT_THAT(isqrt(1u << 14) == 1 << 7);
    ASSERT_THAT(isqrt(1u << 16) == 1 << 8);
    ASSERT_THAT(isqrt(1u << 28) == 1 << 14);
    ASSERT_THAT(isqrt(1u << 30) == 1 << 15);
    ASSERT_THAT(isqrt(std::numeric_limits<uint32_t>::max()) == (1u << 16) - 1);

#endif

    ASSERT_THAT(isqrt(uint64_t{0}) == 0);
    ASSERT_THAT(isqrt(uint64_t{1}) == 1);
    ASSERT_THAT(isqrt(uint64_t{4}) == 2);
    ASSERT_THAT(isqrt(uint64_t{9}) == 3);
    ASSERT_THAT(isqrt(uint64_t{10}) == 3);
    ASSERT_THAT(isqrt(uint64_t{15}) == 3);
    ASSERT_THAT(isqrt(uint64_t{16}) == 4);
    ASSERT_THAT(isqrt(uint64_t{257 * 257}) == 257);
    ASSERT_THAT(isqrt(uint64_t{257 * 257 + 1}) == 257);
    ASSERT_THAT(isqrt(uint64_t{258 * 258 - 1}) == 257);
    ASSERT_THAT(isqrt(uint64_t{1000000007} * 1000000007) == 1000000007u);
    ASSERT_THAT(isqrt(uint64_t{1} << 12) == uint64_t{1} << 6);
    ASSERT_THAT(isqrt(uint64_t{1} << 14) == uint64_t{1} << 7);
    ASSERT_THAT(isqrt(uint64_t{1} << 16) == uint64_t{1} << 8);
    ASSERT_THAT(isqrt(uint64_t{1} << 28) == uint64_t{1} << 14);
    ASSERT_THAT(isqrt(uint64_t{1} << 30) == uint64_t{1} << 15);
    ASSERT_THAT(isqrt(uint64_t{1} << 54) == uint64_t{1} << 27);
    ASSERT_THAT(isqrt(uint64_t{1} << 56) == uint64_t{1} << 28);
    ASSERT_THAT(isqrt(uint64_t{1} << 58) == uint64_t{1} << 29);
    ASSERT_THAT(isqrt(uint64_t{1} << 60) == uint64_t{1} << 30);
    ASSERT_THAT(isqrt(uint64_t{1} << 62) == uint64_t{1} << 31);
    ASSERT_THAT(isqrt(std::numeric_limits<uint64_t>::max()) == 0xFFFFFFFFu);

#if defined(INTEGERS_128_BIT_HPP) && defined(HAS_I128_CONSTEXPR) && HAS_I128_CONSTEXPR

    ASSERT_THAT(isqrt(uint128_t{0}) == 0);
    ASSERT_THAT(isqrt(uint128_t{1}) == 1);
    ASSERT_THAT(isqrt(uint128_t{4}) == 2);
    ASSERT_THAT(isqrt(uint128_t{9}) == 3);
    ASSERT_THAT(isqrt(uint128_t{10}) == 3);
    ASSERT_THAT(isqrt(uint128_t{15}) == 3);
    ASSERT_THAT(isqrt(uint128_t{16}) == 4);
    ASSERT_THAT(isqrt(uint128_t{257 * 257}) == 257);
    ASSERT_THAT(isqrt(uint128_t{257 * 257 + 1}) == 257);
    ASSERT_THAT(isqrt(uint128_t{258 * 258 - 1}) == 257);
    ASSERT_THAT(isqrt(uint128_t{1} << 12) == uint64_t{1} << 6);
    ASSERT_THAT(isqrt(uint128_t{1} << 14) == uint64_t{1} << 7);
    ASSERT_THAT(isqrt(uint128_t{1} << 16) == uint64_t{1} << 8);
    ASSERT_THAT(isqrt(uint128_t{1} << 28) == uint64_t{1} << 14);
    ASSERT_THAT(isqrt(uint128_t{1} << 30) == uint64_t{1} << 15);
    ASSERT_THAT(isqrt(uint128_t{1} << 54) == uint64_t{1} << 27);
    ASSERT_THAT(isqrt(uint128_t{1} << 56) == uint64_t{1} << 28);
    ASSERT_THAT(isqrt(uint128_t{1} << 58) == uint64_t{1} << 29);
    ASSERT_THAT(isqrt(uint128_t{1} << 60) == uint64_t{1} << 30);
    ASSERT_THAT(isqrt(uint128_t{1} << 62) == uint64_t{1} << 31);
    ASSERT_THAT(isqrt(uint128_t(std::numeric_limits<uint64_t>::max())) == (uint64_t{1} << 32) - 1);
    ASSERT_THAT(isqrt(uint128_t{1} << 126) == uint64_t{1} << 63);
    ASSERT_THAT(isqrt(uint128_t(-1)) == (uint128_t{1} << 64) - 1);
    ASSERT_THAT(isqrt(uint128_t{1000000007} * 1000000007) == 1000000007);
    ASSERT_THAT(isqrt(uint128_t{1000000000000000003ull} * 1000000000000000003ull) ==
                1000000000000000003ull);
    ASSERT_THAT(isqrt(uint128_t{1000000000000000009ull} * 1000000000000000009ull) ==
                1000000000000000009ull);
    ASSERT_THAT(isqrt(uint128_t{18446744073709551521ull} * 18446744073709551521ull) ==
                18446744073709551521ull);
    ASSERT_THAT(isqrt(uint128_t{18446744073709551533ull} * 18446744073709551533ull) ==
                18446744073709551533ull);
    ASSERT_THAT(isqrt(uint128_t{18446744073709551557ull} * 18446744073709551557ull) ==
                18446744073709551557ull);
    ASSERT_THAT(isqrt(uint128_t{18446744073709551557ull} * 18446744073709551557ull + 1) ==
                18446744073709551557ull);
    ASSERT_THAT(isqrt(uint128_t{18446744073709551558ull} * 18446744073709551558ull - 1) ==
                18446744073709551557ull);
    ASSERT_THAT(isqrt(uint128_t{18446744073709551558ull} * 18446744073709551558ull) ==
                18446744073709551558ull);
#endif

    ASSERT_THAT(icbrt(uint32_t{0}) == 0);
    ASSERT_THAT(icbrt(uint32_t{1}) == 1);
    ASSERT_THAT(icbrt(uint32_t{8}) == 2);
    ASSERT_THAT(icbrt(uint32_t{27}) == 3);
    ASSERT_THAT(icbrt(uint32_t{64}) == 4);
    ASSERT_THAT(icbrt(uint32_t{65}) == 4);
    ASSERT_THAT(icbrt(uint32_t{124}) == 4);
    ASSERT_THAT(icbrt(uint32_t{125}) == 5);
    ASSERT_THAT(icbrt(uint32_t{126}) == 5);
    ASSERT_THAT(icbrt(uint32_t{3375}) == 15);
    ASSERT_THAT(icbrt(uint32_t{257 * 257 * 257 - 1}) == 256);
    ASSERT_THAT(icbrt(uint32_t{257 * 257 * 257}) == 257);
    ASSERT_THAT(icbrt(uint32_t{257 * 257 * 257 + 1}) == 257);
    ASSERT_THAT(icbrt(uint32_t{258 * 258 * 258 - 1}) == 257);
    ASSERT_THAT(icbrt(uint32_t{258 * 258 * 258}) == 258);
    ASSERT_THAT(icbrt(uint32_t{258 * 258 * 258 + 1}) == 258);
    ASSERT_THAT(icbrt(uint32_t{289} * 289 * 289) == 289);
    ASSERT_THAT(icbrt(uint32_t{289} * 289 * 289 + 1) == 289);
    ASSERT_THAT(icbrt(uint32_t{290} * 290 * 290 - 1) == 289);
    ASSERT_THAT(icbrt(uint32_t{290} * 290 * 290) == 290);
    ASSERT_THAT(icbrt(uint32_t{1} << 15) == 1 << 5);
    ASSERT_THAT(icbrt(uint32_t{1} << 18) == 1 << 6);
    ASSERT_THAT(icbrt(uint32_t{1} << 21) == 1 << 7);
    ASSERT_THAT(icbrt(uint32_t{1} << 24) == 1 << 8);
    ASSERT_THAT(icbrt(uint32_t{1} << 27) == 1 << 9);
    ASSERT_THAT(icbrt(uint32_t{1} << 30) == 1 << 10);
    ASSERT_THAT(icbrt(std::numeric_limits<uint32_t>::max()) == 1625);

    ASSERT_THAT(icbrt(uint64_t{0}) == 0);
    ASSERT_THAT(icbrt(uint64_t{1}) == 1);
    ASSERT_THAT(icbrt(uint64_t{8}) == 2);
    ASSERT_THAT(icbrt(uint64_t{27}) == 3);
    ASSERT_THAT(icbrt(uint64_t{64}) == 4);
    ASSERT_THAT(icbrt(uint64_t{65}) == 4);
    ASSERT_THAT(icbrt(uint64_t{124}) == 4);
    ASSERT_THAT(icbrt(uint64_t{125}) == 5);
    ASSERT_THAT(icbrt(uint64_t{126}) == 5);
    ASSERT_THAT(icbrt(uint64_t{3375}) == 15);
    ASSERT_THAT(icbrt(uint64_t{257 * 257 * 257 - 1}) == 256);
    ASSERT_THAT(icbrt(uint64_t{257 * 257 * 257}) == 257);
    ASSERT_THAT(icbrt(uint64_t{257 * 257 * 257 + 1}) == 257);
    ASSERT_THAT(icbrt(uint64_t{258 * 258 * 258 - 1}) == 257);
    ASSERT_THAT(icbrt(uint64_t{258 * 258 * 258}) == 258);
    ASSERT_THAT(icbrt(uint64_t{258 * 258 * 258 + 1}) == 258);
    ASSERT_THAT(icbrt(uint64_t{289} * 289 * 289) == 289);
    ASSERT_THAT(icbrt(uint64_t{289} * 289 * 289 + 1) == 289);
    ASSERT_THAT(icbrt(uint64_t{290} * 290 * 290 - 1) == 289);
    ASSERT_THAT(icbrt(uint64_t{290} * 290 * 290) == 290);
    ASSERT_THAT(icbrt(uint64_t{1} << 15) == 1 << 5);
    ASSERT_THAT(icbrt(uint64_t{1} << 18) == 1 << 6);
    ASSERT_THAT(icbrt(uint64_t{1} << 21) == 1 << 7);
    ASSERT_THAT(icbrt(uint64_t{1} << 24) == 1 << 8);
    ASSERT_THAT(icbrt(uint64_t{1} << 27) == 1 << 9);
    ASSERT_THAT(icbrt(uint64_t{1} << 30) == 1 << 10);
    ASSERT_THAT(icbrt(uint64_t{std::numeric_limits<uint32_t>::max()}) == 1625);
    ASSERT_THAT(icbrt(uint64_t{1} << 33) == 1 << 11);
    ASSERT_THAT(icbrt(uint64_t{1} << 36) == 1 << 12);
    ASSERT_THAT(icbrt(uint64_t{1} << 39) == 1 << 13);
    ASSERT_THAT(icbrt(uint64_t{1} << 42) == 1 << 14);
    ASSERT_THAT(icbrt(uint64_t{1} << 45) == 1 << 15);
    ASSERT_THAT(icbrt(uint64_t{1} << 48) == 1 << 16);
    ASSERT_THAT(icbrt(uint64_t{1} << 51) == 1 << 17);
    ASSERT_THAT(icbrt(uint64_t{1} << 54) == 1 << 18);
    ASSERT_THAT(icbrt(uint64_t{1} << 57) == 1 << 19);
    ASSERT_THAT(icbrt(uint64_t{1} << 60) == 1 << 20);
    ASSERT_THAT(icbrt(uint64_t{1} << 63) == 1 << 21);
    ASSERT_THAT(icbrt((uint64_t{1} << 63) | (uint64_t{1} << 32)) == 2097152);
    ASSERT_THAT(icbrt(uint64_t{1'367'631'000'000'000ull}) == 111'000);
    ASSERT_THAT(icbrt(uint64_t{1'000'000'000'000'000'000ull}) == 1'000'000);
    ASSERT_THAT(icbrt(uint64_t{1'331'000'000'000'000'000ull}) == 1'100'000);
    ASSERT_THAT(icbrt(uint64_t{8'000'000'000'000'000'000ull}) == 2'000'000);
    ASSERT_THAT(icbrt(uint64_t{15'625'000'000'000'000'000ull}) == 2'500'000);
    ASSERT_THAT(icbrt(std::numeric_limits<uint64_t>::max()) == 2642245);

    ASSERT_THAT(is_perfect_square(uint64_t{0}));
    ASSERT_THAT(is_perfect_square(uint64_t{1}));
    ASSERT_THAT(!is_perfect_square(uint64_t{2}));
    ASSERT_THAT(!is_perfect_square(uint64_t{3}));
    ASSERT_THAT(is_perfect_square(uint64_t{4}));
    ASSERT_THAT(!is_perfect_square(uint64_t{5}));
    ASSERT_THAT(is_perfect_square(uint64_t{9}));
    ASSERT_THAT(!is_perfect_square(uint64_t{15}));
    ASSERT_THAT(is_perfect_square(uint64_t{16}));
    ASSERT_THAT(is_perfect_square(uint64_t{324}));
    ASSERT_THAT(is_perfect_square(uint64_t{1} << 16));
    ASSERT_THAT(is_perfect_square(uint64_t{1} << 24));
    ASSERT_THAT(is_perfect_square(uint64_t{1} << 32));
    ASSERT_THAT(is_perfect_square(uint64_t{1} << 40));
    ASSERT_THAT(is_perfect_square(uint64_t{1} << 48));
    ASSERT_THAT(is_perfect_square(uint64_t{1} << 56));
    ASSERT_THAT(is_perfect_square(uint64_t{1} << 60));
    ASSERT_THAT(is_perfect_square(uint64_t{1} << 62));

#if defined(HAS_I128_CONSTEXPR) && HAS_I128_CONSTEXPR
    ASSERT_THAT(is_perfect_square(uint128_t{0}));
    ASSERT_THAT(is_perfect_square(uint128_t{1}));
    ASSERT_THAT(!is_perfect_square(uint128_t{2}));
    ASSERT_THAT(!is_perfect_square(uint128_t{3}));
    ASSERT_THAT(is_perfect_square(uint128_t{4}));
    ASSERT_THAT(!is_perfect_square(uint128_t{5}));
    ASSERT_THAT(is_perfect_square(uint128_t{9}));
    ASSERT_THAT(!is_perfect_square(uint128_t{15}));
    ASSERT_THAT(is_perfect_square(uint128_t{16}));
    ASSERT_THAT(is_perfect_square(uint128_t{324}));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 16));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 24));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 32));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 40));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 48));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 56));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 60));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 62));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 64));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 66));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 68));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 70));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 72));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 74));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 76));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 78));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 80));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 126));
#endif

    ASSERT_THAT(bit_reverse(uint8_t{0b00000000}) == 0b00000000);
    ASSERT_THAT(bit_reverse(uint8_t{0b00000010}) == 0b01000000);
    ASSERT_THAT(bit_reverse(uint8_t{0b00001100}) == 0b00110000);
    ASSERT_THAT(bit_reverse(uint8_t{0b10101010}) == 0b01010101);
    ASSERT_THAT(bit_reverse(uint8_t{0b01010101}) == 0b10101010);
    ASSERT_THAT(bit_reverse(uint8_t{0b11111111}) == 0b11111111);

    ASSERT_THAT(bit_reverse(0b00000000'00000000'00000000'00000000u) ==
                0b00000000'00000000'00000000'00000000u);
    ASSERT_THAT(bit_reverse(0b00000000'00000000'00000000'00000001u) ==
                0b10000000'00000000'00000000'00000000u);
    ASSERT_THAT(bit_reverse(0b10000000'00000000'00000000'00000000u) ==
                0b00000000'00000000'00000000'00000001u);
    ASSERT_THAT(bit_reverse(0b00000000'11111111'00000000'00000000u) ==
                0b00000000'00000000'11111111'00000000u);
    ASSERT_THAT(bit_reverse(0b00000000'00000000'11111111'00000000u) ==
                0b00000000'11111111'00000000'00000000u);
    ASSERT_THAT(bit_reverse(0b10101010'10101010'10101010'10101010u) ==
                0b01010101'01010101'01010101'01010101u);
    ASSERT_THAT(bit_reverse(0b11111111'00000000'11111111'00000000u) ==
                0b00000000'11111111'00000000'11111111u);

    ASSERT_THAT(
        bit_reverse(uint64_t(
            0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL)) ==
        0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL);
    ASSERT_THAT(
        bit_reverse(uint64_t(
            0b10000001'00000000'10000001'00000000'10000001'00000000'10000001'00000000ULL)) ==
        0b00000000'10000001'00000000'10000001'00000000'10000001'00000000'10000001ULL);
    ASSERT_THAT(
        bit_reverse(uint64_t(
            0b00001111'00000000'11110000'00000000'10101010'00000000'00000000'00000000ULL)) ==
        0b00000000'00000000'00000000'01010101'00000000'00001111'00000000'11110000ULL);
    ASSERT_THAT(
        bit_reverse(uint64_t(
            0b00000000'00000000'00000000'10101010'10101010'00000000'00000000'00000000ULL)) ==
        0b00000000'00000000'00000000'01010101'01010101'00000000'00000000'00000000ULL);
    ASSERT_THAT(
        bit_reverse(uint64_t(
            0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL)) ==
        0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL);
    ASSERT_THAT(
        bit_reverse(uint64_t(
            0b11111111'00000000'11111111'00000000'11111111'00000000'11111111'00000000ULL)) ==
        0b00000000'11111111'00000000'11111111'00000000'11111111'00000000'11111111ULL);
    ASSERT_THAT(
        bit_reverse(uint64_t(
            0b11111111'11111111'11111111'11111111'00000000'00000000'00000000'00000000ULL)) ==
        0b00000000'00000000'00000000'00000000'11111111'11111111'11111111'11111111ULL);

#if defined(HAS_I128_CONSTEXPR) && HAS_I128_CONSTEXPR

    ASSERT_THAT(bit_reverse(uint128_t(0)) == 0);
    ASSERT_THAT(bit_reverse(uint128_t(uint64_t(-1)) << 64) == uint64_t(-1));
    ASSERT_THAT(bit_reverse((uint128_t(uint64_t(-1)) << 64) | 1) ==
                ((uint128_t(1) << 127) | uint64_t(-1)));
    ASSERT_THAT(bit_reverse(uint128_t(-1)) == uint128_t(-1));

#endif

#if CONFIG_HAS_AT_LEAST_CXX_20
    ASSERT_THAT(int(detail::pop_count_32_software(0u)) == int(std::popcount(0u)));
    ASSERT_THAT(int(detail::pop_count_32_software(1u)) == int(std::popcount(1u)));
    ASSERT_THAT(int(detail::pop_count_32_software(2u)) == int(std::popcount(2u)));
    ASSERT_THAT(int(detail::pop_count_32_software(3u)) == int(std::popcount(3u)));
    ASSERT_THAT(int(detail::pop_count_32_software(4u)) == int(std::popcount(4u)));
    ASSERT_THAT(int(detail::pop_count_32_software(0x4788743u)) == int(std::popcount(0x4788743u)));
    ASSERT_THAT(int(detail::pop_count_32_software(0x2D425B23u)) == int(std::popcount(0x2D425B23u)));
    ASSERT_THAT(int(detail::pop_count_32_software(0xFFFFFFFFu - 1)) ==
                int(std::popcount(0xFFFFFFFFu - 1)));
    ASSERT_THAT(int(detail::pop_count_32_software(0xFFFFFFFFu)) == int(std::popcount(0xFFFFFFFFu)));
#endif

#if CONFIG_HAS_AT_LEAST_CXX_20
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t(0))) == int(std::popcount(uint64_t(0))));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t(1))) == int(std::popcount(uint64_t(1))));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t(2))) == int(std::popcount(uint64_t(2))));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t(3))) == int(std::popcount(uint64_t(3))));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t(4))) == int(std::popcount(uint64_t(4))));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t(0x4788743u))) ==
                int(std::popcount(uint64_t(0x4788743u))));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t(0x2D425B23u))) ==
                int(std::popcount(uint64_t(0x2D425B23u))));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t(0xFFFFFFFFu - 1))) ==
                int(std::popcount(uint64_t(0xFFFFFFFFu - 1))));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t(0xFFFFFFFFu))) ==
                int(std::popcount(uint64_t(0xFFFFFFFFu))));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t(0x5873485893484ull))) ==
                int(std::popcount(uint64_t(0x5873485893484ull))));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t(0x85923489853245ull))) ==
                int(std::popcount(uint64_t(0x85923489853245ull))));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t(0xFFFFFFFFFFFFFFFFull - 1))) ==
                int(std::popcount(uint64_t(0xFFFFFFFFFFFFFFFFull - 1))));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t(0xFFFFFFFFFFFFFFFFull))) ==
                int(std::popcount(uint64_t(0xFFFFFFFFFFFFFFFFull))));
#endif

#if CONFIG_HAS_AT_LEAST_CXX_20
    ASSERT_THAT(std::popcount(0u) - std::popcount(0u) == pop_diff(0, 0));
    ASSERT_THAT(int(std::popcount(1u)) - int(std::popcount(0u)) == pop_diff(1, 0));
    ASSERT_THAT(int(std::popcount(0u)) - int(std::popcount(1u)) == pop_diff(0, 1));
    ASSERT_THAT(int(std::popcount(0xABCDEFu)) - int(std::popcount(4u)) == pop_diff(0xABCDEF, 4));
    ASSERT_THAT(int(std::popcount(uint32_t(uint16_t(-1)))) - int(std::popcount(314u)) ==
                pop_diff(uint16_t(-1), 314));
    ASSERT_THAT(int(std::popcount(uint32_t(-1))) - int(std::popcount(0u)) ==
                pop_diff(uint32_t(-1), 0));
    ASSERT_THAT(int(std::popcount(0u)) - int(std::popcount(uint32_t(-1))) ==
                pop_diff(0, uint32_t(-1)));
    ASSERT_THAT(int(std::popcount(uint32_t(-1))) - int(std::popcount(uint32_t(-1))) ==
                pop_diff(uint32_t(-1), uint32_t(-1)));
#endif

#if defined(HAS_I128_CONSTEXPR) && HAS_I128_CONSTEXPR
    ASSERT_THAT(sign(int128_t(0)) == 0);
    ASSERT_THAT(sign(int128_t(1)) == 1);
    ASSERT_THAT(sign(int128_t(-1)) == -1);
    ASSERT_THAT(sign(int128_t(2)) == 1);
    ASSERT_THAT(sign(int128_t(-2)) == -1);
    ASSERT_THAT(sign(int128_t(18446744073709551615ull)) == 1);
    ASSERT_THAT(sign(-int128_t(18446744073709551615ull)) == -1);
    ASSERT_THAT(sign(int128_t(1) << 63) == 1);
    ASSERT_THAT(sign(-(int128_t(1) << 63)) == -1);
    ASSERT_THAT(sign(int128_t(1) << 126) == 1);
    ASSERT_THAT(sign(-(int128_t(1) << 126)) == -1);
    ASSERT_THAT(sign(int128_t((uint128_t(1) << 127) - 1)) == 1);
    ASSERT_THAT(sign(int128_t(-((uint128_t(1) << 127) - 1))) == -1);
    ASSERT_THAT(sign(int128_t(-(uint128_t(1) << 127))) == -1);
#endif

    ASSERT_THAT(same_sign_soft(1, 1));
    ASSERT_THAT(same_sign_soft(1, 0));
    ASSERT_THAT(!same_sign_soft(1, -1));
    ASSERT_THAT(same_sign_soft(0, 1));
    ASSERT_THAT(same_sign_soft(0, 0));
    ASSERT_THAT(!same_sign_soft(0, -1));
    ASSERT_THAT(!same_sign_soft(-1, 1));
    ASSERT_THAT(!same_sign_soft(-1, 0));
    ASSERT_THAT(same_sign_soft(-1, -1));

    ASSERT_THAT(same_sign(1, 1));
    ASSERT_THAT(!same_sign(1, 0));
    ASSERT_THAT(!same_sign(1, -1));
    ASSERT_THAT(!same_sign(0, 1));
    ASSERT_THAT(same_sign(0, 0));
    ASSERT_THAT(!same_sign(0, -1));
    ASSERT_THAT(!same_sign(-1, 1));
    ASSERT_THAT(!same_sign(-1, 0));
    ASSERT_THAT(same_sign(-1, -1));

    ASSERT_THAT(uabs(static_cast<signed char>(0)) == 0);
    ASSERT_THAT(uabs(static_cast<unsigned char>(0)) == 0);
    ASSERT_THAT(uabs(static_cast<short>(0)) == 0);
    ASSERT_THAT(uabs(static_cast<unsigned short>(0)) == 0);
    ASSERT_THAT(uabs(static_cast<int>(0)) == 0);
    ASSERT_THAT(uabs(static_cast<unsigned int>(0)) == 0);
    ASSERT_THAT(uabs(static_cast<long>(0)) == 0);
    ASSERT_THAT(uabs(static_cast<unsigned long>(0)) == 0);
    ASSERT_THAT(uabs(static_cast<long long>(0)) == 0);
    ASSERT_THAT(uabs(static_cast<unsigned long long>(0)) == 0);

    ASSERT_THAT(uabs(static_cast<signed char>(-1)) == 1);
    ASSERT_THAT(uabs(static_cast<unsigned char>(1)) == 1);
    ASSERT_THAT(uabs(static_cast<short>(-1)) == 1);
    ASSERT_THAT(uabs(static_cast<unsigned short>(1)) == 1);
    ASSERT_THAT(uabs(static_cast<int>(-1)) == 1);
    ASSERT_THAT(uabs(static_cast<unsigned>(1)) == 1);
    ASSERT_THAT(uabs(static_cast<long>(-1)) == 1);
    ASSERT_THAT(uabs(static_cast<unsigned long>(1)) == 1);
    ASSERT_THAT(uabs(static_cast<long long>(-1)) == 1);
    ASSERT_THAT(uabs(static_cast<unsigned long long>(1)) == 1);

    ASSERT_THAT(uabs(static_cast<signed char>(-128)) == 128);
    ASSERT_THAT(uabs(static_cast<signed char>(128)) == 128);
    ASSERT_THAT(uabs(static_cast<short>(-128)) == 128);
    ASSERT_THAT(uabs(static_cast<short>(128)) == 128);
    ASSERT_THAT(uabs(static_cast<int>(-128)) == 128);
    ASSERT_THAT(uabs(static_cast<int>(128)) == 128);
    ASSERT_THAT(uabs(static_cast<long>(-128)) == 128);
    ASSERT_THAT(uabs(static_cast<long>(128)) == 128);
    ASSERT_THAT(uabs(static_cast<long long>(-128)) == 128);
    ASSERT_THAT(uabs(static_cast<long long>(128)) == 128);

    ASSERT_THAT(uabs(std::numeric_limits<long long>::min()) ==
                -static_cast<unsigned long long>(std::numeric_limits<long long>::min()));

    ASSERT_THAT(uabs(int8_t{0}) == 0);
    ASSERT_THAT(uabs(uint8_t{0}) == 0);
    ASSERT_THAT(uabs(int32_t{0}) == 0);
    ASSERT_THAT(uabs(uint32_t{0}) == 0);
    ASSERT_THAT(uabs(int64_t{0}) == 0);
    ASSERT_THAT(uabs(uint64_t{0}) == 0);
    ASSERT_THAT(uabs(int8_t{-1}) == 1);
    ASSERT_THAT(uabs(uint8_t{1}) == 1);
    ASSERT_THAT(uabs(int32_t{-1}) == 1);
    ASSERT_THAT(uabs(uint32_t{1}) == 1);
    ASSERT_THAT(uabs(int64_t{-1}) == 1);
    ASSERT_THAT(uabs(uint64_t{1}) == 1);
    ASSERT_THAT(uabs(int8_t{-128}) == 128);
    ASSERT_THAT(uabs(uint8_t{128}) == 128);
    ASSERT_THAT(uabs(int32_t{-128}) == 128);
    ASSERT_THAT(uabs(uint32_t{128}) == 128);
    ASSERT_THAT(uabs(int64_t{-128}) == 128);
    ASSERT_THAT(uabs(uint64_t{128}) == 128);

    ASSERT_THAT(uabs(std::numeric_limits<int32_t>::min()) ==
                -static_cast<uint32_t>(std::numeric_limits<int32_t>::min()));
    ASSERT_THAT(uabs(std::numeric_limits<int64_t>::min()) ==
                -static_cast<uint64_t>(std::numeric_limits<int64_t>::min()));

#if defined(HAS_I128_CONSTEXPR) && HAS_I128_CONSTEXPR
    ASSERT_THAT(uabs(int128_t{0}) == 0);
    ASSERT_THAT(uabs(uint128_t{0}) == 0);
    ASSERT_THAT(uabs(int128_t{-1}) == 1);
    ASSERT_THAT(uabs(int128_t{1}) == 1);
    ASSERT_THAT(uabs(uint128_t{1}) == 1);
    ASSERT_THAT(uabs(int128_t{-4}) == 4);
    ASSERT_THAT(uabs(int128_t{4}) == 4);
    ASSERT_THAT(uabs(uint128_t{4}) == 4);
    ASSERT_THAT(uabs(-int128_t{18446744073709551615ull}) == 18446744073709551615ull);
    ASSERT_THAT(uabs(int128_t{18446744073709551615ull}) == 18446744073709551615ull);
    ASSERT_THAT(uabs(uint128_t{18446744073709551615ull}) == 18446744073709551615ull);
    ASSERT_THAT(uabs(-(int128_t{1} << 126)) == uint128_t{1} << 126);
    ASSERT_THAT(uabs(int128_t{1} << 126) == uint128_t{1} << 126);
    ASSERT_THAT(uabs(uint128_t{1} << 126) == uint128_t{1} << 126);
    ASSERT_THAT(uabs(static_cast<int128_t>(-((uint128_t{1} << 127) - 1))) ==
                (uint128_t{1} << 127) - 1);
    ASSERT_THAT(uabs(static_cast<int128_t>((uint128_t{1} << 127) - 1)) ==
                (uint128_t{1} << 127) - 1);
    ASSERT_THAT(uabs((uint128_t{1} << 127) - 1) == (uint128_t{1} << 127) - 1);
    ASSERT_THAT(uabs(static_cast<int128_t>(-(uint128_t{1} << 127))) == uint128_t{1} << 127);
    ASSERT_THAT(uabs(static_cast<uint128_t>(-(uint128_t{1} << 127))) == uint128_t{1} << 127);
#endif

#if CONFIG_HAS_AT_LEAST_CXX_20

    ASSERT_THAT(sign(std::popcount(0u) - std::popcount(0u)) == sign(pop_cmp(0, 0)));
    ASSERT_THAT(sign(std::popcount(1u) - std::popcount(0u)) == sign(pop_cmp(1, 0)));
    ASSERT_THAT(sign(std::popcount(0u) - std::popcount(1u)) == sign(pop_cmp(0, 1)));
    ASSERT_THAT(sign(std::popcount(0xABCDEFu) - std::popcount(4u)) == pop_cmp(0xABCDEF, 4));
    ASSERT_THAT(sign(std::popcount(uint32_t(uint16_t(-1))) - std::popcount(314u)) ==
                sign(pop_cmp(uint16_t(-1), 314)));
    ASSERT_THAT(sign(std::popcount(uint32_t(-1)) - std::popcount(0u)) ==
                sign(pop_cmp(uint32_t(-1), 0)));
    ASSERT_THAT(sign(std::popcount(0u) - std::popcount(uint32_t(-1))) ==
                sign(pop_cmp(0, uint32_t(-1))));
    ASSERT_THAT(sign(std::popcount(uint32_t(-1)) - std::popcount(uint32_t(-1))) ==
                sign(pop_cmp(uint32_t(-1), uint32_t(-1))));
#endif

    ASSERT_THAT(detail::lz_count_32_software(0) == 32);
    ASSERT_THAT(detail::lz_count_32_software(1) == 31);
    ASSERT_THAT(detail::lz_count_32_software(2) == 30);
    ASSERT_THAT(detail::lz_count_32_software(4) == 29);
    ASSERT_THAT(detail::lz_count_32_software(8) == 28);
    ASSERT_THAT(detail::lz_count_32_software(12) == 28);
    ASSERT_THAT(detail::lz_count_32_software(16) == 27);
    ASSERT_THAT(detail::lz_count_32_software(32) == 26);
    ASSERT_THAT(detail::lz_count_32_software(48) == 26);
    ASSERT_THAT(detail::lz_count_32_software(uint32_t(1) << 30) == 1);
    ASSERT_THAT(detail::lz_count_32_software(uint32_t(1) << 31) == 0);
    ASSERT_THAT(detail::lz_count_32_software(~uint32_t(1)) == 0);

    ASSERT_THAT(detail::lz_count_64_software(0) == 64);
    ASSERT_THAT(detail::lz_count_64_software(1) == 63);
    ASSERT_THAT(detail::lz_count_64_software(2) == 62);
    ASSERT_THAT(detail::lz_count_64_software(4) == 61);
    ASSERT_THAT(detail::lz_count_64_software(8) == 60);
    ASSERT_THAT(detail::lz_count_64_software(12) == 60);
    ASSERT_THAT(detail::lz_count_64_software(16) == 59);
    ASSERT_THAT(detail::lz_count_64_software(32) == 58);
    ASSERT_THAT(detail::lz_count_64_software(48) == 58);
    ASSERT_THAT(detail::lz_count_64_software(uint32_t(1) << 30) == 33);
    ASSERT_THAT(detail::lz_count_64_software(uint32_t(1) << 31) == 32);
    ASSERT_THAT(detail::lz_count_64_software(~uint32_t(1)) == 32);
    ASSERT_THAT(detail::lz_count_64_software(uint64_t(1) << 62) == 1);
    ASSERT_THAT(detail::lz_count_64_software(uint64_t(1) << 63) == 0);
    ASSERT_THAT(detail::lz_count_64_software(uint64_t(-1)) == 0);

    ASSERT_THAT(detail::tz_count_32_software(0u) == 32);
    ASSERT_THAT(detail::tz_count_32_software(1u) == 0);
    ASSERT_THAT(detail::tz_count_32_software(2u) == 1);
    ASSERT_THAT(detail::tz_count_32_software(4u) == 2);
    ASSERT_THAT(detail::tz_count_32_software(8u) == 3);
    ASSERT_THAT(detail::tz_count_32_software(12u) == 2);
    ASSERT_THAT(detail::tz_count_32_software(16u) == 4);
    ASSERT_THAT(detail::tz_count_32_software(32u) == 5);
    ASSERT_THAT(detail::tz_count_32_software(48u) == 4);
    ASSERT_THAT(detail::tz_count_32_software(1u << 30) == 30);
    ASSERT_THAT(detail::tz_count_32_software(1u << 31) == 31);
    ASSERT_THAT(detail::tz_count_32_software(~1u) == 1);
    ASSERT_THAT(detail::tz_count_32_software(uint32_t(-1)) == 0);

    ASSERT_THAT(detail::tz_count_64_software(0u) == 64);
    ASSERT_THAT(detail::tz_count_64_software(1u) == 0);
    ASSERT_THAT(detail::tz_count_64_software(2u) == 1);
    ASSERT_THAT(detail::tz_count_64_software(4u) == 2);
    ASSERT_THAT(detail::tz_count_64_software(8u) == 3);
    ASSERT_THAT(detail::tz_count_64_software(12u) == 2);
    ASSERT_THAT(detail::tz_count_64_software(16u) == 4);
    ASSERT_THAT(detail::tz_count_64_software(32u) == 5);
    ASSERT_THAT(detail::tz_count_64_software(48u) == 4);
    ASSERT_THAT(detail::tz_count_64_software(1u << 30) == 30);
    ASSERT_THAT(detail::tz_count_64_software(1u << 31) == 31);
    ASSERT_THAT(detail::tz_count_64_software(~1u) == 1);
    ASSERT_THAT(detail::tz_count_64_software(uint32_t(-1)) == 0);

    ASSERT_THAT(next_n_bits_permutation(0) == 0);

    ASSERT_THAT(next_n_bits_permutation(0b0010011) == 0b0010101);
    ASSERT_THAT(next_n_bits_permutation(0b0010101) == 0b0010110);
    ASSERT_THAT(next_n_bits_permutation(0b0010110) == 0b0011001);
    ASSERT_THAT(next_n_bits_permutation(0b0011001) == 0b0011010);
    ASSERT_THAT(next_n_bits_permutation(0b0011010) == 0b0011100);
    ASSERT_THAT(next_n_bits_permutation(0b0011100) == 0b0100011);
    ASSERT_THAT(next_n_bits_permutation(0b0100011) == 0b0100101);
    ASSERT_THAT(next_n_bits_permutation(0b0100101) == 0b0100110);
    ASSERT_THAT(next_n_bits_permutation(0b0111000) == 0b1000011);
    ASSERT_THAT(next_n_bits_permutation(0b0110'0000'0000'0000'0000'0000'0000'0000u) ==
                0b1000'0000'0000'0000'0000'0000'0000'0001u);

    ASSERT_THAT(next_n_bits_permutation(0b01) == 0b10);

    ASSERT_THAT(next_n_bits_permutation(0b1111111) == 0b10111111);

    ASSERT_THAT(next_n_bits_permutation(1u << 0) == 1u << 1);
    ASSERT_THAT(next_n_bits_permutation(1u << 1) == 1u << 2);
    ASSERT_THAT(next_n_bits_permutation(1u << 2) == 1u << 3);
    ASSERT_THAT(next_n_bits_permutation(1u << 3) == 1u << 4);
    ASSERT_THAT(next_n_bits_permutation(1u << 4) == 1u << 5);
    ASSERT_THAT(next_n_bits_permutation(1u << 5) == 1u << 6);
    ASSERT_THAT(next_n_bits_permutation(1u << 6) == 1u << 7);
    ASSERT_THAT(next_n_bits_permutation(1u << 7) == 1u << 8);
    ASSERT_THAT(next_n_bits_permutation(1u << 8) == 1u << 9);
    ASSERT_THAT(next_n_bits_permutation(1u << 29) == 1u << 30);
    ASSERT_THAT(next_n_bits_permutation(1u << 30) == 1u << 31);

    ASSERT_THAT(!is_power_of_two(0ull));
    ASSERT_THAT(is_power_of_two(1ull << 0));
    ASSERT_THAT(is_power_of_two(1ull << 1));
    ASSERT_THAT(is_power_of_two(1ull << 2));
    ASSERT_THAT(is_power_of_two(1ull << 3));
    ASSERT_THAT(is_power_of_two(1ull << 4));
    ASSERT_THAT(is_power_of_two(1ull << 5));
    ASSERT_THAT(is_power_of_two(1ull << 6));
    ASSERT_THAT(is_power_of_two(1ull << 7));
    ASSERT_THAT(is_power_of_two(1ull << 8));
    ASSERT_THAT(is_power_of_two(1ull << 9));
    ASSERT_THAT(is_power_of_two(1ull << 60));
    ASSERT_THAT(is_power_of_two(1ull << 61));
    ASSERT_THAT(is_power_of_two(1ull << 62));
    ASSERT_THAT(is_power_of_two(1ull << 63));

#if defined(INTEGERS_128_BIT_HPP) && HAS_I128_CONSTEXPR

    ASSERT_THAT(!is_power_of_two(uint128_t{0}));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 0));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 1));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 2));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 3));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 4));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 5));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 6));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 7));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 8));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 9));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 60));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 61));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 62));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 63));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 64));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 65));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 127));
#endif

    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{0u}) == 1u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1u}) == 1u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{2u}) == 2u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{3u}) == 4u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{4u}) == 4u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{5u}) == 8u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{6u}) == 8u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{7u}) == 8u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{8u}) == 8u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{16u}) == 16u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{17u}) == 32u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{18u}) == 32u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{19u}) == 32u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{20u}) == 32u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{0x7FFFFFFFu}) == 0x80000000u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{0x80000000u}) == 0x80000000u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{0x80000001u}) == 0x100000000ull);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{0xFFFFFFFFu}) == 0x100000000ull);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 0) == uint32_t{1} << 0);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 1) == uint32_t{1} << 1);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 2) == uint32_t{1} << 2);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 3) == uint32_t{1} << 3);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 4) == uint32_t{1} << 4);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 5) == uint32_t{1} << 5);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 6) == uint32_t{1} << 6);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 7) == uint32_t{1} << 7);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 8) == uint32_t{1} << 8);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 9) == uint32_t{1} << 9);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 10) == uint32_t{1} << 10);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 11) == uint32_t{1} << 11);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 12) == uint32_t{1} << 12);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 13) == uint32_t{1} << 13);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 14) == uint32_t{1} << 14);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 15) == uint32_t{1} << 15);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 16) == uint32_t{1} << 16);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 17) == uint32_t{1} << 17);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 18) == uint32_t{1} << 18);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 19) == uint32_t{1} << 19);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 20) == uint32_t{1} << 20);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 21) == uint32_t{1} << 21);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 22) == uint32_t{1} << 22);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 23) == uint32_t{1} << 23);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 24) == uint32_t{1} << 24);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 25) == uint32_t{1} << 25);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 26) == uint32_t{1} << 26);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 27) == uint32_t{1} << 27);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 28) == uint32_t{1} << 28);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 29) == uint32_t{1} << 29);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 30) == uint32_t{1} << 30);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 31) == uint32_t{1} << 31);

    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{0u}) == 1u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1u}) == 1u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{2u}) == 2u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{3u}) == 4u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{4u}) == 4u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{5u}) == 8u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{6u}) == 8u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{7u}) == 8u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{8u}) == 8u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{16u}) == 16u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{17u}) == 32u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{18u}) == 32u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{19u}) == 32u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{20u}) == 32u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{0x7FFFFFFFu}) == 0x80000000u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{0x80000000u}) == 0x80000000u);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{0x80000001u}) == 0x100000000ull);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{0xFFFFFFFFu}) == 0x100000000ull);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{0x7FFFFFFFFFFFFFFFull}) ==
                0x8000000000000000ull);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{0x8000000000000000ull}) ==
                0x8000000000000000ull);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 0) == uint64_t{1} << 0);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 1) == uint64_t{1} << 1);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 2) == uint64_t{1} << 2);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 3) == uint64_t{1} << 3);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 4) == uint64_t{1} << 4);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 5) == uint64_t{1} << 5);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 6) == uint64_t{1} << 6);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 7) == uint64_t{1} << 7);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 8) == uint64_t{1} << 8);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 9) == uint64_t{1} << 9);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 10) == uint64_t{1} << 10);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 11) == uint64_t{1} << 11);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 12) == uint64_t{1} << 12);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 13) == uint64_t{1} << 13);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 14) == uint64_t{1} << 14);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 15) == uint64_t{1} << 15);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 16) == uint64_t{1} << 16);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 17) == uint64_t{1} << 17);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 18) == uint64_t{1} << 18);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 19) == uint64_t{1} << 19);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 20) == uint64_t{1} << 20);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 21) == uint64_t{1} << 21);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 22) == uint64_t{1} << 22);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 23) == uint64_t{1} << 23);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 24) == uint64_t{1} << 24);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 25) == uint64_t{1} << 25);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 26) == uint64_t{1} << 26);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 27) == uint64_t{1} << 27);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 28) == uint64_t{1} << 28);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 29) == uint64_t{1} << 29);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 30) == uint64_t{1} << 30);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 31) == uint64_t{1} << 31);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 32) == uint64_t{1} << 32);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 33) == uint64_t{1} << 33);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 34) == uint64_t{1} << 34);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 35) == uint64_t{1} << 35);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 36) == uint64_t{1} << 36);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 37) == uint64_t{1} << 37);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 38) == uint64_t{1} << 38);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 39) == uint64_t{1} << 39);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 40) == uint64_t{1} << 40);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 41) == uint64_t{1} << 41);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 42) == uint64_t{1} << 42);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 43) == uint64_t{1} << 43);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 44) == uint64_t{1} << 44);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 45) == uint64_t{1} << 45);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 46) == uint64_t{1} << 46);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 47) == uint64_t{1} << 47);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 48) == uint64_t{1} << 48);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 49) == uint64_t{1} << 49);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 50) == uint64_t{1} << 50);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 51) == uint64_t{1} << 51);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 52) == uint64_t{1} << 52);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 53) == uint64_t{1} << 53);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 54) == uint64_t{1} << 54);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 55) == uint64_t{1} << 55);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 56) == uint64_t{1} << 56);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 57) == uint64_t{1} << 57);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 58) == uint64_t{1} << 58);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 59) == uint64_t{1} << 59);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 60) == uint64_t{1} << 60);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 61) == uint64_t{1} << 61);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 62) == uint64_t{1} << 62);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 63) == uint64_t{1} << 63);

    ASSERT_THAT(least_bit_set(0b0) == 0b0);
    ASSERT_THAT(least_bit_set(0b1) == 0b1);
    ASSERT_THAT(least_bit_set(0b10) == 0b10);
    ASSERT_THAT(least_bit_set(0b100) == 0b100);
    ASSERT_THAT(least_bit_set(0b1000) == 0b1000);
    ASSERT_THAT(least_bit_set(0b10000) == 0b10000);
    ASSERT_THAT(least_bit_set(0b100000) == 0b100000);
    ASSERT_THAT(least_bit_set(0b1000000) == 0b1000000);
    ASSERT_THAT(least_bit_set(0b10000000) == 0b10000000);
    ASSERT_THAT(least_bit_set(0b0u) == 0b0u);
    ASSERT_THAT(least_bit_set(0b1u) == 0b1u);
    ASSERT_THAT(least_bit_set(0b10u) == 0b10u);
    ASSERT_THAT(least_bit_set(0b100u) == 0b100u);
    ASSERT_THAT(least_bit_set(0b1000u) == 0b1000u);
    ASSERT_THAT(least_bit_set(0b10000u) == 0b10000u);
    ASSERT_THAT(least_bit_set(0b100000u) == 0b100000u);
    ASSERT_THAT(least_bit_set(0b1000000u) == 0b1000000u);
    ASSERT_THAT(least_bit_set(0b10000000u) == 0b10000000u);
    ASSERT_THAT(least_bit_set(static_cast<int8_t>(0b0)) == static_cast<int8_t>(0b0));
    ASSERT_THAT(least_bit_set(static_cast<int8_t>(0b1)) == static_cast<int8_t>(0b1));
    ASSERT_THAT(least_bit_set(static_cast<int8_t>(0b10)) == static_cast<int8_t>(0b10));
    ASSERT_THAT(least_bit_set(static_cast<int8_t>(0b100)) == static_cast<int8_t>(0b100));
    ASSERT_THAT(least_bit_set(static_cast<int8_t>(0b1000)) == static_cast<int8_t>(0b1000));
    ASSERT_THAT(least_bit_set(static_cast<int8_t>(0b10000)) == static_cast<int8_t>(0b10000));
    ASSERT_THAT(least_bit_set(static_cast<int8_t>(0b100000)) == static_cast<int8_t>(0b100000));
    ASSERT_THAT(least_bit_set(static_cast<int8_t>(0b1000000)) == static_cast<int8_t>(0b1000000));
    ASSERT_THAT(least_bit_set(static_cast<int8_t>(0b10000000)) == static_cast<int8_t>(0b10000000));
    ASSERT_THAT(least_bit_set(uint8_t(0b0)) == uint8_t(0b0));
    ASSERT_THAT(least_bit_set(uint8_t(0b1)) == uint8_t(0b1));
    ASSERT_THAT(least_bit_set(uint8_t(0b10)) == uint8_t(0b10));
    ASSERT_THAT(least_bit_set(uint8_t(0b100)) == uint8_t(0b100));
    ASSERT_THAT(least_bit_set(uint8_t(0b1000)) == uint8_t(0b1000));
    ASSERT_THAT(least_bit_set(uint8_t(0b10000)) == uint8_t(0b10000));
    ASSERT_THAT(least_bit_set(uint8_t(0b100000)) == uint8_t(0b100000));
    ASSERT_THAT(least_bit_set(uint8_t(0b1000000)) == uint8_t(0b1000000));
    ASSERT_THAT(least_bit_set(uint8_t(0b10000000)) == uint8_t(0b10000000));
    ASSERT_THAT(least_bit_set(0b100000000) == 0b100000000);
    ASSERT_THAT(least_bit_set(0b1000000000) == 0b1000000000);
    ASSERT_THAT(least_bit_set(0b10000000000) == 0b10000000000);
    ASSERT_THAT(least_bit_set(0b100000000u) == 0b100000000u);
    ASSERT_THAT(least_bit_set(0b1000000000u) == 0b1000000000u);
    ASSERT_THAT(least_bit_set(0b10000000000u) == 0b10000000000u);
    ASSERT_THAT(
        least_bit_set(0b1000000000000000000000000000000000000000000000000000000000000000ull) ==
        0b1000000000000000000000000000000000000000000000000000000000000000ull);
    ASSERT_THAT(least_bit_set(0b110101010101010101011001) == 0b1);
    ASSERT_THAT(least_bit_set(0b1010101011001101011100010100000ll) == 0b100000ll);
    ASSERT_THAT(least_bit_set(0b1010111001010101101010110101001101011100110011000ll) == 0b1000ll);
    ASSERT_THAT(least_bit_set(0b110101010101010101011001u) == 0b1u);
    ASSERT_THAT(least_bit_set(0b1010101011001101011100010100000llu) == 0b100000llu);
    ASSERT_THAT(least_bit_set(0b1010111001010101101010110101001101011100110011000llu) == 0b1000llu);

    ASSERT_THAT(log2_floor(uint32_t{0}) == static_cast<uint32_t>(-1));
    ASSERT_THAT(log2_floor(uint32_t{1}) == 0);
    ASSERT_THAT(log2_floor(uint32_t{2}) == 1);
    ASSERT_THAT(log2_floor(uint32_t{4}) == 2);
    ASSERT_THAT(log2_floor(uint32_t{8}) == 3);
    ASSERT_THAT(log2_floor(uint32_t{9}) == 3);
    ASSERT_THAT(log2_floor(uint32_t{10}) == 3);
    ASSERT_THAT(log2_floor(uint32_t{15}) == 3);
    ASSERT_THAT(log2_floor(uint32_t{16}) == 4);
    ASSERT_THAT(log2_floor(uint32_t{99}) == 6);
    ASSERT_THAT(log2_floor(uint32_t{100}) == 6);
    ASSERT_THAT(log2_floor(uint32_t{127}) == 6);
    ASSERT_THAT(log2_floor(uint32_t{128}) == 7);
    ASSERT_THAT(log2_floor(uint32_t{129}) == 7);
    ASSERT_THAT(log2_floor(static_cast<uint32_t>(-1)) == 31);

    ASSERT_THAT(log2_ceil(uint32_t{0}) == static_cast<uint32_t>(-1));
    ASSERT_THAT(log2_ceil(uint32_t{1}) == 0);
    ASSERT_THAT(log2_ceil(uint32_t{2}) == 1);
    ASSERT_THAT(log2_ceil(uint32_t{4}) == 2);
    ASSERT_THAT(log2_ceil(uint32_t{8}) == 3);
    ASSERT_THAT(log2_ceil(uint32_t{9}) == 4);
    ASSERT_THAT(log2_ceil(uint32_t{10}) == 4);
    ASSERT_THAT(log2_ceil(uint32_t{15}) == 4);
    ASSERT_THAT(log2_ceil(uint32_t{16}) == 4);
    ASSERT_THAT(log2_ceil(uint32_t{99}) == 7);
    ASSERT_THAT(log2_ceil(uint32_t{100}) == 7);
    ASSERT_THAT(log2_ceil(uint32_t{127}) == 7);
    ASSERT_THAT(log2_ceil(uint32_t{128}) == 7);
    ASSERT_THAT(log2_ceil(uint32_t{129}) == 8);
    ASSERT_THAT(log2_ceil(static_cast<uint32_t>(-1)) == 32);

    LOG10_ASSERT_THAT(log10_floor(uint32_t{0}) == static_cast<uint32_t>(-1));
    LOG10_ASSERT_THAT(log10_floor(uint32_t{1}) == 0);
    LOG10_ASSERT_THAT(log10_floor(uint32_t{9}) == 0);
    LOG10_ASSERT_THAT(log10_floor(uint32_t{10}) == 1);
    LOG10_ASSERT_THAT(log10_floor(uint32_t{11}) == 1);
    LOG10_ASSERT_THAT(log10_floor(uint32_t{99}) == 1);
    LOG10_ASSERT_THAT(log10_floor(uint32_t{100}) == 2);
    LOG10_ASSERT_THAT(log10_floor(uint32_t{101}) == 2);
    LOG10_ASSERT_THAT(log10_floor(uint32_t{1000000000}) == 9);
    LOG10_ASSERT_THAT(log10_floor(uint32_t{2000000000}) == 9);
    LOG10_ASSERT_THAT(log10_floor(uint32_t{4294967294u}) == 9);
    LOG10_ASSERT_THAT(log10_floor(static_cast<uint32_t>(1e8)) == 8);
    LOG10_ASSERT_THAT(log10_floor(static_cast<uint32_t>(1e9)) == 9);
    LOG10_ASSERT_THAT(log10_floor(static_cast<uint32_t>(-1)) == 9);

    LOG10_ASSERT_THAT(log10_floor(uint64_t{0}) == static_cast<uint32_t>(-1));
    LOG10_ASSERT_THAT(log10_floor(uint64_t{1}) == 0);
    LOG10_ASSERT_THAT(log10_floor(uint64_t{9}) == 0);
    LOG10_ASSERT_THAT(log10_floor(uint64_t{10}) == 1);
    LOG10_ASSERT_THAT(log10_floor(uint64_t{11}) == 1);
    LOG10_ASSERT_THAT(log10_floor(uint64_t{99}) == 1);
    LOG10_ASSERT_THAT(log10_floor(uint64_t{100}) == 2);
    LOG10_ASSERT_THAT(log10_floor(uint64_t{101}) == 2);
    LOG10_ASSERT_THAT(log10_floor(static_cast<uint64_t>(1e8)) == 8);
    LOG10_ASSERT_THAT(log10_floor(static_cast<uint64_t>(1e9)) == 9);
    LOG10_ASSERT_THAT(log10_floor(static_cast<uint64_t>(1e18)) == 18);
    LOG10_ASSERT_THAT(log10_floor(static_cast<uint64_t>(1e19)) == 19);
    LOG10_ASSERT_THAT(log10_floor(static_cast<uint64_t>(-1)) == 19);

    LOG10_ASSERT_THAT(base_10_len(uint32_t{0}) == 1);
    LOG10_ASSERT_THAT(base_10_len(uint32_t{1}) == 1);
    LOG10_ASSERT_THAT(base_10_len(uint32_t{9}) == 1);
    LOG10_ASSERT_THAT(base_10_len(uint32_t{10}) == 2);
    LOG10_ASSERT_THAT(base_10_len(uint32_t{11}) == 2);
    LOG10_ASSERT_THAT(base_10_len(uint32_t{99}) == 2);
    LOG10_ASSERT_THAT(base_10_len(uint32_t{100}) == 3);
    LOG10_ASSERT_THAT(base_10_len(uint32_t{101}) == 3);
    LOG10_ASSERT_THAT(base_10_len(uint32_t{1000000000}) == 10);
    LOG10_ASSERT_THAT(base_10_len(uint32_t{2000000000}) == 10);
    LOG10_ASSERT_THAT(base_10_len(uint32_t{4294967294u}) == 10);
    LOG10_ASSERT_THAT(base_10_len(static_cast<uint32_t>(1e8)) == 9);
    LOG10_ASSERT_THAT(base_10_len(static_cast<uint32_t>(1e9)) == 10);
    LOG10_ASSERT_THAT(base_10_len(static_cast<uint32_t>(-1)) == 10);

    LOG10_ASSERT_THAT(base_10_len(uint64_t{0}) == 1);
    LOG10_ASSERT_THAT(base_10_len(uint64_t{1}) == 1);
    LOG10_ASSERT_THAT(base_10_len(uint64_t{9}) == 1);
    LOG10_ASSERT_THAT(base_10_len(uint64_t{10}) == 2);
    LOG10_ASSERT_THAT(base_10_len(uint64_t{11}) == 2);
    LOG10_ASSERT_THAT(base_10_len(uint64_t{99}) == 2);
    LOG10_ASSERT_THAT(base_10_len(uint64_t{100}) == 3);
    LOG10_ASSERT_THAT(base_10_len(uint64_t{101}) == 3);
    LOG10_ASSERT_THAT(base_10_len(uint64_t{1000000000}) == 10);
    LOG10_ASSERT_THAT(base_10_len(uint64_t{2000000000}) == 10);
    LOG10_ASSERT_THAT(base_10_len(uint64_t{4294967294u}) == 10);
    LOG10_ASSERT_THAT(base_10_len(static_cast<uint64_t>(1e8)) == 9);
    LOG10_ASSERT_THAT(base_10_len(static_cast<uint64_t>(1e9)) == 10);
    LOG10_ASSERT_THAT(base_10_len(static_cast<uint64_t>(1e18)) == 19);
    LOG10_ASSERT_THAT(base_10_len(static_cast<uint64_t>(1e19)) == 20);
    LOG10_ASSERT_THAT(base_10_len(static_cast<uint64_t>(-1)) == 20);

#if CONFIG_HAS_AT_LEAST_CXX_17

    ASSERT_THAT(base_b_len(0u) == 1);
    ASSERT_THAT(base_b_len(1u) == 1);
    ASSERT_THAT(base_b_len(9u) == 1);
    ASSERT_THAT(base_b_len(10u) == 2);
    ASSERT_THAT(base_b_len(11u) == 2);
    ASSERT_THAT(base_b_len(99u) == 2);
    ASSERT_THAT(base_b_len(100u) == 3);
    ASSERT_THAT(base_b_len(101u) == 3);
    ASSERT_THAT(base_b_len(uint32_t(-1)) == 10);

    ASSERT_THAT(base_b_len(0) == 1);
    ASSERT_THAT(base_b_len(1) == 1);
    ASSERT_THAT(base_b_len(9) == 1);
    ASSERT_THAT(base_b_len(10) == 2);
    ASSERT_THAT(base_b_len(11) == 2);
    ASSERT_THAT(base_b_len(99) == 2);
    ASSERT_THAT(base_b_len(100) == 3);
    ASSERT_THAT(base_b_len(101) == 3);
    ASSERT_THAT(base_b_len(-0) == 1);
    ASSERT_THAT(base_b_len(-1) == 2);
    ASSERT_THAT(base_b_len(-9) == 2);
    ASSERT_THAT(base_b_len(-10) == 3);
    ASSERT_THAT(base_b_len(-11) == 3);
    ASSERT_THAT(base_b_len(-99) == 3);
    ASSERT_THAT(base_b_len(-100) == 4);
    ASSERT_THAT(base_b_len(-101) == 4);
    ASSERT_THAT(base_b_len(int32_t(uint32_t{1} << 31)) == 11);

    ASSERT_THAT(base_b_len(0ull) == 1);
    ASSERT_THAT(base_b_len(1ull) == 1);
    ASSERT_THAT(base_b_len(9ull) == 1);
    ASSERT_THAT(base_b_len(10ull) == 2);
    ASSERT_THAT(base_b_len(11ull) == 2);
    ASSERT_THAT(base_b_len(99ull) == 2);
    ASSERT_THAT(base_b_len(100ull) == 3);
    ASSERT_THAT(base_b_len(101ull) == 3);
    ASSERT_THAT(base_b_len(static_cast<uint64_t>(-1)) == 20);

    ASSERT_THAT(base_b_len(0ll) == 1);
    ASSERT_THAT(base_b_len(1ll) == 1);
    ASSERT_THAT(base_b_len(9ll) == 1);
    ASSERT_THAT(base_b_len(10ll) == 2);
    ASSERT_THAT(base_b_len(11ll) == 2);
    ASSERT_THAT(base_b_len(99ll) == 2);
    ASSERT_THAT(base_b_len(100ll) == 3);
    ASSERT_THAT(base_b_len(101ll) == 3);
    ASSERT_THAT(base_b_len(-0ll) == 1);
    ASSERT_THAT(base_b_len(-1ll) == 2);
    ASSERT_THAT(base_b_len(-9ll) == 2);
    ASSERT_THAT(base_b_len(-10ll) == 3);
    ASSERT_THAT(base_b_len(-11ll) == 3);
    ASSERT_THAT(base_b_len(-99ll) == 3);
    ASSERT_THAT(base_b_len(-100ll) == 4);
    ASSERT_THAT(base_b_len(-101ll) == 4);
    ASSERT_THAT(base_b_len(std::numeric_limits<int64_t>::min()) == 20);

#if defined(INTEGERS_128_BIT_HPP) && HAS_I128_CONSTEXPR

    ASSERT_THAT(base_b_len(uint128_t{0}) == 1);
    ASSERT_THAT(base_b_len(uint128_t{1}) == 1);
    ASSERT_THAT(base_b_len(uint128_t{9}) == 1);
    ASSERT_THAT(base_b_len(uint128_t{10}) == 2);
    ASSERT_THAT(base_b_len(uint128_t{11}) == 2);
    ASSERT_THAT(base_b_len(uint128_t{99}) == 2);
    ASSERT_THAT(base_b_len(uint128_t{100}) == 3);
    ASSERT_THAT(base_b_len(uint128_t{101}) == 3);
    ASSERT_THAT(base_b_len(uint128_t(-1)) == 39);

    ASSERT_THAT(base_b_len(int128_t{0}) == 1);
    ASSERT_THAT(base_b_len(int128_t{1}) == 1);
    ASSERT_THAT(base_b_len(int128_t{9}) == 1);
    ASSERT_THAT(base_b_len(int128_t{10}) == 2);
    ASSERT_THAT(base_b_len(int128_t{11}) == 2);
    ASSERT_THAT(base_b_len(int128_t{99}) == 2);
    ASSERT_THAT(base_b_len(int128_t{100}) == 3);
    ASSERT_THAT(base_b_len(int128_t{101}) == 3);
    ASSERT_THAT(base_b_len(-int128_t{0}) == 1);
    ASSERT_THAT(base_b_len(-int128_t{1}) == 2);
    ASSERT_THAT(base_b_len(-int128_t{9}) == 2);
    ASSERT_THAT(base_b_len(-int128_t{10}) == 3);
    ASSERT_THAT(base_b_len(-int128_t{11}) == 3);
    ASSERT_THAT(base_b_len(-int128_t{99}) == 3);
    ASSERT_THAT(base_b_len(-int128_t{100}) == 4);
    ASSERT_THAT(base_b_len(-int128_t{101}) == 4);
    ASSERT_THAT(base_b_len(static_cast<int128_t>(uint128_t{1} << 127)) == 40);

#endif  // INTEGERS_128_BIT_HPP

#endif  // CONFIG_HAS_AT_LEAST_CXX_17

#if defined(HAS_I128_CONSTEXPR) && HAS_I128_CONSTEXPR

    ASSERT_THAT(gcd(uint128_t{1}, uint128_t{1}) == 1);
    ASSERT_THAT(gcd(uint128_t{3}, uint128_t{7}) == 1);
    ASSERT_THAT(gcd(uint128_t{0}, uint128_t{112378432}) == 112378432);
    ASSERT_THAT(gcd(uint128_t{112378432}, uint128_t{0}) == 112378432);
    ASSERT_THAT(gcd(uint128_t{429384832}, uint128_t{324884}) == 4);
    ASSERT_THAT(gcd(uint128_t{18446744073709551521ull}, uint128_t{18446744073709551533ull}) == 1);
    ASSERT_THAT(gcd(uint128_t{18446744073709551521ull} * 18446744073709551521ull,
                    uint128_t{18446744073709551521ull}) == 18446744073709551521ull);
    ASSERT_THAT(gcd(uint128_t{23999993441ull} * 23999993377ull,
                    uint128_t{23999992931ull} * 23999539633ull) == 1);
    ASSERT_THAT(gcd(uint128_t{2146514599u} * 2146514603u * 2146514611u,
                    uint128_t{2146514611u} * 2146514621u * 2146514647u) == 2146514611ull);
    ASSERT_THAT(gcd(uint128_t{2146514599u} * 2146514603u * 2146514611u * 2,
                    uint128_t{2146514599u} * 2146514603u * 2146514611u * 3) ==
                uint128_t{2146514599u} * 2146514603u * 2146514611u);
    ASSERT_THAT(gcd(uint128_t{100000000000000003ull} * 1000000000000000003ull,
                    uint128_t{1000000000000000003ull} * 1000000000000000009ull) ==
                1000000000000000003ull);
    ASSERT_THAT(gcd(uint128_t{3u * 2 * 5 * 7 * 11 * 13 * 17 * 19},
                    uint128_t{18446744073709551557ull} * 3) == 3);
    ASSERT_THAT(gcd(uint128_t{1000000000000000009ull},
                    uint128_t{1000000000000000009ull} * 1000000000000000009ull) ==
                1000000000000000009ull);
    ASSERT_THAT(gcd(uint128_t{0}, uint128_t{1000000000000000009ull} * 1000000000000000009ull) ==
                uint128_t{1000000000000000009ull} * 1000000000000000009ull);
    ASSERT_THAT(gcd(uint128_t{18446744073709551557ull}, uint128_t(0)) == 18446744073709551557ull);

    ASSERT_THAT(gcd(uint64_t{2}, int128_t{4}) == 2);
    ASSERT_THAT(gcd(uint64_t{2}, int128_t{-4}) == 2);
    ASSERT_THAT(gcd(uint64_t{3}, int128_t{7}) == 1);
    ASSERT_THAT(gcd(uint64_t{3}, int128_t{-7}) == 1);
    ASSERT_THAT(gcd(uint64_t{3}, int128_t{18446744073709551557ull} * 3) == 3);
    ASSERT_THAT(gcd(uint64_t{3}, int128_t{18446744073709551557ull} * (-3)) == 3);
    ASSERT_THAT(gcd(uint64_t{3} * 2 * 5 * 7 * 11 * 13 * 17 * 19,
                    int128_t{18446744073709551557ull} * 3) == 3);
    ASSERT_THAT(gcd(uint64_t{1000000000000000009ull},
                    int128_t{1000000000000000009ll} * 1000000000000000009ll) ==
                1000000000000000009ull);
    ASSERT_THAT(gcd(uint64_t{0}, int128_t{1000000000000000009ll} * 1000000000000000009ll) ==
                uint128_t{1000000000000000009ll} * 1000000000000000009ull);
    ASSERT_THAT(gcd(uint64_t{18446744073709551557ull}, int128_t{0}) == 18446744073709551557ull);

    ASSERT_THAT(math_functions::popcount(0u) == 0);
    ASSERT_THAT(math_functions::popcount(1u << 1) == 1);
    ASSERT_THAT(math_functions::popcount(1u << 2) == 1);
    ASSERT_THAT(math_functions::popcount(1u << 3) == 1);
    ASSERT_THAT(math_functions::popcount(1u << 4) == 1);
    ASSERT_THAT(math_functions::popcount(1u << 5) == 1);
    ASSERT_THAT(math_functions::popcount(1u << 6) == 1);
    ASSERT_THAT(math_functions::popcount(1u << 7) == 1);
    ASSERT_THAT(math_functions::popcount(1u << 8) == 1);
    ASSERT_THAT(math_functions::popcount(1u << 9) == 1);
    ASSERT_THAT(math_functions::popcount(1u << 10) == 1);
    ASSERT_THAT(math_functions::popcount(1u << 12) == 1);
    ASSERT_THAT(math_functions::popcount(1u << 13) == 1);
    ASSERT_THAT(math_functions::popcount(1u << 14) == 1);
    ASSERT_THAT(math_functions::popcount(1u << 30) == 1);
    ASSERT_THAT(math_functions::popcount(1u << 31) == 1);

    ASSERT_THAT(math_functions::popcount(0ul) == 0);
    ASSERT_THAT(math_functions::popcount(1ul << 1) == 1);
    ASSERT_THAT(math_functions::popcount(1ul << 2) == 1);
    ASSERT_THAT(math_functions::popcount(1ul << 3) == 1);
    ASSERT_THAT(math_functions::popcount(1ul << 4) == 1);
    ASSERT_THAT(math_functions::popcount(1ul << 5) == 1);
    ASSERT_THAT(math_functions::popcount(1ul << 6) == 1);
    ASSERT_THAT(math_functions::popcount(1ul << 7) == 1);
    ASSERT_THAT(math_functions::popcount(1ul << 8) == 1);
    ASSERT_THAT(math_functions::popcount(1ul << 9) == 1);
    ASSERT_THAT(math_functions::popcount(1ul << 10) == 1);
    ASSERT_THAT(math_functions::popcount(1ul << 12) == 1);
    ASSERT_THAT(math_functions::popcount(1ul << 13) == 1);
    ASSERT_THAT(math_functions::popcount(1ul << 14) == 1);
    ASSERT_THAT(math_functions::popcount(1ul << 30) == 1);
    ASSERT_THAT(math_functions::popcount(1ul << 31) == 1);

    ASSERT_THAT(math_functions::popcount(0ull) == 0);
    ASSERT_THAT(math_functions::popcount(1ull << 1) == 1);
    ASSERT_THAT(math_functions::popcount(1ull << 2) == 1);
    ASSERT_THAT(math_functions::popcount(1ull << 3) == 1);
    ASSERT_THAT(math_functions::popcount(1ull << 4) == 1);
    ASSERT_THAT(math_functions::popcount(1ull << 5) == 1);
    ASSERT_THAT(math_functions::popcount(1ull << 6) == 1);
    ASSERT_THAT(math_functions::popcount(1ull << 7) == 1);
    ASSERT_THAT(math_functions::popcount(1ull << 8) == 1);
    ASSERT_THAT(math_functions::popcount(1ull << 9) == 1);
    ASSERT_THAT(math_functions::popcount(1ull << 10) == 1);
    ASSERT_THAT(math_functions::popcount(1ull << 12) == 1);
    ASSERT_THAT(math_functions::popcount(1ull << 13) == 1);
    ASSERT_THAT(math_functions::popcount(1ull << 14) == 1);
    ASSERT_THAT(math_functions::popcount(1ull << 30) == 1);
    ASSERT_THAT(math_functions::popcount(1ull << 31) == 1);
    ASSERT_THAT(math_functions::popcount(1ull << 62) == 1);
    ASSERT_THAT(math_functions::popcount(1ull << 63) == 1);

    ASSERT_THAT(!math_functions::bool_median(false, false, false));
    ASSERT_THAT(!math_functions::bool_median(false, false, true));
    ASSERT_THAT(!math_functions::bool_median(false, true, false));
    ASSERT_THAT(math_functions::bool_median(false, true, true));
    ASSERT_THAT(!math_functions::bool_median(true, false, false));
    ASSERT_THAT(math_functions::bool_median(true, false, true));
    ASSERT_THAT(math_functions::bool_median(true, true, false));
    ASSERT_THAT(math_functions::bool_median(false, true, true));

#endif  // defined(HAS_I128_CONSTEXPR) && HAS_I128_CONSTEXPR

    ASSERT_THAT(([]() constexpr noexcept {
        const auto [q, r] = math_functions::extract_pow2(uint32_t{0});
        return q == 0 && r == 32;
    }()));
    ASSERT_THAT(([]() constexpr noexcept {
        const auto [q, r] = math_functions::extract_pow2(uint32_t{1} << 31);
        return q == 1 && r == 31;
    }()));
    ASSERT_THAT(([]() constexpr noexcept {
        const auto [q, r] = math_functions::extract_pow2(uint64_t{0});
        return q == 0 && r == 64;
    }()));
    ASSERT_THAT(([]() constexpr noexcept {
        const auto [q, r] = math_functions::extract_pow2(uint64_t{1} << 31);
        return q == 1 && r == 31;
    }()));
    ASSERT_THAT(([]() constexpr noexcept {
        const auto [q, r] = math_functions::extract_pow2(uint64_t{1} << 63);
        return q == 1 && r == 63;
    }()));
    ASSERT_THAT(([]() constexpr noexcept {
        const auto [q, r] = math_functions::extract_pow2(uint64_t{9221685055305285632ull});
        return q == 2147090867 && r == 32;
    }()));
    ASSERT_THAT(([]() constexpr noexcept {
        const auto [q, r] = math_functions::extract_pow2(uint64_t{4610842527652642816ull});
        return q == 2147090867 && r == 31;
    }()));

    ASSERT_THAT((math_functions::powers_sum_u64<0>(100) == 100u));
    ASSERT_THAT((math_functions::powers_sum_u64<1>(100) == 100u * (100u + 1) / 2));
    ASSERT_THAT((math_functions::powers_sum_u64<2>(100) == 100u * (100u + 1) * (2 * 100u + 1) / 6));
    ASSERT_THAT(
        (math_functions::powers_sum_u64<3>(100) == 100u * 100u * (100u + 1) * (100u + 1) / 4));

#if defined(HAS_I128_CONSTEXPR) && HAS_I128_CONSTEXPR

    ASSERT_THAT((math_functions::powers_sum_u128<0>(100) == 100u));
    ASSERT_THAT((math_functions::powers_sum_u128<1>(100) == 100u * (100u + 1) / 2));
    ASSERT_THAT(
        (math_functions::powers_sum_u128<2>(100) == 100u * (100u + 1) * (2 * 100u + 1) / 6));
    ASSERT_THAT(
        (math_functions::powers_sum_u128<3>(100) == 100u * 100u * (100u + 1) * (100u + 1) / 4));

    constexpr auto kN    = static_cast<uint32_t>(3e9);
    constexpr auto kN128 = static_cast<uint128_t>(kN);
    ASSERT_THAT(
        (math_functions::powers_sum_u128<3>(kN) == kN128 * kN128 * (kN128 + 1) * (kN128 + 1) / 4));

#endif

    ASSERT_THAT(next_even(0u) == 2);
    ASSERT_THAT(next_even(1u) == 2);
    ASSERT_THAT(next_even(2u) == 4);
    ASSERT_THAT(next_even(0ul) == 2);
    ASSERT_THAT(next_even(1ul) == 2);
    ASSERT_THAT(next_even(2ul) == 4);
    ASSERT_THAT(next_even(0ull) == 2);
    ASSERT_THAT(next_even(1ull) == 2);
    ASSERT_THAT(next_even(2ull) == 4);
#ifdef INTEGERS_128_BIT_HPP
    ASSERT_THAT(next_even(uint128_t{0}) == 2);
    ASSERT_THAT(next_even(uint128_t{1}) == 2);
    ASSERT_THAT(next_even(uint128_t{2}) == 4);
#endif

    constexpr auto kMaxU32 = std::numeric_limits<uint32_t>::max();
    ASSERT_THAT(next_even(kMaxU32 - 3) == kMaxU32 - 1);
    ASSERT_THAT(next_even(kMaxU32 - 2) == kMaxU32 - 1);
    ASSERT_THAT(next_even(kMaxU32 - 1) == 0);
    ASSERT_THAT(next_even(kMaxU32) == 0);

    constexpr auto kMaxU64 = std::numeric_limits<uint64_t>::max();
    ASSERT_THAT(next_even(kMaxU64 - 3) == kMaxU64 - 1);
    ASSERT_THAT(next_even(kMaxU64 - 2) == kMaxU64 - 1);
    ASSERT_THAT(next_even(kMaxU64 - 1) == 0);
    ASSERT_THAT(next_even(kMaxU64 - 0) == 0);

#ifdef INTEGERS_128_BIT_HPP
    constexpr auto kMaxU128 = static_cast<uint128_t>(-1);
    ASSERT_THAT(next_even(kMaxU128 - 3) == kMaxU128 - 1);
    ASSERT_THAT(next_even(kMaxU128 - 2) == kMaxU128 - 1);
    ASSERT_THAT(next_even(kMaxU128 - 1) == 0);
    ASSERT_THAT(next_even(kMaxU128 - 0) == 0);
#endif

    ASSERT_THAT(solve_binary_congruence_modulo_m(24u, 1u << 24, 43284u) == 1);
    ASSERT_THAT(solve_binary_congruence_modulo_m(24u, 1u << 24, 39483924u) == 1);
    ASSERT_THAT(solve_binary_congruence_modulo_m(30u, 1u << 30, 6237443u) == 1);
    ASSERT_THAT(solve_binary_congruence_modulo_m(28u, 1u << 30, 6237443u) == 4);

#undef STRINGIFY
#undef ASSERT_THAT
#undef LOG10_ASSERT_THAT
}

void test_arange() {
    log_tests_started();

    using std::vector;

    assert((arange(2, 5, 1) == vector{2, 3, 4}));
    assert((arange(1, 5, 2) == vector{1, 3}));
    assert((arange(-9, 10, 5) == vector{-9, -4, 1, 6}));
    assert((arange(5, 2, -1) == vector{5, 4, 3}));
    assert((arange(5, 1, -2) == vector{5, 3}));
    assert((arange(7, 6, -3) == vector{7}));
    assert((arange(0, -1, 2).empty()));
    assert((arange(0, -1, 1).empty()));
    assert((arange(0, -1, 0).empty()));
    assert((arange(0, -1, -1) == vector{0}));
    assert((arange(0, -1, -10) == vector{0}));
    assert((arange(0, 0, 2).empty()));
    assert((arange(2, 2, 1).empty()));
    assert((arange(10, 10, -2).empty()));
    assert((arange(3, 7, -1).empty()));
    assert((arange(3, 4, 0).empty()));
    assert((arange(5, -5, 2).empty()));
    assert((arange(3, -7, 0).empty()));

    assert((arange(-3, -3, -3).empty()));
    assert((arange(-3, -3, -2).empty()));
    assert((arange(-3, -3, -1).empty()));
    assert((arange(-3, -3, 0).empty()));
    assert((arange(-3, -3, 1).empty()));
    assert((arange(-3, -3, 2).empty()));
    assert((arange(-3, -3, 3).empty()));
    assert((arange(-3, -2, -3).empty()));
    assert((arange(-3, -2, -2).empty()));
    assert((arange(-3, -2, -1).empty()));
    assert((arange(-3, -2, 0).empty()));
    assert((arange(-3, -2, 1) == vector{-3}));
    assert((arange(-3, -2, 2) == vector{-3}));
    assert((arange(-3, -2, 3) == vector{-3}));
    assert((arange(-3, -1, -3).empty()));
    assert((arange(-3, -1, -2).empty()));
    assert((arange(-3, -1, -1).empty()));
    assert((arange(-3, -1, 0).empty()));
    assert((arange(-3, -1, 1) == vector{-3, -2}));
    assert((arange(-3, -1, 2) == vector{-3}));
    assert((arange(-3, -1, 3) == vector{-3}));
    assert((arange(-3, 0, -3).empty()));
    assert((arange(-3, 0, -2).empty()));
    assert((arange(-3, 0, -1).empty()));
    assert((arange(-3, 0, 0).empty()));
    assert((arange(-3, 0, 1) == vector{-3, -2, -1}));
    assert((arange(-3, 0, 2) == vector{-3, -1}));
    assert((arange(-3, 0, 3) == vector{-3}));
    assert((arange(-3, 1, -3).empty()));
    assert((arange(-3, 1, -2).empty()));
    assert((arange(-3, 1, -1).empty()));
    assert((arange(-3, 1, 0).empty()));
    assert((arange(-3, 1, 1) == vector{-3, -2, -1, 0}));
    assert((arange(-3, 1, 2) == vector{-3, -1}));
    assert((arange(-3, 1, 3) == vector{-3, 0}));
    assert((arange(-3, 2, -3).empty()));
    assert((arange(-3, 2, -2).empty()));
    assert((arange(-3, 2, -1).empty()));
    assert((arange(-3, 2, 0).empty()));
    assert((arange(-3, 2, 1) == vector{-3, -2, -1, 0, 1}));
    assert((arange(-3, 2, 2) == vector{-3, -1, 1}));
    assert((arange(-3, 2, 3) == vector{-3, 0}));
    assert((arange(-3, 3, -3).empty()));
    assert((arange(-3, 3, -2).empty()));
    assert((arange(-3, 3, -1).empty()));
    assert((arange(-3, 3, 0).empty()));
    assert((arange(-3, 3, 1) == vector{-3, -2, -1, 0, 1, 2}));
    assert((arange(-3, 3, 2) == vector{-3, -1, 1}));
    assert((arange(-3, 3, 3) == vector{-3, 0}));
    assert((arange(-2, -3, -3) == vector{-2}));
    assert((arange(-2, -3, -2) == vector{-2}));
    assert((arange(-2, -3, -1) == vector{-2}));
    assert((arange(-2, -3, 0).empty()));
    assert((arange(-2, -3, 1).empty()));
    assert((arange(-2, -3, 2).empty()));
    assert((arange(-2, -3, 3).empty()));
    assert((arange(-2, -2, -3).empty()));
    assert((arange(-2, -2, -2).empty()));
    assert((arange(-2, -2, -1).empty()));
    assert((arange(-2, -2, 0).empty()));
    assert((arange(-2, -2, 1).empty()));
    assert((arange(-2, -2, 2).empty()));
    assert((arange(-2, -2, 3).empty()));
    assert((arange(-2, -1, -3).empty()));
    assert((arange(-2, -1, -2).empty()));
    assert((arange(-2, -1, -1).empty()));
    assert((arange(-2, -1, 0).empty()));
    assert((arange(-2, -1, 1) == vector{-2}));
    assert((arange(-2, -1, 2) == vector{-2}));
    assert((arange(-2, -1, 3) == vector{-2}));
    assert((arange(-2, 0, -3).empty()));
    assert((arange(-2, 0, -2).empty()));
    assert((arange(-2, 0, -1).empty()));
    assert((arange(-2, 0, 0).empty()));
    assert((arange(-2, 0, 1) == vector{-2, -1}));
    assert((arange(-2, 0, 2) == vector{-2}));
    assert((arange(-2, 0, 3) == vector{-2}));
    assert((arange(-2, 1, -3).empty()));
    assert((arange(-2, 1, -2).empty()));
    assert((arange(-2, 1, -1).empty()));
    assert((arange(-2, 1, 0).empty()));
    assert((arange(-2, 1, 1) == vector{-2, -1, 0}));
    assert((arange(-2, 1, 2) == vector{-2, 0}));
    assert((arange(-2, 1, 3) == vector{-2}));
    assert((arange(-2, 2, -3).empty()));
    assert((arange(-2, 2, -2).empty()));
    assert((arange(-2, 2, -1).empty()));
    assert((arange(-2, 2, 0).empty()));
    assert((arange(-2, 2, 1) == vector{-2, -1, 0, 1}));
    assert((arange(-2, 2, 2) == vector{-2, 0}));
    assert((arange(-2, 2, 3) == vector{-2, 1}));
    assert((arange(-2, 3, -3).empty()));
    assert((arange(-2, 3, -2).empty()));
    assert((arange(-2, 3, -1).empty()));
    assert((arange(-2, 3, 0).empty()));
    assert((arange(-2, 3, 1) == vector{-2, -1, 0, 1, 2}));
    assert((arange(-2, 3, 2) == vector{-2, 0, 2}));
    assert((arange(-2, 3, 3) == vector{-2, 1}));
    assert((arange(-1, -3, -3) == vector{-1}));
    assert((arange(-1, -3, -2) == vector{-1}));
    assert((arange(-1, -3, -1) == vector{-1, -2}));
    assert((arange(-1, -3, 0).empty()));
    assert((arange(-1, -3, 1).empty()));
    assert((arange(-1, -3, 2).empty()));
    assert((arange(-1, -3, 3).empty()));
    assert((arange(-1, -2, -3) == vector{-1}));
    assert((arange(-1, -2, -2) == vector{-1}));
    assert((arange(-1, -2, -1) == vector{-1}));
    assert((arange(-1, -2, 0).empty()));
    assert((arange(-1, -2, 1).empty()));
    assert((arange(-1, -2, 2).empty()));
    assert((arange(-1, -2, 3).empty()));
    assert((arange(-1, -1, -3).empty()));
    assert((arange(-1, -1, -2).empty()));
    assert((arange(-1, -1, -1).empty()));
    assert((arange(-1, -1, 0).empty()));
    assert((arange(-1, -1, 1).empty()));
    assert((arange(-1, -1, 2).empty()));
    assert((arange(-1, -1, 3).empty()));
    assert((arange(-1, 0, -3).empty()));
    assert((arange(-1, 0, -2).empty()));
    assert((arange(-1, 0, -1).empty()));
    assert((arange(-1, 0, 0).empty()));
    assert((arange(-1, 0, 1) == vector{-1}));
    assert((arange(-1, 0, 2) == vector{-1}));
    assert((arange(-1, 0, 3) == vector{-1}));
    assert((arange(-1, 1, -3).empty()));
    assert((arange(-1, 1, -2).empty()));
    assert((arange(-1, 1, -1).empty()));
    assert((arange(-1, 1, 0).empty()));
    assert((arange(-1, 1, 1) == vector{-1, 0}));
    assert((arange(-1, 1, 2) == vector{-1}));
    assert((arange(-1, 1, 3) == vector{-1}));
    assert((arange(-1, 2, -3).empty()));
    assert((arange(-1, 2, -2).empty()));
    assert((arange(-1, 2, -1).empty()));
    assert((arange(-1, 2, 0).empty()));
    assert((arange(-1, 2, 1) == vector{-1, 0, 1}));
    assert((arange(-1, 2, 2) == vector{-1, 1}));
    assert((arange(-1, 2, 3) == vector{-1}));
    assert((arange(-1, 3, -3).empty()));
    assert((arange(-1, 3, -2).empty()));
    assert((arange(-1, 3, -1).empty()));
    assert((arange(-1, 3, 0).empty()));
    assert((arange(-1, 3, 1) == vector{-1, 0, 1, 2}));
    assert((arange(-1, 3, 2) == vector{-1, 1}));
    assert((arange(-1, 3, 3) == vector{-1, 2}));
    assert((arange(0, -3, -3) == vector{0}));
    assert((arange(0, -3, -2) == vector{0, -2}));
    assert((arange(0, -3, -1) == vector{0, -1, -2}));
    assert((arange(0, -3, 0).empty()));
    assert((arange(0, -3, 1).empty()));
    assert((arange(0, -3, 2).empty()));
    assert((arange(0, -3, 3).empty()));
    assert((arange(0, -2, -3) == vector{0}));
    assert((arange(0, -2, -2) == vector{0}));
    assert((arange(0, -2, -1) == vector{0, -1}));
    assert((arange(0, -2, 0).empty()));
    assert((arange(0, -2, 1).empty()));
    assert((arange(0, -2, 2).empty()));
    assert((arange(0, -2, 3).empty()));
    assert((arange(0, -1, -3) == vector{0}));
    assert((arange(0, -1, -2) == vector{0}));
    assert((arange(0, -1, -1) == vector{0}));
    assert((arange(0, -1, 0).empty()));
    assert((arange(0, -1, 1).empty()));
    assert((arange(0, -1, 2).empty()));
    assert((arange(0, -1, 3).empty()));
    assert((arange(0, 0, -3).empty()));
    assert((arange(0, 0, -2).empty()));
    assert((arange(0, 0, -1).empty()));
    assert((arange(0, 0, 0).empty()));
    assert((arange(0, 0, 1).empty()));
    assert((arange(0, 0, 2).empty()));
    assert((arange(0, 0, 3).empty()));
    assert((arange(0, 1, -3).empty()));
    assert((arange(0, 1, -2).empty()));
    assert((arange(0, 1, -1).empty()));
    assert((arange(0, 1, 0).empty()));
    assert((arange(0, 1, 1) == vector{0}));
    assert((arange(0, 1, 2) == vector{0}));
    assert((arange(0, 1, 3) == vector{0}));
    assert((arange(0, 2, -3).empty()));
    assert((arange(0, 2, -2).empty()));
    assert((arange(0, 2, -1).empty()));
    assert((arange(0, 2, 0).empty()));
    assert((arange(0, 2, 1) == vector{0, 1}));
    assert((arange(0, 2, 2) == vector{0}));
    assert((arange(0, 2, 3) == vector{0}));
    assert((arange(0, 3, -3).empty()));
    assert((arange(0, 3, -2).empty()));
    assert((arange(0, 3, -1).empty()));
    assert((arange(0, 3, 0).empty()));
    assert((arange(0, 3, 1) == vector{0, 1, 2}));
    assert((arange(0, 3, 2) == vector{0, 2}));
    assert((arange(0, 3, 3) == vector{0}));
    assert((arange(1, -3, -3) == vector{1, -2}));
    assert((arange(1, -3, -2) == vector{1, -1}));
    assert((arange(1, -3, -1) == vector{1, 0, -1, -2}));
    assert((arange(1, -3, 0).empty()));
    assert((arange(1, -3, 1).empty()));
    assert((arange(1, -3, 2).empty()));
    assert((arange(1, -3, 3).empty()));
    assert((arange(1, -2, -3) == vector{1}));
    assert((arange(1, -2, -2) == vector{1, -1}));
    assert((arange(1, -2, -1) == vector{1, 0, -1}));
    assert((arange(1, -2, 0).empty()));
    assert((arange(1, -2, 1).empty()));
    assert((arange(1, -2, 2).empty()));
    assert((arange(1, -2, 3).empty()));
    assert((arange(1, -1, -3) == vector{1}));
    assert((arange(1, -1, -2) == vector{1}));
    assert((arange(1, -1, -1) == vector{1, 0}));
    assert((arange(1, -1, 0).empty()));
    assert((arange(1, -1, 1).empty()));
    assert((arange(1, -1, 2).empty()));
    assert((arange(1, -1, 3).empty()));
    assert((arange(1, 0, -3) == vector{1}));
    assert((arange(1, 0, -2) == vector{1}));
    assert((arange(1, 0, -1) == vector{1}));
    assert((arange(1, 0, 0).empty()));
    assert((arange(1, 0, 1).empty()));
    assert((arange(1, 0, 2).empty()));
    assert((arange(1, 0, 3).empty()));
    assert((arange(1, 1, -3).empty()));
    assert((arange(1, 1, -2).empty()));
    assert((arange(1, 1, -1).empty()));
    assert((arange(1, 1, 0).empty()));
    assert((arange(1, 1, 1).empty()));
    assert((arange(1, 1, 2).empty()));
    assert((arange(1, 1, 3).empty()));
    assert((arange(1, 2, -3).empty()));
    assert((arange(1, 2, -2).empty()));
    assert((arange(1, 2, -1).empty()));
    assert((arange(1, 2, 0).empty()));
    assert((arange(1, 2, 1) == vector{1}));
    assert((arange(1, 2, 2) == vector{1}));
    assert((arange(1, 2, 3) == vector{1}));
    assert((arange(1, 3, -3).empty()));
    assert((arange(1, 3, -2).empty()));
    assert((arange(1, 3, -1).empty()));
    assert((arange(1, 3, 0).empty()));
    assert((arange(1, 3, 1) == vector{1, 2}));
    assert((arange(1, 3, 2) == vector{1}));
    assert((arange(1, 3, 3) == vector{1}));
    assert((arange(2, -3, -3) == vector{2, -1}));
    assert((arange(2, -3, -2) == vector{2, 0, -2}));
    assert((arange(2, -3, -1) == vector{2, 1, 0, -1, -2}));
    assert((arange(2, -3, 0).empty()));
    assert((arange(2, -3, 1).empty()));
    assert((arange(2, -3, 2).empty()));
    assert((arange(2, -3, 3).empty()));
    assert((arange(2, -2, -3) == vector{2, -1}));
    assert((arange(2, -2, -2) == vector{2, 0}));
    assert((arange(2, -2, -1) == vector{2, 1, 0, -1}));
    assert((arange(2, -2, 0).empty()));
    assert((arange(2, -2, 1).empty()));
    assert((arange(2, -2, 2).empty()));
    assert((arange(2, -2, 3).empty()));
    assert((arange(2, -1, -3) == vector{2}));
    assert((arange(2, -1, -2) == vector{2, 0}));
    assert((arange(2, -1, -1) == vector{2, 1, 0}));
    assert((arange(2, -1, 0).empty()));
    assert((arange(2, -1, 1).empty()));
    assert((arange(2, -1, 2).empty()));
    assert((arange(2, -1, 3).empty()));
    assert((arange(2, 0, -3) == vector{2}));
    assert((arange(2, 0, -2) == vector{2}));
    assert((arange(2, 0, -1) == vector{2, 1}));
    assert((arange(2, 0, 0).empty()));
    assert((arange(2, 0, 1).empty()));
    assert((arange(2, 0, 2).empty()));
    assert((arange(2, 0, 3).empty()));
    assert((arange(2, 1, -3) == vector{2}));
    assert((arange(2, 1, -2) == vector{2}));
    assert((arange(2, 1, -1) == vector{2}));
    assert((arange(2, 1, 0).empty()));
    assert((arange(2, 1, 1).empty()));
    assert((arange(2, 1, 2).empty()));
    assert((arange(2, 1, 3).empty()));
    assert((arange(2, 2, -3).empty()));
    assert((arange(2, 2, -2).empty()));
    assert((arange(2, 2, -1).empty()));
    assert((arange(2, 2, 0).empty()));
    assert((arange(2, 2, 1).empty()));
    assert((arange(2, 2, 2).empty()));
    assert((arange(2, 2, 3).empty()));
    assert((arange(2, 3, -3).empty()));
    assert((arange(2, 3, -2).empty()));
    assert((arange(2, 3, -1).empty()));
    assert((arange(2, 3, 0).empty()));
    assert((arange(2, 3, 1) == vector{2}));
    assert((arange(2, 3, 2) == vector{2}));
    assert((arange(2, 3, 3) == vector{2}));
    assert((arange(3, -3, -3) == vector{3, 0}));
    assert((arange(3, -3, -2) == vector{3, 1, -1}));
    assert((arange(3, -3, -1) == vector{3, 2, 1, 0, -1, -2}));
    assert((arange(3, -3, 0).empty()));
    assert((arange(3, -3, 1).empty()));
    assert((arange(3, -3, 2).empty()));
    assert((arange(3, -3, 3).empty()));
    assert((arange(3, -2, -3) == vector{3, 0}));
    assert((arange(3, -2, -2) == vector{3, 1, -1}));
    assert((arange(3, -2, -1) == vector{3, 2, 1, 0, -1}));
    assert((arange(3, -2, 0).empty()));
    assert((arange(3, -2, 1).empty()));
    assert((arange(3, -2, 2).empty()));
    assert((arange(3, -2, 3).empty()));
    assert((arange(3, -1, -3) == vector{3, 0}));
    assert((arange(3, -1, -2) == vector{3, 1}));
    assert((arange(3, -1, -1) == vector{3, 2, 1, 0}));
    assert((arange(3, -1, 0).empty()));
    assert((arange(3, -1, 1).empty()));
    assert((arange(3, -1, 2).empty()));
    assert((arange(3, -1, 3).empty()));
    assert((arange(3, 0, -3) == vector{3}));
    assert((arange(3, 0, -2) == vector{3, 1}));
    assert((arange(3, 0, -1) == vector{3, 2, 1}));
    assert((arange(3, 0, 0).empty()));
    assert((arange(3, 0, 1).empty()));
    assert((arange(3, 0, 2).empty()));
    assert((arange(3, 0, 3).empty()));
    assert((arange(3, 1, -3) == vector{3}));
    assert((arange(3, 1, -2) == vector{3}));
    assert((arange(3, 1, -1) == vector{3, 2}));
    assert((arange(3, 1, 0).empty()));
    assert((arange(3, 1, 1).empty()));
    assert((arange(3, 1, 2).empty()));
    assert((arange(3, 1, 3).empty()));
    assert((arange(3, 2, -3) == vector{3}));
    assert((arange(3, 2, -2) == vector{3}));
    assert((arange(3, 2, -1) == vector{3}));
    assert((arange(3, 2, 0).empty()));
    assert((arange(3, 2, 1).empty()));
    assert((arange(3, 2, 2).empty()));
    assert((arange(3, 2, 3).empty()));
    assert((arange(3, 3, -3).empty()));
    assert((arange(3, 3, -2).empty()));
    assert((arange(3, 3, -1).empty()));
    assert((arange(3, 3, 0).empty()));
    assert((arange(3, 3, 1).empty()));
    assert((arange(3, 3, 2).empty()));
    assert((arange(3, 3, 3).empty()));

    assert((arange(1, -2, 1).empty()));
    assert((arange(1, -2, 2).empty()));
    assert((arange(1, -2, 3).empty()));
    assert((arange(1, -2, 4).empty()));
    assert((arange(1, -2, 5).empty()));
    assert((arange(1, -5, 1).empty()));
    assert((arange(1, -5, 2).empty()));
    assert((arange(1, -5, 3).empty()));
    assert((arange(1, -5, 4).empty()));
    assert((arange(1, -5, 5).empty()));
    assert((arange(1, -10, 1).empty()));
    assert((arange(1, -10, 2).empty()));
    assert((arange(1, -10, 3).empty()));
    assert((arange(1, -10, 4).empty()));
    assert((arange(1, -10, 5).empty()));

    assert((arange(1, -2, -1) == vector{1, 0, -1}));
    assert((arange(1, -2, -2) == vector{1, -1}));
    assert((arange(1, -2, -3) == vector{1}));
    assert((arange(1, -2, -4) == vector{1}));
    assert((arange(1, -2, -5) == vector{1}));
    assert((arange(1, -5, -1) == vector{1, 0, -1, -2, -3, -4}));
    assert((arange(1, -5, -2) == vector{1, -1, -3}));
    assert((arange(1, -5, -3) == vector{1, -2}));
    assert((arange(1, -5, -4) == vector{1, -3}));
    assert((arange(1, -5, -5) == vector{1, -4}));
    assert((arange(1, -10, -1) == vector{1, 0, -1, -2, -3, -4, -5, -6, -7, -8, -9}));
    assert((arange(1, -10, -2) == vector{1, -1, -3, -5, -7, -9}));
    assert((arange(1, -10, -3) == vector{1, -2, -5, -8}));
    assert((arange(1, -10, -4) == vector{1, -3, -7}));
    assert((arange(1, -10, -5) == vector{1, -4, -9}));

    assert((arange(-2, -2, -2).empty()));
    assert((arange(-2, -2, 2).empty()));
    assert((arange(-2, -2, -3).empty()));
    assert((arange(-2, -2, 3).empty()));
    assert((arange(-2, -2, -5).empty()));
    assert((arange(-2, -2, 5).empty()));
    assert((arange(-2, -2, -7).empty()));
    assert((arange(-2, -2, 7).empty()));
    assert((arange(-2, -2, -11).empty()));
    assert((arange(-2, -2, 11).empty()));
    assert((arange(-2, 2, -2).empty()));
    assert((arange(-2, 2, 2) == vector{-2, 0}));
    assert((arange(-2, 2, -3).empty()));
    assert((arange(-2, 2, 3) == vector{-2, 1}));
    assert((arange(-2, 2, -5).empty()));
    assert((arange(-2, 2, 5) == vector{-2}));
    assert((arange(-2, 2, -7).empty()));
    assert((arange(-2, 2, 7) == vector{-2}));
    assert((arange(-2, 2, -11).empty()));
    assert((arange(-2, 2, 11) == vector{-2}));
    assert((arange(-2, -3, -2) == vector{-2}));
    assert((arange(-2, -3, 2).empty()));
    assert((arange(-2, -3, -3) == vector{-2}));
    assert((arange(-2, -3, 3).empty()));
    assert((arange(-2, -3, -5) == vector{-2}));
    assert((arange(-2, -3, 5).empty()));
    assert((arange(-2, -3, -7) == vector{-2}));
    assert((arange(-2, -3, 7).empty()));
    assert((arange(-2, -3, -11) == vector{-2}));
    assert((arange(-2, -3, 11).empty()));
    assert((arange(-2, 3, -2).empty()));
    assert((arange(-2, 3, 2) == vector{-2, 0, 2}));
    assert((arange(-2, 3, -3).empty()));
    assert((arange(-2, 3, 3) == vector{-2, 1}));
    assert((arange(-2, 3, -5).empty()));
    assert((arange(-2, 3, 5) == vector{-2}));
    assert((arange(-2, 3, -7).empty()));
    assert((arange(-2, 3, 7) == vector{-2}));
    assert((arange(-2, 3, -11).empty()));
    assert((arange(-2, 3, 11) == vector{-2}));
    assert((arange(-2, -5, -2) == vector{-2, -4}));
    assert((arange(-2, -5, 2).empty()));
    assert((arange(-2, -5, -3) == vector{-2}));
    assert((arange(-2, -5, 3).empty()));
    assert((arange(-2, -5, -5) == vector{-2}));
    assert((arange(-2, -5, 5).empty()));
    assert((arange(-2, -5, -7) == vector{-2}));
    assert((arange(-2, -5, 7).empty()));
    assert((arange(-2, -5, -11) == vector{-2}));
    assert((arange(-2, -5, 11).empty()));
    assert((arange(-2, 5, -2).empty()));
    assert((arange(-2, 5, 2) == vector{-2, 0, 2, 4}));
    assert((arange(-2, 5, -3).empty()));
    assert((arange(-2, 5, 3) == vector{-2, 1, 4}));
    assert((arange(-2, 5, -5).empty()));
    assert((arange(-2, 5, 5) == vector{-2, 3}));
    assert((arange(-2, 5, -7).empty()));
    assert((arange(-2, 5, 7) == vector{-2}));
    assert((arange(-2, 5, -11).empty()));
    assert((arange(-2, 5, 11) == vector{-2}));
    assert((arange(-2, -7, -2) == vector{-2, -4, -6}));
    assert((arange(-2, -7, 2).empty()));
    assert((arange(-2, -7, -3) == vector{-2, -5}));
    assert((arange(-2, -7, 3).empty()));
    assert((arange(-2, -7, -5) == vector{-2}));
    assert((arange(-2, -7, 5).empty()));
    assert((arange(-2, -7, -7) == vector{-2}));
    assert((arange(-2, -7, 7).empty()));
    assert((arange(-2, -7, -11) == vector{-2}));
    assert((arange(-2, -7, 11).empty()));
    assert((arange(-2, 7, -2).empty()));
    assert((arange(-2, 7, 2) == vector{-2, 0, 2, 4, 6}));
    assert((arange(-2, 7, -3).empty()));
    assert((arange(-2, 7, 3) == vector{-2, 1, 4}));
    assert((arange(-2, 7, -5).empty()));
    assert((arange(-2, 7, 5) == vector{-2, 3}));
    assert((arange(-2, 7, -7).empty()));
    assert((arange(-2, 7, 7) == vector{-2, 5}));
    assert((arange(-2, 7, -11).empty()));
    assert((arange(-2, 7, 11) == vector{-2}));
    assert((arange(-2, -11, -2) == vector{-2, -4, -6, -8, -10}));
    assert((arange(-2, -11, 2).empty()));
    assert((arange(-2, -11, -3) == vector{-2, -5, -8}));
    assert((arange(-2, -11, 3).empty()));
    assert((arange(-2, -11, -5) == vector{-2, -7}));
    assert((arange(-2, -11, 5).empty()));
    assert((arange(-2, -11, -7) == vector{-2, -9}));
    assert((arange(-2, -11, 7).empty()));
    assert((arange(-2, -11, -11) == vector{-2}));
    assert((arange(-2, -11, 11).empty()));
    assert((arange(-2, 11, -2).empty()));
    assert((arange(-2, 11, 2) == vector{-2, 0, 2, 4, 6, 8, 10}));
    assert((arange(-2, 11, -3).empty()));
    assert((arange(-2, 11, 3) == vector{-2, 1, 4, 7, 10}));
    assert((arange(-2, 11, -5).empty()));
    assert((arange(-2, 11, 5) == vector{-2, 3, 8}));
    assert((arange(-2, 11, -7).empty()));
    assert((arange(-2, 11, 7) == vector{-2, 5}));
    assert((arange(-2, 11, -11).empty()));
    assert((arange(-2, 11, 11) == vector{-2, 9}));
    assert((arange(2, -2, -2) == vector{2, 0}));
    assert((arange(2, -2, 2).empty()));
    assert((arange(2, -2, -3) == vector{2, -1}));
    assert((arange(2, -2, 3).empty()));
    assert((arange(2, -2, -5) == vector{2}));
    assert((arange(2, -2, 5).empty()));
    assert((arange(2, -2, -7) == vector{2}));
    assert((arange(2, -2, 7).empty()));
    assert((arange(2, -2, -11) == vector{2}));
    assert((arange(2, -2, 11).empty()));
    assert((arange(2, 2, -2).empty()));
    assert((arange(2, 2, 2).empty()));
    assert((arange(2, 2, -3).empty()));
    assert((arange(2, 2, 3).empty()));
    assert((arange(2, 2, -5).empty()));
    assert((arange(2, 2, 5).empty()));
    assert((arange(2, 2, -7).empty()));
    assert((arange(2, 2, 7).empty()));
    assert((arange(2, 2, -11).empty()));
    assert((arange(2, 2, 11).empty()));
    assert((arange(2, -3, -2) == vector{2, 0, -2}));
    assert((arange(2, -3, 2).empty()));
    assert((arange(2, -3, -3) == vector{2, -1}));
    assert((arange(2, -3, 3).empty()));
    assert((arange(2, -3, -5) == vector{2}));
    assert((arange(2, -3, 5).empty()));
    assert((arange(2, -3, -7) == vector{2}));
    assert((arange(2, -3, 7).empty()));
    assert((arange(2, -3, -11) == vector{2}));
    assert((arange(2, -3, 11).empty()));
    assert((arange(2, 3, -2).empty()));
    assert((arange(2, 3, 2) == vector{2}));
    assert((arange(2, 3, -3).empty()));
    assert((arange(2, 3, 3) == vector{2}));
    assert((arange(2, 3, -5).empty()));
    assert((arange(2, 3, 5) == vector{2}));
    assert((arange(2, 3, -7).empty()));
    assert((arange(2, 3, 7) == vector{2}));
    assert((arange(2, 3, -11).empty()));
    assert((arange(2, 3, 11) == vector{2}));
    assert((arange(2, -5, -2) == vector{2, 0, -2, -4}));
    assert((arange(2, -5, 2).empty()));
    assert((arange(2, -5, -3) == vector{2, -1, -4}));
    assert((arange(2, -5, 3).empty()));
    assert((arange(2, -5, -5) == vector{2, -3}));
    assert((arange(2, -5, 5).empty()));
    assert((arange(2, -5, -7) == vector{2}));
    assert((arange(2, -5, 7).empty()));
    assert((arange(2, -5, -11) == vector{2}));
    assert((arange(2, -5, 11).empty()));
    assert((arange(2, 5, -2).empty()));
    assert((arange(2, 5, 2) == vector{2, 4}));
    assert((arange(2, 5, -3).empty()));
    assert((arange(2, 5, 3) == vector{2}));
    assert((arange(2, 5, -5).empty()));
    assert((arange(2, 5, 5) == vector{2}));
    assert((arange(2, 5, -7).empty()));
    assert((arange(2, 5, 7) == vector{2}));
    assert((arange(2, 5, -11).empty()));
    assert((arange(2, 5, 11) == vector{2}));
    assert((arange(2, -7, -2) == vector{2, 0, -2, -4, -6}));
    assert((arange(2, -7, 2).empty()));
    assert((arange(2, -7, -3) == vector{2, -1, -4}));
    assert((arange(2, -7, 3).empty()));
    assert((arange(2, -7, -5) == vector{2, -3}));
    assert((arange(2, -7, 5).empty()));
    assert((arange(2, -7, -7) == vector{2, -5}));
    assert((arange(2, -7, 7).empty()));
    assert((arange(2, -7, -11) == vector{2}));
    assert((arange(2, -7, 11).empty()));
    assert((arange(2, 7, -2).empty()));
    assert((arange(2, 7, 2) == vector{2, 4, 6}));
    assert((arange(2, 7, -3).empty()));
    assert((arange(2, 7, 3) == vector{2, 5}));
    assert((arange(2, 7, -5).empty()));
    assert((arange(2, 7, 5) == vector{2}));
    assert((arange(2, 7, -7).empty()));
    assert((arange(2, 7, 7) == vector{2}));
    assert((arange(2, 7, -11).empty()));
    assert((arange(2, 7, 11) == vector{2}));
    assert((arange(2, -11, -2) == vector{2, 0, -2, -4, -6, -8, -10}));
    assert((arange(2, -11, 2).empty()));
    assert((arange(2, -11, -3) == vector{2, -1, -4, -7, -10}));
    assert((arange(2, -11, 3).empty()));
    assert((arange(2, -11, -5) == vector{2, -3, -8}));
    assert((arange(2, -11, 5).empty()));
    assert((arange(2, -11, -7) == vector{2, -5}));
    assert((arange(2, -11, 7).empty()));
    assert((arange(2, -11, -11) == vector{2, -9}));
    assert((arange(2, -11, 11).empty()));
    assert((arange(2, 11, -2).empty()));
    assert((arange(2, 11, 2) == vector{2, 4, 6, 8, 10}));
    assert((arange(2, 11, -3).empty()));
    assert((arange(2, 11, 3) == vector{2, 5, 8}));
    assert((arange(2, 11, -5).empty()));
    assert((arange(2, 11, 5) == vector{2, 7}));
    assert((arange(2, 11, -7).empty()));
    assert((arange(2, 11, 7) == vector{2, 9}));
    assert((arange(2, 11, -11).empty()));
    assert((arange(2, 11, 11) == vector{2}));
    assert((arange(-3, -2, -2).empty()));
    assert((arange(-3, -2, 2) == vector{-3}));
    assert((arange(-3, -2, -3).empty()));
    assert((arange(-3, -2, 3) == vector{-3}));
    assert((arange(-3, -2, -5).empty()));
    assert((arange(-3, -2, 5) == vector{-3}));
    assert((arange(-3, -2, -7).empty()));
    assert((arange(-3, -2, 7) == vector{-3}));
    assert((arange(-3, -2, -11).empty()));
    assert((arange(-3, -2, 11) == vector{-3}));
    assert((arange(-3, 2, -2).empty()));
    assert((arange(-3, 2, 2) == vector{-3, -1, 1}));
    assert((arange(-3, 2, -3).empty()));
    assert((arange(-3, 2, 3) == vector{-3, 0}));
    assert((arange(-3, 2, -5).empty()));
    assert((arange(-3, 2, 5) == vector{-3}));
    assert((arange(-3, 2, -7).empty()));
    assert((arange(-3, 2, 7) == vector{-3}));
    assert((arange(-3, 2, -11).empty()));
    assert((arange(-3, 2, 11) == vector{-3}));
    assert((arange(-3, -3, -2).empty()));
    assert((arange(-3, -3, 2).empty()));
    assert((arange(-3, -3, -3).empty()));
    assert((arange(-3, -3, 3).empty()));
    assert((arange(-3, -3, -5).empty()));
    assert((arange(-3, -3, 5).empty()));
    assert((arange(-3, -3, -7).empty()));
    assert((arange(-3, -3, 7).empty()));
    assert((arange(-3, -3, -11).empty()));
    assert((arange(-3, -3, 11).empty()));
    assert((arange(-3, 3, -2).empty()));
    assert((arange(-3, 3, 2) == vector{-3, -1, 1}));
    assert((arange(-3, 3, -3).empty()));
    assert((arange(-3, 3, 3) == vector{-3, 0}));
    assert((arange(-3, 3, -5).empty()));
    assert((arange(-3, 3, 5) == vector{-3, 2}));
    assert((arange(-3, 3, -7).empty()));
    assert((arange(-3, 3, 7) == vector{-3}));
    assert((arange(-3, 3, -11).empty()));
    assert((arange(-3, 3, 11) == vector{-3}));
    assert((arange(-3, -5, -2) == vector{-3}));
    assert((arange(-3, -5, 2).empty()));
    assert((arange(-3, -5, -3) == vector{-3}));
    assert((arange(-3, -5, 3).empty()));
    assert((arange(-3, -5, -5) == vector{-3}));
    assert((arange(-3, -5, 5).empty()));
    assert((arange(-3, -5, -7) == vector{-3}));
    assert((arange(-3, -5, 7).empty()));
    assert((arange(-3, -5, -11) == vector{-3}));
    assert((arange(-3, -5, 11).empty()));
    assert((arange(-3, 5, -2).empty()));
    assert((arange(-3, 5, 2) == vector{-3, -1, 1, 3}));
    assert((arange(-3, 5, -3).empty()));
    assert((arange(-3, 5, 3) == vector{-3, 0, 3}));
    assert((arange(-3, 5, -5).empty()));
    assert((arange(-3, 5, 5) == vector{-3, 2}));
    assert((arange(-3, 5, -7).empty()));
    assert((arange(-3, 5, 7) == vector{-3, 4}));
    assert((arange(-3, 5, -11).empty()));
    assert((arange(-3, 5, 11) == vector{-3}));
    assert((arange(-3, -7, -2) == vector{-3, -5}));
    assert((arange(-3, -7, 2).empty()));
    assert((arange(-3, -7, -3) == vector{-3, -6}));
    assert((arange(-3, -7, 3).empty()));
    assert((arange(-3, -7, -5) == vector{-3}));
    assert((arange(-3, -7, 5).empty()));
    assert((arange(-3, -7, -7) == vector{-3}));
    assert((arange(-3, -7, 7).empty()));
    assert((arange(-3, -7, -11) == vector{-3}));
    assert((arange(-3, -7, 11).empty()));
    assert((arange(-3, 7, -2).empty()));
    assert((arange(-3, 7, 2) == vector{-3, -1, 1, 3, 5}));
    assert((arange(-3, 7, -3).empty()));
    assert((arange(-3, 7, 3) == vector{-3, 0, 3, 6}));
    assert((arange(-3, 7, -5).empty()));
    assert((arange(-3, 7, 5) == vector{-3, 2}));
    assert((arange(-3, 7, -7).empty()));
    assert((arange(-3, 7, 7) == vector{-3, 4}));
    assert((arange(-3, 7, -11).empty()));
    assert((arange(-3, 7, 11) == vector{-3}));
    assert((arange(-3, -11, -2) == vector{-3, -5, -7, -9}));
    assert((arange(-3, -11, 2).empty()));
    assert((arange(-3, -11, -3) == vector{-3, -6, -9}));
    assert((arange(-3, -11, 3).empty()));
    assert((arange(-3, -11, -5) == vector{-3, -8}));
    assert((arange(-3, -11, 5).empty()));
    assert((arange(-3, -11, -7) == vector{-3, -10}));
    assert((arange(-3, -11, 7).empty()));
    assert((arange(-3, -11, -11) == vector{-3}));
    assert((arange(-3, -11, 11).empty()));
    assert((arange(-3, 11, -2).empty()));
    assert((arange(-3, 11, 2) == vector{-3, -1, 1, 3, 5, 7, 9}));
    assert((arange(-3, 11, -3).empty()));
    assert((arange(-3, 11, 3) == vector{-3, 0, 3, 6, 9}));
    assert((arange(-3, 11, -5).empty()));
    assert((arange(-3, 11, 5) == vector{-3, 2, 7}));
    assert((arange(-3, 11, -7).empty()));
    assert((arange(-3, 11, 7) == vector{-3, 4}));
    assert((arange(-3, 11, -11).empty()));
    assert((arange(-3, 11, 11) == vector{-3, 8}));
    assert((arange(3, -2, -2) == vector{3, 1, -1}));
    assert((arange(3, -2, 2).empty()));
    assert((arange(3, -2, -3) == vector{3, 0}));
    assert((arange(3, -2, 3).empty()));
    assert((arange(3, -2, -5) == vector{3}));
    assert((arange(3, -2, 5).empty()));
    assert((arange(3, -2, -7) == vector{3}));
    assert((arange(3, -2, 7).empty()));
    assert((arange(3, -2, -11) == vector{3}));
    assert((arange(3, -2, 11).empty()));
    assert((arange(3, 2, -2) == vector{3}));
    assert((arange(3, 2, 2).empty()));
    assert((arange(3, 2, -3) == vector{3}));
    assert((arange(3, 2, 3).empty()));
    assert((arange(3, 2, -5) == vector{3}));
    assert((arange(3, 2, 5).empty()));
    assert((arange(3, 2, -7) == vector{3}));
    assert((arange(3, 2, 7).empty()));
    assert((arange(3, 2, -11) == vector{3}));
    assert((arange(3, 2, 11).empty()));
    assert((arange(3, -3, -2) == vector{3, 1, -1}));
    assert((arange(3, -3, 2).empty()));
    assert((arange(3, -3, -3) == vector{3, 0}));
    assert((arange(3, -3, 3).empty()));
    assert((arange(3, -3, -5) == vector{3, -2}));
    assert((arange(3, -3, 5).empty()));
    assert((arange(3, -3, -7) == vector{3}));
    assert((arange(3, -3, 7).empty()));
    assert((arange(3, -3, -11) == vector{3}));
    assert((arange(3, -3, 11).empty()));
    assert((arange(3, 3, -2).empty()));
    assert((arange(3, 3, 2).empty()));
    assert((arange(3, 3, -3).empty()));
    assert((arange(3, 3, 3).empty()));
    assert((arange(3, 3, -5).empty()));
    assert((arange(3, 3, 5).empty()));
    assert((arange(3, 3, -7).empty()));
    assert((arange(3, 3, 7).empty()));
    assert((arange(3, 3, -11).empty()));
    assert((arange(3, 3, 11).empty()));
    assert((arange(3, -5, -2) == vector{3, 1, -1, -3}));
    assert((arange(3, -5, 2).empty()));
    assert((arange(3, -5, -3) == vector{3, 0, -3}));
    assert((arange(3, -5, 3).empty()));
    assert((arange(3, -5, -5) == vector{3, -2}));
    assert((arange(3, -5, 5).empty()));
    assert((arange(3, -5, -7) == vector{3, -4}));
    assert((arange(3, -5, 7).empty()));
    assert((arange(3, -5, -11) == vector{3}));
    assert((arange(3, -5, 11).empty()));
    assert((arange(3, 5, -2).empty()));
    assert((arange(3, 5, 2) == vector{3}));
    assert((arange(3, 5, -3).empty()));
    assert((arange(3, 5, 3) == vector{3}));
    assert((arange(3, 5, -5).empty()));
    assert((arange(3, 5, 5) == vector{3}));
    assert((arange(3, 5, -7).empty()));
    assert((arange(3, 5, 7) == vector{3}));
    assert((arange(3, 5, -11).empty()));
    assert((arange(3, 5, 11) == vector{3}));
    assert((arange(3, -7, -2) == vector{3, 1, -1, -3, -5}));
    assert((arange(3, -7, 2).empty()));
    assert((arange(3, -7, -3) == vector{3, 0, -3, -6}));
    assert((arange(3, -7, 3).empty()));
    assert((arange(3, -7, -5) == vector{3, -2}));
    assert((arange(3, -7, 5).empty()));
    assert((arange(3, -7, -7) == vector{3, -4}));
    assert((arange(3, -7, 7).empty()));
    assert((arange(3, -7, -11) == vector{3}));
    assert((arange(3, -7, 11).empty()));
    assert((arange(3, 7, -2).empty()));
    assert((arange(3, 7, 2) == vector{3, 5}));
    assert((arange(3, 7, -3).empty()));
    assert((arange(3, 7, 3) == vector{3, 6}));
    assert((arange(3, 7, -5).empty()));
    assert((arange(3, 7, 5) == vector{3}));
    assert((arange(3, 7, -7).empty()));
    assert((arange(3, 7, 7) == vector{3}));
    assert((arange(3, 7, -11).empty()));
    assert((arange(3, 7, 11) == vector{3}));
    assert((arange(3, -11, -2) == vector{3, 1, -1, -3, -5, -7, -9}));
    assert((arange(3, -11, 2).empty()));
    assert((arange(3, -11, -3) == vector{3, 0, -3, -6, -9}));
    assert((arange(3, -11, 3).empty()));
    assert((arange(3, -11, -5) == vector{3, -2, -7}));
    assert((arange(3, -11, 5).empty()));
    assert((arange(3, -11, -7) == vector{3, -4}));
    assert((arange(3, -11, 7).empty()));
    assert((arange(3, -11, -11) == vector{3, -8}));
    assert((arange(3, -11, 11).empty()));
    assert((arange(3, 11, -2).empty()));
    assert((arange(3, 11, 2) == vector{3, 5, 7, 9}));
    assert((arange(3, 11, -3).empty()));
    assert((arange(3, 11, 3) == vector{3, 6, 9}));
    assert((arange(3, 11, -5).empty()));
    assert((arange(3, 11, 5) == vector{3, 8}));
    assert((arange(3, 11, -7).empty()));
    assert((arange(3, 11, 7) == vector{3, 10}));
    assert((arange(3, 11, -11).empty()));
    assert((arange(3, 11, 11) == vector{3}));
    assert((arange(-5, -2, -2).empty()));
    assert((arange(-5, -2, 2) == vector{-5, -3}));
    assert((arange(-5, -2, -3).empty()));
    assert((arange(-5, -2, 3) == vector{-5}));
    assert((arange(-5, -2, -5).empty()));
    assert((arange(-5, -2, 5) == vector{-5}));
    assert((arange(-5, -2, -7).empty()));
    assert((arange(-5, -2, 7) == vector{-5}));
    assert((arange(-5, -2, -11).empty()));
    assert((arange(-5, -2, 11) == vector{-5}));
    assert((arange(-5, 2, -2).empty()));
    assert((arange(-5, 2, 2) == vector{-5, -3, -1, 1}));
    assert((arange(-5, 2, -3).empty()));
    assert((arange(-5, 2, 3) == vector{-5, -2, 1}));
    assert((arange(-5, 2, -5).empty()));
    assert((arange(-5, 2, 5) == vector{-5, 0}));
    assert((arange(-5, 2, -7).empty()));
    assert((arange(-5, 2, 7) == vector{-5}));
    assert((arange(-5, 2, -11).empty()));
    assert((arange(-5, 2, 11) == vector{-5}));
    assert((arange(-5, -3, -2).empty()));
    assert((arange(-5, -3, 2) == vector{-5}));
    assert((arange(-5, -3, -3).empty()));
    assert((arange(-5, -3, 3) == vector{-5}));
    assert((arange(-5, -3, -5).empty()));
    assert((arange(-5, -3, 5) == vector{-5}));
    assert((arange(-5, -3, -7).empty()));
    assert((arange(-5, -3, 7) == vector{-5}));
    assert((arange(-5, -3, -11).empty()));
    assert((arange(-5, -3, 11) == vector{-5}));
    assert((arange(-5, 3, -2).empty()));
    assert((arange(-5, 3, 2) == vector{-5, -3, -1, 1}));
    assert((arange(-5, 3, -3).empty()));
    assert((arange(-5, 3, 3) == vector{-5, -2, 1}));
    assert((arange(-5, 3, -5).empty()));
    assert((arange(-5, 3, 5) == vector{-5, 0}));
    assert((arange(-5, 3, -7).empty()));
    assert((arange(-5, 3, 7) == vector{-5, 2}));
    assert((arange(-5, 3, -11).empty()));
    assert((arange(-5, 3, 11) == vector{-5}));
    assert((arange(-5, -5, -2).empty()));
    assert((arange(-5, -5, 2).empty()));
    assert((arange(-5, -5, -3).empty()));
    assert((arange(-5, -5, 3).empty()));
    assert((arange(-5, -5, -5).empty()));
    assert((arange(-5, -5, 5).empty()));
    assert((arange(-5, -5, -7).empty()));
    assert((arange(-5, -5, 7).empty()));
    assert((arange(-5, -5, -11).empty()));
    assert((arange(-5, -5, 11).empty()));
    assert((arange(-5, 5, -2).empty()));
    assert((arange(-5, 5, 2) == vector{-5, -3, -1, 1, 3}));
    assert((arange(-5, 5, -3).empty()));
    assert((arange(-5, 5, 3) == vector{-5, -2, 1, 4}));
    assert((arange(-5, 5, -5).empty()));
    assert((arange(-5, 5, 5) == vector{-5, 0}));
    assert((arange(-5, 5, -7).empty()));
    assert((arange(-5, 5, 7) == vector{-5, 2}));
    assert((arange(-5, 5, -11).empty()));
    assert((arange(-5, 5, 11) == vector{-5}));
    assert((arange(-5, -7, -2) == vector{-5}));
    assert((arange(-5, -7, 2).empty()));
    assert((arange(-5, -7, -3) == vector{-5}));
    assert((arange(-5, -7, 3).empty()));
    assert((arange(-5, -7, -5) == vector{-5}));
    assert((arange(-5, -7, 5).empty()));
    assert((arange(-5, -7, -7) == vector{-5}));
    assert((arange(-5, -7, 7).empty()));
    assert((arange(-5, -7, -11) == vector{-5}));
    assert((arange(-5, -7, 11).empty()));
    assert((arange(-5, 7, -2).empty()));
    assert((arange(-5, 7, 2) == vector{-5, -3, -1, 1, 3, 5}));
    assert((arange(-5, 7, -3).empty()));
    assert((arange(-5, 7, 3) == vector{-5, -2, 1, 4}));
    assert((arange(-5, 7, -5).empty()));
    assert((arange(-5, 7, 5) == vector{-5, 0, 5}));
    assert((arange(-5, 7, -7).empty()));
    assert((arange(-5, 7, 7) == vector{-5, 2}));
    assert((arange(-5, 7, -11).empty()));
    assert((arange(-5, 7, 11) == vector{-5, 6}));
    assert((arange(-5, -11, -2) == vector{-5, -7, -9}));
    assert((arange(-5, -11, 2).empty()));
    assert((arange(-5, -11, -3) == vector{-5, -8}));
    assert((arange(-5, -11, 3).empty()));
    assert((arange(-5, -11, -5) == vector{-5, -10}));
    assert((arange(-5, -11, 5).empty()));
    assert((arange(-5, -11, -7) == vector{-5}));
    assert((arange(-5, -11, 7).empty()));
    assert((arange(-5, -11, -11) == vector{-5}));
    assert((arange(-5, -11, 11).empty()));
    assert((arange(-5, 11, -2).empty()));
    assert((arange(-5, 11, 2) == vector{-5, -3, -1, 1, 3, 5, 7, 9}));
    assert((arange(-5, 11, -3).empty()));
    assert((arange(-5, 11, 3) == vector{-5, -2, 1, 4, 7, 10}));
    assert((arange(-5, 11, -5).empty()));
    assert((arange(-5, 11, 5) == vector{-5, 0, 5, 10}));
    assert((arange(-5, 11, -7).empty()));
    assert((arange(-5, 11, 7) == vector{-5, 2, 9}));
    assert((arange(-5, 11, -11).empty()));
    assert((arange(-5, 11, 11) == vector{-5, 6}));
    assert((arange(5, -2, -2) == vector{5, 3, 1, -1}));
    assert((arange(5, -2, 2).empty()));
    assert((arange(5, -2, -3) == vector{5, 2, -1}));
    assert((arange(5, -2, 3).empty()));
    assert((arange(5, -2, -5) == vector{5, 0}));
    assert((arange(5, -2, 5).empty()));
    assert((arange(5, -2, -7) == vector{5}));
    assert((arange(5, -2, 7).empty()));
    assert((arange(5, -2, -11) == vector{5}));
    assert((arange(5, -2, 11).empty()));
    assert((arange(5, 2, -2) == vector{5, 3}));
    assert((arange(5, 2, 2).empty()));
    assert((arange(5, 2, -3) == vector{5}));
    assert((arange(5, 2, 3).empty()));
    assert((arange(5, 2, -5) == vector{5}));
    assert((arange(5, 2, 5).empty()));
    assert((arange(5, 2, -7) == vector{5}));
    assert((arange(5, 2, 7).empty()));
    assert((arange(5, 2, -11) == vector{5}));
    assert((arange(5, 2, 11).empty()));
    assert((arange(5, -3, -2) == vector{5, 3, 1, -1}));
    assert((arange(5, -3, 2).empty()));
    assert((arange(5, -3, -3) == vector{5, 2, -1}));
    assert((arange(5, -3, 3).empty()));
    assert((arange(5, -3, -5) == vector{5, 0}));
    assert((arange(5, -3, 5).empty()));
    assert((arange(5, -3, -7) == vector{5, -2}));
    assert((arange(5, -3, 7).empty()));
    assert((arange(5, -3, -11) == vector{5}));
    assert((arange(5, -3, 11).empty()));
    assert((arange(5, 3, -2) == vector{5}));
    assert((arange(5, 3, 2).empty()));
    assert((arange(5, 3, -3) == vector{5}));
    assert((arange(5, 3, 3).empty()));
    assert((arange(5, 3, -5) == vector{5}));
    assert((arange(5, 3, 5).empty()));
    assert((arange(5, 3, -7) == vector{5}));
    assert((arange(5, 3, 7).empty()));
    assert((arange(5, 3, -11) == vector{5}));
    assert((arange(5, 3, 11).empty()));
    assert((arange(5, -5, -2) == vector{5, 3, 1, -1, -3}));
    assert((arange(5, -5, 2).empty()));
    assert((arange(5, -5, -3) == vector{5, 2, -1, -4}));
    assert((arange(5, -5, 3).empty()));
    assert((arange(5, -5, -5) == vector{5, 0}));
    assert((arange(5, -5, 5).empty()));
    assert((arange(5, -5, -7) == vector{5, -2}));
    assert((arange(5, -5, 7).empty()));
    assert((arange(5, -5, -11) == vector{5}));
    assert((arange(5, -5, 11).empty()));
    assert((arange(5, 5, -2).empty()));
    assert((arange(5, 5, 2).empty()));
    assert((arange(5, 5, -3).empty()));
    assert((arange(5, 5, 3).empty()));
    assert((arange(5, 5, -5).empty()));
    assert((arange(5, 5, 5).empty()));
    assert((arange(5, 5, -7).empty()));
    assert((arange(5, 5, 7).empty()));
    assert((arange(5, 5, -11).empty()));
    assert((arange(5, 5, 11).empty()));
    assert((arange(5, -7, -2) == vector{5, 3, 1, -1, -3, -5}));
    assert((arange(5, -7, 2).empty()));
    assert((arange(5, -7, -3) == vector{5, 2, -1, -4}));
    assert((arange(5, -7, 3).empty()));
    assert((arange(5, -7, -5) == vector{5, 0, -5}));
    assert((arange(5, -7, 5).empty()));
    assert((arange(5, -7, -7) == vector{5, -2}));
    assert((arange(5, -7, 7).empty()));
    assert((arange(5, -7, -11) == vector{5, -6}));
    assert((arange(5, -7, 11).empty()));
    assert((arange(5, 7, -2).empty()));
    assert((arange(5, 7, 2) == vector{5}));
    assert((arange(5, 7, -3).empty()));
    assert((arange(5, 7, 3) == vector{5}));
    assert((arange(5, 7, -5).empty()));
    assert((arange(5, 7, 5) == vector{5}));
    assert((arange(5, 7, -7).empty()));
    assert((arange(5, 7, 7) == vector{5}));
    assert((arange(5, 7, -11).empty()));
    assert((arange(5, 7, 11) == vector{5}));
    assert((arange(5, -11, -2) == vector{5, 3, 1, -1, -3, -5, -7, -9}));
    assert((arange(5, -11, 2).empty()));
    assert((arange(5, -11, -3) == vector{5, 2, -1, -4, -7, -10}));
    assert((arange(5, -11, 3).empty()));
    assert((arange(5, -11, -5) == vector{5, 0, -5, -10}));
    assert((arange(5, -11, 5).empty()));
    assert((arange(5, -11, -7) == vector{5, -2, -9}));
    assert((arange(5, -11, 7).empty()));
    assert((arange(5, -11, -11) == vector{5, -6}));
    assert((arange(5, -11, 11).empty()));
    assert((arange(5, 11, -2).empty()));
    assert((arange(5, 11, 2) == vector{5, 7, 9}));
    assert((arange(5, 11, -3).empty()));
    assert((arange(5, 11, 3) == vector{5, 8}));
    assert((arange(5, 11, -5).empty()));
    assert((arange(5, 11, 5) == vector{5, 10}));
    assert((arange(5, 11, -7).empty()));
    assert((arange(5, 11, 7) == vector{5}));
    assert((arange(5, 11, -11).empty()));
    assert((arange(5, 11, 11) == vector{5}));
    assert((arange(-7, -2, -2).empty()));
    assert((arange(-7, -2, 2) == vector{-7, -5, -3}));
    assert((arange(-7, -2, -3).empty()));
    assert((arange(-7, -2, 3) == vector{-7, -4}));
    assert((arange(-7, -2, -5).empty()));
    assert((arange(-7, -2, 5) == vector{-7}));
    assert((arange(-7, -2, -7).empty()));
    assert((arange(-7, -2, 7) == vector{-7}));
    assert((arange(-7, -2, -11).empty()));
    assert((arange(-7, -2, 11) == vector{-7}));
    assert((arange(-7, 2, -2).empty()));
    assert((arange(-7, 2, 2) == vector{-7, -5, -3, -1, 1}));
    assert((arange(-7, 2, -3).empty()));
    assert((arange(-7, 2, 3) == vector{-7, -4, -1}));
    assert((arange(-7, 2, -5).empty()));
    assert((arange(-7, 2, 5) == vector{-7, -2}));
    assert((arange(-7, 2, -7).empty()));
    assert((arange(-7, 2, 7) == vector{-7, 0}));
    assert((arange(-7, 2, -11).empty()));
    assert((arange(-7, 2, 11) == vector{-7}));
    assert((arange(-7, -3, -2).empty()));
    assert((arange(-7, -3, 2) == vector{-7, -5}));
    assert((arange(-7, -3, -3).empty()));
    assert((arange(-7, -3, 3) == vector{-7, -4}));
    assert((arange(-7, -3, -5).empty()));
    assert((arange(-7, -3, 5) == vector{-7}));
    assert((arange(-7, -3, -7).empty()));
    assert((arange(-7, -3, 7) == vector{-7}));
    assert((arange(-7, -3, -11).empty()));
    assert((arange(-7, -3, 11) == vector{-7}));
    assert((arange(-7, 3, -2).empty()));
    assert((arange(-7, 3, 2) == vector{-7, -5, -3, -1, 1}));
    assert((arange(-7, 3, -3).empty()));
    assert((arange(-7, 3, 3) == vector{-7, -4, -1, 2}));
    assert((arange(-7, 3, -5).empty()));
    assert((arange(-7, 3, 5) == vector{-7, -2}));
    assert((arange(-7, 3, -7).empty()));
    assert((arange(-7, 3, 7) == vector{-7, 0}));
    assert((arange(-7, 3, -11).empty()));
    assert((arange(-7, 3, 11) == vector{-7}));
    assert((arange(-7, -5, -2).empty()));
    assert((arange(-7, -5, 2) == vector{-7}));
    assert((arange(-7, -5, -3).empty()));
    assert((arange(-7, -5, 3) == vector{-7}));
    assert((arange(-7, -5, -5).empty()));
    assert((arange(-7, -5, 5) == vector{-7}));
    assert((arange(-7, -5, -7).empty()));
    assert((arange(-7, -5, 7) == vector{-7}));
    assert((arange(-7, -5, -11).empty()));
    assert((arange(-7, -5, 11) == vector{-7}));
    assert((arange(-7, 5, -2).empty()));
    assert((arange(-7, 5, 2) == vector{-7, -5, -3, -1, 1, 3}));
    assert((arange(-7, 5, -3).empty()));
    assert((arange(-7, 5, 3) == vector{-7, -4, -1, 2}));
    assert((arange(-7, 5, -5).empty()));
    assert((arange(-7, 5, 5) == vector{-7, -2, 3}));
    assert((arange(-7, 5, -7).empty()));
    assert((arange(-7, 5, 7) == vector{-7, 0}));
    assert((arange(-7, 5, -11).empty()));
    assert((arange(-7, 5, 11) == vector{-7, 4}));
    assert((arange(-7, -7, -2).empty()));
    assert((arange(-7, -7, 2).empty()));
    assert((arange(-7, -7, -3).empty()));
    assert((arange(-7, -7, 3).empty()));
    assert((arange(-7, -7, -5).empty()));
    assert((arange(-7, -7, 5).empty()));
    assert((arange(-7, -7, -7).empty()));
    assert((arange(-7, -7, 7).empty()));
    assert((arange(-7, -7, -11).empty()));
    assert((arange(-7, -7, 11).empty()));
    assert((arange(-7, 7, -2).empty()));
    assert((arange(-7, 7, 2) == vector{-7, -5, -3, -1, 1, 3, 5}));
    assert((arange(-7, 7, -3).empty()));
    assert((arange(-7, 7, 3) == vector{-7, -4, -1, 2, 5}));
    assert((arange(-7, 7, -5).empty()));
    assert((arange(-7, 7, 5) == vector{-7, -2, 3}));
    assert((arange(-7, 7, -7).empty()));
    assert((arange(-7, 7, 7) == vector{-7, 0}));
    assert((arange(-7, 7, -11).empty()));
    assert((arange(-7, 7, 11) == vector{-7, 4}));
    assert((arange(-7, -11, -2) == vector{-7, -9}));
    assert((arange(-7, -11, 2).empty()));
    assert((arange(-7, -11, -3) == vector{-7, -10}));
    assert((arange(-7, -11, 3).empty()));
    assert((arange(-7, -11, -5) == vector{-7}));
    assert((arange(-7, -11, 5).empty()));
    assert((arange(-7, -11, -7) == vector{-7}));
    assert((arange(-7, -11, 7).empty()));
    assert((arange(-7, -11, -11) == vector{-7}));
    assert((arange(-7, -11, 11).empty()));
    assert((arange(-7, 11, -2).empty()));
    assert((arange(-7, 11, 2) == vector{-7, -5, -3, -1, 1, 3, 5, 7, 9}));
    assert((arange(-7, 11, -3).empty()));
    assert((arange(-7, 11, 3) == vector{-7, -4, -1, 2, 5, 8}));
    assert((arange(-7, 11, -5).empty()));
    assert((arange(-7, 11, 5) == vector{-7, -2, 3, 8}));
    assert((arange(-7, 11, -7).empty()));
    assert((arange(-7, 11, 7) == vector{-7, 0, 7}));
    assert((arange(-7, 11, -11).empty()));
    assert((arange(-7, 11, 11) == vector{-7, 4}));
    assert((arange(7, -2, -2) == vector{7, 5, 3, 1, -1}));
    assert((arange(7, -2, 2).empty()));
    assert((arange(7, -2, -3) == vector{7, 4, 1}));
    assert((arange(7, -2, 3).empty()));
    assert((arange(7, -2, -5) == vector{7, 2}));
    assert((arange(7, -2, 5).empty()));
    assert((arange(7, -2, -7) == vector{7, 0}));
    assert((arange(7, -2, 7).empty()));
    assert((arange(7, -2, -11) == vector{7}));
    assert((arange(7, -2, 11).empty()));
    assert((arange(7, 2, -2) == vector{7, 5, 3}));
    assert((arange(7, 2, 2).empty()));
    assert((arange(7, 2, -3) == vector{7, 4}));
    assert((arange(7, 2, 3).empty()));
    assert((arange(7, 2, -5) == vector{7}));
    assert((arange(7, 2, 5).empty()));
    assert((arange(7, 2, -7) == vector{7}));
    assert((arange(7, 2, 7).empty()));
    assert((arange(7, 2, -11) == vector{7}));
    assert((arange(7, 2, 11).empty()));
    assert((arange(7, -3, -2) == vector{7, 5, 3, 1, -1}));
    assert((arange(7, -3, 2).empty()));
    assert((arange(7, -3, -3) == vector{7, 4, 1, -2}));
    assert((arange(7, -3, 3).empty()));
    assert((arange(7, -3, -5) == vector{7, 2}));
    assert((arange(7, -3, 5).empty()));
    assert((arange(7, -3, -7) == vector{7, 0}));
    assert((arange(7, -3, 7).empty()));
    assert((arange(7, -3, -11) == vector{7}));
    assert((arange(7, -3, 11).empty()));
    assert((arange(7, 3, -2) == vector{7, 5}));
    assert((arange(7, 3, 2).empty()));
    assert((arange(7, 3, -3) == vector{7, 4}));
    assert((arange(7, 3, 3).empty()));
    assert((arange(7, 3, -5) == vector{7}));
    assert((arange(7, 3, 5).empty()));
    assert((arange(7, 3, -7) == vector{7}));
    assert((arange(7, 3, 7).empty()));
    assert((arange(7, 3, -11) == vector{7}));
    assert((arange(7, 3, 11).empty()));
    assert((arange(7, -5, -2) == vector{7, 5, 3, 1, -1, -3}));
    assert((arange(7, -5, 2).empty()));
    assert((arange(7, -5, -3) == vector{7, 4, 1, -2}));
    assert((arange(7, -5, 3).empty()));
    assert((arange(7, -5, -5) == vector{7, 2, -3}));
    assert((arange(7, -5, 5).empty()));
    assert((arange(7, -5, -7) == vector{7, 0}));
    assert((arange(7, -5, 7).empty()));
    assert((arange(7, -5, -11) == vector{7, -4}));
    assert((arange(7, -5, 11).empty()));
    assert((arange(7, 5, -2) == vector{7}));
    assert((arange(7, 5, 2).empty()));
    assert((arange(7, 5, -3) == vector{7}));
    assert((arange(7, 5, 3).empty()));
    assert((arange(7, 5, -5) == vector{7}));
    assert((arange(7, 5, 5).empty()));
    assert((arange(7, 5, -7) == vector{7}));
    assert((arange(7, 5, 7).empty()));
    assert((arange(7, 5, -11) == vector{7}));
    assert((arange(7, 5, 11).empty()));
    assert((arange(7, -7, -2) == vector{7, 5, 3, 1, -1, -3, -5}));
    assert((arange(7, -7, 2).empty()));
    assert((arange(7, -7, -3) == vector{7, 4, 1, -2, -5}));
    assert((arange(7, -7, 3).empty()));
    assert((arange(7, -7, -5) == vector{7, 2, -3}));
    assert((arange(7, -7, 5).empty()));
    assert((arange(7, -7, -7) == vector{7, 0}));
    assert((arange(7, -7, 7).empty()));
    assert((arange(7, -7, -11) == vector{7, -4}));
    assert((arange(7, -7, 11).empty()));
    assert((arange(7, 7, -2).empty()));
    assert((arange(7, 7, 2).empty()));
    assert((arange(7, 7, -3).empty()));
    assert((arange(7, 7, 3).empty()));
    assert((arange(7, 7, -5).empty()));
    assert((arange(7, 7, 5).empty()));
    assert((arange(7, 7, -7).empty()));
    assert((arange(7, 7, 7).empty()));
    assert((arange(7, 7, -11).empty()));
    assert((arange(7, 7, 11).empty()));
    assert((arange(7, -11, -2) == vector{7, 5, 3, 1, -1, -3, -5, -7, -9}));
    assert((arange(7, -11, 2).empty()));
    assert((arange(7, -11, -3) == vector{7, 4, 1, -2, -5, -8}));
    assert((arange(7, -11, 3).empty()));
    assert((arange(7, -11, -5) == vector{7, 2, -3, -8}));
    assert((arange(7, -11, 5).empty()));
    assert((arange(7, -11, -7) == vector{7, 0, -7}));
    assert((arange(7, -11, 7).empty()));
    assert((arange(7, -11, -11) == vector{7, -4}));
    assert((arange(7, -11, 11).empty()));
    assert((arange(7, 11, -2).empty()));
    assert((arange(7, 11, 2) == vector{7, 9}));
    assert((arange(7, 11, -3).empty()));
    assert((arange(7, 11, 3) == vector{7, 10}));
    assert((arange(7, 11, -5).empty()));
    assert((arange(7, 11, 5) == vector{7}));
    assert((arange(7, 11, -7).empty()));
    assert((arange(7, 11, 7) == vector{7}));
    assert((arange(7, 11, -11).empty()));
    assert((arange(7, 11, 11) == vector{7}));
    assert((arange(-11, -2, -2).empty()));
    assert((arange(-11, -2, 2) == vector{-11, -9, -7, -5, -3}));
    assert((arange(-11, -2, -3).empty()));
    assert((arange(-11, -2, 3) == vector{-11, -8, -5}));
    assert((arange(-11, -2, -5).empty()));
    assert((arange(-11, -2, 5) == vector{-11, -6}));
    assert((arange(-11, -2, -7).empty()));
    assert((arange(-11, -2, 7) == vector{-11, -4}));
    assert((arange(-11, -2, -11).empty()));
    assert((arange(-11, -2, 11) == vector{-11}));
    assert((arange(-11, 2, -2).empty()));
    assert((arange(-11, 2, 2) == vector{-11, -9, -7, -5, -3, -1, 1}));
    assert((arange(-11, 2, -3).empty()));
    assert((arange(-11, 2, 3) == vector{-11, -8, -5, -2, 1}));
    assert((arange(-11, 2, -5).empty()));
    assert((arange(-11, 2, 5) == vector{-11, -6, -1}));
    assert((arange(-11, 2, -7).empty()));
    assert((arange(-11, 2, 7) == vector{-11, -4}));
    assert((arange(-11, 2, -11).empty()));
    assert((arange(-11, 2, 11) == vector{-11, 0}));
    assert((arange(-11, -3, -2).empty()));
    assert((arange(-11, -3, 2) == vector{-11, -9, -7, -5}));
    assert((arange(-11, -3, -3).empty()));
    assert((arange(-11, -3, 3) == vector{-11, -8, -5}));
    assert((arange(-11, -3, -5).empty()));
    assert((arange(-11, -3, 5) == vector{-11, -6}));
    assert((arange(-11, -3, -7).empty()));
    assert((arange(-11, -3, 7) == vector{-11, -4}));
    assert((arange(-11, -3, -11).empty()));
    assert((arange(-11, -3, 11) == vector{-11}));
    assert((arange(-11, 3, -2).empty()));
    assert((arange(-11, 3, 2) == vector{-11, -9, -7, -5, -3, -1, 1}));
    assert((arange(-11, 3, -3).empty()));
    assert((arange(-11, 3, 3) == vector{-11, -8, -5, -2, 1}));
    assert((arange(-11, 3, -5).empty()));
    assert((arange(-11, 3, 5) == vector{-11, -6, -1}));
    assert((arange(-11, 3, -7).empty()));
    assert((arange(-11, 3, 7) == vector{-11, -4}));
    assert((arange(-11, 3, -11).empty()));
    assert((arange(-11, 3, 11) == vector{-11, 0}));
    assert((arange(-11, -5, -2).empty()));
    assert((arange(-11, -5, 2) == vector{-11, -9, -7}));
    assert((arange(-11, -5, -3).empty()));
    assert((arange(-11, -5, 3) == vector{-11, -8}));
    assert((arange(-11, -5, -5).empty()));
    assert((arange(-11, -5, 5) == vector{-11, -6}));
    assert((arange(-11, -5, -7).empty()));
    assert((arange(-11, -5, 7) == vector{-11}));
    assert((arange(-11, -5, -11).empty()));
    assert((arange(-11, -5, 11) == vector{-11}));
    assert((arange(-11, 5, -2).empty()));
    assert((arange(-11, 5, 2) == vector{-11, -9, -7, -5, -3, -1, 1, 3}));
    assert((arange(-11, 5, -3).empty()));
    assert((arange(-11, 5, 3) == vector{-11, -8, -5, -2, 1, 4}));
    assert((arange(-11, 5, -5).empty()));
    assert((arange(-11, 5, 5) == vector{-11, -6, -1, 4}));
    assert((arange(-11, 5, -7).empty()));
    assert((arange(-11, 5, 7) == vector{-11, -4, 3}));
    assert((arange(-11, 5, -11).empty()));
    assert((arange(-11, 5, 11) == vector{-11, 0}));
    assert((arange(-11, -7, -2).empty()));
    assert((arange(-11, -7, 2) == vector{-11, -9}));
    assert((arange(-11, -7, -3).empty()));
    assert((arange(-11, -7, 3) == vector{-11, -8}));
    assert((arange(-11, -7, -5).empty()));
    assert((arange(-11, -7, 5) == vector{-11}));
    assert((arange(-11, -7, -7).empty()));
    assert((arange(-11, -7, 7) == vector{-11}));
    assert((arange(-11, -7, -11).empty()));
    assert((arange(-11, -7, 11) == vector{-11}));
    assert((arange(-11, 7, -2).empty()));
    assert((arange(-11, 7, 2) == vector{-11, -9, -7, -5, -3, -1, 1, 3, 5}));
    assert((arange(-11, 7, -3).empty()));
    assert((arange(-11, 7, 3) == vector{-11, -8, -5, -2, 1, 4}));
    assert((arange(-11, 7, -5).empty()));
    assert((arange(-11, 7, 5) == vector{-11, -6, -1, 4}));
    assert((arange(-11, 7, -7).empty()));
    assert((arange(-11, 7, 7) == vector{-11, -4, 3}));
    assert((arange(-11, 7, -11).empty()));
    assert((arange(-11, 7, 11) == vector{-11, 0}));
    assert((arange(-11, -11, -2).empty()));
    assert((arange(-11, -11, 2).empty()));
    assert((arange(-11, -11, -3).empty()));
    assert((arange(-11, -11, 3).empty()));
    assert((arange(-11, -11, -5).empty()));
    assert((arange(-11, -11, 5).empty()));
    assert((arange(-11, -11, -7).empty()));
    assert((arange(-11, -11, 7).empty()));
    assert((arange(-11, -11, -11).empty()));
    assert((arange(-11, -11, 11).empty()));
    assert((arange(-11, 11, -2).empty()));
    assert((arange(-11, 11, 2) == vector{-11, -9, -7, -5, -3, -1, 1, 3, 5, 7, 9}));
    assert((arange(-11, 11, -3).empty()));
    assert((arange(-11, 11, 3) == vector{-11, -8, -5, -2, 1, 4, 7, 10}));
    assert((arange(-11, 11, -5).empty()));
    assert((arange(-11, 11, 5) == vector{-11, -6, -1, 4, 9}));
    assert((arange(-11, 11, -7).empty()));
    assert((arange(-11, 11, 7) == vector{-11, -4, 3, 10}));
    assert((arange(-11, 11, -11).empty()));
    assert((arange(-11, 11, 11) == vector{-11, 0}));
    assert((arange(11, -2, -2) == vector{11, 9, 7, 5, 3, 1, -1}));
    assert((arange(11, -2, 2).empty()));
    assert((arange(11, -2, -3) == vector{11, 8, 5, 2, -1}));
    assert((arange(11, -2, 3).empty()));
    assert((arange(11, -2, -5) == vector{11, 6, 1}));
    assert((arange(11, -2, 5).empty()));
    assert((arange(11, -2, -7) == vector{11, 4}));
    assert((arange(11, -2, 7).empty()));
    assert((arange(11, -2, -11) == vector{11, 0}));
    assert((arange(11, -2, 11).empty()));
    assert((arange(11, 2, -2) == vector{11, 9, 7, 5, 3}));
    assert((arange(11, 2, 2).empty()));
    assert((arange(11, 2, -3) == vector{11, 8, 5}));
    assert((arange(11, 2, 3).empty()));
    assert((arange(11, 2, -5) == vector{11, 6}));
    assert((arange(11, 2, 5).empty()));
    assert((arange(11, 2, -7) == vector{11, 4}));
    assert((arange(11, 2, 7).empty()));
    assert((arange(11, 2, -11) == vector{11}));
    assert((arange(11, 2, 11).empty()));
    assert((arange(11, -3, -2) == vector{11, 9, 7, 5, 3, 1, -1}));
    assert((arange(11, -3, 2).empty()));
    assert((arange(11, -3, -3) == vector{11, 8, 5, 2, -1}));
    assert((arange(11, -3, 3).empty()));
    assert((arange(11, -3, -5) == vector{11, 6, 1}));
    assert((arange(11, -3, 5).empty()));
    assert((arange(11, -3, -7) == vector{11, 4}));
    assert((arange(11, -3, 7).empty()));
    assert((arange(11, -3, -11) == vector{11, 0}));
    assert((arange(11, -3, 11).empty()));
    assert((arange(11, 3, -2) == vector{11, 9, 7, 5}));
    assert((arange(11, 3, 2).empty()));
    assert((arange(11, 3, -3) == vector{11, 8, 5}));
    assert((arange(11, 3, 3).empty()));
    assert((arange(11, 3, -5) == vector{11, 6}));
    assert((arange(11, 3, 5).empty()));
    assert((arange(11, 3, -7) == vector{11, 4}));
    assert((arange(11, 3, 7).empty()));
    assert((arange(11, 3, -11) == vector{11}));
    assert((arange(11, 3, 11).empty()));
    assert((arange(11, -5, -2) == vector{11, 9, 7, 5, 3, 1, -1, -3}));
    assert((arange(11, -5, 2).empty()));
    assert((arange(11, -5, -3) == vector{11, 8, 5, 2, -1, -4}));
    assert((arange(11, -5, 3).empty()));
    assert((arange(11, -5, -5) == vector{11, 6, 1, -4}));
    assert((arange(11, -5, 5).empty()));
    assert((arange(11, -5, -7) == vector{11, 4, -3}));
    assert((arange(11, -5, 7).empty()));
    assert((arange(11, -5, -11) == vector{11, 0}));
    assert((arange(11, -5, 11).empty()));
    assert((arange(11, 5, -2) == vector{11, 9, 7}));
    assert((arange(11, 5, 2).empty()));
    assert((arange(11, 5, -3) == vector{11, 8}));
    assert((arange(11, 5, 3).empty()));
    assert((arange(11, 5, -5) == vector{11, 6}));
    assert((arange(11, 5, 5).empty()));
    assert((arange(11, 5, -7) == vector{11}));
    assert((arange(11, 5, 7).empty()));
    assert((arange(11, 5, -11) == vector{11}));
    assert((arange(11, 5, 11).empty()));
    assert((arange(11, -7, -2) == vector{11, 9, 7, 5, 3, 1, -1, -3, -5}));
    assert((arange(11, -7, 2).empty()));
    assert((arange(11, -7, -3) == vector{11, 8, 5, 2, -1, -4}));
    assert((arange(11, -7, 3).empty()));
    assert((arange(11, -7, -5) == vector{11, 6, 1, -4}));
    assert((arange(11, -7, 5).empty()));
    assert((arange(11, -7, -7) == vector{11, 4, -3}));
    assert((arange(11, -7, 7).empty()));
    assert((arange(11, -7, -11) == vector{11, 0}));
    assert((arange(11, -7, 11).empty()));
    assert((arange(11, 7, -2) == vector{11, 9}));
    assert((arange(11, 7, 2).empty()));
    assert((arange(11, 7, -3) == vector{11, 8}));
    assert((arange(11, 7, 3).empty()));
    assert((arange(11, 7, -5) == vector{11}));
    assert((arange(11, 7, 5).empty()));
    assert((arange(11, 7, -7) == vector{11}));
    assert((arange(11, 7, 7).empty()));
    assert((arange(11, 7, -11) == vector{11}));
    assert((arange(11, 7, 11).empty()));
    assert((arange(11, -11, -2) == vector{11, 9, 7, 5, 3, 1, -1, -3, -5, -7, -9}));
    assert((arange(11, -11, 2).empty()));
    assert((arange(11, -11, -3) == vector{11, 8, 5, 2, -1, -4, -7, -10}));
    assert((arange(11, -11, 3).empty()));
    assert((arange(11, -11, -5) == vector{11, 6, 1, -4, -9}));
    assert((arange(11, -11, 5).empty()));
    assert((arange(11, -11, -7) == vector{11, 4, -3, -10}));
    assert((arange(11, -11, 7).empty()));
    assert((arange(11, -11, -11) == vector{11, 0}));
    assert((arange(11, -11, 11).empty()));
    assert((arange(11, 11, -2).empty()));
    assert((arange(11, 11, 2).empty()));
    assert((arange(11, 11, -3).empty()));
    assert((arange(11, 11, 3).empty()));
    assert((arange(11, 11, -5).empty()));
    assert((arange(11, 11, 5).empty()));
    assert((arange(11, 11, -7).empty()));
    assert((arange(11, 11, 7).empty()));
    assert((arange(11, 11, -11).empty()));
    assert((arange(11, 11, 11).empty()));
}

}  // namespace

int main() {
    test_general_asserts();
    test_isqrt();
    test_icbrt();
    test_log2();
    test_bit_reverse();
#if defined(HAS_MPFR_DURING_TESTING) && HAS_MPFR_DURING_TESTING
    test_sin_cos_sum();
#endif
    test_visit_all_submasks();
    test_prime_bitarrays();
    test_factorizer();
    test_extended_euclid_algorithm();
    test_solve_congruence_modulo_m_all_roots();
    test_inv_mod_m();
    test_solve_binary_congruence_modulo_m();
    test_solve_factorial_congruence();
    test_powers_sum();
    test_arange();
}
