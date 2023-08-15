#include <array>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <iostream>
#include <unordered_map>
#include <vector>

template <uint64_t Mod = 0>
class CNKCounter {
    static_assert(Mod == 0 || (Mod + 1 < 0xffffffffffffffffULL / 2 && Mod + Mod > Mod),
        "Mod is so big so that adding numbers may cause overflow");
public:
    size_t max_cashed_n_;
    uint64_t** c_n_k_table_;

    explicit CNKCounter(size_t max_cashed_n) noexcept(false) : max_cashed_n_{max_cashed_n} {
        uint64_t** c_n_k = static_cast<uint64_t**>(operator new(sizeof(uint64_t*) * (max_cashed_n + 1)));
        std::memset(c_n_k, 0, sizeof(uint64_t*) * (max_cashed_n_ + 1));

        for (size_t n = 0; n <= max_cashed_n; ++n) {
            c_n_k[n] = static_cast<uint64_t*>(operator new(sizeof(uint64_t) * (n + 1)));
            c_n_k[n][0] = 1;
            c_n_k[n][n] = 1;
        }

        for (size_t n = 2; n <= max_cashed_n; ++n) {
            for (size_t k = 1; k < n; ++k) {
                if constexpr (Mod != 0) {
                    c_n_k[n][k] = (c_n_k[n - 1][k] + c_n_k[n - 1][k - 1]) % Mod;
                }
                else {
                    c_n_k[n][k] = c_n_k[n - 1][k] + c_n_k[n - 1][k - 1];
                }
            }
        }

        c_n_k_table_ = c_n_k;
    }

    uint64_t operator()(size_t n, size_t k) const noexcept {
        if (n <= max_cashed_n_) {
            return c_n_k_table_[n][k];
        }

        if (k == 0 || n == k) {
            return 1;
        }

        if (n < k) {
            return 0;
        }

        if (k > n / 2)
        {// C(n, k) = C(n, n - k+)
            k = n - k;
        }

        uint64_t C_n_1_k_1 = operator()(n - 1, k - 1);
        uint64_t C_n_1_k = operator()(n - 1, k);
        if constexpr (Mod != 0) {
            return (C_n_1_k_1 + C_n_1_k) % Mod;
        }
        else {
            return C_n_1_k_1 + C_n_1_k;
        }
    }

    CNKCounter(const CNKCounter& other) noexcept(false) : max_cashed_n_{other.max_cashed_n_} {
        uint64_t** c_n_k = static_cast<uint64_t**>(operator new(sizeof(uint64_t*) * (max_cashed_n_ + 1)));
        std::memset(c_n_k, 0, sizeof(uint64_t*) * (max_cashed_n_ + 1));

        for (size_t n = 0; n <= max_cashed_n_; ++n) {
            c_n_k[n] = static_cast<uint64_t*>(operator new(sizeof(uint64_t) * (n + 1)));
            std::memcpy(c_n_k[n], other.c_n_k_table_[n], (n + 1) * sizeof(uint64_t));
        }

        c_n_k_table_ = c_n_k;
    }

    CNKCounter& operator=(const CNKCounter& other) noexcept(false) {
        max_cashed_n_ = other.max_cashed_n_;

        uint64_t** c_n_k = static_cast<uint64_t**>(operator new(sizeof(uint64_t*) * (max_cashed_n_ + 1)));
        std::memset(c_n_k, 0, sizeof(uint64_t*) * (max_cashed_n_ + 1));

        for (size_t n = 0; n <= max_cashed_n_; ++n) {
            c_n_k[n] = static_cast<uint64_t*>(operator new(sizeof(uint64_t) * (n + 1)));
            std::memcpy(c_n_k[n], other.c_n_k_table_[n], (n + 1) * sizeof(uint64_t));
        }

        c_n_k_table_ = c_n_k;
        return *this;
    }

