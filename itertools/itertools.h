#pragma once

#include <cstdint>
#include <iterator>
#include <utility>

template <class Iterator>
class IteratorRange {
public:
    constexpr IteratorRange(Iterator begin, Iterator end) noexcept : begin_(begin), end_(end) {
    }

    constexpr Iterator begin() const noexcept {  // NOLINT
        return begin_;
    }

    constexpr Iterator end() const noexcept {  // NOLINT
        return end_;
    }

private:
    Iterator begin_, end_;
};

class RangeIterator {
public:
    constexpr RangeIterator(int64_t value, int64_t step) noexcept : value_(value), step_(step) {
    }

    constexpr bool operator==(const RangeIterator& other) const noexcept {
        return value_ == other.value_;
    }

    constexpr bool operator!=(const RangeIterator& other) const noexcept {
        return value_ < other.value_;
    }

    constexpr bool operator<(const RangeIterator& other) const noexcept {
        return value_ < other.value_;
    }

    constexpr RangeIterator& operator++() noexcept {
        value_ += step_;
        return *this;
    }

    constexpr int64_t& operator*() noexcept {
        return value_;
    }

    constexpr const int64_t& operator*() const noexcept {
        return value_;
    }

private:
    int64_t value_;
    int64_t step_;
};

template <class Iterator1, class Iterator2>
class ZipIterator {
public:
    constexpr ZipIterator(Iterator1 iter1, Iterator2 iter2) : iter1_(iter1), iter2_(iter2) {
    }

    constexpr bool operator!=(const ZipIterator& other) const {
        return iter1_ != other.iter1_ && iter2_ != other.iter2_;
    }

    constexpr ZipIterator& operator++() {
        ++iter1_;
        ++iter2_;
        return *this;
    }

    constexpr auto operator*() {
        return std::make_pair(*iter1_, *iter2_);
    }

    constexpr auto operator*() const {
        return std::make_pair(*iter1_, *iter2_);
    }

private:
    Iterator1 iter1_;
    Iterator2 iter2_;
};

template <class Iterator>
class GroupIterator {
public:
    constexpr explicit GroupIterator(Iterator begin, Iterator end)
        : interval_start_(begin), interval_end_(begin), end_(end) {
        operator++();
    }

    constexpr bool operator!=(const GroupIterator& other) const {
        return interval_start_ != other.interval_start_;
    }

    constexpr GroupIterator& operator++() {
        interval_start_ = interval_end_;
        while (interval_end_ != end_ && *interval_end_ == *interval_start_) {
            ++interval_end_;
        }

        return *this;
    }

    constexpr auto operator*() {
        return IteratorRange(interval_start_, interval_end_);
    }

    constexpr auto operator*() const {
        return IteratorRange(interval_start_, interval_end_);
    }

private:
    Iterator interval_start_;
    Iterator interval_end_;
    Iterator end_;
};

constexpr IteratorRange<RangeIterator> Range(int64_t count) noexcept {
    return IteratorRange(RangeIterator(0, 1), RangeIterator(count, 0));
}

constexpr IteratorRange<RangeIterator> Range(int64_t start, int64_t end) noexcept {
    return IteratorRange(RangeIterator(start, 1), RangeIterator(end, 0));
}

constexpr IteratorRange<RangeIterator> Range(int64_t start, int64_t end, int64_t step) noexcept {
    return IteratorRange(RangeIterator(start, step), RangeIterator(end, 0));
}

template <class Container1, class Container2>
auto Zip(const Container1& c1, const Container2& c2) {
    return IteratorRange(ZipIterator(std::begin(c1), std::begin(c2)), ZipIterator(std::end(c1), std::end(c2)));
}

template <class Container>
auto Group(const Container& container) {
    return IteratorRange(GroupIterator(std::begin(container), std::end(container)),
                         GroupIterator(std::end(container), std::end(container)));
}
