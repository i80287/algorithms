#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <random>

// From https://github.com/OlegSchwann/Technopark-algorithms/blob/master/1-й%20модуль/6%20задание%204%20вариант/main.cpp

// возвращает элементы между left_index и right_index включая концы.
// если left_index == right_index то возвращает его, нельзя допустить деления на 0.
size_t get_pivot_index(size_t left_index, size_t right_index) {
    if(left_index == right_index){
        return left_index;
    }

    size_t pivot_index = left_index + std::rand() % (right_index - left_index);
    return pivot_index;
}

// обратный проход итераторов
template<class stored_type>
size_t Partition2(stored_type* array, size_t left_index, size_t right_index) {
    size_t pivot_index = get_pivot_index(left_index, right_index);
    stored_type pivot = array[pivot_index];

    // Place pivot on the leftest place
    std::swap(array[pivot_index], array[left_index]);

    size_t first_greater = right_index + 1;
    for (auto first_not_considered = right_index; first_not_considered > left_index; --first_not_considered) {
        if (array[first_not_considered] > pivot) {
            --first_greater;
            std::swap(array[first_greater], array[first_not_considered]);
        }
    }

    // All elements in array[lef_index; first_greater - 1] <= pivot
    // All elements in array[first_greater; right_index] > pivot

    // place pivot from the left_index to the first_greater - 1 index
    std::swap(array[first_greater - 1], array[left_index]);

    // first_greater - 1 is a pivot index in the new array
    return first_greater - 1;
}

template<class stored_type>
stored_type ordinal_statistics(stored_type* array, size_t end_index, size_t searched_index) {
    size_t begin_index = 0;
    while (true) {
        size_t found_index = Partition2(array, begin_index, end_index);
        if (begin_index + searched_index == found_index) {
            return array[found_index];
        }
        else if (begin_index + searched_index < found_index) {
            end_index = found_index - 1;
        }
        else {// begin_index + searched_index > found_index

            // searched_index = searched_index - (found_index - begin_index + 1);
            searched_index = begin_index + searched_index - found_index - 1;
            begin_index = found_index + 1;
        }
    }
};


int main() {
    std::srand(static_cast<unsigned int>(std::random_device{}()));
    using T = int64_t;

    constexpr size_t n = 2048;
    std::vector<T> shuffled_array(n);

    std::mt19937 mrs_rnd_engine(std::random_device{}());
    std::uniform_int_distribution<T> dist(-128, 128);
    std::generate(shuffled_array.begin(), shuffled_array.end(), [&dist, &mrs_rnd_engine] {
        return dist(mrs_rnd_engine);
    });

    std::vector<T> sorted_array{shuffled_array};
    std::sort(sorted_array.begin(), sorted_array.end());

    std::vector<T> buffer(n + 1);
    for (size_t i = 0; i < n; i++) {
        buffer = shuffled_array;
        T elem = ordinal_statistics(buffer.data(), n - 1, i);
        assert(elem == sorted_array[i]);
    }

    std::cout << "All tests passed\n";
}
