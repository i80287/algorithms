#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#if __cplusplus >= 202002L
#include <bit>
#include <span>
#endif

enum class UpdateOperation {
    kAdd,
    kMultiply,
    kSetEqual,
};

enum class GetOperation {
    kSum,
    kProduct,
    kMax,
    kMin,
};

template <GetOperation get_op, UpdateOperation upd_op, typename value_t = int64_t>
class [[nodiscard]] SegmentTree final {
    static_assert(std::is_arithmetic_v<value_t>);

    value_t* tree_ = nullptr;
    // Number of nodes above last layer with actual data
    std::size_t n_ = 0;

public:
#if __cplusplus >= 202002L
    constexpr
#endif
        explicit SegmentTree(const std::vector<value_t>& data)
        : SegmentTree(data.data(), data.size()) {
    }

    template <std::size_t N>
#if __cplusplus >= 202002L
    constexpr
#endif
        explicit SegmentTree(const value_t (&data)[N])
        : SegmentTree(static_cast<const value_t*>(data), N) {
    }

    template <std::size_t N>
#if __cplusplus >= 202002L
    constexpr
#endif
        explicit SegmentTree(const std::array<value_t, N>& data)
        : SegmentTree(data.data(), data.size()) {
    }

#if __cplusplus >= 202002L
    template <std::size_t Extent>
    constexpr explicit SegmentTree(std::span<const value_t, Extent> data) : SegmentTree(data.data(), data.size()) {}
#endif

#if __cplusplus >= 202002L
    constexpr
#endif
        SegmentTree(const value_t* data, std::size_t data_size)
        : n_(nearest_two_pow(data_size)) {
        // Node with index 0 is not used, number of used nodes = 2 * n - 1
        tree_ = std::allocator<value_t>().allocate(tree_size());

        value_t* copy_end = std::copy(data, data + data_size, tree_ + n_);
        std::size_t tree_unused_size = n_ - data_size;
        if constexpr (get_op == GetOperation::kSum) {
            std::fill_n(copy_end, tree_unused_size, value_t{0});
        } else if constexpr (get_op == GetOperation::kProduct) {
            std::fill_n(copy_end, tree_unused_size, value_t{1});
        } else if constexpr (get_op == GetOperation::kMax) {
            std::fill_n(copy_end, tree_unused_size, std::numeric_limits<value_t>::min());
        } else if constexpr (get_op == GetOperation::kMin) {
            std::fill_n(copy_end, tree_unused_size, std::numeric_limits<value_t>::max());
        }

        for (std::size_t i = n_ - 1; i != 0; i--) {
            std::size_t l = 2 * i;
            std::size_t r = l | 1;

            if constexpr (get_op == GetOperation::kSum) {
                tree_[i] = tree_[l] + tree_[r];
            } else if constexpr (get_op == GetOperation::kProduct) {
                tree_[i] = tree_[l] * tree_[r];
            } else if constexpr (get_op == GetOperation::kMax) {
                tree_[i] = std::max(tree_[l], tree_[r]);
            } else if constexpr (get_op == GetOperation::kMin) {
                tree_[i] = std::min(tree_[l], tree_[r]);
            }
        }
    }

#if __cplusplus >= 202002L
    constexpr
#endif
        SegmentTree(const SegmentTree& other)
        : tree_(std::allocator<value_t>{}.allocate(other.tree_size())), n_(other.n_) {
        std::copy_n(other.tree_, other.tree_size(), tree_);
    }

#if __cplusplus >= 202002L
    constexpr
#endif
        SegmentTree&
        operator=(const SegmentTree& other) {
        return *this = SegmentTree(other);
    }

#if __cplusplus >= 202002L
    constexpr
#endif
        SegmentTree(SegmentTree&& other) noexcept
        : tree_(std::exchange(other.tree_, nullptr)), n_(std::exchange(other.n_, nullptr)) {
    }

#if __cplusplus >= 202002L
    constexpr
#endif
        SegmentTree&
        operator=(SegmentTree&& other) noexcept {
        swap(*this, other);
        return *this;
    }

#if __cplusplus >= 202002L
    constexpr
#endif
        friend void
        swap(SegmentTree& lhs, SegmentTree& rhs) noexcept {
        using std::swap;
        swap(lhs.tree_, rhs.tree_);
        swap(lhs.n_, rhs.n_);
    }

