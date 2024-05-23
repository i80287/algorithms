#pragma once

#include <cstdint>
#include <functional>
#include <iterator>
#include <random>
#include <ranges>

#include "sorting_algos_config.hpp"

namespace algorithms {
namespace detail {
namespace quick_sort {
namespace {

template <class Iterator, class Comparator>
inline constexpr bool kIsNothrowComparable =
    noexcept(std::declval<Comparator>()(*std::declval<Iterator>(), *std::declval<Iterator>()));

template <class Iterator, class Comparator>
inline constexpr bool kIsNoexceptSortable =
    noexcept(
        std::is_nothrow_move_assignable_v<typename std::iterator_traits<Iterator>::value_type>) &&
    kIsNothrowComparable<Iterator, Comparator> &&
    std::is_nothrow_swappable_v<typename std::iterator_traits<Iterator>::value_type>;

// clang-format off
template <class Iterator, class Comparator>
constexpr Iterator partition(Iterator begin, Iterator end, Iterator pivot_it, Comparator comp)
    noexcept(kIsNoexceptSortable<Iterator, Comparator>) {
    // clang-format on

    // For ADL (more efficient swap may be provided for the given value type)
    using std::swap;

    Iterator l_it = begin;
    while (l_it != end && !comp(*l_it, *pivot_it)) {
        ++l_it;
    }
    if (l_it == end) [[unlikely]] {
        if (pivot_it != begin) {
            swap(*begin, *pivot_it);
        }
        return begin;
    }

    if (begin == pivot_it) {
        pivot_it = l_it;
    } else if (l_it == pivot_it) {
        pivot_it = begin;
    }
    swap(*begin, *l_it);
    l_it = begin;

    Iterator r_it = begin;
    do {
        ++r_it;
    } while (r_it != end && comp(*r_it, *pivot_it));
    Iterator last = end;
    --last;
    if (r_it == end) {
        assert(begin != end);
        swap(*pivot_it, *last);
        return last;
    }
    if (last == pivot_it) {
        pivot_it = r_it;
    } else if (r_it == pivot_it) {
        pivot_it = last;
    }
    swap(*last, *r_it);
    r_it = last;
    while (std::distance(l_it, r_it) >= 2) {
        Iterator next_l_it = l_it;
        ++next_l_it;
        while (!comp(*next_l_it, *pivot_it)) {
            Iterator next_r_it = r_it;
            --next_r_it;
            r_it = next_r_it;
            if (next_l_it == next_r_it) {
                next_l_it = l_it;
                break;
            }
            if (next_r_it == pivot_it) {
                pivot_it = next_l_it;
            } else if (next_l_it == pivot_it) {
                pivot_it = next_r_it;
            }
            swap(*next_l_it, *next_r_it);
        }
        l_it = next_l_it;
    }

    Iterator new_pivot_place_it = l_it;
    ++new_pivot_place_it;
    swap(*new_pivot_place_it, *pivot_it);
    return new_pivot_place_it;
}

// clang-format off
template <class Iterator, class Comparator>
constexpr Iterator select_pivot(Iterator begin, Iterator end, Comparator comp)
    noexcept(kIsNoexceptSortable<Iterator, Comparator>) {
    // clang-format on

    // length should be >= 2 by the invariant of quick_sort_impl.
    // Thus we can derefer (begin + length - 1) safely.
    const auto length = static_cast<std::size_t>(std::distance(begin, end));
#ifdef __GNUC__
    if (length < 2) {
        __builtin_unreachable();
    }
#endif
    const auto qrt_iter     = begin + static_cast<std::ptrdiff_t>(length / 4);
    const auto mid_iter     = begin + static_cast<std::ptrdiff_t>(length / 2);
    const auto thr_qrt_iter = begin + static_cast<std::ptrdiff_t>((3 * length) / 4);

    if (comp(*qrt_iter, *mid_iter)) {
        if (comp(*mid_iter, *thr_qrt_iter)) {
            return mid_iter;
        }
        if (comp(*qrt_iter, *thr_qrt_iter)) {
            return thr_qrt_iter;
        }
        return qrt_iter;
    }
    if (comp(*thr_qrt_iter, *mid_iter)) {
        return mid_iter;
    }
    if (comp(*thr_qrt_iter, *qrt_iter)) {
        return thr_qrt_iter;
    }

    return qrt_iter;
}

// clang-format off
template <class Iterator, class Comparator>
constexpr void quick_sort_impl(Iterator begin, Iterator end, Comparator comp)
    noexcept(kIsNoexceptSortable<Iterator, Comparator>) {
    // clang-format on

    // For ADL (more efficient swap may be provided for the given value type)
    using std::swap;

    const Iterator selected_pivot_iter = select_pivot(begin, end, comp);
    const Iterator new_pivot_iter      = partition(begin, end, selected_pivot_iter, comp);

    if (std::distance(begin, new_pivot_iter) > 1) {
        quick_sort_impl(begin, new_pivot_iter, comp);
    }

    Iterator first_not_equal = new_pivot_iter;
    assert(first_not_equal != end);
    do {
        ++first_not_equal;
    } while (first_not_equal != end && *first_not_equal == *new_pivot_iter);
    if (first_not_equal != end) {
        Iterator r = first_not_equal;
        for (++r; r != end; ++r) {
            if (*r == *new_pivot_iter) {
                swap(*first_not_equal, *r);
                ++first_not_equal;
            }
        }
    }

    if (std::distance(first_not_equal, end) > 1) {
        quick_sort_impl(first_not_equal, end, comp);
    }
}

}  // namespace
}  // namespace quick_sort
}  // namespace detail

struct QuickSortNiebloid final {
    template <std::bidirectional_iterator Iterator, class Comparator = std::ranges::less>
    static constexpr void quick_sort(Iterator begin, Iterator end, Comparator comp = {}) noexcept(
        detail::quick_sort::kIsNoexceptSortable<Iterator, Comparator>) {
        if (std::distance(begin, end) > 1) {
            detail::quick_sort::quick_sort_impl(begin, end, std::move(comp));
        }
    }

    template <std::ranges::bidirectional_range Range, class Comparator = std::ranges::less>
    static constexpr void quick_sort(Range&& range, Comparator comp = {}) noexcept(
        noexcept(quick_sort(std::ranges::begin(range), std::ranges::end(range), std::move(comp)))) {
        quick_sort(std::ranges::begin(range), std::ranges::end(range), std::move(comp));
    }

    template <std::bidirectional_iterator Iterator, class Comparator = std::ranges::less>
    constexpr void operator()(Iterator begin, Iterator end, Comparator comp = {}) const
        noexcept(noexcept(quick_sort(begin, end, std::move(comp)))) {
        quick_sort(begin, end, std::move(comp));
    }

    template <std::ranges::bidirectional_range Range, class Comparator = std::ranges::less>
    constexpr void operator()(Range&& range, Comparator comp = {}) const
        noexcept(noexcept(quick_sort(std::ranges::begin(range), std::ranges::end(range),
                                     std::move(comp)))) {
        quick_sort(std::ranges::begin(range), std::ranges::end(range), std::move(comp));
    }
};

inline constexpr QuickSortNiebloid quick_sort{};

}  // namespace algorithms
