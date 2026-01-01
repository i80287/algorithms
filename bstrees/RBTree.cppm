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
#include <ranges>
#include <source_location>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#ifdef RBTREE_DEBUG
#include <chrono>
#include <string_view>
#endif

#include "../misc/config_macros.hpp"

export module rbtree;

namespace rbtree {
template <class KeyType>
concept RBTreeKeyTypeConstraints =
    std::same_as<KeyType,
                 std::remove_cvref_t<KeyType>>  // Type can't be const/volatile nor reference
    && !std::is_array_v<KeyType>;               // Type can't be an array

template <class ComparatorType, class Type>
concept ComparatorForType = std::same_as<ComparatorType, std::remove_cvref_t<ComparatorType>> &&
                            std::predicate<ComparatorType, const Type&, const Type&>;

export template <RBTreeKeyTypeConstraints KeyType, class ComparatorType = std::less<>>
    requires ComparatorForType<ComparatorType, KeyType>
class RBTree;

export enum class TestStatus : std::uint8_t {
    kOk = 1,
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

export template <class KeyType, class ComparatorType>
[[nodiscard]] TestStatus RBTreeInvariantsUnitTest(const RBTree<KeyType, ComparatorType>& tree);
}  // namespace rbtree

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

#define RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS ATTRIBUTE_NONNULL_ALL_ARGS
#define RBTREE_ATTRIBUTE_NONNULL(...)     ATTRIBUTE_NONNULL(__VA_ARGS__)

class NodeBase {
    friend class RBTreeContainerImpl;

public:
    NodeBase(const NodeBase&) = delete;

    NodeBase& operator=(const NodeBase&) = delete;

protected:
    constexpr NodeBase() noexcept
        : left_{nullptr}, right_{nullptr}, parent_{nullptr}, color_{NodeColor::kRed} {}

    constexpr ~NodeBase() = default;

    constexpr NodeBase(NodeBase&& other) noexcept
        : left_{std::exchange(other.left_, nullptr)}
        , right_{std::exchange(other.right_, nullptr)}
        , parent_{std::exchange(other.parent_, nullptr)}
        , color_{std::exchange(other.color_, NodeColor{})} {}

    constexpr NodeBase& operator=(NodeBase&& other) noexcept ATTRIBUTE_LIFETIME_BOUND {
        left_ = std::exchange(other.left_, nullptr);
        right_ = std::exchange(other.right_, nullptr);
        parent_ = std::exchange(other.parent_, nullptr);
        color_ = std::exchange(other.color_, NodeColor{});
        return *this;
    }

    constexpr void Swap(NodeBase& other) noexcept {
        std::swap(left_, other.left_);
        std::swap(right_, other.right_);
        std::swap(parent_, other.parent_);
        std::swap(color_, other.color_);
    }

public:
    [[nodiscard]]
    ATTRIBUTE_RETURNS_NONNULL constexpr const NodeBase* LeftMostNode() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return LeftMostNodeOf(this);
    }

    [[nodiscard]]
    ATTRIBUTE_RETURNS_NONNULL constexpr NodeBase* LeftMostNode() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return LeftMostNodeOf(this);
    }

    [[nodiscard]]
    ATTRIBUTE_RETURNS_NONNULL constexpr const NodeBase* RightMostNode() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return RightMostNodeOf(this);
    }

    [[nodiscard]]
    ATTRIBUTE_RETURNS_NONNULL constexpr NodeBase* RightMostNode() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return RightMostNodeOf(this);
    }

    [[nodiscard]]
    ATTRIBUTE_RETURNS_NONNULL constexpr const NodeBase* FarthestParent() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return FarthestParentNodeOf(this);
    }

    [[nodiscard]]
    ATTRIBUTE_RETURNS_NONNULL constexpr NodeBase* FarthestParent() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return FarthestParentNodeOf(this);
    }

    [[nodiscard]] constexpr const NodeBase* OtherSibling(const NodeBase* child) const noexcept {
        RBTREE_ASSERT_INVARIANT(IsChildNode(child));
        return left_ == child ? right_ : left_;
    }

    [[nodiscard]] constexpr NodeBase* OtherSibling(const NodeBase* child) noexcept {
        RBTREE_ASSERT_INVARIANT(IsChildNode(child));
        return left_ == child ? right_ : left_;
    }

    [[nodiscard]] constexpr NodeBase* Parent() const noexcept {
        return parent_;
    }

    [[nodiscard]] constexpr NodeBase*& ParentMutableReference() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return parent_;
    }

    constexpr void SetParent(NodeBase* new_parent) noexcept {
        parent_ = new_parent;
    }

    [[nodiscard]] constexpr NodeBase* Left() const noexcept {
        return left_;
    }

    constexpr void SetLeft(NodeBase* new_left) noexcept {
        left_ = new_left;
    }

    [[nodiscard]] constexpr NodeBase* Right() const noexcept {
        return right_;
    }

    constexpr void SetRight(NodeBase* new_right) noexcept {
        right_ = new_right;
    }

    [[nodiscard]] constexpr NodeColor Color() const noexcept {
        return color_;
    }

    constexpr void SetColor(const NodeColor new_color) noexcept {
        color_ = new_color;
    }

private:
    // clang-format off
    template <class NodeType>
    [[nodiscard]]
    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_RETURNS_NONNULL
    ATTRIBUTE_PURE
    ATTRIBUTE_ACCESS(read_only, 1)
    static constexpr NodeType* LeftMostNodeOf(NodeType* node ATTRIBUTE_LIFETIME_BOUND) noexcept {
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
    static constexpr NodeType* RightMostNodeOf(NodeType* node ATTRIBUTE_LIFETIME_BOUND) noexcept {
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
    static constexpr NodeType *FarthestParentNodeOf(NodeType* node ATTRIBUTE_LIFETIME_BOUND) noexcept {
        // clang-format on
        while (node->Parent() != nullptr) {
            node = node->Parent();
        }
        return node;
    }

    [[nodiscard]] constexpr bool IsChildNode(const NodeBase* const node) const noexcept {
        return (node == left_) ^ (node == right_);
    }

    NodeBase* left_;
    NodeBase* right_;
    NodeBase* parent_;
    NodeColor color_;
};

template <class T>
class Node : public NodeBase {
    using Base = NodeBase;

protected:
    using KeyType = T;

public:
    template <class U>
        requires std::constructible_from<KeyType, U&&>
    explicit constexpr Node(U&& key) noexcept(std::is_nothrow_constructible_v<KeyType, U&&>)
        : key_(std::forward<U>(key)) {}

    [[nodiscard]] constexpr const Node* Left() const noexcept {
        return static_cast<const Node*>(Base::Left());
    }

    [[nodiscard]] constexpr Node* Left() noexcept {
        return static_cast<Node*>(Base::Left());
    }

    [[nodiscard]] constexpr const Node* Right() const noexcept {
        return static_cast<const Node*>(Base::Right());
    }

    [[nodiscard]] constexpr Node* Right() noexcept {
        return static_cast<Node*>(Base::Right());
    }

    [[nodiscard]] constexpr const Node* Parent() const noexcept {
        return static_cast<const Node*>(Base::Parent());
    }

    [[nodiscard]] constexpr Node* Parent() noexcept {
        return static_cast<Node*>(Base::Parent());
    }

    template <class AssignedKeyType>
        requires(std::assignable_from<KeyType&, AssignedKeyType>)
    constexpr void SetKey(AssignedKeyType&& new_key) noexcept(
        std::is_nothrow_assignable_v<KeyType, AssignedKeyType&&>) {
        key_ = std::forward<AssignedKeyType>(new_key);
    }

    [[nodiscard]] constexpr const KeyType& Key() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return key_;
    }

    [[nodiscard]] constexpr KeyType& Key() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return key_;
    }

    constexpr void Swap(Node& other) noexcept(std::is_nothrow_swappable_v<KeyType>)
        requires std::swappable<KeyType>
    {
        std::ranges::swap(key_, other.key_);
        Base::Swap(other);
    }

private:
    KeyType key_;
};

class RBTreeContainerImpl {
protected:
    constexpr RBTreeContainerImpl() noexcept : fake_root_{} {
        SetBeginUnchecked(GetEndUnchecked());
        AssertTreeInvariants();
    }

public:
    RBTreeContainerImpl(const RBTreeContainerImpl& other) = delete;

    RBTreeContainerImpl& operator=(const RBTreeContainerImpl& other) = delete;

protected:
    constexpr RBTreeContainerImpl(RBTreeContainerImpl&& other) noexcept
        : fake_root_(std::move(other.fake_root_)) {
        this->SynchronizeInvariantsOnMove();
    }

    RBTreeContainerImpl& operator=(RBTreeContainerImpl&& other) noexcept ATTRIBUTE_LIFETIME_BOUND {
        this->Swap(other);
        this->AssertTreeInvariants();
        return *this;
    }

    constexpr void Swap(RBTreeContainerImpl& other) noexcept {
        fake_root_.Swap(other.fake_root_);
        this->SynchronizeInvariantsOnMove();
        other.SynchronizeInvariantsOnMove();
    }

    struct CompleteTreeRawData {
        NodeBase* root_node;
        NodeBase* leftmost_node;
        NodeBase* rightmost_node;
        std::size_t tree_size;
    };

