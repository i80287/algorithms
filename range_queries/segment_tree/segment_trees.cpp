// #define NDEBUG 1

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

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

namespace segtrees {

template <typename T>
static constexpr T bin_pow(T n, uint32_t p) noexcept {
    T res = 1;
    while (true) {
        if (p & 1) {
            res *= n;
        }
        p >>= 1;
        if (p == 0) {
            break;
        }
        n *= n;
    }

    return res;
}

#if __cplusplus >= 202002L
constexpr
#endif
static uint32_t tree_size(uint32_t n) noexcept {
#if __cplusplus >= 202002L
    const uint32_t lz_count = uint32_t(std::countl_zero(n | 1));
#else
    const uint32_t lz_count = uint32_t(__builtin_clz(n | 1));
#endif
    const uint32_t is_two_pow = uint32_t((n & (n - 1)) == 0);
    return 1u << (32 - lz_count - is_two_pow + 1);
}

template <typename value_t, GetOperation get_op>
class MinMaxSegTreeAdd {
protected:
    static_assert((get_op == GetOperation::max) ||
                  (get_op == GetOperation::min));
    struct Node {
        value_t value;
        value_t promise;
    };

    Node* nodes_;
    uint32_t query_l_{};
    uint32_t query_r_{};
    value_t value_{};
    uint32_t n_;
    uint32_t tree_size_;

public:
    explicit MinMaxSegTreeAdd(const std::vector<value_t>& data)
        : MinMaxSegTreeAdd(data.data(), uint32_t(data.size())) {}

    MinMaxSegTreeAdd(const value_t* data, uint32_t n)
        : n_{n}, tree_size_{tree_size(n)} {
        nodes_ = static_cast<Node*>(operator new(tree_size_ * sizeof(Node)));
        this->BuildRecImpl(data, 0, 0, n - 1);
    }

    void Update(uint32_t l, uint32_t r, value_t upd_value) noexcept {
        assert(l <= r && r < n_);
        query_l_ = l;
        query_r_ = r;
        value_ = upd_value;
        this->UpdateRecImpl(0, 0, n_ - 1);
    }

    value_t Get(uint32_t l, uint32_t r) noexcept {
        assert(l <= r && r < n_);
        return this->GetRecImpl(0, 0, n_ - 1, l, r);
    }

    ~MinMaxSegTreeAdd() {
        operator delete(nodes_);
        nodes_ = nullptr;
    }

protected:
    void BuildRecImpl(const value_t* data, size_t node_index, uint32_t node_l,
                      uint32_t node_r) noexcept {
        assert(node_index < tree_size_ && node_l <= node_r && node_r < n_);
        nodes_[node_index].promise = 0;
        if (node_l == node_r) {
            nodes_[node_index].value = data[node_l];
            return;
        }

        uint32_t node_m = (node_l + node_r) / 2;
        size_t left_node_index = node_index * 2 | 1;
        this->BuildRecImpl(data, left_node_index, node_l, node_m);
        size_t right_node_index = left_node_index + 1;
        this->BuildRecImpl(data, right_node_index, node_m + 1, node_r);
        assert(right_node_index < tree_size_);
        if constexpr (get_op == GetOperation::max) {
            nodes_[node_index].value = std::max(nodes_[left_node_index].value,
                                                nodes_[right_node_index].value);
        } else {
            nodes_[node_index].value = std::min(nodes_[left_node_index].value,
                                                nodes_[right_node_index].value);
        }
    }

    void UpdateRecImpl(size_t node_index, size_t node_l,
                       size_t node_r) noexcept {
        assert(node_index < tree_size_ && node_l <= node_r && node_r < n_);
        if (query_l_ <= node_l && node_r <= query_r_) {
            nodes_[node_index].promise += value_;
            return;
        }

        if (query_r_ < node_l || node_r < query_l_) {
            return;
        }

        size_t node_m = (node_l + node_r) / 2;
        this->PushImpl(node_index);
        size_t left_node_index = node_index * 2 | 1;
        size_t right_node_index = left_node_index + 1;
        assert(right_node_index < tree_size_);
        this->UpdateRecImpl(left_node_index, node_l, node_m);
        this->UpdateRecImpl(right_node_index, node_m + 1, node_r);
        const Node& left_node = nodes_[left_node_index];
        const Node& right_node = nodes_[right_node_index];
        if constexpr (get_op == GetOperation::max) {
            nodes_[node_index].value =
                std::max(left_node.value + left_node.promise,
                         right_node.value + right_node.promise);
        } else {
            nodes_[node_index].value =
                std::min(left_node.value + left_node.promise,
                         right_node.value + right_node.promise);
        }
    }

    void PushImpl(size_t node_index) noexcept {
        assert(node_index < tree_size_);
        size_t left_node_index = node_index * 2 | 1;
        size_t right_node_index = left_node_index + 1;
        assert(right_node_index < tree_size_);
        value_t this_promise = nodes_[node_index].promise;
        if (this_promise == 0) {
            // Not very rare case btw
            return;
        }
        nodes_[node_index].value += this_promise;
        nodes_[left_node_index].promise += this_promise;
        nodes_[right_node_index].promise += this_promise;
        nodes_[node_index].promise = 0;
    }

    value_t GetRecImpl(size_t node_index, uint32_t node_l, uint32_t node_r,
                       uint32_t query_l, uint32_t query_r) noexcept {
        assert(node_index < tree_size_);
        if (query_l == node_l && node_r == query_r) {
            const Node& node = nodes_[node_index];
            return node.value + node.promise;
        }

        uint32_t node_middle = (node_l + node_r) / 2;
        this->PushImpl(node_index);
        size_t left_son = node_index * 2 | 1;
        size_t right_son = left_son + 1;
        if (query_r <= node_middle) {
            return this->GetRecImpl(left_son, node_l, node_middle, query_l,
                                    query_r);
        }

        if (node_middle < query_l) {
            return this->GetRecImpl(right_son, node_middle + 1, node_r, query_l,
                                    query_r);
        }

        // query_l <= node_middle < query_r
        value_t left_result = this->GetRecImpl(left_son, node_l, node_middle,
                                               query_l, node_middle);
        value_t right_result = this->GetRecImpl(
            right_son, node_middle + 1, node_r, node_middle + 1, query_r);
        if constexpr (get_op == GetOperation::max) {
            return std::max(left_result, right_result);
        } else {
            return std::min(left_result, right_result);
        }
    }
};

template <typename value_t, GetOperation get_op>
class MinMaxSegTreeMult {
protected:
    static_assert((get_op == GetOperation::max) ||
                  (get_op == GetOperation::min));
    struct Node {
        value_t min_value;
        value_t max_value;
        value_t promise;
    };
    Node* nodes_;
    uint32_t query_l_{};
    uint32_t query_r_{};
    value_t value_{};
    uint32_t n_;
    uint32_t tree_size_;

public:
    explicit MinMaxSegTreeMult(const std::vector<value_t>& data)
        : MinMaxSegTreeMult(data.data(), uint32_t(data.size())) {}

