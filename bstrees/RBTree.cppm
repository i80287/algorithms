module;

#include <algorithm>
#include <array>
#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <source_location>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "../misc/config_macros.hpp"

export module rbtree;

#ifdef RBTREE_DEBUG
#ifdef NDEBUG
#error("Can't assert rbtree invariants in release mode")
#endif
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define RBTREE_ASSERT_INVARIANT(expr) assert(expr)
#else
#define RBTREE_ASSERT_INVARIANT(expr)
#endif

enum class NodeColor : std::size_t { kRed = 0, kBlack };

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-extension"
#define RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
#define RBTREE_ATTRIBUTE_NONNULL(...)
#else
#define RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS ATTRIBUTE_NONNULL_ALL_ARGS
#define RBTREE_ATTRIBUTE_NONNULL(...)     ATTRIBUTE_NONNULL(__VA_ARGS__)

#endif

class NodeBase {
    friend class RBTreeContainerImpl;

public:
    NodeBase(const NodeBase &)            = delete;
    NodeBase &operator=(const NodeBase &) = delete;

protected:
    constexpr NodeBase() noexcept = default;
    constexpr ~NodeBase()         = default;
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
    constexpr void Swap(NodeBase &other) noexcept {
        std::swap(left_, other.left_);
        std::swap(right_, other.right_);
        std::swap(parent_, other.parent_);
        std::swap(color_, other.color_);
    }

public:
    [[nodiscard]] ATTRIBUTE_RETURNS_NONNULL constexpr const NodeBase *LeftMostNode() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return LeftMostNodeOf(this);
    }
    [[nodiscard]] ATTRIBUTE_RETURNS_NONNULL constexpr NodeBase *LeftMostNode() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return LeftMostNodeOf(this);
    }
    [[nodiscard]] ATTRIBUTE_RETURNS_NONNULL constexpr const NodeBase *RightMostNode() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return RightMostNodeOf(this);
    }
    [[nodiscard]] ATTRIBUTE_RETURNS_NONNULL constexpr NodeBase *RightMostNode() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return RightMostNodeOf(this);
    }
    [[nodiscard]] ATTRIBUTE_RETURNS_NONNULL constexpr const NodeBase *FarthestParent()
        const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return FarthestParentNodeOf(this);
    }
    [[nodiscard]] ATTRIBUTE_RETURNS_NONNULL constexpr NodeBase *FarthestParent() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return FarthestParentNodeOf(this);
    }
    [[nodiscard]] constexpr const NodeBase *OtherSibling(const NodeBase *child) const noexcept {
        RBTREE_ASSERT_INVARIANT(IsChildNode(child));
        return (left_ == child) ? right_ : left_;
    }
    [[nodiscard]] constexpr NodeBase *OtherSibling(const NodeBase *child) noexcept {
        RBTREE_ASSERT_INVARIANT(IsChildNode(child));
        return (left_ == child) ? right_ : left_;
    }
    [[nodiscard]] constexpr const NodeBase *Parent() const noexcept {
        return parent_;
    }
    [[nodiscard]] constexpr NodeBase *Parent() noexcept {
        return parent_;
    }
    [[nodiscard]] constexpr NodeBase *&ParentMutableReference() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return parent_;
    }
    constexpr void SetParent(NodeBase *new_parent) noexcept {
        parent_ = new_parent;
    }
    [[nodiscard]] constexpr const NodeBase *Left() const noexcept {
        return left_;
    }
    [[nodiscard]] constexpr NodeBase *Left() noexcept {
        return left_;
    }
    constexpr void SetLeft(NodeBase *new_left) noexcept {
        left_ = new_left;
    }
    [[nodiscard]] constexpr const NodeBase *Right() const noexcept {
        return right_;
    }
    [[nodiscard]] constexpr NodeBase *Right() noexcept {
        return right_;
    }
    constexpr void SetRight(NodeBase *new_right) noexcept {
        right_ = new_right;
    }
    constexpr void SetColor(const NodeColor new_color) noexcept {
        color_ = new_color;
    }
    [[nodiscard]] constexpr NodeColor Color() const noexcept {
        return color_;
    }

private:
    // clang-format off
        template <class NodeType>
        [[nodiscard]]
        RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
        ATTRIBUTE_RETURNS_NONNULL
        ATTRIBUTE_PURE
        ATTRIBUTE_ACCESS(read_only, 1)
        ATTRIBUTE_ALWAYS_INLINE
        static constexpr NodeType *LeftMostNodeOf(NodeType * CONFIG_CLANG_NONNULL_QUALIFIER node ATTRIBUTE_LIFETIME_BOUND) noexcept {
        // clang-format on
        while (node->Left() != nullptr) {
            node = node->Left();
        }
        return node;
    }

    // clang-format off
    template <class NodeType>
    [[nodiscard]]
    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_RETURNS_NONNULL
    ATTRIBUTE_PURE
    ATTRIBUTE_ACCESS(read_only, 1)
    ATTRIBUTE_ALWAYS_INLINE
    static constexpr NodeType *RightMostNodeOf(NodeType * CONFIG_CLANG_NONNULL_QUALIFIER node ATTRIBUTE_LIFETIME_BOUND) noexcept {
        // clang-format on
        while (node->Right() != nullptr) {
            node = node->Right();
        }
        return node;
    }

    // clang-format off
    template <class NodeType>
    [[nodiscard]]
    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_RETURNS_NONNULL
    ATTRIBUTE_PURE
    ATTRIBUTE_ACCESS(read_only, 1)
    ATTRIBUTE_ALWAYS_INLINE
    static constexpr NodeType *FarthestParentNodeOf(NodeType * CONFIG_CLANG_NONNULL_QUALIFIER node ATTRIBUTE_LIFETIME_BOUND) noexcept {
        // clang-format on
        while (node->Parent() != nullptr) {
            node = node->Parent();
        }
        return node;
    }

    [[nodiscard]] constexpr bool IsChildNode(const NodeBase *node) const noexcept {
        return (node == left_) ^ (node == right_);
    }

    NodeBase *left_   = nullptr;
    NodeBase *right_  = nullptr;
    NodeBase *parent_ = nullptr;
    NodeColor color_  = NodeColor::kRed;
};

template <typename T>
class Node final : public NodeBase {
    using Base    = NodeBase;
    using KeyType = T;

public:
    template <std::constructible_from<KeyType> U>
    explicit constexpr Node(U &&key) noexcept(std::is_nothrow_constructible_v<KeyType, U &&>)
        : key_(std::forward<U>(key)) {}

    [[nodiscard]] constexpr const Node *Left() const noexcept {
        return static_cast<const Node *>(Base::Left());
    }
    [[nodiscard]] constexpr Node *Left() noexcept {
        return static_cast<Node *>(Base::Left());
    }
    [[nodiscard]] constexpr const Node *Right() const noexcept {
        return static_cast<const Node *>(Base::Right());
    }
    [[nodiscard]] constexpr Node *Right() noexcept {
        return static_cast<Node *>(Base::Right());
    }
    [[nodiscard]] constexpr const Node *Parent() const noexcept {
        return static_cast<const Node *>(Base::Parent());
    }
    [[nodiscard]] constexpr Node *Parent() noexcept {
        return static_cast<Node *>(Base::Parent());
    }
    template <std::constructible_from<KeyType> U>
    constexpr void SetKey(U &&key) noexcept(std::is_nothrow_assignable_v<KeyType, U &&>) {
        key_ = std::forward<U>(key);
    }
    [[nodiscard]] constexpr const KeyType &Key() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return key_;
    }
    [[nodiscard]] constexpr KeyType &Key() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return key_;
    }
    constexpr void Swap(Node &other) noexcept(std::is_nothrow_swappable_v<KeyType>)
        requires(std::is_swappable_v<KeyType>)
    {
        std::swap(key_, other.key_);
        Base::Swap(other);
    }

private:
    KeyType key_{};
};

