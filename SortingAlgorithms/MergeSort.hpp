#pragma once

#include <algorithm>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

#include "sorting_algos_config.hpp"

namespace algorithms {
namespace detail {
namespace merge_sort {
namespace {

template <class Iterator, class Comparator>
inline constexpr bool kIsNothrowComparable =
    noexcept(std::declval<Comparator>()(*std::declval<Iterator>(), *std::declval<Iterator>()));

template <class Iterator, class Comparator>
inline constexpr bool kIsNoexceptMergable =
    noexcept(
        std::is_nothrow_move_assignable_v<typename std::iterator_traits<Iterator>::value_type>) &&
    noexcept(std::is_nothrow_move_constructible_v<
             typename std::iterator_traits<Iterator>::value_type>) &&
    kIsNothrowComparable<Iterator, Comparator>;

// clang-format off
template <class Iterator, class Comparator>
constexpr void merge_impl(
    Iterator iter_left_begin,
    Iterator iter_right_begin,
    Iterator iter_right_end,
    Comparator comp,
    std::vector<typename std::iterator_traits<Iterator>::value_type>& buffer)
    noexcept(kIsNoexceptMergable<Iterator, Comparator>) {
    // clang-format on
    const Iterator original_begin = iter_left_begin;
    const Iterator iter_left_end  = iter_right_begin;
    while (iter_left_begin != iter_left_end && iter_right_begin != iter_right_end) {
        if (comp(*iter_left_begin, *iter_right_begin)) {
            buffer.emplace_back(std::move(*iter_left_begin));
            ++iter_left_begin;
        } else {
            buffer.emplace_back(std::move(*iter_right_begin));
            ++iter_right_begin;
        }
    }

    std::move(iter_left_begin, iter_left_end, std::back_inserter(buffer));
    std::move(iter_right_begin, iter_right_end, std::back_inserter(buffer));
    std::move(buffer.begin(), buffer.end(), original_begin);
    buffer.clear();
}

// clang-format off
template <class Iterator, class Comparator>
constexpr void merge_sort_impl(
    Iterator iter_left_begin,
    Iterator iter_right_end,
    Comparator comp,
    std::vector<typename std::iterator_traits<Iterator>::value_type>& buffer)
    noexcept(kIsNoexceptMergable<Iterator, Comparator>) {
    // clang-format on
    const auto size = std::distance(iter_left_begin, iter_right_end);
    if (size < 2) {
        return;
    }
    Iterator iter_right_begin = iter_left_begin + size / 2;
    merge_sort_impl(iter_left_begin, iter_right_begin, comp, buffer);
    merge_sort_impl(iter_right_begin, iter_right_end, comp, buffer);
    merge_impl(iter_left_begin, iter_right_begin, iter_right_end, std::move(comp), buffer);
}

// clang-format off
template <class Iterator, class Comparator>
constexpr void merge_sort_with_empty_comparator_impl(
    Iterator iter_left_begin,
    Iterator iter_right_end,
    std::vector<typename std::iterator_traits<Iterator>::value_type>& buffer)
    noexcept(kIsNoexceptMergable<Iterator, Comparator>) {
    // clang-format on
    const auto size = std::distance(iter_left_begin, iter_right_end);
    if (size < 2) {
        return;
    }
    Iterator iter_right_begin = iter_left_begin + size / 2;
    merge_sort_with_empty_comparator_impl<Iterator, Comparator>(iter_left_begin, iter_right_begin,
                                                                buffer);
    merge_sort_with_empty_comparator_impl<Iterator, Comparator>(iter_right_begin, iter_right_end,
                                                                buffer);
    merge_impl(iter_left_begin, iter_right_begin, iter_right_end, Comparator{}, buffer);
}

// clang-format off
template <class Iterator, class Comparator>
constexpr void inplace_merge_impl(Iterator iter_left, Iterator iter_left_2, Iterator iter_right, Comparator comp)
    noexcept(kIsNoexceptMergable<Iterator, Comparator>) {
    // clang-format on
    while (iter_left != iter_left_2 && iter_left_2 != iter_right) {
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
constexpr void inplace_merge_sort_impl(Iterator iter_left, Iterator iter_right, Comparator comp)
    noexcept(kIsNoexceptMergable<Iterator, Comparator>) {
    // clang-format on
    Iterator iter_left_2 = iter_left + std::distance(iter_left, iter_right) / 2;
    if (std::distance(iter_left, iter_left_2) >= 2) {
        merge_sort_impl(iter_left, iter_left_2, comp);
    }

    if (std::distance(iter_left_2, iter_right) >= 2) {
        merge_sort_impl(iter_left_2, iter_right, comp);
    }
    merge_impl(iter_left, iter_left_2, iter_right, std::move(comp));
}

// clang-format off
template <class Iterator, class Comparator>
constexpr void inplace_merge_sort_with_empty_comparator_impl(Iterator iter_left, Iterator iter_right)
    noexcept(kIsNoexceptMergable<Iterator, Comparator>) {
    // clang-format on
    auto dist            = std::distance(iter_left, iter_right);
    Iterator iter_left_2 = iter_left + dist / 2;
    if (std::distance(iter_left, iter_left_2) >= 2) {
        merge_sort_with_empty_comparator_impl<Iterator, Comparator>(iter_left, iter_left_2);
    }

    if (std::distance(iter_left_2, iter_right) >= 2) {
        merge_sort_with_empty_comparator_impl<Iterator, Comparator>(iter_left_2, iter_right);
    }
    merge_impl(iter_left, iter_left_2, iter_right, Comparator{});
}

}  // namespace
}  // namespace merge_sort
}  // namespace detail

template <bool kInplaceSort = false>
struct MergeSortNiebloid final {
    template <class Iterator, class Comparator>
    static constexpr bool IsNoexceptSortableImpl() noexcept {
        return !kInplaceSort && detail::merge_sort::kIsNoexceptMergable<Iterator, Comparator>;
    }

