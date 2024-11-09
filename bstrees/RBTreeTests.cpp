#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <ranges>
#include <set>
#include <utility>

import rbtree;

template <std::size_t N, std::size_t M>
constexpr bool non_intersecting_sets(const int (&s1)[N], const int (&s2)[M]) noexcept {
    auto s1_arr = std::to_array(s1);
    auto s2_arr = std::to_array(s2);
    std::ranges::sort(s1_arr);
    std::ranges::sort(s2_arr);

    class Iter {
    public:
        using difference_type [[maybe_unused]] = std::ptrdiff_t;
        using value_type                       = int;
        using reference                        = value_type &;
        using pointer                          = value_type *;

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
            Iter copy(*this);
            operator++();
            return copy;
        }
        constexpr Iter operator--(int) & noexcept {
            Iter copy(*this);
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

template <std::ranges::forward_range Range1, std::ranges::forward_range Range2>
    requires(std::is_same_v<std::ranges::range_value_t<Range1>, std::ranges::range_value_t<Range2>>)
void test_on_range(const Range1 &nums, const Range2 &not_in_nums) {
    using T = std::ranges::range_value_t<Range1>;
    static_assert(std::bidirectional_iterator<typename RBTree<T>::iterator>);
    static_assert(std::ranges::bidirectional_range<RBTree<T>>);
    static_assert(!std::is_polymorphic_v<RBTree<T>>);
    static_assert(std::is_nothrow_move_constructible_v<RBTree<T>>);
    static_assert(std::is_nothrow_move_assignable_v<RBTree<T>>);
    static_assert(std::is_swappable_v<RBTree<T>>);
    static_assert(std::is_nothrow_swappable_v<RBTree<T>>);
    static_assert(std::is_nothrow_default_constructible_v<RBTree<T>>);
    static_assert(std::is_standard_layout_v<RBTree<T>>);

    auto compare = [&nums, &not_in_nums](const RBTree<T> &t, const std::set<T> &checker) {
        assert(t.size() == checker.size());

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
                    auto p                = t.lower_bound(lb_num);
                    auto correct_ans_iter = checker.lower_bound(lb_num);
                    assert(t.size() == checker.size());
                    if (correct_ans_iter == checker.end()) {
                        assert(p == t.end());
                    } else {
                        assert(p != t.end());
                        assert(*p == *correct_ans_iter);
                    }
                }
                {
                    auto p                = t.find(lb_num);
                    auto correct_ans_iter = checker.find(lb_num);
                    assert(t.size() == checker.size());
                    if (correct_ans_iter == checker.end()) {
                        assert(p == t.end());
                    } else {
                        assert(p != t.end());
                        assert(*p == *correct_ans_iter);
                    }
                }
            }
        };

        check(nums);
        check(not_in_nums);
        check(std::ranges::views::reverse(nums));
        check(std::ranges::views::reverse(not_in_nums));
    };

    RBTree<T> t;
    std::set<T> checker;
    assert(t.empty());
    assert(t.size() == 0);
    assert(t.find({}) == t.end());
    assert(t.lower_bound({}) == t.end());
    for (const T &num : nums) {
        assert(t.is_rbtree());
        t.insert(num);
        assert(t.size() <= t.max_size());
        assert(t.is_rbtree());
        checker.insert(num);

        compare(t, checker);

        static_assert(std::is_same_v<decltype(std::move(t)), RBTree<T> &&>);
        RBTree<T> t1(std::move(t));
        compare(t1, checker);
        t = std::move(t1);
        compare(t, checker);
        t1.swap(t);
        compare(t1, checker);
        t.swap(t1);
        compare(t, checker);
        t1 = t;
        compare(t1, checker);
    }

    for (const T &elem : nums) {
        assert(t.is_rbtree());
        assert(t.erase(elem) == checker.erase(elem));
        assert(t.size() <= t.max_size());
        assert(t.is_rbtree());

        compare(t, checker);

        RBTree<T> t1(std::move(t));
        compare(t1, checker);
        t = std::move(t1);
        compare(t, checker);
        t1.swap(t);
        compare(t1, checker);
        t.swap(t1);
        compare(t, checker);
        t1 = t;
        compare(t1, checker);
    }
}