    constexpr CNKCounter(CNKCounter&& other) noexcept : max_cashed_n_{other.max_cashed_n_}, c_n_k_table_{other.c_n_k_table_} {
        other.max_cashed_n_ = 0;
        other.c_n_k_table_ = nullptr;
    }

    constexpr CNKCounter& operator=(CNKCounter&& other) noexcept {
        max_cashed_n_ = other.max_cashed_n_;
        c_n_k_table_ = other.c_n_k_table_;
        other.max_cashed_n_ = 0;
        other.c_n_k_table_ = nullptr;
        return *this;
    }

    ~CNKCounter() {
        for (size_t i = 0; i <= max_cashed_n_; ++i) {
            operator delete (c_n_k_table_[i]);
        }

        operator delete (c_n_k_table_);
    }
};

template <>
struct std::hash<std::pair<uint32_t, uint32_t>> {
    constexpr size_t operator()(const std::pair<uint32_t, uint32_t>& pair) const noexcept {
        if constexpr (sizeof(size_t) == 2 * sizeof(uint32_t)) {
            return (static_cast<size_t>(pair.first) << 32) | (static_cast<size_t>(pair.second));
        }
        else {
            return static_cast<size_t>(pair.first ^ pair.second);
        }
    }
};

uint64_t C_n_k(const uint32_t n, const uint32_t k) {
    static std::unordered_map<std::pair<uint32_t, uint32_t>, uint64_t> C_n_k_table;

    uint64_t diff = n - k;
    if (k == 0 || diff == 0) {
        return 1;
    }

    if (n < k) {
        return 0;
    }

    if (n > 20) {
        {
            auto it = C_n_k_table.find({n, k});
            if (it != C_n_k_table.end())
            {
                return it->second;
            }
        }

        uint64_t C_n_1_k_1 = C_n_k(n - 1, k - 1);
        uint64_t C_n_1_k = C_n_k(n - 1, k);

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

        uint64_t ans = C_n_1_k_1 + C_n_1_k;

        C_n_k_table.insert({{n, k}, ans});
        return ans;
    }

    // ans = (n * (n - 1) * ... * (n - k + 1)) / k!
    // ans = n * (n - k + 1) * ... * (n - 1) / (2 * 3 * ... * k)

    uint64_t ans = n;
    while (++diff < n)
    {// Calculate n * (n - k + 1) * (n - k + 2) * ... * (n - 1)
     // n <= 20 => n * (n - 1) * ... * (n - k + 1) <= n! <= 2432902008176640000 < 9223372036854775808 = 2 ^ 63
        ans *= diff;
    }
    
    // ans /= k!
    for (diff = 1; diff < k;)
    {// k! = (2 * 3 * ... * k) = (++1 * ++2 * ... * ++(k - 1))
        ans /= ++diff;
    }

    return ans;
}