    MinMaxSegTreeMult(const value_t* data, uint32_t n)
        : n_{n}, tree_size_{tree_size(n)} {
        nodes_ = static_cast<Node*>(operator new(tree_size_ * sizeof(Node)));
        this->BuildRecImpl(data, 0, 0, n - 1);
        if constexpr (std::is_integral_v<value_t>) {
            std::cerr << "\n[>>>] Warning: min / max segment tree with "
                         "multiplication on update should not be used with "
                         "integral type because of the type overflow!!!"
                         "\n[>>>] If you are sure of what you are doing, "
                         "remove this message in file "
                      << __FILE__ << " on line " << __LINE__ << "\n[>>>] "
                      << __FILE__ << ':' << __LINE__ << "\n\n";
            std::cerr.flush();
        }
    }

    void Update(uint32_t l, uint32_t r, value_t upd_value) noexcept {
        assert(l <= r && r < n_);
        query_l_ = l;
        query_r_ = r;
        value_ = upd_value;
        this->UpdateRecImpl(0, 0, n_ - 1);
    }

    value_t Get(uint32_t l, uint32_t r) noexcept {
        assert(l <= r && r < n_);
        return this->GetRecImpl(0, 0, n_ - 1, l, r);
    }

    ~MinMaxSegTreeMult() {
        operator delete(nodes_);
        nodes_ = nullptr;
    }

protected:
    void BuildRecImpl(const value_t* data, size_t node_index, uint32_t node_l,
                      uint32_t node_r) noexcept {
        assert(node_index < tree_size_ && node_l <= node_r && node_r < n_);
        nodes_[node_index].promise = 1;
        if (node_l == node_r) {
            nodes_[node_index].max_value = nodes_[node_index].min_value =
                data[node_l];
            return;
        }

        uint32_t node_m = (node_l + node_r) / 2;
        size_t left_node_index = node_index * 2 | 1;
        this->BuildRecImpl(data, left_node_index, node_l, node_m);
        size_t right_node_index = left_node_index + 1;
        this->BuildRecImpl(data, right_node_index, node_m + 1, node_r);
        assert(right_node_index < tree_size_);
        const Node& left_node = nodes_[left_node_index];
        const Node& right_node = nodes_[right_node_index];
        nodes_[node_index].min_value =
            std::min(left_node.min_value, right_node.min_value);
        nodes_[node_index].max_value =
            std::max(left_node.max_value, right_node.max_value);
    }

    void UpdateRecImpl(size_t node_index, uint32_t node_l,
                       uint32_t node_r) noexcept {
        assert(node_index < tree_size_);
        if (query_l_ <= node_l && node_r <= query_r_) {
            nodes_[node_index].promise *= value_;
            return;
        }

        if (query_r_ < node_l || node_r < query_l_) {
            return;
        }

        uint32_t node_m = (node_l + node_r) / 2;
        this->PushImpl(node_index);
        size_t left_node_index = node_index * 2 | 1;
        this->UpdateRecImpl(left_node_index, node_l, node_m);
        size_t right_node_index = left_node_index + 1;
        this->UpdateRecImpl(right_node_index, node_m + 1, node_r);
        const Node& left_node = nodes_[left_node_index];
        const Node& right_node = nodes_[right_node_index];
        value_t left_node_promise = left_node.promise;
        value_t right_node_promise = right_node.promise;
        nodes_[node_index].min_value =
            std::min((left_node_promise >= 0 ? left_node.min_value
                                             : left_node.max_value) *
                         left_node_promise,
                     (right_node_promise >= 0 ? right_node.min_value
                                              : right_node.max_value) *
                         right_node_promise);
        nodes_[node_index].max_value =
            std::max((left_node_promise >= 0 ? left_node.max_value
                                             : left_node.min_value) *
                         left_node_promise,
                     (right_node_promise >= 0 ? right_node.max_value
                                              : right_node.min_value) *
                         right_node_promise);
    }

    void PushImpl(size_t node_index) noexcept {
        assert(node_index < tree_size_);
        Node& node = nodes_[node_index];
        value_t this_promise = node.promise;
        if (this_promise == 1) {
            return;
        }

        if (this_promise >= 0) {
            node.max_value *= this_promise;
            node.min_value *= this_promise;
        } else {
            value_t new_min_value = node.max_value * this_promise;
            node.max_value = node.min_value * this_promise;
            node.min_value = new_min_value;
        }
        node.promise = 1;
        size_t left_node_index = node_index * 2 | 1;
        nodes_[left_node_index].promise *= this_promise;
        size_t right_node_index = left_node_index + 1;
        nodes_[right_node_index].promise *= this_promise;
    }

    value_t GetRecImpl(size_t node_index, uint32_t node_l, uint32_t node_r,
                       uint32_t query_l, uint32_t query_r) noexcept {
        assert(node_index < tree_size_);
        if (query_l == node_l && node_r == query_r) {
            const Node& node = nodes_[node_index];
            value_t t1 = node.min_value * node.promise;
            value_t t2 = node.max_value * node.promise;
            if constexpr (get_op == GetOperation::max) {
                return std::max(t1, t2);
            } else {
                return std::min(t1, t2);
            }
        }

        uint32_t node_middle = (node_l + node_r) / 2;
        this->PushImpl(node_index);
        size_t left_son = node_index * 2 | 1;
        size_t right_son = left_son + 1;
        if (query_r <= node_middle) {
            return this->GetRecImpl(left_son, node_l, node_middle, query_l,
                                    query_r);
        }

        if (node_middle < query_l) {
            return this->GetRecImpl(right_son, node_middle + 1, node_r, query_l,
                                    query_r);
        }

        // query_l <= node_middle < query_r
        value_t left_result = this->GetRecImpl(left_son, node_l, node_middle,
                                               query_l, node_middle);
        value_t right_result = this->GetRecImpl(
            right_son, node_middle + 1, node_r, node_middle + 1, query_r);
        if constexpr (get_op == GetOperation::max) {
            return std::max(left_result, right_result);
        } else {
            return std::min(left_result, right_result);
        }
    }
};

template <typename value_t, GetOperation get_op>
class MinMaxSegTreeSetEqual {
protected:
    static_assert((get_op == GetOperation::max) ||
                  (get_op == GetOperation::min));
    struct Node {
        value_t value;
        value_t promise;
        uint32_t has_promise;
    };

    Node* nodes_;
    uint32_t query_l_{};
    uint32_t query_r_{};
    value_t value_{};
    uint32_t n_;
    uint32_t tree_size_;

public:
    explicit MinMaxSegTreeSetEqual(const std::vector<value_t>& data)
        : MinMaxSegTreeSetEqual(data.data(), uint32_t(data.size())) {}

    MinMaxSegTreeSetEqual(const value_t* data, uint32_t n)
        : n_{n}, tree_size_{tree_size(n)} {
        nodes_ = static_cast<Node*>(operator new(tree_size_ * sizeof(Node)));
        this->BuildRecImpl(data, 0, 0, n - 1);
    }

    void Update(uint32_t l, uint32_t r, value_t upd_value) noexcept {
        assert(l <= r && r < n_);
        query_l_ = l;
        query_r_ = r;
        value_ = upd_value;
        this->UpdateRecImpl(0, 0, n_ - 1);
    }

    value_t Get(uint32_t l, uint32_t r) noexcept {
        assert(l <= r && r < n_);
        return this->GetRecImpl(0, 0, n_ - 1, l, r);
    }

