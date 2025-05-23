#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <type_traits>
#include <vector>

/// @brief Binary Index tree (Fenwick tree) implemented for the sums
/// on the segments (+ add on index i and assign on index i in log time)
/// @tparam ValueType type to store sums as
template <class ValueType = int64_t>
class BITree {
    static_assert(std::is_arithmetic_v<ValueType>);

public:
    explicit BITree(const std::vector<ValueType>& elements) : sums_(elements.size()) {
        RecalculateSums(elements.data());
    }

    BITree(std::initializer_list<ValueType> elements) : sums_(elements.size()) {
        RecalculateSums(elements.begin());
    }

    constexpr void RecalculateSums(auto begin_it_or_ptr) noexcept {
        for (size_t i = 0, n = Size(); i < n; ++i) {
            ValueType sum = ValueType(0);
            for (size_t j = i & (i + 1); j <= i; ++j) {
                sum += begin_it_or_ptr[j];
            }

            sums_[i] = sum;
        }
    }

    /// @brief Gets preffix sum of elements on [0..pos]
    /// @param pos Right index of the elements (included) (maybe equal to size_t(-1) )
    /// @return Preffix sum of elements on [0..pos]
    constexpr ValueType PrefixSum(size_t pos) const noexcept {
        ValueType ans = ValueType(0);
        for (size_t i = pos; i != static_cast<size_t>(-1); i = (i & (i + 1)) - 1) {
            ans += sums_[i];
        }

        return ans;
    }

    /// @brief Gets sum of the elements on [l..r]
    /// @param l Left index of the segment (included)
    /// @param r Right index of the segment (included)
    /// @return Sum of the elements on [l..r]
    constexpr ValueType GetSum(size_t l, size_t r) const noexcept {
        return PrefixSum(r) - PrefixSum(l - 1);
    }

    /// @brief Gets element of the current elements on position index
    /// @param index Position of the element
    /// @return Element of the current elements on position index
    constexpr ValueType operator[](size_t index) const noexcept {
        return PrefixSum(index) - PrefixSum(index - 1);
    }

    constexpr void AddAt(size_t pos, ValueType value) noexcept {
        for (size_t i = pos, n = sums_.size(); i < n; i = i | (i + 1)) {
            sums_[i] += value;
        }
    }

    constexpr void AssignAt(size_t pos, ValueType value) noexcept {
        AddAt(pos, value - (*this)[pos]);
    }

    constexpr size_t Size() const noexcept {
        return sums_.size();
    }

    // sums_[i] = sum of a[j] where j in [i & (i + 1); i]
    std::vector<ValueType> sums_;
};

int main() {
    BITree tree{1, 4, 2, 6, 3, 6, 3};
    for (size_t i = 0; i < tree.Size(); ++i) {
        std::cout << tree[i] << ' ';
    }
    std::cout << '\n';

    tree.AddAt(2, 10);
    for (size_t i = 0; i < tree.Size(); ++i) {
        std::cout << tree[i] << ' ';
    }
    std::cout << '\n';

    tree.AssignAt(2, 10);
    for (size_t i = 0; i < tree.Size(); ++i) {
        std::cout << tree[i] << ' ';
    }
    std::cout << '\n';

    std::cout << "Sum on [3; 6]: " << tree.GetSum(3, 6) << '\n'
              << "Prefix sum on [0; 2]: " << tree.PrefixSum(2) << '\n';
}
