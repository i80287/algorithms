#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <type_traits>
#include <vector>

#include "config_macros.hpp"

namespace hungarian_algo {

template <class T>
#if __cplusplus >= 202002L
    requires std::is_arithmetic_v<T>
#endif
class MinAssignmentGraph {
    using vertex_t = size_t;

public:
    static MinAssignmentGraph from_matrix(
        const std::vector<std::vector<T>>& original_matrix) {
        const size_t n = original_matrix.size();

        const size_t matrix_size = align_size(sizeof(T*) * n + sizeof(T) * n * n);
        const size_t bipartite_graph_matrix_size =
            align_size(sizeof(bool*) * n + sizeof(bool) * n * n);
        const size_t first_part_matches_size  = align_size(sizeof(vertex_t) * n);
        const size_t second_part_matches_size = align_size(sizeof(vertex_t) * n);
        const size_t neighbours_size =
            align_size(sizeof(vertex_t*) * n + sizeof(vertex_t) * n * n);
        const size_t neighbours_count_size    = align_size(sizeof(size_t) * n);
        const size_t first_part_visited_size  = align_size(sizeof(bool) * n);
        const size_t second_part_visited_size = align_size(sizeof(bool) * n);
        const size_t algorithm_memory_size =
            matrix_size + bipartite_graph_matrix_size + first_part_matches_size +
            second_part_matches_size + neighbours_size + neighbours_count_size +
            first_part_visited_size + second_part_visited_size;
        char* current_free_memory =
            static_cast<char*>(operator new(algorithm_memory_size));
        std::memset(current_free_memory, 0, algorithm_memory_size);

        T** const matrix     = reinterpret_cast<T**>(current_free_memory);
        T* matrix_data_start = reinterpret_cast<T*>(matrix + n);
        current_free_memory += matrix_size;

        bool** const bipartite_graph_matrix =
            reinterpret_cast<bool**>(current_free_memory);
        bool* bipartite_graph_matrix_data_start =
            reinterpret_cast<bool*>(bipartite_graph_matrix + n);
        current_free_memory += bipartite_graph_matrix_size;

        vertex_t* const first_part_matches =
            reinterpret_cast<vertex_t*>(current_free_memory);
        current_free_memory += first_part_matches_size;

        vertex_t* const second_part_matches =
            reinterpret_cast<vertex_t*>(current_free_memory);
        current_free_memory += second_part_matches_size;

        vertex_t** const neighbours = reinterpret_cast<vertex_t**>(current_free_memory);
        vertex_t* neighbours_data_start = reinterpret_cast<vertex_t*>(neighbours + n);
        current_free_memory += neighbours_size;

        size_t* const neighbours_count = reinterpret_cast<size_t*>(current_free_memory);
        current_free_memory += neighbours_count_size;

        bool* const first_part_visited = reinterpret_cast<bool*>(current_free_memory);
        current_free_memory += first_part_visited_size;

        bool* second_part_visited = reinterpret_cast<bool*>(current_free_memory);

        for (size_t i = 0; i < n; i++) {
            matrix[i]                 = matrix_data_start + i * n;
            bipartite_graph_matrix[i] = bipartite_graph_matrix_data_start + i * n;
            neighbours[i]             = neighbours_data_start + i * n;
        }

        copy_matrix(original_matrix, matrix);

        return MinAssignmentGraph(first_part_matches, second_part_matches,
                                  first_part_visited, second_part_visited, neighbours,
                                  neighbours_count, bipartite_graph_matrix, matrix, n);
    }

    bool next_iter() noexcept {
        fill_bipartite_graph();
        bool is_perfect_matching = find_max_matching();
        if (is_perfect_matching) {
            return true;
        }
        apply_alpha_transformation();
        return false;
    }

    T accumulate_over(const std::vector<std::vector<T>>& original_matrix) noexcept {
        T ans = 0;
        for (size_t i = 0; i < size_; i++) {
            size_t j = first_part_matches_[i];
            assert(j != kNoMatch && second_part_matches_[j] == i);
            ans += original_matrix[i][j];
        }
        return ans;
    }