class RBTreeContainerImpl {
protected:
    constexpr RBTreeContainerImpl() noexcept : fake_root_{} {
        SetBeginUnchecked(GetEndUnchecked());
        AssertTreeInvariants();
    }

public:
    RBTreeContainerImpl(const RBTreeContainerImpl &other)            = delete;
    RBTreeContainerImpl &operator=(const RBTreeContainerImpl &other) = delete;

protected:
    constexpr RBTreeContainerImpl(RBTreeContainerImpl &&other) noexcept
        : fake_root_(std::move(other.fake_root_)) {
        SynchronizeInvariantsOnMove();
    }
    RBTreeContainerImpl &operator=(RBTreeContainerImpl &&other) noexcept ATTRIBUTE_LIFETIME_BOUND {
        this->Swap(other);
        AssertTreeInvariants();
        return *this;
    }
    constexpr void Swap(RBTreeContainerImpl &other) noexcept {
        fake_root_.Swap(other.fake_root_);
        SynchronizeInvariantsOnMove();
        other.SynchronizeInvariantsOnMove();
    }
    struct CompleteTreeRawData {
        NodeBase *root_node;
        NodeBase *leftmost_node;
        NodeBase *rightmost_node;
        std::size_t tree_size;
    };
    explicit constexpr RBTreeContainerImpl(const CompleteTreeRawData data) noexcept : fake_root_{} {
        SetRootUnchecked(data.root_node);
        SetBeginUnchecked(data.leftmost_node);
        SetLastUnchecked(data.rightmost_node);
        SetSize(data.tree_size);
        SynchronizeInvariantsOnMove();
    }
    constexpr ~RBTreeContainerImpl() = default;

    constexpr void SetBegin(NodeBase *new_begin) noexcept {
        AssertBeginInvariants();
        SetBeginUnchecked(new_begin);
    }
    [[nodiscard]] constexpr NodeBase *Begin() noexcept ATTRIBUTE_LIFETIME_BOUND {
        AssertBeginInvariants();
        return GetBeginUnchecked();
    }
    [[nodiscard]] constexpr const NodeBase *Begin() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        AssertBeginInvariants();
        return GetBeginUnchecked();
    }
    [[nodiscard]] constexpr NodeBase *PreRootNil() noexcept ATTRIBUTE_LIFETIME_BOUND {
        AssertPreRootInvariants();
        return GetPreRootNilUnchecked();
    }
    [[nodiscard]] constexpr const NodeBase *PreRootNil() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        AssertPreRootInvariants();
        return GetPreRootNilUnchecked();
    }

    [[nodiscard]] constexpr NodeBase *Root() noexcept ATTRIBUTE_LIFETIME_BOUND {
        AssertRootInvariants();
        return GetRootUnchecked();
    }

    [[nodiscard]] constexpr const NodeBase *Root() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        AssertRootInvariants();
        return GetRootUnchecked();
    }

    [[nodiscard]] constexpr NodeBase *End() noexcept ATTRIBUTE_LIFETIME_BOUND {
        AssertEndInvariants();
        return GetEndUnchecked();
    }

    [[nodiscard]] constexpr const NodeBase *End() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        AssertEndInvariants();
        return GetEndUnchecked();
    }

    constexpr void SetLast(NodeBase *new_last) noexcept {
        AssertLastInvariants();
        SetLastUnchecked(new_last);
    }
    [[nodiscard]] constexpr const NodeBase *Last() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        AssertLastInvariants();
        return GetLastUnchecked();
    }
    [[nodiscard]] constexpr NodeBase *Last() noexcept ATTRIBUTE_LIFETIME_BOUND {
        AssertLastInvariants();
        return GetLastUnchecked();
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr std::size_t Size() const noexcept {
        return static_cast<std::size_t>(fake_root_.Color());
    }

    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_write, 2)
    ATTRIBUTE_ACCESS(read_write, 4)
    void InsertNode(NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER node_parent,
                    const bool is_left,
                    NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER node) noexcept {
        RBTREE_ASSERT_INVARIANT(node != nullptr);
        node->SetParent(node_parent);
        RBTREE_ASSERT_INVARIANT(node->Left() == nullptr);
        RBTREE_ASSERT_INVARIANT(node->Right() == nullptr);
        RBTREE_ASSERT_INVARIANT(node->Color() == NodeColor::kRed);
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

    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_only, 2)
    constexpr void ExtractNode(NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER node) noexcept {
        RBTREE_ASSERT_INVARIANT(Size() >= 1);
        RBTREE_ASSERT_INVARIANT(Begin() != End());
        RBTREE_ASSERT_INVARIANT(node != nullptr);
        RBTREE_ASSERT_INVARIANT(node != PreRootNil());
        NodeBase *const node_parent = node->Parent();
        RBTREE_ASSERT_INVARIANT(node_parent != nullptr);
        NodeBase *const node_left_child  = node->Left();
        NodeBase *const node_right_child = node->Right();
        NodeBase *propagated_node{};
        NodeBase *propagated_node_parent{};
        NodeColor color = node->Color();
        RBTREE_ASSERT_INVARIANT(color == NodeColor::kRed || color == NodeColor::kBlack);
        if (node_left_child == nullptr || node_right_child == nullptr) {
            propagated_node = node_left_child == nullptr ? node_right_child : node_left_child;
            propagated_node_parent = node_parent;
            RBTREE_ASSERT_INVARIANT(propagated_node == nullptr ||
                                    propagated_node->Color() == NodeColor::kRed);
            if (node_left_child == nullptr && node == GetBeginUnchecked()) {
                SetBeginUnchecked(node_right_child != nullptr ? node_right_child->LeftMostNode()
                                                              : node_parent);
            }
            if (node_right_child == nullptr && node == GetLastUnchecked()) {
                SetLastUnchecked(node_left_child != nullptr ? node_left_child->RightMostNode()
                                                            : node_parent);
            }
            Transplant(node, propagated_node);
        } else {
            RBTREE_ASSERT_INVARIANT(node != Begin());
            RBTREE_ASSERT_INVARIANT(node != Last());
            NodeBase *const node_successor = node_right_child->LeftMostNode();
            RBTREE_ASSERT_INVARIANT(node_successor != nullptr);
            RBTREE_ASSERT_INVARIANT(node_successor != PreRootNil());
            RBTREE_ASSERT_INVARIANT(node_successor->Left() == nullptr);
            color = node_successor->Color();
            RBTREE_ASSERT_INVARIANT(color == NodeColor::kRed || color == NodeColor::kBlack);
            propagated_node = node_successor->Right();
            RBTREE_ASSERT_INVARIANT(propagated_node == nullptr ||
                                    propagated_node->Parent() == node_successor);
            RBTREE_ASSERT_INVARIANT(propagated_node == nullptr ||
                                    propagated_node->Color() == NodeColor::kRed);
            if (node_successor != node_right_child) {
                propagated_node_parent = node_successor->Parent();
                Transplant(node_successor, propagated_node);
                node_successor->SetRight(node_right_child);
                node_right_child->SetParent(node_successor);
                node_right_child->SetParent(node_successor);
            } else {
                propagated_node_parent = node_successor;
            }
            Transplant(node, node_successor);
            node_successor->SetLeft(node_left_child);
            node_left_child->SetParent(node_successor);
            node_successor->SetColor(node->Color());
        }

        DecrementSize();
        if (color != NodeColor::kRed) {
            RBTREE_ASSERT_INVARIANT(color == NodeColor::kBlack);
            RebalanceAfterExtraction(propagated_node, propagated_node_parent);
        }
        RBTREE_ASSERT_INVARIANT(Size() == 0 || Root() != nullptr);
    }

