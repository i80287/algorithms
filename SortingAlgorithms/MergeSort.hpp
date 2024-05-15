#pragma once

#include <algorithm>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>

namespace detail {
namespace merge_sort_impl {
namespace {

template <class Iterator, class Comparator>
inline constexpr bool kIsNothrowComparable =
    noexcept(std::declval<Comparator>()(*std::declval<Iterator>(),
                                        *std::declval<Iterator>()));

template <class Iterator, class Comparator>
inline constexpr bool kIsNoexceptSortable =
    noexcept(std::is_nothrow_move_assignable_v<
             typename std::iterator_traits<Iterator>::value_type>) &&
    kIsNothrowComparable<Iterator, Comparator>;

// clang-format off
template <class Iterator, class Comparator>
constexpr void MergeImpl(Iterator iter_left, Iterator iter_left_2, Iterator iter_right, Comparator comp)
    noexcept(kIsNoexceptSortable<Iterator, Comparator>) {
    // clang-format on
    while (iter_left < iter_left_2 && iter_left_2 < iter_right) {
        if (comp(*iter_left_2, *iter_left)) {
            auto value = std::move(*iter_left_2);
            std::move_backward(iter_left, iter_left_2, iter_left_2 + 1);
            *iter_left = std::move(value);
            ++iter_left_2;
        }
        ++iter_left;
    }
}

// clang-format off
template <class Iterator, class Comparator>
constexpr void MergeSortImpl(Iterator iter_left, Iterator iter_right, Comparator comp)
    noexcept(kIsNoexceptSortable<Iterator, Comparator>) {
    // clang-format on
    Iterator iter_left_2 =
        iter_left + std::distance(iter_left, iter_right) / 2;
    if (std::distance(iter_left, iter_left_2) >= 2) {
        MergeSortImpl(iter_left, iter_left_2, comp);
    }

    if (std::distance(iter_left_2, iter_right) >= 2) {
        MergeSortImpl(iter_left_2, iter_right, comp);
    }
    MergeImpl(iter_left, iter_left_2, iter_right, std::move(comp));
}

// clang-format off
template <class Iterator, class Comparator>
constexpr void MergeSortWithEmptyComparatorImpl(Iterator iter_left, Iterator iter_right)
    noexcept(kIsNoexceptSortable<Iterator, Comparator>) {
    // clang-format on
    auto dist            = std::distance(iter_left, iter_right);
    Iterator iter_left_2 = iter_left + dist / 2;
    if (std::distance(iter_left, iter_left_2) >= 2) {
        MergeSortWithEmptyComparatorImpl<Iterator, Comparator>(
            iter_left, iter_left_2);
    }

    if (std::distance(iter_left_2, iter_right) >= 2) {
        MergeSortWithEmptyComparatorImpl<Iterator, Comparator>(iter_left_2,
                                                               iter_right);
    }
    MergeImpl(iter_left, iter_left_2, iter_right, Comparator{});
}

}  // namespace
}  // namespace merge_sort_impl
}  // namespace detail

template <class Iterator, class Comparator = std::ranges::less>
constexpr void
MergeSort(Iterator begin, Iterator end, Comparator comp = {}) noexcept(
    detail::merge_sort_impl::kIsNoexceptSortable<Iterator, Comparator>) {
    if (std::distance(begin, end) >= 2) {
        if constexpr (sizeof(Comparator) == 1 &&
                      std::is_nothrow_constructible_v<Comparator>) {
            detail::merge_sort_impl::MergeSortWithEmptyComparatorImpl<
                Iterator, Comparator>(begin, end);
        } else {
            detail::merge_sort_impl::MergeSortImpl(begin, end,
                                                   std::move(comp));
        }
    }
}

template <std::ranges::random_access_range Range,
          class Comparator = std::ranges::less>
constexpr void MergeSort(Range&& range, Comparator comp = {}) noexcept(
    noexcept(MergeSort(std::ranges::begin(range), std::ranges::end(range),
                       std::move(comp)))) {
    MergeSort(std::ranges::begin(range), std::ranges::end(range),
              std::move(comp));
}
