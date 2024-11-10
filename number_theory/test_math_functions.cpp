#include <algorithm>
#include <array>
#include <atomic>
#include <bitset>
#include <cassert>
#include <cinttypes>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <exception>
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
#include <vector>

#include "config_macros.hpp"
#include "math_functions.hpp"
#if CONFIG_HAS_INCLUDE("integers_128_bit.hpp")
#include "integers_128_bit.hpp"
#endif
#include "test_tools.hpp"

#if CONFIG_HAS_AT_LEAST_CXX_20
#include <bit>
#endif

#if CONFIG_HAS_INCLUDE(<mpfr.h>)
#include <mpfr.h>
#define HAS_MPFR_DURING_TESTING
#else
#if defined(__linux__) && !defined(__MINGW32__)
#error "mpfr tests should be available on linux"
#endif
#endif

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace math_functions;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace test_tools;
using std::gcd;
using std::size_t;
using std::uint32_t;
using std::uint64_t;

// clang-format off
// NOLINTBEGIN(cert-dcl03-c, misc-static-assert, hicpp-static-assert, cppcoreguidelines-avoid-magic-numbers, cert-msc32-c, cert-msc51-cpp)
// clang-format on

namespace {

void test_isqrt() noexcept {
    log_tests_started();

    constexpr uint32_t kIters = 2'000'000;

    constexpr auto test_sqrts = [](uint32_t n, uint32_t n_squared) {
        assert(n == isqrt(n_squared));
        assert(n == isqrt(uint64_t{n_squared}));
        assert(n == isqrt(uint128_t{n_squared}));
    };

    constexpr uint32_t kProbes = 100'000;
    for (uint32_t i = 0; i <= std::min(kProbes, 65535U); i++) {
        const uint32_t i_square      = i * i;
        const uint32_t next_i_square = (i + 1) * (i + 1);
        for (uint32_t j = i_square; j != next_i_square; j++) {
            test_sqrts(i, j);
        }
    }

    for (uint32_t r = std::numeric_limits<uint32_t>::max() - kIters; r != 0; r++) {
        const uint64_t rs = uint64_t{r} * r;
        assert(r - 1 == isqrt(rs - 1));
        assert(r == isqrt(rs));
        assert(r == isqrt(rs + 1));
        assert(r - 1 == isqrt(uint128_t{rs - 1}));
        assert(r == isqrt(uint128_t{rs}));
        assert(r == isqrt(uint128_t{rs + 1}));
    }

    for (uint64_t r = std::numeric_limits<uint64_t>::max() - kIters; r != 0; r++) {
        const uint128_t rs = uint128_t{r} * r;
        assert(r - 1 == isqrt(uint128_t{rs - 1}));
        assert(r == isqrt(rs));
        assert(r == isqrt(rs + 1));
    }

    constexpr std::array<std::pair<uint64_t, uint128_t>, 10> root_with_square = {{
        {uint64_t{std::numeric_limits<uint8_t>::max()},
         uint128_t{std::numeric_limits<uint16_t>::max()}},
        {uint64_t{std::numeric_limits<uint16_t>::max()},
         uint128_t{std::numeric_limits<uint32_t>::max()}},
        {uint64_t{std::numeric_limits<uint32_t>::max()},
         uint128_t{std::numeric_limits<uint64_t>::max()}},
        {uint64_t{8347849ULL}, uint128_t{8347849ULL} * 8347849ULL},
        {uint64_t{23896778463ULL}, uint128_t{23896778463ULL} * 23896778463ULL},
        {uint64_t{26900711288786ULL},
         uint128_t{72364826784263874ULL} * 10'000'000'000ULL + 2'638'723'478},
        {uint64_t{3748237487274238478ULL},
         uint128_t{3748237487274238478ULL} * 3748237487274238478ULL},
        {uint64_t{9472294799293ULL}, uint128_t{8972436876473126137ULL} * 10'000'000ULL + 7'236'478},
        {uint64_t{18015752134763552034ULL},
         (uint128_t{17594829943123320651ULL} << 64U) | 2622055845271657274ULL},
        {std::numeric_limits<uint64_t>::max(), static_cast<uint128_t>(-1)},
    }};
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
    assert(icbrt(1625U * 1625U * 1625U) == 1625);
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
        const uint32_t pw = uint32_t{1} << k;
        assert(log2_floor(pw) == k);
        assert(log2_ceil(pw) == k);
        if (!is_power_of_two(pw + 1)) {
            assert(log2_floor(pw + 1) == k);
            assert(log2_ceil(pw + 1) == k + 1);
        }
    }

    for (uint32_t k = 0; k < sizeof(uint64_t) * CHAR_BIT; k++) {
        const uint64_t pw = uint64_t{1} << k;
        assert(log2_floor(pw) == k);
        assert(log2_ceil(pw) == k);
        if (!is_power_of_two(pw + 1)) {
            assert(log2_floor(pw + 1) == k);
            assert(log2_ceil(pw + 1) == k + 1);
        }
    }

    for (uint32_t k = 0; k < sizeof(uint128_t) * CHAR_BIT; k++) {
        const uint128_t pw = uint128_t{1} << k;
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
        assert(bit_reverse(static_cast<uint8_t>(n)) == (bit_reverse(n) >> 24U));
    }

    constexpr std::array<uint32_t, 32> shifts = {
        2U,  3U,  5U,  7U,  11U, 13U, 17U, 19U, 23U, 29U,  31U,  37U,  41U,  43U,  47U,  53U,
        59U, 61U, 67U, 71U, 73U, 79U, 83U, 89U, 97U, 101U, 103U, 107U, 109U, 113U, 127U, 131U,
    };
    uint128_t n = std::numeric_limits<uint64_t>::max();
    for (auto k = static_cast<uint32_t>(1e7); k > 0; k--) {
        const auto hi_64  = static_cast<uint64_t>(n >> 64U);
        const auto low_64 = static_cast<uint64_t>(n);
        const uint128_t b = (uint128_t{bit_reverse(low_64)} << 64U) | bit_reverse(hi_64);
        assert(bit_reverse(n) == b);
        n += shifts[k % 32];
    }
}

#if defined(HAS_MPFR_DURING_TESTING)

// NOLINTNEXTLINE(cert-err58-cpp)
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
    constexpr double angle_start   = bin_pow(double{angle_scale}, -std::ptrdiff_t{k});

    constexpr FloatType kSumEps = []() constexpr noexcept -> FloatType {
        if constexpr (std::is_same_v<FloatType, float>) {
            return 0.4F;
        } else if constexpr (std::is_same_v<FloatType, double>) {
            return 0.0000001;
        } else {
            return 0.0000001L;
        }
    }();

    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)

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

    // NOLINTEND(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
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
    constexpr std::array primes           = {
        2U,   3U,   5U,   7U,    11U,  13U,  17U,  19U,  23U,  29U,  31U,  37U,  41U,  43U,  47U,
        53U,  59U,  61U,  67U,   71U,  73U,  79U,  83U,  89U,  97U,  101U, 103U, 107U, 109U, 113U,
        127U, 131U, 137U, 139U,  149U, 151U, 157U, 163U, 167U, 173U, 179U, 181U, 191U, 193U, 197U,
        199U, 211U, 223U, 227U,  229U, 233U, 239U, 241U, 251U, 257U, 263U, 269U, 271U, 277U, 281U,
        283U, 293U, 307U, 311U,  313U, 317U, 331U, 337U, 347U, 349U, 353U, 359U, 367U, 373U, 379U,
        383U, 389U, 397U, 401U,  409U, 419U, 421U, 431U, 433U, 439U, 443U, 449U, 457U, 461U, 463U,
        467U, 479U, 487U, 491U,  499U, 503U, 509U, 521U, 523U, 541U, 547U, 557U, 563U, 569U, 571U,
        577U, 587U, 593U, 599U,  601U, 607U, 613U, 617U, 619U, 631U, 641U, 643U, 647U, 653U, 659U,
        661U, 673U, 677U, 683U,  691U, 701U, 709U, 719U, 727U, 733U, 739U, 743U, 751U, 757U, 761U,
        769U, 773U, 787U, 797U,  809U, 811U, 821U, 823U, 827U, 829U, 839U, 853U, 857U, 859U, 863U,
        877U, 881U, 883U, 887U,  907U, 911U, 919U, 929U, 937U, 941U, 947U, 953U, 967U, 971U, 977U,
        983U, 991U, 997U, 1009U,
    };
    static_assert(primes.back() >= N, "Test implementation error: add mode prime numbers");
#if CONFIG_HAS_AT_LEAST_CXX_20
    static_assert(std::ranges::is_sorted(primes));
#endif
    size_t i = 0;
    assert(primes_as_bvector.size() == N + 1);
    for (uint32_t n = 0; n <= N; n++) {
        if (primes[i] < n) {
            i++;
        }
        const bool is_prime = primes[i] == n;
        assert(primes_as_bvector[n] == is_prime);
        assert(primes_bset[n] == is_prime);
    }
}

