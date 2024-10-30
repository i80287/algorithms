module;

#include <cassert>
#include <compare>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <source_location>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "config_macros.hpp"

export module rbtree;

#ifdef RBTREE_DEBUG
#ifdef NDEBUG
#error("Can't assert rbtree invariants in release mode")
#endif
#define RBTREE_ASSERT_INVARIANT(expr) assert(expr)
#else
#define RBTREE_ASSERT_INVARIANT(expr)
#endif

enum class Color : std::size_t { kRed = 0, kBlack };

struct NodeBase {
    friend class RBTreeContainerImpl;

    NodeBase *left_   = nullptr;
    NodeBase *right_  = nullptr;
    NodeBase *parent_ = nullptr;
    Color color_      = Color::kRed;

protected:
    constexpr NodeBase() noexcept = default;

    constexpr ~NodeBase() = default;

public:
    NodeBase(const NodeBase &) = delete;

    NodeBase &operator=(const NodeBase &) = delete;

    constexpr NodeBase(NodeBase &&other) noexcept
        : left_(std::exchange(other.left_, {})),
          right_(std::exchange(other.right_, {})),
          parent_(std::exchange(other.parent_, {})),
          color_(std::exchange(other.color_, {})) {}

    constexpr NodeBase &operator=(NodeBase &&other) noexcept ATTRIBUTE_LIFETIME_BOUND {
        left_   = std::exchange(other.left_, {});
        right_  = std::exchange(other.right_, {});
        parent_ = std::exchange(other.parent_, {});
        color_  = std::exchange(other.color_, {});
        return *this;
    }

    constexpr void swap(NodeBase &other) noexcept {
        std::swap(left_, other.left_);
        std::swap(right_, other.right_);
        std::swap(parent_, other.parent_);
        std::swap(color_, other.color_);
    }

