#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <unordered_map>
#include <utility>

#include "CNKCounter.hpp"
#include "test_tools.hpp"

using std::size_t;
using std::uint32_t;
using std::uint64_t;

struct NK final {
    uint32_t n{};
    uint32_t k{};

#if CONFIG_HAS_AT_LEAST_CXX_20
    constexpr bool operator==(const NK&) const noexcept = default;
#else
    constexpr bool operator==(const NK& other) const noexcept {
        return n == other.n && k == other.k;
    }
#endif
};

template <>
struct std::hash<NK> {
    constexpr size_t operator()(const NK& pair) const noexcept {
        if constexpr (sizeof(size_t) == 2 * sizeof(uint32_t)) {
            return (static_cast<size_t>(pair.n) << 32) | (static_cast<size_t>(pair.k));
        } else {
            return static_cast<size_t>(pair.n ^ pair.k);
        }
    }
};

namespace {

uint64_t C_n_k(const uint32_t n, uint32_t k) {
    static std::unordered_map<NK, uint64_t> C_n_k_table;

    if (n < k) {
        return 0;
    }
    // C(n, k) = C(n, n - k)
    k = std::min(k, n - k);
    switch (k) {
        case 0:
            return 1;
        case 1:
            return n;
        default:
            break;
    }

    if (n > 20) {
        {
            auto it = C_n_k_table.find(NK{n, k});
            if (it != C_n_k_table.end()) {
                return it->second;
            }
        }

        auto C_n_1_k_1 = C_n_k(n - 1, k - 1);
        auto C_n_1_k   = C_n_k(n - 1, k);

        // C(n - 1, k) = C(n - 1, k - 1) * (n - k) / k

        // uint64_t C_n_1_k;
        // if (C_n_1_k_1 % k == 0)
        // {
        //     C_n_1_k = (C_n_1_k_1 / k) * diff;
        // }
        // else
        // {
        //     if (diff % k == 0)
        //     {
        //         C_n_1_k = C_n_1_k_1 * (diff / k);
        //     }
        //     else
        //     {
        //         C_n_1_k = (C_n_1_k_1 * diff) / k;
        //     }
        // }

        // uint64_t q = C_n_1_k_1 / k;
        // uint64_t C_n_1_k =
        //     C_n_1_k_1 == q * k
        //     ? (q * diff) // C_n_1_k_1 % k == 0;
        //     : (
        //         (diff % k == 0)
        //         ? C_n_1_k_1 * (diff / k)
        //         : (C_n_1_k_1 * diff) / k
        //     );

        auto ans = C_n_1_k_1 + C_n_1_k;

        C_n_k_table.insert({{n, k}, ans});
        return ans;
    }

    // ans = (n * (n - 1) * ... * (n - k + 1)) / k!

    // Calculate n * (n - 1) * ... * (n - k + 1)
    // n <= 20 => n * (n - 1) * ... * (n - k + 1) <= n! <= 2432902008176640000 < 9223372036854775808
    // = 2 ^ 63 k >= 2
    uint64_t mult = n - k + 1;
    auto ans      = mult;
    do {
        mult++;
        ans *= mult;
    } while (mult < n);

    // ans /= k!
    mult = k;
    do {
        ans /= mult;
        mult--;
    } while (mult >= 2);

    return ans;
}

uint64_t C_n_k_mod_M(uint32_t n, uint32_t k, uint64_t mod) {
    static std::unordered_map<NK, uint64_t> C_n_k_table;

    uint64_t diff = n - k;
    if (k == 0 || diff == 0) {
        return mod != 1;  // (1 % mod);
    }

    if (n < k) {
        return 0;
    }

    if (n > 20) {
        {
            auto it = C_n_k_table.find(NK{n, k});
            if (it != C_n_k_table.end()) {
                return it->second;
            }
        }

        uint64_t C_n_1_k_1 = C_n_k_mod_M(n - 1, k - 1, mod);
        uint64_t C_n_1_k   = C_n_k_mod_M(n - 1, k, mod);

        // Can be optimized: calculate C(n - 1, k) if ((n - k) < (k - 0))
        // const uint64_t q = C_n_1_k_1 / k;
        // const uint64_t C_n_1_k =
        //     (C_n_1_k_1 == q * k) // C_n_1_k_1 % k == 0;
        //     ? (q * diff)
        //     : ((C_n_1_k_1 * diff) / k);

        uint64_t ans = ((C_n_1_k_1 + C_n_1_k) % mod + mod) % mod;
        C_n_k_table.insert({{n, k}, ans});
        return ans;
    }

    // ans = (n * (n - 1) * ... * (n - k + 1)) / k!
    // ans = n * (n - k + 1) * ... * (n - 1) / (2 * 3 * ... * k)

    uint64_t ans = n;
    while (++diff < n) {  // Calculate n * (n - k + 1) * (n - k + 2) * ... * (n - 1)
                          // n <= 20 => n * (n - 1) * ... * (n - k + 1) <= n! <= 2432902008176640000
                          // < 9223372036854775808 = 2 ^ 63
        ans *= diff;
    }

    // ans /= k!
    for (diff = 1; diff < k;) {  // k! = (2 * 3 * ... * k) = (++1 * ++2 * ... * ++(k - 1))
        ans /= ++diff;
    }

    return ans % mod;
}

void C_n_k_Test() {
    test_tools::log_tests_started();
    constexpr size_t N          = 256;
    static uint64_t c_n_k[N][N] = {};
    CNKCounter<> c_n_k_counter(uint32_t{N} - 10);

    for (size_t n = 0; n < N; ++n) {
        c_n_k[n][0] = 1;
        c_n_k[n][1] = n;
    }

    for (size_t n = 1; n < N; ++n) {
        // for (size_t k = 1; k < N; ++k) {
        //     c_n_k[n][k] = c_n_k[n - 1][k] + c_n_k[n - 1][k - 1];
        // }

        const uint64_t* c_n_1_k_values = c_n_k[n - 1];
        uint64_t* c_n_k_values         = c_n_k[n];
        for (size_t k = 1; k < N; ++k) {
            c_n_k_values[k] = c_n_1_k_values[k] + c_n_1_k_values[k - 1];
        }
    }

    for (uint32_t n = 0; n < N; n++) {
        for (uint32_t k = 0; k < N; k++) {
            const uint64_t cnk0 = C_n_k(n, k);
            const uint64_t cnk1 = c_n_k[n][k];
            assert(cnk0 == cnk1);
            const uint64_t cnk2 = c_n_k_counter(n, k);
            assert(cnk1 == cnk2);
        }
    }
}

void C_n_k_mod_M_Test() {
    test_tools::log_tests_started();
    constexpr uint32_t Mod      = 1'000'000'000 + 7;
    constexpr size_t N          = 256;
    static uint64_t c_n_k[N][N] = {};
    CNKCounter<Mod> c_n_k_counter(uint32_t{N} - 10);

    for (size_t n = 0; n < N; ++n) {
        c_n_k[n][0] = 1;
        c_n_k[n][1] = n;
    }

    for (size_t n = 1; n < N; ++n) {
        // for (size_t k = 1; k < N; ++k) {
        //     c_n_k[n][k] = c_n_k[n - 1][k] + c_n_k[n - 1][k - 1];
        // }

        const uint64_t* c_n_1_k_values = c_n_k[n - 1];
        uint64_t* c_n_k_values         = c_n_k[n];
        for (size_t k = 1; k < N; ++k) {
            c_n_k_values[k] = (c_n_1_k_values[k] + c_n_1_k_values[k - 1]) % Mod;
        }
    }

    constexpr size_t TotalTests = 1ull << 10;
    std::mt19937 mt_prnd_engine(std::random_device{}());

    for (size_t i = 0; i < TotalTests; ++i) {
        const auto n        = static_cast<uint32_t>(mt_prnd_engine() % N);
        const auto k        = static_cast<uint32_t>(mt_prnd_engine() % N);
        const uint64_t cnk0 = C_n_k_mod_M(n, k, Mod);
        const uint64_t cnk1 = c_n_k[n][k] % Mod;
        assert(cnk0 == cnk1);
        const uint64_t cnk2 = c_n_k_counter(n, k);
        assert(cnk1 == cnk2);
    }
}

}  // namespace

int main() {
    C_n_k_Test();
    C_n_k_mod_M_Test();
    return 0;
}