void test_factorizer() {
    log_tests_started();

    constexpr auto N = static_cast<uint32_t>(1e7);
    Factorizer const fact(N);
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

// NOLINTBEGIN(performance-avoid-endl)

template <class IntType>
[[nodiscard]] bool test_extended_euclid_algorithm_a_b(std::size_t thread_id, IntType a,
                                                      IntType b) noexcept {
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

    const bool valid_u_range = (b == 0 && u == ::math_functions::sign(a)) ^
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

    const bool valid_v_range = (a == 0 && v == ::math_functions::sign(b)) ^
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

    constexpr std::size_t kTotalTests     = 1ULL << 30U;
    constexpr std::size_t kTotalThreads   = 4;
    constexpr std::size_t kTestsPerThread = kTotalTests / kTotalThreads;

    std::vector<std::thread> threads;
    threads.reserve(kTotalThreads);
    std::atomic_flag result = ATOMIC_FLAG_INIT;
    for (size_t i = 0; i < kTotalThreads; ++i) {
        threads.emplace_back([thread_id = i, &result
#if defined(_MSC_VER) && !defined(__clang__)
                              ,
                              kTestsPerThread
#endif
        ]() {
            const std::size_t seed = thread_id * 3'829'234'734UL + 27'273'489;
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

// NOLINTEND(performance-avoid-endl)

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
    std::printf("Seed: %" PRIuFAST32 "\n", seed);
    std::mt19937 rnd_32(seed);

    constexpr auto kTotalTests = size_t{1} << 25U;
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

    using SeedType      = typename std::ranlux24::result_type;
    const SeedType seed = std::ranlux24(static_cast<SeedType>(std::time(nullptr)))();
    std::printf("Seed: %" PRIuFAST32 "\n", seed);
    std::mt19937 rnd_32(seed);

    constexpr auto kTotalTests = size_t{1} << 25U;
    for (auto test_iter = kTotalTests; test_iter > 0; --test_iter) {
        const uint32_t m = static_cast<std::uint32_t>(rnd_32());
        if (unlikely(m == 0)) {
            continue;
        }

        const auto k = std::uniform_int_distribution<uint32_t>(
            0, std::numeric_limits<uint16_t>::max())(rnd_32);
        const uint32_t c  = static_cast<uint32_t>(rnd_32());
        const auto x      = solve_binary_congruence_modulo_m(k, c, m);
        const auto [r, s] = math_functions::extract_pow2(m);
        assert(r % 2 == 1 && r << s == m);
        const auto gcd_2k_m = uint32_t{1} << std::min(k, s);
        if (c % gcd_2k_m != 0) {
            assert(x == math_functions::kNoCongruenceSolution);
        } else {
            assert(x < m);
            // (2^{k} * x) % m
            const uint64_t prod_of_2k_x =
                (uint64_t{bin_pow_mod(uint32_t{2}, k, m)} * uint64_t{x}) % m;
            assert(prod_of_2k_x == c % m);
        }

        assert(x == solve_congruence_modulo_m(bin_pow_mod(2U, k, m), c, m));
    }
}

void test_inv_mod_m() {
    log_tests_started();

    constexpr std::array<uint32_t, 64> first_prime_nums = {
        2,   3,   5,   7,   11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,  53,
        59,  61,  67,  71,  73,  79,  83,  89,  97,  101, 103, 107, 109, 113, 127, 131,
        137, 139, 157, 149, 151, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223,
        227, 229, 233, 239, 241, 257, 251, 263, 269, 271, 277, 281, 283, 293, 307, 311,
    };

    for (const uint32_t m : first_prime_nums) {
        for (uint32_t a = 1; a < m; a++) {
            const uint32_t a_inv = math_functions::inv_mod_m(a, m);
            assert(a_inv < m);
            assert((a * uint64_t{a_inv}) % m == 1);
        }
    }

    constexpr std::array<uint32_t, 9> big_prime_nums = {
        2147483489U, 2147483497U, 2147483543U, 2147483549U, 2147483563U,
        2147483579U, 2147483587U, 2147483629U, 2147483647U,
    };
    constexpr uint32_t kLimit = 100;
    for (const uint32_t m : big_prime_nums) {
        for (uint32_t a = 1; a < std::min(m, kLimit); a++) {
            const uint32_t a_inv = math_functions::inv_mod_m(a, m);
            assert(a_inv < m);
            assert((a * uint64_t{a_inv}) % m == 1);
        }
        for (uint32_t a = m - kLimit; a < m; a++) {
            const uint32_t a_inv = math_functions::inv_mod_m(a, m);
            assert(a_inv < m);
            assert((a * uint64_t{a_inv}) % m == 1);
        }
    }

    for (uint32_t m = 3; m <= 1000; m++) {
        auto n_pfs = math_functions::prime_factors_as_vector(m);
        for (uint32_t a = 1; a < m; a++) {
            const bool are_coprime = std::gcd(a, m) == 1;
            assert(are_coprime ==
                   std::all_of(n_pfs.begin(), n_pfs.end(),
                               [a](auto pf) constexpr noexcept { return a % pf.factor != 0; }));
            const uint32_t a_inv = math_functions::inv_mod_m(a, m);
            if (!are_coprime) {
                assert(a_inv == math_functions::kNoCongruenceSolution);
                continue;
            }

            assert(a_inv < m);
            assert(a_inv != math_functions::kNoCongruenceSolution);
            assert((a * uint64_t{a_inv}) % m == 1);
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
    assert(solve_factorial_congruence(0xffffffffU, 2) == 4294967263U);

    static_assert(1ULL * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9 * 10 * 11 * 12 <=
                  std::numeric_limits<uint32_t>::max());
    static_assert(1ULL * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9 * 10 * 11 * 12 * 13 >
                  std::numeric_limits<uint32_t>::max());

    uint32_t n_fact = 1;
    for (uint32_t n = 1; n <= 12; n++) {
        n_fact *= n;
        const uint32_t k = n_fact + 1;
        assert(solve_factorial_congruence(n, k) == 0);
    }
}

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size)
void test_general_asserts() noexcept {
    // NOLINTBEGIN(cppcoreguidelines-macro-usage)

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

    // NOLINTEND(cppcoreguidelines-macro-usage)

    ASSERT_THAT(bin_pow_mod(uint32_t{7}, uint32_t{483}, uint32_t{1000000007U}) == 263145387U);
    ASSERT_THAT(bin_pow_mod(uint32_t{289}, std::numeric_limits<uint32_t>::max(),
                            uint32_t{2146514599U}) == 1349294778U);
    ASSERT_THAT(bin_pow_mod(uint32_t{2146526839U}, uint32_t{578423432U}, uint32_t{2147483629U}) ==
                281853233U);

#if defined(HAS_I128_CONSTEXPR)
    ASSERT_THAT(bin_pow_mod(uint64_t{7}, uint64_t{483}, uint64_t{1000000007U}) == 263145387U);
    ASSERT_THAT(bin_pow_mod(uint64_t{289}, uint64_t{std::numeric_limits<uint32_t>::max()},
                            uint64_t{2146514599U}) == 1349294778U);
    ASSERT_THAT(bin_pow_mod(uint64_t{2146526839U}, uint64_t{578423432U}, uint64_t{2147483629U}) ==
                281853233U);
    ASSERT_THAT(bin_pow_mod(uint64_t{119999999927ULL}, uint64_t{18446744073709515329ULL},
                            uint64_t{100000000000000003ULL}) == 85847679703545452ULL);
    ASSERT_THAT(bin_pow_mod(uint64_t{72057594037927843ULL}, uint64_t{18446744073709515329ULL},
                            uint64_t{1000000000000000003ULL}) == 404835689235904145ULL);
    ASSERT_THAT(bin_pow_mod(uint64_t{999999999999999487ULL}, uint64_t{18446744073709551557ULL},
                            uint64_t{1000000000000000009ULL}) == 802735487082721113ULL);

    ASSERT_THAT(bin_pow_mod(uint64_t{2}, uint64_t{18446744073709551427ULL} - 1,
                            uint64_t{18446744073709551427ULL}) == 1);
    ASSERT_THAT(bin_pow_mod(uint64_t{3}, uint64_t{18446744073709551427ULL} - 1,
                            uint64_t{18446744073709551427ULL}) == 1);
    ASSERT_THAT(bin_pow_mod(uint64_t{1238873}, uint64_t{18446744073709551427ULL} - 1,
                            uint64_t{18446744073709551427ULL}) == 1);
#endif

#if CONFIG_HAS_AT_LEAST_CXX_20

    ASSERT_THAT(isqrt(0U) == 0);
    ASSERT_THAT(isqrt(1U) == 1);
    ASSERT_THAT(isqrt(4U) == 2);
    ASSERT_THAT(isqrt(9U) == 3);
    ASSERT_THAT(isqrt(10U) == 3);
    ASSERT_THAT(isqrt(15U) == 3);
    ASSERT_THAT(isqrt(16U) == 4);
    ASSERT_THAT(isqrt(257U * 257U) == 257);
    ASSERT_THAT(isqrt(257U * 257U + 1) == 257);
    ASSERT_THAT(isqrt(258U * 258U - 1U) == 257);
    ASSERT_THAT(isqrt(1U << 12U) == 1U << 6U);
    ASSERT_THAT(isqrt(1U << 14U) == 1U << 7U);
    ASSERT_THAT(isqrt(1U << 16U) == 1U << 8U);
    ASSERT_THAT(isqrt(1U << 28U) == 1U << 14U);
    ASSERT_THAT(isqrt(1U << 30U) == 1U << 15U);
    ASSERT_THAT(isqrt(std::numeric_limits<uint32_t>::max()) == (1U << 16U) - 1);

#endif

    ASSERT_THAT(isqrt(uint64_t{0}) == 0);
    ASSERT_THAT(isqrt(uint64_t{1}) == 1);
    ASSERT_THAT(isqrt(uint64_t{4}) == 2);
    ASSERT_THAT(isqrt(uint64_t{9}) == 3);
    ASSERT_THAT(isqrt(uint64_t{10}) == 3);
    ASSERT_THAT(isqrt(uint64_t{15}) == 3);
    ASSERT_THAT(isqrt(uint64_t{16}) == 4);
    ASSERT_THAT(isqrt(uint64_t{257} * 257) == 257);
    ASSERT_THAT(isqrt(uint64_t{257} * 257 + 1) == 257);
    ASSERT_THAT(isqrt(uint64_t{258} * 258 - 1) == 257);
    ASSERT_THAT(isqrt(uint64_t{1000000007} * 1000000007) == 1000000007U);
    ASSERT_THAT(isqrt(uint64_t{1} << 12U) == uint64_t{1} << 6U);
    ASSERT_THAT(isqrt(uint64_t{1} << 14U) == uint64_t{1} << 7U);
    ASSERT_THAT(isqrt(uint64_t{1} << 16U) == uint64_t{1} << 8U);
    ASSERT_THAT(isqrt(uint64_t{1} << 28U) == uint64_t{1} << 14U);
    ASSERT_THAT(isqrt(uint64_t{1} << 30U) == uint64_t{1} << 15U);
    ASSERT_THAT(isqrt(uint64_t{1} << 54U) == uint64_t{1} << 27U);
    ASSERT_THAT(isqrt(uint64_t{1} << 56U) == uint64_t{1} << 28U);
    ASSERT_THAT(isqrt(uint64_t{1} << 58U) == uint64_t{1} << 29U);
    ASSERT_THAT(isqrt(uint64_t{1} << 60U) == uint64_t{1} << 30U);
    ASSERT_THAT(isqrt(uint64_t{1} << 62U) == uint64_t{1} << 31U);
    ASSERT_THAT(isqrt(std::numeric_limits<uint64_t>::max()) == 0xFFFFFFFFU);

#if defined(INTEGERS_128_BIT_HPP) && defined(HAS_I128_CONSTEXPR)

    ASSERT_THAT(isqrt(uint128_t{0}) == 0);
    ASSERT_THAT(isqrt(uint128_t{1}) == 1);
    ASSERT_THAT(isqrt(uint128_t{4}) == 2);
    ASSERT_THAT(isqrt(uint128_t{9}) == 3);
    ASSERT_THAT(isqrt(uint128_t{10}) == 3);
    ASSERT_THAT(isqrt(uint128_t{15}) == 3);
    ASSERT_THAT(isqrt(uint128_t{16}) == 4);
    ASSERT_THAT(isqrt(uint128_t{257} * 257) == 257);
    ASSERT_THAT(isqrt(uint128_t{257} * 257 + 1) == 257);
    ASSERT_THAT(isqrt(uint128_t{258} * 258 - 1) == 257);
    ASSERT_THAT(isqrt(uint128_t{1} << 12U) == uint64_t{1} << 6U);
    ASSERT_THAT(isqrt(uint128_t{1} << 14U) == uint64_t{1} << 7U);
    ASSERT_THAT(isqrt(uint128_t{1} << 16U) == uint64_t{1} << 8U);
    ASSERT_THAT(isqrt(uint128_t{1} << 28U) == uint64_t{1} << 14U);
    ASSERT_THAT(isqrt(uint128_t{1} << 30U) == uint64_t{1} << 15U);
    ASSERT_THAT(isqrt(uint128_t{1} << 54U) == uint64_t{1} << 27U);
    ASSERT_THAT(isqrt(uint128_t{1} << 56U) == uint64_t{1} << 28U);
    ASSERT_THAT(isqrt(uint128_t{1} << 58U) == uint64_t{1} << 29U);
    ASSERT_THAT(isqrt(uint128_t{1} << 60U) == uint64_t{1} << 30U);
    ASSERT_THAT(isqrt(uint128_t{1} << 62U) == uint64_t{1} << 31U);
    ASSERT_THAT(isqrt(uint128_t{std::numeric_limits<uint64_t>::max()}) == (uint64_t{1} << 32U) - 1);
    ASSERT_THAT(isqrt(uint128_t{1} << 126U) == uint64_t{1} << 63U);
    ASSERT_THAT(isqrt(static_cast<uint128_t>(-1)) == (uint128_t{1} << 64U) - 1);
    ASSERT_THAT(isqrt(uint128_t{1000000007} * 1000000007) == 1000000007);
    ASSERT_THAT(isqrt(uint128_t{1000000000000000003ULL} * 1000000000000000003ULL) ==
                1000000000000000003ULL);
    ASSERT_THAT(isqrt(uint128_t{1000000000000000009ULL} * 1000000000000000009ULL) ==
                1000000000000000009ULL);
    ASSERT_THAT(isqrt(uint128_t{18446744073709551521ULL} * 18446744073709551521ULL) ==
                18446744073709551521ULL);
    ASSERT_THAT(isqrt(uint128_t{18446744073709551533ULL} * 18446744073709551533ULL) ==
                18446744073709551533ULL);
    ASSERT_THAT(isqrt(uint128_t{18446744073709551557ULL} * 18446744073709551557ULL) ==
                18446744073709551557ULL);
    ASSERT_THAT(isqrt(uint128_t{18446744073709551557ULL} * 18446744073709551557ULL + 1) ==
                18446744073709551557ULL);
    ASSERT_THAT(isqrt(uint128_t{18446744073709551558ULL} * 18446744073709551558ULL - 1) ==
                18446744073709551557ULL);
    ASSERT_THAT(isqrt(uint128_t{18446744073709551558ULL} * 18446744073709551558ULL) ==
                18446744073709551558ULL);
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
    ASSERT_THAT(icbrt(uint32_t{1} << 15U) == 1U << 5U);
    ASSERT_THAT(icbrt(uint32_t{1} << 18U) == 1U << 6U);
    ASSERT_THAT(icbrt(uint32_t{1} << 21U) == 1U << 7U);
    ASSERT_THAT(icbrt(uint32_t{1} << 24U) == 1U << 8U);
    ASSERT_THAT(icbrt(uint32_t{1} << 27U) == 1U << 9U);
    ASSERT_THAT(icbrt(uint32_t{1} << 30U) == 1U << 10U);
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
    ASSERT_THAT(icbrt(uint64_t{257} * 257 * 257 - 1) == 256);
    ASSERT_THAT(icbrt(uint64_t{257} * 257 * 257) == 257);
    ASSERT_THAT(icbrt(uint64_t{257} * 257 * 257 + 1) == 257);
    ASSERT_THAT(icbrt(uint64_t{258} * 258 * 258 - 1) == 257);
    ASSERT_THAT(icbrt(uint64_t{258} * 258 * 258) == 258);
    ASSERT_THAT(icbrt(uint64_t{258} * 258 * 258 + 1) == 258);
    ASSERT_THAT(icbrt(uint64_t{289} * 289 * 289) == 289);
    ASSERT_THAT(icbrt(uint64_t{289} * 289 * 289 + 1) == 289);
    ASSERT_THAT(icbrt(uint64_t{290} * 290 * 290 - 1) == 289);
    ASSERT_THAT(icbrt(uint64_t{290} * 290 * 290) == 290);
    ASSERT_THAT(icbrt(uint64_t{1} << 15U) == 1U << 5U);
    ASSERT_THAT(icbrt(uint64_t{1} << 18U) == 1U << 6U);
    ASSERT_THAT(icbrt(uint64_t{1} << 21U) == 1U << 7U);
    ASSERT_THAT(icbrt(uint64_t{1} << 24U) == 1U << 8U);
    ASSERT_THAT(icbrt(uint64_t{1} << 27U) == 1U << 9U);
    ASSERT_THAT(icbrt(uint64_t{1} << 30U) == 1U << 10U);
    ASSERT_THAT(icbrt(uint64_t{std::numeric_limits<uint32_t>::max()}) == 1625);
    ASSERT_THAT(icbrt(uint64_t{1} << 33U) == 1U << 11U);
    ASSERT_THAT(icbrt(uint64_t{1} << 36U) == 1U << 12U);
    ASSERT_THAT(icbrt(uint64_t{1} << 39U) == 1U << 13U);
    ASSERT_THAT(icbrt(uint64_t{1} << 42U) == 1U << 14U);
    ASSERT_THAT(icbrt(uint64_t{1} << 45U) == 1U << 15U);
    ASSERT_THAT(icbrt(uint64_t{1} << 48U) == 1U << 16U);
    ASSERT_THAT(icbrt(uint64_t{1} << 51U) == 1U << 17U);
    ASSERT_THAT(icbrt(uint64_t{1} << 54U) == 1U << 18U);
    ASSERT_THAT(icbrt(uint64_t{1} << 57U) == 1U << 19U);
    ASSERT_THAT(icbrt(uint64_t{1} << 60U) == 1U << 20U);
    ASSERT_THAT(icbrt(uint64_t{1} << 63U) == 1U << 21U);
    ASSERT_THAT(icbrt((uint64_t{1} << 63U) | (uint64_t{1} << 32U)) == 2097152);
    ASSERT_THAT(icbrt(uint64_t{1'367'631'000'000'000ULL}) == 111'000);
    ASSERT_THAT(icbrt(uint64_t{1'000'000'000'000'000'000ULL}) == 1'000'000);
    ASSERT_THAT(icbrt(uint64_t{1'331'000'000'000'000'000ULL}) == 1'100'000);
    ASSERT_THAT(icbrt(uint64_t{8'000'000'000'000'000'000ULL}) == 2'000'000);
    ASSERT_THAT(icbrt(uint64_t{15'625'000'000'000'000'000ULL}) == 2'500'000);
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
    ASSERT_THAT(is_perfect_square(uint64_t{1} << 16U));
    ASSERT_THAT(is_perfect_square(uint64_t{1} << 24U));
    ASSERT_THAT(is_perfect_square(uint64_t{1} << 32U));
    ASSERT_THAT(is_perfect_square(uint64_t{1} << 40U));
    ASSERT_THAT(is_perfect_square(uint64_t{1} << 48U));
    ASSERT_THAT(is_perfect_square(uint64_t{1} << 56U));
    ASSERT_THAT(is_perfect_square(uint64_t{1} << 60U));
    ASSERT_THAT(is_perfect_square(uint64_t{1} << 62U));

#if defined(HAS_I128_CONSTEXPR)
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
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 16U));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 24U));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 32U));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 40U));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 48U));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 56U));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 60U));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 62U));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 64U));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 66U));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 68U));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 70U));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 72U));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 74U));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 76U));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 78U));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 80U));
    ASSERT_THAT(is_perfect_square(uint128_t{1} << 126U));