    ~MinMaxSegTreeSetEqual() {
        operator delete(nodes_);
        nodes_ = nullptr;
    }

protected:
    void BuildRecImpl(const value_t* data, size_t node_index, uint32_t node_l,
                      uint32_t node_r) noexcept {
        assert(node_index < tree_size_ && node_l <= node_r && node_r < n_);
        nodes_[node_index].has_promise = false;

        if (node_l == node_r) {
            nodes_[node_index].value = data[node_l];
            return;
        }

        uint32_t node_m = (node_l + node_r) / 2;
        size_t left_son = node_index * 2 | 1;
        this->BuildRecImpl(data, left_son, node_l, node_m);
        size_t right_son = left_son + 1;
        this->BuildRecImpl(data, right_son, node_m + 1, node_r);
        assert(right_son < tree_size_);
        if constexpr (get_op == GetOperation::max) {
            nodes_[node_index].value =
                std::max(nodes_[left_son].value, nodes_[right_son].value);
        } else {
            nodes_[node_index].value =
                std::min(nodes_[left_son].value, nodes_[right_son].value);
        }
    }

    void UpdateRecImpl(size_t node_index, uint32_t node_l,
                       uint32_t node_r) noexcept {
        if (query_l_ <= node_l && node_r <= query_r_) {
            nodes_[node_index].promise = value_;
            nodes_[node_index].has_promise = true;
            return;
        }

        if (query_r_ < node_l || node_r < query_l_) {
            return;
        }

        uint32_t node_m = (node_l + node_r) / 2;
        this->PushImpl(node_index);
        size_t left_node = node_index * 2 | 1;
        size_t right_node = left_node + 1;
        this->UpdateRecImpl(left_node, node_l, node_m);
        this->UpdateRecImpl(right_node, node_m + 1, node_r);

        if constexpr (get_op == GetOperation::max) {
            nodes_[node_index].value = std::max(
                nodes_[left_node].has_promise ? nodes_[left_node].promise
                                              : nodes_[left_node].value,
                nodes_[right_node].has_promise ? nodes_[right_node].promise
                                               : nodes_[right_node].value);
        } else {
            nodes_[node_index].value = std::min(
                nodes_[left_node].has_promise ? nodes_[left_node].promise
                                              : nodes_[left_node].value,
                nodes_[right_node].has_promise ? nodes_[right_node].promise
                                               : nodes_[right_node].value);
        }
    }

    void PushImpl(size_t node_index) noexcept {
        assert(node_index < tree_size_);
        if (!nodes_[node_index].has_promise) {
            return;
        }

        value_t this_promise = nodes_[node_index].promise;
        size_t left_node = node_index * 2 | 1;
        size_t right_node = left_node + 1;
        nodes_[node_index].value = this_promise;
        nodes_[node_index].has_promise = false;
        nodes_[left_node].promise = this_promise;
        nodes_[left_node].has_promise = true;
        nodes_[right_node].promise = this_promise;
        nodes_[right_node].has_promise = true;
    }

    value_t GetRecImpl(size_t node_index, uint32_t node_l, uint32_t node_r,
                       uint32_t query_l, uint32_t query_r) noexcept {
        assert(node_index < tree_size_);
        if (query_l == node_l && node_r == query_r) {
            return nodes_[node_index].has_promise ? nodes_[node_index].promise
                                                  : nodes_[node_index].value;
        }

        uint32_t node_middle = (node_l + node_r) / 2;
        this->PushImpl(node_index);
        size_t left_son = node_index * 2 | 1;
        size_t right_son = left_son + 1;
        if (query_r <= node_middle) {
            return this->GetRecImpl(left_son, node_l, node_middle, query_l,
                                    query_r);
        }

        if (node_middle < query_l) {
            return this->GetRecImpl(right_son, node_middle + 1, node_r, query_l,
                                    query_r);
        }

        // query_l <= node_middle < query_r
        value_t left_result = this->GetRecImpl(left_son, node_l, node_middle,
                                               query_l, node_middle);
        value_t right_result = this->GetRecImpl(
            right_son, node_middle + 1, node_r, node_middle + 1, query_r);
        if constexpr (get_op == GetOperation::max) {
            return std::max(left_result, right_result);
        } else {
            return std::min(left_result, right_result);
        }
    }
};

template <typename value_t>
class SumSegTreeSetEqual {
protected:
    struct Node {
        value_t value;
        value_t promise;
        uint32_t has_promise;
    };
    Node* nodes_;
    uint32_t query_l_{};
    uint32_t query_r_{};
    value_t value_{};
    uint32_t n_;
    uint32_t tree_size_;

public:
    explicit SumSegTreeSetEqual(const std::vector<value_t>& data)
        : SumSegTreeSetEqual(data.data(), uint32_t(data.size())) {}

    SumSegTreeSetEqual(const value_t* data, uint32_t n)
        : n_{n}, tree_size_{tree_size(n)} {
        nodes_ = static_cast<Node*>(operator new(tree_size_ * sizeof(Node)));
        this->BuildRecImpl(data, 0, 0, n - 1);
    }

    void Update(uint32_t l, uint32_t r, value_t upd_value) noexcept {
        assert(l <= r && r < n_);
        query_l_ = l;
        query_r_ = r;
        value_ = upd_value;
        this->UpdateRecImpl(0, 0, n_ - 1);
    }

    value_t Get(uint32_t l, uint32_t r) noexcept {
        assert(l <= r && r < n_);
        return this->GetRecImpl(0, 0, n_ - 1, l, r);
    }

    ~SumSegTreeSetEqual() {
        operator delete(nodes_);
        nodes_ = nullptr;
    }

protected:
    void BuildRecImpl(const value_t* data, size_t node_index, uint32_t node_l,
                      uint32_t node_r) noexcept {
        assert(node_index < tree_size_ && node_l <= node_r && node_r < n_);
        nodes_[node_index].has_promise = false;
        if (node_l == node_r) {
            nodes_[node_index].value = data[node_l];
            return;
        }

        uint32_t node_m = (node_l + node_r) / 2;
        size_t left_son = node_index * 2 | 1;
        this->BuildRecImpl(data, left_son, node_l, node_m);
        size_t right_son = left_son + 1;
        this->BuildRecImpl(data, right_son, node_m + 1, node_r);
        assert(right_son < tree_size_);
        nodes_[node_index].value =
            nodes_[left_son].value + nodes_[right_son].value;
    }

    void UpdateRecImpl(size_t node_index, uint32_t node_l,
                       uint32_t node_r) noexcept {
        assert(node_index < tree_size_ && node_l <= node_r && node_r < n_);
        if (query_l_ <= node_l && node_r <= query_r_) {
            nodes_[node_index].promise = value_;
            nodes_[node_index].has_promise = true;
            return;
        }

        if (query_r_ < node_l || node_r < query_l_) {
            return;
        }

        uint32_t node_m = (node_l + node_r) / 2;
        this->PushImpl(node_index, node_l, node_r);
        size_t left_node_index = node_index * 2 | 1;
        size_t right_node_index = left_node_index + 1;
        this->UpdateRecImpl(left_node_index, node_l, node_m);
        this->UpdateRecImpl(right_node_index, node_m + 1, node_r);
        assert(right_node_index < tree_size_);
        const Node& left_node = nodes_[left_node_index];
        const Node& right_node = nodes_[right_node_index];
        value_t left_value = left_node.has_promise
                                 ? (left_node.promise * (node_m - node_l + 1))
                                 : left_node.value;
        value_t right_value = right_node.has_promise
                                  ? (right_node.promise * (node_r - node_m))
                                  : right_node.value;
        nodes_[node_index].value = left_value + right_value;
    }

