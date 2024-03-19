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
static NodeType* findRoot(NodeType* node) noexcept {
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
            nodes_ = static_cast<DSUTreeNode*>(operator new(sizeof(DSUTreeNode) * nodes_count));
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
        other.nodes_ = nullptr;
        other.nodes_count_ = 0;
        this->~DisjointSetUnion();
        nodes_ = nodes;
        nodes_count_ = nodes_count;
        return *this;
    }

    // O(log*(n))
    bool equal(size_t node_x_index,
                                     size_t node_y_index) noexcept {
        assert(node_x_index < nodes_count_ && node_y_index < nodes_count_);
        return findRoot(&nodes_[node_x_index]) ==
               findRoot(&nodes_[node_y_index]);
    }

    // O(log*(n))
    void unite(size_t node_x_index,
                         size_t node_y_index) noexcept {
        assert(node_x_index < nodes_count_ && node_y_index < nodes_count_);
        DSUTreeNode* node_x_root_ptr = findRoot(&nodes_[node_x_index]);
        DSUTreeNode* node_y_root_ptr = findRoot(&nodes_[node_y_index]);
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

    void resetData() {
        std::memset(nodes_, 0, sizeof(DSUTreeNode) * nodes_count_);
    }

    // O(n)
    ~DisjointSetUnion() {
        operator delete(nodes_);
        nodes_ = nullptr;
    }
};

/// @brief See also class DisjointSetUnion and
/// https://youtu.be/MmemGjxsZTc?si=NHMBw-KJmxeXvkNA
class WeightedDisjointSetUnion {
public:
    // Weighted node with rank heuristic
    struct WeightedDSUTreeNode {
        WeightedDSUTreeNode* parent_;
        size_t rank_;
        int64_t weight_;
    };

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
        other.nodes_ = nullptr;
        other.nodes_count_ = 0;
        this->~WeightedDisjointSetUnion();
        nodes_ = nodes;
        nodes_count_ = nodes_count;
        return *this;
    }

    // O(log*(n))
    bool equal(size_t node_x_index,
                                     size_t node_y_index) noexcept {
        assert(node_x_index < nodes_count_ && node_y_index < nodes_count_);
        return findRoot(&nodes_[node_x_index]) ==
               findRoot(&nodes_[node_y_index]);
    }

    // O(log*(n))
    void unite(size_t node_x_index, size_t node_y_index) noexcept {
        assert(node_x_index < nodes_count_ && node_y_index < nodes_count_);
        WeightedDSUTreeNode* node_x_root_ptr = findRoot(&nodes_[node_x_index]);
        WeightedDSUTreeNode* node_y_root_ptr = findRoot(&nodes_[node_y_index]);
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
        std::memset(nodes_, 0, sizeof(WeightedDSUTreeNode) * nodes_count_);
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

    std::cout << "0 eq 1: " << tree.equal(0, 1) << '\n';
    tree.unite(0, 1);
    std::cout << "0 eq 1: " << tree.equal(0, 1) << '\n';

    tree.unite(1, 2);
    tree.unite(2, 3);
    tree.unite(4, 3);
    tree.unite(4, 9);
    std::cout << "1 eq 9: " << tree.equal(1, 9) << '\n';
    std::cout << "1 eq 8: " << tree.equal(1, 8) << '\n';

    tree.unite(8, 9);

    std::cout << "1 eq 8: " << tree.equal(1, 8) << '\n';
}

void TestDSU() {
    constexpr size_t N = 40;
    DisjointSetUnion tree(N);

    for (size_t i = 1; i < N; i++) {
        assert(!tree.equal(i - 1, i));
    }

    for (size_t i = 0; i < N; i++) {
        assert(tree.equal(i, i));
    }

    tree.unite(0, 1);
    tree.unite(2, 3);
    tree.unite(0, 3);
    for (size_t i = 0; i <= 3; i++) {
        for (size_t j = 0; j <= 3; j++) {
            assert(tree.equal(i, j));
        }
    }

    for (size_t i = 4; i < N; i++) {
        assert(!tree.equal(i - 1, i));
    }

    /*
     *     .--37---.
     *    /   /     \
     *  35   36     39
     *  /            \
     * 34            38
     *
     */
    tree.unite(34, 35);
    assert(tree.equal(34, 35));
    assert(!tree.equal(35, 36));
    assert(!tree.equal(36, 37));
    assert(!tree.equal(37, 38));
    assert(!tree.equal(38, 39));
    tree.unite(36, 37);
    assert(tree.equal(34, 35));
    assert(!tree.equal(35, 36));
    assert(tree.equal(36, 37));
    assert(!tree.equal(37, 38));
    assert(!tree.equal(38, 39));
    tree.unite(38, 39);
    assert(tree.equal(34, 35));
    assert(!tree.equal(35, 36));
    assert(tree.equal(36, 37));
    assert(!tree.equal(37, 38));
    assert(tree.equal(38, 39));
    tree.unite(35, 37);
    assert(tree.equal(34, 35));
    assert(tree.equal(35, 36));
    assert(tree.equal(36, 37));
    assert(!tree.equal(37, 38));
    assert(tree.equal(38, 39));
    tree.unite(37, 38);
    assert(tree.equal(34, 35));
    assert(tree.equal(35, 36));
    assert(tree.equal(36, 37));
    assert(tree.equal(37, 38));
    assert(tree.equal(38, 39));

    for (size_t i = 1; i < N; i++) {
        tree.unite(i - 1, i);
    }

    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            assert(tree.equal(i, j));
        }
    }

    puts("DisjointSetUnion tests passed");
}