#endif

    ASSERT_THAT(bit_reverse(uint8_t{0b00000000}) == 0b00000000);
    ASSERT_THAT(bit_reverse(uint8_t{0b00000010}) == 0b01000000);
    ASSERT_THAT(bit_reverse(uint8_t{0b00001100}) == 0b00110000);
    ASSERT_THAT(bit_reverse(uint8_t{0b10101010}) == 0b01010101);
    ASSERT_THAT(bit_reverse(uint8_t{0b01010101}) == 0b10101010);
    ASSERT_THAT(bit_reverse(uint8_t{0b11111111}) == 0b11111111);

    ASSERT_THAT(bit_reverse(0b00000000'00000000'00000000'00000000U) ==
                0b00000000'00000000'00000000'00000000U);
    ASSERT_THAT(bit_reverse(0b00000000'00000000'00000000'00000001U) ==
                0b10000000'00000000'00000000'00000000U);
    ASSERT_THAT(bit_reverse(0b10000000'00000000'00000000'00000000U) ==
                0b00000000'00000000'00000000'00000001U);
    ASSERT_THAT(bit_reverse(0b00000000'11111111'00000000'00000000U) ==
                0b00000000'00000000'11111111'00000000U);
    ASSERT_THAT(bit_reverse(0b00000000'00000000'11111111'00000000U) ==
                0b00000000'11111111'00000000'00000000U);
    ASSERT_THAT(bit_reverse(0b10101010'10101010'10101010'10101010U) ==
                0b01010101'01010101'01010101'01010101U);
    ASSERT_THAT(bit_reverse(0b11111111'00000000'11111111'00000000U) ==
                0b00000000'11111111'00000000'11111111U);

    ASSERT_THAT(
        bit_reverse(uint64_t{
            0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL}) ==
        0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL);
    ASSERT_THAT(
        bit_reverse(uint64_t{
            0b10000001'00000000'10000001'00000000'10000001'00000000'10000001'00000000ULL}) ==
        0b00000000'10000001'00000000'10000001'00000000'10000001'00000000'10000001ULL);
    ASSERT_THAT(
        bit_reverse(uint64_t{
            0b00001111'00000000'11110000'00000000'10101010'00000000'00000000'00000000ULL}) ==
        0b00000000'00000000'00000000'01010101'00000000'00001111'00000000'11110000ULL);
    ASSERT_THAT(
        bit_reverse(uint64_t{
            0b00000000'00000000'00000000'10101010'10101010'00000000'00000000'00000000ULL}) ==
        0b00000000'00000000'00000000'01010101'01010101'00000000'00000000'00000000ULL);
    ASSERT_THAT(
        bit_reverse(uint64_t{
            0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL}) ==
        0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000ULL);
    ASSERT_THAT(
        bit_reverse(uint64_t{
            0b11111111'00000000'11111111'00000000'11111111'00000000'11111111'00000000ULL}) ==
        0b00000000'11111111'00000000'11111111'00000000'11111111'00000000'11111111ULL);
    ASSERT_THAT(
        bit_reverse(uint64_t{
            0b11111111'11111111'11111111'11111111'00000000'00000000'00000000'00000000ULL}) ==
        0b00000000'00000000'00000000'00000000'11111111'11111111'11111111'11111111ULL);

#if defined(HAS_I128_CONSTEXPR)

    ASSERT_THAT(bit_reverse(uint128_t{0}) == 0);
    ASSERT_THAT(bit_reverse(uint128_t{~uint64_t{0}} << 64U) == ~uint64_t{0});
    ASSERT_THAT(bit_reverse((uint128_t{~uint64_t{0}} << 64U) | 1U) ==
                ((uint128_t{1} << 127U) | ~uint64_t{0}));
    ASSERT_THAT(bit_reverse(~uint128_t{0}) == ~uint128_t{0});

#endif

#if CONFIG_HAS_AT_LEAST_CXX_20
    ASSERT_THAT(int(detail::pop_count_32_software(0U)) == std::popcount(0U));
    ASSERT_THAT(int(detail::pop_count_32_software(1U)) == std::popcount(1U));
    ASSERT_THAT(int(detail::pop_count_32_software(2U)) == std::popcount(2U));
    ASSERT_THAT(int(detail::pop_count_32_software(3U)) == std::popcount(3U));
    ASSERT_THAT(int(detail::pop_count_32_software(4U)) == std::popcount(4U));
    ASSERT_THAT(int(detail::pop_count_32_software(0x4788743U)) == std::popcount(0x4788743U));
    ASSERT_THAT(int(detail::pop_count_32_software(0x2D425B23U)) == std::popcount(0x2D425B23U));
    ASSERT_THAT(int(detail::pop_count_32_software(0xFFFFFFFFU - 1)) ==
                std::popcount(0xFFFFFFFFU - 1));
    ASSERT_THAT(int(detail::pop_count_32_software(0xFFFFFFFFU)) == std::popcount(0xFFFFFFFFU));
#endif

#if CONFIG_HAS_AT_LEAST_CXX_20
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t{0})) == std::popcount(uint64_t{0}));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t{1})) == std::popcount(uint64_t{1}));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t{2})) == std::popcount(uint64_t{2}));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t{3})) == std::popcount(uint64_t{3}));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t{4})) == std::popcount(uint64_t{4}));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t{0x4788743U})) ==
                std::popcount(uint64_t{0x4788743U}));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t{0x2D425B23U})) ==
                std::popcount(uint64_t{0x2D425B23U}));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t{0xFFFFFFFFU - 1})) ==
                std::popcount(uint64_t{0xFFFFFFFFU - 1}));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t{0xFFFFFFFFU})) ==
                std::popcount(uint64_t{0xFFFFFFFFU}));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t{0x5873485893484ULL})) ==
                std::popcount(uint64_t{0x5873485893484ULL}));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t{0x85923489853245ULL})) ==
                std::popcount(uint64_t{0x85923489853245ULL}));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t{0xFFFFFFFFFFFFFFFFULL - 1})) ==
                std::popcount(uint64_t{0xFFFFFFFFFFFFFFFFULL - 1}));
    ASSERT_THAT(int(detail::pop_count_64_software(uint64_t{0xFFFFFFFFFFFFFFFFULL})) ==
                std::popcount(uint64_t{0xFFFFFFFFFFFFFFFFULL}));
