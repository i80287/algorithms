#include <cassert>
#include <cstdint>
#include <cstdio>
#include <utility>

template <typename T>
static void merge(T arr_iter, size_t left, size_t mid, size_t right) {
    // индекс начала второй "половины" для слияния
    size_t left_2 = mid + 1;
    // половины уже отсортированы
    if (arr_iter[mid] <= arr_iter[left_2]) {
        return;
    }

    while (left <= mid && left_2 <= right) {
        // первые элементы в правильном порядке
        if (arr_iter[left] <= arr_iter[left_2]) {
            left++;
        }
        else {
            // вставляем на правильное место left элемент arr_iter[left_2]
            // запоминаем его и его индекс
            auto value = std::move(arr_iter[left_2]);
            size_t ind = left_2;

            // сдвигаем все элементы между left и left_2 вправо
            // и записываем на место left элемент arr_iter[left_2]
            while (ind != left) {
                arr_iter[ind] = std::move(arr_iter[ind - 1]);
                ind--;
            }
            arr_iter[left] = std::move(value);

            // передвигаем все рабочие индексы
            left++;
            mid++;
            left_2++;
        }
    }
}

// основной метод сортировки
template <typename T>
static void mergeSort(T arr_iter, size_t left, size_t right) {
    if (left < right) {
        size_t mid = left + (right - left) / 2;

        mergeSort(arr_iter, left, mid);
        mergeSort(arr_iter, mid + 1, right);

        merge(arr_iter, left, mid, right);
    }
}

template <typename T>
static inline void MergeSort(T arr_iter, size_t length) {
    if (length != 0) {
        mergeSort(arr_iter, 0, length - 1);
    }
}
