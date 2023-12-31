#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>
#include <limits>

#if __cplusplus >= 202002L
#include <bit>
#endif

enum class UpdateOperation {
    add,
    multiply,
    set_equal,
};

enum class GetOperation {
    sum,
    product,
    max,
    min,
};

static constexpr bool is_2_pow(size_t n) noexcept {
    return (n & (n - 1)) == 0;
}

static constexpr size_t log2_ceiled(size_t n) noexcept {
#if __cplusplus >= 202002L
    const uint32_t lz_count = uint32_t(std::countl_zero(n | 1));
#else
    const uint32_t lz_count = uint32_t(__builtin_clzll(n | 1));
#endif
    return (63 ^ lz_count) + !is_2_pow(n);
}

static constexpr size_t nearest_two_pow(size_t n) noexcept {
    return size_t(1u) << log2_ceiled(n);
}

template <GetOperation get_op, UpdateOperation upd_op, typename value_t = int64_t>
class SegmentTree {
    value_t* tree_ = nullptr;
    // Number of nodes above last layer with actual data
    size_t n_ = 0;
public:
    constexpr SegmentTree() noexcept = default;

    explicit SegmentTree(const std::vector<value_t>& data)
        : SegmentTree(data.data(), data.size()) {}

    SegmentTree(const value_t* data, size_t data_size) {
        Build(data, data_size);
    }

    void Build(const std::vector<value_t>& data) {
        Build(data.data(), data.size());
    }

    void Build(const value_t* data, size_t data_size) {
        n_ = nearest_two_pow(data_size);
        // Node with index 0 is not used, number of used nodes = 2 * n - 1
        size_t tree_size = 2 * n_;
        tree_ = static_cast<value_t*>(operator new(tree_size * sizeof(value_t)));

        memcpy(tree_ + n_, data, data_size * sizeof(value_t));

        value_t* copy_end = tree_ + n_ + data_size;
        size_t tree_unused_size = n_ - data_size;
        if constexpr (get_op == GetOperation::sum) {
            memset(copy_end, 0, tree_unused_size * sizeof(value_t));
        } else if constexpr (get_op == GetOperation::product) {
            for (size_t i = 0; i < tree_unused_size; i++) {
                copy_end[i] = 1;
            }
        } else if constexpr (get_op == GetOperation::max) {
            for (size_t i = 0; i < tree_unused_size; i++) {
                copy_end[i] = std::numeric_limits<value_t>::min();
            }
        } else if constexpr (get_op == GetOperation::min) {
            for (size_t i = 0; i < tree_unused_size; i++) {
                copy_end[i] = std::numeric_limits<value_t>::max();
            }
        }

        for (size_t i = n_ - 1; i != 0; i--) {
            size_t l = 2 * i;
            size_t r = l | 1;

            if constexpr (get_op == GetOperation::sum) {
                tree_[i] = tree_[l] + tree_[r];
            } else if constexpr (get_op == GetOperation::product) {
                tree_[i] = tree_[l] * tree_[r];
            } else if constexpr (get_op == GetOperation::max) {
                tree_[i] = std::max(tree_[l], tree_[r]);
            } else if constexpr (get_op == GetOperation::min) {
                tree_[i] = std::min(tree_[l], tree_[r]);
            }
        }
    }

    /// @brief 
    /// @param i zero based index in the array
    /// @param upd_value
    void Update(size_t i, value_t upd_value) noexcept {
        assert(i < n_);
        i += n_;

        if constexpr (upd_op == UpdateOperation::add) {
            tree_[i] += upd_value;
        } else if constexpr (upd_op == UpdateOperation::multiply) {
            tree_[i] *= upd_value;
        } else if constexpr (upd_op == UpdateOperation::set_equal) {
            tree_[i] = upd_value;
        }

        for (i /= 2; i != 0; i /= 2) {
            size_t l = 2 * i;
            size_t r = l | 1;

            if constexpr (get_op == GetOperation::sum) {
                tree_[i] = tree_[l] + tree_[r];
            } else if constexpr (get_op == GetOperation::product) {
                tree_[i] = tree_[l] * tree_[r];
            } else if constexpr (get_op == GetOperation::max) {
                tree_[i] = std::max(tree_[l], tree_[r]);
            } else if constexpr (get_op == GetOperation::min) {
                tree_[i] = std::min(tree_[l], tree_[r]);
            }
        }
    }

    /// @brief get on the data[l; r]
    /// @param l left index (including)
    /// @param r right index (including)
    /// @return 
    value_t Get(size_t l, size_t r) noexcept {
        assert(l <= r && r < n_);
        l += n_, r += n_;
        value_t res;
        if constexpr (get_op == GetOperation::sum) {
            res = 0;
        } else if constexpr (get_op == GetOperation::product) {
            res = 1;
        } else if constexpr (get_op == GetOperation::max) {
            res = std::max(tree_[l], tree_[r]);
        } else if constexpr (get_op == GetOperation::min) {
            res = std::min(tree_[l], tree_[r]);
        }

        for (; l <= r; l /= 2, r /= 2) {
            assert(l != 0);
            if (l % 2 != 0) {
                // l is right son
                if constexpr (get_op == GetOperation::sum) {
                    res += tree_[l];
                } else if constexpr (get_op == GetOperation::product) {
                    res *= tree_[l];
                } else if constexpr (get_op == GetOperation::max) {
                    res = std::max(res, tree_[l]);
                } else if constexpr (get_op == GetOperation::min) {
                    res = std::min(res, tree_[l]);
                }
                l++;
            }

            if (r % 2 == 0) {
                // r is left son
                if constexpr (get_op == GetOperation::sum) {
                    res += tree_[r];
                } else if constexpr (get_op == GetOperation::product) {
                    res *= tree_[r];
                } else if constexpr (get_op == GetOperation::max) {
                    res = std::max(res, tree_[r]);
                } else if constexpr (get_op == GetOperation::min) {
                    res = std::min(res, tree_[r]);
                }
                r--;
            }
        }

        return res;
    }

    ~SegmentTree() {
        operator delete(tree_);
        tree_ = nullptr;
    }
};

int main() {
    int arr[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    constexpr size_t arr_len = sizeof(arr) / sizeof(arr[0]);

    SegmentTree<GetOperation::product, UpdateOperation::set_equal, int> tree(arr, arr_len);
    tree.Update(0, 2);
    assert(tree.Get(0, 4) == 240);
}