    template <std::bidirectional_iterator Iterator, class Comparator = std::ranges::less>
    static constexpr void merge_sort(Iterator begin, Iterator end,
                                     Comparator comp = {}) noexcept(IsNoexceptSortableImpl<Iterator, Comparator>()) {
        using ValueType = typename std::iterator_traits<Iterator>::value_type;

        if constexpr (kInplaceSort) {
            if (detail::kOptimizeOutComparator<Comparator, ValueType>) {
                detail::merge_sort::inplace_merge_sort_with_empty_comparator_impl<Iterator,
                                                                                  Comparator>(begin,
                                                                                              end);
            } else {
                detail::merge_sort::inplace_merge_sort_impl(begin, end, std::move(comp));
            }
        } else {
            const auto size = static_cast<std::size_t>(std::distance(begin, end));
            if (size < 2) {
                return;
            }

            std::vector<ValueType> buffer;
            buffer.reserve(size);
            if constexpr (detail::kOptimizeOutComparator<Comparator, ValueType>) {
                detail::merge_sort::merge_sort_with_empty_comparator_impl<Iterator, Comparator>(
                    begin, end, buffer);
            } else {
                detail::merge_sort::merge_sort_impl(begin, end, std::move(comp), buffer);
            }
        }
    }

    template <StringsRange Range, class Comparator = std::ranges::less>
    static constexpr void merge_sort(Range&& range, Comparator comp = {}) noexcept(
        merge_sort(std::ranges::begin(range), std::ranges::end(range), std::move(comp))) {
        merge_sort(std::ranges::begin(range), std::ranges::end(range), std::move(comp));
    }

    template <std::bidirectional_iterator Iterator, class Comparator = std::ranges::less>
    constexpr void operator()(Iterator begin, Iterator end, Comparator comp = {}) const
        noexcept(merge_sort(begin, end, std::move(comp))) {
        merge_sort(begin, end, std::move(comp));
    }

    template <StringsRange Range, class Comparator = std::ranges::less>
    constexpr void operator()(Range&& range, Comparator comp = {}) const
        noexcept(merge_sort(std::ranges::begin(range), std::ranges::end(range), std::move(comp))) {
        merge_sort(std::ranges::begin(range), std::ranges::end(range), std::move(comp));
    }
};

template <bool kInplaceSort = false>
inline constexpr MergeSortNiebloid<kInplaceSort> merge_sort{};

}  // namespace algorithms