    ~MinAssignmentGraph() { operator delete(static_cast<void*>(matrix_)); }

private:
    constexpr MinAssignmentGraph(vertex_t* RESTRICT_QUALIFIER first_part_matches,
                                 vertex_t* RESTRICT_QUALIFIER second_part_matches,
                                 bool* RESTRICT_QUALIFIER first_part_visited,
                                 bool* RESTRICT_QUALIFIER second_part_visited,
                                 size_t** RESTRICT_QUALIFIER neighbours,
                                 size_t* RESTRICT_QUALIFIER neighbours_count,
                                 bool** RESTRICT_QUALIFIER bipartite_graph_matrix,
                                 T** RESTRICT_QUALIFIER matrix, size_t size) noexcept
        : first_part_matches_(first_part_matches),
          second_part_matches_(second_part_matches),
          first_part_visited_(first_part_visited),
          second_part_visited_(second_part_visited),
          neighbours_(neighbours),
          neighbours_count_(neighbours_count),
          bipartite_graph_matrix_(bipartite_graph_matrix),
          matrix_(matrix),
          size_(size) {}

    static constexpr size_t align_size(size_t n) noexcept {
        n = (n + 15) & ~size_t(15);
        ATTRIBUTE_ASSUME(n % 16 == 0);
        return n;
    }

    /// @brief Makes copy of matrix with zero on rows and columns (by subtracting
    /// min value of row from row and min value of column from column)
    /// @param matrix
    /// @param matrix_copy
    static void copy_matrix(const std::vector<std::vector<T>>& matrix,
                            T** RESTRICT_QUALIFIER matrix_copy) noexcept {
        size_t n = matrix.size();
        for (size_t i = 0; i < n; i++) {
            const T* row_i = matrix[i].data();
            T min_in_row   = row_i[0];
            for (size_t j = 1; j < n; j++) {
                if (row_i[j] < min_in_row) {
                    min_in_row = row_i[j];
                }
            }

            T* row_i_copy = matrix_copy[i];
            for (size_t j = 0; j < n; j++) {
                row_i_copy[j] = row_i[j] - min_in_row;
            }
        }

        for (size_t j = 0; j < n; j++) {
            T min_in_column = matrix_copy[0][j];
            for (size_t i = 1; i < n; i++) {
                if (matrix_copy[i][j] < min_in_column) {
                    min_in_column = matrix_copy[i][j];
                }
            }

            for (size_t i = 0; i < n; i++) {
                matrix_copy[i][j] -= min_in_column;
            }
        }
    }

    void fill_bipartite_graph() noexcept {
        size_t matrix_size = size_ * size_;
        std::memset(bipartite_graph_matrix_ + size_, false, sizeof(bool) * matrix_size);
        std::memset(neighbours_ + size_, 0, sizeof(vertex_t) * matrix_size);

        for (vertex_t i = 0; i < size_; i++) {
            bool* bipartite_graph_matrix_row = bipartite_graph_matrix_[i];

            vertex_t* i_neighbours    = neighbours_[i];
            size_t i_neighbours_count = 0;
            for (vertex_t j = 0; j < size_; j++) {
                if (matrix_[i][j] == 0) {
                    bipartite_graph_matrix_row[j]    = true;
                    i_neighbours[i_neighbours_count] = j;
                    i_neighbours_count++;
                }
            }

            neighbours_count_[i] = i_neighbours_count;
        }

        for (vertex_t i = 0; i < size_; i++) {
            first_part_matches_[i]  = kNoMatch;
            second_part_matches_[i] = kNoMatch;
        }

        for (vertex_t i = 0; i < size_; i++) {
            if (first_part_matches_[i] != kNoMatch) {
                assert(second_part_matches_[first_part_matches_[i]] == i);
                continue;
            }

            const size_t* i_neighbours = neighbours_[i];
            size_t i_neighbours_count  = neighbours_count_[i];

            for (size_t neighbour_index = 0; neighbour_index < i_neighbours_count;
                 ++neighbour_index) {
                size_t j = i_neighbours[neighbour_index];
                assert(bipartite_graph_matrix_[i][j]);
                if (second_part_matches_[j] == kNoMatch) {
                    second_part_matches_[j] = i;
                    first_part_matches_[i]  = j;
                    break;
                }
            }
        }
    }

