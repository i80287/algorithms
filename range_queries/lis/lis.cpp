#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <type_traits>
#include <vector>

template <class T>
constexpr std::size_t longest_increasing_subsequence(const std::span<const T> seq)
    requires std::is_arithmetic_v<T>
{
    std::vector<T> dp(seq.size() + 1, std::numeric_limits<T>::max());
    dp[0] = std::numeric_limits<T>::min();
    for (std::size_t i = 0; i < seq.size(); i++) {
        auto dp_iter = std::lower_bound(dp.begin(), dp.end(), seq[i]);
        auto dp_pos  = static_cast<std::size_t>(dp_iter - dp.begin());
        assert(dp_pos < dp.size());
        assert(dp_pos == 0 || dp[dp_pos - 1] < seq[i]);
        assert(seq[i] <= dp[dp_pos]);
        *dp_iter = seq[i];
    }

    assert(std::is_sorted(dp.begin(), dp.end()));
    return static_cast<std::size_t>(
        std::lower_bound(dp.begin(), dp.end(), std::numeric_limits<T>::max()) - dp.begin());
}

template <class Container>
constexpr std::size_t longest_increasing_subsequence(const Container& seq) {
    using T = typename std::iterator_traits<decltype(std::begin(seq))>::value_type;
    return longest_increasing_subsequence(std::span<const T>(seq));
}

int main() {
    constexpr std::array arr = {1, 34, 64, 787, 2328, 3894, 439489, 43348923, 34823443};
    static_assert(longest_increasing_subsequence(arr) == arr.size());
    longest_increasing_subsequence(std::array{
        4,   23,  88,   37,  28,    72,  3,    478, 27, 3,   438, 47,   239,  84, 342, 4,
        23,  423, 4234, 3,   67824, 32,  4723, 7,   47, 68,  23,  6,    324,  37, 8,   7462,
        34,  7,   2348, 7,   48,    246, 728,  164, 2,  83,  4,   21,   0,    74, 284, 50,
        21,  34,  5,    342, 58,    74,  28,   7,   46, 28,  74,  5234, 7234, 75, 342, 36,
        4,   24,  264,  2,   187,   4,   50,   18,  7,  5,   1,   7,    243,  42, 34,  4,
        234, 25,  8,    46,  0,     14,  67,   5,   13, 587, 51,  7,    48,   56,
    });
}