    void PushImpl(size_t node_index, uint32_t node_l,
                  uint32_t node_r) noexcept {
        assert(node_index < tree_size_);
        size_t left_node = node_index * 2 | 1;
        size_t right_node = left_node + 1;
        assert(right_node < tree_size_);
        Node& node = nodes_[node_index];
        if (!node.has_promise) {
            return;
        }

        value_t this_promise = node.promise;
        node.value = this_promise * (node_r - node_l + 1);
        node.has_promise = false;
        nodes_[left_node].promise = this_promise;
        nodes_[left_node].has_promise = true;
        nodes_[right_node].promise = this_promise;
        nodes_[right_node].has_promise = true;
    }

    value_t GetRecImpl(size_t node_index, uint32_t node_l, uint32_t node_r,
                       uint32_t query_l, uint32_t query_r) noexcept {
        assert(node_index < tree_size_);
        if (query_l == node_l && node_r == query_r) {
            const Node& node = nodes_[node_index];
            return node.has_promise ? (node.promise * (node_r - node_l + 1))
                                    : node.value;
        }

        uint32_t node_middle = (node_l + node_r) / 2;
        this->PushImpl(node_index, node_l, node_r);
        size_t left_son = node_index * 2 | 1;
        size_t right_son = left_son + 1;
        if (query_r <= node_middle) {
            return this->GetRecImpl(left_son, node_l, node_middle, query_l,
                                    query_r);
        }

        if (node_middle < query_l) {
            return this->GetRecImpl(right_son, node_middle + 1, node_r, query_l,
                                    query_r);
        }

        // query_l <= node_middle < query_r
        value_t left_result = this->GetRecImpl(left_son, node_l, node_middle,
                                               query_l, node_middle);
        value_t right_result = this->GetRecImpl(
            right_son, node_middle + 1, node_r, node_middle + 1, query_r);
        return left_result + right_result;
    }
};

template <typename value_t>
class ProdSegTreeSetEqual {
protected:
    struct Node {
        value_t value;
        value_t promise;
        uint32_t has_promise;
        value_t cached_promise_x_count;
        uint32_t has_cached_promise_x_count;
    };
    Node* nodes_;
    uint32_t query_l_{};
    uint32_t query_r_{};
    value_t value_{};
    uint32_t n_;
    uint32_t tree_size_;

public:
    explicit ProdSegTreeSetEqual(const std::vector<value_t>& data)
        : ProdSegTreeSetEqual(data.data(), uint32_t(data.size())) {}

    ProdSegTreeSetEqual(const value_t* data, uint32_t n)
        : n_{n}, tree_size_{tree_size(n)} {
        nodes_ = static_cast<Node*>(operator new(tree_size_ * sizeof(Node)));
        this->BuildRecImpl(data, 0, 0, n - 1);
    }

    void BuildRecImpl(const value_t* data, size_t node_index, uint32_t node_l,
                      uint32_t node_r) noexcept {
        assert(node_index < tree_size_ && node_l <= node_r && node_r < n_);
        nodes_[node_index].has_promise = false;
        nodes_[node_index].has_cached_promise_x_count = false;
        if (node_l == node_r) {
            nodes_[node_index].value = data[node_l];
            return;
        }

        uint32_t node_m = (node_l + node_r) / 2;
        size_t left_son = node_index * 2 | 1;
        this->BuildRecImpl(data, left_son, node_l, node_m);
        size_t right_son = left_son + 1;
        this->BuildRecImpl(data, right_son, node_m + 1, node_r);
        assert(right_son < tree_size_);
        nodes_[node_index].value =
            nodes_[left_son].value * nodes_[right_son].value;
    }

    void Update(uint32_t l, uint32_t r, value_t upd_value) noexcept {
        assert(l <= r && r < n_);
        query_l_ = l;
        query_r_ = r;
        value_ = upd_value;
        this->UpdateRecImpl(0, 0, n_ - 1);
    }

    value_t Get(uint32_t l, uint32_t r) noexcept {
        assert(l <= r && r < n_);
        return this->GetRecImpl(0, 0, n_ - 1, l, r);
    }

    ~ProdSegTreeSetEqual() {
        operator delete(nodes_);
        nodes_ = nullptr;
    }

protected:
    void UpdateRecImpl(size_t node_index, uint32_t node_l,
                       uint32_t node_r) noexcept {
        assert(node_index < tree_size_ && node_l <= node_r && node_r < n_);
        if (query_l_ <= node_l && node_r <= query_r_) {
            Node& node = nodes_[node_index];
            node.promise = value_;
            node.has_promise = true;
            node.has_cached_promise_x_count = false;
            return;
        }

        if (query_r_ < node_l || node_r < query_l_) {
            return;
        }

        uint32_t node_m = (node_l + node_r) / 2;
        this->PushImpl(node_index, node_l, node_r);
        size_t left_node_index = node_index * 2 | 1;
        size_t right_node_index = left_node_index + 1;
        this->UpdateRecImpl(left_node_index, node_l, node_m);
        this->UpdateRecImpl(right_node_index, node_m + 1, node_r);
        assert(right_node_index < tree_size_);
        Node& left_node = nodes_[left_node_index];
        Node& right_node = nodes_[right_node_index];
        value_t left_value;
        //  = left_node.has_promise ? (left_node.promise * (node_m - node_l +
        //  1)) : left_node.value;
        if (left_node.has_promise) {
            if (!left_node.has_cached_promise_x_count) {
                left_node.cached_promise_x_count =
                    bin_pow(left_node.promise, node_m - node_l + 1);
                left_node.has_cached_promise_x_count = true;
            }
            left_value = left_node.cached_promise_x_count;
        } else {
            left_value = left_node.value;
        }
        value_t right_value;
        if (right_node.has_promise) {
            if (!right_node.has_cached_promise_x_count) {
                right_node.cached_promise_x_count =
                    bin_pow(right_node.promise, node_r - node_m);
                right_node.has_cached_promise_x_count = true;
            }
            right_value = right_node.cached_promise_x_count;
        } else {
            right_value = right_node.value;
        }
        nodes_[node_index].value = left_value * right_value;
    }

    void PushImpl(size_t node_index, uint32_t node_l,
                  uint32_t node_r) noexcept {
        assert(node_index < tree_size_);
        size_t left_node_index = node_index * 2 | 1;
        size_t right_node_index = left_node_index + 1;
        assert(right_node_index < tree_size_);
        Node& node = nodes_[node_index];
        if (!node.has_promise) {
            return;
        }

        Node& left_node = nodes_[left_node_index];
        Node& right_node = nodes_[right_node_index];
        value_t this_promise = node.promise;
        node.value = node.has_cached_promise_x_count
                         ? node.cached_promise_x_count
                         : bin_pow(this_promise, node_r - node_l + 1);
        node.has_promise = false;
        node.has_cached_promise_x_count = false;
        left_node.promise = this_promise;
        left_node.has_promise = true;
        left_node.has_cached_promise_x_count = false;
        right_node.promise = this_promise;
        right_node.has_promise = true;
        right_node.has_cached_promise_x_count = false;
    }

