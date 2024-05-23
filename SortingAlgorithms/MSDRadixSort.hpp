#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <limits>
#include <ranges>
#include <type_traits>
#include <vector>

#include "string_algos_config.hpp"
#include "StringQuickSort.hpp"

namespace algorithms {
namespace detail {
namespace msd_radix_sort {
namespace {

template <StringIterator Iterator>
inline constexpr std::size_t kAlphabetSize =
    std::numeric_limits<StringIteratorUChar<Iterator>>::max() + 1;

template <class Iterator, class StringType>
constexpr auto counting_sort_impl(const Iterator begin, const Iterator end,
                                  const std::size_t common_prefix_length,
                                  std::vector<StringType>& tmp_buffer) noexcept {
    constexpr auto kThreshold = kAlphabetSize<Iterator>;

    using UChar = StringIteratorUChar<Iterator>;
    std::array<std::uint32_t, kThreshold> count{};
    std::array<std::uint32_t, kThreshold> first_index{};

    const auto size = static_cast<std::size_t>(std::distance(begin, end));
    tmp_buffer.resize(size);

    for (auto iter = begin; iter != end; ++iter) {
        const StringType& str = *iter;
        count[static_cast<UChar>(str[common_prefix_length])]++;
    }

    first_index[0] = 0;
    for (std::size_t i = 1; i < first_index.size(); ++i) {
        first_index[i] = first_index[i - 1] + count[i - 1];
    }

    for (auto iter = begin; iter != end; ++iter) {
        std::size_t pos                = static_cast<UChar>((*iter)[common_prefix_length]);
        tmp_buffer[first_index[pos]++] = std::move(*iter);
    }

    std::move(tmp_buffer.begin(), tmp_buffer.end(), begin);
    tmp_buffer.clear();
    return first_index;
}

template <bool kSwitchToQuickSort, class Iterator, class StringType>
constexpr void msd_radix_sort_impl(Iterator begin, Iterator end,
                                   const std::size_t common_prefix_length,
                                   std::vector<StringType>& buffer) noexcept {
    const auto size = static_cast<std::size_t>(std::distance(begin, end));
    if constexpr (kSwitchToQuickSort) {
        if (size < kAlphabetSize<Iterator>) {
            detail::string_quick_sort::string_quick_sort_impl(begin, end, common_prefix_length);
            return;
        }
    }

    begin = detail::string_quick_sort::move_strings_to_left(
        begin, end, [=](const IteratorStringType<Iterator>& s) constexpr noexcept {
            return s.size() == common_prefix_length;
        });

    auto indexes = counting_sort_impl(begin, end, common_prefix_length, buffer);
#if defined(__GNUC__) && __GNUC__ >= 13 && defined(__cpp_lib_ranges_zip) && \
    __cpp_lib_ranges_zip >= 202110L
    for (auto [i1, i2] : std::views::pairwise(indexes)) {
#else
    for (std::size_t i = 1; i < indexes.size(); i++) {
        auto i1 = indexes[i - 1];
        auto i2 = indexes[i];
#endif
        if (i2 - i1 >= 2) {
            msd_radix_sort_impl<kSwitchToQuickSort>(begin + i1, begin + i2,
                                                    common_prefix_length + 1, buffer);
        }
    }
}

}  // namespace
}  // namespace msd_radix_sort
}  // namespace detail

template <bool kSwitchToQuickSort>
struct MSDRadixSortNiebloid final {
    template <StringIterator Iterator>
    static constexpr void msd_radix_sort(Iterator begin, Iterator end) {
        std::vector<IteratorStringType<Iterator>> buffer(
            static_cast<std::size_t>(std::distance(begin, end)));
        detail::msd_radix_sort::msd_radix_sort_impl<kSwitchToQuickSort>(begin, end, 0, buffer);
    }

    template <StringsRange Range>
    static constexpr void msd_radix_sort(Range&& range) {
        msd_radix_sort(std::ranges::begin(range), std::ranges::end(range));
    }

    template <StringIterator Iterator>
    constexpr void operator()(Iterator begin, Iterator end) const {
        msd_radix_sort(begin, end);
    }

    template <StringsRange Range>
    constexpr void operator()(Range&& range) const {
        msd_radix_sort(std::ranges::begin(range), std::ranges::end(range));
    }
};

template <bool kSwitchToQuickSort = true>
inline constexpr MSDRadixSortNiebloid<kSwitchToQuickSort> msd_radix_sort{};

}  // namespace algorithms
