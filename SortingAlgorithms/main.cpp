
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "HeapSort.hpp"
#include "ShellSort.hpp"
#include "MergeSort.hpp"
#include "SelectionSort.hpp"

namespace arrays_tools {
    template <typename T>
    requires std::is_arithmetic_v<T>
    constexpr void FillArray(T* begin, T* end, T left_border, T right_border) {
        if (left_border > right_border) {
            std::swap(left_border, right_border);
        }

        std::mt19937 mrs_rnd_engine(std::random_device{}());
        std::uniform_int_distribution<T> dist(left_border, right_border);
        std::generate(begin, end, [&dist, &mrs_rnd_engine] {
            return dist(mrs_rnd_engine);
        });
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    void CheckSortedArray(const T* array, size_t length) noexcept(false) {
        for (const T* it = array + 1, *end = array + length; it < end; ++it) {
            if (*(it - 1) > *it) {
                throw std::range_error("array was not sorted correctly");
            }
        }
    }
} // namespace arrays_tools

namespace measure_tools {
    constexpr size_t MEASURE_LIMIT = 16;

    template <typename T>
    requires std::is_arithmetic_v<T>
    uint64_t MeasureQSort(const T* original_array, T* buffer, size_t length) {
        uint64_t total_time = 0;

        for (size_t i = MEASURE_LIMIT; i != 0; i--) {
            std::memcpy(buffer, original_array, length * sizeof(T));

            auto start = std::chrono::steady_clock::now();
            qsort(buffer, length, sizeof(T), [](const void* first, const void* second) constexpr noexcept -> int {
                T a = *static_cast<const T*>(first);
                T b = *static_cast<const T*>(second);
                return (a != b) - ((a < b) << 1);
            });
            auto end = std::chrono::steady_clock::now();
            std::chrono::nanoseconds delta = end - start;
            total_time += delta.count();

            arrays_tools::CheckSortedArray(buffer, length);
        }

        uint64_t res = total_time / MEASURE_LIMIT;
        std::cout << "::qsort quick sort average nanoseconds: " << res << '\n';
        return res;
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    uint64_t MeasureHeapSort(const T* original_array, T* buffer, size_t length) {
        uint64_t total_time = 0;

        for (size_t i = MEASURE_LIMIT; i != 0; i--) {
            std::memcpy(buffer, original_array, length * sizeof(T));

            auto start = std::chrono::steady_clock::now();
            HeapSort(&buffer[0], length);
            auto end = std::chrono::steady_clock::now();
            std::chrono::nanoseconds delta = end - start;
            total_time += delta.count();

            arrays_tools::CheckSortedArray(buffer, length);
        }

        uint64_t res = total_time / MEASURE_LIMIT;
        std::cout << "::HeapSort heap sort average nanoseconds: " << res << '\n';
        return res;
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    uint64_t MeasureStdSort(const T* original_array, T* buffer, size_t length) {
        uint64_t total_time = 0;

        for (size_t i = MEASURE_LIMIT; i != 0; i--) {
            std::memcpy(buffer, original_array, length * sizeof(T));

            auto start = std::chrono::steady_clock::now();
            std::sort(&buffer[0], &buffer[length]);
            auto end = std::chrono::steady_clock::now();
            std::chrono::nanoseconds delta = end - start;
            total_time += delta.count();

            arrays_tools::CheckSortedArray(buffer, length);
        }

        uint64_t res = total_time / MEASURE_LIMIT;
        std::cout << "std::sort intro sort average nanoseconds: " << res << '\n';
        return res;
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    uint64_t MeasureShellSort(const T* original_array, T* buffer, size_t length) {
        uint64_t total_time = 0;

        for (size_t i = MEASURE_LIMIT; i != 0; i--) {
            std::memcpy(buffer, original_array, length * sizeof(T));

            auto start = std::chrono::steady_clock::now();
            ShellSort(&buffer[0], length);
            auto end = std::chrono::steady_clock::now();
            std::chrono::nanoseconds delta = end - start;
            total_time += delta.count();

            arrays_tools::CheckSortedArray(buffer, length);
        }

        uint64_t res = total_time / MEASURE_LIMIT;
        std::cout << "::ShellSort Shell sort average nanoseconds: " << res << '\n';
        return res;
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    uint64_t MeasureMergeSort(const T* original_array, T* buffer, size_t length) {
        uint64_t total_time = 0;

        for (size_t i = MEASURE_LIMIT; i != 0; i--) {
            std::memcpy(buffer, original_array, length * sizeof(T));

            auto start = std::chrono::steady_clock::now();
            MergeSort(&buffer[0], length);
            auto end = std::chrono::steady_clock::now();
            std::chrono::nanoseconds delta = end - start;
            total_time += delta.count();

            arrays_tools::CheckSortedArray(buffer, length);
        }

        uint64_t res = total_time / MEASURE_LIMIT;
        std::cout << "::MergeSort merge sort average nanoseconds: " << res << '\n';
        return res;
    }

    template <typename T>
    requires std::is_arithmetic_v<T>
    uint64_t MeasureSelectionSort(const T* original_array, T* buffer, size_t length) {
        uint64_t total_time = 0;

        for (size_t i = MEASURE_LIMIT; i != 0; i--) {
            std::memcpy(buffer, original_array, length * sizeof(T));

            auto start = std::chrono::steady_clock::now();
            SelectionSort(&buffer[0], length);
            auto end = std::chrono::steady_clock::now();
            std::chrono::nanoseconds delta = end - start;
            total_time += delta.count();

            arrays_tools::CheckSortedArray(buffer, length);
        }

        uint64_t res = total_time / MEASURE_LIMIT;
        std::cout << "::SelectionSort selection sort average nanoseconds: " << res << '\n';
        return res;
    }
} // namespace measure_tools

int main(void) {
    using T = int64_t;
    constexpr size_t length = 5e4;

    std::unique_ptr<T[]> array = std::make_unique<T[]>(length);
    std::unique_ptr<T[]> buffer_for_sorting = std::make_unique<T[]>(length);

    arrays_tools::FillArray<T>(&array[0], &array[length], -65536, 65536);
    uint64_t qsort_time = 0;
    uint64_t std_algo_sort_time = 0;
    uint64_t inplace_heap_sort_time = 0;
    uint64_t shell_sort_time = 0;
    uint64_t merge_sort_time = 0;
    uint64_t selection_sort_time = 0;

    // count of sort algorithms
    constexpr size_t K = 6; 
    constexpr size_t TOTAL_TESTS = 8 * K;
    for (size_t i = 0; i != TOTAL_TESTS; ++i) {
        std::cout << "\n\nTest " << i + 1 << '\n';
        switch (i % K) {
        case 0:
            qsort_time += measure_tools::MeasureQSort(array.get(), buffer_for_sorting.get(), length);
            std_algo_sort_time += measure_tools::MeasureStdSort(array.get(), buffer_for_sorting.get(), length);
            inplace_heap_sort_time += measure_tools::MeasureHeapSort(array.get(), buffer_for_sorting.get(), length);
            shell_sort_time += measure_tools::MeasureShellSort(array.get(), buffer_for_sorting.get(), length);
            merge_sort_time += measure_tools::MeasureMergeSort(array.get(), buffer_for_sorting.get(), length);
            selection_sort_time += measure_tools::MeasureSelectionSort(array.get(), buffer_for_sorting.get(), length);
            break;
        case 1:
            selection_sort_time += measure_tools::MeasureSelectionSort(array.get(), buffer_for_sorting.get(), length);
            qsort_time += measure_tools::MeasureQSort(array.get(), buffer_for_sorting.get(), length);
            std_algo_sort_time += measure_tools::MeasureStdSort(array.get(), buffer_for_sorting.get(), length);
            inplace_heap_sort_time += measure_tools::MeasureHeapSort(array.get(), buffer_for_sorting.get(), length);
            shell_sort_time += measure_tools::MeasureShellSort(array.get(), buffer_for_sorting.get(), length);
            merge_sort_time += measure_tools::MeasureMergeSort(array.get(), buffer_for_sorting.get(), length);
            break;
        case 2:
            merge_sort_time += measure_tools::MeasureMergeSort(array.get(), buffer_for_sorting.get(), length);
            selection_sort_time += measure_tools::MeasureSelectionSort(array.get(), buffer_for_sorting.get(), length);
            qsort_time += measure_tools::MeasureQSort(array.get(), buffer_for_sorting.get(), length);
            std_algo_sort_time += measure_tools::MeasureStdSort(array.get(), buffer_for_sorting.get(), length);
            inplace_heap_sort_time += measure_tools::MeasureHeapSort(array.get(), buffer_for_sorting.get(), length);
            shell_sort_time += measure_tools::MeasureShellSort(array.get(), buffer_for_sorting.get(), length);
            break;
        case 3:
            shell_sort_time += measure_tools::MeasureShellSort(array.get(), buffer_for_sorting.get(), length);
            merge_sort_time += measure_tools::MeasureMergeSort(array.get(), buffer_for_sorting.get(), length);
            selection_sort_time += measure_tools::MeasureSelectionSort(array.get(), buffer_for_sorting.get(), length);
            qsort_time += measure_tools::MeasureQSort(array.get(), buffer_for_sorting.get(), length);
            std_algo_sort_time += measure_tools::MeasureStdSort(array.get(), buffer_for_sorting.get(), length);
            inplace_heap_sort_time += measure_tools::MeasureHeapSort(array.get(), buffer_for_sorting.get(), length);
            break;
        case 4:
            inplace_heap_sort_time += measure_tools::MeasureHeapSort(array.get(), buffer_for_sorting.get(), length);
            shell_sort_time += measure_tools::MeasureShellSort(array.get(), buffer_for_sorting.get(), length);
            merge_sort_time += measure_tools::MeasureMergeSort(array.get(), buffer_for_sorting.get(), length);
            selection_sort_time += measure_tools::MeasureSelectionSort(array.get(), buffer_for_sorting.get(), length);
            qsort_time += measure_tools::MeasureQSort(array.get(), buffer_for_sorting.get(), length);
            std_algo_sort_time += measure_tools::MeasureStdSort(array.get(), buffer_for_sorting.get(), length);
            break;
        case 5:
            static_assert(K == 6, "add more test cases");
            std_algo_sort_time += measure_tools::MeasureStdSort(array.get(), buffer_for_sorting.get(), length);
            inplace_heap_sort_time += measure_tools::MeasureHeapSort(array.get(), buffer_for_sorting.get(), length);
            shell_sort_time += measure_tools::MeasureShellSort(array.get(), buffer_for_sorting.get(), length);
            merge_sort_time += measure_tools::MeasureMergeSort(array.get(), buffer_for_sorting.get(), length);
            selection_sort_time += measure_tools::MeasureSelectionSort(array.get(), buffer_for_sorting.get(), length);
            qsort_time += measure_tools::MeasureQSort(array.get(), buffer_for_sorting.get(), length);
            break;
        }
    }

    std::cout
        << "\nResults:"
        << "\n::qsort quick sort average nanoseconds:         " << qsort_time / TOTAL_TESTS
        << "\nstd::sort intro sort average nanoseconds:       " << std_algo_sort_time / TOTAL_TESTS
        << "\n::HeapSort heap sort average nanoseconds:       " << inplace_heap_sort_time / TOTAL_TESTS
        << "\n::ShellSort Shell sort average nanoseconds:     " << shell_sort_time / TOTAL_TESTS
        << "\n::MergeSort Shell sort average nanoseconds:     " << merge_sort_time / TOTAL_TESTS
        << "\n::SelectionSort Shell sort average nanoseconds: " << selection_sort_time / TOTAL_TESTS;
}

// g++ main.cpp -std=c++2b -fconcepts-diagnostics-depth=200 -O3 -march=native -Wall -Wextra -Wpedantic -Werror -Wunused --pedantic-errors -Wconversion -Warith-conversion -Wshadow -Warray-bounds=2 -Wnull-dereference -Wcast-align=strict -o tmp.exe
