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
requires requires (Comparator comp, T a, T b) {
    { comp(a, b) } -> std::integral;
    { comp(a, b) } -> std::convertible_to<bool>;
}
#endif
class Heap {
public:
    constexpr Heap() noexcept(noexcept(std::vector<T>{}))
        : heap_elements_{}, comp_{} {
    }

    constexpr explicit Heap(Comparator comparator) noexcept(noexcept(std::vector<T>{}))
        : heap_elements_{}, comp_{comparator} {
    }

    constexpr void Reserve(size_t size) {
        heap_elements_.reserve(size);
    }

    constexpr size_t Size() const noexcept {
        return size();
    }

    constexpr size_t size() const noexcept {
        return heap_elements_.size();
    }

    constexpr bool Empty() const noexcept {
        return empty();
    }

    constexpr bool empty() const noexcept {
        return heap_elements_.empty();
    }

    constexpr T& Top() noexcept {
        return heap_elements_.front();
    }

    constexpr const T& Top() const noexcept {
        return heap_elements_.front();
    }

    constexpr const std::vector<T>& Nodes() const noexcept {
        return heap_elements_;
    }

    constexpr std::vector<T>& Nodes() noexcept {
        return heap_elements_;
    }

    void Add(const T& element) {
        // Add elem to the end of the heap and sift upper.

        size_t elem_index = size();
        size_t parent_index = (elem_index - 1) >> 1;
        heap_elements_.emplace_back();

        while (elem_index != 0 && !Compare(heap_elements_[parent_index], element)) {
            heap_elements_[elem_index] = std::move(heap_elements_[parent_index]);
            elem_index = parent_index;
            parent_index = (elem_index - 1) >> 1;
        }

        heap_elements_[elem_index] = element;
    }

    void Add(T&& element) {
        // Add elem to the end of the heap and sift upper.

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

    void PopTop() {
        switch(size()) {
            case 1:
                heap_elements_.pop_back();
                [[fallthrough]];
            case 0:
                return;
        }

        if constexpr (std::is_trivial_v<T>) {
            const T sifting_elem = heap_elements_.back();
            heap_elements_.pop_back();

            // Back pyramide (heap) to the balance state.

            constexpr size_t TopIndex = 0;
            size_t parent_index = TopIndex;
            heap_elements_[TopIndex] = sifting_elem;

            constexpr size_t FirstSonInitIndex = 1;
            constexpr size_t SecondSonInitIndex = 2;
            size_t son_index = FirstSonInitIndex;
            T son_elem = heap_elements_[FirstSonInitIndex];
            if (SecondSonInitIndex < size() && !Compare(son_elem, heap_elements_[SecondSonInitIndex])) {
                son_elem = heap_elements_[SecondSonInitIndex];
                son_index = SecondSonInitIndex;
            }

            while (!Compare(sifting_elem, son_elem)) {
                heap_elements_[parent_index] = heap_elements_[son_index];
                heap_elements_[son_index] = sifting_elem;

                parent_index = son_index;
                son_index = (son_index << 1) + 1;
                if (son_index >= size()) {
                    break;
                }

                son_elem = heap_elements_[son_index];
                if (son_index + 1 != size() && !Compare(son_elem, heap_elements_[son_index + 1])) {
                    son_elem = heap_elements_[++son_index];
                }
            }
        }
        else {
            constexpr size_t TopIndex = 0;
            size_t parent_index = TopIndex;
            heap_elements_[TopIndex] = std::move(heap_elements_.back());
            heap_elements_.pop_back();

            constexpr size_t FirstSonInitIndex = 1;
            constexpr size_t SecondSonInitIndex = 2;
            size_t son_index = FirstSonInitIndex;
            if (SecondSonInitIndex < size() && !Compare(heap_elements_[FirstSonInitIndex], heap_elements_[SecondSonInitIndex])) {
                son_index = SecondSonInitIndex;
            }

            while (!Compare(heap_elements_[parent_index], heap_elements_[son_index])) {
                std::swap(heap_elements_[parent_index], heap_elements_[son_index]);

                parent_index = son_index;
                son_index = (son_index << 1) + 1;
                if (son_index >= size()) {
                    break;
                }

                if (son_index + 1 != size() && !Compare(heap_elements_[son_index], heap_elements_[son_index + 1])) {
                    ++son_index;
                }
            }
        }
    }
protected:
    inline constexpr bool Compare(const T& node1, const T& node2) const {
        if constexpr (std::is_same_v<decltype(comp_(node1, node2)), bool>) {
            return comp_(node1, node2);
        }
        else {
            return comp_(node1, node2) <= 0;
        }
    }

    std::vector<T> heap_elements_;
    Comparator comp_;
};

template <typename T>
using MinHeap = Heap<T, std::less_equal<T>>;

template <typename T>
using MaxHeap = Heap<T, std::greater_equal<T>>;

int main() {
    struct test_wrapper {
        int64_t value_;

        constexpr test_wrapper() noexcept : value_{0} {
        }

        constexpr test_wrapper(int64_t value) noexcept : value_{value} {
        }

        constexpr operator int64_t() const noexcept {
            return value_;
        }
    };

    MaxHeap<test_wrapper> heap;
    heap.Reserve(16);

    heap.Add(4);
    heap.Add(6);
    heap.Add(42);
    heap.Add(3);
    heap.Add(10);
    heap.Add(42);
    heap.Add(2);
    heap.Add(243);
    heap.Add(42);
    heap.Add(1242);
    heap.Add(243);
    heap.Add(23);
    heap.Add(42);

    while (!heap.empty()) {
        std::cout << "Current top: " << static_cast<int64_t>(heap.Top()) << '\n';
        heap.PopTop();
    }

    return 0;
}

namespace unsafe_static_heap {

constexpr size_t N = 100000;
static int64_t heap[N];

inline void heap_add_elem(const int64_t elem, size_t &heap_size);
inline void heap_delete_top(size_t &heap_size);

int main0(void) {
    size_t n = 0;
    std::cin >> n;

    size_t heap_size = 0;
    for (size_t i = 0; i != n; ++i)
    {
        int cmd;
        std::cin >> cmd;
        if (cmd == 0)
        {
            int64_t elem;
            std::cin >> elem;
            heap_add_elem(elem, heap_size);
        }
        else
        {
            std::cout << heap[0] << '\n';
            heap_delete_top(heap_size);
        }
    }

    return 0;
}

inline void heap_add_elem(const int64_t elem, size_t &heap_size)
{// Add elem to the end of the heap and sift upper.
    size_t elem_index = heap_size;
    size_t parent_index = (elem_index - 1) >> 1;
    while (elem_index != 0 && elem > heap[parent_index])
    {
        heap[elem_index] = heap[parent_index];
        elem_index = parent_index;
        parent_index = (elem_index - 1) >> 1;
    }

    heap[elem_index] = elem;
    ++heap_size;
}

inline void heap_delete_top(size_t &heap_size)
{
    if (heap_size == 1)
    {
        heap_size = 0;
        return;
    }
    const int64_t sifting_elem = heap[--heap_size];
    heap[0] = sifting_elem;

    // Back pyramide (heap) to the balance state.
    size_t parent_index = 0;
    size_t son_index = 1;
    int64_t son_elem = heap[1];
    if (heap_size > 2 && son_elem < heap[2])
    {// i = current pyramid size.
        son_elem = heap[2];
        son_index = 2;
    }

    while (sifting_elem < son_elem)
    {
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

}