    explicit constexpr RBTreeContainerImpl(const CompleteTreeRawData& data) noexcept
        : fake_root_{} {
        this->SetRootUnchecked(data.root_node);
        this->SetBeginUnchecked(data.leftmost_node);
        this->SetLastUnchecked(data.rightmost_node);
        this->SetSize(data.tree_size);
        this->SynchronizeInvariantsOnMove();
    }

    constexpr ~RBTreeContainerImpl() = default;

    constexpr void SetBegin(NodeBase* new_begin) noexcept {
        this->AssertBeginInvariants();
        this->SetBeginUnchecked(new_begin);
    }

    [[nodiscard]] constexpr NodeBase* Begin() noexcept ATTRIBUTE_LIFETIME_BOUND {
        this->AssertBeginInvariants();
        return this->GetBeginUnchecked();
    }

    [[nodiscard]] constexpr const NodeBase* Begin() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        this->AssertBeginInvariants();
        return this->GetBeginUnchecked();
    }

    [[nodiscard]] constexpr NodeBase* PreRootNil() noexcept ATTRIBUTE_LIFETIME_BOUND {
        this->AssertPreRootInvariants();
        return this->GetPreRootNilUnchecked();
    }

    [[nodiscard]] constexpr const NodeBase* PreRootNil() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        this->AssertPreRootInvariants();
        return this->GetPreRootNilUnchecked();
    }

    [[nodiscard]] constexpr NodeBase* Root() noexcept ATTRIBUTE_LIFETIME_BOUND {
        this->AssertRootInvariants();
        return this->GetRootUnchecked();
    }

    [[nodiscard]] constexpr const NodeBase* Root() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        this->AssertRootInvariants();
        return this->GetRootUnchecked();
    }

    [[nodiscard]] constexpr NodeBase* End() noexcept ATTRIBUTE_LIFETIME_BOUND {
        this->AssertEndInvariants();
        return this->GetEndUnchecked();
    }

    [[nodiscard]] constexpr const NodeBase* End() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        this->AssertEndInvariants();
        return this->GetEndUnchecked();
    }

    constexpr void SetLast(NodeBase* new_last) noexcept {
        this->AssertLastInvariants();
        this->SetLastUnchecked(new_last);
    }

    [[nodiscard]] constexpr const NodeBase* Last() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        this->AssertLastInvariants();
        return this->GetLastUnchecked();
    }

    [[nodiscard]] constexpr NodeBase* Last() noexcept ATTRIBUTE_LIFETIME_BOUND {
        this->AssertLastInvariants();
        return this->GetLastUnchecked();
    }

    [[nodiscard]] ATTRIBUTE_PURE constexpr std::size_t Size() const noexcept {
        return static_cast<std::size_t>(fake_root_.Color());
    }

    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_write, 2)
    ATTRIBUTE_ACCESS(read_write, 4)
    void InsertNode(NodeBase* const node_parent,
                    const bool is_left,
                    NodeBase* const node) noexcept {
        node->SetParent(node_parent);
        RBTREE_ASSERT_INVARIANT(node->Left() == nullptr);
        RBTREE_ASSERT_INVARIANT(node->Right() == nullptr);
        RBTREE_ASSERT_INVARIANT(node->Color() == NodeColor::kRed);
        RBTREE_ASSERT_INVARIANT(Size() >= 1 || node_parent == PreRootNil());
        RBTREE_ASSERT_INVARIANT(Size() >= 1 || !is_left);
        if (is_left) {
            this->MakeLeftOnInsertion(node_parent, node);
        } else {
            this->MakeRightOnInsertion(node_parent, node);
        }
        this->IncrementSize();
        this->RebalanceAfterInsertion(node);
    }

    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_only, 2)
    constexpr void ExtractNode(const NodeBase* const node) noexcept {
        RBTREE_ASSERT_INVARIANT(Size() >= 1);
        RBTREE_ASSERT_INVARIANT(Begin() != End());
        RBTREE_ASSERT_INVARIANT(node != PreRootNil());
        NodeBase* const node_parent = node->Parent();
        RBTREE_ASSERT_INVARIANT(node_parent != nullptr);
        NodeBase* const node_left_child = node->Left();
        NodeBase* const node_right_child = node->Right();
        NodeBase* propagated_node{};
        NodeBase* propagated_node_parent{};
        NodeColor color = node->Color();
        RBTREE_ASSERT_INVARIANT(color == NodeColor::kRed || color == NodeColor::kBlack);
        if (node_left_child == nullptr || node_right_child == nullptr) {
            propagated_node = node_left_child == nullptr ? node_right_child : node_left_child;
            propagated_node_parent = node_parent;
            RBTREE_ASSERT_INVARIANT(propagated_node == nullptr ||
                                    propagated_node->Color() == NodeColor::kRed);
            if (node_left_child == nullptr && node == GetBeginUnchecked()) {
                this->SetBeginUnchecked(
                    node_right_child != nullptr ? node_right_child->LeftMostNode() : node_parent);
            }
            if (node_right_child == nullptr && node == GetLastUnchecked()) {
                this->SetLastUnchecked(node_left_child != nullptr ? node_left_child->RightMostNode()
                                                                  : node_parent);
            }
            this->Transplant(node, propagated_node);
        } else {
            RBTREE_ASSERT_INVARIANT(node != Begin());
            RBTREE_ASSERT_INVARIANT(node != Last());
            NodeBase* const node_successor = node_right_child->LeftMostNode();
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
                this->Transplant(node_successor, propagated_node);
                node_successor->SetRight(node_right_child);
                node_right_child->SetParent(node_successor);
                node_right_child->SetParent(node_successor);
            } else {
                propagated_node_parent = node_successor;
            }
            this->Transplant(node, node_successor);
            node_successor->SetLeft(node_left_child);
            node_left_child->SetParent(node_successor);
            node_successor->SetColor(node->Color());
        }

        this->DecrementSize();
        if (color != NodeColor::kRed) {
            RBTREE_ASSERT_INVARIANT(color == NodeColor::kBlack);
            this->RebalanceAfterExtraction(propagated_node, propagated_node_parent);
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
        this->SetSize(static_cast<std::size_t>(fake_root_.Color()) + 1);
    }

    constexpr void DecrementSize() noexcept {
        RBTREE_ASSERT_INVARIANT(Size() > 0);
        this->SetSize(static_cast<std::size_t>(fake_root_.Color()) - 1);
    }

    constexpr void SynchronizeInvariantsOnMove() noexcept {
        if (auto* root = this->GetRootUnchecked(); root != nullptr) {
            root->SetParent(this->GetPreRootNilUnchecked());
            RBTREE_ASSERT_INVARIANT(this->Size() >= 1);
        } else {
            this->SetBeginUnchecked(this->GetEndUnchecked());
            RBTREE_ASSERT_INVARIANT(this->Size() == 0);
        }

        this->AssertTreeInvariants();
    }

    constexpr void AssertTreeInvariants() const noexcept {
        this->AssertRootInvariants();
        this->AssertBeginInvariants();
        this->AssertEndInvariants();
        this->AssertPreRootInvariants();
    }

    constexpr void AssertRootInvariants() const noexcept {
        RBTREE_ASSERT_INVARIANT(this->GetRootUnchecked() == nullptr ||
                                this->GetRootUnchecked()->Color() == NodeColor::kBlack);
        RBTREE_ASSERT_INVARIANT(this->GetRootUnchecked() == nullptr ||
                                this->GetRootUnchecked()->Parent() == this->PreRootNil());
        RBTREE_ASSERT_INVARIANT((this->GetRootUnchecked() == nullptr && this->Size() == 0) ^
                                (this->GetRootUnchecked() != nullptr && this->Size() >= 1));
    }

    [[nodiscard]] constexpr NodeBase* GetRootUnchecked() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.Parent();
    }

    [[nodiscard]] constexpr const NodeBase* GetRootUnchecked() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.Parent();
    }

    [[nodiscard]] constexpr NodeBase*& GetRootMutableReferenceUnchecked() noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.ParentMutableReference();
    }

    constexpr void SetRootUnchecked(NodeBase* new_root) noexcept {
        return fake_root_.SetParent(new_root);
    }

    constexpr void AssertPreRootInvariants() const noexcept {
        RBTREE_ASSERT_INVARIANT(this->GetRootUnchecked() == nullptr ||
                                this->GetRootUnchecked()->Parent() ==
                                    this->GetPreRootNilUnchecked());
        RBTREE_ASSERT_INVARIANT((this->GetRootUnchecked() == nullptr && this->Size() == 0) ^
                                (this->GetRootUnchecked() != nullptr && this->Size() >= 1));
    }

    [[nodiscard]] constexpr NodeBase* GetPreRootNilUnchecked() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return std::addressof(fake_root_);
    }

    [[nodiscard]] constexpr const NodeBase* GetPreRootNilUnchecked() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return std::addressof(fake_root_);
    }

    constexpr void AssertBeginInvariants() const noexcept {
        RBTREE_ASSERT_INVARIANT(this->GetBeginUnchecked() != nullptr);
#ifdef RBTREE_DEBUG
        const auto invariants_if_not_empty =
            this->Size() >= 1 && this->GetBeginUnchecked() != this->GetEndUnchecked();
        const auto invariants_if_empty =
            this->Size() == 0 && this->GetBeginUnchecked() == this->GetEndUnchecked();
#endif
        RBTREE_ASSERT_INVARIANT(invariants_if_not_empty ^ invariants_if_empty);
        RBTREE_ASSERT_INVARIANT(this->GetBeginUnchecked() == this->GetEndUnchecked() ||
                                this->GetBeginUnchecked()->Left() == nullptr);
    }

    [[nodiscard]] constexpr NodeBase* GetBeginUnchecked() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.Right();
    }

    [[nodiscard]] constexpr const NodeBase* GetBeginUnchecked() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.Right();
    }

    constexpr void SetBeginUnchecked(NodeBase* new_begin) noexcept {
        return fake_root_.SetRight(new_begin);
    }

    constexpr void AssertEndInvariants() const noexcept {
        RBTREE_ASSERT_INVARIANT(
            (this->Size() == 0 && this->GetBeginUnchecked() == this->GetEndUnchecked()) ^
            (this->Size() >= 1 && this->GetBeginUnchecked() != this->GetEndUnchecked()));
    }

    [[nodiscard]] constexpr NodeBase* GetEndUnchecked() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return this->GetPreRootNilUnchecked();
    }

    [[nodiscard]] constexpr const NodeBase* GetEndUnchecked() const noexcept
        ATTRIBUTE_LIFETIME_BOUND {
        return this->GetPreRootNilUnchecked();
    }

    constexpr void AssertLastInvariants() const noexcept {
        RBTREE_ASSERT_INVARIANT(this->Size() >= 1);
        RBTREE_ASSERT_INVARIANT((this->Size() >= 2) ^
                                (this->GetBeginUnchecked() == this->GetLastUnchecked() &&
                                 this->GetLastUnchecked() == this->GetRootUnchecked()));
        RBTREE_ASSERT_INVARIANT(this->GetLastUnchecked() != nullptr);
        RBTREE_ASSERT_INVARIANT(this->GetLastUnchecked() != this->GetPreRootNilUnchecked());
        RBTREE_ASSERT_INVARIANT(this->GetLastUnchecked()->Right() == nullptr);
    }

    [[nodiscard]] constexpr NodeBase* GetLastUnchecked() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.Left();
    }

    [[nodiscard]]
    constexpr const NodeBase* GetLastUnchecked() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return fake_root_.Left();
    }

    constexpr void SetLastUnchecked(NodeBase* new_last) noexcept {
        return fake_root_.SetLeft(new_last);
    }

    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_write, 1)
    ATTRIBUTE_ACCESS(read_write, 2)
    static constexpr void LeftRotate(NodeBase* const node, NodeBase*& root) noexcept {
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
        RBTREE_ASSERT_INVARIANT(node != root);
        RBTREE_ASSERT_INVARIANT(node != root->Parent());
        NodeBase* const parent = node->Parent();
        RBTREE_ASSERT_INVARIANT(parent != nullptr);
        RBTREE_ASSERT_INVARIANT(parent->Right() == node);
        NodeBase* const parent_parent = parent->Parent();

        parent->SetRight(node->Left());
        if (auto* node_left = node->Left(); node_left != nullptr) {
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
    static constexpr void RightRotate(NodeBase* const node, NodeBase*& root) noexcept {
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
        NodeBase* const parent = node->Parent();
        RBTREE_ASSERT_INVARIANT(parent != nullptr);
        RBTREE_ASSERT_INVARIANT(parent->Left() == node);
        NodeBase* const parent_parent = parent->Parent();

        parent->SetLeft(node->Right());
        if (auto* node_right = node->Right(); node_right != nullptr) {
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

    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_write, 2)
    ATTRIBUTE_ACCESS(read_only, 3)
    constexpr void MakeLeftOnInsertion(NodeBase* const node_parent, NodeBase* const node) noexcept {
        RBTREE_ASSERT_INVARIANT(node_parent != PreRootNil() && node_parent->Left() == nullptr);
        RBTREE_ASSERT_INVARIANT(node->Parent() == node_parent);
        RBTREE_ASSERT_INVARIANT(node->Left() == nullptr);
        RBTREE_ASSERT_INVARIANT(node->Right() == nullptr);
        if (node_parent == Begin()) {
            this->SetBegin(node);
        }
        node_parent->SetLeft(node);
        RBTREE_ASSERT_INVARIANT(this->GetBeginUnchecked() != nullptr);
        RBTREE_ASSERT_INVARIANT(this->GetLastUnchecked() != nullptr);
    }

    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_write, 2)
    ATTRIBUTE_ACCESS(read_only, 3)
    constexpr void MakeRightOnInsertion(NodeBase* const node_parent,
                                        NodeBase* const node) noexcept {
        RBTREE_ASSERT_INVARIANT(node_parent == PreRootNil() || node_parent->Right() == nullptr);
        RBTREE_ASSERT_INVARIANT(node->Parent() == node_parent);
        RBTREE_ASSERT_INVARIANT(node->Left() == nullptr);
        RBTREE_ASSERT_INVARIANT(node->Right() == nullptr);
        const bool is_node_root = node_parent == PreRootNil();
        if (is_node_root) [[unlikely]] {
            this->SetRootUnchecked(node);
            // SetBeginUnchecked(node); will be set during `node_parent->SetRight(node)`
            this->SetLastUnchecked(node);
        } else if (node_parent == Last()) {
            this->SetLast(node);
        }
        node_parent->SetRight(node);
        RBTREE_ASSERT_INVARIANT(this->GetBeginUnchecked() != nullptr);
        RBTREE_ASSERT_INVARIANT(this->GetLastUnchecked() != nullptr);
        RBTREE_ASSERT_INVARIANT(!is_node_root || this->GetBeginUnchecked() == node);
    }

    // clang-format on

    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_write, 2)
    constexpr void RebalanceAfterInsertion(NodeBase* const new_inserted_node) noexcept {
        RBTREE_ASSERT_INVARIANT(new_inserted_node->Color() == NodeColor::kRed);
        RBTREE_ASSERT_INVARIANT(this->Size() >= 1);
        NodeBase* current_node = new_inserted_node;
        NodeBase* current_node_parent = nullptr;
        NodeBase*& root = this->GetRootMutableReferenceUnchecked();
        while ((current_node_parent = current_node->Parent()) != root &&
               current_node_parent->Color() == NodeColor::kRed) {
            RBTREE_ASSERT_INVARIANT(current_node_parent != this->GetPreRootNilUnchecked());
            RBTREE_ASSERT_INVARIANT(current_node_parent->OtherSibling(current_node) == nullptr ||
                                    current_node_parent->OtherSibling(current_node)->Color() ==
                                        NodeColor::kBlack);
            NodeBase* const current_node_parent_parent = current_node_parent->Parent();
            RBTREE_ASSERT_INVARIANT(current_node_parent_parent != nullptr);
            RBTREE_ASSERT_INVARIANT(current_node_parent_parent->Color() == NodeColor::kBlack);

            NodeBase* current_node_parent_sibling =
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
                RBTREE_ASSERT_INVARIANT(current_node_parent_sibling !=
                                        this->GetPreRootNilUnchecked());
                RBTREE_ASSERT_INVARIANT(current_node_parent_sibling != root);
                RBTREE_ASSERT_INVARIANT(current_node_parent_sibling->Left() == nullptr ||
                                        current_node_parent_sibling->Left()->Color() ==
                                            NodeColor::kBlack);
                RBTREE_ASSERT_INVARIANT(current_node_parent_sibling->Left() !=
                                        this->GetPreRootNilUnchecked());
                RBTREE_ASSERT_INVARIANT(current_node_parent_sibling->Right() == nullptr ||
                                        current_node_parent_sibling->Right()->Color() ==
                                            NodeColor::kBlack);
                RBTREE_ASSERT_INVARIANT(current_node_parent_sibling->Right() !=
                                        this->GetPreRootNilUnchecked());
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
    constexpr void Transplant(const NodeBase* const which_node,
                              NodeBase* const with_node) noexcept {
        RBTREE_ASSERT_INVARIANT(which_node != this->PreRootNil());
        RBTREE_ASSERT_INVARIANT(with_node != this->PreRootNil());
        RBTREE_ASSERT_INVARIANT(with_node != this->Root());
        NodeBase* const which_node_parent = which_node->Parent();
        RBTREE_ASSERT_INVARIANT(which_node_parent != nullptr);
        RBTREE_ASSERT_INVARIANT(with_node == nullptr || with_node->Parent() != which_node_parent);
        if (which_node_parent == this->PreRootNil()) [[unlikely]] {
            this->SetRootUnchecked(with_node);
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
    ATTRIBUTE_ACCESS(read_write, 3)
    constexpr void RebalanceAfterExtraction(NodeBase* const node,
                                            NodeBase* const node_parent) noexcept {
        RBTREE_ASSERT_INVARIANT(node != this->GetPreRootNilUnchecked());
        NodeBase* current_node = node;
        NodeBase* current_node_parent = node_parent;
        NodeBase*& root = this->GetRootMutableReferenceUnchecked();
        RBTREE_ASSERT_INVARIANT(root != this->GetPreRootNilUnchecked());
        while (current_node != root &&
               (current_node == nullptr || current_node->Color() == NodeColor::kBlack)) {
            RBTREE_ASSERT_INVARIANT(root != nullptr);
            RBTREE_ASSERT_INVARIANT(current_node_parent != nullptr);
            RBTREE_ASSERT_INVARIANT(current_node_parent != this->PreRootNil());
            if (current_node == current_node_parent->Left()) {
                NodeBase* current_node_sibling = current_node_parent->Right();
                RBTREE_ASSERT_INVARIANT(current_node_sibling != nullptr);
                if (current_node_sibling->Color() == NodeColor::kRed) {
                    RBTREE_ASSERT_INVARIANT(current_node_sibling != this->PreRootNil());
                    RBTREE_ASSERT_INVARIANT(current_node_sibling != root);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Left() != this->PreRootNil());
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Left() != nullptr);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Left()->Color() ==
                                            NodeColor::kBlack);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Right() != this->PreRootNil());
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Right() != nullptr);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Right()->Color() ==
                                            NodeColor::kBlack);
                    current_node_sibling->SetColor(NodeColor::kBlack);
                    current_node_parent->SetColor(NodeColor::kRed);
#ifdef RBTREE_DEBUG
                    const NodeBase* const current_node_sibling_original_left_child =
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
                NodeBase* w = current_node_sibling;
                RBTREE_ASSERT_INVARIANT(w != nullptr);
                NodeBase* w_left = w->Left();
                NodeBase* w_right = w->Right();
                if ((w_left == nullptr || w_left->Color() == NodeColor::kBlack) &&
                    (w_right == nullptr || w_right->Color() == NodeColor::kBlack)) {
                    w->SetColor(NodeColor::kRed);
                    RBTREE_ASSERT_INVARIANT(current_node == nullptr ||
                                            current_node_parent == current_node->Parent());
                    current_node = current_node_parent;
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
                        w = w_left;
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
                NodeBase* current_node_sibling = current_node_parent->Left();
                RBTREE_ASSERT_INVARIANT(current_node_sibling != nullptr);
                if (current_node_sibling->Color() == NodeColor::kRed) {
                    RBTREE_ASSERT_INVARIANT(current_node_sibling != this->PreRootNil());
                    RBTREE_ASSERT_INVARIANT(current_node_sibling != root);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Left() != this->PreRootNil());
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Left() != nullptr);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Left()->Color() ==
                                            NodeColor::kBlack);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Right() != this->PreRootNil());
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Right() != nullptr);
                    RBTREE_ASSERT_INVARIANT(current_node_sibling->Right()->Color() ==
                                            NodeColor::kBlack);
                    current_node_sibling->SetColor(NodeColor::kBlack);
                    current_node_parent->SetColor(NodeColor::kRed);
