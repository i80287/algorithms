#include <array>
#include <random>

#include "segment_trees.hpp"

template <UpdateOperation upd_op, GetOperation get_op, typename value_t = int64_t>
class [[nodiscard]] SegTreeChecker {
public:
    explicit SegTreeChecker(const std::vector<value_t>& data)
        : SegTreeChecker(data.data(), uint32_t(data.size())) {}
    template <std::size_t N>
    explicit SegTreeChecker(const std::array<value_t, N>& data)
        : SegTreeChecker(data.data(), uint32_t(data.size())) {}
    template <std::size_t N>
    explicit SegTreeChecker(const value_t (&data)[N])
        : SegTreeChecker(std::data(data), uint32_t(std::size(data))) {}
    explicit SegTreeChecker(std::initializer_list<value_t> data)
        : SegTreeChecker(std::data(data), uint32_t(std::size(data))) {}

    SegTreeChecker(const value_t* data, uint32_t n) : values_(data, n) {
        assert(data != nullptr);
        assert(n > 0);
    }

    void update(uint32_t l, uint32_t r, value_t value) noexcept {
        assert(l <= r && r < values_.size());
        for (size_t i = l; i <= r; i++) {
            if constexpr (upd_op == UpdateOperation::add) {
                values_[i] += value;
            } else if constexpr (upd_op == UpdateOperation::multiply) {
                values_[i] *= value;
            } else if constexpr (upd_op == UpdateOperation::set_equal) {
                values_[i] = value;
            }
        }
    }

    [[nodiscard]] value_t get(uint32_t l, uint32_t r) const noexcept {
        assert(l <= r && r < values_.size());
        value_t ans = values_[l];
        for (size_t i = l + 1; i <= r; i++) {
            if constexpr (get_op == GetOperation::sum) {
                ans += values_[i];
            } else if constexpr (get_op == GetOperation::product) {
                ans *= values_[i];
            } else if constexpr (get_op == GetOperation::max) {
                ans = std::max(ans, values_[i]);
            } else if constexpr (get_op == GetOperation::min) {
                ans = std::min(ans, values_[i]);
            }
        }

        return ans;
    }

private:
    std::valarray<value_t> values_;
};

template <typename value_t, std::size_t n, std::size_t q>
static void FillData(std::array<value_t, n>& values, std::array<value_t, q / 2>& update_values,
                     std::array<std::uint32_t, q>& l_int,
                     std::array<std::uint32_t, q>& r_int) noexcept {
    std::mt19937 rnd;
    for (value_t& a_i : values) {
        if constexpr (std::is_integral_v<value_t>) {
            a_i = value_t(int32_t(rnd())) % value_t(64);
        } else {
            a_i = value_t(int32_t(rnd()));
        }
    }
    for (value_t& a_i : update_values) {
        if constexpr (std::is_integral_v<value_t>) {
            a_i = value_t(int32_t(rnd())) % value_t(64);
        } else {
            a_i = value_t(int32_t(rnd()));
        }
    }
    for (size_t i = 0; i < q; i++) {
        const auto x = std::uint32_t(rnd() % n);
        const auto y = std::uint32_t(rnd() % n);
        l_int[i]     = std::min(x, y);
        r_int[i]     = std::max(x, y);
    }
}

template <UpdateOperation update_op, GetOperation get_op, typename value_t, bool AllowFuzzyEquality,
          std::size_t n, std::size_t q>
static inline void Test(const std::array<value_t, n>& values,
                        const std::array<value_t, q / 2>& update_values,
                        const std::array<std::uint32_t, q>& l_int,
                        const std::array<std::uint32_t, q>& r_int) {
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<update_op, get_op, value_t> tree(values);
    SegTreeChecker<update_op, get_op, value_t> checker(values);
    for (std::uint32_t i = 0; i < q; i += 2) {
        const auto l_update     = l_int[i];
        const auto r_update     = r_int[i];
        const value_t upd_value = update_values[i / 2];
        tree.update(l_update, r_update, upd_value);
        checker.update(l_update, r_update, upd_value);
        const auto l_get          = l_int[i + 1];
        const auto r_get          = r_int[i + 1];
        const value_t tree_ans    = tree.get(l_get, r_get);
        const value_t checker_ans = checker.get(l_get, r_get);

        if constexpr (AllowFuzzyEquality && std::is_floating_point_v<value_t>) {
            bool overflow_in_tree    = std::isnan(tree_ans) || std::isinf(tree_ans);
            bool overflow_in_checker = std::isnan(checker_ans) || std::isinf(checker_ans);
            assert(overflow_in_tree == overflow_in_checker);
            if (overflow_in_tree || tree_ans == checker_ans) {
                continue;
            }

            auto fuzzy_equal = [](value_t x, value_t y) noexcept {
                return std::abs(x - y) <= value_t(0.001L) * std::min(std::abs(x), std::abs(y));
            };
            assert(fuzzy_equal(tree_ans, checker_ans));
        } else {
            assert(tree_ans == checker_ans);
        }
    }
}

template <typename value_t = int64_t>
static void RunTestsForType() {
    constexpr uint32_t n = 65536;
    constexpr uint32_t q = 32768;
    static constinit std::array<value_t, n> values{};
    static constinit std::array<value_t, q / 2> update_values{};
    static constinit std::array<uint32_t, q> l_int{};
    static constinit std::array<uint32_t, q> r_int{};
    FillData(values, update_values, l_int, r_int);

    using enum UpdateOperation;
    using enum GetOperation;

    Test<add, min, value_t, false>(values, update_values, l_int, r_int);
    Test<set_equal, min, value_t, false>(values, update_values, l_int, r_int);
    Test<add, max, value_t, false>(values, update_values, l_int, r_int);
    Test<set_equal, max, value_t, false>(values, update_values, l_int, r_int);
    Test<set_equal, max, value_t, false>(values, update_values, l_int, r_int);
    Test<multiply, sum, value_t, true>(values, update_values, l_int, r_int);
    Test<add, sum, value_t, false>(values, update_values, l_int, r_int);
    Test<set_equal, sum, value_t, false>(values, update_values, l_int, r_int);
    Test<set_equal, product, value_t, true>(values, update_values, l_int, r_int);
    Test<multiply, product, value_t, true>(values, update_values, l_int, r_int);
    if constexpr (!std::is_integral_v<value_t>) {
        Test<multiply, min, value_t, true>(values, update_values, l_int, r_int);
        Test<multiply, max, value_t, true>(values, update_values, l_int, r_int);
    }
}

static void RunTests() {
    RunTestsForType<std::int32_t>();
    RunTestsForType<std::uint32_t>();
    RunTestsForType<std::int64_t>();
    RunTestsForType<std::uint64_t>();
    RunTestsForType<double>();
    RunTestsForType<long double>();
}

int main() {
    RunTests();
}
