#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <iostream>
#include <vector>
#include <random>

inline constexpr auto NO_MATCH = static_cast<size_t>(-1);
inline constexpr auto NO_VERTEX = static_cast<size_t>(-1);

template <typename T>
requires std::is_integral_v<T>
[[maybe_unused]] static constexpr bool check_simmetric(const T *const * arr, size_t n) noexcept {
    for (size_t i = 0; i < n; i++) {
        for (size_t j = i + 1; j < n; j++) {
            if (arr[i][j] != arr[j][i]) {
                return false;
            }
        }
    }

    return true;
}

template <typename T>
requires std::is_integral_v<T>
static constexpr T** MakeCopyWithZeroOnRowsAndColumns(const std::vector<std::vector<T>>& matrix) noexcept {
    size_t n = matrix.size();
    if (n == 0) {
        return nullptr;
    }

    T** matrix_copy = new T*[n]();

    for (size_t i = 0; i < n; i++) {
        const std::vector<T>& row_i = matrix[i];

        T min_in_row = row_i[0];
        for (size_t j = 1; j < n; j++) {
            if (row_i[j] < min_in_row) {
                min_in_row = row_i[j];
            }
        }

        T* row_i_copy = matrix_copy[i] = new T[n]();
        if (min_in_row != 0) {
            for (size_t j = 0; j < n; j++) {
                row_i_copy[j] = row_i[j] - min_in_row;
            }
        }
        else {
            // Not very rare case
            // Can by optimized by the compiler
            for (size_t j = 0; j < n; j++) {
                row_i_copy[j] = row_i[j];
            }
        }
    }

    for (size_t j = 0; j < n; j++) {
        auto min_in_column = matrix_copy[0][j];
        for (size_t i = 1; i < n; i++) {
            if (matrix_copy[i][j] < min_in_column) {
                min_in_column = matrix_copy[i][j];
            }
        }

        if (min_in_column != 0) {
            for (size_t i = 0; i < n; i++) {
                matrix_copy[i][j] -= min_in_column;
            }
        }
    }

    return matrix_copy;
}

struct DFSPackedInfo {
    size_t* first_part_matches;
    size_t* second_part_matches;
    const size_t *const * neighbours;
    const size_t* neighbours_count;
    bool* first_part_visited;
    bool* second_part_visited;
};