    bool find_max_matching() noexcept {
        for (vertex_t i = 0; i < size_; i++) {
            if (first_part_matches_[i] == kNoMatch) {
                std::memset(first_part_visited_, false, sizeof(bool) * size_);
                std::memset(second_part_visited_, false, sizeof(bool) * size_);
                dfs_find_chain_and_update_matches(i);
            }
        }

        std::memset(first_part_visited_, false, sizeof(bool) * size_);
        std::memset(second_part_visited_, false, sizeof(bool) * size_);
        bool graph_satisfied = true;
        for (vertex_t i = 0; i < size_; i++) {
            if (first_part_matches_[i] == kNoMatch) {
                graph_satisfied = false;
                dfs_from_unmatched(i);
            }
        }

        return graph_satisfied;
    }

    size_t dfs_find_chain_and_update_matches(size_t i) noexcept {
        assert(!first_part_visited_[i]);
        first_part_visited_[i]     = true;
        const size_t* i_neighbours = neighbours_[i];

        for (size_t neighbour_index = 0, total_neighbours = neighbours_count_[i];
             neighbour_index < total_neighbours; ++neighbour_index) {
            size_t j = i_neighbours[neighbour_index];
            size_t k = second_part_matches_[j];

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
                first_part_matches_[i]  = j;
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
                size_t end_vertex       = dfs_find_chain_and_update_matches(k);
                if (end_vertex != kNoVertex) {
                    second_part_matches_[j] = i;
                    first_part_matches_[i]  = j;
                    return end_vertex;
                }
            }
        }

        return kNoVertex;
    }

    void dfs_from_unmatched(size_t i) noexcept {
        first_part_visited_[i]     = true;
        const size_t* i_neighbours = neighbours_[i];

        for (size_t neighbour_index = 0, total_neighbours = neighbours_count_[i];
             neighbour_index < total_neighbours; ++neighbour_index) {
            size_t j   = i_neighbours[neighbour_index];
            vertex_t k = second_part_matches_[j];
            if (k != kNoMatch && !first_part_visited_[k]) {
                second_part_visited_[j] = true;
                dfs_from_unmatched(k);
            }
        }
    }

