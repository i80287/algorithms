#ifndef HEAP_SORT_HPP
#define HEAP_SORT_HPP 1

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <utility>

template <typename T>
#if __cplusplus >= 202002L
requires std::is_arithmetic_v<T>
constexpr
#endif
static inline void heapify(T* arr, size_t length) noexcept {
    for (size_t i = (length >> 1) - 1; i != static_cast<size_t>(-1); --i) {
        size_t parent_index = i;
        T elem = arr[parent_index];
        
        size_t son_index = (parent_index << 1) + 1;
        T son = arr[son_index];
        if (son_index + 1 < length && son < arr[son_index + 1]) {
            son = arr[++son_index];
        }

        while (elem < son) {
            arr[parent_index] = son;
            arr[son_index] = elem;
            
            parent_index = son_index;
            son_index = (parent_index << 1) + 1;
            if (son_index >= length) {
                break;
            }

            son = arr[son_index];
            if (son_index + 1 < length && son < arr[son_index + 1]) {
                son = arr[++son_index];
            }
        }
    }
}

template <typename T>
#if __cplusplus >= 202002L
requires std::is_arithmetic_v<T>
constexpr
#endif
inline void HeapSort(T* arr, size_t length) noexcept {
    if (length <= 1)
    {// (length & ~((size_t)1)) == 0
        return;
    }

    heapify(arr, length);

    for (size_t i = length - 1; i != 1; --i) {
        // Pop max elem from the top of the pyramid and add to the end.
        T sifting_elem = arr[i];
        arr[i] = arr[0];
        arr[0] = sifting_elem;

        // Back pyramide (heap) to the balance state.
        size_t parent_index = 0;
        size_t son_index = 1;
        T son = arr[1];
        if (i > 2 && son < arr[2])
        {// i = current pyramid size.
            son = arr[2];
            son_index = 2;
        }

        while (sifting_elem < son) {
            arr[parent_index] = son;
            arr[son_index] = sifting_elem;
            
            parent_index = son_index;
            son_index = (son_index << 1) + 1;
            if (son_index >= i) {
                break;
            }

            son = arr[son_index];
            if (son_index + 1 != i && son < arr[son_index + 1]) {
                son = arr[++son_index];
            }
        }
    }

    // Swap [0] and [1] elements because for cycle [from length - 1 to 2].
    T tmp = arr[0];
    arr[0] = arr[1];
    arr[1] = tmp;
}

template <typename T, typename SizeType>
#if __cplusplus >= 202002L
requires std::is_arithmetic_v<T> && std::is_integral_v<SizeType>
constexpr
#endif
static inline void heapify(T* arr, size_t length, SizeType* indexes) noexcept {
    for (size_t i = (length >> 1) - 1; i != static_cast<size_t>(-1); --i) {
        size_t parent_index = i;
        T elem = arr[parent_index];
        
        size_t son_index = (parent_index << 1) + 1;
        T son = arr[son_index];
        if (son_index + 1 < length && son < arr[son_index + 1]) {
            son = arr[++son_index];
        }

        while (elem < son) {
            arr[parent_index] = son;
            arr[son_index] = elem;

            std::swap(indexes[parent_index], indexes[son_index]);
            
            parent_index = son_index;
            son_index = (parent_index << 1) + 1;
            if (son_index >= length) {
                break;
            }

            son = arr[son_index];
            if (son_index + 1 < length && son < arr[son_index + 1]) {
                son = arr[++son_index];
            }
        }
    }
}

template <typename T, typename SizeType>
#if __cplusplus >= 202002L
requires std::is_arithmetic_v<T> && std::is_integral_v<SizeType>
constexpr
#endif
inline void HeapSort(T* arr, size_t length, SizeType* indexes) noexcept {
    if (length <= 1)
    {// (length & ~((size_t)1)) == 0
        return;
    }

    heapify(arr, length, indexes);

    for (size_t i = length - 1; i != 1; --i) {
        // Pop max elem from the top of the pyramid and add to the end.
        T sifting_elem = arr[i];
        arr[i] = arr[0];
        arr[0] = sifting_elem;

        std::swap(indexes[0], indexes[i]);

        // Back pyramide (heap) to the balance state.
        size_t parent_index = 0;
        size_t son_index = 1;
        T son = arr[1];
        if (i > 2 && son < arr[2])
        {// i = current pyramid size.
            son = arr[2];
            son_index = 2;
        }

        while (sifting_elem < son) {
            arr[parent_index] = son;
            arr[son_index] = sifting_elem;

            std::swap(indexes[parent_index], indexes[son_index]);
            
            parent_index = son_index;
            son_index = (son_index << 1) + 1;
            if (son_index >= i) {
                break;
            }

            son = arr[son_index];
            if (son_index + 1 != i && son < arr[son_index + 1]) {
                son = arr[++son_index];
            }
        }
    }

    // Swap [0] and [1] elements because for cycle [from length - 1 to 2].
    T tmp = arr[0];
    arr[0] = arr[1];
    arr[1] = tmp;
    std::swap(indexes[0], indexes[1]);
}

#include <functional>
#include <utility>

template <class Iterator, class Comparator>
static inline void Heapify(Iterator begin, size_t length, Comparator comparator) {
    for (size_t i = (length >> 1) - 1; i != static_cast<size_t>(-1); --i) {
        size_t parent_index = i;

        size_t son_index = (parent_index << 1) + 1;
        if (son_index + 1 < length && comparator(*(begin + son_index), *(begin + son_index + 1))) {
            ++son_index;
        }

        while (comparator(*(begin + parent_index), *(begin + son_index))) {
            std::swap(*(begin + parent_index), *(begin + son_index));

            parent_index = son_index;
            son_index = (parent_index << 1) + 1;
            if (son_index >= length) {
                break;
            }

            if (son_index + 1 < length && comparator(*(begin + son_index), *(begin + son_index + 1))) {
                ++son_index;
            }
        }
    }
}

template <class Iterator, class Comparator = std::less<typename std::iterator_traits<Iterator>::value_type>>
void HeapSort(Iterator begin, Iterator end, Comparator comparator = Comparator()) {
    size_t length = static_cast<size_t>(end - begin);
    if (length <= 1) {
        return;
    }

    Heapify(begin, length, comparator);

    for (size_t i = length - 1; i != 1; --i) {
        std::swap(*(begin + i), *begin);

        size_t parent_index = 0;
        size_t son_index = 1;
        constexpr size_t SecondSonIndex = 2;
        if (i > SecondSonIndex && comparator(*(begin + son_index), *(begin + SecondSonIndex))) {
            son_index = SecondSonIndex;
        }

        while (comparator(*(begin + parent_index), *(begin + son_index))) {
            std::swap(*(begin + parent_index), *(begin + son_index));

            parent_index = son_index;
            son_index = (son_index << 1) + 1;
            if (son_index >= i) {
                break;
            }

            if (son_index + 1 != i && comparator(*(begin + son_index), *(begin + son_index + 1))) {
                ++son_index;
            }
        }
    }

    // Swap [0] and [1] elements because for cycle runs from length - 1 to 2.
    std::swap(*begin, *(begin + 1));
}


#endif // HEAP_SORT_HPP
