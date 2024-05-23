#pragma once

#include <concepts>
#include <iterator>
#include <ranges>
#include <string>
#include <type_traits>

namespace algorithms {

template <class Iterator>
concept StringIterator =
    std::random_access_iterator<Iterator> &&
    (std::same_as<
        typename std::iterator_traits<Iterator>::value_type,
        std::basic_string<typename std::iterator_traits<Iterator>::value_type::value_type>>);

template <class Range>
concept StringsRange = std::ranges::bidirectional_range<Range> &&
                       StringIterator<decltype(std::ranges::begin(std::declval<Range>()))>;

template <StringIterator Iterator>
using IteratorStringType = typename std::iterator_traits<Iterator>::value_type;

template <StringIterator Iterator>
using StringIteratorUChar = std::make_unsigned_t<typename IteratorStringType<Iterator>::value_type>;

namespace detail {

template <class Comparator, class ValueType>
inline constexpr bool kOptimizeOutComparator =
    std::is_same_v<Comparator, std::ranges::less> ||
    std::is_same_v<Comparator, std::ranges::less_equal> ||
    std::is_same_v<Comparator, std::ranges::greater> ||
    std::is_same_v<Comparator, std::ranges::greater_equal> ||
    std::is_same_v<Comparator, std::less<ValueType>> ||
    std::is_same_v<Comparator, std::less_equal<ValueType>> ||
    std::is_same_v<Comparator, std::greater<ValueType>> ||
    std::is_same_v<Comparator, std::greater_equal<ValueType>> ||
    std::is_same_v<Comparator, std::less<void>> ||
    std::is_same_v<Comparator, std::less_equal<void>> ||
    std::is_same_v<Comparator, std::greater<void>> ||
    std::is_same_v<Comparator, std::greater_equal<void>>;

}  // namespace detail

}  // namespace algorithms