#ifdef RBTREE_DEBUG
                    const NodeBase* const current_node_sibling_original_right_child =
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
                NodeBase* w = current_node_sibling;
                RBTREE_ASSERT_INVARIANT(w != nullptr);
                NodeBase* w_left = w->Left();
                NodeBase* w_right = w->Right();
                if ((w_left == nullptr || w_left->Color() == NodeColor::kBlack) &&
                    (w_right == nullptr || w_right->Color() == NodeColor::kBlack)) {
                    w->SetColor(NodeColor::kRed);
                    RBTREE_ASSERT_INVARIANT(current_node == nullptr ||
                                            current_node_parent == current_node->Parent());
                    current_node = current_node_parent;
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
                        w = w_right;
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
                    current_node = root;
                    break;
                }
            }
        }

        if (current_node != nullptr) {
            current_node->SetColor(NodeColor::kBlack);
        }
        RBTREE_ASSERT_INVARIANT(this->Size() == 0 || this->Root() != nullptr);
    }

    NodeBase fake_root_;
};

// clang-format off

#ifdef RBTREE_DEBUG

[[nodiscard]]
ATTRIBUTE_ACCESS(read_only, 1)
ATTRIBUTE_PURE
constexpr bool IsFakeNodeOrRoot(const NodeBase *node) noexcept {
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
ATTRIBUTE_RETURNS_NONNULL
constexpr NodeType *Increment(NodeType *node) noexcept {
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

    RBTREE_ASSERT_INVARIANT(node != nullptr);
    RBTREE_ASSERT_INVARIANT(parent != nullptr);
    RBTREE_ASSERT_INVARIANT(node == parent->Left() ||
                            (IsFakeNodeOrRoot(parent) && parent->Parent() == node) ||
                            (IsFakeNodeOrRoot(node) && node->Right() == parent));
    return node->Right() != parent ? parent : node;
}

template <class NodeType>
[[nodiscard]]
RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
ATTRIBUTE_ACCESS(read_only, 1)
ATTRIBUTE_PURE
ATTRIBUTE_RETURNS_NONNULL
constexpr NodeType *Decrement(NodeType *node) noexcept {
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

    RBTREE_ASSERT_INVARIANT(node != nullptr);
    RBTREE_ASSERT_INVARIANT(parent != nullptr);
    RBTREE_ASSERT_INVARIANT(node == parent->Right());
    return parent;
}

// clang-format on

template <std::size_t N>
ATTRIBUTE_ACCESS(write_only, 2)
void FillLengthErrorReport(const std::source_location& location,
                           std::array<char, N>& buffer) noexcept {
    const int written_bytes_count =
        std::snprintf(buffer.data(), buffer.size(), "Could not create node: size error at %s:%u:%s",
                      location.file_name(), location.line(), location.function_name());
    if (written_bytes_count <= 0) [[unlikely]] {
        constexpr std::array kFallbackReport = std::to_array(
            "Could not create error report on node creation error because of the size error");
        static_assert(std::size(kFallbackReport) < N);
        std::ranges::copy(kFallbackReport, buffer.begin());
    }
}

[[noreturn]]
ATTRIBUTE_COLD void ThrowLengthError(
    const std::source_location& location = std::source_location::current()) {
    constexpr std::size_t kErrorReportBufferSize = 1024;
    std::array<char, kErrorReportBufferSize> buffer{};
    FillLengthErrorReport(location, buffer);
    throw std::length_error{buffer.data()};
}

template <class KeyType, std::derived_from<NodeBase> NodeType = Node<KeyType>>
class RBTreeContainer : private RBTreeContainerImpl {
    using Base = RBTreeContainerImpl;

protected:
    template <bool IsConstIterator>
    class Iterator final {
        using IterNodeBaseType = std::conditional_t<IsConstIterator, const NodeBase, NodeBase>;
        using IterNodeType =
            std::conditional_t<IsConstIterator, const Node<KeyType>, Node<KeyType>>;
        using IterKeyType = std::conditional_t<IsConstIterator, const KeyType, KeyType>;
        using OtherIterator = Iterator<!IsConstIterator>;

        friend RBTreeContainer;
        friend OtherIterator;

        RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
        explicit constexpr Iterator(IterNodeType* const node_ptr ATTRIBUTE_LIFETIME_BOUND) noexcept
            : node_(node_ptr) {
            RBTREE_ASSERT_INVARIANT(node_ != nullptr);
        }

    public:
        using value_type = KeyType;
        using reference = IterKeyType&;
        using pointer = IterKeyType*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        constexpr Iterator() noexcept = default;

        // clang-format off
    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    /* implicit */ constexpr Iterator(const OtherIterator other) noexcept
        requires(IsConstIterator)
        : Iterator(other.node_) {
            // clang-format on
        }

        explicit Iterator(std::nullptr_t) = delete;

        [[nodiscard]] constexpr reference operator*() const noexcept {
            RBTREE_ASSERT_INVARIANT(node_ != nullptr);
            return node_->Key();
        }

        [[nodiscard]]
        ATTRIBUTE_RETURNS_NONNULL constexpr pointer operator->() const noexcept {
            RBTREE_ASSERT_INVARIANT(node_ != nullptr);
            return std::addressof(node_->Key());
        }

        [[nodiscard]] bool operator==(const Iterator&) const = default;

        constexpr Iterator& operator++() noexcept ATTRIBUTE_LIFETIME_BOUND {
            node_ = static_cast<IterNodeType*>(Increment<IterNodeBaseType>(node_));
            RBTREE_ASSERT_INVARIANT(node_ != nullptr);
            return *this;
        }

        [[nodiscard]] constexpr Iterator operator++(int) & noexcept {
            const Iterator copy(*this);
            ++*this;
            return copy;
        }

        constexpr Iterator& operator--() noexcept ATTRIBUTE_LIFETIME_BOUND {
            node_ = static_cast<IterNodeType*>(Decrement<IterNodeBaseType>(node_));
            RBTREE_ASSERT_INVARIANT(node_ != nullptr);
            return *this;
        }

        [[nodiscard]] constexpr Iterator operator--(int) & noexcept {
            const Iterator copy(*this);
            --*this;
            return copy;
        }

    private:
        IterNodeType* node_{};
    };

    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using const_iterator = Iterator</*IsConstIterator = */ true>;
    using iterator = const_iterator;
    using value_type = KeyType;
    using key_type = KeyType;
    using reference = value_type&;
    using const_reference = const value_type&;
    using allocator_type = std::allocator<NodeType>;
    using allocator_traits = std::allocator_traits<allocator_type>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using node_type = NodeType;

    static_assert(std::bidirectional_iterator<const_iterator>);
    static_assert(std::bidirectional_iterator<iterator>);

    static constexpr bool kIsNodeNoexceptDestructible = std::is_nothrow_destructible_v<NodeType>;

    RBTreeContainer() = default;

    constexpr RBTreeContainer(const RBTreeContainer& other) : Base(CloneTree(other)) {}

    constexpr RBTreeContainer& operator=(const RBTreeContainer& other) ATTRIBUTE_LIFETIME_BOUND {
        // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature)
        return *this = RBTreeContainer(other);  // NOLINT(misc-unconventional-assign-operator)
    }

    constexpr RBTreeContainer(RBTreeContainer&&) = default;

    constexpr RBTreeContainer& operator=(RBTreeContainer&&) = default;

    constexpr ~RBTreeContainer() noexcept(noexcept(DeleteTree(Root()))) {
        RBTreeContainer::DeleteTree(Root());
    }

    constexpr void swap(RBTreeContainer& other) noexcept {
        Base::Swap(other);
    }

    ATTRIBUTE_REINITIALIZES constexpr void clear() noexcept {
        RBTreeContainer{}.swap(*this);
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
        return const_iterator{static_cast<const NodeType *>(Base::End())};
    }
    [[nodiscard]] constexpr iterator begin() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return iterator{static_cast<NodeType *>(Base::Begin())};
    }
    [[nodiscard]] constexpr iterator end() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return iterator{static_cast<NodeType *>(Base::End())};
    }
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return this->begin();
    }
    [[nodiscard]] constexpr const_iterator cend() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return this->end();
    }
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return const_reverse_iterator{this->end()};
    }
    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return const_reverse_iterator{this->begin()};
    }
    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return reverse_iterator{this->end()};
    }
    [[nodiscard]] constexpr reverse_iterator rend() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return reverse_iterator{this->begin()};
    }
    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return this->rbegin();
    }
    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return this->rend();
    }
    [[nodiscard]] constexpr const_reference front() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return *this->begin();
    }
    [[nodiscard]] constexpr reference front() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return *this->begin();
    }
    [[nodiscard]] constexpr const_reference back() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return static_cast<const NodeType *>(Base::Last())->Key();
    }
    [[nodiscard]] constexpr reference back() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return static_cast<NodeType *>(Base::Last())->Key();
    }

    [[nodiscard]] ATTRIBUTE_CONST static constexpr size_type max_size() noexcept {
        constexpr size_type kMaxSSize = static_cast<size_type>(std::numeric_limits<difference_type>::max()) / sizeof(NodeType);
        constexpr size_type kMaxUSize = std::numeric_limits<size_type>::max() / sizeof(NodeType);
        constexpr size_type kMaxAllocSize = allocator_traits::max_size(GetAllocator());
        constexpr size_type kMaxSize = std::min({kMaxUSize, kMaxSSize, kMaxAllocSize});
        return kMaxSize;
    }

    [[nodiscard]] constexpr NodeType* Root() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return static_cast<NodeType*>(Base::Root());
    }
    [[nodiscard]] constexpr const NodeType* Root() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return static_cast<const NodeType*>(Base::Root());
    }
    [[nodiscard]] constexpr const NodeType* PreRootNodePtr() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return static_cast<const NodeType*>(Base::PreRootNil());
    }
    [[nodiscard]] constexpr NodeType* PreRootNodePtr() noexcept ATTRIBUTE_LIFETIME_BOUND {
        return static_cast<NodeType*>(Base::PreRootNil());
    }
    [[nodiscard]]
    ATTRIBUTE_PURE
    static constexpr iterator NodePtrToIterator(NodeType* node_ptr ATTRIBUTE_LIFETIME_BOUND) noexcept {
        return iterator{node_ptr};
    }
    [[nodiscard]]
    ATTRIBUTE_PURE
    static constexpr const_iterator NodePtrToConstIterator(const NodeType* node_ptr ATTRIBUTE_LIFETIME_BOUND) noexcept {
        return const_iterator{node_ptr};
    }

    template <class... Args>
    [[nodiscard]]
    RBTREE_ATTRIBUTE_NONNULL(2)
    ATTRIBUTE_ACCESS(read_write, 2)
    ATTRIBUTE_RETURNS_NONNULL
    constexpr NodeType *InsertNode(NodeType *const node_parent, const bool insert_left, Args &&...args) ATTRIBUTE_LIFETIME_BOUND {
        NodeType *const new_node = this->CreateNode(std::forward<Args>(args)...);
        Base::InsertNode(node_parent, insert_left, new_node);
        return new_node;
    }

    constexpr void ExtractAndDestroyNode(const const_iterator iter) noexcept(kIsNodeNoexceptDestructible) {
        auto *const node = const_cast<NodeType *>(iter.node_);
        RBTREE_ASSERT_INVARIANT(node != nullptr);
        Base::ExtractNode(node);
        DestroyNode(node);
    }

    // clang-format on

