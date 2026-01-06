#include <array>
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <type_traits>
#include <valarray>
#include <vector>

#include "../../number_theory/math_functions.hpp"

enum class UpdateOperation : std::uint8_t {
    add = 1,
    multiply,
    set_equal,
};

enum class GetOperation : std::uint8_t {
    sum = 1,
    product,
    max,
    min,
};

namespace segtrees {

using std::size_t;
using std::uint32_t;

template <typename T>
[[nodiscard]] ATTRIBUTE_CONST constexpr T bin_pow(const T n, const uint32_t p) noexcept {
    return math_functions::bin_pow(n, p);
}

[[nodiscard]] ATTRIBUTE_CONST constexpr size_t tree_size(const uint32_t n) noexcept {
    return size_t{1ull} << (1 + math_functions::log2_ceil(n | 1));
}

template <typename value_t, GetOperation get_op>
class [[nodiscard]] MinMaxSegTreeAdd {
    static_assert((get_op == GetOperation::max) || (get_op == GetOperation::min));

public:
    explicit MinMaxSegTreeAdd(const std::vector<value_t>& data)
        : MinMaxSegTreeAdd(data.data(), static_cast<uint32_t>(data.size())) {}

    template <size_t N>
    explicit MinMaxSegTreeAdd(const std::array<value_t, N>& data)
        : MinMaxSegTreeAdd(data.data(), static_cast<uint32_t>(data.size())) {}

    template <size_t N>
    explicit MinMaxSegTreeAdd(const value_t (&data)[N])
        : MinMaxSegTreeAdd(std::data(data), static_cast<uint32_t>(std::size(data))) {}

    explicit MinMaxSegTreeAdd(const std::initializer_list<value_t> data)
        : MinMaxSegTreeAdd(std::data(data), static_cast<uint32_t>(std::size(data))) {}

    explicit MinMaxSegTreeAdd(const value_t* const data, const uint32_t n) : nodes_(tree_size(n)), n_(n) {
        assert(data != nullptr);
        assert(n > 0);
        this->BuildRecImpl(data, 0, 0, n - 1);
    }

    void update(const uint32_t l, const uint32_t r, const value_t upd_value) noexcept {
        assert(l <= r && r < n_);
        query_l_ = l;
        query_r_ = r;
        value_ = upd_value;
        this->UpdateRecImpl(0, 0, n_ - 1);
    }

    [[nodiscard]] value_t get(const uint32_t l, const uint32_t r) noexcept {
        assert(l <= r && r < n_);
        return this->GetRecImpl(0, 0, n_ - 1, l, r);
    }

private:
    static constexpr value_t kNoPromise = 0;

    struct Node {
        value_t value{};
        value_t promise = kNoPromise;
    };

    void BuildRecImpl(const value_t* const data,
                      const size_t node_index,
                      const uint32_t node_l,
                      const uint32_t node_r) noexcept {
        assert(node_index < nodes_.size() && node_l <= node_r && node_r < n_);
        if (node_l == node_r) {
            nodes_[node_index].value = data[node_l];
            return;
        }

        const uint32_t node_m = (node_l + node_r) / 2;
        const size_t left_node_index = node_index * 2 + 1;
        this->BuildRecImpl(data, left_node_index, node_l, node_m);
        const size_t right_node_index = left_node_index + 1;
        this->BuildRecImpl(data, right_node_index, node_m + 1, node_r);
        assert(right_node_index < nodes_.size());
        if constexpr (get_op == GetOperation::max) {
            nodes_[node_index].value = std::max(nodes_[left_node_index].value, nodes_[right_node_index].value);
        } else {
            nodes_[node_index].value = std::min(nodes_[left_node_index].value, nodes_[right_node_index].value);
        }
    }

    void UpdateRecImpl(const size_t node_index, const size_t node_l, const size_t node_r) noexcept {
        assert(node_index < nodes_.size() && node_l <= node_r && node_r < n_);
        if (query_l_ <= node_l && node_r <= query_r_) {
            nodes_[node_index].promise += value_;
            return;
        }

        if (query_r_ < node_l || node_r < query_l_) {
            return;
        }

        const size_t node_m = (node_l + node_r) / 2;
        this->PushImpl(node_index);
        const size_t left_node_index = node_index * 2 + 1;
        const size_t right_node_index = left_node_index + 1;
        assert(right_node_index < nodes_.size());
        this->UpdateRecImpl(left_node_index, node_l, node_m);
        this->UpdateRecImpl(right_node_index, node_m + 1, node_r);
        const Node& left_node = nodes_[left_node_index];
        const Node& right_node = nodes_[right_node_index];
        if constexpr (get_op == GetOperation::max) {
            nodes_[node_index].value =
                std::max(left_node.value + left_node.promise, right_node.value + right_node.promise);
        } else {
            nodes_[node_index].value =
                std::min(left_node.value + left_node.promise, right_node.value + right_node.promise);
        }
    }

    void PushImpl(const size_t node_index) noexcept {
        assert(node_index < nodes_.size());
        const size_t left_node_index = node_index * 2 + 1;
        const size_t right_node_index = left_node_index + 1;
        assert(right_node_index < nodes_.size());
        const value_t this_promise = nodes_[node_index].promise;
        if (this_promise == 0) {
            // Not a very rare case btw
            return;
        }
        nodes_[node_index].value += this_promise;
        nodes_[left_node_index].promise += this_promise;
        nodes_[right_node_index].promise += this_promise;
        nodes_[node_index].promise = 0;
    }

    [[nodiscard]] value_t GetRecImpl(const size_t node_index,
                                     const uint32_t node_l,
                                     const uint32_t node_r,
                                     const uint32_t query_l,
                                     const uint32_t query_r) noexcept {
        assert(node_index < nodes_.size());
        if (query_l == node_l && node_r == query_r) {
            const Node& node = nodes_[node_index];
            return node.value + node.promise;
        }

        const uint32_t node_middle = (node_l + node_r) / 2;
        this->PushImpl(node_index);
        const size_t left_son = node_index * 2 + 1;
        const size_t right_son = left_son + 1;
        if (query_r <= node_middle) {
            return this->GetRecImpl(left_son, node_l, node_middle, query_l, query_r);
        }

        if (node_middle < query_l) {
            return this->GetRecImpl(right_son, node_middle + 1, node_r, query_l, query_r);
        }

        // query_l <= node_middle < query_r
        const value_t left_result = this->GetRecImpl(left_son, node_l, node_middle, query_l, node_middle);
        const value_t right_result = this->GetRecImpl(right_son, node_middle + 1, node_r, node_middle + 1, query_r);
        if constexpr (get_op == GetOperation::max) {
            return std::max(left_result, right_result);
        } else {
            return std::min(left_result, right_result);
        }
    }