#endif

#if CONFIG_HAS_AT_LEAST_CXX_20
    ASSERT_THAT(std::popcount(0U) - std::popcount(0U) == pop_diff(0, 0));
    ASSERT_THAT(std::popcount(1U) - std::popcount(0U) == pop_diff(1, 0));
    ASSERT_THAT(std::popcount(0U) - std::popcount(1U) == pop_diff(0, 1));
    ASSERT_THAT(std::popcount(0xABCDEFU) - std::popcount(4U) == pop_diff(0xABCDEF, 4));
    ASSERT_THAT(std::popcount(uint32_t{std::numeric_limits<uint16_t>::max()}) -
                    std::popcount(314U) ==
                pop_diff(uint32_t{std::numeric_limits<uint16_t>::max()}, 314));
    ASSERT_THAT(std::popcount(std::numeric_limits<uint32_t>::max()) - std::popcount(0U) ==
                pop_diff(std::numeric_limits<uint32_t>::max(), 0U));
    ASSERT_THAT(std::popcount(0U) - std::popcount(std::numeric_limits<uint32_t>::max()) ==
                pop_diff(0U, std::numeric_limits<uint32_t>::max()));
    ASSERT_THAT(
        std::popcount(std::numeric_limits<uint32_t>::max()) -
            std::popcount(std::numeric_limits<uint32_t>::max()) ==
        pop_diff(std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max()));
#endif

#if defined(HAS_I128_CONSTEXPR)
    ASSERT_THAT(sign(int128_t{0}) == 0);
    ASSERT_THAT(sign(int128_t{1}) == 1);
    ASSERT_THAT(sign(int128_t{-1}) == -1);
    ASSERT_THAT(sign(int128_t{2}) == 1);
    ASSERT_THAT(sign(int128_t{-2}) == -1);
    ASSERT_THAT(sign(int128_t{18446744073709551615ULL}) == 1);
    ASSERT_THAT(sign(-int128_t{18446744073709551615ULL}) == -1);
    ASSERT_THAT(sign(int128_t{uint64_t{1} << 63U}) == 1);
    ASSERT_THAT(sign(-int128_t{uint64_t{1} << 63U}) == -1);
    ASSERT_THAT(sign(static_cast<int128_t>(uint128_t{1} << 126U)) == 1);
    ASSERT_THAT(sign(-static_cast<int128_t>(uint128_t{1} << 126U)) == -1);
    ASSERT_THAT(sign(static_cast<int128_t>((uint128_t{1} << 127U) - 1)) == 1);
    ASSERT_THAT(sign(static_cast<int128_t>(-((uint128_t{1} << 127U) - 1))) == -1);
    ASSERT_THAT(sign(static_cast<int128_t>(-(uint128_t{1} << 127U))) == -1);
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
    ASSERT_THAT(uabs(0) == 0);
    ASSERT_THAT(uabs(0U) == 0);
    ASSERT_THAT(uabs(0L) == 0);
    ASSERT_THAT(uabs(0UL) == 0);
    ASSERT_THAT(uabs(0LL) == 0);
    ASSERT_THAT(uabs(0ULL) == 0);

    ASSERT_THAT(uabs(static_cast<signed char>(-1)) == 1);
    ASSERT_THAT(uabs(static_cast<unsigned char>(1)) == 1);
    ASSERT_THAT(uabs(static_cast<short>(-1)) == 1);
    ASSERT_THAT(uabs(static_cast<unsigned short>(1)) == 1);
    ASSERT_THAT(uabs(-1) == 1);
    ASSERT_THAT(uabs(1U) == 1);
    ASSERT_THAT(uabs(-1L) == 1);
    ASSERT_THAT(uabs(1UL) == 1);
    ASSERT_THAT(uabs(-1LL) == 1);
    ASSERT_THAT(uabs(1ULL) == 1);

    ASSERT_THAT(uabs(static_cast<signed char>(-128)) == 128);
    ASSERT_THAT(uabs(static_cast<signed char>(128)) == 128);
    ASSERT_THAT(uabs(short{-128}) == 128);
    ASSERT_THAT(uabs(short{128}) == 128);
    ASSERT_THAT(uabs(-128) == 128);
    ASSERT_THAT(uabs(128) == 128);
    ASSERT_THAT(uabs(-128L) == 128);
    ASSERT_THAT(uabs(128L) == 128);
    ASSERT_THAT(uabs(-128LL) == 128);
    ASSERT_THAT(uabs(128LL) == 128);

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

