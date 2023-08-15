#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <iostream>

/// @brief See also https://www.youtube.com/watch?v=KFcpDTpoixo
class DSUTree final
{
public:
    // Rank heuristic
    struct TreeNode
    {
        TreeNode* parent;
        size_t rank;
    };

    static_assert(sizeof(TreeNode) == 2 * sizeof(void*));

    TreeNode* nodes_ = nullptr;
    size_t nodes_count_ = 0;

    DSUTree() = delete;

    inline explicit DSUTree(size_t nodes_count) noexcept(false) : nodes_count_{nodes_count}
    {
        if (nodes_count)
        {
            nodes_ = static_cast<TreeNode*>(operator new(sizeof(TreeNode) * nodes_count));
            std::memset(nodes_, 0, sizeof(TreeNode) * nodes_count);
        }
    }

    // O(n)
    inline DSUTree(const DSUTree& other) noexcept(false)
    {
        InitializeFrom(other);
    }

    // O(n)
    inline DSUTree& operator=(const DSUTree& other) noexcept(false)
    {
        InitializeFrom(other);
        return *this;
    }

    constexpr DSUTree(DSUTree&& other) noexcept : nodes_{other.nodes_}, nodes_count_{other.nodes_count_}
    {
        other.nodes_ = nullptr;
        other.nodes_count_ = 0;
    }

    constexpr DSUTree& operator=(DSUTree&& other) noexcept
    {
        nodes_ = other.nodes_;
        nodes_count_ = other.nodes_count_;
        other.nodes_ = nullptr;
        other.nodes_count_ = 0;
        return *this;
    }

    // O(log*(n))
    constexpr bool Equivalent(size_t node_x_index, size_t node_y_index) const noexcept
    {
        assert(node_x_index < nodes_count_ && node_y_index < nodes_count_);
        const TreeNode* node_x_root_ptr = FindRoot(&nodes_[node_x_index]);
        const TreeNode* node_y_root_ptr = FindRoot(&nodes_[node_y_index]);
        return node_x_root_ptr == node_y_root_ptr;
    }

    // O(log*(n))
    constexpr void Unite(size_t node_x_index, size_t node_y_index) noexcept
    {
        assert(node_x_index < nodes_count_ && node_y_index < nodes_count_);
        TreeNode* node_x_root_ptr = FindRoot(&nodes_[node_x_index]);
        TreeNode* node_y_root_ptr = FindRoot(&nodes_[node_y_index]);
        size_t node_x_root_rank = node_x_root_ptr->rank;
        size_t node_y_root_rank = node_y_root_ptr->rank;
        if (node_x_root_rank > node_y_root_rank)
        {
            node_y_root_ptr->parent = node_x_root_ptr;
        }
        else if (node_x_root_rank != node_y_root_rank)
        {// node_x_root_rank < node_y_root_rank
            node_x_root_ptr->parent = node_y_root_ptr;
        }
        else
        {// node_x_root_rank == node_y_root_rank
            node_x_root_ptr->parent = node_y_root_ptr;
            node_y_root_ptr->rank++;
        }
    }

    // O(log*(n))
    static constexpr TreeNode* FindRoot(TreeNode* node) noexcept
    {
        TreeNode* current_node = node;
        while (current_node->parent != nullptr)
        {
            current_node = current_node->parent;
        }

        // Now 'current_node' points to the root
        while (node != current_node)
        {
            TreeNode* next = node->parent;
            node->parent = current_node;
            node = next;
        }

        return current_node;
    }

    // O(log*(n))
    static constexpr const TreeNode* FindRoot(const TreeNode* node) noexcept
    {
        return FindRoot(const_cast<TreeNode*>(node));
    }

    ~DSUTree()
    {
        operator delete (nodes_);
        nodes_ = nullptr;
    }
private:
    // O(n)
    inline void InitializeFrom(const DSUTree& other) noexcept(false)
    {
        const size_t count = nodes_count_ = other.nodes_count_;
        if (count != 0)
        {
            TreeNode* const this_first_node = nodes_ = static_cast<TreeNode*>(operator new(sizeof(TreeNode) * count));
            const TreeNode* const other_first_node = other.nodes_;

            for (size_t i = 0; i < count; ++i) {
                if (const TreeNode* other_i_node_parent = other_first_node[i].parent)
                {
                    ptrdiff_t offset = other_i_node_parent - other_first_node;
                    this_first_node[i].parent = this_first_node + offset;
                }
                else
                {
                    this_first_node[i].parent = nullptr;
                }

                this_first_node[i].rank = other_first_node[i].rank;
            }
        }
    }
};

int main()
{
    DSUTree tree(10);
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