    std::valarray<Node> nodes_;
    uint32_t query_l_{};
    uint32_t query_r_{};
    value_t value_{};
    uint32_t n_;
};

template <typename value_t, GetOperation get_op>
class [[nodiscard]] MinMaxSegTreeMult {
    static_assert((get_op == GetOperation::max) || (get_op == GetOperation::min));

public:
    explicit MinMaxSegTreeMult(const std::vector<value_t>& data)
        : MinMaxSegTreeMult(data.data(), static_cast<uint32_t>(data.size())) {}

    template <size_t N>
    explicit MinMaxSegTreeMult(const std::array<value_t, N>& data)
        : MinMaxSegTreeMult(data.data(), static_cast<uint32_t>(data.size())) {}

    template <size_t N>
    explicit MinMaxSegTreeMult(const value_t (&data)[N])
        : MinMaxSegTreeMult(std::data(data), static_cast<uint32_t>(std::size(data))) {}

    explicit MinMaxSegTreeMult(const std::initializer_list<value_t> data)
        : MinMaxSegTreeMult(std::data(data), static_cast<uint32_t>(std::size(data))) {}

    explicit MinMaxSegTreeMult(const value_t* const data, const uint32_t n) : nodes_(tree_size(n)), n_(n) {
        static_assert(!std::is_integral_v<value_t>,
                      "Warning: min / max segment tree with "
                      "multiplication on update should not be used with "
                      "integral type because of the type overflow\n"
                      "If you are sure of what you are doing, "
                      "remove this assert ");

        assert(data != nullptr);
        assert(n > 0);
        this->BuildRecImpl(data, 0, 0, n - 1);
    }

    void update(const uint32_t l, const uint32_t r, const value_t upd_value) noexcept {
        assert(l <= r && r < n_);
        query_l_ = l;
        query_r_ = r;
        value_ = upd_value;
        this->UpdateRecImpl(0, 0, n_ - 1);
    }

    [[nodiscard]] value_t get(const uint32_t l, const uint32_t r) noexcept {
        assert(l <= r && r < n_);
        return this->GetRecImpl(0, 0, n_ - 1, l, r);
    }

private:
    static constexpr value_t kNoPromise = 1;

    struct Node {
        value_t min_value{};
        value_t max_value{};
        value_t promise = kNoPromise;
    };

    void BuildRecImpl(const value_t* const data,
                      const size_t node_index,
                      const uint32_t node_l,
                      const uint32_t node_r) noexcept {
        assert(node_index < nodes_.size() && node_l <= node_r && node_r < n_);
        if (node_l == node_r) {
            nodes_[node_index].max_value = nodes_[node_index].min_value = data[node_l];
            return;
        }

        const uint32_t node_m = (node_l + node_r) / 2;
        const size_t left_node_index = node_index * 2 + 1;
        this->BuildRecImpl(data, left_node_index, node_l, node_m);
        const size_t right_node_index = left_node_index + 1;
        this->BuildRecImpl(data, right_node_index, node_m + 1, node_r);
        assert(right_node_index < nodes_.size());
        const Node& left_node = nodes_[left_node_index];
        const Node& right_node = nodes_[right_node_index];
        nodes_[node_index].min_value = std::min(left_node.min_value, right_node.min_value);
        nodes_[node_index].max_value = std::max(left_node.max_value, right_node.max_value);
    }

    void UpdateRecImpl(const size_t node_index, const uint32_t node_l, const uint32_t node_r) noexcept {
        assert(node_index < nodes_.size());
        if (query_l_ <= node_l && node_r <= query_r_) {
            nodes_[node_index].promise *= value_;
            return;
        }

        if (query_r_ < node_l || node_r < query_l_) {
            return;
        }

        const uint32_t node_m = (node_l + node_r) / 2;
        this->PushImpl(node_index);
        const size_t left_node_index = node_index * 2 + 1;
        this->UpdateRecImpl(left_node_index, node_l, node_m);
        const size_t right_node_index = left_node_index + 1;
        this->UpdateRecImpl(right_node_index, node_m + 1, node_r);
        const Node& left_node = nodes_[left_node_index];
        const Node& right_node = nodes_[right_node_index];
        const value_t left_node_promise = left_node.promise;
        const value_t right_node_promise = right_node.promise;
        nodes_[node_index].min_value =
            std::min((left_node_promise >= 0 ? left_node.min_value : left_node.max_value) * left_node_promise,
                     (right_node_promise >= 0 ? right_node.min_value : right_node.max_value) * right_node_promise);
        nodes_[node_index].max_value =
            std::max((left_node_promise >= 0 ? left_node.max_value : left_node.min_value) * left_node_promise,
                     (right_node_promise >= 0 ? right_node.max_value : right_node.min_value) * right_node_promise);
    }

    void PushImpl(const size_t node_index) noexcept {
        assert(node_index < nodes_.size());
        Node& node = nodes_[node_index];
        const value_t this_promise = node.promise;
        if (this_promise == kNoPromise) {
            return;
        }

        if (this_promise >= 0) {
            node.max_value *= this_promise;
            node.min_value *= this_promise;
        } else {
            const value_t new_min_value = node.max_value * this_promise;
            node.max_value = node.min_value * this_promise;
            node.min_value = new_min_value;
        }
        node.promise = kNoPromise;
        const size_t left_node_index = node_index * 2 + 1;
        nodes_[left_node_index].promise *= this_promise;
        const size_t right_node_index = left_node_index + 1;
        nodes_[right_node_index].promise *= this_promise;
    }