    value_t GetRecImpl(size_t node_index, uint32_t node_l, uint32_t node_r,
                       uint32_t query_l, uint32_t query_r) noexcept {
        assert(node_index < tree_size_);
        if (query_l == node_l && node_r == query_r) {
            Node& node = nodes_[node_index];
            if (node.has_promise) {
                if (!node.has_cached_promise_x_count) {
                    node.has_cached_promise_x_count = true;
                    node.cached_promise_x_count =
                        bin_pow(node.promise, node_r - node_l + 1);
                }
                return node.cached_promise_x_count;
            } else {
                return node.value;
            }
        }

        uint32_t node_middle = (node_l + node_r) / 2;
        this->PushImpl(node_index, node_l, node_r);
        size_t left_son = node_index * 2 | 1;
        size_t right_son = left_son + 1;
        if (query_r <= node_middle) {
            return this->GetRecImpl(left_son, node_l, node_middle, query_l,
                                    query_r);
        }

        if (node_middle < query_l) {
            return this->GetRecImpl(right_son, node_middle + 1, node_r, query_l,
                                    query_r);
        }

        // query_l <= node_middle < query_r
        value_t left_result = this->GetRecImpl(left_son, node_l, node_middle,
                                               query_l, node_middle);
        value_t right_result = this->GetRecImpl(
            right_son, node_middle + 1, node_r, node_middle + 1, query_r);
        return left_result * right_result;
    }
};

template <typename value_t>
class SumSegTreeMult {
protected:
    struct Node {
        value_t value;
        value_t promise;
    };

    Node* nodes_;
    uint32_t query_l_{};
    uint32_t query_r_{};
    value_t value_{};
    uint32_t n_;
    uint32_t tree_size_;

public:
    explicit SumSegTreeMult(const std::vector<value_t>& data)
        : SumSegTreeMult(data.data(), uint32_t(data.size())) {}

    SumSegTreeMult(const value_t* data, uint32_t n)
        : n_{n}, tree_size_{tree_size(n)} {
        nodes_ = static_cast<Node*>(operator new(tree_size_ * sizeof(Node)));
        this->BuildRecImpl(data, 0, 0, n - 1);
    }

    void Update(uint32_t l, uint32_t r, value_t upd_value) noexcept {
        assert(l <= r && r < n_);
        query_l_ = l;
        query_r_ = r;
        value_ = upd_value;
        this->UpdateRecImpl(0, 0, n_ - 1);
    }

    value_t Get(uint32_t l, uint32_t r) noexcept {
        assert(l <= r && r < n_);
        return this->GetRecImpl(0, 0, n_ - 1, l, r);
    }

    ~SumSegTreeMult() {
        operator delete(nodes_);
        nodes_ = nullptr;
    }

protected:
    void BuildRecImpl(const value_t* data, size_t node_index, uint32_t node_l,
                      uint32_t node_r) noexcept {
        assert(node_index < tree_size_ && node_l <= node_r && node_r < n_);
        nodes_[node_index].promise = 1;
        if (node_l == node_r) {
            nodes_[node_index].value = data[node_l];
            return;
        }

        uint32_t node_m = (node_l + node_r) / 2;
        size_t left_son = node_index * 2 | 1;
        this->BuildRecImpl(data, left_son, node_l, node_m);
        size_t right_son = left_son + 1;
        this->BuildRecImpl(data, right_son, node_m + 1, node_r);
        assert(right_son < tree_size_);
        nodes_[node_index].value =
            nodes_[left_son].value + nodes_[right_son].value;
    }

    void UpdateRecImpl(size_t node_index, uint32_t node_l,
                       uint32_t node_r) noexcept {
        assert(node_index < tree_size_ && node_l <= node_r && node_r < n_);
        if (query_l_ <= node_l && node_r <= query_r_) {
            nodes_[node_index].promise *= value_;
            return;
        }

        if (query_r_ < node_l || node_r < query_l_) {
            return;
        }

        uint32_t node_m = (node_l + node_r) / 2;
        this->PushImpl(node_index);
        size_t left_node = node_index * 2 | 1;
        size_t right_node = left_node + 1;
        this->UpdateRecImpl(left_node, node_l, node_m);
        this->UpdateRecImpl(right_node, node_m + 1, node_r);
        assert(right_node < tree_size_);
        nodes_[node_index].value =
            nodes_[left_node].value * nodes_[left_node].promise +
            nodes_[right_node].value * nodes_[right_node].promise;
    }

    void PushImpl(size_t node_index) noexcept {
        assert(node_index < tree_size_);
        value_t this_promise = nodes_[node_index].promise;
        // if (__builtin_expect_with_probability(this_promise == 1, 1, 0.25)) {
        //     return;
        // }

        size_t left_node = node_index * 2 | 1;
        size_t right_node = left_node + 1;
        assert(right_node < tree_size_);
        nodes_[node_index].value *= this_promise;
        nodes_[left_node].promise *= this_promise;
        nodes_[right_node].promise *= this_promise;
        nodes_[node_index].promise = 1;
    }

    value_t GetRecImpl(size_t node_index, uint32_t node_l, uint32_t node_r,
                       uint32_t query_l, uint32_t query_r) noexcept {
        assert(node_index < tree_size_);
        if (query_l == node_l && node_r == query_r) {
            return nodes_[node_index].value * nodes_[node_index].promise;
        }

        uint32_t node_middle = (node_l + node_r) / 2;
        this->PushImpl(node_index);
        size_t left_son = node_index * 2 | 1;
        size_t right_son = left_son + 1;
        if (query_r <= node_middle) {
            return this->GetRecImpl(left_son, node_l, node_middle, query_l,
                                    query_r);
        }

        if (node_middle < query_l) {
            return this->GetRecImpl(right_son, node_middle + 1, node_r, query_l,
                                    query_r);
        }

        // query_l <= node_middle < query_r
        value_t left_result = this->GetRecImpl(left_son, node_l, node_middle,
                                               query_l, node_middle);
        value_t right_result = this->GetRecImpl(
            right_son, node_middle + 1, node_r, node_middle + 1, query_r);
        return left_result + right_result;
    }
};

template <typename value_t>
class SumSegTreeAdd {
protected:
    struct Node {
        value_t value;
        value_t promise;
    };
    Node* nodes_;
    uint32_t query_l_{};
    uint32_t query_r_{};
    value_t value_{};
    uint32_t n_;
    uint32_t tree_size_;

public:
    explicit SumSegTreeAdd(const std::vector<value_t>& data)
        : SumSegTreeAdd(data.data(), uint32_t(data.size())) {}

    SumSegTreeAdd(const value_t* data, uint32_t n)
        : n_{n}, tree_size_{tree_size(n)} {
        nodes_ = static_cast<Node*>(operator new(tree_size_ * sizeof(Node)));
        this->BuildRecImpl(data, 0, 0, n - 1);
    }

    void Update(uint32_t l, uint32_t r, value_t upd_value) noexcept {
        assert(l <= r && r < n_);
        query_l_ = l;
        query_r_ = r;
        value_ = upd_value;
        this->UpdateRecImpl(0, 0, n_ - 1);
    }

    value_t Get(uint32_t l, uint32_t r) {
        assert(l <= r && r < n_);
        return this->GetRecImpl(0, 0, n_ - 1, l, r);
    }

