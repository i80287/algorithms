#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#if __cplusplus >= 202002L
#include <concepts>
#endif
#include <type_traits>
#include <utility>
#include <vector>

namespace dsu_impl {

// Node with rank heuristic
struct dsu_node_t {
    dsu_node_t* parent_{};
    std::size_t rank_{};
    std::size_t set_size_ = 1;  // node itself
};

// Weighted node with rank heuristic
struct wdsu_node_t {
    wdsu_node_t* parent_{};
    std::size_t rank_{};
    std::size_t set_size_ = 1;  // node itself
    std::int64_t weight_{};
};

template <class node_type>
#if __cplusplus >= 202002L
    requires requires(node_type* node) {
        { node->parent_ } -> std::convertible_to<node_type*>;
        { node->set_size_ } -> std::convertible_to<std::size_t>;
    }
#endif
class dsu_base {
    using container = std::vector<node_type>;

public:
    using node_t          = node_type;
    using value_type      = typename container::value_type;
    using pointer         = typename container::pointer;
    using const_pointer   = typename container::const_pointer;
    using reference       = typename container::reference;
    using const_reference = typename container::const_reference;
    using size_type       = typename container::size_type;
    using difference_type = typename container::difference_type;
    using allocator_type  = typename container::allocator_type;

    constexpr size_type size() const noexcept {
        return nodes_.size();
    }
    constexpr size_type sets_size() const noexcept {
        return sets_size_;
    }
    // O(1)
    constexpr size_type set_size_of(size_type node_index) const noexcept {
        assert(node_index < size());
        const node_t& node = nodes_[node_index];
        const_pointer parent_ptr = node.parent_;
        return parent_ptr != nullptr ? parent_ptr->set_size_ : node.set_size_;
    }
    // O(log*(n)) = O(a(n))
    constexpr size_type set_size_of(size_type node_index) noexcept {
        return findRoot(node_index)->set_size_;
    }

protected:
    constexpr dsu_base(size_type nodes_count) : nodes_(nodes_count), sets_size_(nodes_count) {}
    dsu_base(const dsu_base& other)            = default;
    dsu_base& operator=(const dsu_base& other) = default;
    constexpr dsu_base(dsu_base&& other) noexcept
        : nodes_(std::move(other.nodes_)), sets_size_(std::exchange(other.sets_size_, 0)) {}
    constexpr dsu_base& operator=(dsu_base&& other) noexcept {
        swap(other);
        return *this;
    }
    constexpr void swap(dsu_base& other) noexcept {
        nodes_.swap(other.nodes_);
        std::swap(sets_size_, other.sets_size_);
    }
    constexpr const_pointer data() const noexcept {
        return nodes_.data();
    }
    constexpr pointer data() noexcept {
        return nodes_.data();
    }
    void reset() noexcept(std::is_nothrow_default_constructible_v<value_type> &&
                          std::is_nothrow_copy_assignable_v<value_type>) {
        std::fill_n(data(), size(), value_type{});
        sets_size_ = size();
    }
    void clear() noexcept {
        nodes_.clear();
    }

    constexpr bool equal(size_type lhs_node_index, size_type rhs_node_index) noexcept {
        return findRoot(lhs_node_index) == findRoot(rhs_node_index);
    }
    constexpr pointer findRoot(size_type node_index) noexcept {
        return findRoot(std::addressof(nodes_[node_index]));
    }
    static constexpr pointer findRoot(pointer node) noexcept {
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

    container nodes_;
    size_type sets_size_;
};

}  // namespace dsu_impl

/// @brief See also https://www.youtube.com/watch?v=KFcpDTpoixo
class dsu_t final : public dsu_impl::dsu_base<dsu_impl::dsu_node_t> {
    using base = dsu_impl::dsu_base<node_t>;

public:
    dsu_t() = delete;

    explicit dsu_t(size_type nodes_count) : base(nodes_count) {}

