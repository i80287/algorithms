#include <cassert>  // assert
#include <cstdint>  // size_t, int64_t
#include <cstring>  // std::memset
#if __cplusplus >= 202002L
#include <concepts>  // std::convertible_to
#endif
#include <utility>  // std::move
#include <vector>   // std::vector

// O(log*(n)) [log* - iterated logarithm, log*(n) = log2(log2(log2(...log2(n),
// until log2 > 0))), log*(65536) = 5, log*(2^65536) = 6]
template <typename NodeType>
#if __cplusplus >= 202002L
    requires requires(NodeType* node) {
        { node->parent_ } -> std::convertible_to<NodeType*>;
    }
#endif
static NodeType* FindRoot(NodeType* node) noexcept {
    NodeType* current_node = node;
    assert(current_node != nullptr);
    while (current_node->parent_ != nullptr) {
        assert(current_node != current_node->parent_);
        current_node = current_node->parent_;
        assert(current_node != nullptr);
    }

    // Now 'current_node' points to the root
    while (node != current_node) {
        NodeType* next = node->parent_;
        node->parent_ = current_node;
        node = next;
    }

    return current_node;
}

/// @brief See also https://www.youtube.com/watch?v=KFcpDTpoixo
class DisjointSetUnion {
public:
    // Node with rank heuristic
    struct DSUTreeNode {
        DSUTreeNode* parent_;
        size_t rank_;
    };

    DSUTreeNode* nodes_ = nullptr;
    size_t nodes_count_ = 0;

    DisjointSetUnion() = delete;

    // O(n)
    explicit DisjointSetUnion(size_t nodes_count)
        : nodes_count_{nodes_count} {
        if (nodes_count) {
            nodes_ = static_cast<DSUTreeNode*>(operator new(
                sizeof(DSUTreeNode) * nodes_count));
            std::memset(nodes_, 0, sizeof(DSUTreeNode) * nodes_count);
        }
    }

    // O(n)
    DisjointSetUnion(const DisjointSetUnion& other) : nodes_count_(other.nodes_count_) {
        const size_t count = nodes_count_;
        if (count == 0) {
            return;
        }

        DSUTreeNode* const this_first_node = nodes_ =
            static_cast<DSUTreeNode*>(operator new(sizeof(DSUTreeNode) *
                                                   count));

        for (size_t i = 0; i < count; ++i) {
            const DSUTreeNode* other_i_node_parent = other.nodes_[i].parent_;
            size_t parent_offset = static_cast<size_t>(other_i_node_parent - other.nodes_);
            this_first_node[i].parent_ = other_i_node_parent ? this_first_node + parent_offset : nullptr;
            this_first_node[i].rank_ = other.nodes_[i].rank_;
        }
    }

    // O(n)
    DisjointSetUnion& operator=(const DisjointSetUnion& other) {
        return *this = DisjointSetUnion(other);
    }

    // O(1)
    constexpr DisjointSetUnion(DisjointSetUnion&& other) noexcept
        : nodes_(other.nodes_), nodes_count_(other.nodes_count_) {
        other.nodes_ = nullptr;
        other.nodes_count_ = 0;
    }

    // O(1)
    DisjointSetUnion& operator=(DisjointSetUnion&& other) noexcept {
        auto nodes = other.nodes_;
        auto nodes_count = other.nodes_count_;
        this->~DisjointSetUnion();
        other.nodes_ = nullptr;
        other.nodes_count_ = 0;
        nodes_ = nodes;
        nodes_count_ = nodes_count;
        return *this;
    }

    // O(log*(n))
    bool Equivalent(size_t node_x_index,
                                     size_t node_y_index) noexcept {
        assert(node_x_index < nodes_count_ && node_y_index < nodes_count_);
        return FindRoot(&nodes_[node_x_index]) ==
               FindRoot(&nodes_[node_y_index]);
    }

    // O(log*(n))
    void Unite(size_t node_x_index,
                         size_t node_y_index) noexcept {
        assert(node_x_index < nodes_count_ && node_y_index < nodes_count_);
        DSUTreeNode* node_x_root_ptr = FindRoot(&nodes_[node_x_index]);
        DSUTreeNode* node_y_root_ptr = FindRoot(&nodes_[node_y_index]);
        if (node_x_root_ptr == node_y_root_ptr) {
            // Do not unite already united nodes so that for each root node:
            // root_node->parent_ == nullptr
            return;
        }

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

    // O(n)
    ~DisjointSetUnion() {
        operator delete(nodes_);
        nodes_ = nullptr;
    }
};

// Weighted node with rank heuristic
struct WeightedDSUTreeNode {
    WeightedDSUTreeNode* parent_;
    size_t rank_;
    int64_t weight_;
};

/// @brief See also class DisjointSetUnion and
/// https://youtu.be/MmemGjxsZTc?si=NHMBw-KJmxeXvkNA
class WeightedDisjointSetUnion {
public:
    WeightedDSUTreeNode* nodes_ = nullptr;
    size_t nodes_count_ = 0;

