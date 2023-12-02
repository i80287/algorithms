#pragma once

#include <cstdint>
#include <utility>

template <typename T>
constexpr void InsertionSort(T iter, size_t n) {
    for (size_t i = 1; i < n; i++) {
        auto tmp = std::move(*(iter + i));
        size_t j = i;
        for (; j != 0 && *(iter + (j - 1)) > tmp; j--) {
            *(iter + j) = std::move(*(iter + (j - 1)));
        }
        *(iter + j) = std::move(tmp);
    }
}