uint64_t C_n_k_mod_M(uint32_t n, uint32_t k, uint64_t mod) {
    static std::unordered_map<std::pair<uint32_t, uint32_t>, uint64_t> C_n_k_table;

    uint64_t diff = n - k;
    if (k == 0 || diff == 0) {
        return mod != 1; // (1 % mod);
    }

    if (n < k) {
        return 0;
    }

    if (n > 20) {
        {
            auto it = C_n_k_table.find({n, k});
            if (it != C_n_k_table.end())
            {
                return it->second;
            }
        }
        
        uint64_t C_n_1_k_1 = C_n_k_mod_M(n - 1, k - 1, mod);
        uint64_t C_n_1_k = C_n_k_mod_M(n - 1, k, mod);

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
    while (++diff < n)
    {// Calculate n * (n - k + 1) * (n - k + 2) * ... * (n - 1)
     // n <= 20 => n * (n - 1) * ... * (n - k + 1) <= n! <= 2432902008176640000 < 9223372036854775808 = 2 ^ 63
        ans *= diff;
    }
    
    // ans /= k!
    for (diff = 1; diff < k;)
    {// k! = (2 * 3 * ... * k) = (++1 * ++2 * ... * ++(k - 1))
        ans /= ++diff;
    }

    return ans % mod;
}

void C_n_k_Test() {
    constexpr size_t N = 128;
    static uint64_t c_n_k[N][N] = {};

    for (size_t n = 0; n < N; ++n) {
        c_n_k[n][0] = 1;
        c_n_k[n][1] = n;
    }

    for (size_t n = 1; n < N; ++n) {
        // for (size_t k = 1; k < N; ++k) {
        //     c_n_k[n][k] = c_n_k[n - 1][k] + c_n_k[n - 1][k - 1];
        // }

        const uint64_t* c_n_1_k_values = c_n_k[n - 1];
        uint64_t* c_n_k_values = c_n_k[n];
        for (size_t k = 1; k < N; ++k) {
            c_n_k_values[k] = c_n_1_k_values[k] + c_n_1_k_values[k - 1];
        }
    }

    constexpr size_t TotalTests = 1ull << 10;
    std::mt19937 mt_prnd_engine(std::random_device{}());

    for (size_t i = 0; i < TotalTests; ++i) {
        uint_fast32_t n = mt_prnd_engine() % N;
        uint_fast32_t k = mt_prnd_engine() % N;
        uint64_t cnk0 = C_n_k(n, k);
        uint64_t cnk1 = c_n_k[n][k];

        if (cnk0 != cnk1) {
            std::cout << "C_n_k(" << n << ',' << k << ") = " << cnk0 << "; c_n_k[" << n << "][" << k << "] = " << cnk1 << '\n';
            return;
        }
    }

    std::cout << "All tests in \"" << __PRETTY_FUNCTION__ << "\" passed\n";
}

void C_n_k_mod_M_Test() {
    constexpr uint64_t Mod = 1'000'000'000 + 7;
    constexpr size_t N = 128;
    static uint64_t c_n_k[N][N] = {};

    for (size_t n = 0; n < N; ++n) {
        c_n_k[n][0] = 1;
        c_n_k[n][1] = n;
    }

    for (size_t n = 1; n < N; ++n) {
        // for (size_t k = 1; k < N; ++k) {
        //     c_n_k[n][k] = c_n_k[n - 1][k] + c_n_k[n - 1][k - 1];
        // }

        const uint64_t* c_n_1_k_values = c_n_k[n - 1];
        uint64_t* c_n_k_values = c_n_k[n];
        for (size_t k = 1; k < N; ++k) {
            c_n_k_values[k] = (c_n_1_k_values[k] + c_n_1_k_values[k - 1]) % Mod;
        }
    }

    constexpr size_t TotalTests = 1ull << 10;
    std::mt19937 mt_prnd_engine(std::random_device{}());

    for (size_t i = 0; i < TotalTests; ++i) {
        uint_fast32_t n = mt_prnd_engine() % N;
        uint_fast32_t k = mt_prnd_engine() % N;
        uint64_t cnk0 = C_n_k_mod_M(n, k, Mod);
        uint64_t cnk1 = c_n_k[n][k] % Mod;

        if (cnk0 != cnk1) {
            std::cout << "C_n_k_mod_M(" << n << ',' << k << ',' << Mod << ") = " << cnk0 << "; c_n_k[" << n << "][" << k << "] % " << Mod << " = " << cnk1 << '\n';
            return;
        }
    }

    std::cout << "All tests in \"" << __PRETTY_FUNCTION__ << "\" passed\n";
}

int main() {
    constexpr uint64_t Mod = 1e9 + 7;
    CNKCounter<Mod> C(120);
    const size_t n = 42;
    for (size_t k = 0; k <= n; ++k) {
        std::cout << C(n, k) << ' ';
    }
    std::cout << '\n';
    
    C_n_k_Test();
    C_n_k_mod_M_Test();

    return 0;
}
