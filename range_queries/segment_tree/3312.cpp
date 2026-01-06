#include <cstdint>
#include <iostream>
#include <vector>

using std::vector;

void fill_tree(const vector<uint32_t>& nums,
               vector<uint32_t>& max_tree,
               vector<uint32_t>& counts_tree,
               size_t i,
               size_t l,
               size_t r) {
    if (l != r) {
        size_t middle = (l + r) / 2;
        size_t left_son_index = 2 * i + 1;
        size_t right_son_index = 2 * i + 2;
        fill_tree(nums, max_tree, counts_tree, left_son_index, l, middle);
        fill_tree(nums, max_tree, counts_tree, right_son_index, middle + 1, r);

        uint32_t left_max = max_tree[left_son_index];
        uint32_t right_max = max_tree[right_son_index];

        if (left_max > right_max) {
            max_tree[i] = left_max;
            counts_tree[i] = counts_tree[left_son_index];
        } else if (left_max != right_max) {  // left_max < right_max
            max_tree[i] = right_max;
            counts_tree[i] = counts_tree[right_son_index];
        } else {  // left_max == right_max
            max_tree[i] = left_max;
            counts_tree[i] = counts_tree[left_son_index] + counts_tree[right_son_index];
        }
    } else {
        max_tree[i] = nums[l];
        counts_tree[i] = 1;
    }
}

uint32_t find_and_count_max(const vector<uint32_t>& max_tree,
                            const vector<uint32_t>& counts_tree,
                            size_t i,
                            size_t tree_l,
                            size_t tree_r,
                            size_t q_l,
                            size_t q_r,
                            uint32_t& max_count) {
    if (tree_l == q_l && tree_r == q_r) {
        max_count = counts_tree[i];
        return max_tree[i];
    }

    size_t middle = (tree_l + tree_r) / 2;
    if (q_r <= middle) {
        return find_and_count_max(max_tree, counts_tree, 2 * i + 1, tree_l, middle, q_l, q_r, max_count);
    } else if (middle < q_l) {
        return find_and_count_max(max_tree, counts_tree, 2 * i + 2, middle + 1, tree_r, q_l, q_r, max_count);
    }

    // q_l <= middle < q_r
    uint32_t left_max_count = 0;
    uint32_t left_max =
        find_and_count_max(max_tree, counts_tree, 2 * i + 1, tree_l, middle, q_l, middle, left_max_count);
    uint32_t right_max_count = 0;
    uint32_t right_max =
        find_and_count_max(max_tree, counts_tree, 2 * i + 2, middle + 1, tree_r, middle + 1, q_r, right_max_count);
    if (left_max > right_max) {
        max_count = left_max_count;
        return left_max;
    }
    if (left_max != right_max) {
        max_count = right_max_count;
        return right_max;
    }

    max_count = left_max_count + right_max_count;
    return left_max;
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

    vector<uint32_t> max_tree(4 * n, 0);
    vector<uint32_t> counts_tree(4 * n, 0);
    fill_tree(nums, max_tree, counts_tree, 0, 0, n - 1);

    size_t q = 0;
    std::cin >> q;
    while (q--) {
        size_t l = 0, r = 0;
        std::cin >> l >> r;
        uint32_t max_count = 0;
        uint32_t max_elem = find_and_count_max(max_tree, counts_tree, 0, 0, n - 1, --l, --r, max_count);
        std::cout << max_elem << ' ' << max_count << '\n';
    }
    std::cout.flush();
}