    ~SumSegTreeAdd() {
        operator delete(nodes_);
        nodes_ = nullptr;
    }

protected:
    void BuildRecImpl(const value_t* data, size_t node_index, uint32_t node_l,
                      uint32_t node_r) noexcept {
        assert(node_index < tree_size_ && node_l <= node_r && node_r < n_);
        nodes_[node_index].promise = 0;
        if (node_l == node_r) {
            nodes_[node_index].value = data[node_l];
            return;
        }

        uint32_t node_m = (node_l + node_r) / 2;
        size_t left_son = node_index * 2 | 1;
        this->BuildRecImpl(data, left_son, node_l, node_m);
        size_t right_son = left_son + 1;
        this->BuildRecImpl(data, right_son, node_m + 1, node_r);
        assert(right_son < tree_size_);
        nodes_[node_index].value =
            nodes_[left_son].value + nodes_[right_son].value;
    }

    void UpdateRecImpl(size_t node_index, uint32_t node_l,
                       uint32_t node_r) noexcept {
        assert(node_index < tree_size_ && node_l <= node_r && node_r < n_);
        if (query_l_ <= node_l && node_r <= query_r_) {
            nodes_[node_index].promise += value_;
            return;
        }

        if (query_r_ < node_l || node_r < query_l_) {
            return;
        }

        uint32_t node_m = (node_l + node_r) / 2;
        this->PushImpl(node_index, node_l, node_r);
        size_t left_node_index = node_index * 2 | 1;
        size_t right_node_index = left_node_index + 1;
        this->UpdateRecImpl(left_node_index, node_l, node_m);
        this->UpdateRecImpl(right_node_index, node_m + 1, node_r);
        assert(right_node_index < tree_size_);
        const Node& left_node = nodes_[left_node_index];
        const Node& right_node = nodes_[right_node_index];
        nodes_[node_index].value =
            left_node.value + left_node.promise * value_t(node_m - node_l + 1) +
            right_node.value + right_node.promise * value_t(node_r - node_m);
    }

    void PushImpl(size_t node_index, uint32_t node_l,
                  uint32_t node_r) noexcept {
        assert(node_index < tree_size_);
        size_t left_node = node_index * 2 | 1;
        size_t right_node = left_node + 1;
        assert(right_node < tree_size_);
        value_t this_promise = nodes_[node_index].promise;
        if (this_promise == 0) {
            return;
        }
        nodes_[node_index].value += this_promise * (node_r - node_l + 1);
        nodes_[left_node].promise += this_promise;
        nodes_[right_node].promise += this_promise;
        nodes_[node_index].promise = 0;
    }

    value_t GetRecImpl(size_t node_index, uint32_t node_l, uint32_t node_r,
                       uint32_t query_l, uint32_t query_r) noexcept {
        assert(node_index < tree_size_);
        if (query_l == node_l && node_r == query_r) {
            const Node& node = nodes_[node_index];
            return node.value + node.promise * value_t(node_r - node_l + 1);
        }

        uint32_t node_middle = (node_l + node_r) / 2;
        this->PushImpl(node_index, node_l, node_r);
        size_t left_son = node_index * 2 | 1;
        size_t right_son = left_son + 1;
        if (query_r <= node_middle) {
            return this->GetRecImpl(left_son, node_l, node_middle, query_l,
                                    query_r);
        }

        if (node_middle < query_l) {
            return this->GetRecImpl(right_son, node_middle + 1, node_r, query_l,
                                    query_r);
        }

        // query_l <= node_middle < query_r
        value_t left_result = this->GetRecImpl(left_son, node_l, node_middle,
                                               query_l, node_middle);
        value_t right_result = this->GetRecImpl(
            right_son, node_middle + 1, node_r, node_middle + 1, query_r);
        return left_result + right_result;
    }
};

template <typename value_t>
class ProdSegTreeMult {
protected:
    struct Node {
        value_t value;
        value_t promise;
        value_t cached_promise_x_count;
        uint32_t has_cached_promise_x_count;
    };
    Node* nodes_;
    uint32_t query_l_{};
    uint32_t query_r_{};
    value_t value_{};
    uint32_t n_;
    uint32_t tree_size_;

public:
    explicit ProdSegTreeMult(const std::vector<value_t>& data)
        : ProdSegTreeMult(data.data(), uint32_t(data.size())) {}

    ProdSegTreeMult(const value_t* data, uint32_t n)
        : n_{n}, tree_size_{tree_size(n)} {
        nodes_ = static_cast<Node*>(operator new(tree_size_ * sizeof(Node)));
        this->BuildRecImpl(data, 0, 0, n - 1);
    }

    void Update(uint32_t l, uint32_t r, value_t upd_value) noexcept {
        assert(l <= r && r < n_);
        query_l_ = l;
        query_r_ = r;
        value_ = upd_value;
        this->UpdateRecImpl(0, 0, n_ - 1);
    }

    value_t Get(uint32_t l, uint32_t r) noexcept {
        assert(l <= r && r < n_);
        return this->GetRecImpl(0, 0, n_ - 1, l, r);
    }

    ~ProdSegTreeMult() {
        operator delete(nodes_);
        nodes_ = nullptr;
    }

protected:
    void BuildRecImpl(const value_t* data, size_t node_index, uint32_t node_l,
                      uint32_t node_r) noexcept {
        assert(node_index < tree_size_ && node_l <= node_r && node_r < n_);
        Node& node = nodes_[node_index];
        node.promise = 1;
        node.cached_promise_x_count = 1;
        node.has_cached_promise_x_count = true;
        if (node_l == node_r) {
            node.value = data[node_l];
            return;
        }

        uint32_t node_m = (node_l + node_r) / 2;
        size_t left_son = node_index * 2 | 1;
        this->BuildRecImpl(data, left_son, node_l, node_m);
        size_t right_son = left_son + 1;
        this->BuildRecImpl(data, right_son, node_m + 1, node_r);
        assert(right_son < tree_size_);
        node.value = nodes_[left_son].value * nodes_[right_son].value;
    }

    void UpdateRecImpl(size_t node_index, uint32_t node_l,
                       uint32_t node_r) noexcept {
        assert(node_index < tree_size_ && node_l <= node_r && node_r < n_);
        if (query_l_ <= node_l && node_r <= query_r_) {
            Node& node = nodes_[node_index];
            node.promise *= value_;
            node.has_cached_promise_x_count = false;
            return;
        }

        if (query_r_ < node_l || node_r < query_l_) {
            return;
        }

        uint32_t node_m = (node_l + node_r) / 2;
        this->PushImpl(node_index, node_l, node_r);
        size_t left_node_index = node_index * 2 | 1;
        size_t right_node_index = left_node_index + 1;
        this->UpdateRecImpl(left_node_index, node_l, node_m);
        this->UpdateRecImpl(right_node_index, node_m + 1, node_r);
        assert(right_node_index < tree_size_);
        Node& left_node = nodes_[left_node_index];
        if (!left_node.has_cached_promise_x_count) {
            left_node.cached_promise_x_count =
                bin_pow(left_node.promise, node_m - node_l + 1);
            left_node.has_cached_promise_x_count = true;
        }
        Node& right_node = nodes_[right_node_index];
        if (!right_node.has_cached_promise_x_count) {
            right_node.cached_promise_x_count =
                bin_pow(right_node.promise, node_r - node_m);
            right_node.has_cached_promise_x_count = true;
        }
        nodes_[node_index].value =
            left_node.value * left_node.cached_promise_x_count *
            right_node.value * right_node.cached_promise_x_count;
    }

