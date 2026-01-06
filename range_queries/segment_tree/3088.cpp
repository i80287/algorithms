#include <cstdint>
#include <iostream>
#include <vector>

using std::vector;

void fill_tree(const vector<uint32_t>& nums, vector<uint64_t>& tree, size_t i, size_t l, size_t r) {
    if (l != r) {
        size_t middle = (l + r) / 2;
        size_t first_son_index = 2 * i + 1;
        size_t second_son_index = 2 * i + 2;
        fill_tree(nums, tree, first_son_index, l, middle);
        fill_tree(nums, tree, second_son_index, middle + 1, r);
        tree[i] = tree[first_son_index] + tree[second_son_index];
    } else {
        tree[i] = nums[l];
    }
}

uint64_t find_sum(const vector<uint64_t>& tree, size_t i, size_t tree_l, size_t tree_r, size_t q_l, size_t q_r) {
    if (q_l > q_r) {
        return 0;
    }

    if (tree_l == q_l && tree_r == q_r) {
        return tree[i];
    }

    size_t middle = (tree_l + tree_r) / 2;
    return find_sum(tree, 2 * i + 1, tree_l, middle, q_l, std::min(q_r, middle)) +
           find_sum(tree, 2 * i + 2, middle + 1, tree_r, std::max(q_l, middle + 1), q_r);
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    uint32_t n = 0;
    std::cin >> n;
    vector<uint32_t> nums(n);
    for (size_t i = 0; i < n; ++i) {
        std::cin >> nums[i];
    }

    vector<uint64_t> tree(4 * n);
    fill_tree(nums, tree, 0, 0, n - 1);

    size_t q = 0;
    std::cin >> q;
    while (q--) {
        size_t l, r;
        std::cin >> l >> r;
        std::cout << find_sum(tree, 0, 0, n - 1, --l, --r) << ' ';
    }
    std::cout.flush();
}
