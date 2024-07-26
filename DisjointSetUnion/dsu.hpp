#pragma once

#include <cassert>
#include <cstdint>
#include <cstddef>
#if __cplusplus >= 202002L
#include <concepts>
#endif
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace dsu_impl {

// Node with rank heuristic
struct dsu_node_t {
    dsu_node_t* parent_{};
    size_t rank_{};
};

// Weighted node with rank heuristic
struct wdsu_node_t {
    wdsu_node_t* parent_{};
    size_t rank_{};
    int64_t weight_{};
};

template <class node_type>
#if __cplusplus >= 202002L
    requires requires(node_type* node) {
        { node->parent_ } -> std::convertible_to<node_type*>;
    }
#endif
class dsu_base {
public:
    using node_t          = node_type;
    using value_type      = node_t;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using reference       = node_t&;
    using const_reference = const node_t&;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using allocator_type  = typename std::allocator<value_type>;

    constexpr size_type size() const noexcept {
        return nodes_count_;
    }
    constexpr size_type sets() const noexcept {
        return sets_count_;
    }

private:
    static constexpr allocator_type allocator() noexcept {
        return {};
    }

protected:
    constexpr dsu_base(size_t nodes_count)
        : nodes_(allocator().allocate(nodes_count))
        , nodes_count_(nodes_count)
        , sets_count_(nodes_count) {
        std::uninitialized_default_construct_n(nodes_, nodes_count_);
    }
    dsu_base(const dsu_base& other)
        : nodes_(allocator().allocate(other.nodes_count_))
        , nodes_count_(other.nodes_count_)
        , sets_count_(other.sets_count_) {
        std::uninitialized_copy_n(other.nodes_, other.nodes_count_, nodes_);
    }
    dsu_base& operator=(const dsu_base& other) {
        return *this = dsu_base(other);
    }
    constexpr dsu_base(dsu_base&& other) noexcept
        : nodes_(std::exchange(other.nodes_, nullptr))
        , nodes_count_(std::exchange(other.nodes_count_, 0))
        , sets_count_(std::exchange(other.sets_count_, 0))
        {}
    constexpr dsu_base& operator=(dsu_base&& other) noexcept {
        swap(other);
        return *this;
    }
    constexpr void swap(dsu_base& other) noexcept {
        std::swap(nodes_, other.nodes_);
        std::swap(nodes_count_, other.nodes_count_);
        std::swap(sets_count_, other.sets_count_);
    }
    constexpr const_pointer data() const noexcept {
        return nodes_;
    }
    constexpr pointer data() noexcept {
        return nodes_;
    }
    void reset() noexcept(std::is_nothrow_default_constructible_v<value_type> && std::is_nothrow_copy_assignable_v<value_type>) {
        std::fill_n(data(), size(), value_type{});
        sets_count_ = nodes_count_;
    }
    void clear() noexcept {
        std::destroy_n(nodes_, nodes_count_);
        allocator().deallocate(nodes_, nodes_count_);
        nodes_ = nullptr;
    }
    ~dsu_base() {
        clear();
    }

    static bool equal(pointer lhs_node, pointer rhs_node) noexcept {
        return findRoot(lhs_node) == findRoot(rhs_node);
    }
    static pointer findRoot(pointer node) noexcept {
        pointer current_node = node;
        assert(current_node != nullptr);
        while (current_node->parent_ != nullptr) {
            assert(current_node != current_node->parent_);
            current_node = current_node->parent_;
            assert(current_node != nullptr);
        }

        // Now 'current_node' points to the root
        while (node != current_node) {
            pointer next  = node->parent_;
            node->parent_ = current_node;
            node          = next;
        }

        return current_node;
    }

    pointer nodes_;
    size_type nodes_count_;
    size_type sets_count_;
};

}  // namespace dsu_impl

/// @brief See also https://www.youtube.com/watch?v=KFcpDTpoixo
class dsu_t : public dsu_impl::dsu_base<dsu_impl::dsu_node_t> {
    using base = dsu_impl::dsu_base<node_t>;

public:
    dsu_t() = delete;

    explicit dsu_t(size_t nodes_count) : base(nodes_count) {}

    dsu_t(const dsu_t& other) : base(other.nodes_count_) {
        node_t* const this_first_node = nodes_;

        for (size_t i = 0; i < other.nodes_count_; ++i) {
            const node_t* other_i_node_parent = other.nodes_[i].parent_;
            this_first_node[i].parent_ = other_i_node_parent != nullptr
                ? this_first_node + static_cast<difference_type>(other_i_node_parent - other.nodes_)
                : nullptr;
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
    friend constexpr void swap(dsu_t& lhs, dsu_t& rhs) noexcept {
        lhs.swap(rhs);
    }

    // O(log*(n)) = O(a(n))
    bool equal(size_t node_x_index, size_t node_y_index) noexcept {
        assert(node_x_index < nodes_count_ && node_y_index < nodes_count_);
        return base::equal(std::addressof(nodes_[node_x_index]), std::addressof(nodes_[node_y_index]));
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
};

/// @brief See also class dsu_t and
/// https://youtu.be/MmemGjxsZTc?si=NHMBw-KJmxeXvkNA
class weighted_dsu_t final : public dsu_impl::dsu_base<dsu_impl::wdsu_node_t> {
    using base = dsu_base<node_t>;

public:
    weighted_dsu_t() = delete;

    // O(n)
    explicit weighted_dsu_t(size_t nodes_count) : base(nodes_count) {}
    // O(n)
    explicit weighted_dsu_t(const std::vector<std::int64_t>& weights) : base(weights.size()) {
        for (node_t* nodes_iter = nodes_; const auto weight : weights) {
            nodes_iter->weight_ = weight;
            ++nodes_iter;
        }
    }
    // O(n)
    weighted_dsu_t(const weighted_dsu_t& other) : base(other.nodes_count_) {
        node_t* const this_first_node = nodes_;

        for (size_t i = 0; i < other.nodes_count_; ++i) {
            const node_t* other_i_node_parent = other.nodes_[i].parent_;
            this_first_node[i].parent_ = other_i_node_parent != nullptr
                ? this_first_node + static_cast<difference_type>(other_i_node_parent - other.nodes_)
                : nullptr;
            this_first_node[i].rank_   = other.nodes_[i].rank_;
            this_first_node[i].weight_ = other.nodes_[i].weight_;
        }
    }
    // O(n)
    weighted_dsu_t& operator=(const weighted_dsu_t& other) {
        return *this = weighted_dsu_t(other);
    }
    constexpr weighted_dsu_t(weighted_dsu_t&& other) noexcept : base(std::move(other)) {}
    constexpr weighted_dsu_t& operator=(weighted_dsu_t&& other) noexcept {
        base::operator=(std::move(other));
        return *this;
    }
    friend constexpr void swap(weighted_dsu_t& lhs, weighted_dsu_t& rhs) noexcept {
        lhs.swap(rhs);
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
        return findRoot(std::addressof(nodes_[node_index]))->weight_;
    }

    // O(log*(n))
    void addWeightInSet(size_t node_index, int64_t delta) noexcept {
        assert(node_index < nodes_count_);
        findRoot(std::addressof(nodes_[node_index]))->weight_ += delta;
    }

    // O(log*(n))
    void setWeightInSet(size_t node_index, int64_t weight) noexcept {
        assert(node_index < nodes_count_);
        findRoot(std::addressof(nodes_[node_index]))->weight_ = weight;
    }
};