#if defined(HAS_I128_CONSTEXPR)
    ASSERT_THAT(uabs(int128_t{0}) == 0);
    ASSERT_THAT(uabs(uint128_t{0}) == 0);
    ASSERT_THAT(uabs(int128_t{-1}) == 1);
    ASSERT_THAT(uabs(int128_t{1}) == 1);
    ASSERT_THAT(uabs(uint128_t{1}) == 1);
    ASSERT_THAT(uabs(int128_t{-4}) == 4);
    ASSERT_THAT(uabs(int128_t{4}) == 4);
    ASSERT_THAT(uabs(uint128_t{4}) == 4);
    ASSERT_THAT(uabs(-int128_t{18446744073709551615ULL}) == 18446744073709551615ULL);
    ASSERT_THAT(uabs(int128_t{18446744073709551615ULL}) == 18446744073709551615ULL);
    ASSERT_THAT(uabs(uint128_t{18446744073709551615ULL}) == 18446744073709551615ULL);
    ASSERT_THAT(uabs(-static_cast<int128_t>(uint128_t{1} << 126U)) == uint128_t{1} << 126U);
    ASSERT_THAT(uabs(static_cast<int128_t>(uint128_t{1} << 126U)) == uint128_t{1} << 126U);
    ASSERT_THAT(uabs(uint128_t{1} << 126U) == uint128_t{1} << 126U);
    ASSERT_THAT(uabs(static_cast<int128_t>(-((uint128_t{1} << 127U) - 1))) ==
                (uint128_t{1} << 127U) - 1);
    ASSERT_THAT(uabs(static_cast<int128_t>((uint128_t{1} << 127U) - 1)) ==
                (uint128_t{1} << 127U) - 1);
    ASSERT_THAT(uabs((uint128_t{1} << 127U) - 1) == (uint128_t{1} << 127U) - 1);
    ASSERT_THAT(uabs(static_cast<int128_t>(-(uint128_t{1} << 127U))) == uint128_t{1} << 127U);
    ASSERT_THAT(uabs(-(uint128_t{1} << 127U)) == uint128_t{1} << 127U);
#endif

#if CONFIG_HAS_AT_LEAST_CXX_20

    ASSERT_THAT(sign(std::popcount(0U) - std::popcount(0U)) == sign(pop_cmp(0, 0)));
    ASSERT_THAT(sign(std::popcount(1U) - std::popcount(0U)) == sign(pop_cmp(1, 0)));
    ASSERT_THAT(sign(std::popcount(0U) - std::popcount(1U)) == sign(pop_cmp(0, 1)));
    ASSERT_THAT(sign(std::popcount(0xABCDEFU) - std::popcount(4U)) == pop_cmp(0xABCDEF, 4));
    ASSERT_THAT(
        sign(std::popcount(uint32_t{std::numeric_limits<uint16_t>::max()}) - std::popcount(314U)) ==
        sign(pop_cmp(uint32_t{std::numeric_limits<uint16_t>::max()}, 314U)));
    ASSERT_THAT(sign(std::popcount(std::numeric_limits<uint32_t>::max()) - std::popcount(0U)) ==
                sign(pop_cmp(std::numeric_limits<uint32_t>::max(), 0U)));
    ASSERT_THAT(sign(std::popcount(0U) - std::popcount(std::numeric_limits<uint32_t>::max())) ==
                sign(pop_cmp(0U, std::numeric_limits<uint32_t>::max())));
    ASSERT_THAT(
        sign(std::popcount(std::numeric_limits<uint32_t>::max()) -
             std::popcount(std::numeric_limits<uint32_t>::max())) ==
        sign(pop_cmp(std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::max())));
#endif

    ASSERT_THAT(detail::lz_count_32_software(0U) == 32);
    ASSERT_THAT(detail::lz_count_32_software(1U) == 31);
    ASSERT_THAT(detail::lz_count_32_software(2U) == 30);
    ASSERT_THAT(detail::lz_count_32_software(4U) == 29);
    ASSERT_THAT(detail::lz_count_32_software(8U) == 28);
    ASSERT_THAT(detail::lz_count_32_software(12U) == 28);
    ASSERT_THAT(detail::lz_count_32_software(16U) == 27);
    ASSERT_THAT(detail::lz_count_32_software(32U) == 26);
    ASSERT_THAT(detail::lz_count_32_software(48U) == 26);
    ASSERT_THAT(detail::lz_count_32_software(uint32_t{1} << 30U) == 1);
    ASSERT_THAT(detail::lz_count_32_software(uint32_t{1} << 31U) == 0);
    ASSERT_THAT(detail::lz_count_32_software(~uint32_t{1}) == 0);

    ASSERT_THAT(detail::lz_count_64_software(0U) == 64);
    ASSERT_THAT(detail::lz_count_64_software(1U) == 63);
    ASSERT_THAT(detail::lz_count_64_software(2U) == 62);
    ASSERT_THAT(detail::lz_count_64_software(4U) == 61);
    ASSERT_THAT(detail::lz_count_64_software(8U) == 60);
    ASSERT_THAT(detail::lz_count_64_software(12U) == 60);
    ASSERT_THAT(detail::lz_count_64_software(16U) == 59);
    ASSERT_THAT(detail::lz_count_64_software(32U) == 58);
    ASSERT_THAT(detail::lz_count_64_software(48U) == 58);
    ASSERT_THAT(detail::lz_count_64_software(uint32_t{1} << 30U) == 33);
    ASSERT_THAT(detail::lz_count_64_software(uint32_t{1} << 31U) == 32);
    ASSERT_THAT(detail::lz_count_64_software(~uint32_t{1}) == 32);
    ASSERT_THAT(detail::lz_count_64_software(uint64_t{1} << 62U) == 1);
    ASSERT_THAT(detail::lz_count_64_software(uint64_t{1} << 63U) == 0);
    ASSERT_THAT(detail::lz_count_64_software(~uint64_t{1}) == 0);

    ASSERT_THAT(detail::tz_count_32_software(0U) == 32);
    ASSERT_THAT(detail::tz_count_32_software(1U) == 0);
    ASSERT_THAT(detail::tz_count_32_software(2U) == 1);
    ASSERT_THAT(detail::tz_count_32_software(4U) == 2);
    ASSERT_THAT(detail::tz_count_32_software(8U) == 3);
    ASSERT_THAT(detail::tz_count_32_software(12U) == 2);
    ASSERT_THAT(detail::tz_count_32_software(16U) == 4);
    ASSERT_THAT(detail::tz_count_32_software(32U) == 5);
    ASSERT_THAT(detail::tz_count_32_software(48U) == 4);
    ASSERT_THAT(detail::tz_count_32_software(1U << 30U) == 30);
    ASSERT_THAT(detail::tz_count_32_software(1U << 31U) == 31);
    ASSERT_THAT(detail::tz_count_32_software(~1U) == 1);
    ASSERT_THAT(detail::tz_count_32_software(std::numeric_limits<uint32_t>::max()) == 0);

    ASSERT_THAT(detail::tz_count_64_software(0U) == 64);
    ASSERT_THAT(detail::tz_count_64_software(1U) == 0);
    ASSERT_THAT(detail::tz_count_64_software(2U) == 1);
    ASSERT_THAT(detail::tz_count_64_software(4U) == 2);
    ASSERT_THAT(detail::tz_count_64_software(8U) == 3);
    ASSERT_THAT(detail::tz_count_64_software(12U) == 2);
    ASSERT_THAT(detail::tz_count_64_software(16U) == 4);
    ASSERT_THAT(detail::tz_count_64_software(32U) == 5);
    ASSERT_THAT(detail::tz_count_64_software(48U) == 4);
    ASSERT_THAT(detail::tz_count_64_software(1U << 30U) == 30);
    ASSERT_THAT(detail::tz_count_64_software(1U << 31U) == 31);
    ASSERT_THAT(detail::tz_count_64_software(~1U) == 1);
    ASSERT_THAT(detail::tz_count_64_software(std::numeric_limits<uint32_t>::max()) == 0);

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
    ASSERT_THAT(next_n_bits_permutation(0b0110'0000'0000'0000'0000'0000'0000'0000U) ==
                0b1000'0000'0000'0000'0000'0000'0000'0001U);

    ASSERT_THAT(next_n_bits_permutation(0b01) == 0b10);

    ASSERT_THAT(next_n_bits_permutation(0b1111111) == 0b10111111);

    ASSERT_THAT(next_n_bits_permutation(1U << 0U) == 1U << 1U);
    ASSERT_THAT(next_n_bits_permutation(1U << 1U) == 1U << 2U);
    ASSERT_THAT(next_n_bits_permutation(1U << 2U) == 1U << 3U);
    ASSERT_THAT(next_n_bits_permutation(1U << 3U) == 1U << 4U);
    ASSERT_THAT(next_n_bits_permutation(1U << 4U) == 1U << 5U);
    ASSERT_THAT(next_n_bits_permutation(1U << 5U) == 1U << 6U);
    ASSERT_THAT(next_n_bits_permutation(1U << 6U) == 1U << 7U);
    ASSERT_THAT(next_n_bits_permutation(1U << 7U) == 1U << 8U);
    ASSERT_THAT(next_n_bits_permutation(1U << 8U) == 1U << 9U);
    ASSERT_THAT(next_n_bits_permutation(1U << 29U) == 1U << 30U);
    ASSERT_THAT(next_n_bits_permutation(1U << 30U) == 1U << 31U);

    ASSERT_THAT(!is_power_of_two(0ULL));
    ASSERT_THAT(is_power_of_two(1ULL << 0U));
    ASSERT_THAT(is_power_of_two(1ULL << 1U));
    ASSERT_THAT(is_power_of_two(1ULL << 2U));
    ASSERT_THAT(is_power_of_two(1ULL << 3U));
    ASSERT_THAT(is_power_of_two(1ULL << 4U));
    ASSERT_THAT(is_power_of_two(1ULL << 5U));
    ASSERT_THAT(is_power_of_two(1ULL << 6U));
    ASSERT_THAT(is_power_of_two(1ULL << 7U));
    ASSERT_THAT(is_power_of_two(1ULL << 8U));
    ASSERT_THAT(is_power_of_two(1ULL << 9U));
    ASSERT_THAT(is_power_of_two(1ULL << 60U));
    ASSERT_THAT(is_power_of_two(1ULL << 61U));
    ASSERT_THAT(is_power_of_two(1ULL << 62U));
    ASSERT_THAT(is_power_of_two(1ULL << 63U));

#if defined(INTEGERS_128_BIT_HPP)

    ASSERT_THAT(!is_power_of_two(uint128_t{0}));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 0U));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 1U));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 2U));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 3U));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 4U));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 5U));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 6U));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 7U));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 8U));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 9U));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 60U));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 61U));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 62U));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 63U));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 64U));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 65U));
    ASSERT_THAT(is_power_of_two(uint128_t{1} << 127U));