void TestWightedDSU() {
    constexpr size_t N = 40;
    WeightedDisjointSetUnion tree(N);

    for (size_t i = 1; i < N; i++) {
        assert(!tree.equal(i - 1, i));
    }

    for (size_t i = 0; i < N; i++) {
        assert(tree.equal(i, i));
    }

    tree.unite(0, 1);
    tree.unite(2, 3);
    tree.unite(0, 3);
    for (size_t i = 0; i <= 3; i++) {
        for (size_t j = 0; j <= 3; j++) {
            assert(tree.equal(i, j));
        }
    }

    tree.addWeightInSet(0, 10);
    tree.addWeightInSet(2, 10);
    for (size_t i = 0; i <= 3; i++) {
        assert(tree.getWeightInSet(i) == 20);
    }

    tree.setWeightInSet(0, 10);
    for (size_t i = 0; i <= 3; i++) {
        assert(tree.getWeightInSet(i) == 10);
    }

    for (size_t i = 4; i < N; i++) {
        assert(!tree.equal(i - 1, i));
    }

    /*
     *     .--37---.
     *    /   /     \
     *  35   36     39
     *  /            \
     * 34            38
     *
     */
    tree.unite(34, 35);
    tree.addWeightInSet(34, 2);
    assert(tree.equal(34, 35));
    assert(!tree.equal(35, 36));
    assert(!tree.equal(36, 37));
    assert(!tree.equal(37, 38));
    assert(!tree.equal(38, 39));
    assert(tree.getWeightInSet(34) == 2);
    assert(tree.getWeightInSet(35) == 2);
    assert(tree.getWeightInSet(36) == 0);
    assert(tree.getWeightInSet(37) == 0);
    assert(tree.getWeightInSet(38) == 0);
    assert(tree.getWeightInSet(39) == 0);
    tree.unite(36, 37);
    tree.addWeightInSet(37, 3);
    assert(tree.equal(34, 35));
    assert(!tree.equal(35, 36));
    assert(tree.equal(36, 37));
    assert(!tree.equal(37, 38));
    assert(!tree.equal(38, 39));
    assert(tree.getWeightInSet(34) == 2);
    assert(tree.getWeightInSet(35) == 2);
    assert(tree.getWeightInSet(36) == 3);
    assert(tree.getWeightInSet(37) == 3);
    assert(tree.getWeightInSet(38) == 0);
    assert(tree.getWeightInSet(39) == 0);
    tree.unite(38, 39);
    tree.addWeightInSet(38, 4);
    assert(tree.equal(34, 35));
    assert(!tree.equal(35, 36));
    assert(tree.equal(36, 37));
    assert(!tree.equal(37, 38));
    assert(tree.equal(38, 39));
    assert(tree.getWeightInSet(34) == 2);
    assert(tree.getWeightInSet(35) == 2);
    assert(tree.getWeightInSet(36) == 3);
    assert(tree.getWeightInSet(37) == 3);
    assert(tree.getWeightInSet(38) == 4);
    assert(tree.getWeightInSet(39) == 4);
    tree.unite(35, 37);
    assert(tree.equal(34, 35));
    assert(tree.equal(35, 36));
    assert(tree.equal(36, 37));
    assert(!tree.equal(37, 38));
    assert(tree.equal(38, 39));
    assert(tree.getWeightInSet(34) == 5);
    assert(tree.getWeightInSet(35) == 5);
    assert(tree.getWeightInSet(36) == 5);
    assert(tree.getWeightInSet(37) == 5);
    assert(tree.getWeightInSet(38) == 4);
    assert(tree.getWeightInSet(39) == 4);
    tree.unite(37, 38);
    assert(tree.equal(34, 35));
    assert(tree.equal(35, 36));
    assert(tree.equal(36, 37));
    assert(tree.equal(37, 38));
    assert(tree.equal(38, 39));
    assert(tree.getWeightInSet(34) == 9);
    assert(tree.getWeightInSet(35) == 9);
    assert(tree.getWeightInSet(36) == 9);
    assert(tree.getWeightInSet(37) == 9);
    assert(tree.getWeightInSet(38) == 9);
    assert(tree.getWeightInSet(39) == 9);
    for (size_t i = 34; i <= 39; i++) {
        for (size_t j = 34; j <= 39; j++) {
            assert(tree.equal(i, j));
        }
    }

    for (size_t i = 1; i < N; i++) {
        tree.unite(i - 1, i);
    }

    for (size_t i = 0; i < N; i++) {
        for (size_t j = 0; j < N; j++) {
            assert(tree.equal(i, j));
        }

        assert(tree.getWeightInSet(i) == 10 + 9);
    }

    std::vector<int64_t> vec{1, 2, 4, 8, 16, 32, 64};
    size_t n = vec.size();
    WeightedDisjointSetUnion wdsu{vec};
    for (size_t i = 0; i < n; i++) {
        assert(wdsu.getWeightInSet(i) == vec[i]);
    }

    wdsu.unite(0, 1);
    wdsu.unite(2, 3);
    wdsu.unite(0, 2);
    int64_t sum = vec[0] + vec[1] + vec[2] + vec[3];
    for (size_t i = 0; i <= 3; i++) {
        assert(wdsu.getWeightInSet(i) == sum);
    }

    for (size_t i = 1; i < n; i++) {
        wdsu.unite(i - 1, i);
    }

    for (size_t i = 4; i < n; i++) {
        sum += vec[i];
    }

    for (size_t i = 0; i < n; i++) {
        assert(wdsu.getWeightInSet(i) == sum);
    }

    puts("WeightedDisjointSetUnion tests passed");
}

int main() {
    TestDSU();
    TestWightedDSU();
}
