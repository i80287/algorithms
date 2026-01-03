#pragma once

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <ranges>
#include <type_traits>

#include "string_algos_config.hpp"

namespace algorithms {
namespace detail {
namespace string_quick_sort {
namespace {

template <class Iterator, class CompareFn>
constexpr Iterator move_strings_to_left(Iterator begin, Iterator end, CompareFn compare_func) noexcept {
    auto insertion_pos_iter = std::find_if_not(begin, end, compare_func);
    if (insertion_pos_iter == end) {
        return insertion_pos_iter;
    }

    auto first_equal_iter = insertion_pos_iter;
    for (++first_equal_iter; first_equal_iter != end;) {
        first_equal_iter = std::find_if(first_equal_iter, end, compare_func);
        if (first_equal_iter == end) {
            break;
        }

        insertion_pos_iter->swap(*first_equal_iter);
        ++insertion_pos_iter;
        insertion_pos_iter = std::find_if_not(insertion_pos_iter, end, compare_func);
    }

    return insertion_pos_iter;
}

template <class Iterator>
constexpr Iterator select_pivot_string(Iterator begin, Iterator end) noexcept {
    // length should be >= 2 by the invariant of quick_sort_impl.
    // Thus we can derefer (begin + length - 1) safely.
    const auto length = static_cast<std::size_t>(std::distance(begin, end));
#ifdef __GNUC__
    if (length < 2) {
        __builtin_unreachable();
    }
#endif
    const auto qrt_iter = begin + static_cast<std::ptrdiff_t>(length / 4);
    const auto mid_iter = begin + static_cast<std::ptrdiff_t>(length / 2);
    const auto thr_qrt_iter = begin + static_cast<std::ptrdiff_t>((3 * length) / 4);

    if (*qrt_iter < *mid_iter) {
        if (*mid_iter < *thr_qrt_iter) {
            return mid_iter;
        }
        if (*qrt_iter < *thr_qrt_iter) {
            return thr_qrt_iter;
        }
        return qrt_iter;
    }
    if (*thr_qrt_iter < *mid_iter) {
        return mid_iter;
    }
    if (*thr_qrt_iter < *qrt_iter) {
        return thr_qrt_iter;
    }

    return qrt_iter;
}

template <class Iterator>
struct PartitionResult final {
    Iterator first_equal;
    Iterator first_greater;
};

template <class Iterator>
constexpr PartitionResult<Iterator> partition(Iterator begin,
                                              Iterator end,
                                              Iterator pivot_iter,
                                              const std::size_t common_prefix_length) noexcept {
    using StringType = IteratorStringType<Iterator>;
    using UCharType = std::make_unsigned_t<typename StringType::value_type>;
    const UCharType pivot_cmp_char = static_cast<UCharType>((*pivot_iter)[common_prefix_length]);

    begin = move_strings_to_left(begin, end, [=](const StringType& s) constexpr noexcept {
        return static_cast<UCharType>(s[common_prefix_length]) < pivot_cmp_char;
    });

    Iterator first_greater = move_strings_to_left(begin, end, [=](const StringType& s) constexpr noexcept {
        return static_cast<UCharType>(s[common_prefix_length]) == pivot_cmp_char;
    });

    return {begin, first_greater};
}

template <class Iterator>
constexpr void string_quick_sort_impl(Iterator begin, Iterator end, const std::size_t common_prefix_length) noexcept {
    if (std::distance(begin, end) <= 1) {
        return;
    }

    begin = move_strings_to_left(begin, end, [=](const IteratorStringType<Iterator>& s) constexpr noexcept {
        return s.size() == common_prefix_length;
    });
    if (std::distance(begin, end) <= 1) {
        return;
    }

    const auto pivot_iter = select_pivot_string(begin, end);
    const auto [first_equal, first_greater] = partition(begin, end, pivot_iter, common_prefix_length);
    string_quick_sort_impl(begin, first_equal, common_prefix_length);
    string_quick_sort_impl(first_equal, first_greater, common_prefix_length + 1);
    string_quick_sort_impl(first_greater, end, common_prefix_length);
}

}  // namespace
}  // namespace string_quick_sort
}  // namespace detail

struct StringQuickSortNiebloid final {
    template <StringIterator Iterator>
    static constexpr void string_quick_sort(Iterator begin, Iterator end) noexcept {
        detail::string_quick_sort::string_quick_sort_impl(begin, end, 0);
    }

    template <StringsRange Range>
    static constexpr void string_quick_sort(Range&& range) noexcept {
        string_quick_sort(std::ranges::begin(range), std::ranges::end(range));
    }

    template <StringIterator Iterator>
    constexpr void operator()(Iterator begin, Iterator end) const noexcept {
        string_quick_sort(begin, end);
    }

    template <StringsRange Range>
    constexpr void operator()(Range&& range) const noexcept {
        string_quick_sort(std::ranges::begin(range), std::ranges::end(range));
    }
};

inline constexpr StringQuickSortNiebloid string_quick_sort{};

}  // namespace algorithms