private:
    constexpr void SetSize(std::size_t new_size) noexcept {
        static_assert(
            std::is_same_v<std::underlying_type_t<decltype(fake_root_.Color())>, std::size_t>);
        // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
        fake_root_.SetColor(static_cast<NodeColor>(new_size));
    }
    constexpr void IncrementSize() noexcept {
        SetSize(static_cast<std::size_t>(fake_root_.Color()) + 1);
    }

    constexpr void DecrementSize() noexcept {
        RBTREE_ASSERT_INVARIANT(Size() > 0);
        SetSize(static_cast<std::size_t>(fake_root_.Color()) - 1);
    }

    constexpr void SynchronizeInvariantsOnMove() noexcept {
        if (auto *root = GetRootUnchecked(); root != nullptr) {
            root->SetParent(GetPreRootNilUnchecked());
            RBTREE_ASSERT_INVARIANT(Size() >= 1);
        } else {
            SetBeginUnchecked(GetEndUnchecked());
            RBTREE_ASSERT_INVARIANT(Size() == 0);
        }

        AssertTreeInvariants();
    }

    constexpr void AssertTreeInvariants() const noexcept {
        AssertRootInvariants();
        AssertBeginInvariants();
        AssertEndInvariants();
        AssertPreRootInvariants();
    }

    constexpr void AssertRootInvariants() const noexcept {
        RBTREE_ASSERT_INVARIANT(GetRootUnchecked() == nullptr ||
                                GetRootUnchecked()->Color() == NodeColor::kBlack);
        RBTREE_ASSERT_INVARIANT(GetRootUnchecked() == nullptr ||
                                GetRootUnchecked()->Parent() == PreRootNil());
        RBTREE_ASSERT_INVARIANT((GetRootUnchecked() == nullptr && Size() == 0) ^
                                (GetRootUnchecked() != nullptr && Size() >= 1));
    }
    [[nodiscard]] constexpr NodeBase *GetRootUnchecked() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.Parent();
    }
    [[nodiscard]] constexpr const NodeBase *GetRootUnchecked() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.Parent();
    }
    [[nodiscard]] constexpr NodeBase *&GetRootMutableReferenceUnchecked() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.ParentMutableReference();
    }
    constexpr void SetRootUnchecked(NodeBase *new_root) noexcept {
        return fake_root_.SetParent(new_root);
    }

    constexpr void AssertPreRootInvariants() const noexcept {
        RBTREE_ASSERT_INVARIANT(GetRootUnchecked() == nullptr ||
                                GetRootUnchecked()->Parent() == GetPreRootNilUnchecked());
        RBTREE_ASSERT_INVARIANT((GetRootUnchecked() == nullptr && Size() == 0) ^
                                (GetRootUnchecked() != nullptr && Size() >= 1));
    }
    [[nodiscard]] constexpr NodeBase *GetPreRootNilUnchecked() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return std::addressof(fake_root_);
    }

    [[nodiscard]] constexpr const NodeBase *GetPreRootNilUnchecked() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return std::addressof(fake_root_);
    }

    constexpr void AssertBeginInvariants() const noexcept {
        RBTREE_ASSERT_INVARIANT(GetBeginUnchecked() != nullptr);
#ifdef RBTREE_DEBUG
        const auto invariants_if_not_empty =
            Size() >= 1 && GetBeginUnchecked() != GetEndUnchecked();
        const auto invariants_if_empty = Size() == 0 && GetBeginUnchecked() == GetEndUnchecked();
#endif
        RBTREE_ASSERT_INVARIANT(invariants_if_not_empty ^ invariants_if_empty);
        RBTREE_ASSERT_INVARIANT(GetBeginUnchecked() == GetEndUnchecked() ||
                                GetBeginUnchecked()->Left() == nullptr);
    }
    [[nodiscard]] constexpr NodeBase *GetBeginUnchecked() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.Right();
    }
    [[nodiscard]] constexpr const NodeBase *GetBeginUnchecked() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.Right();
    }
    constexpr void SetBeginUnchecked(NodeBase *new_begin) noexcept {
        return fake_root_.SetRight(new_begin);
    }

    constexpr void AssertEndInvariants() const noexcept {
        RBTREE_ASSERT_INVARIANT((Size() == 0 && GetBeginUnchecked() == GetEndUnchecked()) ^
                                (Size() >= 1 && GetBeginUnchecked() != GetEndUnchecked()));
    }
    [[nodiscard]] constexpr NodeBase *GetEndUnchecked() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return GetPreRootNilUnchecked();
    }

    [[nodiscard]] constexpr const NodeBase *GetEndUnchecked() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return GetPreRootNilUnchecked();
    }

    constexpr void AssertLastInvariants() const noexcept {
        RBTREE_ASSERT_INVARIANT(Size() >= 1);
        RBTREE_ASSERT_INVARIANT((Size() >= 2) ^ (GetBeginUnchecked() == GetLastUnchecked() &&
                                                 GetLastUnchecked() == GetRootUnchecked()));
        RBTREE_ASSERT_INVARIANT(GetLastUnchecked() != nullptr);
        RBTREE_ASSERT_INVARIANT(GetLastUnchecked() != GetPreRootNilUnchecked());
        RBTREE_ASSERT_INVARIANT(GetLastUnchecked()->Right() == nullptr);
    }
    [[nodiscard]] constexpr NodeBase *GetLastUnchecked() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.Left();
    }
    [[nodiscard]] constexpr const NodeBase *GetLastUnchecked() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.Left();
    }
    constexpr void SetLastUnchecked(NodeBase *new_last) noexcept {
        return fake_root_.SetLeft(new_last);
    }

    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
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
        RBTREE_ASSERT_INVARIANT(node != root->Parent());
        NodeBase *const parent = node->Parent();
        RBTREE_ASSERT_INVARIANT(parent != nullptr);
        RBTREE_ASSERT_INVARIANT(parent->Right() == node);
        NodeBase *const parent_parent = parent->Parent();

        parent->SetRight(node->Left());
        if (auto *node_left = node->Left(); node_left != nullptr) {
            node_left->SetParent(parent);
        }

        node->SetLeft(parent);
        parent->SetParent(node);

        RBTREE_ASSERT_INVARIANT(parent_parent != nullptr);
        node->SetParent(parent_parent);
        if (parent == root) [[unlikely]] {
            root = node;
        } else if (parent_parent->Left() == parent) {
            parent_parent->SetLeft(node);
        } else {
            RBTREE_ASSERT_INVARIANT(parent_parent->Right() == parent);
            parent_parent->SetRight(node);
        }
    }

    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
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
        NodeBase *const parent = node->Parent();
        RBTREE_ASSERT_INVARIANT(parent != nullptr);
        RBTREE_ASSERT_INVARIANT(parent->Left() == node);
        NodeBase *const parent_parent = parent->Parent();

        parent->SetLeft(node->Right());
        if (auto *node_right = node->Right(); node_right != nullptr) {
            node_right->SetParent(parent);
        }

        node->SetRight(parent);
        parent->SetParent(node);

        RBTREE_ASSERT_INVARIANT(parent_parent != nullptr);
        node->SetParent(parent_parent);
        if (parent == root) [[unlikely]] {
            root = node;
        } else if (parent_parent->Left() == parent) {
            parent_parent->SetLeft(node);
        } else {
            RBTREE_ASSERT_INVARIANT(parent_parent->Right() == parent);
            parent_parent->SetRight(node);
        }
    }

    // clang-format off

    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_write, 2)
    ATTRIBUTE_ACCESS(read_only, 3)
    constexpr void MakeLeftOnInsertion(NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER node_parent,
                                       NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER node) noexcept {
        RBTREE_ASSERT_INVARIANT(node_parent != nullptr);
        RBTREE_ASSERT_INVARIANT(node_parent != PreRootNil() && node_parent->Left() == nullptr);
        RBTREE_ASSERT_INVARIANT(node != nullptr);
        RBTREE_ASSERT_INVARIANT(node->Parent() == node_parent);
        RBTREE_ASSERT_INVARIANT(node->Left() == nullptr);
        RBTREE_ASSERT_INVARIANT(node->Right() == nullptr);
        if (node_parent == Begin()) {
            SetBegin(node);
        }
        node_parent->SetLeft(node);
        RBTREE_ASSERT_INVARIANT(GetBeginUnchecked() != nullptr);
        RBTREE_ASSERT_INVARIANT(GetLastUnchecked() != nullptr);
    }

    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_write, 2)
    ATTRIBUTE_ACCESS(read_only, 3)
    constexpr void MakeRightOnInsertion(NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER node_parent,
                                       NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER node) noexcept {
        RBTREE_ASSERT_INVARIANT(node_parent != nullptr);
        RBTREE_ASSERT_INVARIANT(node_parent == PreRootNil() || node_parent->Right() == nullptr);
        RBTREE_ASSERT_INVARIANT(node != nullptr);
        RBTREE_ASSERT_INVARIANT(node->Parent() == node_parent);
        RBTREE_ASSERT_INVARIANT(node->Left() == nullptr);
        RBTREE_ASSERT_INVARIANT(node->Right() == nullptr);
        if (node_parent == PreRootNil()) [[unlikely]] {
            SetRootUnchecked(node) ;
            // SetBeginUnchecked(node); will be set during `node_parent->SetRight(node)`
            SetLastUnchecked(node)  ;
        } else if (node_parent == Last()) {
            SetLast(node);
        }
        node_parent->SetRight(node);
        RBTREE_ASSERT_INVARIANT(GetBeginUnchecked() != nullptr);
        RBTREE_ASSERT_INVARIANT(GetLastUnchecked() != nullptr);
    }

    // clang-format on

    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_write, 2)
    constexpr void RebalanceAfterInsertion(
        NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER new_inserted_node) noexcept {
        RBTREE_ASSERT_INVARIANT(new_inserted_node != nullptr);
        RBTREE_ASSERT_INVARIANT(new_inserted_node->Color() == NodeColor::kRed);
        RBTREE_ASSERT_INVARIANT(Size() >= 1);
        NodeBase *CONFIG_CLANG_NONNULL_QUALIFIER current_node = new_inserted_node;
        NodeBase *current_node_parent                         = nullptr;
        NodeBase *&root                                       = GetRootMutableReferenceUnchecked();
        while ((current_node_parent = current_node->Parent()) != root &&
               current_node_parent->Color() == NodeColor::kRed) {
            RBTREE_ASSERT_INVARIANT(current_node_parent != GetPreRootNilUnchecked());
            RBTREE_ASSERT_INVARIANT(current_node_parent->OtherSibling(current_node) == nullptr ||
                                    current_node_parent->OtherSibling(current_node)->Color() ==
                                        NodeColor::kBlack);
            NodeBase *CONFIG_CLANG_NONNULL_QUALIFIER current_node_parent_parent =
                current_node_parent->Parent();
            RBTREE_ASSERT_INVARIANT(current_node_parent_parent != nullptr);
            RBTREE_ASSERT_INVARIANT(current_node_parent_parent->Color() == NodeColor::kBlack);

            NodeBase *current_node_parent_sibling =
                current_node_parent_parent->OtherSibling(current_node_parent);
            if (current_node_parent_sibling != nullptr &&
                current_node_parent_sibling->Color() == NodeColor::kRed) {
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
                RBTREE_ASSERT_INVARIANT(current_node_parent_sibling->Left() == nullptr ||
                                        current_node_parent_sibling->Left()->Color() ==
                                            NodeColor::kBlack);
                RBTREE_ASSERT_INVARIANT(current_node_parent_sibling->Left() !=
                                        GetPreRootNilUnchecked());
                RBTREE_ASSERT_INVARIANT(current_node_parent_sibling->Right() == nullptr ||
                                        current_node_parent_sibling->Right()->Color() ==
                                            NodeColor::kBlack);
                RBTREE_ASSERT_INVARIANT(current_node_parent_sibling->Right() !=
                                        GetPreRootNilUnchecked());
                RBTREE_ASSERT_INVARIANT((current_node_parent_sibling->Left() == nullptr &&
                                         current_node_parent_sibling->Right() == nullptr) ^
                                        (current_node_parent_sibling->Left() != nullptr &&
                                         current_node_parent_sibling->Right() != nullptr));
                current_node_parent_parent->SetColor(NodeColor::kRed);
                current_node_parent_sibling->SetColor(NodeColor::kBlack);
                current_node_parent->SetColor(NodeColor::kBlack);
                current_node = current_node_parent_parent;
                continue;
            }

            /**
             * if (current_node_parent == current_node_parent_parent->Left()) {
             *     if (current_node == current_node_parent->Right()) {
             *         current_node->SetColor(Color::kBlack);
             *         current_node_parent_parent->SetColor(Color::kRed);
             *         LeftRotate(current_node);
             *         RightRotate(current_node);
             *     } else {
             *         current_node_parent->SetColor(Color::kBlack);
             *         current_node_parent_parent->SetColor(Color::kRed);
             *         RightRotate(current_node_parent)
             *     }
             * } else {
             *     if (current_node == current_node_parent->Left()) {
             *         current_node->SetColor(Color::kBlack);
             *         current_node_parent_parent->SetColor(Color::kRed);
             *         RightRotate(current_node);
             *         LeftRotate(current_node);
             *     } else {
             *         current_node_parent->SetColor(Color::kBlack);
             *         current_node_parent_parent->SetColor(Color::kRed);
             *         LeftRotate(current_node_parent);
             *     }
             * }
             */
            if (current_node_parent == current_node_parent_parent->Left()) {
                if (current_node == current_node_parent->Right()) {
                    LeftRotate(current_node, root);
                    current_node_parent = current_node;
                } else {
                    RBTREE_ASSERT_INVARIANT(current_node == current_node_parent->Left());
                }
                RightRotate(current_node_parent, root);
            } else {
                RBTREE_ASSERT_INVARIANT(current_node_parent == current_node_parent_parent->Right());
                if (current_node == current_node_parent->Left()) {
                    RightRotate(current_node, root);
                    current_node_parent = current_node;
                } else {
                    RBTREE_ASSERT_INVARIANT(current_node == current_node_parent->Right());
                }
                LeftRotate(current_node_parent, root);
            }
            current_node_parent_parent->SetColor(NodeColor::kRed);
            current_node_parent->SetColor(NodeColor::kBlack);
            break;
        }

        RBTREE_ASSERT_INVARIANT(root != nullptr);
        root->SetColor(NodeColor::kBlack);
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
            SetRootUnchecked(with_node);
        } else if (which_node == which_node_parent->Left()) {
            which_node_parent->SetLeft(with_node);
        } else {
            RBTREE_ASSERT_INVARIANT(which_node == which_node_parent->Right());
            which_node_parent->SetRight(with_node);
        }
        if (with_node != nullptr) {
            with_node->SetParent(which_node_parent);
        }
    }

    RBTREE_ATTRIBUTE_NONNULL(3)
    ATTRIBUTE_ACCESS(read_write, 2)
    constexpr void RebalanceAfterExtraction(NodeBase *const CONFIG_CLANG_NULLABLE_QUALIFIER node,
                                            NodeBase *const CONFIG_CLANG_NONNULL_QUALIFIER
                                                node_parent) noexcept {
        RBTREE_ASSERT_INVARIANT(node != GetPreRootNilUnchecked());
        NodeBase *current_node        = node;
        NodeBase *current_node_parent = node_parent;
        NodeBase *&root               = GetRootMutableReferenceUnchecked();
        RBTREE_ASSERT_INVARIANT(root != GetPreRootNilUnchecked());
        while (current_node != root &&
               (current_node == nullptr || current_node->Color() == NodeColor::kBlack)) {
            RBTREE_ASSERT_INVARIANT(root != nullptr);
            RBTREE_ASSERT_INVARIANT(current_node_parent != nullptr);
            RBTREE_ASSERT_INVARIANT(current_node_parent != PreRootNil());
            if (current_node == current_node_parent->Left()) {
                NodeBase *current_node_sibling = current_node_parent->Right();
                RBTREE_ASSERT_INVARIANT(current_node_sibling != nullptr);
                if (current_node_sibling->Color() == NodeColor::kRed) {
                    RBTREE_ASSERT_INVARIANT(current_node_sibling != PreRootNil());
                    RBTREE_ASSERT_INVARIANT(current_node_sibling != root);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Left() != PreRootNil());
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Left() != nullptr);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Left()->Color() ==
                                            NodeColor::kBlack);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Right() != PreRootNil());
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Right() != nullptr);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Right()->Color() ==
                                            NodeColor::kBlack);
                    current_node_sibling->SetColor(NodeColor::kBlack);
                    current_node_parent->SetColor(NodeColor::kRed);