    void PushImpl(size_t node_index, uint32_t node_l,
                  uint32_t node_r) noexcept {
        assert(node_index < tree_size_);
        size_t left_node_index = node_index * 2 | 1;
        size_t right_node_index = left_node_index + 1;
        assert(right_node_index < tree_size_);
        Node& node = nodes_[node_index];
        value_t this_promise = node.promise;
        if (this_promise == 1) {
            return;
        }

        node.value *= node.has_cached_promise_x_count
                          ? node.cached_promise_x_count
                          : bin_pow(this_promise, node_r - node_l + 1);
        ;
        node.promise = 1;
        node.cached_promise_x_count = 1;
        node.has_cached_promise_x_count = true;

        nodes_[left_node_index].promise *= this_promise;
        nodes_[left_node_index].has_cached_promise_x_count = false;
        nodes_[right_node_index].promise *= this_promise;
        nodes_[right_node_index].has_cached_promise_x_count = false;
    }

    value_t GetRecImpl(size_t node_index, uint32_t node_l, uint32_t node_r,
                       uint32_t query_l, uint32_t query_r) noexcept {
        assert(node_index < tree_size_);
        if (query_l == node_l && node_r == query_r) {
            Node& node = nodes_[node_index];
            if (!node.has_cached_promise_x_count) {
                node.cached_promise_x_count =
                    bin_pow(node.promise, node_r - node_l + 1);
                node.has_cached_promise_x_count = true;
            }
            return node.value * node.cached_promise_x_count;
        }

        uint32_t node_middle = (node_l + node_r) / 2;
        this->PushImpl(node_index, node_l, node_r);
        size_t left_son = node_index * 2 | 1;
        size_t right_son = left_son + 1;
        if (query_r <= node_middle) {
            return this->GetRecImpl(left_son, node_l, node_middle, query_l,
                                    query_r);
        }

        if (node_middle < query_l) {
            return this->GetRecImpl(right_son, node_middle + 1, node_r, query_l,
                                    query_r);
        }

        // query_l <= node_middle < query_r
        value_t left_result = this->GetRecImpl(left_son, node_l, node_middle,
                                               query_l, node_middle);
        value_t right_result = this->GetRecImpl(
            right_son, node_middle + 1, node_r, node_middle + 1, query_r);
        return left_result * right_result;
    }
};

template <UpdateOperation upd_op, GetOperation get_op,
          typename value_t = int64_t>
class SegTreeChecker {
protected:
    value_t* values_;
    uint32_t n_;

public:
    explicit SegTreeChecker(const std::vector<value_t>& data)
        : SegTreeChecker(data.data(), uint32_t(data.size())) {}

    SegTreeChecker(const value_t* data, uint32_t n) : n_{n} {
        values_ = static_cast<value_t*>(operator new(n * sizeof(value_t)));
        memcpy(values_, data, n * sizeof(value_t));
    }

    void Update(uint32_t l, uint32_t r, value_t value) {
        assert(l <= r && r < n_);
        for (size_t i = l; i <= r; i++) {
            if constexpr (upd_op == UpdateOperation::add) {
                values_[i] += value;
            } else if constexpr (upd_op == UpdateOperation::multiply) {
                values_[i] *= value;
            } else if constexpr (upd_op == UpdateOperation::set_equal) {
                values_[i] = value;
            }
        }
    }

    value_t Get(uint32_t l, uint32_t r) {
        assert(l <= r && r < n_);
        value_t ans = values_[l];
        for (size_t i = l + 1; i <= r; i++) {
            if constexpr (get_op == GetOperation::sum) {
                ans += values_[i];
            } else if constexpr (get_op == GetOperation::product) {
                ans *= values_[i];
            } else if constexpr (get_op == GetOperation::max) {
                ans = std::max(ans, values_[i]);
            } else if constexpr (get_op == GetOperation::min) {
                ans = std::min(ans, values_[i]);
            }
        }

        return ans;
    }

    ~SegTreeChecker() {
        operator delete(values_);
        values_ = nullptr;
    }
};

template <typename value_t, GetOperation get_op, UpdateOperation upd_op>
struct SegTreeHelper {
    using type = void;
};

template <typename value_t>
struct SegTreeHelper<value_t, GetOperation::min, UpdateOperation::add> {
    using type = MinMaxSegTreeAdd<value_t, GetOperation::min>;
};

template <typename value_t>
struct SegTreeHelper<value_t, GetOperation::max, UpdateOperation::add> {
    using type = MinMaxSegTreeAdd<value_t, GetOperation::max>;
};

template <typename value_t>
struct SegTreeHelper<value_t, GetOperation::min, UpdateOperation::multiply> {
    using type = MinMaxSegTreeMult<value_t, GetOperation::min>;
};

template <typename value_t>
struct SegTreeHelper<value_t, GetOperation::max, UpdateOperation::multiply> {
    using type = MinMaxSegTreeMult<value_t, GetOperation::max>;
};

template <typename value_t>
struct SegTreeHelper<value_t, GetOperation::sum, UpdateOperation::multiply> {
    using type = SumSegTreeMult<value_t>;
};

template <typename value_t>
struct SegTreeHelper<value_t, GetOperation::sum, UpdateOperation::set_equal> {
    using type = SumSegTreeSetEqual<value_t>;
};

template <typename value_t>
struct SegTreeHelper<value_t, GetOperation::product,
                     UpdateOperation::set_equal> {
    using type = ProdSegTreeSetEqual<value_t>;
};

template <typename value_t>
struct SegTreeHelper<value_t, GetOperation::max, UpdateOperation::set_equal> {
    using type = MinMaxSegTreeSetEqual<value_t, GetOperation::max>;
};

template <typename value_t>
struct SegTreeHelper<value_t, GetOperation::min, UpdateOperation::set_equal> {
    using type = MinMaxSegTreeSetEqual<value_t, GetOperation::min>;
};

template <typename value_t>
struct SegTreeHelper<value_t, GetOperation::sum, UpdateOperation::add> {
    using type = SumSegTreeAdd<value_t>;
};

template <typename value_t>
struct SegTreeHelper<value_t, GetOperation::product,
                     UpdateOperation::multiply> {
    using type = ProdSegTreeMult<value_t>;
};

}  // namespace segtrees

template <UpdateOperation upd_op, GetOperation get_op, typename value_t = int64_t>
using SegTree = typename segtrees::SegTreeHelper<value_t, get_op, upd_op>::type;

#include <random>

