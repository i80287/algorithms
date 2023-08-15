#include <cstdint>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <random>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

/**
 * @brief Finds such integer u and v so that `a * u + b * v = gcd(a, b)`
 * gcd(a, b) >= 0
 * if a == 0 => u == 0 && v == 1 && (a * u + b * v = b = gcd(0, b))
 * if b == 0 => u == 1 && v == 0 && (a * u + b * v = a = gcd(a, 0))
 * if a != 0 => - |a| <= v <= |a|
 * if b != 0 => - |b| <= u <= |b|
 */
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

/*
 * Solves a * x === c (mod m)
 */
[[maybe_unused]] static std::vector<int64_t> Solve(uint64_t a, int64_t c, uint64_t m) {
    uint64_t d = std::gcd(a, m);
    if (a == 0 || m == 0 || c % d != 0)
    {
        return {};
    }

    /*
     * Solves a_ * x === c_ (mod m_) as gcd(a_, m_) == 1
     */
    uint64_t a_ = a / d;
    uint64_t m_ = m / d;
    // a_ * u_ + m_ * v_ == 1
    int64_t u_ = std::get<0>(ExtendedEuclidAlgorithm(a_, m_));

    // a_ * (u_ * c_) + m_ * (v_ * c_) == c_
    // a * (u_ * c_) + m * (v_ * c_) == c
    int64_t signed_d = static_cast<int64_t>(d);
    int64_t c_ = c / signed_d;
    int64_t x0 = u_ * c_;

    std::vector<int64_t> solutions(d);
    for (int64_t& solution_i : solutions)
    {
        solution_i = x0;
        x0 += signed_d;
    }

    return solutions;
}

[[maybe_unused]] static void MultiThreadTestsWithUnsigned() {
    constexpr size_t Limit = 1ull << 32;
    constexpr size_t TotalThreads = 12;
    constexpr size_t TestsPerThread = Limit / TotalThreads;

    std::vector<std::thread> threads;
    threads.reserve(TotalThreads);
    for (size_t i = 0; i < TotalThreads; ++i)
    {
        threads.emplace_back([thread_id = i]() -> void {
            printf("Entred thread %llu\n", thread_id);
            std::random_device rnd;
            std::mt19937 mrs_rnd_engine(rnd());
        
            for (size_t test_iter = 0; test_iter != TestsPerThread; ++test_iter)
            {
                uint_fast32_t a = mrs_rnd_engine();
                uint_fast32_t b = mrs_rnd_engine();
                auto [u, v, gcd] = ExtendedEuclidAlgorithm(a, b);
                auto real_gcd = std::gcd(a, b);

                if (gcd != real_gcd)
                {
                    printf("In thread %llu calculated gcd != std::gcd(a, b) when a = %u, b = %u, u = %lld, v = %lld, gcd = %lld\n", thread_id, a, b, u, v, gcd);
                    return;
                }

                if (a * u + b * v != real_gcd)
                {
                    printf("In thread %llu a * u + b * v != std::gcd(a, b) when a = %u, b = %u, u = %lld, v = %lld\n", thread_id, a, b, u, v);
                    return;
                }

                auto u_abs = static_cast<uint64_t>(std::abs(u));
                if (!(b == 0 || u_abs <= b))
                {
                    printf("In thread %llu !(b == 0 || (- |b| <= u && u <= |b|)) when a = %u, b = %u, u = %lld, v = %lld\n", thread_id, a, b, u, v);
                    return;
                }

                auto v_abs = std::abs(v);
                if (!(a == 0 || v_abs <= a))
                {
                    printf("In thread %llu !(a == 0 || (- |a| <= v && v <= |a|)) when a = %u, b = %u, u = %lld, v = %lld\n", thread_id, a, b, u, v);
                    return;
                }
            }

            printf("Exited thread %llu without errors\n", thread_id);
        });
    }

    for (std::thread& thread : threads)
    {
        thread.join();
    }
}

[[maybe_unused]] static void MultiThreadTestsWithSigned()
{
    constexpr size_t Limit = 1ull << 32;
    constexpr size_t TotalThreads = 12;
    constexpr size_t TestsPerThread = Limit / TotalThreads;

    std::vector<std::thread> threads;
    threads.reserve(TotalThreads);
    for (size_t i = 0; i < TotalThreads; ++i)
    {
        threads.emplace_back([thread_id = i]() -> void {
            printf("Entred thread %llu\n", thread_id);
            std::random_device rnd;
            std::mt19937_64 mrs_rnd_engine(rnd());

            for (size_t test_iter = 0; test_iter != TestsPerThread; ++test_iter)
            {
                // std::mt19937_64::operator() returns uint_fast64_t
                int64_t a = static_cast<int64_t>(mrs_rnd_engine());
                int64_t b = static_cast<int64_t>(mrs_rnd_engine());

                auto [u, v, gcd] = ExtendedEuclidAlgorithm(a, b);
                auto real_gcd = std::gcd(a, b);
                if (gcd != real_gcd)
                {
                    printf("In thread %llu calculated gcd != std::gcd(a, b) when a = %lld, b = %lld, u = %lld, v = %lld, gcd = %lld\n", thread_id, a, b, u, v, gcd);
                    return;
                }

                if (a * u + b * v != real_gcd)
                {
                    printf("In thread %llu a * u + b * v != std::gcd(a, b) when a = %lld, b = %lld, u = %lld, v = %lld\n", thread_id, a, b, u, v);
                    return;
                }

                int64_t a_abs = std::abs(a);
                if (!(a == 0 || (-a_abs <= v && v <= a_abs)))
                {
                    printf("In thread %llu !(a == 0 || (- |a| <= v && v <= |a|)) when a = %lld, b = %lld, u = %lld, v = %lld\n", thread_id, a, b, u, v);
                    return;
                }

                int64_t b_abs = std::abs(b);
                if (!(b == 0 || (-b_abs <= u && u <= b_abs)))
                {
                    printf("In thread %llu !(b == 0 || (- |b| <= u && u <= |b|)) when a = %lld, b = %lld, u = %lld, v = %lld\n", thread_id, a, b, u, v);
                    return;
                }
            }

            printf("Exited thread %llu without errors\n", thread_id);
        });
    }

    for (std::thread& thread : threads)
    {
        thread.join();
    }
}

[[maybe_unused]] static void ConsoleTests()
{
    uint64_t a = 0;
    std::cout << "Unsigned integral a = ";
    std::cin >> a;

    int64_t c = 0;
    std::cout << "Signed integral c = ";
    std::cin >> c;

    uint64_t m = 0;
    std::cout << "Unsigned integral m = ";
    std::cin >> m;

    auto solutions = Solve(a, c, m);
    if (!solutions.empty())
    {
        std::cout << "Solutions of a * x === c (mod m):\n";
        auto signed_a = static_cast<int64_t>(a);
        auto signed_m = static_cast<int64_t>(m);
        for (int64_t solution : solutions)
        {
            auto mod1 = static_cast<uint64_t>(((signed_a * solution) % signed_m) + signed_m) % m;
            auto mod2 = static_cast<uint64_t>((c % signed_m) + signed_m) % m;
            if (mod1 != mod2)
            {
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

int main()
{
    MultiThreadTestsWithSigned();
    // ConsoleTests();
}
