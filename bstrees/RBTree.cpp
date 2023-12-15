#include <cassert>
#include <cstdint>
#include <deque>
#include <initializer_list>
#include <tuple>

enum class Color { RED, BLACK };

template <typename T>
struct Node {
    Node* left = nullptr;
    Node* right = nullptr;
    T key{};
    Color color = Color::RED;
    Node* parent = nullptr;

    constexpr Node<T>* another_child(const Node<T>* child) noexcept {
        return (left == child) ? right : left;
    }
};

template <typename T>
class RBTree {
public:
    RBTree(std::initializer_list<T> list);

    RBTree(const RBTree&) = default;
    RBTree(RBTree&&) noexcept = default;
    RBTree& operator=(const RBTree&) = default;
    RBTree& operator=(RBTree&&) noexcept = default;

    ~RBTree();

    void insert(T key);

    constexpr size_t size() const noexcept {
        return size_;
    }

    constexpr bool empty() const noexcept {
        return size() == 0;
    }

    T *lower_bound(T key) const noexcept;

    T *find(T key) const noexcept;

    Node<T>* root{};
protected:
    static void left_rotate(Node<T>* node) noexcept {
        /**
         *  parent_parent
         *       |
         *     parent
         *      /  \
         *  subt1  node
         *         /  \
         *     subt2  subt3
         * 
         *    parent_parent
         *         |
         *        node
         *        /  \
         *   parent  subt3
         *    /  \
         * subt1 subt2
         */
        assert(node != nullptr);
        Node<T>* parent = node->parent;
        assert(parent != nullptr);
        assert(parent->right == node);
        Node<T>* parent_parent = parent->parent;

        parent->right = node->left;
        if (node->left) {
            node->left->parent = parent;
        }

        node->left = parent;
        parent->parent = node;

        node->parent = parent_parent;
        if (parent_parent) {
            if (parent_parent->left == parent) {
                parent_parent->left = node;
            }
            else {
                assert(parent_parent->right == parent);
                parent_parent->right = node;
            }
        }
    }

    static void right_rotate(Node<T>* node) noexcept {
        /**
         *    parent_parent
         *         |
         *       parent
         *        /  \
         *     node subt1
         *     /  \
         * subt3  subt2
         * 
         *    parent_parent
         *         |
         *        node
         *        /  \
         *    subt3 parent
         *           /  \
         *        subt2 subt1
         */
        assert(node != nullptr);
        Node<T>* parent = node->parent;
        assert(parent != nullptr);
        assert(parent->left == node);
        Node<T>* parent_parent = parent->parent;

        parent->left = node->right;
        if (node->right) {
            node->right->parent = parent;
        }

        node->right = parent;
        parent->parent = node;

        node->parent = parent_parent;
        if (parent_parent) {
            if (parent_parent->left == parent) {
                parent_parent->left = node;
            }
            else {
                assert(parent_parent->right == parent);
                parent_parent->right = node;
            }
        }
    }

    static Node<T>* AnotherChild(Node<T>* node, Node<T>* child) noexcept {
        return (node->left == child) ? node->right : node->left;
    }

    static constexpr int32_t kInvalidBHeight = -1;
public: // FIXME:
    bool is_rbtree() const {
        if (root == nullptr) {
            return true;
        }
        if (root->color != Color::BLACK) {
            return false;
        }

        auto [h, min, max] = impl(root);
        if (h == kInvalidBHeight) {
            assert(min == max && max == 0);
            return false;
        }

        if (root->left) {
            assert(min < root->key);
        }
        else {
            assert(min == root->key);
        }
        if (root->right) {
            assert(root->key < max);
        }
        else {
            assert(root->key == max);
        }
        return true;
    }
protected:
    std::tuple<int32_t, int32_t, int32_t> impl(Node<T>* node) const {
        int32_t height_l = 1;
        int32_t height_r = 1;
        int32_t min_l = node->key;
        int32_t max_r = node->key;

        if (node->left) {
            if (node->left->key >= node->key ||
                (node->color == Color::RED && node->left->color == Color::RED)) {
                return { kInvalidBHeight, 0, 0 };
            }

            auto [h, min, max] = impl(node->left);
            assert(min <= max);
            if (h == kInvalidBHeight || max >= node->key) {
                return { kInvalidBHeight, 0, 0 };
            }

            min_l = std::min(min_l, min);
            height_l = h;
        }

        if (node->right) {
            if (node->key >= node->right->key ||
                (node->color == Color::RED && node->right->color == Color::RED)) {
                return { kInvalidBHeight, 0, 0 };
            }

            auto [h, min, max] = impl(node->right);
            assert(min <= max);
            if (h == kInvalidBHeight || node->key >= min) {
                return { kInvalidBHeight, 0, 0 };
            }

            max_r = std::max(max_r, max);
            height_r = h;
        }

        assert(min_l <= max_r);
        if (height_l == height_r) {
            return { height_l + (node->color == Color::BLACK), min_l, max_r };
        }

        return { kInvalidBHeight, 0, 0 };
    }

