#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <new>
#include <type_traits>

#include "../../misc/config_macros.hpp"

namespace hungarian_algo {

namespace detail {

template <class MatrixRow>
using MatrixRowValueType = std::iter_value_t<typename std::ranges::iterator_t<MatrixRow>>;

template <class MatrixIterator>
using MatrixValueType = MatrixRowValueType<typename std::iter_value_t<MatrixIterator>>;

template <class Iterator>
concept MatrixRowIterator =
    std::random_access_iterator<Iterator> && std::is_arithmetic_v<typename std::iter_value_t<Iterator>>;

template <class Container>
concept MatrixRow =
    std::ranges::random_access_range<Container> && MatrixRowIterator<typename std::ranges::iterator_t<Container>>;

template <class Iterator>
concept MatrixIterator =
    std::random_access_iterator<Iterator> && MatrixRow<typename std::iterator_traits<Iterator>::value_type>;

template <class T>
class MinAssignmentGraph final {
    using vertex_t = typename std::size_t;

public:
    template <std::ranges::range Matrix>
    [[nodiscard]] static T min_assignment(const Matrix& matrix) {
        return min_assignment(std::begin(matrix), std::end(matrix));
    }

    template <MatrixIterator Iterator>
    [[nodiscard]] static T min_assignment(Iterator matrix_iter_begin, Iterator matrix_iter_end) {
        auto g = MinAssignmentGraph<T>::from_matrix(matrix_iter_begin, matrix_iter_end);
        bool found_perfect_matching{};
        do {
            found_perfect_matching = g.next_iter();
        } while (!found_perfect_matching);
        return g.accumulate_over(matrix_iter_begin);
    }

    MinAssignmentGraph(const MinAssignmentGraph&) = delete;
    MinAssignmentGraph(MinAssignmentGraph&&) = delete;
    MinAssignmentGraph& operator=(const MinAssignmentGraph&) = delete;
    MinAssignmentGraph& operator=(MinAssignmentGraph&&) = delete;
    ~MinAssignmentGraph() {
        operator delete(std::bit_cast<std::byte*>(matrix_));
    }

private:
    template <class Iterator>
    [[nodiscard]] static MinAssignmentGraph from_matrix(Iterator original_matrix_iter_begin,
                                                        Iterator original_matrix_iter_end) {
        const auto n = static_cast<std::size_t>(std::distance(original_matrix_iter_begin, original_matrix_iter_end));

        const size_t matrix_size = align_size(sizeof(T*) * n + sizeof(T) * n * n);
        const size_t bipartite_graph_matrix_size = align_size(sizeof(bool*) * n + sizeof(bool) * n * n);
        const size_t first_part_matches_size = align_size(sizeof(vertex_t) * n);
        const size_t second_part_matches_size = align_size(sizeof(vertex_t) * n);
        const size_t neighbours_size = align_size(sizeof(vertex_t*) * n + sizeof(vertex_t) * n * n);
        const size_t neighbours_count_size = align_size(sizeof(size_t) * n);
        const size_t first_part_visited_size = align_size(sizeof(bool) * n);
        const size_t second_part_visited_size = align_size(sizeof(bool) * n);
        const size_t algorithm_memory_size = matrix_size + bipartite_graph_matrix_size + first_part_matches_size +
                                             second_part_matches_size + neighbours_size + neighbours_count_size +
                                             first_part_visited_size + second_part_visited_size;
        std::byte* current_free_memory_ptr = static_cast<std::byte*>(operator new(algorithm_memory_size));
        std::fill_n(current_free_memory_ptr, algorithm_memory_size, std::byte{0});
        [[maybe_unused]] const auto* const allocated_memory_begin = current_free_memory_ptr;

        T** const matrix = std::launder(std::bit_cast<T**>(current_free_memory_ptr));
        T* matrix_data_start = std::launder(std::bit_cast<T*>(matrix + n));
        current_free_memory_ptr += matrix_size;

        bool** const bipartite_graph_matrix = std::launder(std::bit_cast<bool**>(current_free_memory_ptr));
        bool* bipartite_graph_matrix_data_start = std::launder(std::bit_cast<bool*>(bipartite_graph_matrix + n));
        current_free_memory_ptr += bipartite_graph_matrix_size;

        vertex_t* const first_part_matches = std::launder(std::bit_cast<vertex_t*>(current_free_memory_ptr));
        current_free_memory_ptr += first_part_matches_size;

        vertex_t* const second_part_matches = std::launder(std::bit_cast<vertex_t*>(current_free_memory_ptr));
        current_free_memory_ptr += second_part_matches_size;

        vertex_t** const neighbours = std::launder(std::bit_cast<vertex_t**>(current_free_memory_ptr));
        vertex_t* neighbours_data_start = std::launder(std::bit_cast<vertex_t*>(neighbours + n));
        current_free_memory_ptr += neighbours_size;

        std::size_t* const neighbours_count = std::launder(std::bit_cast<std::size_t*>(current_free_memory_ptr));
        current_free_memory_ptr += neighbours_count_size;

        bool* const first_part_visited = std::launder(std::bit_cast<bool*>(current_free_memory_ptr));
        current_free_memory_ptr += first_part_visited_size;

        bool* second_part_visited = std::launder(std::bit_cast<bool*>(current_free_memory_ptr));
        current_free_memory_ptr += second_part_visited_size;
        assert(current_free_memory_ptr == allocated_memory_begin + algorithm_memory_size);

        for (std::size_t i = 0; i < n; i++) {
            matrix[i] = matrix_data_start + i * n;
            bipartite_graph_matrix[i] = bipartite_graph_matrix_data_start + i * n;
            neighbours[i] = neighbours_data_start + i * n;
        }

        copy_matrix_with_subtraction(original_matrix_iter_begin, n, matrix);

        return MinAssignmentGraph(first_part_matches, second_part_matches, first_part_visited, second_part_visited,
                                  neighbours, neighbours_count, bipartite_graph_matrix, matrix, n);
    }