template <bool IsDebug = false>
static size_t DFSFindChainUpdateMatches(size_t i, DFSPackedInfo& dfs_packed_info) noexcept {
    if constexpr (IsDebug) { assert(!dfs_packed_info.first_part_visited[i]); }
    dfs_packed_info.first_part_visited[i] = true;
    const size_t* i_neighbours = dfs_packed_info.neighbours[i];

    for (size_t neighbour_index = 0, total_neighbours = dfs_packed_info.neighbours_count[i];
        neighbour_index < total_neighbours;
        ++neighbour_index) {

        size_t j = i_neighbours[neighbour_index];
        size_t k = dfs_packed_info.second_part_matches[j];

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
        if (k == NO_MATCH) {
            if constexpr (IsDebug) { std::cout << "End: " << i + 1 << " -> " << j + 1 << '\n'; }
            dfs_packed_info.second_part_matches[j] = i;
            dfs_packed_info.first_part_matches[i] = j;
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
        if (!dfs_packed_info.second_part_visited[j] && !dfs_packed_info.first_part_visited[k]) {
            dfs_packed_info.second_part_visited[j] = true;
            if constexpr (IsDebug) { std::cout << "Go: " << i + 1 << " -> " << j + 1 << " -> " << k + 1 << '\n'; }
            size_t end_vertex = DFSFindChainUpdateMatches<IsDebug>(k, dfs_packed_info);
            if constexpr (IsDebug) { std::cout << "After dfs: " << i + 1 << " -> " << j + 1 << " -> " << k + 1 << " -> " << end_vertex + 1 << '\n'; }
            if (end_vertex != NO_VERTEX) {
                dfs_packed_info.second_part_matches[j] = i;
                dfs_packed_info.first_part_matches[i] = j;
                return end_vertex;
            }
        }
    }

    return NO_VERTEX;
}

static void DfsFromUnmatched(size_t i, DFSPackedInfo& dfs_packed_info) noexcept {
    dfs_packed_info.first_part_visited[i] = true;
    const size_t* i_neighbours = dfs_packed_info.neighbours[i];

    for (size_t neighbour_index = 0, total_neighbours = dfs_packed_info.neighbours_count[i];
        neighbour_index < total_neighbours;
        ++neighbour_index) {
        size_t j = i_neighbours[neighbour_index];
        size_t k = dfs_packed_info.second_part_matches[j];
        if (k != NO_MATCH && !dfs_packed_info.first_part_visited[k]) {
            dfs_packed_info.second_part_visited[j] = true;
            DfsFromUnmatched(k, dfs_packed_info);
        }
    }
}

template <bool IsDebug = false, typename T>
requires std::is_integral_v<T>
static inline constexpr void FillBipartiteGraph(
    const T* const* matrix,
    size_t n,
    bool** bipartite_graph_matrix,
    size_t** neighbours,
    size_t* neighbours_count,
    size_t* first_part_matches,
    size_t* second_part_matches) noexcept {
    for (size_t i = 0; i < n; i++) {
        bool* bipartite_graph_matrix_row = bipartite_graph_matrix[i];
        std::memset(bipartite_graph_matrix_row, 0, sizeof(bool) * n);

        size_t* i_neighbours = neighbours[i];
        std::memset(i_neighbours, 0, sizeof(size_t) * n);

        size_t i_neighbours_count = 0;

        for (size_t j = 0; j < n; j++) {
            if (matrix[i][j] == 0) {
                bipartite_graph_matrix_row[j] = true;
                i_neighbours[i_neighbours_count] = j;
                i_neighbours_count++;
            }
        }

        neighbours_count[i] = i_neighbours_count;
    }
    
    for (size_t i = 0; i < n; i++) {
        first_part_matches[i] = NO_MATCH;
        second_part_matches[i] = NO_MATCH;
    }

    for (size_t i = 0; i < n; i++) {
        if (first_part_matches[i] != NO_MATCH) {
            if constexpr (IsDebug) { assert(second_part_matches[first_part_matches[i]] == i); }
            continue;
        }

        const size_t* i_neighbours = neighbours[i];
        size_t i_neighbours_count = neighbours_count[i];

        for (size_t neighbour_index = 0; neighbour_index < i_neighbours_count; ++neighbour_index) {
            size_t j = i_neighbours[neighbour_index];
            if constexpr (IsDebug) { assert(bipartite_graph_matrix[i][j]); }
            if (second_part_matches[j] == NO_MATCH) {
                second_part_matches[j] = i;
                first_part_matches[i] = j;
                break;
            }
        }
    }
}

static inline void PrintMatches(const size_t* first_part_matches, const size_t* second_part_matches, size_t n) try {
    std::cout << "\nInverse edges:\n";
    for (size_t __j = 0; __j < n; __j++) {
        size_t __i = second_part_matches[__j];
        if (__i != NO_MATCH) {
            std::cout << "Inverse edge " << __i + 1 << " <- " << __j + 1 << '\n';
            assert(first_part_matches[__i] == __j);
        }
    }
}
catch (...) {
}

template <typename T>
requires std::is_integral_v<T>
static inline void PrintMatrix(const T *const * matrix, size_t n) {
    std::cout << "\nMatrix:\n";
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            std::cout << matrix[i][j] << ' ';
        }
        std::cout << '\n';
    }
}

