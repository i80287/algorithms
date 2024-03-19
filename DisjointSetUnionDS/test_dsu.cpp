#include <algorithm>
#include <array>
#include <iostream>
#include <numeric>
#include <random>

#include "dsu.hpp"

template <size_t DSUSize>
struct slowdsu {
    std::array<size_t, DSUSize> colors;
    size_t sets = DSUSize;

    constexpr slowdsu() { resetData(); }

    constexpr bool equal(size_t x, size_t y) const noexcept {
        return colors[x] == colors[y];
    }

    constexpr void unite(size_t x, size_t y) noexcept {
        size_t c1 = colors[x];
        size_t c2 = colors[y];
        if (c1 != c2) {
            sets--;
            std::replace_if(
                colors.begin(), colors.end(),
                [c2](size_t c) constexpr noexcept -> bool { return c == c2; }, c1);
        }
    }

    constexpr void resetData() noexcept {
        sets = DSUSize;
        std::iota(colors.begin(), colors.end(), 0);
    }
};

void ConsoleTest() {
    dsu_t tree(10);
    std::cout << std::boolalpha;

    std::cout << "0 eq 1: " << tree.equal(0, 1) << '\n';
    tree.unite(0, 1);
    std::cout << "0 eq 1: " << tree.equal(0, 1) << '\n';

    tree.unite(1, 2);
    tree.unite(2, 3);
    tree.unite(4, 3);
    tree.unite(4, 9);
    std::cout << "1 eq 9: " << tree.equal(1, 9) << '\n';
    std::cout << "1 eq 8: " << tree.equal(1, 8) << '\n';

    tree.unite(8, 9);

    std::cout << "1 eq 8: " << tree.equal(1, 8) << '\n';
}