private:
    // clang-format off
    template <class... Args>
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_RETURNS_NONNULL
    constexpr NodeType *CreateNode(Args &&...args) {
        return RBTreeContainer::CheckSizeAndCreateNode(this->size(), std::forward<Args>(args)...);
    }

    template <class... Args>
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_RETURNS_NONNULL
    static constexpr NodeType *CheckSizeAndCreateNode(const size_type tree_size, Args &&...args)
        requires(std::constructible_from<NodeType, Args...>) {
        // clang-format on
        if (tree_size >= max_size()) [[unlikely]] {
            RBTREE_ASSERT_INVARIANT(tree_size == max_size());
            ThrowLengthError();
        }

        return RBTreeContainer::CreateNodeUnchecked(std::forward<Args>(args)...);
    }

    // clang-format off
    template <class... Args>
    [[nodiscard]]
    ATTRIBUTE_ALWAYS_INLINE
    ATTRIBUTE_RETURNS_NONNULL
    static constexpr NodeType *CreateNodeUnchecked(Args &&...args) {
        // clang-format on

        class NodeStorageProxy final {
        public:
            using pointer = NodeType*;

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
            constexpr NodeStorageProxy() : node_storage_{AllocateStorage()} {}

            NodeStorageProxy(const NodeStorageProxy&) = delete;

            NodeStorageProxy& operator=(const NodeStorageProxy&) = delete;

            NodeStorageProxy(NodeStorageProxy&& other) noexcept = delete;

            NodeStorageProxy& operator=(NodeStorageProxy&& other) noexcept = delete;

            ATTRIBUTE_ALWAYS_INLINE constexpr ~NodeStorageProxy() {
                if (node_storage_ != nullptr) [[unlikely]] {
                    allocator_type alloc = GetAllocator();
                    allocator_traits::deallocate(alloc, node_storage_, 1);
                }
            }

            // clang-format off
            [[nodiscard]]
            ATTRIBUTE_ALWAYS_INLINE
            ATTRIBUTE_RETURNS_NONNULL
            constexpr pointer ConstructNodeAndReleaseStorage(Args&& ... args) {
                // clang-format on
                allocator_type alloc = GetAllocator();
                RBTREE_ASSERT_INVARIANT(node_storage_ != nullptr);
                allocator_traits::construct(alloc, node_storage_, std::forward<Args>(args)...);
                return std::exchange(node_storage_, nullptr);
            }

        private:
            pointer node_storage_;
        };

        return NodeStorageProxy{}.ConstructNodeAndReleaseStorage(std::forward<Args>(args)...);
    }

    [[nodiscard]] ATTRIBUTE_ALWAYS_INLINE static constexpr allocator_type GetAllocator() noexcept {
        static_assert(sizeof(allocator_type) <= sizeof([]() {}) &&
                          std::is_nothrow_default_constructible_v<allocator_type>,
                      "Stateless allocator is expected");
        return allocator_type{};
    }

    static constexpr void DestroyNode(NodeType* const node) noexcept(kIsNodeNoexceptDestructible) {
        static_assert(std::is_destructible_v<KeyType>);
        allocator_type alloc = GetAllocator();
        allocator_traits::destroy(alloc, node);
        allocator_traits::deallocate(alloc, node, 1);
    }

    struct ClonedTreeData {
        NodeType* root_node;
        NodeType* left_most_node;
        NodeType* right_most_node;
    };

    static constexpr CompleteTreeRawData CloneTree(const RBTreeContainer& other) {
        const NodeType* const other_root = other.Root();
        const ClonedTreeData tree_data =
            other_root != nullptr ? CloneTreeImpl(other_root) : ClonedTreeData{};
        return CompleteTreeRawData{
            .root_node = tree_data.root_node,
            .leftmost_node = tree_data.left_most_node,
            .rightmost_node = tree_data.right_most_node,
            .tree_size = other.size(),
        };
    }

    RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS
    ATTRIBUTE_ACCESS(read_only, 1)
    static constexpr ClonedTreeData CloneTreeImpl(const NodeType* const other_root) {
        struct TemporaryTreeDeleter final {
            void operator()(NodeType* const node) const
                noexcept(noexcept(RBTreeContainer::DeleteTree(node))) {
                RBTreeContainer::DeleteTree(node);
            }
        };
        using TemporaryTreeHolder = std::unique_ptr<NodeType, TemporaryTreeDeleter>;
        TemporaryTreeHolder new_root(RBTreeContainer::CreateNodeUnchecked(other_root->Key()));
        new_root->SetColor(other_root->Color());
        NodeType* left_most_node = new_root.get();
        NodeType* right_most_node = new_root.get();
        if (const NodeType* left = other_root->Left(); left != nullptr) {
            const ClonedTreeData left_clone_data = CloneTreeImpl(left);
            new_root->SetLeft(left_clone_data.root_node);
            left_clone_data.root_node->SetParent(new_root.get());
            left_most_node = left_clone_data.left_most_node;
        }
        if (const NodeType* right = other_root->Right(); right != nullptr) {
            const ClonedTreeData right_clone_data = CloneTreeImpl(right);
            new_root->SetRight(right_clone_data.root_node);
            right_clone_data.root_node->SetParent(new_root.get());
            right_most_node = right_clone_data.right_most_node;
        }

        return ClonedTreeData{
            .root_node = new_root.release(),
            .left_most_node = left_most_node,
            .right_most_node = right_most_node,
        };
    }

    ATTRIBUTE_ACCESS(read_write, 1)

    static constexpr void DeleteTree(Node<KeyType>* const root) noexcept(
        std::is_nothrow_destructible_v<KeyType>) {
        if (root == nullptr) {
            return;
        }
        root->SetParent(nullptr);
        Node<KeyType>* local_root = root;
        do {
            Node<KeyType>* const next_node = [left = local_root->Left(),
                                              right = local_root->Right(),
                                              local_root]() noexcept -> Node<KeyType>* {
                if (left != nullptr && right != nullptr) {
                    RBTREE_ASSERT_INVARIANT(left->Parent() == local_root);
                    RBTREE_ASSERT_INVARIANT(right->Parent() == local_root);
                    right->SetParent(local_root->Parent());
                    left->SetParent(right);
                    return left;
                }

                if (left != nullptr) {
                    RBTREE_ASSERT_INVARIANT(left->Parent() == local_root);
                    left->SetParent(local_root->Parent());
                    return left;
                }

                if (right != nullptr) {
                    RBTREE_ASSERT_INVARIANT(right->Parent() == local_root);
                    right->SetParent(local_root->Parent());
                    return right;
                }

                return local_root->Parent();
            }();

            RBTreeContainer::DestroyNode(local_root);
            local_root = next_node;
        } while (local_root != nullptr);
    }
};

