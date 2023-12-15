#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>

class BTree {
   public:
    explicit BTree(size_t);

    void insert(int32_t);

    constexpr size_t size() const noexcept {
        return nodes_.size() - 1;
    }

    int64_t sum() const noexcept;

   private:
    struct Node {
        int32_t* keys_ = nullptr;
        uint32_t* child_indexes_ = nullptr;
        size_t keys_size_ = 0;
        size_t next_in_row_index_ = 0;

        explicit Node(size_t keys_capacity) {
            assert(keys_capacity != 0);
            keys_ = new int32_t[keys_capacity]();
            child_indexes_ = new uint32_t[keys_capacity + 1]();
        }

        Node(const Node&) = delete;
        Node& operator=(const Node&) = delete;

        constexpr Node(Node&& other) noexcept
            : keys_(other.keys_),
              child_indexes_(other.child_indexes_),
              keys_size_(other.keys_size_),
              next_in_row_index_(other.next_in_row_index_) {
            other.keys_ = nullptr;
            other.child_indexes_ = nullptr;
            other.keys_size_ = 0;
            other.next_in_row_index_ = 0;
        }

        constexpr Node& operator=(Node&& other) noexcept {
            this->~Node();
            keys_ = other.keys_;
            child_indexes_ = other.child_indexes_;
            keys_size_ = other.keys_size_;
            next_in_row_index_ = other.next_in_row_index_;
            other.keys_ = nullptr;
            other.child_indexes_ = nullptr;
            other.keys_size_ = 0;
            other.next_in_row_index_ = 0;
            return *this;
        }

        ~Node() {
            delete[] child_indexes_;
            delete[] keys_;
        }

        size_t next_node_index(int32_t key) const noexcept {
            assert(keys_size_ != 0);
            size_t l = 0;
            size_t r = keys_size_ - 1;
            while (ssize_t(r) >= ssize_t(l)) {
                size_t m = (l + r) / 2;
                if (keys_[m] > key) {
                    r = m - 1;
                } else if (keys_[m] != key) {
                    l = m + 1;
                } else {
                    return size_t(-1);
                }
            }

            assert(l <= keys_size_);
            assert(l == keys_size_ || key < keys_[l]);
            assert(l == 0 || keys_[l - 1] < key);
            return l;
        }

        constexpr size_t keys_size() const noexcept { return keys_size_; }

        constexpr int32_t first_key() const noexcept { return keys_[0]; }

        constexpr int32_t last_key() const noexcept {
            return keys_[keys_size() - 1];
        }

        constexpr int64_t keys_sum() const noexcept {
            int64_t ksum = 0;
            for (int32_t* iter = keys_, *iter_end = iter + keys_size_; iter != iter_end; ++iter) {
                ksum += *iter;
            }
            return ksum;
        }

        void insert_checked(size_t pos, int32_t key,
                            uint32_t child_index) noexcept {
            assert(pos <= keys_size_);
            size_t right_keys_size = keys_size_ - pos;
            if (right_keys_size) {
                memmove(&keys_[pos + 1], &keys_[pos],
                        right_keys_size * sizeof(int32_t));
                memmove(&child_indexes_[pos + 2], &child_indexes_[pos + 1],
                        right_keys_size * sizeof(uint32_t));
            }

            keys_[pos] = key;
            keys_size_++;
            child_indexes_[pos + 1] = child_index;
        }

        int32_t split_into(Node* new_brother, size_t new_brother_index, size_t pos, int32_t key,
                           uint32_t child_index,
                           size_t keys_capacity) noexcept {
            size_t middle_key_index = keys_size_ / 2;
            assert(middle_key_index + 1 < keys_capacity);

            size_t new_node_size = keys_capacity - (middle_key_index + 1);
            assert(((keys_capacity + 1) / 2) - 1 <= new_node_size &&
                   new_node_size < keys_capacity);

            memcpy(&new_brother->keys_[0], &keys_[middle_key_index + 1],
                   new_node_size * sizeof(int32_t));
            new_brother->keys_size_ = new_node_size;
            keys_size_ = middle_key_index;
            memcpy(&new_brother->child_indexes_[0],
                   &child_indexes_[middle_key_index + 1],
                   (new_node_size + 1) * sizeof(uint32_t));

            int32_t jostile_key = keys_[middle_key_index];

            if (pos <= middle_key_index) {
                insert_checked(pos, key, child_index);
            } else {
                pos -= (middle_key_index + 1);
                new_brother->insert_checked(pos, key, child_index);
            }

            new_brother->next_in_row_index_ = next_in_row_index_;
            next_in_row_index_ = new_brother_index;

            return jostile_key;
        }
    };