    [[nodiscard]] ATTRIBUTE_RETURNS_NONNULL constexpr const NodeBase *LeftMostNode() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        const NodeBase *node = this;
        while (node->left_ != nullptr) {
            node = node->left_;
        }
        return node;
    }

    [[nodiscard]] ATTRIBUTE_RETURNS_NONNULL constexpr NodeBase *LeftMostNode() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        NodeBase *node = this;
        while (node->left_ != nullptr) {
            node = node->left_;
        }
        return node;
    }

    [[nodiscard]] ATTRIBUTE_RETURNS_NONNULL constexpr const NodeBase *RightMostNode() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        const NodeBase *node = this;
        while (node->right_ != nullptr) {
            node = node->right_;
        }
        return node;
    }

    [[nodiscard]] ATTRIBUTE_RETURNS_NONNULL constexpr NodeBase *RightMostNode() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        NodeBase *node = this;
        while (node->right_ != nullptr) {
            node = node->right_;
        }
        return node;
    }

    [[nodiscard]] ATTRIBUTE_RETURNS_NONNULL constexpr const NodeBase *FarthestParent()
        const noexcept ATTRIBUTE_LIFETIME_BOUND {
        const NodeBase *node = this;
        while (node->parent_ != nullptr) {
            node = node->parent_;
        }
        return node;
    }

    [[nodiscard]] ATTRIBUTE_RETURNS_NONNULL constexpr NodeBase *FarthestParent() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        NodeBase *node = this;
        while (node->parent_ != nullptr) {
            node = node->parent_;
        }
        return node;
    }

    [[nodiscard]] constexpr const NodeBase *OtherSibling(const NodeBase *child) const noexcept {
        RBTREE_ASSERT_INVARIANT(child == left_ ^ child == right_);
        return (left_ == child) ? right_ : left_;
    }

    [[nodiscard]] constexpr NodeBase *OtherSibling(const NodeBase *child) noexcept {
        RBTREE_ASSERT_INVARIANT(child == left_ ^ child == right_);
        return (left_ == child) ? right_ : left_;
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr const NodeBase *Parent() const noexcept {
        return parent_;
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr NodeBase *Parent() noexcept {
        return parent_;
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr const NodeBase *Left() const noexcept {
        return left_;
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr NodeBase *Left() noexcept {
        return left_;
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr const NodeBase *Right() const noexcept {
        return right_;
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr NodeBase *Right() noexcept {
        return right_;
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr Color Color() const noexcept {
        return color_;
    }
};

template <typename T>
struct Node final : NodeBase {
    T key_{};

    explicit constexpr Node(const T &key) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : key_(key) {}

    explicit constexpr Node(T &&key) noexcept(std::is_nothrow_move_constructible_v<T>)
        : key_(std::move(key)) {}

    [[nodiscard]] ATTRIBUTE_PURE constexpr const Node *Left() const noexcept {
        return static_cast<const Node *>(left_);
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr Node *Left() noexcept {
        return static_cast<Node *>(left_);
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr const Node *Right() const noexcept {
        return static_cast<const Node *>(right_);
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr Node *Right() noexcept {
        return static_cast<Node *>(right_);
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr const Node *Parent() const noexcept {
        return static_cast<const Node *>(parent_);
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr Node *Parent() noexcept {
        return static_cast<Node *>(parent_);
    }
};

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-extension"
#define RBTREE_NONNULL_ALL_ARGS
#define RBTREE_ATTRIBUTE_NONNULL(...)
#else
#define RBTREE_NONNULL_ALL_ARGS       ATTRIBUTE_NONNULL_ALL_ARGS
#define RBTREE_ATTRIBUTE_NONNULL(...) ATTRIBUTE_NONNULL(__VA_ARGS__)

#endif

class RBTreeContainerImpl {
protected:
    constexpr RBTreeContainerImpl() noexcept {
        GetBeginUnchecked() = GetEndUnchecked();
    }

public:
    RBTreeContainerImpl(const RBTreeContainerImpl &)            = delete;  // TODO:
    RBTreeContainerImpl &operator=(const RBTreeContainerImpl &) = delete;  // TODO:
protected:
    constexpr RBTreeContainerImpl(RBTreeContainerImpl &&other) noexcept
        : fake_root_(std::move(other.fake_root_)) {
        SynchronizeInvariantsOnMove();
    }

    RBTreeContainerImpl &operator=(RBTreeContainerImpl &&other) noexcept ATTRIBUTE_LIFETIME_BOUND {
        swap(other);
        return *this;
    }

    constexpr void swap(RBTreeContainerImpl &other) noexcept {
        fake_root_.swap(other.fake_root_);
        SynchronizeInvariantsOnMove();
        other.SynchronizeInvariantsOnMove();
    }

    [[nodiscard]] constexpr NodeBase *&Begin() noexcept ATTRIBUTE_LIFETIME_BOUND {
        RBTREE_ASSERT_INVARIANT(GetBeginUnchecked() != nullptr);
        const auto lhs = Size() >= 1 && GetBeginUnchecked() != GetEndUnchecked();
        const auto rhs = Size() == 0 && GetBeginUnchecked() == GetEndUnchecked();
        RBTREE_ASSERT_INVARIANT((lhs) ^ (rhs));
        RBTREE_ASSERT_INVARIANT(GetBeginUnchecked() == GetEndUnchecked() ||
                                GetBeginUnchecked()->left_ == nullptr);
        return GetBeginUnchecked();
    }

    [[nodiscard]] constexpr const NodeBase *Begin() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        RBTREE_ASSERT_INVARIANT(GetBeginUnchecked() != nullptr);
        const auto lhs = Size() >= 1 && GetBeginUnchecked() != GetEndUnchecked();
        const auto rhs = Size() == 0 && GetBeginUnchecked() == GetEndUnchecked();
        RBTREE_ASSERT_INVARIANT((lhs) ^ (rhs));
        RBTREE_ASSERT_INVARIANT(GetBeginUnchecked() == GetEndUnchecked() ||
                                GetBeginUnchecked()->left_ == nullptr);
        return GetBeginUnchecked();
    }

    [[nodiscard]] constexpr NodeBase *PreRootNil() noexcept ATTRIBUTE_LIFETIME_BOUND {
        RBTREE_ASSERT_INVARIANT(GetRootUnchecked() == nullptr ||
                                GetRootUnchecked()->parent_ == GetPreRootNilUnchecked());
        RBTREE_ASSERT_INVARIANT((GetRootUnchecked() == nullptr && Size() == 0) ^
                                (GetRootUnchecked() != nullptr && Size() >= 1));
        return GetPreRootNilUnchecked();
    }

    [[nodiscard]] constexpr const NodeBase *PreRootNil() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        RBTREE_ASSERT_INVARIANT(GetRootUnchecked() == nullptr ||
                                GetRootUnchecked()->parent_ == GetPreRootNilUnchecked());
        RBTREE_ASSERT_INVARIANT((GetRootUnchecked() == nullptr && Size() == 0) ^
                                (GetRootUnchecked() != nullptr && Size() >= 1));
        return GetPreRootNilUnchecked();
    }

    [[nodiscard]] constexpr NodeBase *&Root() noexcept ATTRIBUTE_LIFETIME_BOUND {
        RBTREE_ASSERT_INVARIANT(GetRootUnchecked() == nullptr ||
                                GetRootUnchecked()->color_ == Color::kBlack);
        RBTREE_ASSERT_INVARIANT(GetRootUnchecked() == nullptr ||
                                GetRootUnchecked()->parent_ == PreRootNil());
        RBTREE_ASSERT_INVARIANT((GetRootUnchecked() == nullptr && Size() == 0) ^
                                (GetRootUnchecked() != nullptr && Size() >= 1));
        return GetRootUnchecked();
    }

    [[nodiscard]] constexpr const NodeBase *Root() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        RBTREE_ASSERT_INVARIANT(GetRootUnchecked() == nullptr ||
                                GetRootUnchecked()->color_ == Color::kBlack);
        RBTREE_ASSERT_INVARIANT(GetRootUnchecked() == nullptr ||
                                GetRootUnchecked()->parent_ == PreRootNil());
        RBTREE_ASSERT_INVARIANT((GetRootUnchecked() == nullptr && Size() == 0) ^
                                (GetRootUnchecked() != nullptr && Size() >= 1));
        return GetRootUnchecked();
    }

    [[nodiscard]] constexpr NodeBase *End() noexcept ATTRIBUTE_LIFETIME_BOUND {
        RBTREE_ASSERT_INVARIANT((Size() == 0 && GetBeginUnchecked() == GetEndUnchecked()) ^
                                (Size() >= 1 && GetBeginUnchecked() != GetEndUnchecked()));
        return GetEndUnchecked();
    }

    [[nodiscard]] constexpr const NodeBase *End() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        RBTREE_ASSERT_INVARIANT((Size() == 0 && GetBeginUnchecked() == GetEndUnchecked()) ^
                                (Size() >= 1 && GetBeginUnchecked() != GetEndUnchecked()));
        return GetEndUnchecked();
    }

    [[nodiscard]] constexpr NodeBase *&Last() noexcept ATTRIBUTE_LIFETIME_BOUND {
        RBTREE_ASSERT_INVARIANT(Size() >= 1);
        RBTREE_ASSERT_INVARIANT((Size() >= 2) ^ (GetBeginUnchecked() == GetLastUnchecked() &&
                                                 GetLastUnchecked() == GetRootUnchecked()));
        RBTREE_ASSERT_INVARIANT(GetLastUnchecked() != nullptr);
        RBTREE_ASSERT_INVARIANT(GetLastUnchecked() != GetPreRootNilUnchecked());
        RBTREE_ASSERT_INVARIANT(GetLastUnchecked()->right_ == nullptr);
        return GetLastUnchecked();
    }

    [[nodiscard]] constexpr const NodeBase *Last() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        RBTREE_ASSERT_INVARIANT(Size() >= 1);
        RBTREE_ASSERT_INVARIANT((Size() >= 2) ^ (GetBeginUnchecked() == GetLastUnchecked() &&
                                                 GetLastUnchecked() == GetRootUnchecked()));
        RBTREE_ASSERT_INVARIANT(GetLastUnchecked() != nullptr);
        RBTREE_ASSERT_INVARIANT(GetLastUnchecked() != GetPreRootNilUnchecked());
        RBTREE_ASSERT_INVARIANT(GetLastUnchecked()->right_ == nullptr);
        return GetLastUnchecked();
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr std::size_t Size() const noexcept {
        return static_cast<std::size_t>(fake_root_.color_);
    }

    RBTREE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_write, 2)
    ATTRIBUTE_ACCESS(read_write, 4)
    void InsertNode(NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER node_parent, const bool is_left,
                    NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER node) noexcept {
        RBTREE_ASSERT_INVARIANT(node != nullptr);
        node->parent_ = node_parent;
        RBTREE_ASSERT_INVARIANT(node->left_ == nullptr);
        RBTREE_ASSERT_INVARIANT(node->right_ == nullptr);
        RBTREE_ASSERT_INVARIANT(node->color_ == Color::kRed);
        RBTREE_ASSERT_INVARIANT(node_parent != nullptr);
        RBTREE_ASSERT_INVARIANT(Size() >= 1 || node_parent == PreRootNil());
        RBTREE_ASSERT_INVARIANT(Size() >= 1 || !is_left);
        if (is_left) {
            MakeLeftOnInsertion(node_parent, node);
        } else {
            MakeRightOnInsertion(node_parent, node);
        }
        IncrementSize();
        RebalanceAfterInsertion(node);
    }

    RBTREE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_only, 2)
    constexpr void ExtractNode(NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER node) noexcept {
        RBTREE_ASSERT_INVARIANT(Size() >= 1);
        RBTREE_ASSERT_INVARIANT(Begin() != End());
        RBTREE_ASSERT_INVARIANT(node != nullptr);
        RBTREE_ASSERT_INVARIANT(node != PreRootNil());
        NodeBase *const node_parent = node->parent_;
        RBTREE_ASSERT_INVARIANT(node_parent != nullptr);
        NodeBase *const node_left_child  = node->Left();
        NodeBase *const node_right_child = node->Right();
        NodeBase *propagated_node{};
        Color color = node->Color();
        RBTREE_ASSERT_INVARIANT(color == Color::kRed || color == Color::kBlack);
        if (node_left_child == nullptr || node_right_child == nullptr) {
            propagated_node = node_left_child == nullptr ? node_right_child : node_left_child;
            RBTREE_ASSERT_INVARIANT(propagated_node == nullptr ||
                                    propagated_node->color_ == Color::kRed);
            if (node_left_child == nullptr && node == GetBeginUnchecked()) {
                GetBeginUnchecked() =
                    node_right_child != nullptr ? node_right_child->LeftMostNode() : node_parent;
            }
            if (node_right_child == nullptr && node == GetLastUnchecked()) {
                GetLastUnchecked() =
                    node_left_child != nullptr ? node_left_child->RightMostNode() : node_parent;
            }
            Transplant(node, propagated_node);
        } else {
            RBTREE_ASSERT_INVARIANT(node != Begin());
            RBTREE_ASSERT_INVARIANT(node != Last());
            NodeBase *const node_successor = node_right_child->LeftMostNode();
            RBTREE_ASSERT_INVARIANT(node_successor != nullptr);
            RBTREE_ASSERT_INVARIANT(node_successor != PreRootNil());
            RBTREE_ASSERT_INVARIANT(node_successor->left_ == nullptr);
            color = node_successor->color_;
            RBTREE_ASSERT_INVARIANT(color == Color::kRed || color == Color::kBlack);
            propagated_node = node_successor->right_;
            RBTREE_ASSERT_INVARIANT(propagated_node == nullptr ||
                                    propagated_node->parent_ == node_successor);
            RBTREE_ASSERT_INVARIANT(propagated_node == nullptr ||
                                    propagated_node->color_ == Color::kRed);
            if (node_successor != node_right_child) {
                Transplant(node_successor, propagated_node);
                node_successor->right_    = node_right_child;
                node_right_child->parent_ = node_successor;
            }
            Transplant(node, node_successor);
            node_successor->left_    = node_left_child;
            node_left_child->parent_ = node_successor;
            node_successor->color_   = node->Color();
        }

        DecrementSize();
        if (color != Color::kRed && propagated_node != nullptr) {
            RBTREE_ASSERT_INVARIANT(color == Color::kBlack);
            RebalanceAfterExtraction(propagated_node);
        }
        RBTREE_ASSERT_INVARIANT(Size() == 0 || Root() != nullptr);
    }

private:
    constexpr void IncrementSize() noexcept {
        static_assert(
            std::is_same_v<std::underlying_type_t<decltype(fake_root_.color_)>, std::size_t>);
        fake_root_.color_ = static_cast<Color>(static_cast<std::size_t>(fake_root_.color_) + 1);
    }

    constexpr void DecrementSize() noexcept {
        RBTREE_ASSERT_INVARIANT(static_cast<std::size_t>(fake_root_.color_) > 0);
        fake_root_.color_ = static_cast<Color>(static_cast<std::size_t>(fake_root_.color_) - 1);
    }

    constexpr void SynchronizeInvariantsOnMove() noexcept {
        if (auto *root = GetRootUnchecked(); root != nullptr) {
            root->parent_ = GetPreRootNilUnchecked();
            RBTREE_ASSERT_INVARIANT(Size() >= 1);
        } else {
            GetBeginUnchecked() = GetEndUnchecked();
            RBTREE_ASSERT_INVARIANT(Size() == 0);
        }

        // Access methods in order to trigger internal invariants checks
        RBTREE_ASSERT_INVARIANT(Begin() == Begin());
        RBTREE_ASSERT_INVARIANT(End() == End());
        RBTREE_ASSERT_INVARIANT(Root() == Root());
        RBTREE_ASSERT_INVARIANT(PreRootNil() == PreRootNil());
    }

    // clang-format off

    [[nodiscard]] constexpr NodeBase *&GetRootUnchecked() noexcept
    ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.parent_;
    }

    [[nodiscard]] constexpr const NodeBase *GetRootUnchecked()
    const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.parent_;
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr NodeBase *
    GetPreRootNilUnchecked() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return std::addressof(fake_root_);
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr const NodeBase *
    GetPreRootNilUnchecked() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return std::addressof(fake_root_);
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr NodeBase *&
    GetBeginUnchecked() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.right_;
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr const NodeBase *
    GetBeginUnchecked() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.right_;
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr NodeBase *
    GetEndUnchecked() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return GetPreRootNilUnchecked();
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr const NodeBase *GetEndUnchecked()
    const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return GetPreRootNilUnchecked();
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr NodeBase *&
    GetLastUnchecked() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.left_;
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr const NodeBase *
    GetLastUnchecked() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.left_;
    }

    // clang-format on

    RBTREE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_write, 1)
    ATTRIBUTE_ACCESS(read_write, 2)
    static constexpr void LeftRotate(NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER node,
                                     NodeBase *&root) noexcept {
        /**
         *  parent_parent (or nullptr)
         *       |
         *     parent
         *      /  \
         *  subt1  node
         *         /  \
         *     subt2  subt3
         *
         *    parent_parent (or nullptr)
         *         |
         *        node
         *        /  \
         *   parent  subt3
         *    /  \
         * subt1 subt2
         */
        RBTREE_ASSERT_INVARIANT(node != nullptr);
        RBTREE_ASSERT_INVARIANT(node != root);
        RBTREE_ASSERT_INVARIANT(node != root->parent_);
        NodeBase *const parent = node->parent_;
        RBTREE_ASSERT_INVARIANT(parent != nullptr);
        RBTREE_ASSERT_INVARIANT(parent->right_ == node);
        NodeBase *const parent_parent = parent->parent_;

        parent->right_ = node->left_;
        if (auto *node_left = node->left_; node_left != nullptr) {
            node_left->parent_ = parent;
        }

        node->left_     = parent;
        parent->parent_ = node;

        RBTREE_ASSERT_INVARIANT(parent_parent != nullptr);
        node->parent_ = parent_parent;
        if (parent == root) [[unlikely]] {
            root = node;
        } else if (parent_parent->left_ == parent) {
            parent_parent->left_ = node;
        } else {
            RBTREE_ASSERT_INVARIANT(parent_parent->right_ == parent);
            parent_parent->right_ = node;
        }
    }

    RBTREE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_write, 1)
    ATTRIBUTE_ACCESS(read_write, 2)
    static constexpr void RightRotate(NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER node,
                                      NodeBase *&root) noexcept {
        /**
         *    parent_parent
         *         |
         *       parent_
         *        /  \
         *     node subt1
         *     /  \
         * subt3  subt2
         *
         *    parent_parent
         *         |
         *        node
         *        /  \
         *    subt3 parent_
         *           /  \
         *        subt2 subt1
         */
        RBTREE_ASSERT_INVARIANT(node != nullptr);
        NodeBase *const parent = node->parent_;
        RBTREE_ASSERT_INVARIANT(parent != nullptr);
        RBTREE_ASSERT_INVARIANT(parent->left_ == node);
        NodeBase *const parent_parent = parent->parent_;

        parent->left_ = node->right_;
        if (auto *node_right = node->right_; node_right != nullptr) {
            node_right->parent_ = parent;
        }

        node->right_    = parent;
        parent->parent_ = node;

        RBTREE_ASSERT_INVARIANT(parent_parent != nullptr);
        node->parent_ = parent_parent;
        if (parent == root) [[unlikely]] {
            root = node;
        } else if (parent_parent->left_ == parent) {
            parent_parent->left_ = node;
        } else {
            RBTREE_ASSERT_INVARIANT(parent_parent->right_ == parent);
            parent_parent->right_ = node;
        }
    }

    // clang-format off

    RBTREE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_write, 2)
    ATTRIBUTE_ACCESS(read_only, 3)
    constexpr void MakeLeftOnInsertion(NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER node_parent,
                                       NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER node) noexcept {
        RBTREE_ASSERT_INVARIANT(node_parent != nullptr);
        RBTREE_ASSERT_INVARIANT(node_parent != PreRootNil() && node_parent->left_ == nullptr);
        RBTREE_ASSERT_INVARIANT(node != nullptr);
        RBTREE_ASSERT_INVARIANT(node->parent_ == node_parent);
        RBTREE_ASSERT_INVARIANT(node->left_ == nullptr);
        RBTREE_ASSERT_INVARIANT(node->right_ == nullptr);
        if (node_parent == Begin()) {
            Begin() = node;
        }
        node_parent->left_ = node;
        RBTREE_ASSERT_INVARIANT(GetBeginUnchecked() != nullptr);
        RBTREE_ASSERT_INVARIANT(GetLastUnchecked() != nullptr);
    }

    RBTREE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_write, 2)
    ATTRIBUTE_ACCESS(read_only, 3)
    constexpr void MakeRightOnInsertion(NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER node_parent,
                                       NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER node) noexcept {
        RBTREE_ASSERT_INVARIANT(node_parent != nullptr);
        RBTREE_ASSERT_INVARIANT(node_parent == PreRootNil() || node_parent->right_ == nullptr);
        RBTREE_ASSERT_INVARIANT(node != nullptr);
        RBTREE_ASSERT_INVARIANT(node->parent_ == node_parent);
        RBTREE_ASSERT_INVARIANT(node->left_ == nullptr);
        RBTREE_ASSERT_INVARIANT(node->right_ == nullptr);
        if (node_parent == PreRootNil()) [[unlikely]] {
            GetRootUnchecked() = node;
            // GetBeginUnchecked() = node; will be set during `node_parent->right_ = node`
            GetLastUnchecked() = node;
        } else if (node_parent == Last()) {
            Last() = node;
        }
        node_parent->right_ = node;
        RBTREE_ASSERT_INVARIANT(GetBeginUnchecked() != nullptr);
        RBTREE_ASSERT_INVARIANT(GetLastUnchecked() != nullptr);
    }

    // clang-format on

    RBTREE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_write, 2)
    constexpr void RebalanceAfterInsertion(
        NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER new_inserted_node) noexcept {
        RBTREE_ASSERT_INVARIANT(new_inserted_node != nullptr);
        RBTREE_ASSERT_INVARIANT(new_inserted_node->color_ == Color::kRed);
        RBTREE_ASSERT_INVARIANT(Size() >= 1);
        NodeBase *current_node = new_inserted_node;
        NodeBase *current_node_parent;
        NodeBase *&root = GetRootUnchecked();
        while ((current_node_parent = current_node->parent_) != root &&
               current_node_parent->color_ == Color::kRed) {
            RBTREE_ASSERT_INVARIANT(current_node_parent != GetPreRootNilUnchecked());
            RBTREE_ASSERT_INVARIANT(current_node_parent->OtherSibling(current_node) == nullptr ||
                                    current_node_parent->OtherSibling(current_node)->color_ ==
                                        Color::kBlack);
            NodeBase *current_node_parent_parent = current_node_parent->parent_;
            RBTREE_ASSERT_INVARIANT(current_node_parent_parent != nullptr);
            RBTREE_ASSERT_INVARIANT(current_node_parent_parent->color_ == Color::kBlack);

            NodeBase *current_node_parent_sibling =
                current_node_parent_parent->OtherSibling(current_node_parent);
            if (current_node_parent_sibling != nullptr &&
                current_node_parent_sibling->color_ == Color::kRed) {
                // clang-format off
                /**
                 *                     (current_node_parent_parent, B)
                 *                       /                \
                 *  (current_node_parent, R)             (current_node_parent_sibling, R)  [order can be reversed]
                 *      /          \                                     |         \
                 *  (_, B) (current_node, R) [order can be reversed]  (_, B)     (_, B)
                 *
                 *  into:
                 *
                 *                     (current_node_parent_parent, R)
                 *                       /                \
                 *  (current_node_parent, B)             (current_node_parent_sibling, B)  [order can be reversed]
                 *      /          \                                     |         \
                 *  (_, B) (current_node, R) [order can be reversed]  (_, B)     (_, B)
                 */
                // clang-format on
                RBTREE_ASSERT_INVARIANT(current_node_parent_sibling != GetPreRootNilUnchecked());
                RBTREE_ASSERT_INVARIANT(current_node_parent_sibling != root);
                RBTREE_ASSERT_INVARIANT(current_node_parent_sibling->left_ == nullptr ||
                                        current_node_parent_sibling->left_->color_ ==
                                            Color::kBlack);
                RBTREE_ASSERT_INVARIANT(current_node_parent_sibling->left_ !=
                                        GetPreRootNilUnchecked());
                RBTREE_ASSERT_INVARIANT(current_node_parent_sibling->right_ == nullptr ||
                                        current_node_parent_sibling->right_->color_ ==
                                            Color::kBlack);
                RBTREE_ASSERT_INVARIANT(current_node_parent_sibling->right_ !=
                                        GetPreRootNilUnchecked());
                RBTREE_ASSERT_INVARIANT((current_node_parent_sibling->left_ == nullptr &&
                                         current_node_parent_sibling->right_ == nullptr) ^
                                        (current_node_parent_sibling->left_ != nullptr &&
                                         current_node_parent_sibling->right_ != nullptr));
                current_node_parent_parent->color_  = Color::kRed;
                current_node_parent_sibling->color_ = Color::kBlack;
                current_node_parent->color_         = Color::kBlack;
                current_node                        = current_node_parent_parent;
                continue;
            }

            /**
             * if (current_node_parent == current_node_parent_parent->left_) {
             *     if (current_node == current_node_parent->right_) {
             *         current_node->color_               = Color::kBlack;
             *         current_node_parent_parent->color_ = Color::kRed;
             *         LeftRotate(current_node);
             *         RightRotate(current_node);
             *     } else {
             *         current_node_parent->color_        = Color::kBlack;
             *         current_node_parent_parent->color_ = Color::kRed;
             *         RightRotate(current_node_parent)
             *     }
             * } else {
             *     if (current_node == current_node_parent->left_) {
             *         current_node->color_               = Color::kBlack;
             *         current_node_parent_parent->color_ = Color::kRed;
             *         RightRotate(current_node);
             *         LeftRotate(current_node);
             *     } else {
             *         current_node_parent->color_        = Color::kBlack;
             *         current_node_parent_parent->color_ = Color::kRed;
             *         LeftRotate(current_node_parent);
             *     }
             * }
             */
            if (current_node_parent == current_node_parent_parent->left_) {
                if (current_node == current_node_parent->right_) {
                    LeftRotate(current_node, root);
                    current_node_parent = current_node;
                } else {
                    RBTREE_ASSERT_INVARIANT(current_node == current_node_parent->left_);
                }
                RightRotate(current_node_parent, root);
            } else {
                RBTREE_ASSERT_INVARIANT(current_node_parent == current_node_parent_parent->right_);
                if (current_node == current_node_parent->left_) {
                    RightRotate(current_node, root);
                    current_node_parent = current_node;
                } else {
                    RBTREE_ASSERT_INVARIANT(current_node == current_node_parent->right_);
                }
                LeftRotate(current_node_parent, root);
            }
            current_node_parent_parent->color_ = Color::kRed;
            current_node_parent->color_        = Color::kBlack;
            break;
        }

        RBTREE_ASSERT_INVARIANT(root != nullptr);
        root->color_ = Color::kBlack;
        RBTREE_ASSERT_INVARIANT(Root() != nullptr);
    }

    RBTREE_ATTRIBUTE_NONNULL(2)
    ATTRIBUTE_ACCESS(read_only, 2)
    ATTRIBUTE_ACCESS(read_write, 3)
    constexpr void Transplant(NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER which_node,
                              NodeBase *const with_node) noexcept {
        RBTREE_ASSERT_INVARIANT(which_node != nullptr);
        RBTREE_ASSERT_INVARIANT(which_node != PreRootNil());
        RBTREE_ASSERT_INVARIANT(with_node != PreRootNil());
        RBTREE_ASSERT_INVARIANT(with_node != Root());
        NodeBase *const which_node_parent = which_node->Parent();
        RBTREE_ASSERT_INVARIANT(which_node_parent != nullptr);
        RBTREE_ASSERT_INVARIANT(with_node == nullptr || with_node->Parent() != which_node_parent);
        if (which_node_parent == PreRootNil()) [[unlikely]] {
            GetRootUnchecked() = with_node;
        } else if (which_node == which_node_parent->left_) {
            which_node_parent->left_ = with_node;
        } else {
            RBTREE_ASSERT_INVARIANT(which_node == which_node_parent->right_);
            which_node_parent->right_ = with_node;
        }
        if (with_node != nullptr) {
            with_node->parent_ = which_node_parent;
        }
    }

    RBTREE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_write, 2)
    constexpr void RebalanceAfterExtraction(
        NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER node) noexcept {
        RBTREE_ASSERT_INVARIANT(node != nullptr);
        RBTREE_ASSERT_INVARIANT(node != GetPreRootNilUnchecked());
        NodeBase *current_node = node;
        NodeBase *&root        = GetRootUnchecked();
        RBTREE_ASSERT_INVARIANT(root != nullptr);
        RBTREE_ASSERT_INVARIANT(root != GetPreRootNilUnchecked());
        while (current_node != root && current_node->color_ == Color::kBlack) {
            NodeBase *const current_node_parent = current_node->parent_;
            RBTREE_ASSERT_INVARIANT(current_node_parent != nullptr);
            RBTREE_ASSERT_INVARIANT(current_node_parent != PreRootNil());
            if (current_node == current_node_parent->left_) {
                NodeBase *current_node_sibling = current_node_parent->right_;
                RBTREE_ASSERT_INVARIANT(current_node_sibling != nullptr);
                if (current_node_sibling->color_ == Color::kRed) {
                    RBTREE_ASSERT_INVARIANT(current_node_sibling != PreRootNil());
                    RBTREE_ASSERT_INVARIANT(current_node_sibling != root);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->left_ != PreRootNil());
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->left_ != nullptr);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->left_->color_ == Color::kBlack);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->right_ != PreRootNil());
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->right_ != nullptr);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->right_->color_ == Color::kBlack);
                    current_node_sibling->color_ = Color::kBlack;
                    current_node_parent->color_  = Color::kRed;