#endif

    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{0U}) == 1U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1U}) == 1U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{2U}) == 2U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{3U}) == 4U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{4U}) == 4U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{5U}) == 8U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{6U}) == 8U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{7U}) == 8U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{8U}) == 8U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{16U}) == 16U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{17U}) == 32U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{18U}) == 32U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{19U}) == 32U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{20U}) == 32U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{0x7FFFFFFFU}) == 0x80000000U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{0x80000000U}) == 0x80000000U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{0x80000001U}) == 0x100000000ULL);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{0xFFFFFFFFU}) == 0x100000000ULL);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 0U) == uint32_t{1} << 0U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 1U) == uint32_t{1} << 1U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 2U) == uint32_t{1} << 2U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 3U) == uint32_t{1} << 3U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 4U) == uint32_t{1} << 4U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 5U) == uint32_t{1} << 5U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 6U) == uint32_t{1} << 6U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 7U) == uint32_t{1} << 7U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 8U) == uint32_t{1} << 8U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 9U) == uint32_t{1} << 9U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 10U) == uint32_t{1} << 10U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 11U) == uint32_t{1} << 11U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 12U) == uint32_t{1} << 12U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 13U) == uint32_t{1} << 13U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 14U) == uint32_t{1} << 14U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 15U) == uint32_t{1} << 15U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 16U) == uint32_t{1} << 16U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 17U) == uint32_t{1} << 17U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 18U) == uint32_t{1} << 18U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 19U) == uint32_t{1} << 19U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 20U) == uint32_t{1} << 20U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 21U) == uint32_t{1} << 21U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 22U) == uint32_t{1} << 22U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 23U) == uint32_t{1} << 23U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 24U) == uint32_t{1} << 24U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 25U) == uint32_t{1} << 25U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 26U) == uint32_t{1} << 26U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 27U) == uint32_t{1} << 27U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 28U) == uint32_t{1} << 28U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 29U) == uint32_t{1} << 29U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 30U) == uint32_t{1} << 30U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint32_t{1} << 31U) == uint32_t{1} << 31U);

    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{0U}) == 1U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1U}) == 1U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{2U}) == 2U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{3U}) == 4U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{4U}) == 4U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{5U}) == 8U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{6U}) == 8U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{7U}) == 8U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{8U}) == 8U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{16U}) == 16U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{17U}) == 32U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{18U}) == 32U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{19U}) == 32U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{20U}) == 32U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{0x7FFFFFFFU}) == 0x80000000U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{0x80000000U}) == 0x80000000U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{0x80000001U}) == 0x100000000ULL);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{0xFFFFFFFFU}) == 0x100000000ULL);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{0x7FFFFFFFFFFFFFFFULL}) ==
                0x8000000000000000ULL);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{0x8000000000000000ULL}) ==
                0x8000000000000000ULL);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 0U) == uint64_t{1} << 0U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 1U) == uint64_t{1} << 1U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 2U) == uint64_t{1} << 2U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 3U) == uint64_t{1} << 3U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 4U) == uint64_t{1} << 4U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 5U) == uint64_t{1} << 5U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 6U) == uint64_t{1} << 6U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 7U) == uint64_t{1} << 7U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 8U) == uint64_t{1} << 8U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 9U) == uint64_t{1} << 9U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 10U) == uint64_t{1} << 10U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 11U) == uint64_t{1} << 11U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 12U) == uint64_t{1} << 12U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 13U) == uint64_t{1} << 13U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 14U) == uint64_t{1} << 14U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 15U) == uint64_t{1} << 15U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 16U) == uint64_t{1} << 16U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 17U) == uint64_t{1} << 17U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 18U) == uint64_t{1} << 18U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 19U) == uint64_t{1} << 19U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 20U) == uint64_t{1} << 20U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 21U) == uint64_t{1} << 21U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 22U) == uint64_t{1} << 22U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 23U) == uint64_t{1} << 23U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 24U) == uint64_t{1} << 24U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 25U) == uint64_t{1} << 25U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 26U) == uint64_t{1} << 26U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 27U) == uint64_t{1} << 27U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 28U) == uint64_t{1} << 28U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 29U) == uint64_t{1} << 29U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 30U) == uint64_t{1} << 30U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 31U) == uint64_t{1} << 31U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 32U) == uint64_t{1} << 32U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 33U) == uint64_t{1} << 33U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 34U) == uint64_t{1} << 34U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 35U) == uint64_t{1} << 35U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 36U) == uint64_t{1} << 36U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 37U) == uint64_t{1} << 37U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 38U) == uint64_t{1} << 38U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 39U) == uint64_t{1} << 39U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 40U) == uint64_t{1} << 40U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 41U) == uint64_t{1} << 41U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 42U) == uint64_t{1} << 42U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 43U) == uint64_t{1} << 43U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 44U) == uint64_t{1} << 44U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 45U) == uint64_t{1} << 45U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 46U) == uint64_t{1} << 46U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 47U) == uint64_t{1} << 47U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 48U) == uint64_t{1} << 48U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 49U) == uint64_t{1} << 49U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 50U) == uint64_t{1} << 50U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 51U) == uint64_t{1} << 51U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 52U) == uint64_t{1} << 52U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 53U) == uint64_t{1} << 53U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 54U) == uint64_t{1} << 54U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 55U) == uint64_t{1} << 55U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 56U) == uint64_t{1} << 56U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 57U) == uint64_t{1} << 57U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 58U) == uint64_t{1} << 58U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 59U) == uint64_t{1} << 59U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 60U) == uint64_t{1} << 60U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 61U) == uint64_t{1} << 61U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 62U) == uint64_t{1} << 62U);
    ASSERT_THAT(nearest_greater_equal_power_of_two(uint64_t{1} << 63U) == uint64_t{1} << 63U);

    ASSERT_THAT(least_bit_set(0b0) == 0b0);
    ASSERT_THAT(least_bit_set(0b1) == 0b1);
    ASSERT_THAT(least_bit_set(0b10) == 0b10);
    ASSERT_THAT(least_bit_set(0b100) == 0b100);
    ASSERT_THAT(least_bit_set(0b1000) == 0b1000);
    ASSERT_THAT(least_bit_set(0b10000) == 0b10000);
    ASSERT_THAT(least_bit_set(0b100000) == 0b100000);
    ASSERT_THAT(least_bit_set(0b1000000) == 0b1000000);
    ASSERT_THAT(least_bit_set(0b10000000) == 0b10000000);
    ASSERT_THAT(least_bit_set(0b0U) == 0b0U);
    ASSERT_THAT(least_bit_set(0b1U) == 0b1U);
    ASSERT_THAT(least_bit_set(0b10U) == 0b10U);
    ASSERT_THAT(least_bit_set(0b100U) == 0b100U);
    ASSERT_THAT(least_bit_set(0b1000U) == 0b1000U);
    ASSERT_THAT(least_bit_set(0b10000U) == 0b10000U);
    ASSERT_THAT(least_bit_set(0b100000U) == 0b100000U);
    ASSERT_THAT(least_bit_set(0b1000000U) == 0b1000000U);
    ASSERT_THAT(least_bit_set(0b10000000U) == 0b10000000U);
    ASSERT_THAT(least_bit_set(static_cast<int8_t>(0b0)) == static_cast<int8_t>(0b0));
    ASSERT_THAT(least_bit_set(static_cast<int8_t>(0b1)) == static_cast<int8_t>(0b1));
    ASSERT_THAT(least_bit_set(static_cast<int8_t>(0b10)) == static_cast<int8_t>(0b10));
    ASSERT_THAT(least_bit_set(static_cast<int8_t>(0b100)) == static_cast<int8_t>(0b100));
    ASSERT_THAT(least_bit_set(static_cast<int8_t>(0b1000)) == static_cast<int8_t>(0b1000));
    ASSERT_THAT(least_bit_set(static_cast<int8_t>(0b10000)) == static_cast<int8_t>(0b10000));
    ASSERT_THAT(least_bit_set(static_cast<int8_t>(0b100000)) == static_cast<int8_t>(0b100000));
    ASSERT_THAT(least_bit_set(static_cast<int8_t>(0b1000000)) == static_cast<int8_t>(0b1000000));
    ASSERT_THAT(least_bit_set(static_cast<int8_t>(0b10000000)) == static_cast<int8_t>(0b10000000));
    ASSERT_THAT(least_bit_set(uint8_t{0b0}) == uint8_t{0b0});
    ASSERT_THAT(least_bit_set(uint8_t{0b1}) == uint8_t{0b1});
    ASSERT_THAT(least_bit_set(uint8_t{0b10}) == uint8_t{0b10});
    ASSERT_THAT(least_bit_set(uint8_t{0b100}) == uint8_t{0b100});
    ASSERT_THAT(least_bit_set(uint8_t{0b1000}) == uint8_t{0b1000});
    ASSERT_THAT(least_bit_set(uint8_t{0b10000}) == uint8_t{0b10000});
    ASSERT_THAT(least_bit_set(uint8_t{0b100000}) == uint8_t{0b100000});
    ASSERT_THAT(least_bit_set(uint8_t{0b1000000}) == uint8_t{0b1000000});
    ASSERT_THAT(least_bit_set(uint8_t{0b10000000}) == uint8_t{0b10000000});
    ASSERT_THAT(least_bit_set(0b100000000) == 0b100000000);
    ASSERT_THAT(least_bit_set(0b1000000000) == 0b1000000000);
    ASSERT_THAT(least_bit_set(0b10000000000) == 0b10000000000);
    ASSERT_THAT(least_bit_set(0b100000000U) == 0b100000000U);
    ASSERT_THAT(least_bit_set(0b1000000000U) == 0b1000000000U);
    ASSERT_THAT(least_bit_set(0b10000000000U) == 0b10000000000U);
    ASSERT_THAT(
        least_bit_set(0b1000000000000000000000000000000000000000000000000000000000000000ULL) ==
        0b1000000000000000000000000000000000000000000000000000000000000000ULL);
    ASSERT_THAT(least_bit_set(0b110101010101010101011001) == 0b1);
    ASSERT_THAT(least_bit_set(0b1010101011001101011100010100000LL) == 0b100000LL);
    ASSERT_THAT(least_bit_set(0b1010111001010101101010110101001101011100110011000LL) == 0b1000LL);
    ASSERT_THAT(least_bit_set(0b110101010101010101011001U) == 0b1U);
    ASSERT_THAT(least_bit_set(0b1010101011001101011100010100000ULL) == 0b100000ULL);
    ASSERT_THAT(least_bit_set(0b1010111001010101101010110101001101011100110011000ULL) == 0b1000ULL);

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
    LOG10_ASSERT_THAT(log10_floor(uint32_t{4294967294U}) == 9);
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
    LOG10_ASSERT_THAT(base_10_len(uint32_t{4294967294U}) == 10);
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
    LOG10_ASSERT_THAT(base_10_len(uint64_t{4294967294U}) == 10);
    LOG10_ASSERT_THAT(base_10_len(static_cast<uint64_t>(1e8)) == 9);
    LOG10_ASSERT_THAT(base_10_len(static_cast<uint64_t>(1e9)) == 10);
    LOG10_ASSERT_THAT(base_10_len(static_cast<uint64_t>(1e18)) == 19);
    LOG10_ASSERT_THAT(base_10_len(static_cast<uint64_t>(1e19)) == 20);
    LOG10_ASSERT_THAT(base_10_len(static_cast<uint64_t>(-1)) == 20);