    WeightedDisjointSetUnion() = delete;

    // O(n)
    explicit WeightedDisjointSetUnion(size_t nodes_count) noexcept(false)
        : nodes_count_{nodes_count} {
        if (nodes_count) {
            nodes_ = static_cast<WeightedDSUTreeNode*>(operator new(
                sizeof(WeightedDSUTreeNode) * nodes_count));
            std::memset(nodes_, 0, sizeof(WeightedDSUTreeNode) * nodes_count);
        }
    }

    // O(n)
    explicit WeightedDisjointSetUnion(
        const std::vector<int64_t>& weights) noexcept(false)
        : nodes_count_{weights.size()} {
        if (nodes_count_) {
            nodes_ = static_cast<WeightedDSUTreeNode*>(operator new(
                sizeof(WeightedDSUTreeNode) * nodes_count_));
            std::memset(nodes_, 0, sizeof(WeightedDSUTreeNode) * nodes_count_);
            WeightedDSUTreeNode* nodes_iter = nodes_;
            for (auto iter = weights.begin(), end = weights.end(); iter != end;
                 ++iter, ++nodes_iter) {
                nodes_iter->weight_ = *iter;
            }
        }
    }

    // O(n)
    WeightedDisjointSetUnion(const WeightedDisjointSetUnion& other)
        : nodes_count_(other.nodes_count_) {
        const size_t count = nodes_count_;
        if (count == 0) {
            return;
        }

        WeightedDSUTreeNode* const this_first_node = nodes_ =
            static_cast<WeightedDSUTreeNode*>(operator new(
                sizeof(WeightedDSUTreeNode) * count));

        for (size_t i = 0; i < count; ++i) {
            const WeightedDSUTreeNode* other_i_node_parent =
                other.nodes_[i].parent_;
            size_t parent_offset =
                static_cast<size_t>(other_i_node_parent - other.nodes_);
            this_first_node[i].parent_ =
                other_i_node_parent ? this_first_node + parent_offset : nullptr;
            this_first_node[i].rank_ = other.nodes_[i].rank_;
            this_first_node[i].weight_ = other.nodes_[i].weight_;
        }
    }

    // O(n)
    WeightedDisjointSetUnion& operator=(
        const WeightedDisjointSetUnion& other) {
        return *this = WeightedDisjointSetUnion(other);
    }

    // O(1)
    constexpr WeightedDisjointSetUnion(WeightedDisjointSetUnion&& other) noexcept
        : nodes_(other.nodes_), nodes_count_(other.nodes_count_) {
        other.nodes_ = nullptr;
        other.nodes_count_ = 0;
    }

    // O(1)
    WeightedDisjointSetUnion& operator=(WeightedDisjointSetUnion&& other) noexcept {
        auto nodes = other.nodes_;
        auto nodes_count = other.nodes_count_;
        this->~WeightedDisjointSetUnion();
        other.nodes_ = nullptr;
        other.nodes_count_ = 0;
        nodes_ = nodes;
        nodes_count_ = nodes_count;
        return *this;
    }

    // O(log*(n))
    bool Equivalent(size_t node_x_index,
                                     size_t node_y_index) noexcept {
        assert(node_x_index < nodes_count_ && node_y_index < nodes_count_);
        return FindRoot(&nodes_[node_x_index]) ==
               FindRoot(&nodes_[node_y_index]);
    }