#ifdef RBTREE_DEBUG
                    const NodeBase *const current_node_sibling_original_left_child =
                        current_node_sibling->left_;
                    RBTREE_ASSERT_INVARIANT(current_node_sibling_original_left_child != nullptr);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling_original_left_child->color_ ==
                                            Color::kBlack);
#endif
                    LeftRotate(current_node_sibling, root);
                    RBTREE_ASSERT_INVARIANT(current_node_parent->right_ != current_node_sibling);
                    RBTREE_ASSERT_INVARIANT(current_node_parent->right_ ==
                                            current_node_sibling_original_left_child);
                    current_node_sibling = current_node_parent->right_;
                    RBTREE_ASSERT_INVARIANT(current_node_sibling != nullptr);
                }
                NodeBase *w = current_node_sibling;
                RBTREE_ASSERT_INVARIANT(w != nullptr);
                NodeBase *const w_left  = w->left_;
                NodeBase *const w_right = w->right_;
                if ((w_left == nullptr || w_left->color_ == Color::kBlack) &&
                    (w_right == nullptr || w_right->color_ == Color::kBlack)) {
                    w->color_ = Color::kRed;
                    RBTREE_ASSERT_INVARIANT(current_node_parent == current_node->parent_);
                    current_node = current_node_parent;
                } else {
                    RBTREE_ASSERT_INVARIANT(w_right != nullptr);
                    RBTREE_ASSERT_INVARIANT(w_right->parent_ == w);
                    RBTREE_ASSERT_INVARIANT(w_left != nullptr);
                    RBTREE_ASSERT_INVARIANT(w_left->parent_ == w);
                    if (w_right->color_ == Color::kBlack) {
                        RBTREE_ASSERT_INVARIANT(w_left->color_ == Color::kRed);
                        w_left->color_ = Color::kBlack;
                        w->color_      = Color::kRed;
                        RightRotate(w_left, root);
                        RBTREE_ASSERT_INVARIANT(current_node_parent ==
                                                current_node->parent_);  // ????
                        w = current_node_parent->right_;
                    } else {
                        RBTREE_ASSERT_INVARIANT(w_right->color_ == Color::kRed);
                    }
                    RBTREE_ASSERT_INVARIANT(current_node_parent == current_node->parent_);  // ????
                    w->color_                   = current_node_parent->color_;
                    current_node_parent->color_ = Color::kBlack;
                    RBTREE_ASSERT_INVARIANT(w_right == w->right_);
                    w_right->color_ = Color::kBlack;
                    RBTREE_ASSERT_INVARIANT(current_node->parent_->right_ == current_node_sibling);
                    LeftRotate(current_node_sibling, root);
                    current_node = root;
                    break;
                }
            } else {
                RBTREE_ASSERT_INVARIANT(current_node == current_node_parent->right_);
                NodeBase *current_node_sibling = current_node_parent->left_;
                RBTREE_ASSERT_INVARIANT(current_node_sibling != nullptr);
                if (current_node_sibling->color_ == Color::kRed) {
                    RBTREE_ASSERT_INVARIANT(current_node_sibling != PreRootNil());
                    RBTREE_ASSERT_INVARIANT(current_node_sibling != root);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->left_ != PreRootNil());
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->left_ != nullptr);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->left_->color_ == Color::kBlack);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->right_ != PreRootNil());
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->right_ != nullptr);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->right_->color_ == Color::kBlack);
                    current_node_sibling->color_ = Color::kBlack;
                    current_node_parent->color_  = Color::kRed;
