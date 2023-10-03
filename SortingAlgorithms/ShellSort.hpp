#ifndef SHELL_SORT_HPP
#define SHELL_SORT_HPP 1

#include <cstdint>
#include <iterator>
#include <type_traits>
#include <utility>

template<typename Iterator>
#if __cplusplus >= 202002L
requires std::random_access_iterator<Iterator>
constexpr
#endif
static inline void ShellSort(Iterator begin, size_t size) {
	for (size_t l = size / 2; l != 0; l /= 2) {
		for(size_t i = l; i < size; ++i) {
			size_t temp_index = i;
			size_t diff = i - l;

			if constexpr (std::is_arithmetic_v<typename std::iterator_traits<Iterator>::value_type>) {
				const auto temp_shifting_item = *(begin + i);
				while(*(begin + diff) > *(begin + temp_index)) {
					*(begin + temp_index) = *(begin + diff);
					*(begin + diff) = temp_shifting_item;

					temp_index = diff;
					if (diff < l) {
						break;
					}

					diff -= l;
				}
			}
			else {
				while (*(begin + diff) > *(begin + temp_index)) {
					std::swap(*(begin + temp_index), *(begin + diff));

					temp_index = diff;
					if (diff < l) {
						break;
					}

					diff -= l;
				}
			}
		}
	}
}

template<typename Iterator>
#if __cplusplus >= 202002L
requires std::random_access_iterator<Iterator>
constexpr
#endif
static inline void ShellSort(Iterator begin, Iterator end) {
	ShellSort(begin, static_cast<size_t>(std::distance(begin, end)));
}

#endif // SHELL_SORT_HPP
