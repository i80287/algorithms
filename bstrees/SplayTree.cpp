#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>

struct Node {
    int key;
    int32_t height_;
    Node* left;
    Node* right;
    Node* parent;

    constexpr int32_t LeftChildHeight() const noexcept {
        return left != nullptr ? left->height_ : -1;
    }

    constexpr int32_t RightChildHeight() const noexcept {
        return right != nullptr ? right->height_ : -1;
    }

    constexpr void RecalculateHeight() noexcept {
        height_ = std::max(LeftChildHeight(), RightChildHeight()) + 1;
    }
};

class SplayTree {
public:
    SplayTree() : nodes_(static_cast<Node*>(operator new(sizeof(Node) << 15))) {
        memset(nodes_, 0, sizeof(Node) << 15);
    }

    ~SplayTree() {
        operator delete(nodes_);
    }

    void insert(int32_t);

    Node* find(int32_t);

    int splay(Node* node);

    constexpr size_t height() const noexcept {
        return root_ != nullptr ? size_t(uint32_t(root_->height_)) : 0;
    }

public:
    Node* root_ = nullptr;

private:
    Node* nodes_            = nullptr;
    size_t allocator_index_ = 0;

    void LeftRotateImpl(Node* node) noexcept {
        assert(node != nullptr);
        Node* parent_node = node->parent;
        assert(parent_node != nullptr);
        assert(parent_node->right == node);

        assert(node->height_ == 1 + std::max(node->LeftChildHeight(), node->RightChildHeight()));

        parent_node->right = node->left;
        if (node->left != nullptr) {
            node->left->parent = parent_node;
        }

        Node* parent_parent_node = parent_node->parent;
        node->parent             = parent_parent_node;
        if (parent_parent_node != nullptr) {
            if (parent_parent_node->left == parent_node) {
                parent_parent_node->left = node;
            } else {
                assert(parent_parent_node->right == parent_node);
                parent_parent_node->right = node;
            }
        } else {
            assert(parent_node == root_);
        }

        node->left          = parent_node;
        parent_node->parent = node;

        if (parent_node == root_) {
            root_ = node;
        }

        parent_node->RecalculateHeight();
        node->RecalculateHeight();
        if (parent_parent_node) {
            parent_parent_node->RecalculateHeight();
        }
    }

    void RightRotateImpl(Node* node) noexcept {
        assert(node != nullptr);
        Node* parent_node = node->parent;
        assert(parent_node != nullptr);
        assert(parent_node->left == node);

        assert(node->height_ == 1 + std::max(node->LeftChildHeight(), node->RightChildHeight()));

        parent_node->left = node->right;
        if (node->right != 0) {
            node->right->parent = parent_node;
        }

        Node* parent_parent_node = parent_node->parent;
        node->parent             = parent_parent_node;
        if (parent_parent_node != nullptr) {
            if (parent_parent_node->left == parent_node) {
                parent_parent_node->left = node;
            } else {
                assert(parent_parent_node->right == parent_node);
                parent_parent_node->right = node;
            }
        } else {
            assert(parent_node == root_);
        }

        node->right         = parent_node;
        parent_node->parent = node;

        if (parent_node == root_) {
            root_ = node;
        }

        parent_node->RecalculateHeight();
        node->RecalculateHeight();
        if (parent_parent_node) {
            parent_parent_node->RecalculateHeight();
        }
    }

    int Rotate(Node* node) noexcept {
        if (__builtin_expect(node == nullptr, 0)) {
            return 0;
        }

        Node* parent_node = node->parent;
        if (parent_node == nullptr) {
            // node points to the root
            assert(node == root_);
            return 0;
        }

        const Node* parent_parent_node = parent_node->parent;
        if (parent_parent_node == nullptr) {
            // node_index points to the child of the root
            assert(parent_node == root_);
            if (parent_node->left == node) {
                RightRotateImpl(node);
            } else {
                assert(parent_node->right == node);
                LeftRotateImpl(node);
            }
            assert(root_ == node);
            return 1;
        }

        if (parent_node->left == node) {
            if (parent_parent_node->left == parent_node) {
                RightRotateImpl(parent_node);
                RightRotateImpl(node);
                return 2;
            } else {
                assert(parent_parent_node->right == parent_node);
                RightRotateImpl(node);
                LeftRotateImpl(node);
                return 1;
            }
        } else {
            assert(parent_node->right == node);
            if (parent_parent_node->left == parent_node) {
                LeftRotateImpl(node);
                RightRotateImpl(node);
                return 1;
            } else {
                assert(parent_parent_node->right == parent_node);
                LeftRotateImpl(parent_node);
                LeftRotateImpl(node);
                return 2;
            }
        }
    }

    constexpr Node* AddNewNode(int key) noexcept {
        Node* new_node = &nodes_[allocator_index_++];
        new_node->key  = key;
        return new_node;
    }
};

int SplayTree::splay(Node* node) {
    int rotates = 0;
    for (int rotate; (rotate = Rotate(node)) != 0; rotates += rotate) {
    }

    assert(root_ == node);
    return rotates;
}

void SplayTree::insert(int32_t key) {
    Node* current_node = root_;
    if (__builtin_expect(current_node == nullptr, 0)) {
        root_ = AddNewNode(key);
        assert(height() == 0);
        return;
    }

    while (true) {
        int node_key = current_node->key;
        if (__builtin_expect(node_key == key, 0)) {
            return;
        }

        Node* next_node = node_key < key ? current_node->right : current_node->left;
        if (next_node == nullptr) {
            break;
        }

        current_node = next_node;
    }

    Node* new_node   = AddNewNode(key);
    new_node->parent = current_node;
    if (current_node->key < key) {
        current_node->right = new_node;
    } else {
        current_node->left = new_node;
    }

    do {
        current_node->RecalculateHeight();
        current_node = current_node->parent;
    } while (current_node != nullptr);
}

Node* SplayTree::find(int32_t key) {
    Node* current_node = root_;
    while (current_node != nullptr) {
        int node_key = current_node->key;
        if (node_key == key) {
            splay(current_node);
            return current_node;
        }

        if (node_key < key) {
            current_node = current_node->right;
        } else {
            current_node = current_node->left;
        }
    }

    return nullptr;
}