    [[nodiscard]] value_t GetRecImpl(const size_t node_index,
                                     const uint32_t node_l,
                                     const uint32_t node_r,
                                     const uint32_t query_l,
                                     const uint32_t query_r) noexcept {
        assert(node_index < nodes_.size());
        if (query_l == node_l && node_r == query_r) {
            const Node& node = nodes_[node_index];
            const value_t t1 = node.min_value * node.promise;
            const value_t t2 = node.max_value * node.promise;
            if constexpr (get_op == GetOperation::max) {
                return std::max(t1, t2);
            } else {
                return std::min(t1, t2);
            }
        }

        const uint32_t node_middle = (node_l + node_r) / 2;
        this->PushImpl(node_index);
        const size_t left_son = node_index * 2 + 1;
        const size_t right_son = left_son + 1;
        if (query_r <= node_middle) {
            return this->GetRecImpl(left_son, node_l, node_middle, query_l, query_r);
        }

        if (node_middle < query_l) {
            return this->GetRecImpl(right_son, node_middle + 1, node_r, query_l, query_r);
        }

        // query_l <= node_middle < query_r
        const value_t left_result = this->GetRecImpl(left_son, node_l, node_middle, query_l, node_middle);
        const value_t right_result = this->GetRecImpl(right_son, node_middle + 1, node_r, node_middle + 1, query_r);
        if constexpr (get_op == GetOperation::max) {
            return std::max(left_result, right_result);
        } else {
            return std::min(left_result, right_result);
        }
    }

    std::valarray<Node> nodes_;
    uint32_t query_l_{};
    uint32_t query_r_{};
    value_t value_{};
    uint32_t n_;
};

template <typename value_t, GetOperation get_op>
class [[nodiscard]] MinMaxSegTreeSetEqual {
private:
    static_assert((get_op == GetOperation::max) || (get_op == GetOperation::min));

public:
    explicit MinMaxSegTreeSetEqual(const std::vector<value_t>& data)
        : MinMaxSegTreeSetEqual(data.data(), static_cast<uint32_t>(data.size())) {}

    template <size_t N>
    explicit MinMaxSegTreeSetEqual(const std::array<value_t, N>& data)
        : MinMaxSegTreeSetEqual(data.data(), static_cast<uint32_t>(data.size())) {}

    template <size_t N>
    explicit MinMaxSegTreeSetEqual(const value_t (&data)[N])
        : MinMaxSegTreeSetEqual(std::data(data), static_cast<uint32_t>(std::size(data))) {}

    explicit MinMaxSegTreeSetEqual(const std::initializer_list<value_t> data)
        : MinMaxSegTreeSetEqual(std::data(data), static_cast<uint32_t>(std::size(data))) {}

    explicit MinMaxSegTreeSetEqual(const value_t* const data, const uint32_t n) : nodes_(tree_size(n)), n_(n) {
        assert(data != nullptr);
        assert(n > 0);
        this->BuildRecImpl(data, 0, 0, n - 1);
    }

    void update(const uint32_t l, const uint32_t r, const value_t upd_value) noexcept {
        assert(l <= r && r < n_);
        query_l_ = l;
        query_r_ = r;
        value_ = upd_value;
        this->UpdateRecImpl(0, 0, n_ - 1);
    }

    [[nodiscard]] value_t get(const uint32_t l, const uint32_t r) noexcept {
        assert(l <= r && r < n_);
        return this->GetRecImpl(0, 0, n_ - 1, l, r);
    }

private:
    struct Node {
        value_t value{};
        value_t promise{};
        bool has_promise = false;
    };

    void BuildRecImpl(const value_t* const data,
                      const size_t node_index,
                      const uint32_t node_l,
                      const uint32_t node_r) noexcept {
        assert(node_index < nodes_.size() && node_l <= node_r && node_r < n_);

        if (node_l == node_r) {
            nodes_[node_index].value = data[node_l];
            return;
        }

        const uint32_t node_m = (node_l + node_r) / 2;
        const size_t left_son = node_index * 2 + 1;
        this->BuildRecImpl(data, left_son, node_l, node_m);
        const size_t right_son = left_son + 1;
        this->BuildRecImpl(data, right_son, node_m + 1, node_r);
        assert(right_son < nodes_.size());
        if constexpr (get_op == GetOperation::max) {
            nodes_[node_index].value = std::max(nodes_[left_son].value, nodes_[right_son].value);
        } else {
            nodes_[node_index].value = std::min(nodes_[left_son].value, nodes_[right_son].value);
        }
    }

    void UpdateRecImpl(const size_t node_index, const uint32_t node_l, const uint32_t node_r) noexcept {
        if (query_l_ <= node_l && node_r <= query_r_) {
            nodes_[node_index].promise = value_;
            nodes_[node_index].has_promise = true;
            return;
        }

        if (query_r_ < node_l || node_r < query_l_) {
            return;
        }

        const uint32_t node_m = (node_l + node_r) / 2;
        this->PushImpl(node_index);
        const size_t left_node = node_index * 2 + 1;
        const size_t right_node = left_node + 1;
        this->UpdateRecImpl(left_node, node_l, node_m);
        this->UpdateRecImpl(right_node, node_m + 1, node_r);

        if constexpr (get_op == GetOperation::max) {
            nodes_[node_index].value =
                std::max(nodes_[left_node].has_promise ? nodes_[left_node].promise : nodes_[left_node].value,
                         nodes_[right_node].has_promise ? nodes_[right_node].promise : nodes_[right_node].value);
        } else {
            nodes_[node_index].value =
                std::min(nodes_[left_node].has_promise ? nodes_[left_node].promise : nodes_[left_node].value,
                         nodes_[right_node].has_promise ? nodes_[right_node].promise : nodes_[right_node].value);
        }
    }

    void PushImpl(const size_t node_index) noexcept {
        assert(node_index < nodes_.size());
        if (!nodes_[node_index].has_promise) {
            return;
        }

        const value_t this_promise = nodes_[node_index].promise;
        const size_t left_node = node_index * 2 + 1;
        const size_t right_node = left_node + 1;
        nodes_[node_index].value = this_promise;
        nodes_[node_index].has_promise = false;
        nodes_[left_node].promise = this_promise;
        nodes_[left_node].has_promise = true;
        nodes_[right_node].promise = this_promise;
        nodes_[right_node].has_promise = true;
    }