    [[nodiscard]] constexpr bool next_iter() noexcept {
        fill_bipartite_graph();
        if (const bool found_perfect_matching = find_max_matching(); found_perfect_matching) {
            return true;
        }
        apply_alpha_transformation();
        return false;
    }

    template <class Iterator>
    [[nodiscard]] constexpr T accumulate_over(Iterator matrix_iter_begin) const noexcept {
        T ans = 0;
        for (std::size_t i = 0; i < size_; ++matrix_iter_begin, ++i) {
            const std::size_t j = first_part_matches_[i];
            assert(j != kNoMatch && second_part_matches_[j] == i);
            const auto& matrix_row = *matrix_iter_begin;
            ans += matrix_row[j];
        }
        return ans;
    }

    ATTRIBUTE_NONNULL_ALL_ARGS
    constexpr MinAssignmentGraph(vertex_t* RESTRICT_QUALIFIER first_part_matches,
                                 vertex_t* RESTRICT_QUALIFIER second_part_matches,
                                 bool* RESTRICT_QUALIFIER first_part_visited,
                                 bool* RESTRICT_QUALIFIER second_part_visited,
                                 std::size_t** RESTRICT_QUALIFIER neighbours,
                                 std::size_t* RESTRICT_QUALIFIER neighbours_count,
                                 bool** RESTRICT_QUALIFIER bipartite_graph_matrix,
                                 T** RESTRICT_QUALIFIER matrix,
                                 std::size_t size) noexcept
        : first_part_matches_(first_part_matches)
        , second_part_matches_(second_part_matches)
        , first_part_visited_(first_part_visited)
        , second_part_visited_(second_part_visited)
        , neighbours_(neighbours)
        , neighbours_count_(neighbours_count)
        , bipartite_graph_matrix_(bipartite_graph_matrix)
        , matrix_(matrix)
        , size_(size) {}

    [[nodiscard]] ATTRIBUTE_CONST static constexpr std::size_t align_size(std::size_t n) noexcept {
        constexpr std::size_t kAlignmentBoundary = 32;
        n = (n + kAlignmentBoundary) & ~(kAlignmentBoundary - 1);
        return n;
    }