#ifdef RBTREE_DEBUG
                    const NodeBase *const current_node_sibling_original_right_child =
                        current_node_sibling->right_;
                    RBTREE_ASSERT_INVARIANT(current_node_sibling_original_right_child != nullptr);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling_original_right_child->color_ ==
                                            Color::kBlack);
#endif
                    RightRotate(current_node_sibling, root);
                    RBTREE_ASSERT_INVARIANT(current_node_parent->left_ != current_node_sibling);
                    RBTREE_ASSERT_INVARIANT(current_node_parent->left_ ==
                                            current_node_sibling_original_right_child);
                    current_node_sibling = current_node_parent->left_;
                    RBTREE_ASSERT_INVARIANT(current_node_sibling != nullptr);
                }
                NodeBase *w = current_node_sibling;
                RBTREE_ASSERT_INVARIANT(w != nullptr);
                NodeBase *const w_left  = w->left_;
                NodeBase *const w_right = w->right_;
                if ((w_left == nullptr || w_left->color_ == Color::kBlack) &&
                    (w_right == nullptr || w_right->color_ == Color::kBlack)) {
                    w->color_ = Color::kRed;
                    RBTREE_ASSERT_INVARIANT(current_node_parent == current_node->parent_);
                    current_node = current_node_parent;
                } else {
                    RBTREE_ASSERT_INVARIANT(w_right != nullptr);
                    RBTREE_ASSERT_INVARIANT(w_right->parent_ == w);
                    RBTREE_ASSERT_INVARIANT(w_left != nullptr);
                    RBTREE_ASSERT_INVARIANT(w_left->parent_ == w);
                    if (w_left->color_ == Color::kBlack) {
                        RBTREE_ASSERT_INVARIANT(w_right->color_ == Color::kRed);
                        w_right->color_ = Color::kBlack;
                        w->color_       = Color::kRed;
                        LeftRotate(w_right, root);
                        RBTREE_ASSERT_INVARIANT(current_node_parent ==
                                                current_node->parent_);  // ????
                        w = current_node_parent->left_;
                    } else {
                        RBTREE_ASSERT_INVARIANT(w_left->color_ == Color::kRed);
                    }
                    RBTREE_ASSERT_INVARIANT(current_node_parent == current_node->parent_);  // ????
                    w->color_                   = current_node_parent->color_;
                    current_node_parent->color_ = Color::kBlack;
                    RBTREE_ASSERT_INVARIANT(w_left == w->left_);
                    w_left->color_ = Color::kBlack;
                    RBTREE_ASSERT_INVARIANT(current_node->parent_->left_ == current_node_sibling);
                    RightRotate(current_node_sibling, root);
                    current_node = root;
                    break;
                }
            }
        }

        RBTREE_ASSERT_INVARIANT(current_node != nullptr);
        current_node->color_ = Color::kBlack;
        RBTREE_ASSERT_INVARIANT(Size() == 0 || Root() != nullptr);
    }

    NodeBase fake_root_;
};