    [[nodiscard]] value_t GetRecImpl(const size_t node_index,
                                     const uint32_t node_l,
                                     const uint32_t node_r,
                                     const uint32_t query_l,
                                     const uint32_t query_r) noexcept {
        assert(node_index < nodes_.size());
        if (query_l == node_l && node_r == query_r) {
            return nodes_[node_index].has_promise ? nodes_[node_index].promise : nodes_[node_index].value;
        }

        const uint32_t node_middle = (node_l + node_r) / 2;
        this->PushImpl(node_index);
        const size_t left_son = node_index * 2 + 1;
        const size_t right_son = left_son + 1;
        if (query_r <= node_middle) {
            return this->GetRecImpl(left_son, node_l, node_middle, query_l, query_r);
        }

        if (node_middle < query_l) {
            return this->GetRecImpl(right_son, node_middle + 1, node_r, query_l, query_r);
        }

        // query_l <= node_middle < query_r
        const value_t left_result = this->GetRecImpl(left_son, node_l, node_middle, query_l, node_middle);
        const value_t right_result = this->GetRecImpl(right_son, node_middle + 1, node_r, node_middle + 1, query_r);
        if constexpr (get_op == GetOperation::max) {
            return std::max(left_result, right_result);
        } else {
            return std::min(left_result, right_result);
        }
    }

    std::valarray<Node> nodes_;
    uint32_t query_l_{};
    uint32_t query_r_{};
    value_t value_{};
    uint32_t n_;
};

template <typename value_t>
class [[nodiscard]] SumSegTreeSetEqual {
public:
    explicit SumSegTreeSetEqual(const std::vector<value_t>& data)
        : SumSegTreeSetEqual(data.data(), static_cast<uint32_t>(data.size())) {}

    template <size_t N>
    explicit SumSegTreeSetEqual(const std::array<value_t, N>& data)
        : SumSegTreeSetEqual(data.data(), static_cast<uint32_t>(data.size())) {}

    template <size_t N>
    explicit SumSegTreeSetEqual(const value_t (&data)[N])
        : SumSegTreeSetEqual(std::data(data), static_cast<uint32_t>(std::size(data))) {}

    explicit SumSegTreeSetEqual(const std::initializer_list<value_t> data)
        : SumSegTreeSetEqual(std::data(data), static_cast<uint32_t>(std::size(data))) {}

    explicit SumSegTreeSetEqual(const value_t* const data, const uint32_t n) : nodes_(tree_size(n)), n_(n) {
        assert(data != nullptr);
        assert(n > 0);
        this->BuildRecImpl(data, 0, 0, n - 1);
    }

    void update(const uint32_t l, const uint32_t r, const value_t upd_value) noexcept {
        assert(l <= r && r < n_);
        query_l_ = l;
        query_r_ = r;
        value_ = upd_value;
        this->UpdateRecImpl(0, 0, n_ - 1);
    }

    [[nodiscard]] value_t get(const uint32_t l, const uint32_t r) noexcept {
        assert(l <= r && r < n_);
        return this->GetRecImpl(0, 0, n_ - 1, l, r);
    }

private:
    struct Node {
        value_t value{};
        value_t promise{};
        bool has_promise = false;
    };

    void BuildRecImpl(const value_t* const data,
                      const size_t node_index,
                      const uint32_t node_l,
                      const uint32_t node_r) noexcept {
        assert(node_index < nodes_.size() && node_l <= node_r && node_r < n_);
        if (node_l == node_r) {
            nodes_[node_index].value = data[node_l];
            return;
        }

        const uint32_t node_m = (node_l + node_r) / 2;
        const size_t left_son = node_index * 2 + 1;
        this->BuildRecImpl(data, left_son, node_l, node_m);
        const size_t right_son = left_son + 1;
        this->BuildRecImpl(data, right_son, node_m + 1, node_r);
        assert(right_son < nodes_.size());
        nodes_[node_index].value = nodes_[left_son].value + nodes_[right_son].value;
    }

    void UpdateRecImpl(const size_t node_index, const uint32_t node_l, const uint32_t node_r) noexcept {
        assert(node_index < nodes_.size() && node_l <= node_r && node_r < n_);
        if (query_l_ <= node_l && node_r <= query_r_) {
            nodes_[node_index].promise = value_;
            nodes_[node_index].has_promise = true;
            return;
        }

        if (query_r_ < node_l || node_r < query_l_) {
            return;
        }

        const uint32_t node_m = (node_l + node_r) / 2;
        this->PushImpl(node_index, node_l, node_r);
        const size_t left_node_index = node_index * 2 + 1;
        const size_t right_node_index = left_node_index + 1;
        this->UpdateRecImpl(left_node_index, node_l, node_m);
        this->UpdateRecImpl(right_node_index, node_m + 1, node_r);
        assert(right_node_index < nodes_.size());
        const Node& left_node = nodes_[left_node_index];
        const Node& right_node = nodes_[right_node_index];
        const value_t left_value =
            left_node.has_promise ? (left_node.promise * static_cast<value_t>(node_m - node_l + 1)) : left_node.value;
        const value_t right_value =
            right_node.has_promise ? (right_node.promise * static_cast<value_t>(node_r - node_m)) : right_node.value;
        nodes_[node_index].value = left_value + right_value;
    }

    void PushImpl(const size_t node_index, const uint32_t node_l, const uint32_t node_r) noexcept {
        assert(node_index < nodes_.size());
        const size_t left_node = node_index * 2 + 1;
        const size_t right_node = left_node + 1;
        assert(right_node < nodes_.size());
        Node& node = nodes_[node_index];
        if (!node.has_promise) {
            return;
        }

        const value_t this_promise = node.promise;
        node.value = this_promise * static_cast<value_t>(node_r - node_l + 1);
        node.has_promise = false;
        nodes_[left_node].promise = this_promise;
        nodes_[left_node].has_promise = true;
        nodes_[right_node].promise = this_promise;
        nodes_[right_node].has_promise = true;
    }

    [[nodiscard]] value_t GetRecImpl(const size_t node_index,
                                     const uint32_t node_l,
                                     const uint32_t node_r,
                                     const uint32_t query_l,
                                     const uint32_t query_r) noexcept {
        assert(node_index < nodes_.size());
        if (query_l == node_l && node_r == query_r) {
            const Node& node = nodes_[node_index];
            return node.has_promise ? (node.promise * static_cast<value_t>(node_r - node_l + 1)) : node.value;
        }

        const uint32_t node_middle = (node_l + node_r) / 2;
        this->PushImpl(node_index, node_l, node_r);
        const size_t left_son = node_index * 2 + 1;
        const size_t right_son = left_son + 1;
        if (query_r <= node_middle) {
            return this->GetRecImpl(left_son, node_l, node_middle, query_l, query_r);
        }

        if (node_middle < query_l) {
            return this->GetRecImpl(right_son, node_middle + 1, node_r, query_l, query_r);
        }

        // query_l <= node_middle < query_r
        const value_t left_result = this->GetRecImpl(left_son, node_l, node_middle, query_l, node_middle);
        const value_t right_result = this->GetRecImpl(right_son, node_middle + 1, node_r, node_middle + 1, query_r);
        return left_result + right_result;
    }

