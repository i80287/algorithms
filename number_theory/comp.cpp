#include <cassert>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <random>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

/// @brief Finds such integer u and v so that `a * u + b * v = gcd(a, b)`
/// gcd(a, b) >= 0
/// if a == 0 => u == 0 && v == 1 && (a * u + b * v = b = gcd(0, b))
/// if b == 0 => u == 1 && v == 0 && (a * u + b * v = a = gcd(a, 0))
/// if a != 0 => - |a| <= v <= |a|
/// if b != 0 => - |b| <= u <= |b|
/// Works in O(log(min(a, b)))
/// @tparam IntType integer type
/// @param a a value
/// @param b b value
/// @return {u, v, gcd(a, b)}
template <typename IntType>
requires std::is_integral_v<IntType>
static constexpr std::tuple<int64_t, int64_t, int64_t> ExtendedEuclidAlgorithm(IntType a, IntType b) noexcept {
    int64_t u_previous = a != 0;
    int64_t u_current = 0;
    int64_t v_previous = 0;
    int64_t v_current = 1;

    IntType r_previous = a;
    IntType r_current = b;
    while (r_current != 0)
    {
        int64_t q_current = static_cast<int64_t>(r_previous / r_current);
        IntType r_next = r_previous % r_current;

        r_previous = r_current;
        r_current = r_next;

        int64_t u_next = u_previous - u_current * q_current;
        u_previous = u_current;
        u_current = u_next;

        int64_t v_next = v_previous - v_current * q_current;
        v_previous = v_current;
        v_current = v_next;
    }

    if (r_previous < 0)
    {
        u_previous = -u_previous;
        v_previous = -v_previous;
        r_previous = -r_previous;
    }

    return {u_previous, v_previous, r_previous};
}

/// @brief Finds such integer u and v so that `a * u + b * v = gcd(a, b)`
/// Solves a * x === c (mod m)
/// Roots exist <=> c % gcd(a, m) == 0
/// If roots exists, then exactly gcd(a, m) will be returned
/// Works in O(log(min(a, m))
/// @param a 
/// @param c 
/// @param m 
/// @return vector or roots such that 0 <= x_{0} < x_{1} < ... < x_{gcd(a, m)-1} < m
[[maybe_unused]] static std::vector<uint32_t> Solve(uint64_t a, int64_t c, uint32_t m) {
    uint32_t d = static_cast<uint32_t>(std::gcd(a, m));
    if (a == 0 || m == 0 || c % static_cast<int64_t>(d) != 0) {
        return {};
    }

    /*
     * Solves a_ * x === c_ (mod m_) as gcd(a_, m_) == 1
     */
    uint64_t a_ = a / d;
    uint32_t m_ = m / d;
    // a_ * u_ + m_ * v_ == 1
    int64_t u_ = std::get<0>(ExtendedEuclidAlgorithm<uint64_t>(a_, m_));
    uint64_t unsigned_u_ = uint64_t(u_ + (m_ * (u_ < 0)));

    // a_ * (u_ * c_) + m_ * (v_ * c_) == c_
    // a * (u_ * c_) + m * (v_ * c_) == c
    int64_t c_ = c / int64_t(d);
    int64_t signed_m_ = int64_t(m_);
    uint64_t unsigned_c_ = uint64_t((c_ % signed_m_) + signed_m_) % m_;
    // x0 = u_ * c_

    // now 0 <= unsigned_u_ < m_ && 0 <= unsigned_c_ < m_ =>
    // => unsigned_u_ * unsigned_c_ wont cause overflow (since m_ <= m < 2^32)
    uint32_t x0 = uint32_t(uint64_t(((unsigned_u_ * unsigned_c_) % signed_m_) + signed_m_) % m_);

    std::vector<uint32_t> solutions(d);
    for (uint32_t& solution_i : solutions) {
        solution_i = x0;
        assert(x0 < m && x0 < x0 + d);
        x0 += m_;
    }

    return solutions;
}

