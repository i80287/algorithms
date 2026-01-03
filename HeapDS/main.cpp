#include <cstdint>
#include <functional>
#include <iostream>
#include <type_traits>
#include <vector>

#if __cplusplus > 201703L && __cpp_concepts >= 201907L
#include <concepts>
#endif

template <typename T, typename Comparator = std::less<T>>
#if __cplusplus > 201703L && __cpp_concepts >= 201907L
    requires requires(Comparator comp, T a, T b) {
        { comp(a, b) } -> std::integral;
        { comp(a, b) } -> std::convertible_to<bool>;
    }
#endif
class Heap {
    static constexpr bool kUseByValue = std::is_standard_layout_v<T> && std::is_trivially_copyable_v<T> &&
                                        std::is_trivially_copy_constructible_v<T> &&
                                        std::is_trivially_copy_assignable_v<T> &&
                                        std::is_nothrow_copy_assignable_v<T> && std::is_nothrow_copy_constructible_v<T>;

public:
    constexpr Heap() noexcept(noexcept(std::vector<T>{})) : heap_elements_{}, comp_{} {}

    constexpr explicit Heap(const Comparator& comparator) noexcept(noexcept(std::vector<T>{}))
        : heap_elements_{}, comp_{comparator} {}

    constexpr void reserve(size_t size) {
        heap_elements_.reserve(size);
    }

    constexpr size_t size() const noexcept {
        return heap_elements_.size();
    }

    constexpr bool empty() const noexcept {
        return heap_elements_.empty();
    }

    constexpr T& top() noexcept {
        return heap_elements_.front();
    }

    constexpr const T& top() const noexcept {
        return heap_elements_.front();
    }

    constexpr const std::vector<T>& nodes() const noexcept {
        return heap_elements_;
    }

    constexpr std::vector<T>& nodes() noexcept {
        return heap_elements_;
    }

    /// @brief Element is passed by value intentionally
    /// @param element
    void push(T element) {
        // push elem to the end of the heap and sift upper.

        size_t elem_index = size();
        size_t parent_index = (elem_index - 1) >> 1;
        heap_elements_.emplace_back();

        while (elem_index != 0 && !Compare(heap_elements_[parent_index], element)) {
            heap_elements_[elem_index] = std::move(heap_elements_[parent_index]);
            elem_index = parent_index;
            parent_index = (elem_index - 1) >> 1;
        }

        heap_elements_[elem_index] = std::move(element);
    }

    void pop_top() {
        if constexpr (kUseByValue) {
            const T sifting_elem = heap_elements_.front() = heap_elements_.back();
            for (size_t parent_index = 0, son_index = 1, len = size(); son_index + 1 < len;) {
                if (!Compare(heap_elements_[son_index], heap_elements_[son_index + 1])) {
                    son_index++;
                }

                if (Compare(sifting_elem, heap_elements_[son_index])) {
                    break;
                }
                heap_elements_[parent_index] = heap_elements_[son_index];
                heap_elements_[son_index] = sifting_elem;
                parent_index = son_index;
                son_index = parent_index * 2 | 1;
            }

            heap_elements_.pop_back();
        } else {
            heap_elements_[0] = std::move(heap_elements_.back());
            heap_elements_.pop_back();
            for (size_t parent_index = 0, son_index = 1, len = size(); son_index < len;) {
                if (son_index + 1 < len && !Compare(heap_elements_[son_index], heap_elements_[son_index + 1])) {
                    son_index++;
                }
                if (Compare(heap_elements_[parent_index], heap_elements_[son_index])) {
                    break;
                }
                std::swap(heap_elements_[parent_index], heap_elements_[son_index]);
                parent_index = son_index;
                son_index = son_index * 2 | 1;
            }
        }
    }

protected:
    constexpr bool Compare(const T& node1, const T& node2) const {
        if constexpr (std::is_same_v<decltype(comp_(node1, node2)), bool>) {
            return comp_(node1, node2);
        } else {
            return comp_(node1, node2) <= 0;
        }
    }

    template <class ValueType>
    constexpr void decrease_key(size_t node_index, ValueType value) noexcept {
        heap_elements_[node_index].value = value;
        rebalance_heap(node_index);
    }

