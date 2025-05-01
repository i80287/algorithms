// clang-format off
#include "dsu.hpp"
// clang-format on

#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <string_view>
#include <utility>

#include "../misc/get_typename.hpp"

// clang-format off
#include "dsu.hpp"
// clang-format on

namespace {

template <std::size_t DSUSize>
class slowdsu {
public:
    using color_t = std::uint32_t;

    constexpr slowdsu() : colors(), sets_count(DSUSize) {
        static_assert(DSUSize <= std::numeric_limits<color_t>::max());
        std::iota(colors.begin(), colors.end(), color_t{0});
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return colors.size();
    }

    [[nodiscard]] constexpr bool equal(const std::size_t x, const std::size_t y) const noexcept {
        assert(x < size() && y < size());
        return colors[x] == colors[y];
    }

    constexpr void unite(const std::size_t x, const std::size_t y) noexcept {
        assert(x < size() && y < size());
        const color_t c1 = colors[x];
        const color_t c2 = colors[y];
        if (c1 != c2) {
            sets_count--;
            std::replace_if(
                colors.begin(), colors.end(),
                [c2](const color_t c) constexpr noexcept { return c == c2; }, c1);
        }
    }

    [[nodiscard]]
    constexpr std::size_t get_size_of_node_set(const std::size_t node_index) const noexcept {
        assert(node_index < size());
        const color_t node_color = colors[node_index];
        return static_cast<std::size_t>(std::count_if(
            colors.cbegin(), colors.cend(),
            [node_color](const color_t c) constexpr noexcept { return c == node_color; }));
    }