    // O(log*(n))
    void Unite(size_t node_x_index, size_t node_y_index) noexcept {
        assert(node_x_index < nodes_count_ && node_y_index < nodes_count_);
        WeightedDSUTreeNode* node_x_root_ptr = FindRoot(&nodes_[node_x_index]);
        WeightedDSUTreeNode* node_y_root_ptr = FindRoot(&nodes_[node_y_index]);
        if (node_x_root_ptr == node_y_root_ptr) {
            // Do not unite already united nodes so that for each root node:
            // root_node->parent_ == nullptr
            return;
        }

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
    int64_t GetWeightInSet(size_t node_index) noexcept {
        assert(node_index < nodes_count_);
        return FindRoot(&nodes_[node_index])->weight_;
    }

    // O(log*(n))
    void AddWeightInSet(size_t node_index, int64_t delta) {
        assert(node_index < nodes_count_);
        FindRoot(&nodes_[node_index])->weight_ += delta;
    }

    // O(log*(n))
    void SetWeightInSet(size_t node_index, int64_t weight) {
        assert(node_index < nodes_count_);
        FindRoot(&nodes_[node_index])->weight_ = weight;
    }

    // O(n)
    ~WeightedDisjointSetUnion() {
        operator delete(nodes_);
        nodes_ = nullptr;
        nodes_count_ = 0;
    }
};

#include <iostream>

void ConsoleTest() {
    DisjointSetUnion tree(10);
    std::cout << std::boolalpha;

    std::cout << "0 eq 1: " << tree.Equivalent(0, 1) << '\n';
    tree.Unite(0, 1);
    std::cout << "0 eq 1: " << tree.Equivalent(0, 1) << '\n';

    tree.Unite(1, 2);
    tree.Unite(2, 3);
    tree.Unite(4, 3);
    tree.Unite(4, 9);
    std::cout << "1 eq 9: " << tree.Equivalent(1, 9) << '\n';
    std::cout << "1 eq 8: " << tree.Equivalent(1, 8) << '\n';

    tree.Unite(8, 9);

    std::cout << "1 eq 8: " << tree.Equivalent(1, 8) << '\n';
}

void TestDSU() {
    constexpr size_t N = 40;
    DisjointSetUnion tree(N);

    for (size_t i = 1; i < N; i++) {
        assert(!tree.Equivalent(i - 1, i));
    }

    for (size_t i = 0; i < N; i++) {
        assert(tree.Equivalent(i, i));
    }

    tree.Unite(0, 1);
    tree.Unite(2, 3);
    tree.Unite(0, 3);
    for (size_t i = 0; i <= 3; i++) {
        for (size_t j = 0; j <= 3; j++) {
            assert(tree.Equivalent(i, j));
        }
    }

    for (size_t i = 4; i < N; i++) {
        assert(!tree.Equivalent(i - 1, i));
    }

    /*
     *     .--37---.
     *    /   /     \
     *  35   36     39
     *  /            \
     * 34            38
     *
     */
    tree.Unite(34, 35);
    assert(tree.Equivalent(34, 35));
    assert(!tree.Equivalent(35, 36));
    assert(!tree.Equivalent(36, 37));
    assert(!tree.Equivalent(37, 38));
    assert(!tree.Equivalent(38, 39));
    tree.Unite(36, 37);
    assert(tree.Equivalent(34, 35));
    assert(!tree.Equivalent(35, 36));
    assert(tree.Equivalent(36, 37));
    assert(!tree.Equivalent(37, 38));
    assert(!tree.Equivalent(38, 39));
    tree.Unite(38, 39);
    assert(tree.Equivalent(34, 35));
    assert(!tree.Equivalent(35, 36));
    assert(tree.Equivalent(36, 37));
    assert(!tree.Equivalent(37, 38));
    assert(tree.Equivalent(38, 39));
    tree.Unite(35, 37);
    assert(tree.Equivalent(34, 35));
    assert(tree.Equivalent(35, 36));
    assert(tree.Equivalent(36, 37));
    assert(!tree.Equivalent(37, 38));
    assert(tree.Equivalent(38, 39));
    tree.Unite(37, 38);
    assert(tree.Equivalent(34, 35));
    assert(tree.Equivalent(35, 36));
    assert(tree.Equivalent(36, 37));
    assert(tree.Equivalent(37, 38));
    assert(tree.Equivalent(38, 39));

    for (size_t i = 1; i < N; i++) {
        tree.Unite(i - 1, i);
    }

    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            assert(tree.Equivalent(i, j));
        }
    }

    puts("DisjointSetUnion tests passed");
}

