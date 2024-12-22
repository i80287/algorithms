#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

#include "../misc/config_macros.hpp"

#if CONFIG_HAS_CONCEPTS
#include <concepts>
#endif

#if CONFIG_HAS_AT_LEAST_CXX_20 && !defined(_GLIBCXX_DEBUG) && !defined(_GLIBCXX_ASSERTIONS)
#define CONSTEXPR_VECTOR constexpr
#else
#define CONSTEXPR_VECTOR inline
#endif

namespace dsu_impl {

using Weight = std::int64_t;

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
    Weight weight_{};
};

template <class node_type>
#if CONFIG_HAS_CONCEPTS
    requires requires(node_type node) {
        { node.parent_ } -> std::convertible_to<node_type*>;
        { node.set_size_ } -> std::convertible_to<std::size_t>;
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

    [[nodiscard]] constexpr size_type size() const noexcept {
        return nodes_.size();
    }
    [[nodiscard]] constexpr size_type sets_size() const noexcept {
        return sets_size_;
    }
    // O(1)
    [[nodiscard]] CONSTEXPR_VECTOR size_type set_size_of(size_type node_index) const noexcept {
        assert(node_index < size());
        const node_t& node       = nodes_[node_index];
        const_pointer parent_ptr = node.parent_;
        return parent_ptr != nullptr ? parent_ptr->set_size_ : node.set_size_;
    }
    // O(log*(n)) = O(a(n))
    [[nodiscard]] CONSTEXPR_VECTOR size_type set_size_of(size_type node_index) noexcept {
        assert(node_index < size());
        return find_root(node_index)->set_size_;
    }

protected:
    CONSTEXPR_VECTOR dsu_base(size_type nodes_count)
        : nodes_(nodes_count), sets_size_(nodes_count) {}
    CONSTEXPR_VECTOR dsu_base(const dsu_base& other)            = default;
    CONSTEXPR_VECTOR dsu_base& operator=(const dsu_base& other) = default;
    CONSTEXPR_VECTOR dsu_base(dsu_base&& other) noexcept
        : nodes_(std::move(other.nodes_)), sets_size_(std::exchange(other.sets_size_, 0)) {}
    CONSTEXPR_VECTOR dsu_base& operator=(dsu_base&& other) noexcept {
        swap(other);
        return *this;
    }
    CONSTEXPR_VECTOR ~dsu_base() = default;
    CONSTEXPR_VECTOR void swap(dsu_base& other) noexcept ATTRIBUTE_LIFETIME_BOUND {
        nodes_.swap(other.nodes_);
        std::swap(sets_size_, other.sets_size_);
    }
    [[nodiscard]] CONSTEXPR_VECTOR const_pointer data() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return nodes_.data();
    }
    [[nodiscard]] CONSTEXPR_VECTOR pointer data() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return nodes_.data();
    }
    CONSTEXPR_VECTOR void reset() noexcept(std::is_nothrow_default_constructible_v<value_type> &&
                                           std::is_nothrow_copy_assignable_v<value_type> &&
                                           std::is_nothrow_destructible_v<value_type>) {
        std::fill_n(data(), size(), value_type{});
        sets_size_ = size();
    }
    CONSTEXPR_VECTOR void clear() noexcept {
        nodes_.clear();
    }

    [[nodiscard]] CONSTEXPR_VECTOR bool equal(size_type lhs_node_index,
                                              size_type rhs_node_index) noexcept {
        return find_root(lhs_node_index) == find_root(rhs_node_index);
    }
    [[nodiscard]] CONSTEXPR_VECTOR pointer find_root(size_type node_index) noexcept {
        return find_root(std::addressof(nodes_[node_index]));
    }
    [[nodiscard]] static constexpr pointer find_root(pointer node) noexcept {
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

    [[nodiscard]]
    static CONSTEXPR_VECTOR dsu_t with_nodes_count(size_type nodes_count) {
        return dsu_t{nodes_count};
    }
    CONSTEXPR_VECTOR dsu_t(const dsu_t& other) : base(other) {
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
    CONSTEXPR_VECTOR dsu_t& operator=(const dsu_t& other) ATTRIBUTE_LIFETIME_BOUND {
        return *this = dsu_t(other);
    }
    CONSTEXPR_VECTOR dsu_t(dsu_t&& other) noexcept : base(std::move(other)) {}
    CONSTEXPR_VECTOR dsu_t& operator=(dsu_t&& other) noexcept ATTRIBUTE_LIFETIME_BOUND {
        base::operator=(std::move(other));
        return *this;
    }

    CONSTEXPR_VECTOR void swap(dsu_t& other) noexcept {
        base::swap(other);
    }
    friend CONSTEXPR_VECTOR void swap(dsu_t& lhs, dsu_t& rhs) noexcept {
        lhs.swap(rhs);
    }

    // O(log*(n)) = O(a(n))
    [[nodiscard]] CONSTEXPR_VECTOR bool equal(size_type node_x_index,
                                              size_type node_y_index) noexcept {
        assert(node_x_index < size() && node_y_index < size());
        return base::equal(node_x_index, node_y_index);
    }
    // O(log*(n)) = O(a(n))
    CONSTEXPR_VECTOR void unite(size_type node_x_index, size_type node_y_index) noexcept {
        assert(node_x_index < size() && node_y_index < size());
        node_t* node_x_root_ptr = base::find_root(node_x_index);
        node_t* node_y_root_ptr = base::find_root(node_y_index);
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

private:
    explicit CONSTEXPR_VECTOR dsu_t(size_type nodes_count) : base(nodes_count) {}
};

/// @brief See also class dsu_t and
/// https://youtu.be/MmemGjxsZTc?si=NHMBw-KJmxeXvkNA
class weighted_dsu_t final : public dsu_impl::dsu_base<dsu_impl::wdsu_node_t> {
    using base = dsu_base<node_t>;

public:
    weighted_dsu_t() = delete;

    // O(n)
    [[nodiscard]]
    static CONSTEXPR_VECTOR weighted_dsu_t with_nodes_count(size_type nodes_count) {
        return weighted_dsu_t{nodes_count};
    }

    using WeightsVec = std::vector<dsu_impl::Weight>;
    // O(n)
    [[nodiscard]]
    static CONSTEXPR_VECTOR weighted_dsu_t from_weights_vec(const WeightsVec& weights) {
        return weighted_dsu_t{weights};
    }

    // O(n)
    CONSTEXPR_VECTOR weighted_dsu_t(const weighted_dsu_t& other) : base(other) {
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
    CONSTEXPR_VECTOR weighted_dsu_t& operator=(const weighted_dsu_t& other)
        ATTRIBUTE_LIFETIME_BOUND {
        return *this = weighted_dsu_t(other);
    }
    CONSTEXPR_VECTOR weighted_dsu_t(weighted_dsu_t&& other) noexcept : base(std::move(other)) {}
    CONSTEXPR_VECTOR weighted_dsu_t& operator=(weighted_dsu_t&& other) noexcept {
        base::operator=(std::move(other));
        return *this;
    }

    CONSTEXPR_VECTOR void swap(weighted_dsu_t& other) noexcept {
        base::swap(other);
    }
    friend CONSTEXPR_VECTOR void swap(weighted_dsu_t& lhs, weighted_dsu_t& rhs) noexcept {
        lhs.swap(rhs);
    }

    // O(log*(n))
    [[nodiscard]] CONSTEXPR_VECTOR bool equal(size_type node_x_index,
                                              size_type node_y_index) noexcept {
        assert(node_x_index < size() && node_y_index < size());
        return base::equal(node_x_index, node_y_index);
    }
    // O(log*(n))
    CONSTEXPR_VECTOR void unite(size_type node_x_index, size_type node_y_index) noexcept {
        assert(node_x_index < size() && node_y_index < size());
        node_t* node_x_root_ptr = base::find_root(node_x_index);
        node_t* node_y_root_ptr = base::find_root(node_y_index);
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
    [[nodiscard]] CONSTEXPR_VECTOR int64_t get_weight_is_set(size_type node_index) noexcept {
        assert(node_index < size());
        return base::find_root(node_index)->weight_;
    }

    // O(log*(n))
    CONSTEXPR_VECTOR void add_weight_in_set(size_type node_index, int64_t delta) noexcept {
        assert(node_index < size());
        base::find_root(node_index)->weight_ += delta;
    }

    // O(log*(n))
    CONSTEXPR_VECTOR void set_weight_in_set(size_type node_index, int64_t weight) noexcept {
        assert(node_index < size());
        base::find_root(node_index)->weight_ = weight;
    }

private:
    explicit CONSTEXPR_VECTOR weighted_dsu_t(size_type nodes_count) : base(nodes_count) {}

    explicit CONSTEXPR_VECTOR weighted_dsu_t(const WeightsVec& weights) : base(weights.size()) {
        node_t* nodes_iter = data();
        for (const dsu_impl::Weight weight : weights) {
            nodes_iter->weight_ = weight;
            ++nodes_iter;
        }
    }
};