template <uint32_t n, uint32_t q, typename value_t>
[[maybe_unused]] static inline void fillData(
    std::vector<value_t>& values, std::vector<value_t>& update_values,
    std::vector<uint32_t>& l_int, std::vector<uint32_t>& r_int) {
    std::mt19937 rnd;
    for (value_t& a_i : values) {
        if constexpr (std::is_integral_v<value_t>) {
            a_i = value_t(int32_t(rnd())) % value_t(64);
        } else {
            a_i = value_t(int32_t(rnd()));
        }
    }
    for (value_t& a_i : update_values) {
        if constexpr (std::is_integral_v<value_t>) {
            a_i = value_t(int32_t(rnd())) % value_t(64);
        } else {
            a_i = value_t(int32_t(rnd()));
        }
    }
    for (size_t i = 0; i < q; i++) {
        uint32_t l = uint32_t(rnd() % n);
        uint32_t r = uint32_t(rnd() % n);
        if (l > r) {
            std::swap(l, r);
        }
        l_int[i] = l;
        r_int[i] = r;
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testMinAdd(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::add, GetOperation::min, value_t> tree(values);
    segtrees::SegTreeChecker<UpdateOperation::add, GetOperation::min, value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        assert(tree_ans == checker_ans);
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testMinSetEqual(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::set_equal, GetOperation::min, value_t> tree(
        values);
    segtrees::SegTreeChecker<UpdateOperation::set_equal, GetOperation::min,
                             value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        assert(tree_ans == checker_ans);
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testMaxAdd(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::add, GetOperation::max, value_t> tree(values);
    segtrees::SegTreeChecker<UpdateOperation::add, GetOperation::max, value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        assert(tree_ans == checker_ans);
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testMaxSetEqual(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::set_equal, GetOperation::max, value_t> tree(
        values);
    segtrees::SegTreeChecker<UpdateOperation::set_equal, GetOperation::max,
                             value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        assert(tree_ans == checker_ans);
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testSumMultiply(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::multiply, GetOperation::sum, value_t> tree(values);
    segtrees::SegTreeChecker<UpdateOperation::multiply, GetOperation::sum,
                             value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        if constexpr (std::is_floating_point_v<value_t>) {
            bool overflow = std::isnan(tree_ans) || std::isinf(tree_ans);
            assert(overflow ==
                   (std::isnan(checker_ans) || std::isinf(checker_ans)));
            if (overflow || tree_ans == checker_ans) {
                continue;
            }
            auto fraq = tree_ans / checker_ans;
            assert(value_t(1.0L - 0.001L) <= fraq &&
                   fraq <= value_t(1.0L + 0.001L));
        } else {
            assert(tree_ans == checker_ans);
        }
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testSumAdd(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::add, GetOperation::sum, value_t> tree(values);
    segtrees::SegTreeChecker<UpdateOperation::add, GetOperation::sum, value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        assert(tree_ans == checker_ans);
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testSumSetEqual(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::set_equal, GetOperation::sum, value_t> tree(
        values);
    segtrees::SegTreeChecker<UpdateOperation::set_equal, GetOperation::sum,
                             value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        assert(tree_ans == checker_ans);
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testProductSetEqual(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::set_equal, GetOperation::product, value_t> tree(
        values);
    segtrees::SegTreeChecker<UpdateOperation::set_equal, GetOperation::product,
                             value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        if constexpr (std::is_floating_point_v<value_t>) {
            bool overflow = std::isnan(tree_ans) || std::isinf(tree_ans);
            assert(overflow ==
                   (std::isnan(checker_ans) || std::isinf(checker_ans)));
            if (overflow || tree_ans == checker_ans) {
                continue;
            }
            auto fraq = tree_ans / checker_ans;
            assert(value_t(1.0L - 0.001L) <= fraq &&
                   fraq <= value_t(1.0L + 0.001L));
        } else {
            assert(tree_ans == checker_ans);
        }
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testProductMultiply(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::multiply, GetOperation::product, value_t> tree(
        values);
    segtrees::SegTreeChecker<UpdateOperation::multiply, GetOperation::product,
                             value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        if constexpr (std::is_floating_point_v<value_t>) {
            bool overflow = std::isnan(tree_ans) || std::isinf(tree_ans);
            assert(overflow ==
                   (std::isnan(checker_ans) || std::isinf(checker_ans)));
            if (overflow || tree_ans == checker_ans) {
                continue;
            }
            auto fraq = tree_ans / checker_ans;
            assert(value_t(1.0L - 0.001L) <= fraq &&
                   fraq <= value_t(1.0L + 0.001L));
        } else {
            assert(tree_ans == checker_ans);
        }
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testMinMultiply(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    if (std::is_integral_v<value_t>) {
        return;
    }

    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::multiply, GetOperation::min, value_t> tree(values);
    segtrees::SegTreeChecker<UpdateOperation::multiply, GetOperation::min,
                             value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        if constexpr (std::is_floating_point_v<value_t>) {
            bool overflow = std::isnan(tree_ans) || std::isinf(tree_ans);
            assert(overflow ==
                   (std::isnan(checker_ans) || std::isinf(checker_ans)));
            if (overflow || tree_ans == checker_ans) {
                continue;
            }
            auto fraq = tree_ans / checker_ans;
            assert(value_t(1.0L - 0.001L) <= fraq &&
                   fraq <= value_t(1.0L + 0.001L));
        } else {
            assert(tree_ans == checker_ans);
        }
    }
}

template <typename value_t>
[[maybe_unused]] static inline void testMaxMultiply(
    const std::vector<value_t>& values,
    const std::vector<value_t>& update_values,
    const std::vector<uint32_t>& l_int, const std::vector<uint32_t>& r_int) {
    if (std::is_integral_v<value_t>) {
        return;
    }

    const uint32_t q = uint32_t(l_int.size());
    assert(q == r_int.size());
    assert(q / 2 == update_values.size());
    SegTree<UpdateOperation::multiply, GetOperation::max, value_t> tree(values);
    segtrees::SegTreeChecker<UpdateOperation::multiply, GetOperation::max,
                             value_t>
        checker(values);
    for (uint32_t i = 0; i < q; i += 2) {
        uint32_t l = l_int[i];
        uint32_t r = r_int[i];
        value_t upd_value = update_values[i / 2];
        tree.Update(l, r, upd_value);
        checker.Update(l, r, upd_value);
        l = l_int[i + 1];
        r = r_int[i + 1];
        value_t tree_ans = tree.Get(l, r);
        value_t checker_ans = checker.Get(l, r);
        if constexpr (std::is_floating_point_v<value_t>) {
            bool overflow = std::isnan(tree_ans) || std::isinf(tree_ans);
            assert(overflow ==
                   (std::isnan(checker_ans) || std::isinf(checker_ans)));
            if (overflow || tree_ans == checker_ans) {
                continue;
            }
            auto fraq = tree_ans / checker_ans;
            assert(value_t(1.0L - 0.001L) <= fraq &&
                   fraq <= value_t(1.0L + 0.001L));
        } else {
            assert(tree_ans == checker_ans);
        }
    }
}

template <typename value_t = int64_t>
void tests() {
    constexpr uint32_t n = 65536;
    constexpr uint32_t q = 32768;
    std::vector<value_t> values(n);
    std::vector<value_t> update_values(q / 2);
    std::vector<uint32_t> l_int(q);
    std::vector<uint32_t> r_int(q);
    fillData<n, q>(values, update_values, l_int, r_int);
    testMinAdd(values, update_values, l_int, r_int);
    testMinSetEqual(values, update_values, l_int, r_int);
    testMaxAdd(values, update_values, l_int, r_int);
    testMaxSetEqual(values, update_values, l_int, r_int);
    testSumMultiply(values, update_values, l_int, r_int);
    testSumAdd(values, update_values, l_int, r_int);
    testSumSetEqual(values, update_values, l_int, r_int);
    testProductSetEqual(values, update_values, l_int, r_int);
    testProductMultiply(values, update_values, l_int, r_int);
    testMinMultiply(values, update_values, l_int, r_int);
    testMaxMultiply(values, update_values, l_int, r_int);
}

int main() {
    tests();
    tests<double>();
}
