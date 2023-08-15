#include <cstdint>
#include <vector>
#include <iostream>

using std::vector;

void fill_tree(const vector<uint32_t>& nums, vector<size_t>& indexes_tree, size_t i, size_t l, size_t r) {
    if (l != r) {
        size_t left_son_index = 2 * i + 1;
        size_t right_son_index = 2 * i + 2;
        size_t middle = (l + r) / 2;
        fill_tree(nums, indexes_tree, left_son_index, l, middle);
        fill_tree(nums, indexes_tree, right_son_index, middle + 1, r);

        size_t left_max_index = indexes_tree[left_son_index];
        size_t right_max_index = indexes_tree[right_son_index];
        indexes_tree[i] = nums[left_max_index] >= nums[right_max_index] ? left_max_index : right_max_index;
    } else {
        indexes_tree[i] = l;
    }
}

size_t find_left_max_index(const vector<uint32_t>& nums, const vector<size_t>& indexes_tree, size_t i, size_t tree_l, size_t tree_r, size_t q_l, size_t q_r) {
    if (tree_l == q_l && tree_r == q_r) {
        return indexes_tree[i];
    }

    size_t middle = (tree_l + tree_r) / 2;
    if (q_r <= middle) {
        return find_left_max_index(nums, indexes_tree, 2 * i + 1, tree_l, middle, q_l, q_r);
    }

    if (q_l > middle) {
        return find_left_max_index(nums, indexes_tree, 2 * i + 2, middle + 1, tree_r, q_l, q_r);
    }

    // q_l <= middle < q_r

    size_t left_max_index = find_left_max_index(nums, indexes_tree, 2 * i + 1, tree_l, middle, q_l, middle);
    size_t right_max_index = find_left_max_index(nums, indexes_tree, 2 * i + 2, middle + 1, tree_r, middle + 1, q_r);
    return nums[left_max_index] >= nums[right_max_index] ? left_max_index : right_max_index;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    size_t n = 0;
    std::cin >> n;
    vector<uint32_t> nums(n);
    for (size_t i = 0; i < n; i++) {
        std::cin >> nums[i];
    }

    vector<size_t> indexes_tree(4 * n, 0);
    fill_tree(nums, indexes_tree, 0, 0, n - 1);

    size_t q = 0;
    std::cin >> q;
    while (q--) {
        size_t l = 0, r = 0;
        std::cin >> l >> r;
        std::cout << find_left_max_index(nums, indexes_tree, 0, 0, n - 1, --l, --r) + 1 << ' ';
    }
    std::cout.flush();
}