#undef RBTREE_ATTRIBUTE_NONNULL
#undef RBTREE_ATTRIBUTE_NONNULL_ALL_ARGS

template <class T>
#ifndef RBTREE_DEBUG
[[maybe_unused]]
#endif
inline constexpr bool kUseByValue =
    std::is_trivial_v<T> ||
    (std::is_standard_layout_v<T> && !std::is_polymorphic_v<T> && !std::is_array_v<T> &&
     std::is_nothrow_default_constructible_v<T> && sizeof(T) <= 16 &&
     std::is_nothrow_copy_constructible_v<T> && std::is_trivially_copy_constructible_v<T> &&
     std::is_nothrow_copy_assignable_v<T> && std::is_trivially_copy_assignable_v<T> &&
     std::is_nothrow_move_constructible_v<T> && std::is_trivially_move_constructible_v<T> &&
     std::is_nothrow_move_assignable_v<T> && std::is_trivially_move_assignable_v<T> &&
     std::is_nothrow_destructible_v<T> && std::is_trivially_destructible_v<T>);

#ifdef RBTREE_DEBUG
static_assert(kUseByValue<bool>);
static_assert(kUseByValue<std::int64_t>);
static_assert(kUseByValue<long double>);
static_assert(kUseByValue<std::nullptr_t>);
static_assert(kUseByValue<std::chrono::nanoseconds>);
static_assert(kUseByValue<std::string_view>);
#endif