    [[nodiscard]] constexpr std::size_t get_sets_count() const noexcept {
        return sets_count;
    }

private:
    std::array<color_t, DSUSize> colors;
    std::size_t sets_count;
};

template <class DsuType>
static void test_manual() {
    constexpr size_t N = 40;
    constexpr bool is_weighted = std::is_same_v<DsuType, weighted_dsu_t>;

    DsuType tree = DsuType::with_nodes_count(N);

    for (size_t i = 1; i < N; i++) {
        assert(!tree.equal(i - 1, i));
    }

    for (size_t i = 0; i < N; i++) {
        assert(tree.equal(i, i));
    }

    tree.unite(0, 1);
    tree.unite(2, 3);
    tree.unite(0, 3);
    for (size_t i = 0; i <= 3; i++) {
        for (size_t j = 0; j <= 3; j++) {
            assert(tree.equal(i, j));
        }
    }

    if constexpr (is_weighted) {
        tree.add_weight_in_set(0, 10);
        tree.add_weight_in_set(2, 10);
        for (size_t i = 0; i <= 3; i++) {
            assert(tree.get_weight_in_set(i) == 20);
        }

        tree.set_weight_in_set(0, 10);
        for (size_t i = 0; i <= 3; i++) {
            assert(tree.get_weight_in_set(i) == 10);
        }
    }

    for (size_t i = 4; i < N; i++) {
        assert(!tree.equal(i - 1, i));
    }

    /*
     *     .--37---.
     *    /   /     \
     *  35   36     39
     *  /            \
     * 34            38
     *
     */
    tree.unite(34, 35);
    if constexpr (is_weighted) {
        tree.add_weight_in_set(34, 2);
    }
    assert(tree.equal(34, 35));
    assert(!tree.equal(35, 36));
    assert(!tree.equal(36, 37));
    assert(!tree.equal(37, 38));
    assert(!tree.equal(38, 39));
    if constexpr (is_weighted) {
        assert(tree.get_weight_in_set(34) == 2);
        assert(tree.get_weight_in_set(35) == 2);
        assert(tree.get_weight_in_set(36) == 0);
        assert(tree.get_weight_in_set(37) == 0);
        assert(tree.get_weight_in_set(38) == 0);
        assert(tree.get_weight_in_set(39) == 0);
    }
    tree.unite(36, 37);
    if constexpr (is_weighted) {
        tree.add_weight_in_set(37, 3);
    }
    assert(tree.equal(34, 35));
    assert(!tree.equal(35, 36));
    assert(tree.equal(36, 37));
    assert(!tree.equal(37, 38));
    assert(!tree.equal(38, 39));
    if constexpr (is_weighted) {
        assert(tree.get_weight_in_set(34) == 2);
        assert(tree.get_weight_in_set(35) == 2);
        assert(tree.get_weight_in_set(36) == 3);
        assert(tree.get_weight_in_set(37) == 3);
        assert(tree.get_weight_in_set(38) == 0);
        assert(tree.get_weight_in_set(39) == 0);
    }
    tree.unite(38, 39);
    if constexpr (is_weighted) {
        tree.add_weight_in_set(38, 4);
    }
    assert(tree.equal(34, 35));
    assert(!tree.equal(35, 36));
    assert(tree.equal(36, 37));
    assert(!tree.equal(37, 38));
    assert(tree.equal(38, 39));
    if constexpr (is_weighted) {
        assert(tree.get_weight_in_set(34) == 2);
        assert(tree.get_weight_in_set(35) == 2);
        assert(tree.get_weight_in_set(36) == 3);
        assert(tree.get_weight_in_set(37) == 3);
        assert(tree.get_weight_in_set(38) == 4);
        assert(tree.get_weight_in_set(39) == 4);
    }
    tree.unite(35, 37);
    assert(tree.equal(34, 35));
    assert(tree.equal(35, 36));
    assert(tree.equal(36, 37));
    assert(!tree.equal(37, 38));
    assert(tree.equal(38, 39));
    if constexpr (is_weighted) {
        assert(tree.get_weight_in_set(34) == 5);
        assert(tree.get_weight_in_set(35) == 5);
        assert(tree.get_weight_in_set(36) == 5);
        assert(tree.get_weight_in_set(37) == 5);
        assert(tree.get_weight_in_set(38) == 4);
        assert(tree.get_weight_in_set(39) == 4);
    }
    tree.unite(37, 38);
    assert(tree.equal(34, 35));
    assert(tree.equal(35, 36));
    assert(tree.equal(36, 37));
    assert(tree.equal(37, 38));
    assert(tree.equal(38, 39));
    if constexpr (is_weighted) {
        assert(tree.get_weight_in_set(34) == 9);
        assert(tree.get_weight_in_set(35) == 9);
        assert(tree.get_weight_in_set(36) == 9);
        assert(tree.get_weight_in_set(37) == 9);
        assert(tree.get_weight_in_set(38) == 9);
        assert(tree.get_weight_in_set(39) == 9);
    }
    for (size_t i = 34; i <= 39; i++) {
        for (size_t j = 34; j <= 39; j++) {
            assert(tree.equal(i, j));
        }
    }

    for (size_t i = 1; i < N; i++) {
        tree.unite(i - 1, i);
    }

    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            assert(tree.equal(i, j));
        }

        if constexpr (is_weighted) {
            assert(tree.get_weight_in_set(i) == 10 + 9);
        }
    }

    if constexpr (is_weighted) {
        std::vector<int64_t> vec{1, 2, 4, 8, 16, 32, 64};
        size_t n = vec.size();
        DsuType wdsu = DsuType::from_weights_vec(vec);
        for (size_t i = 0; i < n; i++) {
            assert(wdsu.get_weight_in_set(i) == vec[i]);
        }

        wdsu.unite(0, 1);
        wdsu.unite(2, 3);
        wdsu.unite(0, 2);
        int64_t sum = vec[0] + vec[1] + vec[2] + vec[3];
        for (size_t i = 0; i <= 3; i++) {
            assert(wdsu.get_weight_in_set(i) == sum);
        }

        for (size_t i = 1; i < n; i++) {
            wdsu.unite(i - 1, i);
        }

        for (size_t i = 4; i < n; i++) {
            sum += vec[i];
        }

        for (size_t i = 0; i < n; i++) {
            assert(wdsu.get_weight_in_set(i) == sum);
        }
    }
}

