#include <vector>
#include <cstdint>
#include <iostream>

using std::vector;

constexpr uint32_t INITIAL_TIME = 0;
constexpr uint32_t START_TIME = 1;

void fill_tree(
    const vector<uint32_t>& nums,
    vector<uint32_t>& tree,
    vector<uint32_t>& times_tree,
    size_t node_index,
    size_t l,
    size_t r
) {
    if (l < r) {
        size_t middle = (l + r) / 2;
        fill_tree(nums, tree, times_tree, node_index * 2 + 1, l, middle);
        fill_tree(nums, tree, times_tree, node_index * 2 + 2, middle + 1, r);
    } else {
        // l == r
        tree[node_index] = nums[l];
        times_tree[node_index] = INITIAL_TIME;
    }
}

uint32_t get_value_and_its_time(
    const vector<uint32_t>& tree,
    const vector<uint32_t>& times_tree,
    uint32_t& value,
    size_t node_index,
    size_t l,
    size_t r,
    size_t index
) {
    if (l == r) {
        value = tree[node_index];
        return times_tree[node_index];
    }

    size_t middle = (l + r) / 2;

    uint32_t next_value = 0;
    uint32_t next_value_time =
        index <= middle
        ? get_value_and_its_time(tree, times_tree, next_value, node_index * 2 + 1, l, middle, index)
        : get_value_and_its_time(tree, times_tree, next_value, node_index * 2 + 2, middle + 1, r, index);
    
    uint32_t this_value_time = times_tree[node_index];
    uint32_t this_value = tree[node_index];
    if (this_value_time > next_value_time) {
        value = this_value;
        return this_value_time;
    }

    value = next_value;
    return next_value_time;
}

void update_tree(
    vector<uint32_t>& tree,
    vector<uint32_t>& times_tree,
    uint32_t time,
    size_t node_index,
    size_t tree_l,
    size_t tree_r,
    size_t q_l,
    size_t q_r,
    uint32_t value
) {
    if (q_l == tree_l && q_r == tree_r) {
        tree[node_index] = value;
        times_tree[node_index] = time;
        return;
    }

    size_t middle = (tree_l + tree_r) / 2;
    if (q_r <= middle) {
        update_tree(tree, times_tree, time, 2 * node_index + 1, tree_l, middle, q_l, q_r, value);
        return;
    }
    if (middle < q_l) {
        update_tree(tree, times_tree, time, 2 * node_index + 2, middle + 1, tree_r, q_l, q_r, value);
        return;
    }

    // q_l <= middle < q_r
    update_tree(tree, times_tree, time, 2 * node_index + 1, tree_l, middle, q_l, middle, value);
    update_tree(tree, times_tree, time, 2 * node_index + 2, middle + 1, tree_r, middle + 1, q_r, value);
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
    vector<uint32_t> times_tree(4 * n, 0);
    fill_tree(nums, tree, times_tree, 0, 0, n - 1);

    uint32_t m = 0;
    std::cin >> m;
    for (uint32_t time = START_TIME; time <= m; ++time) {
        char q;
        std::cin >> q;
        if (q == 'g') {
            size_t index;
            std::cin >> index;
            uint32_t value = 0;
            get_value_and_its_time(tree, times_tree, value, 0, 0, n - 1, --index);
            std::cout << value << ' ';
        } else {
            size_t l, r;
            uint32_t d;
            std::cin >> l >> r >> d;
            update_tree(tree, times_tree, time, 0, 0, n - 1, --l, --r, d);
        }
    }
    std::cout.flush();
}