export template <class Comparator>
concept StatelessComparator =
    std::is_trivially_default_constructible_v<Comparator> &&
    std::is_nothrow_default_constructible_v<Comparator> &&
    std::is_trivially_destructible_v<Comparator> && std::is_nothrow_destructible_v<Comparator> &&
    sizeof(Comparator) <= std::min({sizeof(std::less<int>), sizeof(std::less<>), sizeof([]() {})});

static_assert(StatelessComparator<std::less<>>);
static_assert(StatelessComparator<std::less<std::pair<double, float>>>);
static_assert(StatelessComparator<std::greater<int>>);
static_assert(StatelessComparator<std::greater<long*>>);

template <class ComparatorType>
class StatelessComparatorStorage {
    static_assert(StatelessComparator<ComparatorType>);

protected:
    static constexpr bool kNoexceptDefaultConstructable = true;
    static constexpr bool kNoexceptConstructableFromComparator = true;
};

template <class ComparatorType>
class ComparatorStorage {
protected:
    static constexpr bool kNoexceptDefaultConstructable =
        std::is_nothrow_default_constructible_v<ComparatorType>;

    static constexpr bool kNoexceptConstructableFromComparator =
        std::is_nothrow_move_constructible_v<ComparatorType>;

    constexpr ComparatorStorage() noexcept(kNoexceptDefaultConstructable) = default;

    explicit constexpr ComparatorStorage(ComparatorType comp) noexcept(
        kNoexceptConstructableFromComparator)
        : comp_{std::move(comp)} {}

    [[nodiscard]]
    constexpr const ComparatorType& GetComparator() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return comp_;
    }

private:
    ComparatorType comp_;
};

template <class KeyType, class ComparatorType>
    requires rbtree::ComparatorForType<ComparatorType, KeyType>
class NonReflexiveComparatorHelper
    : private std::conditional_t<StatelessComparator<ComparatorType>,
                                 StatelessComparatorStorage<ComparatorType>,
                                 ComparatorStorage<ComparatorType>> {
    static constexpr bool kStatelessComparatorStorage = StatelessComparator<ComparatorType>;

    using Base = std::conditional_t<kStatelessComparatorStorage,
                                    StatelessComparatorStorage<ComparatorType>,
                                    ComparatorStorage<ComparatorType>>;

    static constexpr bool kStdLess = std::is_same_v<ComparatorType, std::less<>> ||
                                     std::is_same_v<ComparatorType, std::less<KeyType>> ||
                                     std::is_same_v<ComparatorType, std::ranges::less>;

    static constexpr bool kStdGreater = std::is_same_v<ComparatorType, std::greater<>> ||
                                        std::is_same_v<ComparatorType, std::greater<KeyType>> ||
                                        std::is_same_v<ComparatorType, std::ranges::greater>;

    static constexpr bool kReflexiveComparator =
        std::is_same_v<ComparatorType, std::less_equal<>> ||
        std::is_same_v<ComparatorType, std::greater_equal<>> ||
        std::is_same_v<ComparatorType, std::less_equal<KeyType>> ||
        std::is_same_v<ComparatorType, std::greater_equal<KeyType>> ||
        std::is_same_v<ComparatorType, std::ranges::less_equal> ||
        std::is_same_v<ComparatorType, std::ranges::greater_equal>;

    static_assert(
        !kReflexiveComparator,
        "Comparator should not establish reflexive relation (i.e. <= and >= can't be used)");

    template <class LhsKeyType, class RhsKeyType>
    static constexpr bool IsNoexceptThreeWayComparable() noexcept {
        if constexpr (kStdLess || kStdGreater) {
            return noexcept(std::compare_weak_order_fallback(std::declval<const LhsKeyType&>(),
                                                             std::declval<const RhsKeyType&>())) &&
                   noexcept(std::compare_weak_order_fallback(std::declval<const RhsKeyType&>(),
                                                             std::declval<const LhsKeyType&>()));
        } else {
            return noexcept(std::declval<const ComparatorType&>()(
                       std::declval<const LhsKeyType&>(), std::declval<const RhsKeyType&>())) &&
                   noexcept(std::declval<const ComparatorType&>()(
                       std::declval<const RhsKeyType&>(), std::declval<const LhsKeyType&>()));
        }
    }

protected:
    constexpr NonReflexiveComparatorHelper() noexcept(Base::kNoexceptDefaultConstructable) =
        default;

    explicit constexpr NonReflexiveComparatorHelper(ComparatorType /*cmp*/) noexcept
        requires(kStatelessComparatorStorage)
    {}

    explicit constexpr NonReflexiveComparatorHelper(ComparatorType cmp) noexcept(
        Base::kNoexceptConstructableFromComparator)
        requires(!kStatelessComparatorStorage)
        : Base(std::move(cmp)) {}

    template <class LhsKeyType, class RhsKeyType>
    static constexpr bool kAreNoexceptThreeWayComparable =
        IsNoexceptThreeWayComparable<LhsKeyType, RhsKeyType>();

    template <class OtherKeyType>
    static constexpr bool kIsComparableWithKey =
        std::same_as<OtherKeyType, std::remove_cvref_t<OtherKeyType>> &&
        requires(const KeyType key,
                 const KeyType& key_reference,
                 OtherKeyType other_key,
                 const OtherKeyType& other_key_reference,
                 const ComparatorType& comparator) {
            { comparator(key, other_key) } -> std::same_as<bool>;
            { comparator(other_key, key) } -> std::same_as<bool>;
            { comparator(key_reference, other_key) } -> std::same_as<bool>;
            { comparator(other_key, key_reference) } -> std::same_as<bool>;
            { comparator(key, other_key_reference) } -> std::same_as<bool>;
            { comparator(other_key_reference, key) } -> std::same_as<bool>;
            { comparator(key_reference, other_key_reference) } -> std::same_as<bool>;
            { comparator(other_key_reference, key_reference) } -> std::same_as<bool>;
        };

    static_assert(kIsComparableWithKey<KeyType>,
                  "Key type should be comparable with itself by the given const comparator");

    template <class OtherKeyType>
    static constexpr bool kIsNoexceptThreeWayComparableWith =
        kIsComparableWithKey<OtherKeyType> &&
        kAreNoexceptThreeWayComparable<const OtherKeyType&, const KeyType&>;

    template <class LhsKeyType, class RhsKeyType>
        requires(kIsComparableWithKey<LhsKeyType> && kIsComparableWithKey<RhsKeyType>)
    [[nodiscard]] constexpr std::weak_ordering CompareThreeWay(const LhsKeyType& lhs,
                                                               const RhsKeyType& rhs) const
        noexcept(kAreNoexceptThreeWayComparable<LhsKeyType, RhsKeyType>) {
        if constexpr (kStdLess) {
            return std::compare_weak_order_fallback(lhs, rhs);
        } else if constexpr (kStdGreater) {
            return std::compare_weak_order_fallback(rhs, lhs);
        } else {
            const ComparatorType& cmp = Base::GetComparator();
            if (cmp(lhs, rhs)) {
                return std::weak_ordering::less;
            }
            if (cmp(rhs, lhs)) {
                return std::weak_ordering::greater;
            }
            return std::weak_ordering::equivalent;
        }
    }
};