void TestWightedDSU() {
    constexpr size_t N = 40;
    WeightedDisjointSetUnion tree(N);

    for (size_t i = 1; i < N; i++) {
        assert(!tree.Equivalent(i - 1, i));
    }

    for (size_t i = 0; i < N; i++) {
        assert(tree.Equivalent(i, i));
    }

    tree.Unite(0, 1);
    tree.Unite(2, 3);
    tree.Unite(0, 3);
    for (size_t i = 0; i <= 3; i++) {
        for (size_t j = 0; j <= 3; j++) {
            assert(tree.Equivalent(i, j));
        }
    }

    tree.AddWeightInSet(0, 10);
    tree.AddWeightInSet(2, 10);
    for (size_t i = 0; i <= 3; i++) {
        assert(tree.GetWeightInSet(i) == 20);
    }

    tree.SetWeightInSet(0, 10);
    for (size_t i = 0; i <= 3; i++) {
        assert(tree.GetWeightInSet(i) == 10);
    }

    for (size_t i = 4; i < N; i++) {
        assert(!tree.Equivalent(i - 1, i));
    }

    /*
     *     .--37---.
     *    /   /     \
     *  35   36     39
     *  /            \
     * 34            38
     *
     */
    tree.Unite(34, 35);
    tree.AddWeightInSet(34, 2);
    assert(tree.Equivalent(34, 35));
    assert(!tree.Equivalent(35, 36));
    assert(!tree.Equivalent(36, 37));
    assert(!tree.Equivalent(37, 38));
    assert(!tree.Equivalent(38, 39));
    assert(tree.GetWeightInSet(34) == 2);
    assert(tree.GetWeightInSet(35) == 2);
    assert(tree.GetWeightInSet(36) == 0);
    assert(tree.GetWeightInSet(37) == 0);
    assert(tree.GetWeightInSet(38) == 0);
    assert(tree.GetWeightInSet(39) == 0);
    tree.Unite(36, 37);
    tree.AddWeightInSet(37, 3);
    assert(tree.Equivalent(34, 35));
    assert(!tree.Equivalent(35, 36));
    assert(tree.Equivalent(36, 37));
    assert(!tree.Equivalent(37, 38));
    assert(!tree.Equivalent(38, 39));
    assert(tree.GetWeightInSet(34) == 2);
    assert(tree.GetWeightInSet(35) == 2);
    assert(tree.GetWeightInSet(36) == 3);
    assert(tree.GetWeightInSet(37) == 3);
    assert(tree.GetWeightInSet(38) == 0);
    assert(tree.GetWeightInSet(39) == 0);
    tree.Unite(38, 39);
    tree.AddWeightInSet(38, 4);
    assert(tree.Equivalent(34, 35));
    assert(!tree.Equivalent(35, 36));
    assert(tree.Equivalent(36, 37));
    assert(!tree.Equivalent(37, 38));
    assert(tree.Equivalent(38, 39));
    assert(tree.GetWeightInSet(34) == 2);
    assert(tree.GetWeightInSet(35) == 2);
    assert(tree.GetWeightInSet(36) == 3);
    assert(tree.GetWeightInSet(37) == 3);
    assert(tree.GetWeightInSet(38) == 4);
    assert(tree.GetWeightInSet(39) == 4);
    tree.Unite(35, 37);
    assert(tree.Equivalent(34, 35));
    assert(tree.Equivalent(35, 36));
    assert(tree.Equivalent(36, 37));
    assert(!tree.Equivalent(37, 38));
    assert(tree.Equivalent(38, 39));
    assert(tree.GetWeightInSet(34) == 5);
    assert(tree.GetWeightInSet(35) == 5);
    assert(tree.GetWeightInSet(36) == 5);
    assert(tree.GetWeightInSet(37) == 5);
    assert(tree.GetWeightInSet(38) == 4);
    assert(tree.GetWeightInSet(39) == 4);
    tree.Unite(37, 38);
    assert(tree.Equivalent(34, 35));
    assert(tree.Equivalent(35, 36));
    assert(tree.Equivalent(36, 37));
    assert(tree.Equivalent(37, 38));
    assert(tree.Equivalent(38, 39));
    assert(tree.GetWeightInSet(34) == 9);
    assert(tree.GetWeightInSet(35) == 9);
    assert(tree.GetWeightInSet(36) == 9);
    assert(tree.GetWeightInSet(37) == 9);
    assert(tree.GetWeightInSet(38) == 9);
    assert(tree.GetWeightInSet(39) == 9);
    for (size_t i = 34; i <= 39; i++) {
        for (size_t j = 34; j <= 39; j++) {
            assert(tree.Equivalent(i, j));
        }
    }

    for (size_t i = 1; i < N; i++) {
        tree.Unite(i - 1, i);
    }

    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            assert(tree.Equivalent(i, j));
        }

        assert(tree.GetWeightInSet(i) == 10 + 9);
    }

    std::vector<int64_t> vec{1, 2, 4, 8, 16, 32, 64};
    size_t n = vec.size();
    WeightedDisjointSetUnion wdsu{vec};
    for (size_t i = 0; i < n; i++) {
        assert(wdsu.GetWeightInSet(i) == vec[i]);
    }

    wdsu.Unite(0, 1);
    wdsu.Unite(2, 3);
    wdsu.Unite(0, 2);
    int64_t sum = vec[0] + vec[1] + vec[2] + vec[3];
    for (size_t i = 0; i <= 3; i++) {
        assert(wdsu.GetWeightInSet(i) == sum);
    }

    for (size_t i = 1; i < n; i++) {
        wdsu.Unite(i - 1, i);
    }

    for (size_t i = 4; i < n; i++) {
        sum += vec[i];
    }

    for (size_t i = 0; i < n; i++) {
        assert(wdsu.GetWeightInSet(i) == sum);
    }

    puts("WeightedDisjointSetUnion tests passed");
}

int main() {
    TestDSU();
    TestWightedDSU();
}
