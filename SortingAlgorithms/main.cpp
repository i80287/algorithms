#include <time.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cinttypes>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <random>
#include <type_traits>
#include <utility>

#include "HeapSort.hpp"
#include "MergeSort.hpp"
#include "QuickSort.hpp"
#include "ShellSort.hpp"

// Source: https://youtu.be/TYHefcf43Bw?si=lLmzr5FQfbpnDzzS
#if defined(_MSC_VER) || defined(__MINGW32__)
#include <intrin.h>  // for _ReadWriteBarrier
static void __declspec(noinline) UseCharPointer(char const volatile*) {}
static inline void noopt(int value) {
    UseCharPointer((char const volatile*)&value);
    _ReadWriteBarrier();
}
#define NOOPT(x) noopt(x)
#else
#define NOOPT(x) asm("" ::"r,i"(x))
#endif

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
}  // namespace arrays_tools

namespace measure_tools {

constexpr size_t kMeasureLimit = 16;

template <class T>
uint64_t MeasureSort(const T* original_array, T* buffer, size_t length,
                     auto lambda)
    requires std::is_scalar_v<T>
{
    timespec t1{};
    timespec t2{};
    uint64_t total_time = 0;
    for (size_t i = kMeasureLimit; i > 0; i--) {
        std::copy_n(original_array, length, buffer);

        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t1);
        lambda(buffer, length);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &t2);

        int j = static_cast<int>(buffer[0]);
        NOOPT(j);

        assert(t2.tv_sec >= t1.tv_sec);
        assert(t2.tv_nsec >= 0);
        assert(t1.tv_nsec >= 0);
        const auto sec_passed =
            static_cast<uint64_t>(t2.tv_sec - t1.tv_sec);

        using unsigned_nsec_t = std::make_unsigned_t<decltype(t2.tv_nsec)>;
        auto nsec_passed      = sec_passed * 1'000'000'000;
        nsec_passed += static_cast<unsigned_nsec_t>(t2.tv_nsec);
        nsec_passed -= static_cast<unsigned_nsec_t>(t1.tv_nsec);
        total_time += nsec_passed;
    }

    return total_time / kMeasureLimit;
}
}  // namespace measure_tools

int main() {
    using T                 = int64_t;
    constexpr size_t length = 1e6;

    std::unique_ptr<T[]> array = std::make_unique<T[]>(length);
    std::unique_ptr<T[]> buffer_for_sorting =
        std::make_unique<T[]>(length);

    arrays_tools::FillArray<T>(&array[0], &array[length], -65536, 65536);
    uint64_t qsort_time             = 0;
    uint64_t std_algo_sort_time     = 0;
    uint64_t inplace_heap_sort_time = 0;
    uint64_t shell_sort_time        = 0;
    uint64_t merge_sort_time        = 0;
    uint64_t quick_sort_time        = 0;

    // count of sort algorithms
    constexpr size_t kNumberOfAlgorithms = 6;
    constexpr size_t kTotalTests         = kNumberOfAlgorithms * 8;

    for (size_t i = 0; i < kTotalTests; i++) {
        const uint64_t qsort_time_res = measure_tools::MeasureSort(
            array.get(), buffer_for_sorting.get(), length,
            [](T* buffer, size_t len) noexcept {
                qsort(
                    buffer, len, sizeof(T),
                    +[](const void* first,
                        const void* second) constexpr noexcept {
                        T a = *static_cast<const T*>(first);
                        T b = *static_cast<const T*>(second);
                        return int(a > b) - int(a < b);
                    });
            });
        const uint64_t std_algo_sort_time_res = measure_tools::MeasureSort(
            array.get(), buffer_for_sorting.get(), length,
            [](T* buffer, size_t len) constexpr noexcept {
                std::sort(&buffer[0], &buffer[len]);
            });
        const uint64_t inplace_heap_sort_time_res =
            measure_tools::MeasureSort(
                array.get(), buffer_for_sorting.get(), length,
                [](T* buffer, size_t len) constexpr noexcept {
                    heap_sort(&buffer[0], len);
                });
        const uint64_t shell_sort_time_res = measure_tools::MeasureSort(
            array.get(), buffer_for_sorting.get(), length,
            [](T* buffer, size_t len) constexpr noexcept {
                algorithms::shell_sort(&buffer[0], len);
            });
        const uint64_t merge_sort_time_res = measure_tools::MeasureSort(
            array.get(), buffer_for_sorting.get(), length,
            [](T* buffer, size_t len) {
                std::stable_sort(&buffer[0], &buffer[len]);
            });
        const uint64_t quick_sort_time_res = measure_tools::MeasureSort(
            array.get(), buffer_for_sorting.get(), length,
            [](T* buffer, size_t len) constexpr noexcept {
                algorithms::quick_sort(&buffer[0], &buffer[len]);
            });

        // clang-format off
        printf(
            "std::sort intro sort average nanoseconds:        %" PRIu64 "\n"
            "qsort quick sort average nanoseconds:            %" PRIu64 "\n"
            "heap_sort inplace heap sort average nanoseconds: %" PRIu64 "\n"
            "std::stable_sort merge sort average nanoseconds: %" PRIu64 "\n"
            "quick_sort quick sort average nanoseconds:       %" PRIu64 "\n"
            "shell_sort Shell sort average nanoseconds:       %" PRIu64 "\n",
            std_algo_sort_time_res,
            qsort_time_res,
            inplace_heap_sort_time_res,
            merge_sort_time_res,
            quick_sort_time_res,
            shell_sort_time_res);
        // clang-format on

        qsort_time += qsort_time_res;
        std_algo_sort_time += std_algo_sort_time_res;
        inplace_heap_sort_time += inplace_heap_sort_time_res;
        shell_sort_time += shell_sort_time_res;
        merge_sort_time += merge_sort_time_res;
        quick_sort_time += quick_sort_time_res;
    }

    // clang-format off
    printf("Results:\n"
        "std::sort intro sort average nanoseconds:         %" PRIu64 "\n"
        "::qsort quick sort average nanoseconds:           %" PRIu64 "\n"
        "::heap_sort inplace heap sort average nanoseconds: %" PRIu64 "\n"
        "std::stable_sort merge sort average nanoseconds:  %" PRIu64 "\n"
        "::QuickSort quick sort average nanoseconds:       %" PRIu64 "\n"
        "::shell_sort Shell sort average nanoseconds:       %" PRIu64 "\n",
        std_algo_sort_time / kTotalTests,
        qsort_time / kTotalTests,
        inplace_heap_sort_time / kTotalTests,
        merge_sort_time / kTotalTests,
        quick_sort_time / kTotalTests,
        shell_sort_time / kTotalTests);
    // clang-format on    
}
