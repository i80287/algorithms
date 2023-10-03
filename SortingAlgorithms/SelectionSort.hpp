#include <cstdint>
#include <cstdio>
#include <utility>

template <typename T>
static inline constexpr void SelectionSort(T iter, size_t n) {
    for (size_t i = 0; i < n; i++) {
        size_t min_index = i;
        for (size_t j = i + 1; j < n; j++) {
            if (*(iter + j) < *(iter + min_index)) {
                min_index = j;
            }
        }

        std::swap(*(iter + min_index), *(iter + i));
    }
}