namespace rbtree {
template <RBTreeKeyTypeConstraints KeyType, class ComparatorType>
    requires ComparatorForType<ComparatorType, KeyType>
class RBTree
    : private RBTreeContainer<KeyType>
    , private NonReflexiveComparatorHelper<KeyType, ComparatorType> {
    using Base = RBTreeContainer<KeyType>;
    using ComparatorBase = NonReflexiveComparatorHelper<KeyType, ComparatorType>;

    using ComparatorBase::CompareThreeWay;
    using ComparatorBase::kAreNoexceptThreeWayComparable;
    using ComparatorBase::kIsComparableWithKey;
    using ComparatorBase::kIsNoexceptThreeWayComparableWith;

public:
    using size_type = typename Base::size_type;
    using iterator = typename Base::iterator;
    using const_iterator = typename Base::const_iterator;
    using difference_type = typename Base::difference_type;
    using value_type = typename Base::value_type;
    using reference = typename Base::reference;
    using const_reference = typename Base::const_reference;
    using key_type = typename Base::key_type;
    using reverse_iterator = typename Base::reverse_iterator;
    using const_reverse_iterator = typename Base::const_reverse_iterator;
    using node_type = typename Base::node_type;
    using allocator_type = typename Base::allocator_type;
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

    explicit constexpr RBTree(ComparatorType cmp) noexcept : ComparatorBase(std::move(cmp)) {}

    constexpr RBTree(const std::initializer_list<KeyType> list)
        : RBTree(list.begin(), list.end()) {}

    template <std::input_iterator Iter, std::sentinel_for<Iter> SentinelIter>
    constexpr RBTree(Iter begin_iter, SentinelIter end_iter) {
        this->insert_range(std::move(begin_iter), std::move(end_iter));
    }

    template <std::input_iterator Iter, std::sentinel_for<Iter> SentinelIter>
    constexpr RBTree(Iter begin_iter, SentinelIter end_iter, ComparatorType cmp)
        : ComparatorBase(std::move(cmp)) {
        this->insert_range(std::move(begin_iter), std::move(end_iter));
    }

    template <std::ranges::input_range Range>
    constexpr void insert_range(const Range& range) {
        this->insert_range(std::begin(range), std::end(range));
    }

    template <std::ranges::input_range Range>
        requires(!std::is_reference_v<Range>)
    constexpr void insert_range(Range&& range) {
        this->insert_range(std::make_move_iterator(std::begin(range)),
                           std::make_move_iterator(std::end(range)));
    }

    template <std::input_iterator Iter, std::sentinel_for<Iter> SentinelIter>
    constexpr void insert_range(Iter begin_iter, const SentinelIter end_iter) {
        for (; begin_iter != end_iter; ++begin_iter) {
            this->insert(*begin_iter);  // TODO: insert_hint
        }
    }

    template <std::input_iterator Iter, std::sentinel_for<Iter> SentinelIter>
    constexpr void assign_range(Iter begin_iter, SentinelIter end_iter) {
        this->clear();
        this->insert_range(std::move(begin_iter), std::move(end_iter));
    }

    template <std::ranges::input_range Range>
    constexpr void assign_range(const Range& range) {
        this->assign_range(std::begin(range), std::end(range));
    }

    template <std::ranges::input_range Range>
        requires(!std::is_reference_v<Range>)
    constexpr void assign_range(Range&& range) {
        this->assign_range(std::make_move_iterator(std::begin(range)),
                           std::make_move_iterator(std::end(range)));
    }

    constexpr void swap(RBTree& other) noexcept {
        Base::swap(other);
    }

    friend constexpr void swap(RBTree& lhs, RBTree& rhs) noexcept {
        lhs.swap(rhs);
    }

    struct InsertResult {
        iterator iter;
        bool inserted;
    };

    template <class U = KeyType>
    constexpr InsertResult insert(U&& key) ATTRIBUTE_LIFETIME_BOUND {
        return this->template insert_impl</*OverwriteIfExists = */ false>(std::forward<U>(key));
    }

    template <class U = KeyType>
    constexpr iterator insert_or_overwrite(U&& key) ATTRIBUTE_LIFETIME_BOUND {
        return this->template insert_impl</*OverwriteIfExists = */ true>(std::forward<U>(key));
    }

    template <class EraseKeyType = KeyType>
        requires(ComparatorBase::template kIsComparableWithKey<EraseKeyType>)
    constexpr size_type erase(const EraseKeyType& key) noexcept(
        ComparatorBase::template kIsNoexceptThreeWayComparableWith<EraseKeyType> &&
        Base::kIsNodeNoexceptDestructible) {
        const_iterator iter = this->find(key);
        if (iter == this->end()) [[unlikely]] {
            return 0;
        }
        this->erase(std::move(iter));
        return 1;
    }

    constexpr void erase(const_iterator iter) noexcept(Base::kIsNodeNoexceptDestructible) {
        RBTREE_ASSERT_INVARIANT(iter != end());
        this->erase_impl(std::move(iter));
    }

    template <class LowerBoundKeyType = KeyType>
        requires(ComparatorBase::template kIsComparableWithKey<LowerBoundKeyType>)
    [[nodiscard]] constexpr const_iterator lower_bound(const LowerBoundKeyType& key) const
        noexcept(ComparatorBase::template kIsNoexceptThreeWayComparableWith<LowerBoundKeyType>)
            ATTRIBUTE_LIFETIME_BOUND {
        const node_type* last_left_turn = nullptr;
        for (auto* current_node = this->Root(); current_node != nullptr;) {
            const auto compare_result = CompareThreeWay(key, current_node->Key());
            if (compare_result < 0) {
                last_left_turn = current_node;
                current_node = current_node->Left();
            } else if (compare_result > 0) {
                current_node = current_node->Right();
            } else {
                return Base::NodePtrToConstIterator(current_node);
            }
        }

        return last_left_turn != nullptr ? Base::NodePtrToConstIterator(last_left_turn)
                                         : this->end();
    }

    template <class FindKeyType = KeyType>
        requires(ComparatorBase::template kIsComparableWithKey<FindKeyType>)
    [[nodiscard]] constexpr const_iterator find(const FindKeyType& key) const
        noexcept(ComparatorBase::template kIsNoexceptThreeWayComparableWith<FindKeyType>)
            ATTRIBUTE_LIFETIME_BOUND {
        for (auto* current_node = this->Root(); current_node != nullptr;) {
            const auto compare_result = CompareThreeWay(key, current_node->Key());
            if (compare_result < 0) {
                current_node = current_node->Left();
            } else if (compare_result > 0) {
                current_node = current_node->Right();
            } else {
                return Base::NodePtrToConstIterator(current_node);
            }
        }

        return this->end();
    }

    template <class FindKeyType = KeyType>
        requires(ComparatorBase::template kIsComparableWithKey<FindKeyType>)
    [[nodiscard]] constexpr bool contains(const FindKeyType& key) const
        noexcept(ComparatorBase::template kIsNoexceptThreeWayComparableWith<FindKeyType>) {
        return this->find(key) != this->end();
    }

    friend TestStatus rbtree::RBTreeInvariantsUnitTest<KeyType, ComparatorType>(
        const RBTree<KeyType, ComparatorType>& tree);

private:
    [[nodiscard]] constexpr const node_type* Root() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        return Base::Root();
    }

