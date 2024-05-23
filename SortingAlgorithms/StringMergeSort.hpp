#pragma once

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <vector>

#include "string_algos_config.hpp"

namespace algorithms {
namespace detail {
namespace string_merge_sort {
namespace {

template <class IteratorStringType>
struct Pair final {
    IteratorStringType str;
    std::size_t lcp;
};

struct LCPCompareResult final {
    std::strong_ordering cmp_result;
    std::size_t new_lcp;
};

template <class IteratorStringType>
constexpr LCPCompareResult lcp_compare(const IteratorStringType& str1,
                                       const IteratorStringType& str2, std::size_t lcp) noexcept {
    using CharType  = typename IteratorStringType::value_type;
    using UCharType = std::make_unsigned_t<CharType>;
    assert(lcp <= str1.size());
    assert(lcp <= str2.size());
    assert((std::basic_string_view<CharType>(str1).substr(0, lcp) ==
            std::basic_string_view<CharType>(str2).substr(0, lcp)));

    const auto min_size = std::min(str1.size(), str2.size());
    for (auto i = lcp; i < min_size; i++) {
        if constexpr (!kIsMeasuringTime) {
            string_char_comparisons_count++;
        }
        const auto cmp_res = static_cast<UCharType>(str1[i]) <=> static_cast<UCharType>(str2[i]);
        if (cmp_res != std::strong_ordering::equivalent) {
            return {cmp_res, i};
        }
    }

    return {str1.size() <=> str2.size(), min_size};
}

template <class Iterator, class IteratorStringType>
constexpr void string_merge_impl(Iterator iter_left_begin, Iterator iter_right_begin,
                                 Iterator iter_right_end,
                                 std::vector<Pair<IteratorStringType>>& buffer) noexcept {
    const Iterator original_begin = iter_left_begin;
    const Iterator iter_left_end  = iter_right_begin;
    while (iter_left_begin != iter_left_end && iter_right_begin != iter_right_end) {
        if (iter_left_begin->lcp > iter_right_begin->lcp) {
            buffer.emplace_back(std::move(*iter_left_begin));
            ++iter_left_begin;
        } else if (iter_left_begin->lcp < iter_right_begin->lcp) {
            buffer.emplace_back(std::move(*iter_right_begin));
            ++iter_right_begin;
        } else {
            auto [cmp_result, new_lcp] =
                lcp_compare(iter_left_begin->str, iter_right_begin->str, iter_right_begin->lcp);
            if (cmp_result == std::strong_ordering::less) {
                buffer.emplace_back(std::move(*iter_left_begin));
                ++iter_left_begin;
                iter_right_begin->lcp = new_lcp;
            } else {
                buffer.emplace_back(std::move(*iter_right_begin));
                ++iter_right_begin;
                iter_left_begin->lcp = new_lcp;
            }
        }
    }

    std::move(iter_left_begin, iter_left_end, std::back_inserter(buffer));
    std::move(iter_right_begin, iter_right_end, std::back_inserter(buffer));
    std::move(buffer.begin(), buffer.end(), original_begin);
    buffer.clear();
}

template <class Iterator, class IteratorStringType>
constexpr void string_merge_sort_impl(Iterator iter_left_begin, Iterator iter_right_end,
                                      std::vector<Pair<IteratorStringType>>& buffer) noexcept {
    const auto size = std::distance(iter_left_begin, iter_right_end);
    if (size < 2) {
        return;
    }
    Iterator iter_right_begin = iter_left_begin + size / 2;
    string_merge_sort_impl(iter_left_begin, iter_right_begin, buffer);
    string_merge_sort_impl(iter_right_begin, iter_right_end, buffer);
    string_merge_impl(iter_left_begin, iter_right_begin, iter_right_end, buffer);
}

}  // namespace
}  // namespace string_merge_sort
}  // namespace detail

struct StringMergeSortNiebloid final {
    template <StringIterator Iterator>
    static constexpr void string_merge_sort(Iterator begin, Iterator end) {
        using IteratorStringType = std::iterator_traits<Iterator>::value_type;
        using PairType           = detail::string_merge_sort::Pair<IteratorStringType>;

        const auto size = static_cast<std::size_t>(std::distance(begin, end));
        std::vector<PairType> vec;
        std::vector<PairType> buffer;
        vec.reserve(size);
        buffer.reserve(size);
        for (auto iter = begin; iter != end; ++iter) {
            vec.emplace_back(std::move(*iter), std::size_t(0));
        }
        detail::string_merge_sort::string_merge_sort_impl(vec.begin(), vec.end(), buffer);
        for (auto&& [str, lcp] : vec) {
            *begin = std::move(str);
            ++begin;
        }
    }

    template <StringsRange Range>
    static constexpr void string_merge_sort(Range&& range) {
        string_merge_sort(std::ranges::begin(range), std::ranges::end(range));
    }

    template <StringIterator Iterator>
    constexpr void operator()(Iterator begin, Iterator end) const {
        string_merge_sort(begin, end);
    }

    template <StringsRange Range>
    constexpr void operator()(Range&& range) const {
        string_merge_sort(std::ranges::begin(range), std::ranges::end(range));
    }
};

inline constexpr StringMergeSortNiebloid string_merge_sort{};

}  // namespace algorithms
