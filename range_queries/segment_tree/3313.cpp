#include <cstdint>
#include <iostream>
#include <vector>

namespace {

using std::vector;

void fill_tree(const vector<uint32_t>& nums, vector<uint32_t>& tree, size_t i, size_t l, size_t r) {
    if (l != r) {
        size_t middle = (l + r) / 2;
        size_t left_son_index = 2 * i + 1;
        size_t right_son_index = 2 * i + 2;
        fill_tree(nums, tree, left_son_index, l, middle);
        fill_tree(nums, tree, right_son_index, middle + 1, r);
        tree[i] = tree[left_son_index] + tree[right_son_index];
    } else {
        tree[i] = (nums[l] == 0);
    }
}

[[nodiscard]] uint32_t count_zeros(
    const vector<uint32_t>& tree, size_t i, size_t tree_l, size_t tree_r, size_t q_l, size_t q_r) {
    if (tree_l == q_l && tree_r == q_r) {
        return tree[i];
    }

    size_t middle = (tree_l + tree_r) / 2;
    if (q_r <= middle) {
        return count_zeros(tree, 2 * i + 1, tree_l, middle, q_l, q_r);
    } else if (middle < q_l) {
        return count_zeros(tree, 2 * i + 2, middle + 1, tree_r, q_l, q_r);
    }

    // q_l <= middle < q_r
    uint32_t left_zeros_count = count_zeros(tree, 2 * i + 1, tree_l, middle, q_l, middle);
    uint32_t right_zeros_count = count_zeros(tree, 2 * i + 2, middle + 1, tree_r, middle + 1, q_r);
    return left_zeros_count + right_zeros_count;
}

}  // namespace

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    size_t n = 0;
    std::cin >> n;
    vector<uint32_t> nums(n);
    for (size_t i = 0; i < n; i++) {
        std::cin >> nums[i];
    }

    vector<uint32_t> tree(4 * n, 0);
    fill_tree(nums, tree, 0, 0, n - 1);

    size_t q = 0;
    std::cin >> q;
    while (q--) {
        size_t l = 0, r = 0;
        std::cin >> l >> r;
        std::cout << count_zeros(tree, 0, 0, n - 1, --l, --r) << ' ';
    }
    std::cout.flush();
}
