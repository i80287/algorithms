#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <utility>

import rbtree;

namespace {

using rbtree::RBTree;

template <std::integral T, std::size_t N, std::size_t M>
constexpr bool non_intersecting_sets(const std::array<T, N> &s1, const std::array<T, M> &s2) noexcept {
    std::array<T, N> s1_arr(s1);
    std::array<T, M> s2_arr(s2);
    std::ranges::sort(s1_arr);
    std::ranges::sort(s2_arr);

    class Iter {
    public:
        using difference_type [[maybe_unused]] = std::ptrdiff_t;
        using value_type = T;
        using reference = value_type &;
        using pointer = value_type *;

        constexpr pointer operator->() const noexcept {
            was_de_referenced_ = true;
            return std::addressof(dummy_field_);
        }
        constexpr reference operator*() const noexcept {
            was_de_referenced_ = true;
            return dummy_field_;
        }
        constexpr Iter &operator++() noexcept {
            return *this;
        }
        constexpr Iter &operator--() noexcept {
            return *this;
        }
        constexpr Iter operator++(int) & noexcept {
            const Iter copy(*this);
            operator++();
            return copy;
        }
        constexpr Iter operator--(int) & noexcept {
            const Iter copy(*this);
            operator++();
            return copy;
        }
        constexpr bool operator==(const Iter &) const noexcept = default;
        [[nodiscard]] constexpr bool WasDeReferenced() const noexcept {
            return was_de_referenced_;
        }

    private:
        mutable int dummy_field_{};
        mutable bool was_de_referenced_ = false;
    };

    static_assert(std::bidirectional_iterator<Iter>);

    Iter observer;
    std::ranges::set_intersection(s1, s2, observer);
    return !observer.WasDeReferenced();
}

template <std::ranges::bidirectional_range Range1, std::ranges::bidirectional_range Range2>
    requires(std::is_same_v<std::ranges::range_value_t<Range1>, std::ranges::range_value_t<Range2>>)
void test_on_range(const Range1 &nums, const Range2 &not_in_nums) {
    using T = std::ranges::range_value_t<Range1>;

    static_assert(std::bidirectional_iterator<typename RBTree<T>::iterator>);
    static_assert(std::ranges::bidirectional_range<RBTree<T>>);
    static_assert(!std::is_polymorphic_v<RBTree<T>>);
    static_assert(std::is_nothrow_default_constructible_v<RBTree<T>>);
    static_assert(std::is_copy_constructible_v<RBTree<T>>);
    static_assert(std::is_copy_assignable_v<RBTree<T>>);
    static_assert(!std::is_nothrow_copy_constructible_v<RBTree<T>>);
    static_assert(!std::is_nothrow_copy_assignable_v<RBTree<T>>);
    static_assert(std::is_nothrow_move_constructible_v<RBTree<T>>);
    static_assert(std::is_nothrow_move_assignable_v<RBTree<T>>);
    static_assert(std::is_swappable_v<RBTree<T>>);
    static_assert(std::is_nothrow_swappable_v<RBTree<T>>);
    static_assert(std::is_standard_layout_v<RBTree<T>>);

    auto compare = [&nums, &not_in_nums](const RBTree<T> &t, const std::set<T> &checker) {
        assert(t.size() == checker.size());
        assert(t.size() <= t.max_size());
        assert(t.size() <= RBTree<T>::max_size());

        assert(std::ranges::equal(t, checker));
        assert(std::equal(t.begin(), t.end(), checker.begin()));
        assert(std::equal(t.cbegin(), t.cend(), checker.cbegin()));
        assert(std::equal(t.rbegin(), t.rend(), checker.rbegin()));
        assert(std::equal(t.crbegin(), t.crend(), checker.crbegin()));

        if (!checker.empty()) {
            assert(*t.begin() == *checker.begin());
            assert(*--t.end() == *--checker.end());
            assert(*t.cbegin() == *checker.cbegin());
            assert(*--t.cend() == *--checker.cend());
            assert(t.front() == *checker.cbegin());
            assert(t.back() == *--checker.cend());
            assert(*std::as_const(t).begin() == *checker.begin());
            assert(*--std::as_const(t).end() == *--checker.end());
            assert(*std::as_const(t).cbegin() == *checker.cbegin());
            assert(*--std::as_const(t).cend() == *--checker.cend());
            assert(std::as_const(t).front() == *checker.cbegin());
            assert(std::as_const(t).back() == *--checker.cend());
        } else {
            assert(t.empty());
        }

        auto check = [&t, &checker](const auto &sub_range) {
            for (const T &lb_num : sub_range) {
                {
                    const auto p = t.lower_bound(lb_num);
                    const auto correct_ans_iter = checker.lower_bound(lb_num);
                    assert(t.size() == checker.size());
                    if (correct_ans_iter == checker.end()) {
                        assert(p == t.end());
                        assert(!t.contains(lb_num));
                    } else {
                        assert(p != t.end());
                        assert(*p == *correct_ans_iter);
                    }
                }
                {
                    const auto p = t.find(lb_num);
                    const auto correct_ans_iter = checker.find(lb_num);
                    assert(t.size() == checker.size());
                    if (correct_ans_iter == checker.end()) {
                        assert(p == t.end());
                        assert(!t.contains(lb_num));
                    } else {
                        assert(p != t.end());
                        assert(*p == *correct_ans_iter);
                        assert(t.contains(lb_num));
                    }
                }
            }
        };

        check(nums);
        check(not_in_nums);
        check(std::ranges::views::reverse(nums));
        check(std::ranges::views::reverse(not_in_nums));
    };

    auto test_tree = [&compare](const RBTree<T> &t, const std::set<T> &checker) {
        compare(t, checker);
        RBTree<T> t1 = t;
        compare(t1, checker);
        static_assert(std::is_same_v<decltype(std::move(t1)), RBTree<T> &&>);
        RBTree<T> t2 = std::move(t1);
        compare(t2, checker);
        RBTree<T> &t1_ref = t1 = std::move(t2);
        assert(std::addressof(t1_ref) == std::addressof(t1));
        compare(t1, checker);
        t2.clear();
        t2.swap(t1);
        compare(t2, checker);
        t2.swap(t1);
        compare(t1, checker);
        RBTree<T> &t2_ref = t2 = t1;
        assert(std::addressof(t2_ref) == std::addressof(t2));
        compare(t2, checker);
    };

    RBTree<T> t;
    std::set<T> checker;
    assert(t.empty());
    assert(t.size() == 0);
    assert(t.find({}) == t.end());
    assert(t.lower_bound({}) == t.end());
    for (const T &num : nums) {
        assert(rbtree::RBTreeInvariantsUnitTest(t) == rbtree::TestStatus::kOk);
        t.insert(num);
        assert(t.size() <= t.max_size());
        assert(rbtree::RBTreeInvariantsUnitTest(t) == rbtree::TestStatus::kOk);
        checker.insert(num);
        test_tree(t, checker);
    }

    for (const T &elem : nums) {
        assert(rbtree::RBTreeInvariantsUnitTest(t) == rbtree::TestStatus::kOk);
        assert(t.erase(elem) == checker.erase(elem));
        assert(t.size() <= t.max_size());
        assert(rbtree::RBTreeInvariantsUnitTest(t) == rbtree::TestStatus::kOk);
        test_tree(t, checker);
    }
}

template <std::ranges::random_access_range Range1, std::ranges::random_access_range Range2>
void test_on_sub_ranges(const Range1 &range, const Range2 &not_in_range) {
    for (std::size_t drop_size = 0; drop_size < std::size(range); drop_size++) {
        for (std::size_t take_size = 0; take_size <= std::size(range) - drop_size; take_size++) {
            test_on_range(range | std::views::drop(drop_size) | std::views::take(take_size), not_in_range);
        }
    }
}

void test_rbtree_on_ranges() {
    constexpr std::array nums = {
        1,          2,         -3,      4,       0,         -4,     35,      -45,      20,
        23,         22,        21,      -15,     -28,       56,     57,      44,       69,
        72,         101,       118,     114,     -114,      -118,   -101,    13,       -13,
        12,         -12,       32,      23,      12,        54,     34,      5645,     2,
        34,         234,       23,      4234,    4234,      34,     3253,    6546,     567,
        5,          736,       462476,  4574327, 245762456, 623456, 4256,    52623456, 2454,
        1264367436, 743256342, 4673345, 34256,   674324,    47643,  2347824, 2178,     12387,
        -12387,     8123,      67284,   -2348,   12738,     93284,  -1238,   238743,   -1'000'000'000,
        5,          736,       462476,  462475,  462474,    462473, 462472,  462471,   12,
        13,         14,        1515,    161616,  17171717,  0,      0,
    };
    constexpr std::array not_in_nums = {
        -100, -50, -10, 10, 100, 200, 300, 400, 500, 1000, 20000, 4023087, 2'091'371'239, 2'111'222'333,
    };

    static_assert(non_intersecting_sets(nums, not_in_nums));

    test_on_sub_ranges(nums, not_in_nums);

    using namespace std::string_view_literals;

    constexpr std::array string_views = {
        "asd"sv,           "3284"sv,          "f7823h7yf3"sv,   "23f87g2quf"sv,      "w2uv9f3w"sv,      "v23fvn4ev"sv,
        "vf324v3hv34v"sv,  "23bvuywvb"sv,     "whbuwbhjv"sv,    "f2q3gfyu2bv"sv,     "cqw3gbhbve"sv,    "q3wnj"sv,
        "dawbcnac"sv,      "acdjawbcawc"sv,   "awjcbacn"sv,     "awjcbanc"sv,        "awkcjakcsn"sv,    "whfjancaw"sv,
        "cq39fc98hcnac"sv, "acdnbnzxm"sv,     "dawjcna"sv,      "cawbcawmcnvehvb"sv, "vjabevjhnbnsc"sv, "cawjcjawc"sv,
        "asd"sv,           "3284"sv,          "f7823h7yf3"sv,   "23f87g2quf"sv,      "w2uv9f3w"sv,      "v23fvn4ev"sv,
        "vf324v3hv34v"sv,  "23bvuywvb"sv,     "whbuwbhjv"sv,    "f2q3gfyu2bv"sv,     "cqw3gbhbve"sv,    "q3wnj"sv,
        "dawbcnac"sv,      "acdjawbcawc"sv,   "awjcbacn"sv,     "awjcbanc"sv,        "awkcjakcsn"sv,    "whfjancaw"sv,
        "cq39fc98hcnac"sv, "acdnbnzxm"sv,     "dawjcna"sv,      "cawbcawmcnvehvb"sv, "vjabevjhnbnsc"sv, "cawjcjawc"sv,
        "28378234231"sv,   "4928342348"sv,    "234823478234"sv, "53745834543"sv,     "234893248234"sv,  "324823748"sv,
        "4358983459345"sv, "9345834583458"sv,
    };
    constexpr std::array not_in_string_views = {
        "cjweh"sv,         "dajw"sv,         "awcsn"sv,     "23nmfce"sv,    "cajwbncvuie"sv,
        "awbcnwn"sv,       "vcabndicanjs"sv, "cawbcncaw"sv, "cawbcnawc"sv,  "cabwbcnawcn"sv,
        "4398347583458"sv, "345832478324"sv, "428347234"sv, "3492348234"sv, "234u3284234"sv,
    };
    test_on_sub_ranges(string_views, not_in_string_views);

    std::array<std::string, std::size(string_views)> strings{};
    std::ranges::copy(string_views, strings.begin());
    std::array<std::string, std::size(string_views)> not_in_strings{};
    std::ranges::copy(not_in_string_views, not_in_strings.begin());
    test_on_sub_ranges(strings, not_in_strings);
}

template <class TypeWithDefaultCtorAndWithoutOperatorLess>
void test_with_comparator_impl1() {
    using T = TypeWithDefaultCtorAndWithoutOperatorLess;
    static_assert(requires { T{}; });
    static_assert(std::is_default_constructible_v<TypeWithDefaultCtorAndWithoutOperatorLess>);
    static_assert(!requires { RBTree<T>{{T{}, T{}, T{}}}; });
}

void test_with_comparator() {
    struct S1 {
    private:
        [[maybe_unused]] int x_{};
    };
    test_with_comparator_impl1<S1>();

    class T {
    public:
        T(const uint32_t x, const uint32_t y) : x_{x}, y_{y} {}

        [[nodiscard]] uint32_t x() const {
            return x_;
        }
        [[nodiscard]] uint32_t y() const {
            return y_;
        }

    private:
        uint32_t x_;
        uint32_t y_;
    };
    class Cmp {
    public:
        bool operator()(const T &lhs, const T &rhs) const {
            return lhs.x() + lhs.y() + u < rhs.x() + rhs.y() + u;
        }

    private:
        uint8_t u{10};
    };
    class NonCopyableCmp : public Cmp {
    public:
        NonCopyableCmp() = default;
        NonCopyableCmp(const NonCopyableCmp &) = delete;
        NonCopyableCmp &operator=(const NonCopyableCmp &) = delete;
        NonCopyableCmp(NonCopyableCmp &&) = default;
        [[maybe_unused]] NonCopyableCmp &operator=(NonCopyableCmp &&) = default;
    };

    using Set = RBTree<T, NonCopyableCmp>;
    using StdSet = std::set<T, Cmp>;
    auto cmp_sets = [](const Set &lhs, const StdSet &rhs) {
        assert(lhs.size() == rhs.size());
        for (auto rhs_iter = rhs.begin(); const T &lhs_t : lhs) {
            assert(lhs_t.x() == rhs_iter->x());
            assert(lhs_t.y() == rhs_iter->y());
            assert(rhs_iter != rhs.end());
            ++rhs_iter;
        }
    };
    Set t1{NonCopyableCmp{}};
    auto cmp = Cmp{};
    StdSet std_t{cmp};
    std::array r1{T{1, 1}, T{2, 123}, T{4382, 32489}, T{2, 3}, T{23, 32738}, T{32873, 2339}};
    std_t.insert(r1.begin(), r1.end());
    t1.insert_range(std::move(r1));
    cmp_sets(t1, std_t);

    Set t = std::move(t1);

    std::array r2{T{34, 3289}, T{48, 438}, T{3492, 328}, T{328, 328}, T{432873, 43289}};
    std_t.insert(r2.begin(), r2.end());
    t.insert_range(std::move(r2));
    cmp_sets(t, std_t);

    while (!std_t.empty()) {
        assert(!t.empty());
        assert(t.begin() != t.end());
        std_t.erase(std_t.begin());
        t.erase(t.begin());
        cmp_sets(t, std_t);
    }

    assert(t.empty());
    assert(t.begin() == t.end());
    assert(t.size() == 0);
}

}  // namespace

int main() {
    test_rbtree_on_ranges();
    test_with_comparator();
}
