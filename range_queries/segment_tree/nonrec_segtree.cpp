#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>

#if __cplusplus >= 202002L
#include <bit>
#endif

#if __cplusplus >= 202002L
constexpr
#endif
static size_t nearest_two_pow(size_t n) noexcept {
#if __cplusplus >= 202002L
    const uint32_t lz_count = uint32_t(std::countl_zero(n | 1));
#else
    const uint32_t lz_count = uint32_t(__builtin_clzll(n | 1));
#endif
    const uint32_t is_two_pow = uint32_t((n & (n - 1)) == 0);
    return size_t(1u) << (sizeof(size_t) * 8 - lz_count - is_two_pow);
}

template <typename value_t = int64_t>
class SumSegTree {
    value_t* tree_;
    // Number of nodes above last layer with actual data
    size_t n_;
public:
    explicit SumSegTree(const std::vector<value_t>& data)
        : SumSegTree(data.data(), data.size()) {}

    SumSegTree(const value_t* data, size_t data_size)
        : n_{nearest_two_pow(data_size)} {
        // Node with index 0 is not used, number of used nodes = 2 * n - 1
        size_t tree_size = 2 * n_;
        tree_ = static_cast<value_t*>(operator new(tree_size * sizeof(value_t)));
        memset(static_cast<void*>(tree_), 0, tree_size * sizeof(value_t));

        for (size_t i = 0; i < data_size; i++) {
            tree_[n_ + i] = data[i];
        }

        for (size_t i = n_ - 1; i != 0; i--) {
            size_t l = 2 * i;
            size_t r = l | 1;
            tree_[i] = tree_[l] + tree_[r];
        }
    }

    /// @brief 
    /// @param i zero based index in the array
    /// @param upd_value
    void Update(size_t i, value_t upd_value) noexcept {
        assert(i < n_);
        i += n_;
        tree_[i] = upd_value;
        for (i /= 2; i != 0; i /= 2) {
            size_t l = 2 * i;
            size_t r = l | 1;
            tree_[i] = tree_[l] + tree_[r];
        }
    }

    /// @brief get on the data[l; r]
    /// @param l left index (including)
    /// @param r right index (including)
    /// @return 
    value_t Get(size_t l, size_t r) noexcept {
        assert(l <= r && r < n_);
        value_t res = 0;
        for (l += n_, r += n_; l <= r; l /= 2, r /= 2) {
            assert(l != 0);
            if (l % 2 != 0) {
                // l is right son
                res += tree_[l];
                l++;
            }

            if (r % 2 == 0) {
                // r is left son
                res += tree_[r];
                r--;
            }
        }

        return res;
    }

    ~SumSegTree() {
        operator delete(tree_);
        tree_ = nullptr;
    }
};

int main() {
    int arr[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    SumSegTree t(arr, sizeof(arr) / sizeof(arr[0]));
    t.Update(0, 2);
    assert(t.Get(0, 4) == 16);
}
