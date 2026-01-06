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
        : nodes_(other.nodes_), nodes_count_(other.nodes_count_), sets_count_(other.sets_count_) {
        other.nodes_ = nullptr;
        other.nodes_count_ = 0;
        other.sets_count_ = 0;
    }

    // O(1)
    dsu_base& operator=(dsu_base&& other) noexcept {
        auto nodes = other.nodes_;
        auto nodes_count = other.nodes_count_;
        auto sets_count = other.sets_count_;
        other.nodes_ = nullptr;
        other.nodes_count_ = 0;
        other.sets_count_ = 0;
        this->~dsu_base();
        nodes_ = nodes;
        nodes_count_ = nodes_count;
        sets_count_ = sets_count;
        return *this;
    }

    constexpr const node_t* getNodes() const noexcept {
        return nodes_;
    }

    constexpr node_t* getNodes() noexcept {
        return nodes_;
    }

    constexpr size_t size() const noexcept {
        return nodes_count_;
    }

    constexpr size_t sets() const noexcept {
        return sets_count_;
    }

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
            node_t* next = node->parent_;
            node->parent_ = current_node;
            node = next;
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
    using base = dsu_impl::dsu_base<node_t>;

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
            size_t parent_offset = static_cast<size_t>(other_i_node_parent - other.nodes_);
            this_first_node[i].parent_ = other_i_node_parent ? this_first_node + parent_offset : nullptr;
            this_first_node[i].rank_ = other.nodes_[i].rank_;
        }
    }

    dsu_t& operator=(const dsu_t& other) {
        return *this = dsu_t(other);
    }

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

#include <set>

using vertex_t = int64_t;
using edge_t = std::pair<vertex_t, vertex_t>;

template <class Comp>
static std::vector<edge_t> kruskal_mst(const std::set<edge_t, Comp>& edges, size_t n) {
    dsu_t dsu(n);
    std::vector<edge_t> mst;
    mst.reserve(n - 1);
    for (auto [u, v] : edges) {
        if (!dsu.equal(u, v)) {
            mst.push_back({u, v});
            dsu.unite(u, v);
        }
    }
    return mst;
}