    void apply_alpha_transformation() noexcept {
        T min = std::numeric_limits<T>::max();
        for (size_t i = 0; i < size_; i++) {
            for (size_t j = 0; j < size_; j++) {
                bool x_i = first_part_visited_[i];
                bool y_j = second_part_visited_[j];
                if (x_i && !y_j) {
                    T current = matrix_[i][j];
                    assert(current != 0);
                    if (current < min) {
                        min = current;
                    }
                }
            }
        }

        assert(min != std::numeric_limits<T>::max());
        for (size_t i = 0; i < size_; i++) {
            for (size_t j = 0; j < size_; j++) {
                bool x_i = first_part_visited_[i];
                bool y_j = second_part_visited_[j];
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

    static constexpr vertex_t kNoMatch  = static_cast<vertex_t>(-1);
    static constexpr vertex_t kNoVertex = static_cast<vertex_t>(-1);

    vertex_t* const RESTRICT_QUALIFIER first_part_matches_;
    vertex_t* const RESTRICT_QUALIFIER second_part_matches_;
    bool* const RESTRICT_QUALIFIER first_part_visited_;
    bool* const RESTRICT_QUALIFIER second_part_visited_;
    size_t** const RESTRICT_QUALIFIER neighbours_;
    size_t* const RESTRICT_QUALIFIER neighbours_count_;
    bool** const RESTRICT_QUALIFIER bipartite_graph_matrix_;
    T** const RESTRICT_QUALIFIER matrix_;
    const size_t size_;
};

template <class T>
#if __cplusplus >= 202002L
    requires std::is_arithmetic_v<T>
#endif
T min_assignment(const std::vector<std::vector<T>>& original_matrix) {
    MinAssignmentGraph<T> g = MinAssignmentGraph<T>::from_matrix(original_matrix);

    while (true) {
        bool found_perfect_matching = g.next_iter();
        if (found_perfect_matching) {
            break;
        }
    }

    return g.accumulate_over(original_matrix);
}

}  // namespace hungarian_algo

static void TestHungarianAlgorithm() {
    std::vector<std::vector<std::vector<uint32_t>>> input{
        {
            {1},
        },
        {
            {1, 6, 1},
            {3, 8, 5},
            {2, 7, 6},
        },
        {
            {32, 28, 4, 26, 4},
            {17, 19, 4, 17, 4},
            {4, 4, 5, 4, 4},
            {17, 14, 4, 14, 4},
            {21, 16, 4, 13, 4},
        },
        {
            {1, 1, 1, 0, 0, 0},
            {1, 1, 0, 1, 0, 0},
            {1, 0, 1, 1, 1, 0},
            {0, 1, 1, 1, 0, 0},
            {0, 0, 1, 0, 1, 1},
            {0, 0, 0, 0, 1, 1},
        },
        {
            {61, 80, 89, 22, 41, 76, 79, 62, 4, 58},
            {54, 64, 61, 18, 43, 37, 67, 62, 91, 2},
            {23, 87, 35, 1, 39, 90, 72, 51, 15, 96},
            {69, 69, 67, 45, 47, 90, 38, 94, 10, 89},
            {64, 47, 50, 79, 64, 86, 9, 41, 91, 46},
            {52, 75, 43, 64, 40, 56, 73, 76, 14, 90},
            {73, 79, 98, 49, 39, 39, 87, 75, 57, 63},
            {68, 41, 23, 22, 48, 63, 2, 7, 19, 59},
            {36, 25, 45, 11, 25, 11, 96, 15, 22, 27},
            {17, 33, 25, 22, 39, 26, 48, 60, 11, 57},
        },
        {
            {10, 64, 15, 53, 93, 95, 90, 7, 38, 42},
            {77, 77, 57, 20, 45, 28, 48, 71, 15, 62},
            {61, 43, 12, 59, 53, 30, 81, 24, 70, 62},
            {39, 37, 92, 20, 57, 77, 94, 10, 85, 90},
            {33, 30, 40, 93, 46, 20, 69, 81, 66, 39},
            {15, 61, 41, 42, 85, 31, 17, 46, 53, 68},
            {11, 88, 7, 57, 67, 69, 60, 55, 63, 1},
            {58, 24, 72, 44, 67, 81, 28, 58, 31, 5},
            {82, 54, 30, 5, 48, 41, 23, 91, 59, 10},
            {21, 76, 10, 71, 11, 23, 79, 18, 8, 33},
        },
        {
            {47, 6, 53, 82, 11, 67, 56, 37, 82, 25},
            {75, 35, 63, 16, 44, 75, 58, 53, 94, 26},
            {13, 32, 27, 71, 53, 34, 27, 21, 92, 96},
            {46, 7, 62, 76, 76, 36, 33, 72, 17, 38},
            {43, 94, 55, 12, 9, 9, 60, 18, 80, 71},
            {2, 54, 84, 11, 60, 75, 48, 32, 76, 23},
            {43, 52, 20, 29, 41, 75, 37, 80, 38, 95},
            {92, 23, 28, 18, 25, 90, 84, 35, 97, 83},
            {94, 59, 67, 56, 88, 16, 82, 28, 46, 80},
            {75, 76, 86, 2, 79, 1, 49, 8, 72, 69},
        },
        {
            {1, 0, 1, 0, 1, 1, 0, 1, 0, 1},
            {1, 1, 1, 0, 0, 1, 0, 1, 0, 0},
            {1, 0, 1, 1, 1, 0, 1, 1, 0, 0},
            {0, 1, 0, 0, 0, 0, 1, 0, 1, 0},
            {1, 0, 1, 0, 1, 1, 0, 0, 0, 1},
            {0, 0, 0, 1, 0, 1, 0, 0, 0, 1},
            {1, 0, 0, 1, 1, 1, 1, 0, 0, 1},
            {0, 1, 0, 0, 1, 0, 0, 1, 1, 1},
            {0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
            {1, 0, 0, 0, 1, 1, 1, 0, 0, 1},
        }};

    uint32_t output[]            = {1, 11, 39, 0, 194, 125, 149, 0};
    constexpr size_t total_tests = sizeof(output) / sizeof(output[0]);
    assert(input.size() == total_tests);

    for (size_t k = 0; k < total_tests; k++) {
        uint32_t ans = hungarian_algo::min_assignment<uint32_t>(input[k]);
        std::cout << "Test " << (k + 1) << ((ans == output[k]) ? "" : " not")
                  << " passed\nAlgorithm answer: " << ans
                  << "\nCorrect answer: " << output[k] << '\n';
    }
}

int main() { TestHungarianAlgorithm(); }
