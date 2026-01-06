#include <cassert>
#include <iterator>
#include <random>

#include "SparseTable.cpp"

int main() {
    using T = int64_t;
    constexpr T arr[] = {
        1, -2, -34, -2, 5, -2, 44, 53, 2,  2,  1,  4,  3,  6, 7, 4, 2, 5,  2,  3, 5,
        6, 3,  4,   3,  4, 23, 3,  4,  -2, -1, -1, 23, -3, 0, 0, 1, 3, 21, -1, 2,
    };
    const size_t n = std::size(arr);

    SparseTable<T> sparsetable(arr, n);
    std::mt19937 rnd(std::random_device{}());
    for (size_t k = 1 << 25; k != 0; k--) {
        size_t l = rnd() % n;
        size_t r = rnd() % n;
        if (l > r) {
            std::swap(l, r);
        }

        T res = arr[l];
        for (size_t i = l + 1; i <= r; i++) {
            res = std::min(res, arr[i]);
        }

        assert(sparsetable(l, r) == res);
    }
}