template <bool IsDebug = false, typename T>
requires std::is_integral_v<T>
static inline constexpr void MakeAlphaTransformation(T** matrix, size_t n, const bool* first_part_visited, const bool* second_part_visited) noexcept {
    T min = std::numeric_limits<T>::max();
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            bool x_i = first_part_visited[i];
            bool y_j = second_part_visited[j];
            if (x_i && !y_j) {
                T current = matrix[i][j];
                if constexpr (IsDebug) { assert(current != 0); }
                if (current < min) {
                    min = current;
                }
            }
        }
    }

    assert(min != std::numeric_limits<T>::max());
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            bool x_i = first_part_visited[i];
            bool y_j = second_part_visited[j];
            if (x_i != y_j) {
                if (x_i) {
                    matrix[i][j] -= min;
                }
                else {
                    matrix[i][j] += min;
                }
            }
        }
    }
}

template <bool IsDebug = false, typename T>
T HungarianAlgorithm(const std::vector<std::vector<T>>& original_matrix) {
    T** matrix = MakeCopyWithZeroOnRowsAndColumns(original_matrix);

    size_t n = original_matrix.size();
    bool** bipartite_graph_matrix = new bool*[n]();
    size_t** neighbours = new size_t*[n]();
    size_t* neighbours_count = new size_t[n]();
    for (size_t i = 0; i < n; i++) {
        bipartite_graph_matrix[i] = new bool[n]();
        neighbours[i] = new size_t[n]();
    }

    size_t* first_part_matches = new size_t[n]();
    size_t* second_part_matches = new size_t[n]();
    bool* first_part_visited = new bool[n]();
    bool* second_part_visited = new bool[n]();

    do {
        FillBipartiteGraph<IsDebug>(matrix, n, bipartite_graph_matrix, neighbours, neighbours_count, first_part_matches, second_part_matches);

        if constexpr (IsDebug) {
            PrintMatrix(matrix, n);
            PrintMatches(first_part_matches, second_part_matches, n);
        }

        DFSPackedInfo dfs_packed_info = {
            .first_part_matches = first_part_matches,
            .second_part_matches = second_part_matches,
            .neighbours = neighbours,
            .neighbours_count = neighbours_count,
            .first_part_visited = first_part_visited,
            .second_part_visited = second_part_visited
        };
        for (size_t i = 0; i < n; i++) {
            if (first_part_matches[i] == NO_MATCH) {
                std::memset(first_part_visited, false, sizeof(bool) * n);
                std::memset(second_part_visited, false, sizeof(bool) * n);
                DFSFindChainUpdateMatches<IsDebug>(i, dfs_packed_info);

                if constexpr (IsDebug) {
                    PrintMatches(first_part_matches, second_part_matches, n);
                }
            }
        }

        std::memset(first_part_visited, false, sizeof(bool) * n);
        std::memset(second_part_visited, false, sizeof(bool) * n);
        bool graph_satisfied = true;
        for (size_t i = 0; i < n; i++) {
            if (first_part_matches[i] == NO_MATCH) {
                graph_satisfied = false;
                DfsFromUnmatched(i, dfs_packed_info);
            }
        }

        if (!graph_satisfied) {
            MakeAlphaTransformation<IsDebug>(matrix, n, first_part_visited, second_part_visited);
        }
        else {
            break;
        }
    } while (true);

    delete[] second_part_visited;
    delete[] first_part_visited;
    delete[] neighbours_count;
    for (size_t i = 0; i < n; i++) {
        delete[] neighbours[i];
        delete[] bipartite_graph_matrix[i];
        delete[] matrix[i];
    }
    delete[] neighbours;
    delete[] bipartite_graph_matrix;
    delete[] matrix;

    T ans = 0;
    for (size_t i = 0; i < n; i++) {
        size_t j = first_part_matches[i];
        if constexpr (IsDebug) {
            assert(j != NO_MATCH && second_part_matches[j] == i);
        }
        ans += original_matrix[i][j];
    }

    delete[] second_part_matches;
    delete[] first_part_matches;

    return ans;
}