    std::valarray<Node> nodes_;
    uint32_t query_l_{};
    uint32_t query_r_{};
    value_t value_{};
    uint32_t n_;
};

template <typename value_t>
class [[nodiscard]] ProdSegTreeSetEqual {
public:
    explicit ProdSegTreeSetEqual(const std::vector<value_t>& data)
        : ProdSegTreeSetEqual(data.data(), static_cast<uint32_t>(data.size())) {}

    template <size_t N>
    explicit ProdSegTreeSetEqual(const std::array<value_t, N>& data)
        : ProdSegTreeSetEqual(data.data(), static_cast<uint32_t>(data.size())) {}

    template <size_t N>
    explicit ProdSegTreeSetEqual(const value_t (&data)[N])
        : ProdSegTreeSetEqual(std::data(data), static_cast<uint32_t>(std::size(data))) {}

    explicit ProdSegTreeSetEqual(const std::initializer_list<value_t> data)
        : ProdSegTreeSetEqual(std::data(data), static_cast<uint32_t>(std::size(data))) {}

    explicit ProdSegTreeSetEqual(const value_t* const data, const uint32_t n) : nodes_(tree_size(n)), n_(n) {
        assert(data != nullptr);
        assert(n > 0);
        this->BuildRecImpl(data, 0, 0, n - 1);
    }

    void BuildRecImpl(const value_t* const data,
                      const size_t node_index,
                      const uint32_t node_l,
                      const uint32_t node_r) noexcept {
        assert(node_index < nodes_.size() && node_l <= node_r && node_r < n_);
        if (node_l == node_r) {
            nodes_[node_index].value = data[node_l];
            return;
        }

        const uint32_t node_m = (node_l + node_r) / 2;
        const size_t left_son = node_index * 2 + 1;
        this->BuildRecImpl(data, left_son, node_l, node_m);
        const size_t right_son = left_son + 1;
        this->BuildRecImpl(data, right_son, node_m + 1, node_r);
        assert(right_son < nodes_.size());
        nodes_[node_index].value = nodes_[left_son].value * nodes_[right_son].value;
    }

    void update(const uint32_t l, const uint32_t r, const value_t upd_value) noexcept {
        assert(l <= r && r < n_);
        query_l_ = l;
        query_r_ = r;
        value_ = upd_value;
        this->UpdateRecImpl(0, 0, n_ - 1);
    }

    [[nodiscard]] value_t get(const uint32_t l, const uint32_t r) noexcept {
        assert(l <= r && r < n_);
        return this->GetRecImpl(0, 0, n_ - 1, l, r);
    }

private:
    struct Node {
        value_t value{};
        value_t promise{};
        bool has_promise = false;
        value_t cached_promise_x_count{};
        bool has_cached_promise_x_count = false;
    };

    void UpdateRecImpl(const size_t node_index, const uint32_t node_l, const uint32_t node_r) noexcept {
        assert(node_index < nodes_.size() && node_l <= node_r && node_r < n_);
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

        const uint32_t node_m = (node_l + node_r) / 2;
        this->PushImpl(node_index, node_l, node_r);
        const size_t left_node_index = node_index * 2 + 1;
        const size_t right_node_index = left_node_index + 1;
        this->UpdateRecImpl(left_node_index, node_l, node_m);
        this->UpdateRecImpl(right_node_index, node_m + 1, node_r);
        assert(right_node_index < nodes_.size());
        Node& left_node = nodes_[left_node_index];
        Node& right_node = nodes_[right_node_index];
        // clang-format off
        //  = left_node.has_promise ? (left_node.promise * (node_m - node_l + 1)) : left_node.value;
        // clang-format on
        const value_t left_value = [&]() {
            if (left_node.has_promise) {
                if (!left_node.has_cached_promise_x_count) {
                    left_node.cached_promise_x_count = bin_pow(left_node.promise, node_m - node_l + 1);
                    left_node.has_cached_promise_x_count = true;
                }
                return left_node.cached_promise_x_count;
            } else {
                return left_node.value;
            }
        }();
        const value_t right_value = [&]() {
            if (right_node.has_promise) {
                if (!right_node.has_cached_promise_x_count) {
                    right_node.cached_promise_x_count = bin_pow(right_node.promise, node_r - node_m);
                    right_node.has_cached_promise_x_count = true;
                }
                return right_node.cached_promise_x_count;
            } else {
                return right_node.value;
            }
        }();
        nodes_[node_index].value = left_value * right_value;
    }

    void PushImpl(const size_t node_index, const uint32_t node_l, const uint32_t node_r) noexcept {
        assert(node_index < nodes_.size());
        const size_t left_node_index = node_index * 2 + 1;
        const size_t right_node_index = left_node_index + 1;
        assert(right_node_index < nodes_.size());
        Node& node = nodes_[node_index];
        if (!node.has_promise) {
            return;
        }

        Node& left_node = nodes_[left_node_index];
        Node& right_node = nodes_[right_node_index];
        const value_t this_promise = node.promise;
        node.value =
            node.has_cached_promise_x_count ? node.cached_promise_x_count : bin_pow(this_promise, node_r - node_l + 1);
        node.has_promise = false;
        node.has_cached_promise_x_count = false;
        left_node.promise = this_promise;
        left_node.has_promise = true;
        left_node.has_cached_promise_x_count = false;
        right_node.promise = this_promise;
        right_node.has_promise = true;
        right_node.has_cached_promise_x_count = false;
    }

