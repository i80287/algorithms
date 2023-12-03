#include <cassert>
#include <vector>
#include <cstdint>
#include <cstring>
#include <iostream>

/// @brief seealso https://e-maxx.ru/algo/matching_edmonds
namespace EdmondsMatchingAlgorithm {

using vertex_t = size_t;
using graph_t = std::vector<std::vector<vertex_t>>;

inline constexpr vertex_t MAX_GRAPH_SIZE = 128;

inline constexpr vertex_t NO_VERTEX = static_cast<vertex_t>(-1);

// Parent tree for odd vertexes
static vertex_t parent[MAX_GRAPH_SIZE] = {};

// blossom_cycle_base[i] is a vertex number that is base of the blossom for the vertex i
static vertex_t blossom_cycle_base[MAX_GRAPH_SIZE] = {};

static inline vertex_t find_lca(const std::vector<vertex_t>& matches, vertex_t vertex1, vertex_t vertex2) {
    static bool used_in_cycle[MAX_GRAPH_SIZE];
    memset(used_in_cycle, 0, sizeof(used_in_cycle));

    // Go up from vertex1 to root, marking all even vertexes
    while (true) {
        vertex1 = blossom_cycle_base[vertex1];
        used_in_cycle[vertex1] = true;
        if (matches[vertex1] == NO_VERTEX) {
            // vertex1 is a root
            break;
        }

        vertex1 = parent[matches[vertex1]];
    }

    while (true) {
        vertex2 = blossom_cycle_base[vertex2];
        if (used_in_cycle[vertex2]) {
            // vertex2 now is LCA of input vertex1 and vertex2
            return vertex2;
        }

        vertex_t m = matches[vertex2];
        assert(m != NO_VERTEX);
        vertex2 = parent[m];
    }
}

static inline void mark_path_in_cycle(const std::vector<vertex_t>& matches, bool current_blossom_cycle_vertexes[], vertex_t v, vertex_t lca_base, vertex_t child) {
    while (blossom_cycle_base[v] != lca_base) {
        assert(matches[v] != NO_VERTEX);
        current_blossom_cycle_vertexes[blossom_cycle_base[v]] = true;
        current_blossom_cycle_vertexes[blossom_cycle_base[matches[v]]] = true;
        parent[v] = child;
        child = matches[v];
        assert(child != NO_VERTEX);
        v = parent[matches[v]];
    }
}

static vertex_t find_increasing_path(const graph_t& graph, const std::vector<vertex_t>& matches, vertex_t n, vertex_t root) {
    for (vertex_t i = 0; i < n; i++) { blossom_cycle_base[i] = i; }
    std::memset(parent, static_cast<int>(NO_VERTEX), sizeof(parent));

    constexpr size_t QUEUE_MAX_SIZE = MAX_GRAPH_SIZE + MAX_GRAPH_SIZE;

    // Queue with even vertexes
    static vertex_t queue[QUEUE_MAX_SIZE] = {};
    memset(queue, 0, sizeof(queue));
    size_t queue_head = 0;
    size_t queue_tail = 0;
    queue[queue_tail++] = root;

    static bool used[MAX_GRAPH_SIZE];
    memset(used, false, sizeof(used));
    used[root] = true;

    while (queue_head < queue_tail) {
        assert(queue_head < QUEUE_MAX_SIZE);
        vertex_t v = queue[queue_head++];

        for (vertex_t to : graph[v]) {
            // (v, to) - edge in BFS

            // egde (v, to) does not belongs to E || (v, to) is already in matches (v - even vertex)
            if (blossom_cycle_base[v] == blossom_cycle_base[to] || matches[v] == to) {
                continue;
            }

            if (to == root || (matches[to] != NO_VERTEX && parent[matches[to]] != NO_VERTEX)) {
                // Odd length cycle, blossom found

                // compress blossom
                vertex_t current_base = find_lca(matches, v, to);

                // Array of vertexes belongs to the current blossom
                static bool current_blossom_cycle_vertexes[MAX_GRAPH_SIZE];
                std::memset(current_blossom_cycle_vertexes, false, sizeof(current_blossom_cycle_vertexes));

                mark_path_in_cycle(matches, current_blossom_cycle_vertexes, v, current_base, to);
                mark_path_in_cycle(matches, current_blossom_cycle_vertexes, to, current_base, v);

                for (vertex_t u = 0; u < n; u++) {
                    if (current_blossom_cycle_vertexes[blossom_cycle_base[u]]) {
                        blossom_cycle_base[u] = current_base;
                        if (!used[u]) {
                            used[u] = true;
                            assert(queue_tail < QUEUE_MAX_SIZE);
                            queue[queue_tail++] = u;
                        }
                    }
                }
            }
            else if (parent[to] == NO_VERTEX) {
                parent[to] = v;
                if (matches[to] == NO_VERTEX) {
                    return to;
                }

                to = matches[to];
                assert(to != NO_VERTEX);
                used[to] = true;
                assert(queue_tail < QUEUE_MAX_SIZE);
                queue[queue_tail++] = to;
            }
        }
    }

    return NO_VERTEX;
}

std::vector<vertex_t> RunEdmondsMatchingAlgorithm(const graph_t& graph) {
    vertex_t n = static_cast<vertex_t>(graph.size());
    assert(n <= MAX_GRAPH_SIZE);

    std::vector<vertex_t> matches(n, NO_VERTEX);

    // Generate first max match
    for (vertex_t v = 0; v < n; v++) {
        if (matches[v] == NO_VERTEX) {
            for (vertex_t u : graph[v]) {
                if (matches[u] == NO_VERTEX) {
                    matches[u] = v;
                    matches[v] = u;
                    break;
                }
            }
        }
    }

    for (vertex_t u = 0; u < n; u++) {
        if (matches[u] == NO_VERTEX) {
            vertex_t v = find_increasing_path(graph, matches, n, u);
            while (v != NO_VERTEX) {
                vertex_t parent_v = parent[v];
                assert(parent_v != NO_VERTEX);
                vertex_t parent_parent_v = matches[parent_v];
                matches[v] = parent_v;
                matches[parent_v] = v;
                v = parent_parent_v;
            }
        }
    }

    return matches;
}

void print_matches(const std::vector<vertex_t>& matches) {
    printf("\nMatches:\n");
    for (vertex_t u = 0, n = static_cast<vertex_t>(matches.size()); u < n; u++) {
        vertex_t v = matches[u];
        if (v != NO_VERTEX) {
            std::cout << u << " -> " << v << '\n';
        }
        else {
            std::cout << u << " -> no match\n";
        }
    }
}

} // namespace EdmondsMatchingAlgorithm

int main() {
    using namespace EdmondsMatchingAlgorithm;

    {
        graph_t graph {
            { 1, 2, 3, 4 },
            { 0, 2, 5 },
            { 0, 1 },
            { 0, 5 },
            { 0, 5 },
            { 1, 3, 4 },
            { 7 },
            { 6 }
        };

        std::vector<vertex_t> matches = RunEdmondsMatchingAlgorithm(graph);
        print_matches(matches);
    }

    {
        graph_t graph {
            { 1 },
            { 0, 2 },
            { 1 }
        };

        std::vector<vertex_t> matches = RunEdmondsMatchingAlgorithm(graph);
        print_matches(matches);
    }

    return 0;
}