template <bool IsDebug = false>
static void TestHungarianAlgorithm() {
    std::vector<std::vector<std::vector<uint32_t>>> input {
        {
            { 1 }
        },
        {
            { 1, 6, 1 },
            { 3, 8, 5 },
            { 2, 7, 6 }
        },
        {
            { 32, 28,  4, 26,  4 },
            { 17, 19,  4, 17,  4 },
            {  4,  4,  5,  4,  4 },
            { 17, 14,  4, 14,  4 },
            { 21, 16,  4, 13,  4 }
        },
        {
            { 1, 1, 1, 0, 0, 0 },
            { 1, 1, 0, 1, 0, 0 },
            { 1, 0, 1, 1, 1, 0 },
            { 0, 1, 1, 1, 0, 0 },
            { 0, 0, 1, 0, 1, 1 },
            { 0, 0, 0, 0, 1, 1 }
        },
        {
            {61,80,89,22,41,76,79,62,4,58},
            {54,64,61,18,43,37,67,62,91,2},
            {23,87,35,1,39,90,72,51,15,96},
            {69,69,67,45,47,90,38,94,10,89},
            {64,47,50,79,64,86,9,41,91,46},
            {52,75,43,64,40,56,73,76,14,90},
            {73,79,98,49,39,39,87,75,57,63},
            {68,41,23,22,48,63,2,7,19,59},
            {36,25,45,11,25,11,96,15,22,27},
            {17,33,25,22,39,26,48,60,11,57},
        },
        {
            {10,64,15,53,93,95,90,7,38,42},
            {77,77,57,20,45,28,48,71,15,62},
            {61,43,12,59,53,30,81,24,70,62},
            {39,37,92,20,57,77,94,10,85,90},
            {33,30,40,93,46,20,69,81,66,39},
            {15,61,41,42,85,31,17,46,53,68},
            {11,88,7,57,67,69,60,55,63,1},  
            {58,24,72,44,67,81,28,58,31,5}, 
            {82,54,30,5,48,41,23,91,59,10}, 
            {21,76,10,71,11,23,79,18,8,33},
        },
        {
            {47,6,53,82,11,67,56,37,82,25},
            {75,35,63,16,44,75,58,53,94,26},
            {13,32,27,71,53,34,27,21,92,96},
            {46,7,62,76,76,36,33,72,17,38},
            {43,94,55,12,9,9,60,18,80,71},
            {2,54,84,11,60,75,48,32,76,23},
            {43,52,20,29,41,75,37,80,38,95},
            {92,23,28,18,25,90,84,35,97,83},
            {94,59,67,56,88,16,82,28,46,80},
            {75,76,86,2,79,1,49,8,72,69},
        },
        {
            {1,0,1,0,1,1,0,1,0,1},
            {1,1,1,0,0,1,0,1,0,0},
            {1,0,1,1,1,0,1,1,0,0},
            {0,1,0,0,0,0,1,0,1,0},
            {1,0,1,0,1,1,0,0,0,1},
            {0,0,0,1,0,1,0,0,0,1},
            {1,0,0,1,1,1,1,0,0,1},
            {0,1,0,0,1,0,0,1,1,1},
            {0,1,1,0,0,0,0,0,0,0},
            {1,0,0,0,1,1,1,0,0,1},
        }
    };

    uint32_t output[] = { 1, 11, 39, 0, 194, 125, 149, 0 };
    constexpr size_t total_tests = sizeof(output) / sizeof(output[0]);
    assert(input.size() == total_tests);

    for (size_t k = 0; k < total_tests; k++) {
        uint32_t ans = HungarianAlgorithm<IsDebug, uint32_t>(input[k]);
        std::cout << "Test " << (k + 1) << ((ans == output[k]) ? "" : " not") << " passed\nAlgorithm answer: " << ans << "\nCorrect answer: " << output[k] << '\n';
    }
}

int main() {
    TestHungarianAlgorithm<false>();
}