#if CONFIG_HAS_AT_LEAST_CXX_17

    ASSERT_THAT(base_b_len(0U) == 1);
    ASSERT_THAT(base_b_len(1U) == 1);
    ASSERT_THAT(base_b_len(9U) == 1);
    ASSERT_THAT(base_b_len(10U) == 2);
    ASSERT_THAT(base_b_len(11U) == 2);
    ASSERT_THAT(base_b_len(99U) == 2);
    ASSERT_THAT(base_b_len(100U) == 3);
    ASSERT_THAT(base_b_len(101U) == 3);
    ASSERT_THAT(base_b_len(std::numeric_limits<uint32_t>::max()) == 10);

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
    ASSERT_THAT(base_b_len(static_cast<int32_t>(uint32_t{1} << 31U)) == 11);

    ASSERT_THAT(base_b_len(0ULL) == 1);
    ASSERT_THAT(base_b_len(1ULL) == 1);
    ASSERT_THAT(base_b_len(9ULL) == 1);
    ASSERT_THAT(base_b_len(10ULL) == 2);
    ASSERT_THAT(base_b_len(11ULL) == 2);
    ASSERT_THAT(base_b_len(99ULL) == 2);
    ASSERT_THAT(base_b_len(100ULL) == 3);
    ASSERT_THAT(base_b_len(101ULL) == 3);
    ASSERT_THAT(base_b_len(static_cast<uint64_t>(-1)) == 20);

    ASSERT_THAT(base_b_len(0LL) == 1);
    ASSERT_THAT(base_b_len(1LL) == 1);
    ASSERT_THAT(base_b_len(9LL) == 1);
    ASSERT_THAT(base_b_len(10LL) == 2);
    ASSERT_THAT(base_b_len(11LL) == 2);
    ASSERT_THAT(base_b_len(99LL) == 2);
    ASSERT_THAT(base_b_len(100LL) == 3);
    ASSERT_THAT(base_b_len(101LL) == 3);
    ASSERT_THAT(base_b_len(-0LL) == 1);
    ASSERT_THAT(base_b_len(-1LL) == 2);
    ASSERT_THAT(base_b_len(-9LL) == 2);
    ASSERT_THAT(base_b_len(-10LL) == 3);
    ASSERT_THAT(base_b_len(-11LL) == 3);
    ASSERT_THAT(base_b_len(-99LL) == 3);
    ASSERT_THAT(base_b_len(-100LL) == 4);
    ASSERT_THAT(base_b_len(-101LL) == 4);
    ASSERT_THAT(base_b_len(std::numeric_limits<int64_t>::min()) == 20);

#if defined(INTEGERS_128_BIT_HPP)

    ASSERT_THAT(base_b_len(uint128_t{0}) == 1);
    ASSERT_THAT(base_b_len(uint128_t{1}) == 1);
    ASSERT_THAT(base_b_len(uint128_t{9}) == 1);
    ASSERT_THAT(base_b_len(uint128_t{10}) == 2);
    ASSERT_THAT(base_b_len(uint128_t{11}) == 2);
    ASSERT_THAT(base_b_len(uint128_t{99}) == 2);
    ASSERT_THAT(base_b_len(uint128_t{100}) == 3);
    ASSERT_THAT(base_b_len(uint128_t{101}) == 3);
    ASSERT_THAT(base_b_len(static_cast<uint128_t>(-1)) == 39);

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
    ASSERT_THAT(base_b_len(static_cast<int128_t>(uint128_t{1} << 127U)) == 40);

#endif  // INTEGERS_128_BIT_HPP

#endif  // CONFIG_HAS_AT_LEAST_CXX_17