    dsu_t(const dsu_t& other) : base(other) {
        node_t* const this_first_node        = data();
        const node_t* const other_first_node = other.data();

        for (size_type i = 0; i < other.size(); ++i) {
            const node_t* other_i_node_parent = other_first_node[i].parent_;
            this_first_node[i].parent_ =
                other_i_node_parent != nullptr
                    ? this_first_node +
                          static_cast<difference_type>(other_i_node_parent - other_first_node)
                    : nullptr;
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
    constexpr void swap(dsu_t& other) noexcept {
        base::swap(other);
    }
    friend constexpr void swap(dsu_t& lhs, dsu_t& rhs) noexcept {
        lhs.swap(rhs);
    }

    // O(log*(n)) = O(a(n))
    bool equal(size_type node_x_index, size_type node_y_index) noexcept {
        assert(node_x_index < size() && node_y_index < size());
        return base::equal(node_x_index, node_y_index);
    }
    // O(log*(n)) = O(a(n))
    void unite(size_type node_x_index, size_type node_y_index) noexcept {
        assert(node_x_index < size() && node_y_index < size());
        node_t* node_x_root_ptr = findRoot(node_x_index);
        node_t* node_y_root_ptr = findRoot(node_y_index);
        if (node_x_root_ptr == node_y_root_ptr) {
            // Do not unite already united nodes so that for each root node:
            // root_node->parent_ == nullptr
            return;
        }
        sets_size_--;
        size_type node_x_root_rank = node_x_root_ptr->rank_;
        size_type node_y_root_rank = node_y_root_ptr->rank_;
        if (node_x_root_rank > node_y_root_rank) {
            node_y_root_ptr->parent_ = node_x_root_ptr;
            node_x_root_ptr->set_size_ += node_y_root_ptr->set_size_;
        } else {
            node_x_root_ptr->parent_ = node_y_root_ptr;
            node_y_root_ptr->set_size_ += node_x_root_ptr->set_size_;
            if (node_x_root_rank == node_y_root_rank) {
                node_y_root_ptr->rank_++;
            }
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
    explicit weighted_dsu_t(size_type nodes_count) : base(nodes_count) {}
    // O(n)
    explicit weighted_dsu_t(const std::vector<std::int64_t>& weights) : base(weights.size()) {
        for (node_t* nodes_iter = data(); const auto weight : weights) {
            nodes_iter->weight_ = weight;
            ++nodes_iter;
        }
    }
    // O(n)
    weighted_dsu_t(const weighted_dsu_t& other) : base(other) {
        node_t* const this_first_node        = data();
        const node_t* const other_first_node = other.data();

        for (size_type i = 0; i < other.size(); ++i) {
            const node_t* other_i_node_parent = other_first_node[i].parent_;
            this_first_node[i].parent_ =
                other_i_node_parent != nullptr
                    ? this_first_node +
                          static_cast<difference_type>(other_i_node_parent - other_first_node)
                    : nullptr;
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
    constexpr void swap(weighted_dsu_t& other) noexcept {
        base::swap(other);
    }
    friend constexpr void swap(weighted_dsu_t& lhs, weighted_dsu_t& rhs) noexcept {
        lhs.swap(rhs);
    }

    // O(log*(n))
    bool equal(size_type node_x_index, size_type node_y_index) noexcept {
        assert(node_x_index < size() && node_y_index < size());
        return base::equal(node_x_index, node_y_index);
    }
    // O(log*(n))
    void unite(size_type node_x_index, size_type node_y_index) noexcept {
        assert(node_x_index < size() && node_y_index < size());
        node_t* node_x_root_ptr = findRoot(node_x_index);
        node_t* node_y_root_ptr = findRoot(node_y_index);
        if (node_x_root_ptr == node_y_root_ptr) {
            // Do not unite already united nodes so that for each root node:
            // root_node->parent_ == nullptr
            return;
        }
        sets_size_--;
        size_type node_x_root_rank = node_x_root_ptr->rank_;
        size_type node_y_root_rank = node_y_root_ptr->rank_;

        if (node_x_root_rank > node_y_root_rank) {
            node_y_root_ptr->parent_ = node_x_root_ptr;
            node_x_root_ptr->set_size_ += node_y_root_ptr->set_size_;
            node_x_root_ptr->weight_ += node_y_root_ptr->weight_;
        } else {
            node_x_root_ptr->parent_ = node_y_root_ptr;
            node_y_root_ptr->set_size_ += node_x_root_ptr->set_size_;
            node_y_root_ptr->weight_ += node_x_root_ptr->weight_;
            if (node_x_root_rank == node_y_root_rank) {
                node_y_root_ptr->rank_++;
            }
        }
    }

    // O(log*(n))
    int64_t getWeightInSet(size_type node_index) noexcept {
        assert(node_index < size());
        return findRoot(node_index)->weight_;
    }

    // O(log*(n))
    void addWeightInSet(size_type node_index, int64_t delta) noexcept {
        assert(node_index < size());
        findRoot(node_index)->weight_ += delta;
    }

    // O(log*(n))
    void setWeightInSet(size_type node_index, int64_t weight) noexcept {
        assert(node_index < size());
        findRoot(node_index)->weight_ = weight;
    }
};