void TestDSU() {
    {
        constexpr size_t N = 40;
        dsu_t tree(N);
        assert(tree.size() == N);

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
        assert(tree.equal(34, 35));
        assert(!tree.equal(35, 36));
        assert(!tree.equal(36, 37));
        assert(!tree.equal(37, 38));
        assert(!tree.equal(38, 39));
        tree.unite(36, 37);
        assert(tree.equal(34, 35));
        assert(!tree.equal(35, 36));
        assert(tree.equal(36, 37));
        assert(!tree.equal(37, 38));
        assert(!tree.equal(38, 39));
        tree.unite(38, 39);
        assert(tree.equal(34, 35));
        assert(!tree.equal(35, 36));
        assert(tree.equal(36, 37));
        assert(!tree.equal(37, 38));
        assert(tree.equal(38, 39));
        tree.unite(35, 37);
        assert(tree.equal(34, 35));
        assert(tree.equal(35, 36));
        assert(tree.equal(36, 37));
        assert(!tree.equal(37, 38));
        assert(tree.equal(38, 39));
        tree.unite(37, 38);
        assert(tree.equal(34, 35));
        assert(tree.equal(35, 36));
        assert(tree.equal(36, 37));
        assert(tree.equal(37, 38));
        assert(tree.equal(38, 39));

        for (size_t i = 1; i < N; i++) {
            tree.unite(i - 1, i);
        }

        for (size_t i = 0; i < N; i++) {
            for (size_t j = 0; j < N; j++) {
                assert(tree.equal(i, j));
            }
        }
    }

    {
        dsu_t d1(4);
        d1.unite(0, 1);
        d1.unite(2, 3);
        auto d2 = d1;
        auto d3 = std::move(d1);
        d2      = std::move(d3);
        assert(d2.equal(0, 1));
        assert(d2.equal(2, 3));
        assert(!d2.equal(0, 2));
        assert(!d2.equal(0, 3));
        assert(!d2.equal(1, 2));
        assert(!d2.equal(1, 3));
        d3 = d2;
        assert(d3.equal(0, 1));
        assert(d3.equal(2, 3));
        assert(!d3.equal(0, 2));
        assert(!d3.equal(0, 3));
        assert(!d3.equal(1, 2));
        assert(!d3.equal(1, 3));
    }

    {
        constexpr size_t N = 1e3;
        dsu_t dsu(N);
        assert(dsu.size() == N);
        slowdsu<N> checker;
        std::mt19937 rnd;

        auto compare = [&]() constexpr noexcept -> bool {
            if (checker.sets != dsu.sets()) {
                return false;
            }

            for (size_t i = 0; i < N; i++) {
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

    puts("dsu_t tests passed");
}

void TestWightedDSU() {
    constexpr size_t N = 40;
    weighted_dsu_t tree(N);

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

    tree.addWeightInSet(0, 10);
    tree.addWeightInSet(2, 10);
    for (size_t i = 0; i <= 3; i++) {
        assert(tree.getWeightInSet(i) == 20);
    }

    tree.setWeightInSet(0, 10);
    for (size_t i = 0; i <= 3; i++) {
        assert(tree.getWeightInSet(i) == 10);
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
    tree.addWeightInSet(34, 2);
    assert(tree.equal(34, 35));
    assert(!tree.equal(35, 36));
    assert(!tree.equal(36, 37));
    assert(!tree.equal(37, 38));
    assert(!tree.equal(38, 39));
    assert(tree.getWeightInSet(34) == 2);
    assert(tree.getWeightInSet(35) == 2);
    assert(tree.getWeightInSet(36) == 0);
    assert(tree.getWeightInSet(37) == 0);
    assert(tree.getWeightInSet(38) == 0);
    assert(tree.getWeightInSet(39) == 0);
    tree.unite(36, 37);
    tree.addWeightInSet(37, 3);
    assert(tree.equal(34, 35));
    assert(!tree.equal(35, 36));
    assert(tree.equal(36, 37));
    assert(!tree.equal(37, 38));
    assert(!tree.equal(38, 39));
    assert(tree.getWeightInSet(34) == 2);
    assert(tree.getWeightInSet(35) == 2);
    assert(tree.getWeightInSet(36) == 3);
    assert(tree.getWeightInSet(37) == 3);
    assert(tree.getWeightInSet(38) == 0);
    assert(tree.getWeightInSet(39) == 0);
    tree.unite(38, 39);
    tree.addWeightInSet(38, 4);
    assert(tree.equal(34, 35));
    assert(!tree.equal(35, 36));
    assert(tree.equal(36, 37));
    assert(!tree.equal(37, 38));
    assert(tree.equal(38, 39));
    assert(tree.getWeightInSet(34) == 2);
    assert(tree.getWeightInSet(35) == 2);
    assert(tree.getWeightInSet(36) == 3);
    assert(tree.getWeightInSet(37) == 3);
    assert(tree.getWeightInSet(38) == 4);
    assert(tree.getWeightInSet(39) == 4);
    tree.unite(35, 37);
    assert(tree.equal(34, 35));
    assert(tree.equal(35, 36));
    assert(tree.equal(36, 37));
    assert(!tree.equal(37, 38));
    assert(tree.equal(38, 39));
    assert(tree.getWeightInSet(34) == 5);
    assert(tree.getWeightInSet(35) == 5);
    assert(tree.getWeightInSet(36) == 5);
    assert(tree.getWeightInSet(37) == 5);
    assert(tree.getWeightInSet(38) == 4);
    assert(tree.getWeightInSet(39) == 4);
    tree.unite(37, 38);
    assert(tree.equal(34, 35));
    assert(tree.equal(35, 36));
    assert(tree.equal(36, 37));
    assert(tree.equal(37, 38));
    assert(tree.equal(38, 39));
    assert(tree.getWeightInSet(34) == 9);
    assert(tree.getWeightInSet(35) == 9);
    assert(tree.getWeightInSet(36) == 9);
    assert(tree.getWeightInSet(37) == 9);
    assert(tree.getWeightInSet(38) == 9);
    assert(tree.getWeightInSet(39) == 9);
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

        assert(tree.getWeightInSet(i) == 10 + 9);
    }

    std::vector<int64_t> vec{1, 2, 4, 8, 16, 32, 64};
    size_t n = vec.size();
    weighted_dsu_t wdsu{vec};
    for (size_t i = 0; i < n; i++) {
        assert(wdsu.getWeightInSet(i) == vec[i]);
    }

    wdsu.unite(0, 1);
    wdsu.unite(2, 3);
    wdsu.unite(0, 2);
    int64_t sum = vec[0] + vec[1] + vec[2] + vec[3];
    for (size_t i = 0; i <= 3; i++) {
        assert(wdsu.getWeightInSet(i) == sum);
    }

    for (size_t i = 1; i < n; i++) {
        wdsu.unite(i - 1, i);
    }

    for (size_t i = 4; i < n; i++) {
        sum += vec[i];
    }

    for (size_t i = 0; i < n; i++) {
        assert(wdsu.getWeightInSet(i) == sum);
    }

    puts("weighted_dsu_t tests passed");
}

int main() {
    TestDSU();
    TestWightedDSU();
}