#if defined(HAS_I128_CONSTEXPR)

    ASSERT_THAT(gcd(uint128_t{1}, uint128_t{1}) == 1);
    ASSERT_THAT(gcd(uint128_t{3}, uint128_t{7}) == 1);
    ASSERT_THAT(gcd(uint128_t{0}, uint128_t{112378432}) == 112378432);
    ASSERT_THAT(gcd(uint128_t{112378432}, uint128_t{0}) == 112378432);
    ASSERT_THAT(gcd(uint128_t{429384832}, uint128_t{324884}) == 4);
    ASSERT_THAT(gcd(uint128_t{18446744073709551521ULL}, uint128_t{18446744073709551533ULL}) == 1);
    ASSERT_THAT(gcd(uint128_t{18446744073709551521ULL} * 18446744073709551521ULL,
                    uint128_t{18446744073709551521ULL}) == 18446744073709551521ULL);
    ASSERT_THAT(gcd(uint128_t{23999993441ULL} * 23999993377ULL,
                    uint128_t{23999992931ULL} * 23999539633ULL) == 1);
    ASSERT_THAT(gcd(uint128_t{2146514599U} * 2146514603U * 2146514611U,
                    uint128_t{2146514611U} * 2146514621U * 2146514647U) == 2146514611ULL);
    ASSERT_THAT(gcd(uint128_t{2146514599U} * 2146514603U * 2146514611U * 2,
                    uint128_t{2146514599U} * 2146514603U * 2146514611U * 3) ==
                uint128_t{2146514599U} * 2146514603U * 2146514611U);
    ASSERT_THAT(gcd(uint128_t{100000000000000003ULL} * 1000000000000000003ULL,
                    uint128_t{1000000000000000003ULL} * 1000000000000000009ULL) ==
                1000000000000000003ULL);
    ASSERT_THAT(gcd(uint128_t{3} * 2 * 5 * 7 * 11 * 13 * 17 * 19,
                    uint128_t{18446744073709551557ULL} * 3) == 3);
    ASSERT_THAT(gcd(uint128_t{1000000000000000009ULL},
                    uint128_t{1000000000000000009ULL} * 1000000000000000009ULL) ==
                1000000000000000009ULL);
    ASSERT_THAT(gcd(uint128_t{0}, uint128_t{1000000000000000009ULL} * 1000000000000000009ULL) ==
                uint128_t{1000000000000000009ULL} * 1000000000000000009ULL);
    ASSERT_THAT(gcd(uint128_t{18446744073709551557ULL}, uint128_t{0}) == 18446744073709551557ULL);

    ASSERT_THAT(gcd(uint64_t{2}, int128_t{4}) == 2);
    ASSERT_THAT(gcd(uint64_t{2}, int128_t{-4}) == 2);
    ASSERT_THAT(gcd(uint64_t{3}, int128_t{7}) == 1);
    ASSERT_THAT(gcd(uint64_t{3}, int128_t{-7}) == 1);
    ASSERT_THAT(gcd(uint64_t{3}, int128_t{18446744073709551557ULL} * 3) == 3);
    ASSERT_THAT(gcd(uint64_t{3}, int128_t{18446744073709551557ULL} * (-3)) == 3);
    ASSERT_THAT(gcd(uint64_t{3} * 2 * 5 * 7 * 11 * 13 * 17 * 19,
                    int128_t{18446744073709551557ULL} * 3) == 3);
    ASSERT_THAT(gcd(uint64_t{1000000000000000009ULL},
                    int128_t{1000000000000000009LL} * 1000000000000000009LL) ==
                1000000000000000009ULL);
    ASSERT_THAT(gcd(uint64_t{0}, int128_t{1000000000000000009LL} * 1000000000000000009LL) ==
                uint128_t{1000000000000000009LL} * 1000000000000000009ULL);
    ASSERT_THAT(gcd(uint64_t{18446744073709551557ULL}, int128_t{0}) == 18446744073709551557ULL);

    ASSERT_THAT(math_functions::popcount(0U) == 0);
    ASSERT_THAT(math_functions::popcount(1U << 1U) == 1);
    ASSERT_THAT(math_functions::popcount(1U << 2U) == 1);
    ASSERT_THAT(math_functions::popcount(1U << 3U) == 1);
    ASSERT_THAT(math_functions::popcount(1U << 4U) == 1);
    ASSERT_THAT(math_functions::popcount(1U << 5U) == 1);
    ASSERT_THAT(math_functions::popcount(1U << 6U) == 1);
    ASSERT_THAT(math_functions::popcount(1U << 7U) == 1);
    ASSERT_THAT(math_functions::popcount(1U << 8U) == 1);
    ASSERT_THAT(math_functions::popcount(1U << 9U) == 1);
    ASSERT_THAT(math_functions::popcount(1U << 10U) == 1);
    ASSERT_THAT(math_functions::popcount(1U << 12U) == 1);
    ASSERT_THAT(math_functions::popcount(1U << 13U) == 1);
    ASSERT_THAT(math_functions::popcount(1U << 14U) == 1);
    ASSERT_THAT(math_functions::popcount(1U << 30U) == 1);
    ASSERT_THAT(math_functions::popcount(1U << 31U) == 1);

    ASSERT_THAT(math_functions::popcount(0UL) == 0);
    ASSERT_THAT(math_functions::popcount(1UL << 1U) == 1);
    ASSERT_THAT(math_functions::popcount(1UL << 2U) == 1);
    ASSERT_THAT(math_functions::popcount(1UL << 3U) == 1);
    ASSERT_THAT(math_functions::popcount(1UL << 4U) == 1);
    ASSERT_THAT(math_functions::popcount(1UL << 5U) == 1);
    ASSERT_THAT(math_functions::popcount(1UL << 6U) == 1);
    ASSERT_THAT(math_functions::popcount(1UL << 7U) == 1);
    ASSERT_THAT(math_functions::popcount(1UL << 8U) == 1);
    ASSERT_THAT(math_functions::popcount(1UL << 9U) == 1);
    ASSERT_THAT(math_functions::popcount(1UL << 10U) == 1);
    ASSERT_THAT(math_functions::popcount(1UL << 12U) == 1);
    ASSERT_THAT(math_functions::popcount(1UL << 13U) == 1);
    ASSERT_THAT(math_functions::popcount(1UL << 14U) == 1);
    ASSERT_THAT(math_functions::popcount(1UL << 30U) == 1);
    ASSERT_THAT(math_functions::popcount(1UL << 31U) == 1);

    ASSERT_THAT(math_functions::popcount(0ULL) == 0);
    ASSERT_THAT(math_functions::popcount(1ULL << 1U) == 1);
    ASSERT_THAT(math_functions::popcount(1ULL << 2U) == 1);
    ASSERT_THAT(math_functions::popcount(1ULL << 3U) == 1);
    ASSERT_THAT(math_functions::popcount(1ULL << 4U) == 1);
    ASSERT_THAT(math_functions::popcount(1ULL << 5U) == 1);
    ASSERT_THAT(math_functions::popcount(1ULL << 6U) == 1);
    ASSERT_THAT(math_functions::popcount(1ULL << 7U) == 1);
    ASSERT_THAT(math_functions::popcount(1ULL << 8U) == 1);
    ASSERT_THAT(math_functions::popcount(1ULL << 9U) == 1);
    ASSERT_THAT(math_functions::popcount(1ULL << 10U) == 1);
    ASSERT_THAT(math_functions::popcount(1ULL << 12U) == 1);
    ASSERT_THAT(math_functions::popcount(1ULL << 13U) == 1);
    ASSERT_THAT(math_functions::popcount(1ULL << 14U) == 1);
    ASSERT_THAT(math_functions::popcount(1ULL << 30U) == 1);
    ASSERT_THAT(math_functions::popcount(1ULL << 31U) == 1);
    ASSERT_THAT(math_functions::popcount(1ULL << 62U) == 1);
    ASSERT_THAT(math_functions::popcount(1ULL << 63U) == 1);

    ASSERT_THAT(!math_functions::bool_median(false, false, false));
    ASSERT_THAT(!math_functions::bool_median(false, false, true));
    ASSERT_THAT(!math_functions::bool_median(false, true, false));
    ASSERT_THAT(math_functions::bool_median(false, true, true));
    ASSERT_THAT(!math_functions::bool_median(true, false, false));
    ASSERT_THAT(math_functions::bool_median(true, false, true));
    ASSERT_THAT(math_functions::bool_median(true, true, false));
    ASSERT_THAT(math_functions::bool_median(false, true, true));

#endif

    ASSERT_THAT(([]() constexpr noexcept {
        const auto [q, r] = math_functions::extract_pow2(uint32_t{0});
        return q == 0 && r == 32;
    }()));
    ASSERT_THAT(([]() constexpr noexcept {
        const auto [q, r] = math_functions::extract_pow2(uint32_t{1} << 31U);
        return q == 1 && r == 31;
    }()));
    ASSERT_THAT(([]() constexpr noexcept {
        const auto [q, r] = math_functions::extract_pow2(uint64_t{0});
        return q == 0 && r == 64;
    }()));
    ASSERT_THAT(([]() constexpr noexcept {
        const auto [q, r] = math_functions::extract_pow2(uint64_t{1} << 31U);
        return q == 1 && r == 31;
    }()));
    ASSERT_THAT(([]() constexpr noexcept {
        const auto [q, r] = math_functions::extract_pow2(uint64_t{1} << 63U);
        return q == 1 && r == 63;
    }()));
    ASSERT_THAT(([]() constexpr noexcept {
        const auto [q, r] = math_functions::extract_pow2(uint64_t{9221685055305285632ULL});
        return q == 2147090867 && r == 32;
    }()));
    ASSERT_THAT(([]() constexpr noexcept {
        const auto [q, r] = math_functions::extract_pow2(uint64_t{4610842527652642816ULL});
        return q == 2147090867 && r == 31;
    }()));

    ASSERT_THAT((math_functions::powers_sum_u64<0>(100) == 100U));
    ASSERT_THAT((math_functions::powers_sum_u64<1>(100) == 100U * (100U + 1) / 2));
    ASSERT_THAT((math_functions::powers_sum_u64<2>(100) == 100U * (100U + 1) * (2 * 100U + 1) / 6));
    ASSERT_THAT(
        (math_functions::powers_sum_u64<3>(100) == 100U * 100U * (100U + 1) * (100U + 1) / 4));

#if defined(HAS_I128_CONSTEXPR)

    ASSERT_THAT((math_functions::powers_sum_u128<0>(100) == 100U));
    ASSERT_THAT((math_functions::powers_sum_u128<1>(100) == 100U * (100U + 1) / 2));
    ASSERT_THAT(
        (math_functions::powers_sum_u128<2>(100) == 100U * (100U + 1) * (2 * 100U + 1) / 6));
    ASSERT_THAT(
        (math_functions::powers_sum_u128<3>(100) == 100U * 100U * (100U + 1) * (100U + 1) / 4));

    constexpr auto kN    = static_cast<uint32_t>(3e9);
    constexpr auto kN128 = static_cast<uint128_t>(kN);
    ASSERT_THAT(
        (math_functions::powers_sum_u128<3>(kN) == kN128 * kN128 * (kN128 + 1) * (kN128 + 1) / 4));

#endif

    ASSERT_THAT(next_even(0U) == 2);
    ASSERT_THAT(next_even(1U) == 2);
    ASSERT_THAT(next_even(2U) == 4);
    ASSERT_THAT(next_even(0UL) == 2);
    ASSERT_THAT(next_even(1UL) == 2);
    ASSERT_THAT(next_even(2UL) == 4);
    ASSERT_THAT(next_even(0ULL) == 2);
    ASSERT_THAT(next_even(1ULL) == 2);
    ASSERT_THAT(next_even(2ULL) == 4);
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

    ASSERT_THAT(solve_binary_congruence_modulo_m(24U, 1U << 24U, 43284U) == 1);
    ASSERT_THAT(solve_binary_congruence_modulo_m(24U, 1U << 24U, 39483924U) == 1);
    ASSERT_THAT(solve_binary_congruence_modulo_m(30U, 1U << 30U, 6237443U) == 1);
    ASSERT_THAT(solve_binary_congruence_modulo_m(28U, 1U << 30U, 6237443U) == 4);

#undef STRINGIFY
#undef ASSERT_THAT
#undef LOG10_ASSERT_THAT
}

// NOLINTNEXTLINE(google-readability-function-size, hicpp-function-size)
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

void test_masked_popcount_sum() noexcept {
    log_tests_started();

    auto check_n_k = [](uint32_t n, uint32_t k) noexcept {
        uint32_t correct_sum = 0;
        for (uint32_t i = 0; i <= n; i++) {
#if CONFIG_HAS_AT_LEAST_CXX_20
            const int popcnt = std::popcount(i & k);
#else
            const int popcnt = math_functions::popcount(i & k);
#endif
            correct_sum += static_cast<std::uint32_t>(popcnt);
        }
        const uint32_t fast_calc_sum = masked_popcount_sum(n, k);
        assert(correct_sum == fast_calc_sum);
    };
    constexpr uint32_t K = 5000;
    for (uint32_t n = 0; n <= K; n++) {
        for (uint32_t k = 0; k <= K; k++) {
            check_n_k(n, k);
        }
        for (uint32_t k = std::numeric_limits<uint32_t>::max() - K; k != 0; k++) {
            check_n_k(n, k);
        }
    }
}

}  // namespace

// clang-format off
// NOLINTEND(cert-dcl03-c, misc-static-assert, hicpp-static-assert, cppcoreguidelines-avoid-magic-numbers, cert-msc32-c, cert-msc51-cpp)
// clang-format on

int main() {
    test_general_asserts();
    test_isqrt();
    test_icbrt();
    test_log2();
    test_bit_reverse();
#if defined(HAS_MPFR_DURING_TESTING)
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
    test_masked_popcount_sum();
}