    /// @brief Makes copy of matrix with zero on rows and columns (by subtracting
    /// min value of row from row and min value of column from column)
    /// @param matrix
    /// @param matrix_copy
    template <class Iterator>
    ATTRIBUTE_ACCESS(read_write, 3)
    static void copy_matrix_with_subtraction(Iterator original_matrix_iter_begin,
                                             std::size_t n,
                                             T** RESTRICT_QUALIFIER matrix_copy) noexcept {
        for (std::size_t i = 0; i < n; ++original_matrix_iter_begin, ++i) {
            const auto& original_matrix_row = *original_matrix_iter_begin;
            if constexpr (std::ranges::sized_range<decltype(original_matrix_row)>) {
                assert(std::ranges::size(original_matrix_row) == n);
            }
            std::copy_n(std::begin(original_matrix_row), n, matrix_copy[i]);
        }

        for (std::size_t i = 0; i < n; i++) {
            T* row = matrix_copy[i];
            const T min_in_row = *std::min_element(row, row + n);
            for (std::size_t j = 0; j < n; j++) {
                row[j] -= min_in_row;
            }
        }

        for (std::size_t j = 0; j < n; j++) {
            T min_in_column = matrix_copy[0][j];
            for (std::size_t i = 1; i < n; i++) {
                min_in_column = std::min(min_in_column, matrix_copy[i][j]);
            }

            if (min_in_column != 0) {
                for (std::size_t i = 0; i < n; i++) {
                    matrix_copy[i][j] -= min_in_column;
                }
            }
        }
    }

    constexpr void fill_bipartite_graph() noexcept {
        const std::size_t matrix_size = size_ * size_;
        assert(std::bit_cast<std::uintptr_t>(bipartite_graph_matrix_[0]) ==
               std::bit_cast<std::uintptr_t>(bipartite_graph_matrix_ + size_));
        std::fill_n(bipartite_graph_matrix_[0], matrix_size, false);
        assert(std::bit_cast<std::uintptr_t>(neighbours_[0]) == std::bit_cast<std::uintptr_t>(neighbours_ + size_));
        std::fill_n(neighbours_[0], matrix_size, vertex_t{0});

        for (vertex_t i = 0; i < size_; i++) {
            bool* bipartite_graph_matrix_row = bipartite_graph_matrix_[i];

            vertex_t* i_neighbours = neighbours_[i];
            std::size_t i_neighbours_count = 0;
            for (vertex_t j = 0; j < size_; j++) {
                if (matrix_[i][j] == 0) {
                    bipartite_graph_matrix_row[j] = true;
                    i_neighbours[i_neighbours_count] = j;
                    i_neighbours_count++;
                }
            }

            neighbours_count_[i] = i_neighbours_count;
        }

        for (vertex_t i = 0; i < size_; i++) {
            first_part_matches_[i] = kNoMatch;
            second_part_matches_[i] = kNoMatch;
        }

        for (vertex_t i = 0; i < size_; i++) {
            if (first_part_matches_[i] != kNoMatch) {
                assert(second_part_matches_[first_part_matches_[i]] == i);
                continue;
            }

            const std::size_t* i_neighbours = neighbours_[i];
            const std::size_t i_neighbours_count = neighbours_count_[i];
            for (std::size_t neighbour_index = 0; neighbour_index < i_neighbours_count; ++neighbour_index) {
                const std::size_t j = i_neighbours[neighbour_index];
                assert(bipartite_graph_matrix_[i][j]);
                if (second_part_matches_[j] == kNoMatch) {
                    second_part_matches_[j] = i;
                    first_part_matches_[i] = j;
                    break;
                }
            }
        }
    }

    [[nodiscard]] constexpr bool find_max_matching() noexcept {
        for (vertex_t i = 0; i < size_; ++i) {
            if (first_part_matches_[i] == kNoMatch) {
                std::fill_n(first_part_visited_, size_, false);
                std::fill_n(second_part_visited_, size_, false);
                static_cast<void>(dfs_find_chain_and_update_matches(i));
            }
        }

        std::fill_n(first_part_visited_, size_, false);
        std::fill_n(second_part_visited_, size_, false);
        bool graph_satisfied = true;
        for (vertex_t i = 0; i < size_; i++) {
            if (first_part_matches_[i] == kNoMatch) {
                graph_satisfied = false;
                dfs_from_unmatched(i);
            }
        }

        return graph_satisfied;
    }