    // clang-format off
    template <bool OverwriteIfExists, class U>
    [[nodiscard]]
    constexpr std::conditional_t<OverwriteIfExists, iterator, InsertResult> insert_impl(U &&key) ATTRIBUTE_LIFETIME_BOUND {
        // clang-format on
        auto* previous_node = Base::PreRootNodePtr();
        node_type* current_node = Base::Root();

        bool last_cmp_res_was_left = false;
        while (current_node != nullptr) {
            const auto compare_result = this->CompareThreeWay(key, current_node->Key());
            previous_node = current_node;
            last_cmp_res_was_left = compare_result < 0;
            if (compare_result < 0) {
                current_node = current_node->Left();
            } else if (compare_result > 0) {
                current_node = current_node->Right();
            } else {
                if constexpr (OverwriteIfExists) {
                    current_node->SetKey(std::forward<U>(key));
                    return Base::NodePtrToIterator(current_node);
                } else {
                    return InsertResult{
                        .iter = Base::NodePtrToIterator(current_node),
                        .inserted = false,
                    };
                }
            }
        }

        const bool insert_left = last_cmp_res_was_left;
        node_type* const new_node =
            Base::InsertNode(previous_node, insert_left, std::forward<U>(key));
        auto inserted_node_iter = Base::NodePtrToIterator(new_node);
        if constexpr (OverwriteIfExists) {
            return inserted_node_iter;
        } else {
            return InsertResult{
                .iter = std::move(inserted_node_iter),
                .inserted = true,
            };
        }
    }

    constexpr void erase_impl(const_iterator iter) noexcept(Base::kIsNodeNoexceptDestructible) {
        Base::ExtractAndDestroyNode(iter);
    }
};

namespace impl {
template <class TimeType>
    requires std::is_arithmetic_v<TimeType>
class TimeInterval final {
public:
    static TimeInterval FromLowHighEndpoints(const TimeType low, const TimeType high) {
        if (low > high) [[unlikely]] {
            ThrowOnInvalidTimeIntervalEndpoints(low, high);
        }

        return TimeInterval{
            low,
            high,
        };
    }

    [[nodiscard]] constexpr const TimeType& LowEndpoint() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        CheckInvariants();
        return low_;
    }

    [[nodiscard]] constexpr const TimeType& HighEndpoint() const noexcept ATTRIBUTE_LIFETIME_BOUND {
        CheckInvariants();
        return high_;
    }

private:
    constexpr void CheckInvariants() const noexcept {
        RBTREE_ASSERT_INVARIANT(low_ <= high_);
        CONFIG_ASSUME_STATEMENT(low_ <= high_);
    }

    constexpr TimeInterval(const TimeType low, const TimeType high) noexcept(
        std::is_nothrow_copy_constructible_v<TimeType>)
        : low_(low), high_(high) {
        CheckInvariants();
    }

    [[noreturn]]
    ATTRIBUTE_COLD static void ThrowOnInvalidTimeIntervalEndpoints(
        [[maybe_unused]] const TimeType low,
        [[maybe_unused]] const TimeType high,
        const std::source_location& location = std::source_location::current()) {
        using namespace std::string_view_literals;
        throw std::runtime_error{"low (which is "sv + std::to_string(low) +
                                 ") > high (which is "sv + std::to_string(high) + ") at "sv +
                                 location.file_name() + ':' + std::to_string(location.line()) +
                                 ':' + location.function_name()};
    }

    TimeType low_;
    TimeType high_;
};

class TimeIntervalComparator {
public:
    template <class TimeType>
    [[nodiscard]] constexpr bool operator()(
        const TimeInterval<TimeType>& lhs,
        const TimeInterval<TimeType>& rhs) noexcept(noexcept(std::declval<const TimeType&>() <
                                                             std::declval<const TimeType&>())) {
        return lhs.LowEndpoint() < rhs.LowEndpoint();
    }
};

template <class TimeType>
class TimeIntervalNode final : Node<TimeInterval<TimeType>> {
    using Base = Node<TimeInterval<TimeType>>;

public:
    using Base::Base;
    using Base::operator=;

private:
    TimeType max_subtree_endpoint_;
};

template <class TimeType = std::int64_t, class ComparatorType = TimeIntervalComparator>
class IntervalTree final
    : private RBTreeContainer<TimeInterval<TimeType>, TimeIntervalNode<TimeType>>
    , private NonReflexiveComparatorHelper<TimeInterval<TimeType>, ComparatorType> {};
}  // namespace impl

template <class KeyType>
struct [[nodiscard]] TestImplResult {
    TestStatus status{};
    std::int32_t height{};
    KeyType min_key{};
    KeyType max_key{};
};

template <class KeyType>
[[nodiscard]]
TestImplResult<KeyType> CheckRBTreeNodeInvariants(const typename RBTree<KeyType>::node_type& node) {
#if CONFIG_CLANG_AT_LEAST(21, 0)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnrvo"
#endif

    std::int32_t height_l = 0;
    std::int32_t height_r = 0;

    KeyType min_l = node.Key();
    KeyType max_r = node.Key();

    const auto* const left_child_ptr = node.Left();
    const auto* const right_child_ptr = node.Right();

    switch (node.Color()) {
        case NodeColor::kRed: {
            if (auto* parent = node.Parent();
                parent == nullptr || parent->Color() != NodeColor::kBlack) {
                return {
                    .status = TestStatus::kInvalidOrNotBlackParentOfRedNode,
                    .height = {},
                    .min_key = {},
                    .max_key = {},
                };
            }
            const bool only_one_nil = (left_child_ptr == nullptr && right_child_ptr != nullptr) ||
                                      (left_child_ptr != nullptr && right_child_ptr == nullptr);
            if (only_one_nil) {
                return {
                    .status = TestStatus::kRedNodeHasExactlyOneNilChild,
                    .height = {},
                    .min_key = {},
                    .max_key = {},
                };
            }

            break;
        }
        case NodeColor::kBlack: {
            break;
        }
        default: {
            return {
                .status = TestStatus::kNodeHasInvalidColor,
                .height = {},
                .min_key = {},
                .max_key = {},
            };
        }
    }

    if (left_child_ptr != nullptr) {
        const auto& left_child = *left_child_ptr;
        if (left_child.Key() >= node.Key()) {
            return {
                .status = TestStatus::kKeyOfLeftSonGEThanKeyOfNode,
                .height = {},
                .min_key = {},
                .max_key = {},
            };
        }
        if (node.Color() == NodeColor::kRed && left_child.Color() != NodeColor::kBlack) {
            return {
                .status = TestStatus::kLeftSonOfRedNodeIsNotBlack,
                .height = {},
                .min_key = {},
                .max_key = {},
            };
        }

        const TestImplResult<KeyType> left_subtree_info =
            CheckRBTreeNodeInvariants<KeyType>(left_child);
        RBTREE_ASSERT_INVARIANT(left_subtree_info.min_key <= left_subtree_info.max_key);
        if (left_subtree_info.status != TestStatus::kOk) {
            return left_subtree_info;
        }
        if (left_subtree_info.max_key >= node.Key()) {
            return {
                .status = TestStatus::kMaxKeyInLeftSubtreeGEThanKeyOfNode,
                .height = {},
                .min_key = {},
                .max_key = {},
            };
        }

        min_l = std::min(min_l, left_subtree_info.min_key);
        height_l = left_subtree_info.height;
    }

    if (right_child_ptr != nullptr) {
        const auto& right_child = *right_child_ptr;
        if (node.Key() >= right_child.Key()) {
            return {
                .status = TestStatus::kKeyOfNodeGEThanKeyOfRightSon,
                .height = {},
                .min_key = {},
                .max_key = {},
            };
        }
        if (node.Color() == NodeColor::kRed && right_child.Color() != NodeColor::kBlack) {
            return {
                .status = TestStatus::kRightSonOfRedNodeIsNotBlack,
                .height = {},
                .min_key = {},
                .max_key = {},
            };
        }

        const TestImplResult<KeyType> right_subtree_info =
            CheckRBTreeNodeInvariants<KeyType>(right_child);
        RBTREE_ASSERT_INVARIANT(right_subtree_info.min_key <= right_subtree_info.max_key);
        if (right_subtree_info.status != TestStatus::kOk) {
            return right_subtree_info;
        }
        if (node.Key() >= right_subtree_info.min_key) {
            return {
                .status = TestStatus::kKeyOfNodeGEThanMinKeyInRightSubtree,
                .height = {},
                .min_key = {},
                .max_key = {},
            };
        }

        max_r = std::max(max_r, right_subtree_info.max_key);
        height_r = right_subtree_info.height;
    }

    RBTREE_ASSERT_INVARIANT(min_l <= max_r);
    if (height_l < height_r) {
        return {
            .status = TestStatus::kBlackHeightOfLeftSubtreeLSThanBlackHeightOfRightSubtree,
            .height = {},
            .min_key = {},
            .max_key = {},
        };
    }
    if (height_l > height_r) {
        return {
            .status = TestStatus::kBlackHeightOfLeftSubtreeGTThanBlackHeightOfRightSubtree,
            .height = {},
            .min_key = {},
            .max_key = {},
        };
    }

    return {
        .status = TestStatus::kOk,
        .height = height_l + int32_t{node.Color() == NodeColor::kBlack},
        .min_key = min_l,
        .max_key = max_r,
    };
#if CONFIG_CLANG_AT_LEAST(21, 0)
#pragma clang diagnostic pop
#endif
}

template <class KeyType, class ComparatorType>
TestStatus RBTreeInvariantsUnitTest(const RBTree<KeyType, ComparatorType>& tree) {
    const typename RBTree<KeyType>::node_type* root_ptr = tree.Root();
    if (root_ptr == nullptr) {
        return TestStatus::kOk;
    }
    const typename RBTree<KeyType>::node_type& root = *root_ptr;
    if (root.Color() != NodeColor::kBlack) {
        return TestStatus::kRootIsNotBlack;
    }

    const auto [status, h, min, max] = CheckRBTreeNodeInvariants<KeyType>(root);
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