    void rebalance(Node<T>* current_node, Node<T>* new_red_son) noexcept {
        assert(current_node != nullptr);
        assert((current_node->left == new_red_son) ^ (current_node->right == new_red_son));
        assert(new_red_son != nullptr && new_red_son->color == Color::RED);
        if (current_node->color == Color::BLACK) {
            return;
        }

        assert(current_node != root);
        Node<T>* parent_node = current_node->parent;
        assert(parent_node != nullptr);
        assert(parent_node->color == Color::BLACK);

        Node<T>* current_node_bruder = parent_node.another_child(current_node);

        if (current_node_bruder != nullptr &&
            current_node_bruder->color == Color::RED) {
            Node<T>* parent_parent_node = parent_node->parent;
            if (parent_parent_node == nullptr) {
                assert(parent_node == root);
                current_node->color = Color::BLACK;
                current_node_bruder->color = Color::BLACK;
                return;
            }

            if (parent_parent_node->color == Color::BLACK) {
                parent_node->color = Color::RED;
                current_node_bruder->color = Color::BLACK;
                current_node->color = Color::BLACK;
                return;
            }

            assert(parent_parent_node->parent != nullptr);
            parent_node->color = Color::RED;
            current_node_bruder->color = Color::BLACK;
            current_node->color = Color::BLACK;
            rebalance(parent_parent_node, parent_node);
            return;
        }

        if (current_node->left == new_red_son) {
            if (parent_node->left == current_node) {
                right_rotate(current_node);
                parent_node->color = Color::RED;
                current_node->color = Color::BLACK;
                if (current_node->parent == nullptr) {
                    assert(current_node->right == root);
                    root = current_node;
                }
            } else {
                assert(parent_node->right == current_node);
                right_rotate(new_red_son);
                left_rotate(new_red_son);
                parent_node->color = Color::RED;
                new_red_son->color = Color::BLACK;
                if (new_red_son->parent == nullptr) {
                    root = new_red_son;
                }
            }
        } else {
            assert(current_node->right == new_red_son);
            if (parent_node->right == current_node) {
                left_rotate(current_node);
                parent_node->color = Color::RED;
                current_node->color = Color::BLACK;
                if (current_node->parent == nullptr) {
                    assert(current_node->left == root);
                    root = current_node;
                }
            } else {
                assert(parent_node->left == current_node);
                left_rotate(new_red_son);
                right_rotate(new_red_son);
                parent_node->color = Color::RED;
                new_red_son->color = Color::BLACK;
                if (new_red_son->parent == nullptr) {
                    root = new_red_son;
                }
            }
        }
    }

    size_t size_ = 0;
};

template <typename T>
RBTree<T>::RBTree(std::initializer_list<T> list) {
    for (const auto& elem : list) {
        insert(elem);
    }
}

template <typename T>
RBTree<T>::~RBTree() {
    if (root == nullptr) {
        return;
    }

    std::deque<Node<T>*> queue;
    queue.push_back(root);
    do {
        Node<T>* node = queue.front();
        queue.pop_front();
        if (node->left) {
            queue.push_back(node->left);
        }
        if (node->right) {
            queue.push_back(node->right);
        }
        delete node;
    } while (!queue.empty());
}

template <typename T>
void RBTree<T>::insert(T key) {
    Node<T>* prev_node = nullptr;
    Node<T>* current_node = root;

    while(current_node != nullptr) {
        prev_node = current_node;
        if (key < current_node->key) {
            current_node = current_node->left;
        }
        else if (key != current_node->key) {
            current_node = current_node ->right;
        }
        else {
            return;
        }
    }

    Node<T>* new_node = new Node<T>();
    new_node->key = key;
    size_++;

    current_node = prev_node;
    if (current_node == nullptr) {
        assert(root == nullptr);
        root = new_node;
        root->color = Color::BLACK;
        return;
    }

    new_node->parent = current_node;
    if (key < current_node->key) {
        assert(current_node->left == nullptr);
        current_node->left = new_node;
    } else {
        assert(current_node->key < key);
        assert(current_node->right == nullptr);
        current_node->right = new_node;
    }

    rebalance(current_node, new_node);
}

template <typename T>
T* RBTree<T>::lower_bound(T key) const noexcept {
    Node<T>* last_left_turn = nullptr;
    for (Node<T>* current_node = root; current_node != nullptr; ) {
        if (current_node->key == key) {
            return &current_node->key;
        }

        if (current_node->key < key) {
            current_node = current_node->right;
        } else {
            last_left_turn = current_node;
            current_node = current_node->left;
        }
    }
    
    return last_left_turn != nullptr ? &last_left_turn->key : nullptr;
}

template <typename T>
T* RBTree<T>::find(T key) const noexcept {
    for (Node<T>* current_node = root; current_node != nullptr; ) {
        if (current_node->key < key) {
            current_node = current_node->right;
        } else if (current_node->key != key) {
            current_node = current_node->left;
        } else {
            return &current_node->key;
        }
    }

    return nullptr;
}