#ifdef RBTREE_DEBUG
                    const NodeBase *const current_node_sibling_original_left_child =
                        current_node_sibling->Left();
                    RBTREE_ASSERT_INVARIANT(current_node_sibling_original_left_child != nullptr);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling_original_left_child->Color() ==
                                            NodeColor::kBlack);
#endif
                    LeftRotate(current_node_sibling, root);
                    RBTREE_ASSERT_INVARIANT(current_node_parent->Right() != current_node_sibling);
                    RBTREE_ASSERT_INVARIANT(current_node_parent->Right() ==
                                            current_node_sibling_original_left_child);
                    current_node_sibling = current_node_parent->Right();
                    RBTREE_ASSERT_INVARIANT(current_node_sibling != nullptr);
                }
                NodeBase *w = current_node_sibling;
                RBTREE_ASSERT_INVARIANT(w != nullptr);
                NodeBase *w_left  = w->Left();
                NodeBase *w_right = w->Right();
                if ((w_left == nullptr || w_left->Color() == NodeColor::kBlack) &&
                    (w_right == nullptr || w_right->Color() == NodeColor::kBlack)) {
                    w->SetColor(NodeColor::kRed);
                    RBTREE_ASSERT_INVARIANT(current_node == nullptr ||
                                            current_node_parent == current_node->Parent());
                    current_node        = current_node_parent;
                    current_node_parent = current_node_parent->Parent();
                } else {
                    RBTREE_ASSERT_INVARIANT(w_right == nullptr || w_right->Parent() == w);
                    if (w_right == nullptr || w_right->Color() == NodeColor::kBlack) {
                        RBTREE_ASSERT_INVARIANT(w_left != nullptr);
                        RBTREE_ASSERT_INVARIANT(w_left->Parent() == w);
                        RBTREE_ASSERT_INVARIANT(w_left->Color() == NodeColor::kRed);
                        w_left->SetColor(NodeColor::kBlack);
                        w->SetColor(NodeColor::kRed);
                        RightRotate(w_left, root);
                        RBTREE_ASSERT_INVARIANT(current_node == nullptr ||
                                                current_node_parent ==
                                                    current_node->Parent());  // ????
                        RBTREE_ASSERT_INVARIANT(current_node_parent->Right() == w_left);
                        w      = w_left;
                        w_left = nullptr;  // to ensure this won't be used later and thus should not
                                           // be set to w->Left()
                        w_right = w->Right();
                    } else {
                        RBTREE_ASSERT_INVARIANT(w_right->Color() == NodeColor::kRed);
                    }
                    RBTREE_ASSERT_INVARIANT(current_node == nullptr ||
                                            current_node_parent == current_node->Parent());  // ????
                    w->SetColor(current_node_parent->Color());
                    current_node_parent->SetColor(NodeColor::kBlack);
                    RBTREE_ASSERT_INVARIANT(w_right == w->Right());
                    if (w_right != nullptr) {
                        w_right->SetColor(NodeColor::kBlack);
                    }
                    LeftRotate(w, root);
                    // useless write, just in order to follow the Cormen's book
                    current_node = root;
                    break;
                }
            } else {
                RBTREE_ASSERT_INVARIANT(current_node == nullptr ||
                                        current_node == current_node_parent->Right());
                NodeBase *current_node_sibling = current_node_parent->Left();
                RBTREE_ASSERT_INVARIANT(current_node_sibling != nullptr);
                if (current_node_sibling->Color() == NodeColor::kRed) {
                    RBTREE_ASSERT_INVARIANT(current_node_sibling != PreRootNil());
                    RBTREE_ASSERT_INVARIANT(current_node_sibling != root);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Left() != PreRootNil());
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Left() != nullptr);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Left()->Color() ==
                                            NodeColor::kBlack);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Right() != PreRootNil());
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Right() != nullptr);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Right()->Color() ==
                                            NodeColor::kBlack);
                    current_node_sibling->SetColor(NodeColor::kBlack);
                    current_node_parent->SetColor(NodeColor::kRed);
