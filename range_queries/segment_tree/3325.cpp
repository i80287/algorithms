#include <cstdint>
#include <iostream>
#include <vector>

using std::vector;

void fill_tree(const vector<uint32_t>& nums, vector<uint32_t>& tree, size_t node_index, size_t l, size_t r) {
    if (l != r) {
        size_t left_son_index = node_index * 2 + 1;
        size_t right_son_index = node_index * 2 + 2;
        size_t middle = (l + r) / 2;
        fill_tree(nums, tree, left_son_index, l, middle);
        fill_tree(nums, tree, right_son_index, middle + 1, r);
        tree[node_index] = tree[left_son_index] + tree[right_son_index];
    } else {
        tree[node_index] = (nums[l] == 0);
    }
}

uint32_t count_zeros(
    const vector<uint32_t>& tree, size_t node_index, size_t tree_l, size_t tree_r, size_t q_l, size_t q_r) {
    if (tree_l == q_l && tree_r == q_r) {
        return tree[node_index];
    }

    size_t middle = (tree_l + tree_r) / 2;
    if (q_r <= middle) {
        return count_zeros(tree, node_index * 2 + 1, tree_l, middle, q_l, q_r);
    }

    if (middle < q_l) {
        return count_zeros(tree, node_index * 2 + 2, middle + 1, tree_r, q_l, q_r);
    }

    // q_l <= middle < q_r
    return count_zeros(tree, node_index * 2 + 1, tree_l, middle, q_l, middle) +
           count_zeros(tree, node_index * 2 + 2, middle + 1, tree_r, middle + 1, q_r);
}

void update_tree(vector<uint32_t>& tree, size_t node_index, size_t tree_l, size_t tree_r, size_t index, bool is_zero) {
    if (tree_l == tree_r) {
        tree[node_index] = is_zero;
        return;
    }

    size_t middle = (tree_l + tree_r) / 2;
    size_t left_son_index = node_index * 2 + 1;
    size_t right_son_index = node_index * 2 + 2;
    if (index <= middle) {
        update_tree(tree, left_son_index, tree_l, middle, index, is_zero);
    } else {
        update_tree(tree, right_son_index, middle + 1, tree_r, index, is_zero);
    }
    tree[node_index] = tree[left_son_index] + tree[right_son_index];
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    size_t n = 0;
    std::cin >> n;
    vector<uint32_t> nums(n);
    for (size_t i = 0; i < n; ++i) {
        std::cin >> nums[i];
    }

    vector<uint32_t> tree(4 * n, 0);
    fill_tree(nums, tree, 0, 0, n - 1);

    uint32_t q = 0;
    std::cin >> q;
    while (q--) {
        char cmd = '\0';
        std::cin >> cmd;
        if (cmd == 's') {
            size_t l = 0, r = 0;
            std::cin >> l >> r;
            std::cout << count_zeros(tree, 0, 0, n - 1, --l, --r) << ' ';
        } else {
            size_t index = 0;
            uint32_t value = 0;
            std::cin >> index >> value;
            update_tree(tree, 0, 0, n - 1, index - 1, value == 0);
        }
    }
    std::cout.flush();
}