[[maybe_unused]] static void MultiThreadTestsWithUnsigned() {
    constexpr size_t Limit = 1ull << 32;
    constexpr size_t TotalThreads = 12;
    constexpr size_t TestsPerThread = Limit / TotalThreads;

    std::vector<std::thread> threads;
    threads.reserve(TotalThreads);
    for (size_t i = 0; i < TotalThreads; ++i) {
        threads.emplace_back([thread_id = i]() -> void {
            printf("Entred thread %llu\n", thread_id);
            std::mt19937 mrs_rnd(std::random_device{}());
        
            for (size_t test_iter = TestsPerThread; test_iter != 0; --test_iter) {
                uint_fast32_t a;
                uint_fast32_t b;
                try {
                    a = mrs_rnd();
                    b = mrs_rnd();
                } catch (...) {
                    a = b = 0;
                }

                auto [u, v, gcd] = ExtendedEuclidAlgorithm(a, b);
                auto real_gcd = std::gcd(a, b);

                if (gcd != real_gcd) {
                    printf("In thread %llu calculated gcd != std::gcd(a, b) when a = %u, b = %u, u = %lld, v = %lld, gcd = %lld\n", thread_id, a, b, u, v, gcd);
                    return;
                }

                if (a * u + b * v != real_gcd) {
                    printf("In thread %llu a * u + b * v != std::gcd(a, b) when a = %u, b = %u, u = %lld, v = %lld\n", thread_id, a, b, u, v);
                    return;
                }

                auto u_abs = static_cast<uint64_t>(std::abs(u));
                if (!(b == 0 || u_abs <= b)) {
                    printf("In thread %llu !(b == 0 || (- |b| <= u && u <= |b|)) when a = %u, b = %u, u = %lld, v = %lld\n", thread_id, a, b, u, v);
                    return;
                }

                auto v_abs = std::abs(v);
                if (!(a == 0 || v_abs <= a)) {
                    printf("In thread %llu !(a == 0 || (- |a| <= v && v <= |a|)) when a = %u, b = %u, u = %lld, v = %lld\n", thread_id, a, b, u, v);
                    return;
                }
            }

            printf("Exited thread %llu without errors\n", thread_id);
        });
    }

    for (std::thread& thread : threads) {
        thread.join();
    }

    printf("Tests in %s ended\n", __PRETTY_FUNCTION__);
}

[[maybe_unused]] static void MultiThreadTestsWithSigned() {
    constexpr size_t Limit = 1ull << 32;
    constexpr size_t TotalThreads = 12;
    constexpr size_t TestsPerThread = Limit / TotalThreads;

    std::vector<std::thread> threads;
    threads.reserve(TotalThreads);
    for (size_t i = 0; i < TotalThreads; ++i) {
        threads.emplace_back([thread_id = i]() -> void {
            std::mt19937_64 mrs_rnd(std::random_device{}());
            printf("Entred thread %llu\n", thread_id);

            for (size_t test_iter = TestsPerThread; test_iter != 0; --test_iter) {
                int64_t a;
                int64_t b;
                try {
                    // std::mt19937_64::operator() returns uint_fast64_t
                    a = static_cast<int64_t>(mrs_rnd());
                    b = static_cast<int64_t>(mrs_rnd());
                } catch (...) {
                    a = b = 0;
                }

                auto [u, v, gcd] = ExtendedEuclidAlgorithm(a, b);
                auto real_gcd = std::gcd(a, b);
                if (gcd != real_gcd) {
                    printf("In thread %llu calculated gcd != std::gcd(a, b) when a = %lld, b = %lld, u = %lld, v = %lld, gcd = %lld\n", thread_id, a, b, u, v, gcd);
                    return;
                }

                if (a * u + b * v != real_gcd) {
                    printf("In thread %llu a * u + b * v != std::gcd(a, b) when a = %lld, b = %lld, u = %lld, v = %lld\n", thread_id, a, b, u, v);
                    return;
                }

                int64_t a_abs = std::abs(a);
                if (!(a == 0 || (-a_abs <= v && v <= a_abs))) {
                    printf("In thread %llu !(a == 0 || (- |a| <= v && v <= |a|)) when a = %lld, b = %lld, u = %lld, v = %lld\n", thread_id, a, b, u, v);
                    return;
                }

                int64_t b_abs = std::abs(b);
                if (!(b == 0 || (-b_abs <= u && u <= b_abs))) {
                    printf("In thread %llu !(b == 0 || (- |b| <= u && u <= |b|)) when a = %lld, b = %lld, u = %lld, v = %lld\n", thread_id, a, b, u, v);
                    return;
                }
            }

            printf("Exited thread %llu without errors\n", thread_id);
        });
    }

    for (std::thread& thread : threads) {
        thread.join();
    }

    printf("Tests in %s ended\n", __PRETTY_FUNCTION__);
}

