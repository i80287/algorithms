#pragma once

#include <cassert>  // assert
#include <cstdint>  // size_t, int64_t
#include <cstring>  // std::memset
#if __cplusplus >= 202002L
#include <concepts>  // std::convertible_to
#endif
#include <utility>  // std::move
#include <vector>   // std::vector

namespace dsu_impl {

// Node with rank heuristic
struct dsu_node_t {
    dsu_node_t* parent_;
    size_t rank_;
};

// Weighted node with rank heuristic
struct wdsu_node_t {
    wdsu_node_t* parent_;
    size_t rank_;
    int64_t weight_;
};

template <class node_t>
#if __cplusplus >= 202002L
    requires requires(node_t* node) {
        { node->parent_ } -> std::convertible_to<node_t*>;
    }
#endif
class dsu_base {
public:
    constexpr dsu_base(size_t nodes_count) noexcept
        : nodes_(nullptr), nodes_count_(nodes_count), sets_count_(nodes_count) {}

    dsu_base(const dsu_base& other) = delete;

    dsu_base& operator=(const dsu_base& other) = delete;

    // O(1)
    constexpr dsu_base(dsu_base&& other) noexcept
        : nodes_(other.nodes_),
          nodes_count_(other.nodes_count_),
          sets_count_(other.sets_count_) {
        other.nodes_       = nullptr;
        other.nodes_count_ = 0;
        other.sets_count_  = 0;
    }

    // O(1)
    dsu_base& operator=(dsu_base&& other) noexcept {
        auto nodes         = other.nodes_;
        auto nodes_count   = other.nodes_count_;
        auto sets_count    = other.sets_count_;
        other.nodes_       = nullptr;
        other.nodes_count_ = 0;
        other.sets_count_  = 0;
        this->~dsu_base();
        nodes_       = nodes;
        nodes_count_ = nodes_count;
        sets_count_  = sets_count;
        return *this;
    }

    constexpr const node_t* getNodes() const noexcept { return nodes_; }

    constexpr node_t* getNodes() noexcept { return nodes_; }

    constexpr size_t size() const noexcept { return nodes_count_; }

    constexpr size_t sets() const noexcept { return sets_count_; }

    ~dsu_base() {
        operator delete(nodes_);
        nodes_ = nullptr;
    }

protected:
    static node_t* findRoot(node_t* node) noexcept {
        node_t* current_node = node;
        assert(current_node != nullptr);
        while (current_node->parent_ != nullptr) {
            assert(current_node != current_node->parent_);
            current_node = current_node->parent_;
            assert(current_node != nullptr);
        }

        // Now 'current_node' points to the root
        while (node != current_node) {
            node_t* next  = node->parent_;
            node->parent_ = current_node;
            node          = next;
        }

        return current_node;
    }

    node_t* nodes_;
    size_t nodes_count_;
    size_t sets_count_;
};

}  // namespace dsu_impl

/// @brief See also https://www.youtube.com/watch?v=KFcpDTpoixo
class dsu_t : public dsu_impl::dsu_base<dsu_impl::dsu_node_t> {
    using node_t = dsu_impl::dsu_node_t;
    using base   = dsu_impl::dsu_base<node_t>;

public:
    dsu_t() = delete;

    explicit dsu_t(size_t nodes_count) : base(nodes_count) {
        nodes_ = static_cast<node_t*>(operator new(sizeof(node_t) * nodes_count));
        std::memset(nodes_, 0, sizeof(node_t) * nodes_count);
    }

    dsu_t(const dsu_t& other) : base(other.nodes_count_) {
        node_t* const this_first_node = nodes_ =
            static_cast<node_t*>(operator new(sizeof(node_t) * other.nodes_count_));

        for (size_t i = 0; i < other.nodes_count_; ++i) {
            const node_t* other_i_node_parent = other.nodes_[i].parent_;
            size_t parent_offset =
                static_cast<size_t>(other_i_node_parent - other.nodes_);
            this_first_node[i].parent_ =
                other_i_node_parent ? this_first_node + parent_offset : nullptr;
            this_first_node[i].rank_ = other.nodes_[i].rank_;
        }
    }

    dsu_t& operator=(const dsu_t& other) { return *this = dsu_t(other); }

    constexpr dsu_t(dsu_t&& other) noexcept : base(std::move(other)) {}

    dsu_t& operator=(dsu_t&& other) noexcept {
        base::operator=(std::move(other));
        return *this;
    }

    // O(log*(n)) = O(a(n))
    bool equal(size_t node_x_index, size_t node_y_index) noexcept {
        assert(node_x_index < nodes_count_ && node_y_index < nodes_count_);
        return findRoot(&nodes_[node_x_index]) == findRoot(&nodes_[node_y_index]);
    }

    // O(log*(n)) = O(a(n))
    void unite(size_t node_x_index, size_t node_y_index) noexcept {
        assert(node_x_index < nodes_count_ && node_y_index < nodes_count_);
        node_t* node_x_root_ptr = findRoot(&nodes_[node_x_index]);
        node_t* node_y_root_ptr = findRoot(&nodes_[node_y_index]);
        if (node_x_root_ptr == node_y_root_ptr) {
            // Do not unite already united nodes so that for each root node:
            // root_node->parent_ == nullptr
            return;
        }
        sets_count_--;
        size_t node_x_root_rank = node_x_root_ptr->rank_;
        size_t node_y_root_rank = node_y_root_ptr->rank_;
        if (node_x_root_rank > node_y_root_rank) {
            node_y_root_ptr->parent_ = node_x_root_ptr;
        } else if (node_x_root_rank != node_y_root_rank) {
            // node_x_root_rank < node_y_root_rank
            node_x_root_ptr->parent_ = node_y_root_ptr;
        } else {
            // node_x_root_rank == node_y_root_rank
            node_x_root_ptr->parent_ = node_y_root_ptr;
            node_y_root_ptr->rank_++;
        }
    }