[[nodiscard]] ATTRIBUTE_ACCESS(read_only, 1) ATTRIBUTE_PURE
    constexpr bool IsFakeNodeOrRoot(const NodeBase *node) noexcept {
    if (node == nullptr) {
        return false;
    }

    const NodeBase *parent = node->parent_;
    return parent != nullptr && parent->parent_ == node;
}

template <class NodeType>
[[nodiscard]] RBTREE_NONNULL_ALL_ARGS ATTRIBUTE_ACCESS(read_only, 1) ATTRIBUTE_PURE
    constexpr NodeType *Increment(NodeType *CONFIG_CLANG_NONNULL_QUALIFIER node) noexcept {
    if (auto *right_child = node->right_; right_child != nullptr) {
        return right_child->LeftMostNode();
    }

    auto *parent = node->parent_;
    RBTREE_ASSERT_INVARIANT(parent != nullptr);
    RBTREE_ASSERT_INVARIANT((parent->color_ == Color::kRed || parent->color_ == Color::kBlack) ||
                            IsFakeNodeOrRoot(parent));
    while (node == parent->right_) {
        node   = parent;
        parent = node->parent_;
        RBTREE_ASSERT_INVARIANT(parent != nullptr);
        RBTREE_ASSERT_INVARIANT(
            (parent->color_ == Color::kRed || parent->color_ == Color::kBlack) ||
            IsFakeNodeOrRoot(parent));
    }

    RBTREE_ASSERT_INVARIANT(node == parent->left_ ||
                            (IsFakeNodeOrRoot(parent) && parent->parent_ == node) ||
                            (IsFakeNodeOrRoot(node) && node->right_ == parent));
    return node->right_ != parent ? parent : node;
}