#ifdef RBTREE_DEBUG
                    const NodeBase *const current_node_sibling_original_right_child =
                        current_node_sibling->Right();
                    RBTREE_ASSERT_INVARIANT(current_node_sibling_original_right_child != nullptr);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling_original_right_child->Color() ==
                                            NodeColor::kBlack);
#endif
                    RightRotate(current_node_sibling, root);
                    RBTREE_ASSERT_INVARIANT(current_node_parent->Left() != current_node_sibling);
                    RBTREE_ASSERT_INVARIANT(current_node_parent->Left() ==
                                            current_node_sibling_original_right_child);
                    current_node_sibling = current_node_parent->Left();
                    RBTREE_ASSERT_INVARIANT(current_node_sibling != nullptr);
                }
                NodeBase *w = current_node_sibling;
                RBTREE_ASSERT_INVARIANT(w != nullptr);
                NodeBase *w_left  = w->Left();
                NodeBase *w_right = w->Right();
                if ((w_left == nullptr || w_left->Color() == NodeColor::kBlack) &&
                    (w_right == nullptr || w_right->Color() == NodeColor::kBlack)) {
                    w->SetColor(NodeColor::kRed);
                    RBTREE_ASSERT_INVARIANT(current_node == nullptr ||
                                            current_node_parent == current_node->Parent());
                    current_node        = current_node_parent;
                    current_node_parent = current_node_parent->Parent();
                } else {
                    RBTREE_ASSERT_INVARIANT(w_left == nullptr || w_left->Parent() == w);
                    if (w_left == nullptr || w_left->Color() == NodeColor::kBlack) {
                        RBTREE_ASSERT_INVARIANT(w_right != nullptr);
                        RBTREE_ASSERT_INVARIANT(w_right->Parent() == w);
                        RBTREE_ASSERT_INVARIANT(w_right->Color() == NodeColor::kRed);
                        w_right->SetColor(NodeColor::kBlack);
                        w->SetColor(NodeColor::kRed);
                        LeftRotate(w_right, root);
                        RBTREE_ASSERT_INVARIANT(current_node == nullptr ||
                                                current_node_parent == current_node->Parent());
                        RBTREE_ASSERT_INVARIANT(current_node_parent->Left() == w_right);
                        w       = w_right;
                        w_right = nullptr;  // to ensure this won't be used later and thus should
                                            // not be set to w->Right()
                        w_left = w->Left();
                    } else {
                        RBTREE_ASSERT_INVARIANT(w_left->Color() == NodeColor::kRed);
                    }
                    RBTREE_ASSERT_INVARIANT(current_node == nullptr ||
                                            current_node_parent == current_node->Parent());  // ????
                    w->SetColor(current_node_parent->Color());
                    current_node_parent->SetColor(NodeColor::kBlack);
                    RBTREE_ASSERT_INVARIANT(w_left == w->Left());
                    if (w_left != nullptr) {
                        w_left->SetColor(NodeColor::kBlack);
                    }
                    RightRotate(w, root);
                    // useless write, just in order to follow the Cormen's book
                    current_node = root;
                    break;
                }
            }
        }

        if (current_node != nullptr) {
            current_node->SetColor(NodeColor::kBlack);
        }
        RBTREE_ASSERT_INVARIANT(Size() == 0 || Root() != nullptr);
    }

    NodeBase fake_root_;
};

// clang-format off

#ifdef RBTREE_DEBUG

[[nodiscard]]
ATTRIBUTE_ACCESS(read_only, 1)
ATTRIBUTE_PURE constexpr bool IsFakeNodeOrRoot(const NodeBase *node) noexcept {
    if (node == nullptr) {
        return false;
    }

    const NodeBase *parent = node->Parent();
    return parent != nullptr && parent->Parent() == node;
}

#endif

template <class NodeType>
[[nodiscard]]
RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
ATTRIBUTE_ACCESS(read_only, 1)
ATTRIBUTE_PURE
constexpr NodeType *Increment(NodeType *CONFIG_CLANG_NONNULL_QUALIFIER node) noexcept {
    if (auto *right_child = node->Right(); right_child != nullptr) {
        return right_child->LeftMostNode();
    }

    auto *parent = node->Parent();
    RBTREE_ASSERT_INVARIANT(parent != nullptr);
    RBTREE_ASSERT_INVARIANT(
        parent->Color() == NodeColor::kRed || parent->Color() == NodeColor::kBlack ||
        IsFakeNodeOrRoot(parent));
    while (node == parent->Right()) {
        node   = parent;
        parent = node->Parent();
        RBTREE_ASSERT_INVARIANT(parent != nullptr);
        RBTREE_ASSERT_INVARIANT(
            (parent->Color() == NodeColor::kRed || parent->Color() == NodeColor::kBlack) ||
            IsFakeNodeOrRoot(parent));
    }

    RBTREE_ASSERT_INVARIANT(node == parent->Left() ||
                            (IsFakeNodeOrRoot(parent) && parent->Parent() == node) ||
                            (IsFakeNodeOrRoot(node) && node->Right() == parent));
    return node->Right() != parent ? parent : node;
}

template <class NodeType>
[[nodiscard]] RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS ATTRIBUTE_ACCESS(read_only, 1) ATTRIBUTE_PURE
    constexpr NodeType *Decrement(NodeType *CONFIG_CLANG_NONNULL_QUALIFIER node) noexcept {
    RBTREE_ASSERT_INVARIANT(node != nullptr);
    if (auto *left_child = node->Left(); left_child != nullptr) {
        return left_child->RightMostNode();
    }

    auto *parent = node->Parent();
    RBTREE_ASSERT_INVARIANT(parent != nullptr);
    RBTREE_ASSERT_INVARIANT(
        (parent->Color() == NodeColor::kRed || parent->Color() == NodeColor::kBlack) ||
        IsFakeNodeOrRoot(parent));
    while (node == parent->Left()) {
        node   = parent;
        parent = node->Parent();
        RBTREE_ASSERT_INVARIANT(parent != nullptr);
        RBTREE_ASSERT_INVARIANT(parent->Color() == NodeColor::kRed ||
                                parent->Color() == NodeColor::kBlack);
    }

    RBTREE_ASSERT_INVARIANT(node == parent->Right());
    return parent;
}

// clang-format on

template <class KeyType, class DerivedRBTree>
class RBTreeContainer;

template <class KeyType, bool IsConstIterator, class RBTreeFriend>
class Iterator final {
    using IterNodeBaseType = std::conditional_t<IsConstIterator, const NodeBase, NodeBase>;
    using IterNodeType  = std::conditional_t<IsConstIterator, const Node<KeyType>, Node<KeyType>>;
    using IterKeyType   = std::conditional_t<IsConstIterator, const KeyType, KeyType>;
    using OtherIterator = Iterator<KeyType, !IsConstIterator, RBTreeFriend>;

    friend class RBTreeContainer<KeyType, RBTreeFriend>;
    friend OtherIterator;
    friend RBTreeFriend;

    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
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
    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    /* implicit */ constexpr Iterator(OtherIterator other) noexcept
        requires(IsConstIterator)
        : Iterator(other.node_) {}