    [[nodiscard]] value_t GetRecImpl(const size_t node_index,
                                     const uint32_t node_l,
                                     const uint32_t node_r,
                                     const uint32_t query_l,
                                     const uint32_t query_r) noexcept {
        assert(node_index < nodes_.size());
        if (query_l == node_l && node_r == query_r) {
            Node& node = nodes_[node_index];
            if (node.has_promise) {
                if (!node.has_cached_promise_x_count) {
                    node.has_cached_promise_x_count = true;
                    node.cached_promise_x_count = bin_pow(node.promise, node_r - node_l + 1);
                }
                return node.cached_promise_x_count;
            } else {
                return node.value;
            }
        }

        const uint32_t node_middle = (node_l + node_r) / 2;
        this->PushImpl(node_index, node_l, node_r);
        const size_t left_son = node_index * 2 + 1;
        const size_t right_son = left_son + 1;
        if (query_r <= node_middle) {
            return this->GetRecImpl(left_son, node_l, node_middle, query_l, query_r);
        }

        if (node_middle < query_l) {
            return this->GetRecImpl(right_son, node_middle + 1, node_r, query_l, query_r);
        }

        // query_l <= node_middle < query_r
        const value_t left_result = this->GetRecImpl(left_son, node_l, node_middle, query_l, node_middle);
        const value_t right_result = this->GetRecImpl(right_son, node_middle + 1, node_r, node_middle + 1, query_r);
        return left_result * right_result;
    }

    std::valarray<Node> nodes_;
    uint32_t query_l_{};
    uint32_t query_r_{};
    value_t value_{};
    uint32_t n_;
};

template <typename value_t>
class [[nodiscard]] SumSegTreeMult {
public:
    explicit SumSegTreeMult(const std::vector<value_t>& data)
        : SumSegTreeMult(data.data(), static_cast<uint32_t>(data.size())) {}

    template <size_t N>
    explicit SumSegTreeMult(const std::array<value_t, N>& data)
        : SumSegTreeMult(data.data(), static_cast<uint32_t>(data.size())) {}

    template <size_t N>
    explicit SumSegTreeMult(const value_t (&data)[N])
        : SumSegTreeMult(std::data(data), static_cast<uint32_t>(std::size(data))) {}

    explicit SumSegTreeMult(const std::initializer_list<value_t> data)
        : SumSegTreeMult(std::data(data), static_cast<uint32_t>(std::size(data))) {}

    explicit SumSegTreeMult(const value_t* const data, const uint32_t n) : nodes_(tree_size(n)), n_(n) {
        assert(data != nullptr);
        assert(n > 0);
        this->BuildRecImpl(data, 0, 0, n - 1);
    }

    void update(const uint32_t l, const uint32_t r, const value_t upd_value) noexcept {
        assert(l <= r && r < n_);
        query_l_ = l;
        query_r_ = r;
        value_ = upd_value;
        this->UpdateRecImpl(0, 0, n_ - 1);
    }

    [[nodiscard]] value_t get(const uint32_t l, const uint32_t r) noexcept {
        assert(l <= r && r < n_);
        return this->GetRecImpl(0, 0, n_ - 1, l, r);
    }

private:
    static constexpr value_t kNoPromise = 1;

    struct Node {
        value_t value{};
        value_t promise = kNoPromise;
    };

    void BuildRecImpl(const value_t* const data,
                      const size_t node_index,
                      const uint32_t node_l,
                      const uint32_t node_r) noexcept {
        assert(node_index < nodes_.size() && node_l <= node_r && node_r < n_);
        if (node_l == node_r) {
            nodes_[node_index].value = data[node_l];
            return;
        }

        const uint32_t node_m = (node_l + node_r) / 2;
        const size_t left_son = node_index * 2 + 1;
        this->BuildRecImpl(data, left_son, node_l, node_m);
        const size_t right_son = left_son + 1;
        this->BuildRecImpl(data, right_son, node_m + 1, node_r);
        assert(right_son < nodes_.size());
        nodes_[node_index].value = nodes_[left_son].value + nodes_[right_son].value;
    }

    void UpdateRecImpl(const size_t node_index, const uint32_t node_l, const uint32_t node_r) noexcept {
        assert(node_index < nodes_.size() && node_l <= node_r && node_r < n_);
        if (query_l_ <= node_l && node_r <= query_r_) {
            nodes_[node_index].promise *= value_;
            return;
        }

        if (query_r_ < node_l || node_r < query_l_) {
            return;
        }

        const uint32_t node_m = (node_l + node_r) / 2;
        this->PushImpl(node_index);
        const size_t left_node = node_index * 2 + 1;
        const size_t right_node = left_node + 1;
        this->UpdateRecImpl(left_node, node_l, node_m);
        this->UpdateRecImpl(right_node, node_m + 1, node_r);
        assert(right_node < nodes_.size());
        nodes_[node_index].value =
            nodes_[left_node].value * nodes_[left_node].promise + nodes_[right_node].value * nodes_[right_node].promise;
    }

    void PushImpl(const size_t node_index) noexcept {
        assert(node_index < nodes_.size());
        const value_t this_promise = nodes_[node_index].promise;
        if (this_promise == kNoPromise) {
            return;
        }

        const size_t left_node = node_index * 2 + 1;
        const size_t right_node = left_node + 1;
        assert(right_node < nodes_.size());
        nodes_[node_index].value *= this_promise;
        nodes_[left_node].promise *= this_promise;
        nodes_[right_node].promise *= this_promise;
        nodes_[node_index].promise = kNoPromise;
    }

    [[nodiscard]] value_t GetRecImpl(const size_t node_index,
                                     const uint32_t node_l,
                                     const uint32_t node_r,
                                     const uint32_t query_l,
                                     const uint32_t query_r) noexcept {
        assert(node_index < nodes_.size());
        if (query_l == node_l && node_r == query_r) {
            return nodes_[node_index].value * nodes_[node_index].promise;
        }

        const uint32_t node_middle = (node_l + node_r) / 2;
        this->PushImpl(node_index);
        const size_t left_son = node_index * 2 + 1;
        const size_t right_son = left_son + 1;
        if (query_r <= node_middle) {
            return this->GetRecImpl(left_son, node_l, node_middle, query_l, query_r);
        }

        if (node_middle < query_l) {
            return this->GetRecImpl(right_son, node_middle + 1, node_r, query_l, query_r);
        }

        // query_l <= node_middle < query_r
        const value_t left_result = this->GetRecImpl(left_son, node_l, node_middle, query_l, node_middle);
        const value_t right_result = this->GetRecImpl(right_son, node_middle + 1, node_r, node_middle + 1, query_r);
        return left_result + right_result;
    }

    std::valarray<Node> nodes_;
    uint32_t query_l_{};
    uint32_t query_r_{};
    value_t value_{};
    uint32_t n_;
};

template <typename value_t>
class [[nodiscard]] SumSegTreeAdd {
public:
    explicit SumSegTreeAdd(const std::vector<value_t>& data)
        : SumSegTreeAdd(data.data(), static_cast<uint32_t>(data.size())) {}