template <class NodeType>
[[nodiscard]] RBTREE_NONNULL_ALL_ARGS ATTRIBUTE_ACCESS(read_only, 1) ATTRIBUTE_PURE
    constexpr NodeType *Decrement(NodeType *CONFIG_CLANG_NONNULL_QUALIFIER node) noexcept {
    RBTREE_ASSERT_INVARIANT(node != nullptr);
    if (auto *left_child = node->left_; left_child != nullptr) {
        return left_child->RightMostNode();
    }

    auto *parent = node->parent_;
    RBTREE_ASSERT_INVARIANT(parent != nullptr);
    RBTREE_ASSERT_INVARIANT((parent->color_ == Color::kRed || parent->color_ == Color::kBlack) ||
                            IsFakeNodeOrRoot(parent));
    while (node == parent->left_) {
        node   = parent;
        parent = node->parent_;
        RBTREE_ASSERT_INVARIANT(parent != nullptr);
        RBTREE_ASSERT_INVARIANT(parent->color_ == Color::kRed || parent->color_ == Color::kBlack);
    }

    RBTREE_ASSERT_INVARIANT(node == parent->right_);
    return parent;
}

template <class KeyType>
ATTRIBUTE_ACCESS(read_write, 1)
constexpr void DeleteTree(Node<KeyType> *const root) noexcept(
    std::is_nothrow_destructible_v<KeyType>) {
    if (root == nullptr) {
        return;
    }
    root->parent_             = nullptr;
    Node<KeyType> *local_root = root;
    do {
        Node<KeyType> *const left  = local_root->Left();
        Node<KeyType> *const right = local_root->Right();
        Node<KeyType> *next_node{};
        if (left != nullptr && right != nullptr) {
            RBTREE_ASSERT_INVARIANT(left->Parent() == local_root);
            RBTREE_ASSERT_INVARIANT(right->Parent() == local_root);
            right->parent_ = local_root->Parent();
            left->parent_  = right;
            next_node      = left;
        } else if (left != nullptr) {
            RBTREE_ASSERT_INVARIANT(left->parent_ == local_root);
            left->parent_ = local_root->Parent();
            next_node     = left;
        } else if (right != nullptr) {
            RBTREE_ASSERT_INVARIANT(right->parent_ == local_root);
            right->parent_ = local_root->Parent();
            next_node      = right;
        } else {
            next_node = local_root->Parent();
        }

        delete local_root;
        local_root = next_node;
    } while (local_root != nullptr);
}

[[noreturn]] RBTREE_NONNULL_ALL_ARGS ATTRIBUTE_ACCESS(read_only, 1) ATTRIBUTE_COLD
    void ThrowLengthError(const char *CONFIG_CLANG_NONNULL_QUALIFIER function_name) {
    char buffer[1024]{};
    [[maybe_unused]] const int ret = std::snprintf(
        buffer, std::size(buffer), "Could not create node: size error at %s", function_name);
    RBTREE_ASSERT_INVARIANT(ret > 0);
    throw std::length_error(buffer);
}

template <class KeyType, class DerivedRBTree>
class RBTreeContainer;

template <class KeyType, bool IsConstIterator, class RBTreeFriend>
class Iterator final {
    using IterNodeBaseType = std::conditional_t<IsConstIterator, const NodeBase, NodeBase>;
    using IterNodeType  = std::conditional_t<IsConstIterator, const Node<KeyType>, Node<KeyType> >;
    using IterKeyType   = std::conditional_t<IsConstIterator, const KeyType, KeyType>;
    using OtherIterator = Iterator<KeyType, !IsConstIterator, RBTreeFriend>;

    friend class RBTreeContainer<KeyType, RBTreeFriend>;
    friend OtherIterator;
    friend RBTreeFriend;

    RBTREE_NONNULL_ALL_ARGS
    explicit constexpr Iterator(
        IterNodeType *CONFIG_CLANG_NONNULL_QUALIFIER node_ptr ATTRIBUTE_LIFETIME_BOUND) noexcept
        : node_(node_ptr) {
        RBTREE_ASSERT_INVARIANT(node_ != nullptr);
    }

public:
    using value_type        = KeyType;
    using reference         = IterKeyType &;
    using pointer           = IterKeyType *;
    using difference_type   = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    constexpr Iterator() noexcept = default;

    constexpr Iterator(const Iterator &) noexcept = default;

    constexpr Iterator(Iterator &&) noexcept = default;

    constexpr Iterator &operator=(const Iterator &) noexcept = default;

    constexpr Iterator &operator=(Iterator &&) noexcept = default;

    /* implicit */
    constexpr Iterator(OtherIterator other) noexcept
        requires(IsConstIterator)
        : Iterator(other.node_) {}

    explicit Iterator(std::nullptr_t) = delete;

    [[nodiscard]] constexpr reference operator*() const noexcept {
        return node_->key_;
    }

    [[nodiscard]] ATTRIBUTE_RETURNS_NONNULL constexpr pointer operator->() const noexcept {
        return std::addressof(node_->key_);
    }

    [[nodiscard]] constexpr bool operator==(const Iterator &) const noexcept = default;

    constexpr Iterator &operator++() noexcept ATTRIBUTE_LIFETIME_BOUND {
        node_ = static_cast<IterNodeType *>(Increment<IterNodeBaseType>(node_));
        RBTREE_ASSERT_INVARIANT(node_ != nullptr);
        return *this;
    }

    [[nodiscard]] constexpr Iterator operator++(int) & noexcept {
        Iterator copy(*this);
        ++*this;
        return copy;
    }

    constexpr Iterator &operator--() noexcept ATTRIBUTE_LIFETIME_BOUND {
        node_ = static_cast<IterNodeType *>(Decrement<IterNodeBaseType>(node_));
        RBTREE_ASSERT_INVARIANT(node_ != nullptr);
        return *this;
    }

    [[nodiscard]] constexpr Iterator operator--(int) & noexcept {
        Iterator copy(*this);
        --*this;
        return copy;
    }

private:
    IterNodeType *node_{};
};

template <class KeyType, class DerivedRBTree>
class RBTreeContainer : private RBTreeContainerImpl {
    static_assert(!std::is_reference_v<KeyType>, "Type can't be reference");
    static_assert(!std::is_array_v<KeyType>, "Type can't be array");
    static_assert(sizeof(KeyType) >= 1, "Type can't be incomplete type");

    using Base = RBTreeContainerImpl;

protected:
    using Base::Base;
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;
    using const_iterator         = Iterator<KeyType, /*IsConstIterator = */ true, DerivedRBTree>;
    using iterator               = const_iterator;
    using value_type             = KeyType;
    using key_type               = KeyType;
    using reference              = value_type &;
    using const_reference        = const value_type &;
    using NodeType               = Node<KeyType>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using node_type              = NodeType;

    static constexpr bool kIsNodeNoexceptDestructible = std::is_nothrow_destructible_v<NodeType>;

    constexpr RBTreeContainer() = default;

    //    constexpr RBTreeContainer(const RBTreeContainer &) noexcept(
    //        std::is_nothrow_copy_constructible_v<Base>) = default;
    //    constexpr RBTreeContainer &operator=(const RBTreeContainer &) noexcept(
    //        std::is_nothrow_copy_assignabl_v<Base>) = default;
    constexpr RBTreeContainer(RBTreeContainer &&) = default;

    constexpr RBTreeContainer &operator=(RBTreeContainer &&) = default;

    constexpr ~RBTreeContainer() {
        DeleteTree(static_cast<NodeType *>(Base::Root()));
    }

    constexpr void swap(RBTreeContainer &other) noexcept {
        Base::swap(other);
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr size_type size() const noexcept {
        const size_type value = Base::Size();
        RBTREE_ASSERT_INVARIANT(value <= max_size());
        CONFIG_ASSUME_STATEMENT(value <= max_size());
        return value;
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr bool empty() const noexcept {
        return size() == 0;
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr const_iterator begin() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return const_iterator{static_cast<const NodeType *>(Base::Begin())};
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr const_iterator end() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return iterator{static_cast<const NodeType *>(Base::End())};
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr iterator begin() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return iterator(static_cast<NodeType *>(Base::Begin()));
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr iterator end() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return iterator{static_cast<NodeType *>(Base::End())};
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr const_iterator cbegin() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return begin();
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr const_iterator cend() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return end();
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr const_reverse_iterator rbegin() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return const_reverse_iterator{end()};
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr const_reverse_iterator rend() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return const_reverse_iterator{begin()};
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr reverse_iterator rbegin() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return reverse_iterator{end()};
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr reverse_iterator rend() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return reverse_iterator{begin()};
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr const_reverse_iterator crbegin() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return rbegin();
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr const_reverse_iterator crend() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return rend();
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr const_reference front() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return *begin();
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr reference front() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return *begin();
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr const_reference back() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return static_cast<const NodeType *>(Base::Last())->key_;
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr reference back() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return static_cast<NodeType *>(Base::Last())->key_;
    }

    [[nodiscard]] ATTRIBUTE_PURE static consteval size_type max_size() noexcept {
        return std::min(static_cast<size_type>(std::numeric_limits<difference_type>::max()) /
                            sizeof(NodeType),
                        std::numeric_limits<size_type>::max()) /
               sizeof(NodeType);
    }

    [[nodiscard]] constexpr NodeType *Root() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return static_cast<NodeType *>(Base::Root());
    }

    [[nodiscard]] constexpr const NodeType *Root() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return static_cast<const NodeType *>(Base::Root());
    }