[[maybe_unused]] static void TestSolve() {
    auto seed = std::ranlux24{uint32_t(std::time(nullptr))}();
    std::mt19937_64 rnd_64{seed};
    std::mt19937 rnd_32{seed};
    const size_t TOTAL_TESTS = (1 << 24);
    for (auto test_iter = TOTAL_TESTS; test_iter != 0; --test_iter) {
        uint64_t a = rnd_64();
        int64_t c = static_cast<int64_t>(rnd_64());
        uint32_t m = rnd_32();
        uint64_t mod2 = ((c % static_cast<int64_t>(m)) + static_cast<int64_t>(m)) % m;
        a %= m;
        auto res = Solve(a, c, m);
        for (auto x : res) {
            if (x >= m) {
                std::cout << "Solution " << x << " overflow\n";
                std::cout << "a = " << a << "; c = " << c << "; m = " << m << '\n';
                return;
            }

            if ((a * x) % m != mod2) {
                std::cout << "Solution " << x << " failed\n";
                std::cout << "a = " << a << "; c = " << c << "; m = " << m << '\n';
                return;
            }
        }
    }

    std::cout << "All " << __PRETTY_FUNCTION__ << " tests passed\n";
}

[[maybe_unused]] static void ConsoleTests()
{
    uint64_t a = 0;
    std::cout << "Unsigned integral a = ";
    std::cin >> a;

    int64_t c = 0;
    std::cout << "Signed integral c = ";
    std::cin >> c;

    uint32_t m = 0;
    std::cout << "Unsigned integral m = ";
    std::cin >> m;
    if (std::cin.fail()) {
        std::cout << "Input failed\n";
        return;
    }

    auto solutions = Solve(a, c, m);
    if (!solutions.empty()) {
        std::cout << "Solutions of a * x === c (mod m):\n";
        a %= m;
        uint32_t mod2 = static_cast<uint32_t>(static_cast<uint64_t>((c % static_cast<int64_t>(m)) + static_cast<int64_t>(m)) % m);
        for (uint32_t solution : solutions) {
            auto mod1 = (a * solution) % m;
            if (mod1 != mod2) {
                std::cerr << "!(a * x === c (mod m)) when a == " << a << "; c == " << c << "; m == " << m << "; wrong x = " << solution << '\n';
                return;
            }

            std::cout << solution << ' ';
        }
        std::cout << '\n';
    }
    else
    {
        std::cout << "a * x === c (mod m) has no solutions\n";
    }
}

int main() {
    TestSolve();
    MultiThreadTestsWithUnsigned();
    MultiThreadTestsWithSigned();
}

// g++ comp.cpp -std=c++2b -Ofast -march=native -fconcepts-diagnostics-depth=200 -Wconversion -Warith-conversion -Wshadow -Warray-bounds=2 -ftree-vrp -Wnull-dereference -Wall -Wextra -Wpedantic -Werror --pedantic-errors -o comp.exe