    explicit Iterator(std::nullptr_t) = delete;

    [[nodiscard]] constexpr reference operator*() const noexcept {
        return node_->Key();
    }

    [[nodiscard]] ATTRIBUTE_RETURNS_NONNULL constexpr pointer operator->() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return std::addressof(node_->Key());
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
    static_assert(sizeof(KeyType) >= 1, "Type can't be incomplete");
    static_assert(!std::is_array_v<KeyType>, "Type can't be array");
    static_assert(std::is_same_v<KeyType, std::remove_cvref_t<KeyType>>,
                  "Type can't be const nor reference");

    using Base = RBTreeContainerImpl;

protected:
    using size_type              = std::size_t;
    using difference_type        = std::ptrdiff_t;
    using const_iterator         = Iterator<KeyType, /*IsConstIterator = */ true, DerivedRBTree>;
    using iterator               = const_iterator;
    using value_type             = KeyType;
    using key_type               = KeyType;
    using reference              = value_type &;
    using const_reference        = const value_type &;
    using NodeType               = Node<KeyType>;
    using allocator_type         = std::allocator<NodeType>;
    using allocator_traits       = std::allocator_traits<allocator_type>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using node_type              = NodeType;

    static_assert(std::bidirectional_iterator<const_iterator>);
    static_assert(std::bidirectional_iterator<iterator>);

    static constexpr bool kIsNodeNoexceptDestructible = std::is_nothrow_destructible_v<NodeType>;

    constexpr RBTreeContainer() noexcept = default;
    constexpr RBTreeContainer(const RBTreeContainer &other) : Base(CloneTree(other)) {}
    constexpr RBTreeContainer &operator=(const RBTreeContainer &other) ATTRIBUTE_LIFETIME_BOUND {
        // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)
        return *this = RBTreeContainer(other);  // NOLINT(misc-unconventional-assign-operator)
    }
    constexpr RBTreeContainer(RBTreeContainer &&)            = default;
    constexpr RBTreeContainer &operator=(RBTreeContainer &&) = default;
    constexpr ~RBTreeContainer() noexcept(noexcept(DeleteTree(Root()))) {
        DeleteTree(Root());
    }
    constexpr void Swap(RBTreeContainer &other) noexcept {
        Base::Swap(other);
    }
    ATTRIBUTE_REINITIALIZES constexpr void clear() noexcept {
        RBTreeContainer{}.Swap(*this);
    }

    // clang-format off
    [[nodiscard]] constexpr size_type size() const noexcept {
        const size_type value = Base::Size();
        RBTREE_ASSERT_INVARIANT(value <= max_size());
        CONFIG_ASSUME_STATEMENT(value <= max_size());
        return value;
    }
    [[nodiscard]] constexpr bool empty() const noexcept {
        return size() == 0;
    }
    [[nodiscard]] constexpr const_iterator begin() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return const_iterator{static_cast<const NodeType *>(Base::Begin())};
    }
    [[nodiscard]] constexpr const_iterator end() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return iterator{static_cast<const NodeType *>(Base::End())};
    }
    [[nodiscard]] constexpr iterator begin() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return iterator(static_cast<NodeType *>(Base::Begin()));
    }
    [[nodiscard]] constexpr iterator end() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return iterator{static_cast<NodeType *>(Base::End())};
    }
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return begin();
    }
    [[nodiscard]] constexpr const_iterator cend() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return end();
    }
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return const_reverse_iterator{end()};
    }
    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return const_reverse_iterator{begin()};
    }
    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return reverse_iterator{end()};
    }
    [[nodiscard]] constexpr reverse_iterator rend() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return reverse_iterator{begin()};
    }
    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return rbegin();
    }
    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return rend();
    }
    [[nodiscard]] constexpr const_reference front() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return *begin();
    }
    [[nodiscard]] constexpr reference front() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return *begin();
    }
    [[nodiscard]] constexpr const_reference back() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return static_cast<const NodeType *>(Base::Last())->Key();
    }
    [[nodiscard]] constexpr reference back() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return static_cast<NodeType *>(Base::Last())->Key();
    }

    [[nodiscard]] ATTRIBUTE_CONST static consteval size_type max_size() noexcept {
        constexpr auto kMaxSSize = static_cast<size_type>(std::numeric_limits<difference_type>::max()) / sizeof(NodeType);
        constexpr auto kMaxUSize = std::numeric_limits<size_type>::max() / sizeof(NodeType);
        constexpr auto kMaxAllocSize = allocator_traits::max_size(GetAllocator());
        constexpr auto kMaxSize = std::min({kMaxUSize, kMaxSSize, kMaxAllocSize});
        return kMaxSize;
    }

    [[nodiscard]] constexpr NodeType *Root() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return static_cast<NodeType *>(Base::Root());
    }
    [[nodiscard]] constexpr const NodeType *Root() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return static_cast<const NodeType *>(Base::Root());
    }
    [[nodiscard]] constexpr const NodeType *PreRootNodePtr() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return static_cast<const NodeType *>(Base::PreRootNil());
    }
    [[nodiscard]] constexpr NodeType *PreRootNodePtr() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return static_cast<NodeType *>(Base::PreRootNil());
    }

    template <class... Args>
    [[nodiscard]]
    RBTREE_ATTRIBUTE_NONNULL(2)
    ATTRIBUTE_ACCESS(read_write, 2)
    constexpr NodeType *InsertNode(NodeType *const CONFIG_CLANG_NONNULL_QUALIFIER node_parent,
                                   const bool insert_left,
                                   Args &&...args) ATTRIBUTE_LIFETIME_BOUND {
        auto *const new_node = CreateNode(std::forward<Args>(args)...);
        Base::InsertNode(node_parent, insert_left, new_node);
        return new_node;
    }

    constexpr void ExtractAndDestroyNode(const_iterator iter) noexcept(kIsNodeNoexceptDestructible) {
        auto *const node = const_cast<NodeType *>(iter.node_);
        RBTREE_ASSERT_INVARIANT(node != nullptr);
        Base::ExtractNode(node);
        DestroyNode(node);
    }

    // clang-format on