    [[nodiscard]] constexpr const NodeType *PreRootNodePtr() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return static_cast<const NodeType *>(Base::PreRootNil());
    }

    [[nodiscard]] constexpr NodeType *PreRootNodePtr() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return static_cast<NodeType *>(Base::PreRootNil());
    }

    // clang-format off

    template <class... Args>
    [[nodiscard]]
    RBTREE_ATTRIBUTE_NONNULL(2)
    ATTRIBUTE_ACCESS(read_write, 2)
    constexpr NodeType *InsertNode(NodeType *const CONFIG_CLANG_NONNULL_QUALIFIER node_parent,
                                   const bool insert_left,
                                   Args &&...args) ATTRIBUTE_LIFETIME_BOUND {
        // clang-format on
        auto *const new_node = CreateNode(std::forward<Args>(args)...);
        Base::InsertNode(node_parent, insert_left, new_node);
        return new_node;
    }

    constexpr void ExtractAndDestroyNode(const_iterator iter) noexcept(
        kIsNodeNoexceptDestructible) {
        auto *const node = const_cast<NodeType *>(iter.node_);
        RBTREE_ASSERT_INVARIANT(node != nullptr);
        Base::ExtractNode(node);
        DestroyNode(node);
    }

private:
    template <class... Args>
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE constexpr NodeType *CreateNode(Args &&...args) {
        return CheckSizeAndCreateNode(Size(), std::forward<Args>(args)...);
    }

    template <class... Args>
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE static constexpr NodeType *CheckSizeAndCreateNode(
        const size_type tree_size, Args &&...args) {
        if (tree_size >= max_size()) [[unlikely]] {
            RBTREE_ASSERT_INVARIANT(tree_size == max_size());
            ThrowLengthError(std::source_location::current().function_name());
        }

        return new NodeType(std::forward<Args>(args)...);
    }

    static constexpr void DestroyNode(NodeType *const node) noexcept(kIsNodeNoexceptDestructible) {
        delete node;
    }
};

#undef RBTREE_ATTRIBUTE_NONNULL
#undef RBTREE_NONNULL_ALL_ARGS
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

template <class T>
inline constexpr bool kUseByValue =
    std::is_trivial_v<T> ||
    (std::is_standard_layout_v<T> && !std::is_abstract_v<T> && std::is_default_constructible_v<T> &&
     std::is_trivially_copy_constructible_v<T> && std::is_trivially_move_constructible_v<T> &&
     std::is_trivially_copy_assignable_v<T> && std::is_trivially_move_assignable_v<T> &&
     std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_move_constructible_v<T> &&
     std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_move_assignable_v<T> &&
     std::is_trivially_destructible_v<T> && std::is_nothrow_destructible_v<T> && sizeof(T) <= 32);

template <class T>
struct IsSpecializationOfBasicString {
    static constexpr bool value = false;
};

template <class CharType>
struct IsSpecializationOfBasicString<std::basic_string<CharType> > {
    static constexpr bool value = true;
};

template <class T>
struct IsSpecializationOfBasicStringView {
    static constexpr bool value = false;
};

template <class CharType>
struct IsSpecializationOfBasicStringView<std::basic_string_view<CharType> > {
    static constexpr bool value = true;
};

template <class KeyType, class ComparatorType>
class ComparatorHelper {
private:
    static constexpr bool kUseStdThreeWayComparison =
        std::is_same_v<ComparatorType, std::less<void> > ||
        std::is_same_v<ComparatorType, std::greater<void> > ||
        std::is_same_v<ComparatorType, std::less_equal<void> > ||
        std::is_same_v<ComparatorType, std::greater_equal<void> > ||
        std::is_same_v<ComparatorType, std::less<KeyType> > ||
        std::is_same_v<ComparatorType, std::greater<KeyType> > ||
        std::is_same_v<ComparatorType, std::less_equal<KeyType> > ||
        std::is_same_v<ComparatorType, std::greater_equal<KeyType> >;

    static constexpr bool IsNoexceptThreeWayComparable() noexcept {
        if constexpr (kUseStdThreeWayComparison) {
            return noexcept(std::compare_weak_order_fallback(std::declval<const KeyType &>(),
                                                             std::declval<const KeyType &>()));
        } else {
            return noexcept(
                ComparatorType{}(std::declval<const KeyType &>(), std::declval<const KeyType &>()));
        }
    }

protected:
    static constexpr bool kIsNoexceptThreeWayComparable = IsNoexceptThreeWayComparable();

    [[nodiscard]] static constexpr std::weak_ordering CompareThreeWay(
        const KeyType &lhs, const KeyType &rhs) noexcept(kIsNoexceptThreeWayComparable) {
        if constexpr (kUseStdThreeWayComparison) {
            return std::compare_weak_order_fallback(lhs, rhs);
        } else {
            if (ComparatorType{}(lhs, rhs)) {
                return std::weak_ordering::less;
            } else if (ComparatorType{}(rhs, lhs)) {
                return std::weak_ordering::greater;
            } else {
                return std::weak_ordering::equivalent;
            }
        }
    }
};

template <class Comparator>
concept StatelessComparator =
    std::is_trivially_default_constructible_v<Comparator> &&
    std::is_nothrow_default_constructible_v<Comparator> && sizeof(Comparator) <= sizeof(char);

static_assert(StatelessComparator<std::less<std::string> >);
static_assert(StatelessComparator<std::greater<int> >);

export template <class KeyType, StatelessComparator ComparatorType = std::less<>,
                 bool UseByValueWherePossible = kUseByValue<KeyType> >