    constexpr void rebalance_heap(size_t index) noexcept {
        size_t son_index = index * 2 | 1;
        const size_t len = size();
        if (son_index >= len) {
            return;
        }

        if constexpr (kUseByValue) {
            size_t parent_index = index;
            const T sifting_elem = heap_elements_[index];
            if (son_index + 1 != size() && !Compare(heap_elements_[son_index], heap_elements_[son_index + 1])) {
                son_index++;
            }

            while (!Compare(sifting_elem, heap_elements_[son_index])) {
                heap_elements_[parent_index] = heap_elements_[son_index];
                heap_elements_[son_index] = sifting_elem;

                parent_index = son_index;
                son_index = son_index * 2 | 1;
                if (son_index >= len) {
                    return;
                }

                if (son_index + 1 != len && !Compare(heap_elements_[son_index], heap_elements_[son_index + 1])) {
                    son_index++;
                }
            }
        }
    }

    std::vector<T> heap_elements_;
    Comparator comp_;
};

template <typename T>
using MinHeap = Heap<T, std::less_equal<T>>;

template <typename T>
using MaxHeap = Heap<T, std::greater_equal<T>>;

#include <cassert>
#include <queue>

int main() {
    struct TestWrapper {
        int64_t value_;

        constexpr TestWrapper() noexcept : value_{0} {}

        constexpr TestWrapper(int64_t value) noexcept : value_{value} {}

        constexpr operator int64_t() const noexcept {
            return value_;
        }
    };

    MaxHeap<TestWrapper> heap;
    std::priority_queue<TestWrapper, std::vector<TestWrapper>, std::less<TestWrapper>> correct_heap;
    heap.reserve(16);
    for (int value : {4, 6, 42, 3, 10, 42, 2, 243, 42, 1242, 243, 23, 42}) {
        heap.push(value);
        correct_heap.push(value);
    }
    assert(heap.size() == correct_heap.size());
    while (!heap.empty()) {
        std::cout << "Current top: " << static_cast<int64_t>(heap.top()) << '\n';
        assert(heap.top() == correct_heap.top());
        heap.pop_top();
        correct_heap.pop();
    }

    return 0;
}

namespace unsafe_static_heap {

constexpr size_t N = 100000;
static int64_t heap[N];

inline void heap_add_elem(const int64_t elem, size_t& heap_size);
inline void heap_delete_top(size_t& heap_size);

int main0(void) {
    size_t n = 0;
    std::cin >> n;

    size_t heap_size = 0;
    for (size_t i = 0; i != n; ++i) {
        int cmd;
        std::cin >> cmd;
        if (cmd == 0) {
            int64_t elem;
            std::cin >> elem;
            heap_add_elem(elem, heap_size);
        } else {
            std::cout << heap[0] << '\n';
            heap_delete_top(heap_size);
        }
    }

    return 0;
}

inline void heap_add_elem(const int64_t elem,
                          size_t& heap_size) {  // push elem to the end of the heap and sift upper.
    size_t elem_index = heap_size;
    size_t parent_index = (elem_index - 1) >> 1;
    while (elem_index != 0 && elem > heap[parent_index]) {
        heap[elem_index] = heap[parent_index];
        elem_index = parent_index;
        parent_index = (elem_index - 1) >> 1;
    }

    heap[elem_index] = elem;
    ++heap_size;
}

inline void heap_delete_top(size_t& heap_size) {
    if (heap_size == 1) {
        heap_size = 0;
        return;
    }
    const int64_t sifting_elem = heap[--heap_size];
    heap[0] = sifting_elem;

    // Back pyramide (heap) to the balance state.
    size_t parent_index = 0;
    size_t son_index = 1;
    int64_t son_elem = heap[1];
    if (heap_size > 2 && son_elem < heap[2]) {  // i = current pyramid size.
        son_elem = heap[2];
        son_index = 2;
    }

    while (sifting_elem < son_elem) {
        heap[parent_index] = heap[son_index];
        heap[son_index] = sifting_elem;

        parent_index = son_index;
        son_index = (son_index << 1) + 1;
        if (son_index >= heap_size) {
            break;
        }

        son_elem = heap[son_index];
        if (son_index + 1 != heap_size && son_elem < heap[son_index + 1]) {
            son_elem = heap[++son_index];
        }
    }
}

}  // namespace unsafe_static_heap