    void resetData() {
        std::memset(nodes_, 0, sizeof(node_t) * nodes_count_);
        sets_count_ = nodes_count_;
    }
};

/// @brief See also class dsu_t and
/// https://youtu.be/MmemGjxsZTc?si=NHMBw-KJmxeXvkNA
class weighted_dsu_t : dsu_impl::dsu_base<dsu_impl::wdsu_node_t> {
    using node_t = dsu_impl::wdsu_node_t;
    using base   = dsu_base<node_t>;

public:
    weighted_dsu_t() = delete;

    // O(n)
    explicit weighted_dsu_t(size_t nodes_count) : base(nodes_count) {
        nodes_ = static_cast<node_t*>(operator new(sizeof(node_t) * nodes_count));
        std::memset(nodes_, 0, sizeof(node_t) * nodes_count);
    }

    // O(n)
    explicit weighted_dsu_t(const std::vector<int64_t>& weights) noexcept(false)
        : base(weights.size()) {
        nodes_ = static_cast<node_t*>(operator new(sizeof(node_t) * nodes_count_));
        std::memset(nodes_, 0, sizeof(node_t) * nodes_count_);
        node_t* nodes_iter = nodes_;
        for (auto iter = weights.begin(), end = weights.end(); iter != end;
             ++iter, ++nodes_iter) {
            nodes_iter->weight_ = *iter;
        }
    }

    // O(n)
    weighted_dsu_t(const weighted_dsu_t& other) : base(other.nodes_count_) {
        node_t* const this_first_node = nodes_ =
            static_cast<node_t*>(operator new(sizeof(node_t) * other.nodes_count_));

        for (size_t i = 0; i < other.nodes_count_; ++i) {
            const node_t* other_i_node_parent = other.nodes_[i].parent_;
            size_t parent_offset =
                static_cast<size_t>(other_i_node_parent - other.nodes_);
            this_first_node[i].parent_ =
                other_i_node_parent ? this_first_node + parent_offset : nullptr;
            this_first_node[i].rank_   = other.nodes_[i].rank_;
            this_first_node[i].weight_ = other.nodes_[i].weight_;
        }
    }

    // O(n)
    weighted_dsu_t& operator=(const weighted_dsu_t& other) {
        return *this = weighted_dsu_t(other);
    }

    constexpr weighted_dsu_t(weighted_dsu_t&& other) noexcept : base(std::move(other)) {}

    weighted_dsu_t& operator=(weighted_dsu_t&& other) noexcept {
        base::operator=(std::move(other));
        return *this;
    }

    // O(log*(n))
    bool equal(size_t node_x_index, size_t node_y_index) noexcept {
        assert(node_x_index < nodes_count_ && node_y_index < nodes_count_);
        return findRoot(&nodes_[node_x_index]) == findRoot(&nodes_[node_y_index]);
    }

    // O(log*(n))
    void unite(size_t node_x_index, size_t node_y_index) noexcept {
        assert(node_x_index < nodes_count_ && node_y_index < nodes_count_);
        node_t* node_x_root_ptr = findRoot(&nodes_[node_x_index]);
        node_t* node_y_root_ptr = findRoot(&nodes_[node_y_index]);
        if (node_x_root_ptr == node_y_root_ptr) {
            // Do not unite already united nodes so that for each root node:
            // root_node->parent_ == nullptr
            return;
        }
        sets_count_--;
        size_t node_x_root_rank = node_x_root_ptr->rank_;
        size_t node_y_root_rank = node_y_root_ptr->rank_;
        if (node_x_root_rank > node_y_root_rank) {
            node_y_root_ptr->parent_ = node_x_root_ptr;
            node_x_root_ptr->weight_ += node_y_root_ptr->weight_;
        } else if (node_x_root_rank != node_y_root_rank) {
            // node_x_root_rank < node_y_root_rank
            node_x_root_ptr->parent_ = node_y_root_ptr;
            node_y_root_ptr->weight_ += node_x_root_ptr->weight_;
        } else {
            // node_x_root_rank == node_y_root_rank
            node_x_root_ptr->parent_ = node_y_root_ptr;
            node_y_root_ptr->weight_ += node_x_root_ptr->weight_;
            node_y_root_ptr->rank_++;
        }
    }

    // O(log*(n))
    int64_t getWeightInSet(size_t node_index) noexcept {
        assert(node_index < nodes_count_);
        return findRoot(&nodes_[node_index])->weight_;
    }

    // O(log*(n))
    void addWeightInSet(size_t node_index, int64_t delta) {
        assert(node_index < nodes_count_);
        findRoot(&nodes_[node_index])->weight_ += delta;
    }

    // O(log*(n))
    void setWeightInSet(size_t node_index, int64_t weight) {
        assert(node_index < nodes_count_);
        findRoot(&nodes_[node_index])->weight_ = weight;
    }

    void resetData() {
        std::memset(nodes_, 0, sizeof(node_t) * nodes_count_);
        sets_count_ = nodes_count_;
    }
};