    template <size_t N>
    explicit SumSegTreeAdd(const std::array<value_t, N>& data)
        : SumSegTreeAdd(data.data(), static_cast<uint32_t>(data.size())) {}

    template <size_t N>
    explicit SumSegTreeAdd(const value_t (&data)[N])
        : SumSegTreeAdd(std::data(data), static_cast<uint32_t>(std::size(data))) {}

    explicit SumSegTreeAdd(const std::initializer_list<value_t> data)
        : SumSegTreeAdd(std::data(data), static_cast<uint32_t>(std::size(data))) {}

    explicit SumSegTreeAdd(const value_t* const data, const uint32_t n) : nodes_(tree_size(n)), n_(n) {
        assert(data != nullptr);
        assert(n > 0);
        this->BuildRecImpl(data, 0, 0, n - 1);
    }

    void update(const uint32_t l, const uint32_t r, const value_t upd_value) noexcept {
        assert(l <= r && r < n_);
        query_l_ = l;
        query_r_ = r;
        value_ = upd_value;
        this->UpdateRecImpl(0, 0, n_ - 1);
    }

    [[nodiscard]] value_t get(const uint32_t l, const uint32_t r) {
        assert(l <= r && r < n_);
        return this->GetRecImpl(0, 0, n_ - 1, l, r);
    }

private:
    static constexpr value_t kNoPromise = 0;

    struct Node {
        value_t value{};
        value_t promise = kNoPromise;
    };

    void BuildRecImpl(const value_t* const data,
                      const size_t node_index,
                      const uint32_t node_l,
                      const uint32_t node_r) noexcept {
        assert(node_index < nodes_.size() && node_l <= node_r && node_r < n_);
        if (node_l == node_r) {
            nodes_[node_index].value = data[node_l];
            return;
        }

        const uint32_t node_m = (node_l + node_r) / 2;
        const size_t left_son = node_index * 2 + 1;
        this->BuildRecImpl(data, left_son, node_l, node_m);
        const size_t right_son = left_son + 1;
        this->BuildRecImpl(data, right_son, node_m + 1, node_r);
        assert(right_son < nodes_.size());
        nodes_[node_index].value = nodes_[left_son].value + nodes_[right_son].value;
    }

    void UpdateRecImpl(const size_t node_index, const uint32_t node_l, const uint32_t node_r) noexcept {
        assert(node_index < nodes_.size() && node_l <= node_r && node_r < n_);
        if (query_l_ <= node_l && node_r <= query_r_) {
            nodes_[node_index].promise += value_;
            return;
        }

        if (query_r_ < node_l || node_r < query_l_) {
            return;
        }

        const uint32_t node_m = (node_l + node_r) / 2;
        this->PushImpl(node_index, node_l, node_r);
        const size_t left_node_index = node_index * 2 + 1;
        const size_t right_node_index = left_node_index + 1;
        this->UpdateRecImpl(left_node_index, node_l, node_m);
        this->UpdateRecImpl(right_node_index, node_m + 1, node_r);
        assert(right_node_index < nodes_.size());
        const Node& left_node = nodes_[left_node_index];
        const Node& right_node = nodes_[right_node_index];
        nodes_[node_index].value = left_node.value + left_node.promise * static_cast<value_t>(node_m - node_l + 1) +
                                   right_node.value + right_node.promise * static_cast<value_t>(node_r - node_m);
    }

    void PushImpl(const size_t node_index, const uint32_t node_l, const uint32_t node_r) noexcept {
        assert(node_index < nodes_.size());
        const size_t left_node = node_index * 2 + 1;
        const size_t right_node = left_node + 1;
        assert(right_node < nodes_.size());
        const value_t this_promise = nodes_[node_index].promise;
        if (this_promise == kNoPromise) {
            return;
        }
        nodes_[node_index].value += this_promise * static_cast<value_t>(node_r - node_l + 1);
        nodes_[left_node].promise += this_promise;
        nodes_[right_node].promise += this_promise;
        nodes_[node_index].promise = kNoPromise;
    }

    [[nodiscard]] value_t GetRecImpl(const size_t node_index,
                                     const uint32_t node_l,
                                     const uint32_t node_r,
                                     const uint32_t query_l,
                                     const uint32_t query_r) noexcept {
        assert(node_index < nodes_.size());
        if (query_l == node_l && node_r == query_r) {
            const Node& node = nodes_[node_index];
            return node.value + node.promise * static_cast<value_t>(node_r - node_l + 1);
        }

        const uint32_t node_middle = (node_l + node_r) / 2;
        this->PushImpl(node_index, node_l, node_r);
        const size_t left_son = node_index * 2 + 1;
        const size_t right_son = left_son + 1;
        if (query_r <= node_middle) {
            return this->GetRecImpl(left_son, node_l, node_middle, query_l, query_r);
        }

        if (node_middle < query_l) {
            return this->GetRecImpl(right_son, node_middle + 1, node_r, query_l, query_r);
        }

        // query_l <= node_middle < query_r
        const value_t left_result = this->GetRecImpl(left_son, node_l, node_middle, query_l, node_middle);
        const value_t right_result = this->GetRecImpl(right_son, node_middle + 1, node_r, node_middle + 1, query_r);
        return left_result + right_result;
    }

    std::valarray<Node> nodes_;
    uint32_t query_l_{};
    uint32_t query_r_{};
    value_t value_{};
    uint32_t n_;
};

template <typename value_t>
class [[nodiscard]] ProdSegTreeMult {
public:
    explicit ProdSegTreeMult(const std::vector<value_t>& data)
        : ProdSegTreeMult(data.data(), static_cast<uint32_t>(data.size())) {}

    template <size_t N>
    explicit ProdSegTreeMult(const std::array<value_t, N>& data)
        : ProdSegTreeMult(data.data(), static_cast<uint32_t>(data.size())) {}

    template <size_t N>
    explicit ProdSegTreeMult(const value_t (&data)[N])
        : ProdSegTreeMult(std::data(data), static_cast<uint32_t>(std::size(data))) {}

    explicit ProdSegTreeMult(const std::initializer_list<value_t> data)
        : ProdSegTreeMult(std::data(data), static_cast<uint32_t>(std::size(data))) {}