    /// @brief Update in the zero based index i (add, multiply or set equal)
    /// @param i zero based index in the array
    /// @param upd_value
    constexpr void Update(std::size_t i, value_t upd_value) noexcept {
        assert(i < n_);
        i += n_;

        if constexpr (upd_op == UpdateOperation::kAdd) {
            tree_[i] += upd_value;
        } else if constexpr (upd_op == UpdateOperation::kMultiply) {
            tree_[i] *= upd_value;
        } else if constexpr (upd_op == UpdateOperation::kSetEqual) {
            tree_[i] = upd_value;
        }

        for (i /= 2; i != 0; i /= 2) {
            std::size_t l = 2 * i;
            std::size_t r = l | 1;

            if constexpr (get_op == GetOperation::kSum) {
                tree_[i] = tree_[l] + tree_[r];
            } else if constexpr (get_op == GetOperation::kProduct) {
                tree_[i] = tree_[l] * tree_[r];
            } else if constexpr (get_op == GetOperation::kMax) {
                tree_[i] = std::max(tree_[l], tree_[r]);
            } else if constexpr (get_op == GetOperation::kMin) {
                tree_[i] = std::min(tree_[l], tree_[r]);
            }
        }
    }

    /// @brief get on the [l; r]
    /// @param l left index (including)
    /// @param r right index (including)
    /// @return value (sum, product, min or max) on the [l; r]
    [[nodiscard]] constexpr value_t Get(std::size_t l, std::size_t r) const noexcept {
        assert(l <= r && r < n_);
        l += n_;
        r += n_;
        value_t res{};
        if constexpr (get_op == GetOperation::kSum) {
            res = 0;
        } else if constexpr (get_op == GetOperation::kProduct) {
            res = 1;
        } else if constexpr (get_op == GetOperation::kMax) {
            res = std::max(tree_[l], tree_[r]);
        } else if constexpr (get_op == GetOperation::kMin) {
            res = std::min(tree_[l], tree_[r]);
        }

        for (; l <= r; l /= 2, r /= 2) {
            assert(l != 0);
            if (l % 2 != 0) {
                // l is right son
                if constexpr (get_op == GetOperation::kSum) {
                    res += tree_[l];
                } else if constexpr (get_op == GetOperation::kProduct) {
                    res *= tree_[l];
                } else if constexpr (get_op == GetOperation::kMax) {
                    res = std::max(res, tree_[l]);
                } else if constexpr (get_op == GetOperation::kMin) {
                    res = std::min(res, tree_[l]);
                }
                l++;
            }

            if (r % 2 == 0) {
                // r is left son
                if constexpr (get_op == GetOperation::kSum) {
                    res += tree_[r];
                } else if constexpr (get_op == GetOperation::kProduct) {
                    res *= tree_[r];
                } else if constexpr (get_op == GetOperation::kMax) {
                    res = std::max(res, tree_[r]);
                } else if constexpr (get_op == GetOperation::kMin) {
                    res = std::min(res, tree_[r]);
                }
                r--;
            }
        }

        return res;
    }

#if __cplusplus >= 202002L
    constexpr
#endif
        ~SegmentTree() {
        std::allocator<value_t>{}.deallocate(tree_, tree_size());
    }

private:
    [[nodiscard]] constexpr auto tree_size() const noexcept {
        return 2 * n_;
    }

    [[nodiscard]] static constexpr bool is_2_pow(std::size_t n) noexcept {
        return (n & (n - 1)) == 0;
    }

    [[nodiscard]] static constexpr std::size_t log2_ceiled(std::size_t n) noexcept {
#if __cplusplus >= 202002L
        const uint32_t lz_count = uint32_t(std::countl_zero(n | 1));
#else
        const uint32_t lz_count = uint32_t(__builtin_clzll(n | 1));
#endif
        return (63 ^ lz_count) + !is_2_pow(n);
    }

    [[nodiscard]] constexpr std::size_t nearest_two_pow(std::size_t n) noexcept {
        return std::size_t(1u) << log2_ceiled(n);
    }
};

int main() {
    const int64_t arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    SegmentTree<GetOperation::kProduct, UpdateOperation::kSetEqual> tree(arr);
    tree.Update(0, 2);
    assert(tree.Get(0, 4) == 2 * 2 * 3 * 4 * 5);
    assert(tree.Get(0, 9) == 2 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9 * 10);
}