template <std::ranges::forward_range Range1, std::ranges::forward_range Range2>
static void test_on_sub_ranges(const Range1 &range, const Range2 &not_in_range) {
    for (std::size_t drop_size = 0; drop_size < std::size(range); drop_size++) {
        for (std::size_t take_size = 0; take_size <= std::size(range) - drop_size; take_size++) {
            test_on_range(range | std::views::drop(drop_size) | std::views::take(take_size),
                          not_in_range);
        }
    }
}

int main() {
    constexpr int nums[] = {
        1,       2,         -3,     4,      0,        -4,       35,         -45,
        20,      23,        22,     21,     -15,      -28,      56,         57,
        44,      69,        72,     101,    118,      114,      -114,       -118,
        -101,    13,        -13,    12,     -12,      32,       23,         12,
        54,      34,        5645,   2,      34,       234,      23,         4234,
        4234,    34,        3253,   6546,   567,      5,        736,        462476,
        4574327, 245762456, 623456, 4256,   52623456, 2454,     1264367436, 743256342,
        4673345, 34256,     674324, 47643,  2347824,  2178,     12387,      -12387,
        8123,    67284,     -2348,  12738,  93284,    -1238,    238743,     -1'000'000'000,
        5,       736,       462476, 462475, 462474,   462473,   462472,     462471,
        12,      13,        14,     1515,   161616,   17171717, 0,          0,
    };
    constexpr int not_in_nums[] = {
        -100, -50, -10,  10,    100,     200,           300,
        400,  500, 1000, 20000, 4023087, 2'091'371'239, 2'111'222'333,
    };

    static_assert(non_intersecting_sets(nums, not_in_nums));

    test_on_sub_ranges(nums, not_in_nums);

    constexpr std::string_view string_views[] = {
        "asd",           "3284",
        "f7823h7yf3",    "23f87g2quf",
        "w2uv9f3w",      "v23fvn4ev",
        "vf324v3hv34v",  "23bvuywvb",
        "whbuwbhjv",     "f2q3gfyu2bv",
        "cqw3gbhbve",    "q3wnj",
        "dawbcnac",      "acdjawbcawc",
        "awjcbacn",      "awjcbanc",
        "awkcjakcsn",    "whfjancaw",
        "cq39fc98hcnac", "acdnbnzxm",
        "dawjcna",       "cawbcawmcnvehvb",
        "vjabevjhnbnsc", "cawjcjawc",
        "asd",           "3284",
        "f7823h7yf3",    "23f87g2quf",
        "w2uv9f3w",      "v23fvn4ev",
        "vf324v3hv34v",  "23bvuywvb",
        "whbuwbhjv",     "f2q3gfyu2bv",
        "cqw3gbhbve",    "q3wnj",
        "dawbcnac",      "acdjawbcawc",
        "awjcbacn",      "awjcbanc",
        "awkcjakcsn",    "whfjancaw",
        "cq39fc98hcnac", "acdnbnzxm",
        "dawjcna",       "cawbcawmcnvehvb",
        "vjabevjhnbnsc", "cawjcjawc",
        "28378234231",   "4928342348",
        "234823478234",  "53745834543",
        "234893248234",  "324823748",
        "4358983459345", "9345834583458",
    };
    constexpr std::string_view not_in_string_views[] = {
        "cjweh",         "dajw",         "awcsn",     "23nmfce",    "cajwbncvuie",
        "awbcnwn",       "vcabndicanjs", "cawbcncaw", "cawbcnawc",  "cabwbcnawcn",
        "4398347583458", "345832478324", "428347234", "3492348234", "234u3284234",
    };
    test_on_sub_ranges(string_views, not_in_string_views);

    std::array<std::string, std::size(string_views)> strings{};
    std::ranges::copy(string_views, strings.begin());
    std::array<std::string, std::size(string_views)> not_in_strings{};
    std::ranges::copy(not_in_string_views, not_in_strings.begin());
    test_on_sub_ranges(strings, not_in_strings);
}
