#include <cstdint>
#include <cstddef>

/// @brief let i := return index
///     if (num \in container) => (
///         i < container.size() && container[i] == num
///         && if (i + 1 != container.size()) => (
///             num < container[i + 1]
///         )
///     )
///     else if (!container.empty() && num > container[0]) => (
///         i < container.size() && container[i] < num
///         && if (i + 1 != container.size()) => (
///             num < container[i + 1]
///         )
///     )
///     else => (
///         i == size_t(-1)
///     )
/// @tparam Container 
/// @tparam T 
/// @param container 
/// @param value 
/// @return i
template <class Container, class T>
static size_t find_rightest(const Container& container, T value) {
    size_t l = 0;
    size_t r = container.size() - 1;
    while (int64_t(l) <= int64_t(r)) {
        size_t mb = (l + r) / 2;
        if (value < container[mb]) {
            r = mb - 1;
        } else {
            l = mb + 1;
        }
    }
    // assert(r == l - 1);
    return r;
}

/// @brief let i := return index
///     if (num \in container) => (
///         i < container.size() && num == container[i]
///         && if (i != 0) => (
///             container[i - 1] < num
///         )
///     )
///     else if (!container.empty() && num < container[container.size() - 1]) => (
///         i < container.size() && num < container[i]
///         && if (i != 0) => (
///             container[i - 1] < num
///         )
///     )
///     else => (
///         i == container.size()
///     )
/// @tparam Container 
/// @tparam T 
/// @param container 
/// @param value 
/// @return i
template <class Container, class T>
static size_t find_leftest(const Container& container, T value) {
    size_t l = 0;
    size_t r = container.size() - 1;
    while (int64_t(l) <= int64_t(r)) {
        size_t mb = (l + r) / 2;
        if (container[mb] < value) {
            l = mb + 1;
        } else {
            r = mb - 1;
        }
    }
    // assert(r == l - 1);
    return l;
}

#include <cassert>
#include <vector>

int main() {
    const std::vector<int> arr = {
        1,     3,      5,      7,      9,      123,    124,    125,
        213,   213,    213,    213,    213,    213,    213,    213,
        34523, 213123, 312389, 312389, 312389, 312389, 312389, 1232312};

    for (size_t i = 0; i <= 7; i++) {
        assert(find_leftest(arr, arr[i]) == i);
        assert(find_rightest(arr, arr[i]) == i);
    }

    assert(find_leftest(arr, 0) == 0);
    assert(find_rightest(arr, 0) == size_t(-1));

    assert(find_leftest(arr, 213) == 8);
    assert(find_rightest(arr, 213) == 15);
    assert(find_leftest(arr, 34523) == 16);
    assert(find_rightest(arr, 34523) == 16);
    assert(find_leftest(arr, 213123) == 17);
    assert(find_rightest(arr, 213123) == 17);
    assert(find_leftest(arr, 312389) == 18);
    assert(find_rightest(arr, 312389) == 22);

    assert(find_leftest(arr, 312390) == 23);
    assert(find_rightest(arr, 312390) == 22);    

    assert(find_rightest(arr, 1232312) == 23);

    assert(find_leftest(arr, 1232313) == arr.size());
    assert(find_rightest(arr, 1232313) == arr.size() - 1);
}