    [[nodiscard]] constexpr std::size_t dfs_find_chain_and_update_matches(std::size_t i) noexcept {
        assert(!first_part_visited_[i]);
        first_part_visited_[i] = true;
        const std::size_t* i_neighbours = neighbours_[i];

        for (std::size_t neighbour_index = 0, total_neighbours = neighbours_count_[i];
             neighbour_index < total_neighbours; ++neighbour_index) {
            const std::size_t j = i_neighbours[neighbour_index];
            const std::size_t k = second_part_matches_[j];

            /*
             * Bipart. Graph
             *
             *      X   Y
             *
             * 1 -> i
             *       \
             *        \
             *         %
             *          j <- 2
             *
             */
            if (k == kNoMatch) {
                second_part_matches_[j] = i;
                first_part_matches_[i] = j;
                return j;
            }

            /*
             * Bipart. Graph
             *
             *      X   Y
             *
             * 1 -> i
             *       \
             *        \
             *         %
             * 3 -> k<--j <- 2
             *
             */
            if (!second_part_visited_[j] && !first_part_visited_[k]) {
                second_part_visited_[j] = true;
                const std::size_t end_vertex = dfs_find_chain_and_update_matches(k);
                if (end_vertex != kNoVertex) {
                    second_part_matches_[j] = i;
                    first_part_matches_[i] = j;
                    return end_vertex;
                }
            }
        }

        return kNoVertex;
    }

    constexpr void dfs_from_unmatched(std::size_t i) noexcept {
        first_part_visited_[i] = true;
        const std::size_t* i_neighbours = neighbours_[i];

        for (std::size_t neighbour_index = 0, total_neighbours = neighbours_count_[i];
             neighbour_index < total_neighbours; ++neighbour_index) {
            const std::size_t j = i_neighbours[neighbour_index];
            const vertex_t k = second_part_matches_[j];
            if (k != kNoMatch && !first_part_visited_[k]) {
                second_part_visited_[j] = true;
                dfs_from_unmatched(k);
            }
        }
    }

    constexpr void apply_alpha_transformation() noexcept {
        const auto min = find_min_for_alpha_transformation();
        assert(min != std::numeric_limits<T>::max());
        for (std::size_t i = 0; i < size_; i++) {
            for (std::size_t j = 0; j < size_; j++) {
                const bool x_i = first_part_visited_[i];
                const bool y_j = second_part_visited_[j];
                if (x_i != y_j) {
                    if (x_i) {
                        matrix_[i][j] -= min;
                    } else {
                        matrix_[i][j] += min;
                    }
                }
            }
        }
    }

    [[nodiscard]] constexpr T find_min_for_alpha_transformation() const noexcept {
        T min = std::numeric_limits<T>::max();
        for (std::size_t i = 0; i < size_; i++) {
            for (std::size_t j = 0; j < size_; j++) {
                const bool x_i = first_part_visited_[i];
                const bool y_j = second_part_visited_[j];
                if (x_i && !y_j) {
                    assert(matrix_[i][j] != 0);
                    min = std::min(min, matrix_[i][j]);
                }
            }
        }

        return min;
    }

    static constexpr vertex_t kNoMatch = static_cast<vertex_t>(-1);
    static constexpr vertex_t kNoVertex = static_cast<vertex_t>(-1);

    vertex_t* const RESTRICT_QUALIFIER first_part_matches_;
    vertex_t* const RESTRICT_QUALIFIER second_part_matches_;
    bool* const RESTRICT_QUALIFIER first_part_visited_;
    bool* const RESTRICT_QUALIFIER second_part_visited_;
    vertex_t** const RESTRICT_QUALIFIER neighbours_;
    std::size_t* const RESTRICT_QUALIFIER neighbours_count_;
    bool** const RESTRICT_QUALIFIER bipartite_graph_matrix_;
    T** const RESTRICT_QUALIFIER matrix_;
    const std::size_t size_;
};

}  // namespace detail

template <hungarian_algo::detail::MatrixIterator Iterator>
[[nodiscard]] auto min_assignment(Iterator matrix_iter_begin, Iterator matrix_iter_end) {
    using T = typename hungarian_algo::detail::MatrixValueType<Iterator>;
    return hungarian_algo::detail::MinAssignmentGraph<T>::min_assignment(matrix_iter_begin, matrix_iter_end);
}

template <std::ranges::random_access_range TMatrix>
[[nodiscard]] auto min_assignment(const TMatrix& matrix) {
    return hungarian_algo::min_assignment(std::begin(matrix), std::end(matrix));
}

}  // namespace hungarian_algo
