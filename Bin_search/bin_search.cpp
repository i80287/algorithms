#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <map>
#include <vector>
#include <type_traits>

template <typename T, typename Container>
static inline size_t binsearch_rightest_lesser_or_equal_then_value(const Container& values, T value) {
    static_assert(std::is_same_v<std::remove_cvref_t<decltype(values[0])>, std::remove_cvref_t<T>>);
    assert(values.size() != 0);

    size_t l = 0;
    size_t r = values.size() - 1;
    if (values[r] <= value) {
        return r;
    }

    if (value < values[0]) {
        return static_cast<size_t>(-1);
    }

    while (l < r) {
        size_t m_index = (l + r + 1) / 2;
        T m_value = values[m_index];
        if (m_value < value) {
            l = m_index;
        }
        else if (value < m_value) {
            r = m_index - 1;
        }
        else {
            size_t size = values.size();
            while (m_index + 1 < size && values[m_index + 1] == value) {
                ++m_index;
            }

            return m_index;
        }
    }

    return l;
}

template <typename T, typename Container>
static inline size_t binsearch_leftest_greater_or_equal_then_value(const Container& values, T value) {
    static_assert(std::is_same_v<std::remove_cvref_t<decltype(values[0])>, std::remove_cvref_t<T>>);
    assert(values.size() != 0);

    size_t l = 0;
    size_t r = values.size() - 1;
    if (value <= values[0]) {
        return 0;
    }

    if (value > values[r]) {
        return r + 1; // values.size()
    }

    while (l < r) {
        size_t m_index = (l + r) / 2;
        T m_value = values[m_index];
        if (m_value < value) {
            l = m_index + 1;
        }
        else if (value < m_value) {
            r = m_index;
        }
        else
        {
            while (m_index != 0 && values[m_index - 1] == m_value) {
                --m_index;
            }

            return m_index;
        }
    }

    return l;
}

template <typename T>
static inline size_t binsearch_leftest_greater_or_righter_equal_then_value(const T* values, size_t size, T value) {
    assert(size != 0);

    size_t l = 0;
    size_t r = size - 1;
    if (value <= values[0]) {
        return 0;
    }

    if (value > values[r]) {
        return r + 1; // values.size()
    }

    while (l < r)
    {
        size_t m_index = (l + r) / 2;
        T m_value = values[m_index];
        if (m_value < value) {
            l = m_index + 1;
        }
        else if (value < m_value) {
            r = m_index;
        }
        else {
            while (m_index < size && values[m_index + 1] == m_value)
            {
                ++m_index;
            }

            return m_index;
        }
    }

    return l;
}

template <typename T>
static inline size_t binsearch_leftest_greater_or_righter_equal_then_value(const std::vector<T>& values, T value) {
    return binsearch_leftest_greater_or_righter_equal_then_value(values.data(), values.size(), value);
}

template <typename T>
static inline size_t binsearch_rightest_lesser_or_leftest_equal_then_value(const T* values, size_t size, T value) {
    assert(size != 0);

    size_t l = 0;
    size_t r = size - 1;
    if (values[r] <= value) {
        return r;
    }

    if (value < values[0]) {
        return static_cast<size_t>(-1);
    }

    while (l < r) {
        size_t m_index = (l + r + 1) / 2;
        T m_value = values[m_index];
        if (m_value < value) {
            l = m_index;
        }
        else if (value < m_value) {
            r = m_index - 1;
        }
        else {
            while (m_index > 0 && values[m_index -1] == value)
            {
                --m_index;
            }

            return m_index;
        }
    }

    return l;
}

template <typename T>
static inline size_t binsearch_rightest_lesser_or_leftest_equal_then_value(const std::vector<T>& values, T value) {
    return binsearch_rightest_lesser_or_leftest_equal_then_value(values.data(), values.size(), value);
}