class RBTree
    : private RBTreeContainer<KeyType, RBTree<KeyType, ComparatorType, UseByValueWherePossible> >,
      private ComparatorHelper<KeyType, ComparatorType> {
    using Base     = RBTreeContainer<KeyType, RBTree>;
    using NodeType = typename Base::NodeType;
    using Base::Root;
    using ReadOnlyKeyType = std::conditional_t<UseByValueWherePossible, KeyType, const KeyType &>;
    using ComparatorHelper<KeyType, ComparatorType>::kIsNoexceptThreeWayComparable;
    using ComparatorHelper<KeyType, ComparatorType>::CompareThreeWay;

public:
    using size_type              = typename Base::size_type;
    using iterator               = typename Base::iterator;
    using const_iterator         = typename Base::const_iterator;
    using difference_type        = typename Base::difference_type;
    using value_type             = typename Base::value_type;
    using reference              = typename Base::reference;
    using const_reference        = typename Base::const_reference;
    using key_type               = typename Base::key_type;
    using reverse_iterator       = typename Base::reverse_iterator;
    using const_reverse_iterator = typename Base::const_reverse_iterator;
    using node_type              = typename Base::node_type;
    using Base::back;
    using Base::Base;
    using Base::begin;
    using Base::cbegin;
    using Base::cend;
    using Base::crbegin;
    using Base::crend;
    using Base::empty;
    using Base::end;
    using Base::front;
    using Base::max_size;
    using Base::rbegin;
    using Base::rend;
    using Base::size;

    RBTree(std::initializer_list<KeyType> list) {
        for (const auto &elem : list) {
            insert(elem);  // TODO: insert_hint
        }
    }

    constexpr void swap(RBTree &other) noexcept {
        Base::swap(other);
    }

    friend constexpr void swap(RBTree &lhs, RBTree &rhs) noexcept {
        lhs.swap(rhs);
    }

    struct InsertResult {
        iterator iter;
        bool inserted;
    };

    template <class U>
    constexpr InsertResult insert(U &&key) ATTRIBUTE_LIFETIME_BOUND {
        return insert_impl</*OverwriteIfExists = */ false>(std::forward<U>(key));
    }

    template <class U>
    constexpr iterator insert_or_overwrite(U &&key) ATTRIBUTE_LIFETIME_BOUND {
        return insert_impl</*OverwriteIfExists = */ true>(std::forward<U>(key));
    }

    constexpr size_type erase(ReadOnlyKeyType key) noexcept(kIsNoexceptThreeWayComparable &&
                                                            Base::kIsNodeNoexceptDestructible) {
        const_iterator iter = find(key);
        if (iter == end()) [[unlikely]] {
            return 0;
        }
        erase(iter);
        return 1;
    }

    constexpr void erase(const_iterator iter) noexcept(Base::kIsNodeNoexceptDestructible) {
        RBTREE_ASSERT_INVARIANT(iter != end());
        erase_impl(iter);
    }

    [[nodiscard]] constexpr const_iterator lower_bound(ReadOnlyKeyType key) const
        noexcept(kIsNoexceptThreeWayComparable) ATTRIBUTE_LIFETIME_BOUND {
        const NodeType *last_left_turn = nullptr;
        for (auto *current_node = Root(); current_node != nullptr;) {
            const auto compare_result = CompareThreeWay(key, current_node->key_);
            if (compare_result < 0) {
                last_left_turn = current_node;
                current_node   = current_node->Left();
            } else if (compare_result > 0) {
                current_node = current_node->Right();
            } else {
                return const_iterator(current_node);
            }
        }

        return last_left_turn != nullptr ? const_iterator(last_left_turn) : end();
    }

    [[nodiscard]] constexpr const_iterator find(ReadOnlyKeyType key) const
        noexcept(kIsNoexceptThreeWayComparable) ATTRIBUTE_LIFETIME_BOUND {
        for (auto *current_node = Root(); current_node != nullptr;) {
            const auto compare_result = CompareThreeWay(key, current_node->key_);
            if (compare_result < 0) {
                current_node = current_node->Left();
            } else if (compare_result > 0) {
                current_node = current_node->Right();
            } else {
                return const_iterator(current_node);
            }
        }

        return end();
    }

    // DELETEME:
    static constexpr int32_t kInvalidBHeight = -1;

    [[nodiscard]] bool is_rbtree() const {
        if (Root() == nullptr) {
            return true;
        }
        if (Root()->color_ != Color::kBlack) {
            fprintf(stderr, "Root is not black\n");
            fflush(nullptr);
            return false;
        }

        auto [h, min, max] = impl(Root());
        if (h == kInvalidBHeight) {
            RBTREE_ASSERT_INVARIANT(min == max && max == KeyType{});
            return false;
        }
        RBTREE_ASSERT_INVARIANT(h > 0);

        if (Root()->left_) {
            RBTREE_ASSERT_INVARIANT(min < Root()->key_);
        } else {
            RBTREE_ASSERT_INVARIANT(min == Root()->key_);
        }
        if (Root()->right_) {
            RBTREE_ASSERT_INVARIANT(Root()->key_ < max);
        } else {
            RBTREE_ASSERT_INVARIANT(Root()->key_ == max);
        }
        return true;
    }

private:
    [[nodiscard]] std::tuple<std::int32_t, KeyType, KeyType> impl(const NodeType *node) const {
        RBTREE_ASSERT_INVARIANT(node != nullptr);
        std::int32_t height_l = 1;
        std::int32_t height_r = 1;
        KeyType min_l         = node->key_;
        KeyType max_r         = node->key_;
        auto *left_child      = node->Left();
        auto *right_child     = node->Right();

        switch (node->color_) {
            case Color::kRed: {
                if (auto *parent = node->parent_;
                    parent == nullptr || parent->color_ != Color::kBlack) {
                    fprintf(stderr, "Red node doesn't have valid black parent\n");
                    fflush(nullptr);
                    return {kInvalidBHeight, {}, {}};
                }
                const bool only_one_nil = (left_child == nullptr && right_child != nullptr) ||
                                          (left_child != nullptr && right_child == nullptr);
                if (only_one_nil) {
                    fprintf(stderr, "Red node has exactly one nil child\n");
                    fflush(nullptr);
                    return {kInvalidBHeight, {}, {}};
                }
            } break;
            case Color::kBlack:
                break;
            default:
                fprintf(stderr, "Node has invalid color\n");
                fflush(nullptr);
                return {kInvalidBHeight, {}, {}};
        }

        if (left_child != nullptr) {
            if (left_child->key_ >= node->key_) {
                fprintf(stderr, "Left key >= node key\n");
                fflush(nullptr);
                return {kInvalidBHeight, {}, {}};
            }
            if (node->color_ == Color::kRed && left_child->color_ != Color::kBlack) {
                fprintf(stderr, "Red node doesn't have black left child\n");
                fflush(nullptr);
                return {kInvalidBHeight, {}, {}};
            }

            const auto [h, left_min, left_max] = impl(left_child);
            RBTREE_ASSERT_INVARIANT(left_min <= left_max);
            if (h == kInvalidBHeight || left_max >= node->key_) {
                fprintf(stderr, "Max key in left subtree >= node key\n");
                fflush(nullptr);
                return {kInvalidBHeight, {}, {}};
            }

            min_l    = std::min(min_l, left_min);
            height_l = h;
        }

        if (right_child != nullptr) {
            if (node->key_ >= right_child->key_) {
                fprintf(stderr, "Node key >= right key\n");
                fflush(nullptr);
                return {kInvalidBHeight, {}, {}};
            }
            if (node->color_ == Color::kRed && right_child->color_ != Color::kBlack) {
                fprintf(stderr, "Red node doesn't have black right child\n");
                fflush(nullptr);
                return {kInvalidBHeight, {}, {}};
            }

            const auto [h, right_min, right_max] = impl(right_child);
            RBTREE_ASSERT_INVARIANT(right_min <= right_max);
            if (h == kInvalidBHeight || node->key_ >= right_min) {
                fprintf(stderr, "Node key >= max key in right subtree\n");
                fflush(nullptr);
                return {kInvalidBHeight, {}, {}};
            }

            max_r    = std::max(max_r, right_max);
            height_r = h;
        }

        RBTREE_ASSERT_INVARIANT(min_l <= max_r);
        if (height_l != height_r) {
            fprintf(stderr, "Left bh != right bh\n");
            fflush(nullptr);
            return {kInvalidBHeight, {}, {}};
        }

        return {height_l + (node->color_ == Color::kBlack), min_l, max_r};
    }

    // clang-format off
    template <bool OverwriteIfExists, class U>
    [[nodiscard]]
    constexpr std::conditional_t<OverwriteIfExists, iterator, InsertResult> insert_impl(U &&key) {
        // clang-format on
        auto *previous_node    = Base::PreRootNodePtr();
        NodeType *current_node = Root();

        bool last_cmp_res_was_left = false;
        while (current_node != nullptr) {
            const auto compare_result = CompareThreeWay(key, current_node->key_);
            previous_node             = current_node;
            last_cmp_res_was_left     = compare_result < 0;
            if (compare_result < 0) {
                current_node = current_node->Left();
            } else if (compare_result > 0) {
                current_node = current_node->Right();
            } else {
                if constexpr (OverwriteIfExists) {
                    current_node->key_ = std::forward<U>(key);
                    return iterator{current_node};
                } else {
                    return InsertResult{.iter = iterator(current_node), .inserted = false};
                }
            }
        }

        const bool insert_left = last_cmp_res_was_left;
        auto *const new_node   = Base::InsertNode(previous_node, insert_left, std::forward<U>(key));
        if constexpr (OverwriteIfExists) {
            return iterator{new_node};
        } else {
            return InsertResult{.iter = iterator(new_node), .inserted = true};
        }
    }

    constexpr void erase_impl(const_iterator iter) noexcept(Base::kIsNodeNoexceptDestructible) {
        Base::ExtractAndDestroyNode(iter);
    }
};