    explicit ProdSegTreeMult(const value_t* const data, const uint32_t n) : nodes_(tree_size(n)), n_(n) {
        assert(data != nullptr);
        assert(n > 0);
        this->BuildRecImpl(data, 0, 0, n - 1);
    }

    void update(const uint32_t l, const uint32_t r, const value_t upd_value) noexcept {
        assert(l <= r && r < n_);
        query_l_ = l;
        query_r_ = r;
        value_ = upd_value;
        this->UpdateRecImpl(0, 0, n_ - 1);
    }

    [[nodiscard]] value_t get(const uint32_t l, const uint32_t r) noexcept {
        assert(l <= r && r < n_);
        return this->GetRecImpl(0, 0, n_ - 1, l, r);
    }

private:
    static constexpr value_t kNoPromise = 1;

    struct Node {
        value_t value{};
        value_t promise = kNoPromise;
        value_t cached_promise_x_count = kNoPromise * 1;
        bool has_cached_promise_x_count = true;
    };

    void BuildRecImpl(const value_t* const data,
                      const size_t node_index,
                      const uint32_t node_l,
                      const uint32_t node_r) noexcept {
        assert(node_index < nodes_.size() && node_l <= node_r && node_r < n_);
        Node& node = nodes_[node_index];
        if (node_l == node_r) {
            node.value = data[node_l];
            return;
        }

        const uint32_t node_m = (node_l + node_r) / 2;
        const size_t left_son = node_index * 2 + 1;
        this->BuildRecImpl(data, left_son, node_l, node_m);
        const size_t right_son = left_son + 1;
        this->BuildRecImpl(data, right_son, node_m + 1, node_r);
        assert(right_son < nodes_.size());
        node.value = nodes_[left_son].value * nodes_[right_son].value;
    }

    void UpdateRecImpl(const size_t node_index, const uint32_t node_l, const uint32_t node_r) noexcept {
        assert(node_index < nodes_.size() && node_l <= node_r && node_r < n_);
        if (query_l_ <= node_l && node_r <= query_r_) {
            Node& node = nodes_[node_index];
            node.promise *= value_;
            node.has_cached_promise_x_count = false;
            return;
        }

        if (query_r_ < node_l || node_r < query_l_) {
            return;
        }

        const uint32_t node_m = (node_l + node_r) / 2;
        this->PushImpl(node_index, node_l, node_r);
        const size_t left_node_index = node_index * 2 + 1;
        const size_t right_node_index = left_node_index + 1;
        this->UpdateRecImpl(left_node_index, node_l, node_m);
        this->UpdateRecImpl(right_node_index, node_m + 1, node_r);
        assert(right_node_index < nodes_.size());
        Node& left_node = nodes_[left_node_index];
        if (!left_node.has_cached_promise_x_count) {
            left_node.cached_promise_x_count = bin_pow(left_node.promise, node_m - node_l + 1);
            left_node.has_cached_promise_x_count = true;
        }
        Node& right_node = nodes_[right_node_index];
        if (!right_node.has_cached_promise_x_count) {
            right_node.cached_promise_x_count = bin_pow(right_node.promise, node_r - node_m);
            right_node.has_cached_promise_x_count = true;
        }
        nodes_[node_index].value =
            left_node.value * left_node.cached_promise_x_count * right_node.value * right_node.cached_promise_x_count;
    }

    void PushImpl(const size_t node_index, const uint32_t node_l, const uint32_t node_r) noexcept {
        assert(node_index < nodes_.size());
        const size_t left_node_index = node_index * 2 + 1;
        const size_t right_node_index = left_node_index + 1;
        assert(right_node_index < nodes_.size());
        Node& node = nodes_[node_index];
        const value_t this_promise = node.promise;
        if (this_promise == kNoPromise) {
            return;
        }

        node.value *=
            node.has_cached_promise_x_count ? node.cached_promise_x_count : bin_pow(this_promise, node_r - node_l + 1);
        node.promise = kNoPromise;
        node.cached_promise_x_count = kNoPromise * 1;
        node.has_cached_promise_x_count = true;

        Node& nodes_left = nodes_[left_node_index];
        Node& nodes_right = nodes_[right_node_index];
        nodes_left.promise *= this_promise;
        nodes_left.has_cached_promise_x_count = false;
        nodes_right.promise *= this_promise;
        nodes_right.has_cached_promise_x_count = false;
    }

    [[nodiscard]] value_t GetRecImpl(const size_t node_index,
                                     const uint32_t node_l,
                                     const uint32_t node_r,
                                     const uint32_t query_l,
                                     const uint32_t query_r) noexcept {
        assert(node_index < nodes_.size());
        if (query_l == node_l && node_r == query_r) {
            Node& node = nodes_[node_index];
            if (!node.has_cached_promise_x_count) {
                node.cached_promise_x_count = bin_pow(node.promise, node_r - node_l + 1);
                node.has_cached_promise_x_count = true;
            }
            return node.value * node.cached_promise_x_count;
        }

        const uint32_t node_middle = (node_l + node_r) / 2;
        this->PushImpl(node_index, node_l, node_r);
        const size_t left_son = node_index * 2 + 1;
        const size_t right_son = left_son + 1;
        if (query_r <= node_middle) {
            return this->GetRecImpl(left_son, node_l, node_middle, query_l, query_r);
        }

        if (node_middle < query_l) {
            return this->GetRecImpl(right_son, node_middle + 1, node_r, query_l, query_r);
        }

        // query_l <= node_middle < query_r
        const value_t left_result = this->GetRecImpl(left_son, node_l, node_middle, query_l, node_middle);
        const value_t right_result = this->GetRecImpl(right_son, node_middle + 1, node_r, node_middle + 1, query_r);
        return left_result * right_result;
    }

    std::valarray<Node> nodes_;
    uint32_t query_l_{};
    uint32_t query_r_{};
    value_t value_{};
    uint32_t n_;
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
struct SegTreeHelper<value_t, GetOperation::product, UpdateOperation::set_equal> {
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
struct SegTreeHelper<value_t, GetOperation::product, UpdateOperation::multiply> {
    using type = ProdSegTreeMult<value_t>;
};

}  // namespace segtrees

template <UpdateOperation upd_op, GetOperation get_op, typename value_t = std::int64_t>
#if __cplusplus >= 202002L
    requires(sizeof(value_t) >= sizeof(int))
#endif
using SegTree = typename segtrees::SegTreeHelper<value_t, get_op, upd_op>::type;