template <typename T>
static inline std::pair<size_t, size_t> binsearch_all_equal(const T* values, size_t values_size, T value) {
    size_t l = 0;
    size_t r = values_size - 1;
    if ((value < values[0]) | (value > values[r])) {
		// No elements equal to value
		return {static_cast<size_t>(-1), static_cast<size_t>(-1)};
	}

    while (l < r) {
        size_t m_index = (l + r) / 2;
        if (values[m_index] < value)
        {
            l = m_index + 1;
        }
        else
        {
            r = m_index;
        }
    }

    if (values[l] != value) {
		// No elements equal to value
		return {static_cast<size_t>(-1), static_cast<size_t>(-1)};
	}

    assert(l == 0 || values[l - 1] != value);

	// Now values[l] == values[l + 1] == ... == values[l + ?] == value

	// Find rightest border r such
	// values[l] == values[l + 1] == ... == values[r] == value
	// in log time

    size_t prev_r = r;
    r = values_size - 1;
    while (prev_r < r) {
        size_t m_index = (prev_r + r + 1) / 2;
        if (value != values[m_index])
        {
            r = m_index - 1;
        }
        else
        {
            prev_r = m_index;
        }
    }

    return {l, r};
}

int main() {
	const uint32_t array[] = { 1, 2, 3, 3, 4, 4, 7, 7, 9, 10, 10, 12, 12, 12, 12, 12, 12, 12, 14, 14, 18, 19 };
    const std::vector<uint32_t> vector{ 1, 2, 3, 3, 4, 4, 7, 7, 9, 10, 10, 12, 12, 12, 12, 12, 12, 12, 14, 14, 18, 19 };
	const size_t size = sizeof(array) / sizeof(array[0]);

    {
        size_t l = binsearch_rightest_lesser_or_leftest_equal_then_value<uint32_t>(array, size, 5);
        std::cout << "array[" << l << "]: " << array[l] << '\n'; // array[5]: 4
        size_t r = binsearch_leftest_greater_or_righter_equal_then_value<uint32_t>(array, size, 5);
        std::cout << "array[" << r << "]: " << array[r] << '\n'; // array[6]: 7

        l = binsearch_rightest_lesser_or_leftest_equal_then_value<uint32_t>(array, size, 9);
        std::cout << "array[" << l << "]: " << array[l] << '\n'; // array[8]: 9
        r = binsearch_leftest_greater_or_righter_equal_then_value<uint32_t>(array, size, 9);
        std::cout << "array[" << r << "]: " << array[r] << '\n'; // array[8]: 9

        l = binsearch_rightest_lesser_or_leftest_equal_then_value<uint32_t>(array, size, 12);
        std::cout << "array[" << l << "]: " << array[l] << '\n'; // array[11]: 12
        r = binsearch_leftest_greater_or_righter_equal_then_value<uint32_t>(array, size, 12);
        std::cout << "array[" << r << "]: " << array[r] << '\n'; // array[17]: 12
    }

    std::cout << '\n';

    {
        size_t l = binsearch_rightest_lesser_or_equal_then_value<uint32_t>(vector, 5);
        std::cout << "array[" << l << "]: " << array[l] << '\n'; // array[5]: 4
        size_t r = binsearch_leftest_greater_or_equal_then_value<uint32_t>(vector, 5);
        std::cout << "array[" << r << "]: " << array[r] << '\n'; // array[6]: 7

        l = binsearch_rightest_lesser_or_equal_then_value<uint32_t>(vector, 9);
        std::cout << "array[" << l << "]: " << array[l] << '\n'; // array[8]: 9
        r = binsearch_leftest_greater_or_equal_then_value<uint32_t>(vector, 9);
        std::cout << "array[" << r << "]: " << array[r] << '\n'; // array[8]: 9

        l = binsearch_rightest_lesser_or_equal_then_value<uint32_t>(vector, 12);
        std::cout << "array[" << l << "]: " << array[l] << '\n'; // array[17]: 12
        r = binsearch_leftest_greater_or_equal_then_value<uint32_t>(vector, 12);
        std::cout << "array[" << r << "]: " << array[r] << '\n'; // array[11]: 12
    }
}