private:
    template <class... Args>
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE constexpr NodeType *CreateNode(Args &&...args) {
        return CheckSizeAndCreateNode(this->size(), std::forward<Args>(args)...);
    }

    // clang-format off
    template <class... Args>
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE
    static constexpr NodeType *CheckSizeAndCreateNode(const size_type tree_size, Args &&...args)
        requires(std::constructible_from<NodeType, Args...>) {
        // clang-format on
        if (tree_size >= max_size()) [[unlikely]] {
            RBTREE_ASSERT_INVARIANT(tree_size == max_size());
            ThrowLengthError(std::source_location::current().function_name());
        }

        return CreateNodeUnchecked(std::forward<Args>(args)...);
    }

    template <class... Args>
    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE static constexpr NodeType *CreateNodeUnchecked(
        Args &&...args) {
        class NodeStorageProxy final {
        public:
            using pointer = NodeType *;

        private:
            // clang-format off
            [[nodiscard]]
            ATTRIBUTE_ALWAYS_INLINE
            ATTRIBUTE_RETURNS_NONNULL
            static constexpr pointer AllocateStorage() {
                // clang-format on
                allocator_type alloc = GetAllocator();
                return allocator_traits::allocate(alloc, 1);
            }

        public:
            ATTRIBUTE_ALWAYS_INLINE
            constexpr NodeStorageProxy() : node_storage_(AllocateStorage()) {}
            NodeStorageProxy(const NodeStorageProxy &)                     = delete;
            NodeStorageProxy &operator=(const NodeStorageProxy &)          = delete;
            NodeStorageProxy(NodeStorageProxy &&other) noexcept            = delete;
            NodeStorageProxy &operator=(NodeStorageProxy &&other) noexcept = delete;
            ATTRIBUTE_ALWAYS_INLINE constexpr ~NodeStorageProxy() {
                if (node_storage_) {
                    allocator_type alloc = GetAllocator();
                    allocator_traits::destroy(alloc, node_storage_);
                    allocator_traits::deallocate(alloc, node_storage_, 1);
                }
            }
            // clang-format off
            [[nodiscard]]
            ATTRIBUTE_ALWAYS_INLINE
            constexpr pointer ConstructNodeAndReleaseStorage(Args... args) noexcept {
                // clang-format on
                allocator_type alloc = GetAllocator();
                RBTREE_ASSERT_INVARIANT(node_storage_ != nullptr);
                allocator_traits::construct(alloc, node_storage_, std::forward<Args>(args)...);
                return std::exchange(node_storage_, nullptr);
            }

        private:
            pointer node_storage_{};
        };

        return NodeStorageProxy{}.ConstructNodeAndReleaseStorage(std::forward<Args>(args)...);
    }

    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE static constexpr allocator_type GetAllocator() noexcept {
        static_assert(
            sizeof(allocator_type) <= 1 && std::is_nothrow_default_constructible_v<allocator_type>,
            "Stateless allocator is expected");
        return allocator_type{};
    }

    static constexpr void DestroyNode(NodeType *const node) noexcept(kIsNodeNoexceptDestructible) {
        static_assert(std::is_destructible_v<KeyType>);
        allocator_type alloc = GetAllocator();
        allocator_traits::destroy(alloc, node);
        allocator_traits::deallocate(alloc, node, 1);
    }

    struct ClonedTreeData {
        NodeType *root_node{};
        NodeType *left_most_node{};
        NodeType *right_most_node{};
    };
    static constexpr CompleteTreeRawData CloneTree(const RBTreeContainer &other) {
        const NodeType *const other_root = other.Root();
        const ClonedTreeData tree_data =
            other_root != nullptr ? CloneTreeImpl(other_root) : ClonedTreeData{};
        return CompleteTreeRawData{
            .root_node      = tree_data.root_node,
            .leftmost_node  = tree_data.left_most_node,
            .rightmost_node = tree_data.right_most_node,
            .tree_size      = other.size(),
        };
    }
    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_only, 1)
    static constexpr ClonedTreeData CloneTreeImpl(
        const NodeType *const CONFIG_CLANG_NONNULL_QUALIFIER other_root) {
        RBTREE_ASSERT_INVARIANT(other_root != nullptr);
        struct TemporaryTreeDeleter {
            void operator()(NodeType *node) const noexcept {
                DeleteTree(node);
            }
        };
        using TemporaryTreeHolder = std::unique_ptr<NodeType, TemporaryTreeDeleter>;
        TemporaryTreeHolder new_root(CreateNodeUnchecked(other_root->Key()));
        new_root->SetColor(other_root->Color());
        NodeType *left_most_node  = new_root.get();
        NodeType *right_most_node = new_root.get();
        if (const NodeType *left = other_root->Left(); left != nullptr) {
            const ClonedTreeData left_clone_data = CloneTreeImpl(left);
            new_root->SetLeft(left_clone_data.root_node);
            left_clone_data.root_node->SetParent(new_root.get());
            left_most_node = left_clone_data.left_most_node;
        }
        if (const NodeType *right = other_root->Right(); right != nullptr) {
            const ClonedTreeData right_clone_data = CloneTreeImpl(right);
            new_root->SetRight(right_clone_data.root_node);
            right_clone_data.root_node->SetParent(new_root.get());
            right_most_node = right_clone_data.right_most_node;
        }

        return ClonedTreeData{
            .root_node       = new_root.release(),
            .left_most_node  = left_most_node,
            .right_most_node = right_most_node,
        };
    }

    ATTRIBUTE_ACCESS(read_write, 1)
    static constexpr void DeleteTree(Node<KeyType> *const root) noexcept(
        std::is_nothrow_destructible_v<KeyType>) {
        if (root == nullptr) {
            return;
        }
        root->SetParent(nullptr);
        Node<KeyType> *local_root = root;
        do {
            Node<KeyType> *const left  = local_root->Left();
            Node<KeyType> *const right = local_root->Right();
            Node<KeyType> *next_node{};
            if (left != nullptr && right != nullptr) {
                RBTREE_ASSERT_INVARIANT(left->Parent() == local_root);
                RBTREE_ASSERT_INVARIANT(right->Parent() == local_root);
                right->SetParent(local_root->Parent());
                left->SetParent(right);
                next_node = left;
            } else if (left != nullptr) {
                RBTREE_ASSERT_INVARIANT(left->Parent() == local_root);
                left->SetParent(local_root->Parent());
                next_node = left;
            } else if (right != nullptr) {
                RBTREE_ASSERT_INVARIANT(right->Parent() == local_root);
                right->SetParent(local_root->Parent());
                next_node = right;
            } else {
                next_node = local_root->Parent();
            }

            DestroyNode(local_root);
            local_root = next_node;
        } while (local_root != nullptr);
    }

    // clang-format off
    [[noreturn]]
    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_only, 1)
    ATTRIBUTE_COLD
    static void ThrowLengthError(const char *CONFIG_CLANG_NONNULL_QUALIFIER function_name) {
        // clang-format on
        constexpr std::size_t kErrorReportBufferSize = 1024;
        std::array<char, kErrorReportBufferSize> buffer{};
        FillLengthErrorReport(function_name, buffer);
        throw std::length_error(std::data(buffer));
    }

    // clang-format off
    template <std::size_t N>
    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_only, 1)
    ATTRIBUTE_ACCESS(write_only, 2)
    static void FillLengthErrorReport(const char *CONFIG_CLANG_NONNULL_QUALIFIER function_name,
                                      std::array<char, N>& buffer) noexcept {
        // clang-format on
        const int ret = std::snprintf(std::data(buffer), std::size(buffer),
                                      "Could not create node: size error at %s", function_name);
        if (ret <= 0) [[unlikely]] {
            constexpr std::array kFallbackReport = std::to_array(
                "Could not create error report on node creation error because of the "
                "size error");
            static_assert(std::size(kFallbackReport) < N);
            std::ranges::copy(kFallbackReport, std::begin(buffer));
        }
    }
};

#undef RBTREE_ATTRIBUTE_NONNULL
#undef RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

template <class T>
inline constexpr bool kUseByValue =
    std::is_trivial_v<T> ||
    (std::is_standard_layout_v<T> && !std::is_polymorphic_v<T> && !std::is_array_v<T> &&
     std::is_nothrow_default_constructible_v<T> && sizeof(T) <= 32 &&
     std::is_nothrow_copy_constructible_v<T> && std::is_trivially_copy_constructible_v<T> &&
     std::is_nothrow_copy_assignable_v<T> && std::is_trivially_copy_assignable_v<T> &&
     std::is_nothrow_move_constructible_v<T> && std::is_trivially_move_constructible_v<T> &&
     std::is_nothrow_move_assignable_v<T> && std::is_trivially_move_assignable_v<T> &&
     std::is_nothrow_destructible_v<T> && std::is_trivially_destructible_v<T>);

template <class KeyType, class ComparatorType>
class ComparatorHelper {
private:
    static constexpr bool kUseStdThreeWayComparison =
        std::is_same_v<ComparatorType, std::less<>> ||
        std::is_same_v<ComparatorType, std::greater<>> ||
        std::is_same_v<ComparatorType, std::less_equal<>> ||
        std::is_same_v<ComparatorType, std::greater_equal<>> ||
        std::is_same_v<ComparatorType, std::less<KeyType>> ||
        std::is_same_v<ComparatorType, std::greater<KeyType>> ||
        std::is_same_v<ComparatorType, std::less_equal<KeyType>> ||
        std::is_same_v<ComparatorType, std::greater_equal<KeyType>>;

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
            }
            if (ComparatorType{}(rhs, lhs)) {
                return std::weak_ordering::greater;
            }
            return std::weak_ordering::equivalent;
        }
    }
};

template <class Comparator>
concept StatelessComparator =
    std::is_trivially_default_constructible_v<Comparator> &&
    std::is_nothrow_default_constructible_v<Comparator> && sizeof(Comparator) <= sizeof(char);

static_assert(StatelessComparator<std::less<std::string>>);
static_assert(StatelessComparator<std::greater<int>>);