template <class DsuType>
static void test_value_semantic() {
    DsuType d1 = DsuType::with_nodes_count(4);

    d1.unite(0, 1);
    d1.unite(2, 3);
    auto d2 = d1;
    assert(d2.size() == d1.size());
    assert(d2.get_sets_count() == d1.get_sets_count());
    assert(d2.equal(0, 1));
    assert(d2.equal(2, 3));
    assert(!d2.equal(0, 2));
    assert(!d2.equal(0, 3));
    assert(!d2.equal(1, 2));
    assert(!d2.equal(1, 3));
    auto d3 = std::move(d1);
    d2 = std::move(d3);
    assert(d2.equal(0, 1));
    assert(d2.equal(2, 3));
    assert(!d2.equal(0, 2));
    assert(!d2.equal(0, 3));
    assert(!d2.equal(1, 2));
    assert(!d2.equal(1, 3));
    d3 = d2;
    assert(d2.size() == d3.size());
    assert(d2.get_sets_count() == d3.get_sets_count());
    assert(d3.equal(0, 1));
    assert(d3.equal(2, 3));
    assert(!d3.equal(0, 2));
    assert(!d3.equal(0, 3));
    assert(!d3.equal(1, 2));
    assert(!d3.equal(1, 3));

    constexpr std::size_t size_d4 = 9;
    constexpr std::size_t size_d5 = 10;

    DsuType d4 = DsuType::with_nodes_count(size_d4);
    DsuType d5 = DsuType::with_nodes_count(size_d5);

    assert(d4.size() == size_d4);
    assert(d4.get_sets_count() == size_d4);
    assert(d5.size() == size_d5);
    assert(d5.get_sets_count() == size_d5);

    constexpr std::pair<size_t, size_t> unites_d4[] = {
        {1, 2}, {3, 4}, {5, 6}, {0, 6}, {1, 7},
    };
    constexpr std::pair<size_t, size_t> unites_d5[] = {
        {1, 5}, {2, 4}, {5, 3}, {7, 8}, {6, 0}, {9, 2},
    };
    for (auto&& [x, y] : unites_d4) {
        d4.unite(x, y);
    }
    for (auto&& [x, y] : unites_d5) {
        d5.unite(x, y);
    }

    d4.swap(d5);
    assert(d4.size() == size_d5);
    assert(d5.size() == size_d4);

    for (auto&& [x, y] : unites_d4) {
        assert(d5.equal(x, y));
    }
    for (auto&& [x, y] : unites_d5) {
        assert(d4.equal(x, y));
    }

    swap(d4, d5);
    assert(d4.size() == size_d4);
    assert(d5.size() == size_d5);

    for (auto&& [x, y] : unites_d4) {
        assert(d4.equal(x, y));
    }
    for (auto&& [x, y] : unites_d5) {
        assert(d5.equal(x, y));
    }
}

template <class DsuType>
static void test_random_with_check() {
    constexpr size_t N = 1500;
    DsuType dsu = DsuType::with_nodes_count(N);
    assert(dsu.size() == N);
    slowdsu<N> checker;
    std::mt19937 rnd;

    const auto compare = [&]() constexpr noexcept -> bool {
        if (checker.get_sets_count() != dsu.get_sets_count()) {
            return false;
        }

        for (size_t i = 0; i < N; i++) {
            const auto s = checker.get_size_of_node_set(i);
            if (s != dsu.get_size_of_node_set(i)) {
                return false;
            }
            for (size_t j = 0; j < N; j++) {
                if (checker.equal(i, j) != dsu.equal(i, j)) {
                    return false;
                }
            }
        }
        return true;
    };

    for (size_t iters = N; iters > 0; iters--) {
        size_t x = rnd() % N;
        size_t y = rnd() % N;

        checker.unite(x, y);
        dsu.unite(x, y);
        assert(compare());
    }
}

template <class DsuType>
void test_dsu() {
    constexpr std::string_view tname = misc::get_typename<DsuType>();
    std::cout << "Started testing type \"" << tname << "\"\n";

    // test_manual<DsuType>();
    // test_value_semantic<DsuType>();
    test_random_with_check<DsuType>();
}
}  // namespace

int main() {
    test_dsu<dsu_t>();
    test_dsu<weighted_dsu_t>();
}
