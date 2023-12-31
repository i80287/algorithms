#include "segment_trees.cpp"

#include <random>

template <uint32_t n, uint32_t q, typename value_t>
[[maybe_unused]] static inline void fillData(
    std::vector<value_t>& values, std::vector<value_t>& update_values,
    std::vector<uint32_t>& l_int, std::vector<uint32_t>& r_int) {
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
        uint32_t l = uint32_t(rnd() % n);
        uint32_t r = uint32_t(rnd() % n);
        if (l > r) {
            std::swap(l, r);
        }
        l_int[i] = l;
        r_int[i] = r;
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testMinAdd(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::add, GetOperation::min, value_t> tree(values);
    segtrees::SegTreeChecker<UpdateOperation::add, GetOperation::min, value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        assert(tree_ans == checker_ans);
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testMinSetEqual(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::set_equal, GetOperation::min, value_t> tree(
        values);
    segtrees::SegTreeChecker<UpdateOperation::set_equal, GetOperation::min,
                             value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        assert(tree_ans == checker_ans);
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testMaxAdd(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::add, GetOperation::max, value_t> tree(values);
    segtrees::SegTreeChecker<UpdateOperation::add, GetOperation::max, value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        assert(tree_ans == checker_ans);
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testMaxSetEqual(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::set_equal, GetOperation::max, value_t> tree(
        values);
    segtrees::SegTreeChecker<UpdateOperation::set_equal, GetOperation::max,
                             value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        assert(tree_ans == checker_ans);
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testSumMultiply(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::multiply, GetOperation::sum, value_t> tree(values);
    segtrees::SegTreeChecker<UpdateOperation::multiply, GetOperation::sum,
                             value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        if constexpr (std::is_floating_point_v<value_t>) {
            bool overflow = std::isnan(tree_ans) || std::isinf(tree_ans);
            assert(overflow ==
                   (std::isnan(checker_ans) || std::isinf(checker_ans)));
            if (overflow || tree_ans == checker_ans) {
                continue;
            }
            auto fraq = tree_ans / checker_ans;
            assert(value_t(1.0L - 0.001L) <= fraq &&
                   fraq <= value_t(1.0L + 0.001L));
        } else {
            assert(tree_ans == checker_ans);
        }
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testSumAdd(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::add, GetOperation::sum, value_t> tree(values);
    segtrees::SegTreeChecker<UpdateOperation::add, GetOperation::sum, value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        assert(tree_ans == checker_ans);
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testSumSetEqual(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::set_equal, GetOperation::sum, value_t> tree(
        values);
    segtrees::SegTreeChecker<UpdateOperation::set_equal, GetOperation::sum,
                             value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        assert(tree_ans == checker_ans);
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testProductSetEqual(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::set_equal, GetOperation::product, value_t> tree(
        values);
    segtrees::SegTreeChecker<UpdateOperation::set_equal, GetOperation::product,
                             value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        if constexpr (std::is_floating_point_v<value_t>) {
            bool overflow = std::isnan(tree_ans) || std::isinf(tree_ans);
            assert(overflow ==
                   (std::isnan(checker_ans) || std::isinf(checker_ans)));
            if (overflow || tree_ans == checker_ans) {
                continue;
            }
            auto fraq = tree_ans / checker_ans;
            assert(value_t(1.0L - 0.001L) <= fraq &&
                   fraq <= value_t(1.0L + 0.001L));
        } else {
            assert(tree_ans == checker_ans);
        }
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testProductMultiply(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::multiply, GetOperation::product, value_t> tree(
        values);
    segtrees::SegTreeChecker<UpdateOperation::multiply, GetOperation::product,
                             value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        if constexpr (std::is_floating_point_v<value_t>) {
            bool overflow = std::isnan(tree_ans) || std::isinf(tree_ans);
            assert(overflow ==
                   (std::isnan(checker_ans) || std::isinf(checker_ans)));
            if (overflow || tree_ans == checker_ans) {
                continue;
            }
            auto fraq = tree_ans / checker_ans;
            assert(value_t(1.0L - 0.001L) <= fraq &&
                   fraq <= value_t(1.0L + 0.001L));
        } else {
            assert(tree_ans == checker_ans);
        }
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testMinMultiply(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    if (std::is_integral_v<value_t>) {
        return;
    }

    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::multiply, GetOperation::min, value_t> tree(values);
    segtrees::SegTreeChecker<UpdateOperation::multiply, GetOperation::min,
                             value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        if constexpr (std::is_floating_point_v<value_t>) {
            bool overflow = std::isnan(tree_ans) || std::isinf(tree_ans);
            assert(overflow ==
                   (std::isnan(checker_ans) || std::isinf(checker_ans)));
            if (overflow || tree_ans == checker_ans) {
                continue;
            }
            auto fraq = tree_ans / checker_ans;
            assert(value_t(1.0L - 0.001L) <= fraq &&
                   fraq <= value_t(1.0L + 0.001L));
        } else {
            assert(tree_ans == checker_ans);
        }
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testMaxMultiply(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    if (std::is_integral_v<value_t>) {
        return;
    }

    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::multiply, GetOperation::max, value_t> tree(values);
    segtrees::SegTreeChecker<UpdateOperation::multiply, GetOperation::max,
                             value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        if constexpr (std::is_floating_point_v<value_t>) {
            bool overflow = std::isnan(tree_ans) || std::isinf(tree_ans);
            assert(overflow ==
                   (std::isnan(checker_ans) || std::isinf(checker_ans)));
            if (overflow || tree_ans == checker_ans) {
                continue;
            }
            auto fraq = tree_ans / checker_ans;
            assert(value_t(1.0L - 0.001L) <= fraq &&
                   fraq <= value_t(1.0L + 0.001L));
        } else {
            assert(tree_ans == checker_ans);
        }
    }
}

template <typename value_t = int64_t>
void tests() {
    constexpr uint32_t n = 65536;
    constexpr uint32_t q = 32768;
    std::vector<value_t> values(n);
    std::vector<value_t> update_values(q / 2);
    std::vector<uint32_t> l_int(q);
    std::vector<uint32_t> r_int(q);
    fillData<n, q>(values, update_values, l_int, r_int);
    testMinAdd(values, update_values, l_int, r_int);
    testMinSetEqual(values, update_values, l_int, r_int);
    testMaxAdd(values, update_values, l_int, r_int);
    testMaxSetEqual(values, update_values, l_int, r_int);
    testSumMultiply(values, update_values, l_int, r_int);
    testSumAdd(values, update_values, l_int, r_int);
    testSumSetEqual(values, update_values, l_int, r_int);
    testProductSetEqual(values, update_values, l_int, r_int);
    testProductMultiply(values, update_values, l_int, r_int);
    testMinMultiply(values, update_values, l_int, r_int);
    testMaxMultiply(values, update_values, l_int, r_int);
}

int main() {
    tests();
    tests<double>();
}