namespace rbtree {

export template <class KeyType,
                 StatelessComparator ComparatorType = std::less<>,
                 bool UseByValueWherePossible       = kUseByValue<KeyType>>
class RBTree;

export enum class TestStatus : std::uint32_t {
    kOk = 0,
    kRootIsNotBlack,
    kInvalidOrNotBlackParentOfRedNode,
    kRedNodeHasExactlyOneNilChild,
    kNodeHasInvalidColor,
    kKeyOfLeftSonGEThanKeyOfNode,
    kLeftSonOfRedNodeIsNotBlack,
    kMaxKeyInLeftSubtreeGEThanKeyOfNode,
    kKeyOfNodeGEThanKeyOfRightSon,
    kRightSonOfRedNodeIsNotBlack,
    kKeyOfNodeGEThanMinKeyInRightSubtree,
    kBlackHeightOfLeftSubtreeGTThanBlackHeightOfRightSubtree,
    kBlackHeightOfLeftSubtreeLSThanBlackHeightOfRightSubtree,
};

export template <class KeyType>
[[nodiscard]]
TestStatus IsRBTreeUnitTest(const RBTree<KeyType> &tree);

template <class KeyType, StatelessComparator ComparatorType, bool UseByValueWherePossible>
class RBTree
    : private RBTreeContainer<KeyType, RBTree<KeyType, ComparatorType, UseByValueWherePossible>>,
      private ComparatorHelper<KeyType, ComparatorType> {
    using Base            = RBTreeContainer<KeyType, RBTree>;
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
    using allocator_type         = typename Base::allocator_type;
    using Base::back;
    using Base::begin;
    using Base::cbegin;
    using Base::cend;
    using Base::clear;
    using Base::crbegin;
    using Base::crend;
    using Base::empty;
    using Base::end;
    using Base::front;
    using Base::max_size;
    using Base::rbegin;
    using Base::rend;
    using Base::size;

    constexpr RBTree() noexcept = default;
    constexpr RBTree(std::initializer_list<KeyType> list) : RBTree(list.begin(), list.end()) {}
    template <std::input_iterator Iter, std::sentinel_for<Iter> SentinelIter>
    constexpr RBTree(Iter begin_iter, SentinelIter end_iter) {
        for (; begin_iter != end_iter; ++begin_iter) {
            insert(*begin_iter);  // TODO: insert_hint
        }
    }

    constexpr void swap(RBTree &other) noexcept {
        Base::Swap(other);
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
        const node_type *last_left_turn = nullptr;
        for (auto *current_node = this->Root(); current_node != nullptr;) {
            const auto compare_result = CompareThreeWay(key, current_node->Key());
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
        for (auto *current_node = this->Root(); current_node != nullptr;) {
            const auto compare_result = CompareThreeWay(key, current_node->Key());
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

    friend TestStatus IsRBTreeUnitTest<KeyType>(const RBTree &tree);

private:
    [[nodiscard]] constexpr const node_type *Root() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return Base::Root();
    }

    // clang-format off
    template <bool OverwriteIfExists, class U>
    [[nodiscard]]
    constexpr std::conditional_t<OverwriteIfExists, iterator, InsertResult> insert_impl(U &&key) ATTRIBUTE_LIFETIME_BOUND {
        // clang-format on
        auto *previous_node     = Base::PreRootNodePtr();
        node_type *current_node = Base::Root();

        bool last_cmp_res_was_left = false;
        while (current_node != nullptr) {
            const auto compare_result = CompareThreeWay(key, current_node->Key());
            previous_node             = current_node;
            last_cmp_res_was_left     = compare_result < 0;
            if (compare_result < 0) {
                current_node = current_node->Left();
            } else if (compare_result > 0) {
                current_node = current_node->Right();
            } else {
                if constexpr (OverwriteIfExists) {
                    current_node->SetKey(std::forward<U>(key));
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

template <class KeyType>
struct [[nodiscard]] TestImplResult {
    TestStatus status{};
    std::int32_t height{};
    KeyType min_key{};
    KeyType max_key{};
};

template <class KeyType>
[[nodiscard]]
TestImplResult<KeyType> IsRBTreeUnitTestImpl(const typename RBTree<KeyType>::node_type &node) {
    std::int32_t height_l       = 0;
    std::int32_t height_r       = 0;
    KeyType min_l               = node.Key();
    KeyType max_r               = node.Key();
    const auto *left_child_ptr  = node.Left();
    const auto *right_child_ptr = node.Right();

    switch (node.Color()) {
        case NodeColor::kRed: {
            if (auto *parent = node.Parent();
                parent == nullptr || parent->Color() != NodeColor::kBlack) {
                return {
                    .status  = TestStatus::kInvalidOrNotBlackParentOfRedNode,
                    .height  = {},
                    .min_key = {},
                    .max_key = {},
                };
            }
            const bool only_one_nil = (left_child_ptr == nullptr && right_child_ptr != nullptr) ||
                                      (left_child_ptr != nullptr && right_child_ptr == nullptr);
            if (only_one_nil) {
                return {
                    .status  = TestStatus::kRedNodeHasExactlyOneNilChild,
                    .height  = {},
                    .min_key = {},
                    .max_key = {},
                };
            }
        } break;
        case NodeColor::kBlack:
            break;
        default:
            return {
                .status  = TestStatus::kNodeHasInvalidColor,
                .height  = {},
                .min_key = {},
                .max_key = {},
            };
    }

    if (left_child_ptr != nullptr) {
        const auto &left_child = *left_child_ptr;
        if (left_child.Key() >= node.Key()) {
            return {
                .status  = TestStatus::kKeyOfLeftSonGEThanKeyOfNode,
                .height  = {},
                .min_key = {},
                .max_key = {},
            };
        }
        if (node.Color() == NodeColor::kRed && left_child.Color() != NodeColor::kBlack) {
            return {
                .status  = TestStatus::kLeftSonOfRedNodeIsNotBlack,
                .height  = {},
                .min_key = {},
                .max_key = {},
            };
        }

        const TestImplResult<KeyType> left_subtree_info = IsRBTreeUnitTestImpl<KeyType>(left_child);
        RBTREE_ASSERT_INVARIANT(left_subtree_info.min_key <= left_subtree_info.max_key);
        if (left_subtree_info.status != TestStatus::kOk) {
            return left_subtree_info;
        }
        if (left_subtree_info.max_key >= node.Key()) {
            return {
                .status  = TestStatus::kMaxKeyInLeftSubtreeGEThanKeyOfNode,
                .height  = {},
                .min_key = {},
                .max_key = {},
            };
        }

        min_l    = std::min(min_l, left_subtree_info.min_key);
        height_l = left_subtree_info.height;
    }

    if (right_child_ptr != nullptr) {
        const auto &right_child = *right_child_ptr;
        if (node.Key() >= right_child.Key()) {
            return {
                .status  = TestStatus::kKeyOfNodeGEThanKeyOfRightSon,
                .height  = {},
                .min_key = {},
                .max_key = {},
            };
        }
        if (node.Color() == NodeColor::kRed && right_child.Color() != NodeColor::kBlack) {
            return {
                .status  = TestStatus::kRightSonOfRedNodeIsNotBlack,
                .height  = {},
                .min_key = {},
                .max_key = {},
            };
        }

        const TestImplResult<KeyType> right_subtree_info =
            IsRBTreeUnitTestImpl<KeyType>(right_child);
        RBTREE_ASSERT_INVARIANT(right_subtree_info.min_key <= right_subtree_info.max_key);
        if (right_subtree_info.status != TestStatus::kOk) {
            return right_subtree_info;
        }
        if (node.Key() >= right_subtree_info.min_key) {
            return {
                .status  = TestStatus::kKeyOfNodeGEThanMinKeyInRightSubtree,
                .height  = {},
                .min_key = {},
                .max_key = {},
            };
        }

        max_r    = std::max(max_r, right_subtree_info.max_key);
        height_r = right_subtree_info.height;
    }

    RBTREE_ASSERT_INVARIANT(min_l <= max_r);
    if (height_l < height_r) {
        return {
            .status  = TestStatus::kBlackHeightOfLeftSubtreeLSThanBlackHeightOfRightSubtree,
            .height  = {},
            .min_key = {},
            .max_key = {},
        };
    }
    if (height_l > height_r) {
        return {
            .status  = TestStatus::kBlackHeightOfLeftSubtreeGTThanBlackHeightOfRightSubtree,
            .height  = {},
            .min_key = {},
            .max_key = {},
        };
    }

    return {
        .status  = TestStatus::kOk,
        .height  = height_l + int32_t{node.Color() == NodeColor::kBlack},
        .min_key = min_l,
        .max_key = max_r,
    };
}

template <class KeyType>
TestStatus IsRBTreeUnitTest(const RBTree<KeyType> &tree) {
    const typename RBTree<KeyType>::node_type *root_ptr = tree.Root();
    if (root_ptr == nullptr) {
        return TestStatus::kOk;
    }
    const typename RBTree<KeyType>::node_type &root = *root_ptr;
    if (root.Color() != NodeColor::kBlack) {
        return TestStatus::kRootIsNotBlack;
    }

    const auto [status, h, min, max] = IsRBTreeUnitTestImpl<KeyType>(root);
    if (status == TestStatus::kOk) {
        RBTREE_ASSERT_INVARIANT(h > 0);
        if (root.Left()) {
            RBTREE_ASSERT_INVARIANT(min < root.Key());
        } else {
            RBTREE_ASSERT_INVARIANT(min == root.Key());
        }
        if (root.Right()) {
            RBTREE_ASSERT_INVARIANT(root.Key() < max);
        } else {
            RBTREE_ASSERT_INVARIANT(root.Key() == max);
        }
    } else {
        RBTREE_ASSERT_INVARIANT(min == max && max == KeyType{});
    }

    return status;
}

}  // namespace rbtree