    uint32_t add_new_node() {
        size_t new_index = nodes_.size();
        nodes_.emplace_back(size_t(keys_capacity_));
        return uint32_t(new_index);
    }

   public:
    std::vector<Node> nodes_;
    size_t keys_capacity_ = 0;
    uint32_t root_index_ = 0;
};

BTree::BTree(size_t t) {
    keys_capacity_ = 2 * t - 1;
    nodes_.reserve(64);
    // Fake node with null index 0
    nodes_.emplace_back(1);
}

void BTree::insert(int32_t key) {
    size_t pos;
    uint32_t current_node_index = root_index_;
    if (__builtin_expect(current_node_index == 0, 0)) {
        root_index_ = add_new_node();
        nodes_[root_index_].keys_[0] = key;
        nodes_[root_index_].keys_size_ = 1;
        return;
    }

    static uint32_t prev_node_index_stack[64];
    size_t prev_node_index_stack_index = size_t(-1);
    for (;;) {
        pos = nodes_[current_node_index].next_node_index(key);
        if (pos == size_t(-1)) {
            return;
        }

        assert(pos <= nodes_[current_node_index].keys_size());
        uint32_t next_node_index =
            nodes_[current_node_index].child_indexes_[pos];
        if (next_node_index) {
            prev_node_index_stack[++prev_node_index_stack_index] =
                current_node_index;
            current_node_index = next_node_index;
        } else {
            break;
        }
    }

    for (uint32_t child_index = 0;;) {
        assert(current_node_index != 0);
        Node* current_node = &nodes_[current_node_index];
        assert(current_node->keys_size_ != 0 &&
               current_node->keys_size_ <= keys_capacity_);
        assert(pos <= current_node->keys_size_);
        if (current_node->keys_size_ != keys_capacity_) {
            current_node->insert_checked(pos, key, child_index);
            break;
        }

        uint32_t jostle_node_index = add_new_node();
        key = nodes_[current_node_index].split_into(
            &nodes_[jostle_node_index], jostle_node_index, pos, key, child_index, keys_capacity_);
        child_index = jostle_node_index;
        if (__builtin_expect(prev_node_index_stack_index != size_t(-1), 1)) {
            current_node_index =
                prev_node_index_stack[prev_node_index_stack_index--];
            pos = nodes_[current_node_index].next_node_index(key);
            continue;
        }

        assert(root_index_ == current_node_index);
        uint32_t new_root_index = add_new_node();
        nodes_[new_root_index].keys_[0] = key;
        nodes_[new_root_index].keys_size_ = 1;

        current_node = &nodes_[current_node_index];
        Node* jostle_node = &nodes_[jostle_node_index];
        assert(current_node->first_key() <= current_node->last_key());
        assert(jostle_node->first_key() <= jostle_node->last_key());

        if (current_node->first_key() < jostle_node->first_key()) {
            assert(current_node->last_key() < jostle_node->first_key());
            nodes_[new_root_index].child_indexes_[0] = current_node_index;
            nodes_[new_root_index].child_indexes_[1] = jostle_node_index;
        } else {
            assert(jostle_node->last_key() < current_node->first_key());
            nodes_[new_root_index].child_indexes_[0] = jostle_node_index;
            nodes_[new_root_index].child_indexes_[1] = current_node_index;
        }

        root_index_ = new_root_index;
        break;
    }
}

int64_t BTree::sum() const noexcept {
    size_t current_node_index = 0;
    for (size_t next_node_index = root_index_; next_node_index != 0; ) {
        assert(next_node_index < nodes_.size());
        current_node_index = next_node_index;
        assert(nodes_[next_node_index].child_indexes_ != nullptr);
        next_node_index = nodes_[next_node_index].child_indexes_[0];
    }

    int64_t level_sum = 0;
    do {
        level_sum += nodes_[current_node_index].keys_sum();
        current_node_index = nodes_[current_node_index].next_in_row_index_;
    } while (current_node_index != 0);
    return level_sum;
}
