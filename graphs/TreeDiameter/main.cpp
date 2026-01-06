#include <cassert>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iostream>
#include <vector>

using graph_t = std::vector<std::vector<uint32_t>>;
using vertex_t = uint32_t;

std::vector<uint32_t> GetDistancesFromVertex(const graph_t& graph, vertex_t start_vertex) {
    std::vector<bool> visited(graph.size());
    std::vector<uint32_t> distances(graph.size());
    // distances[start_vertex] = 0;

    std::deque<std::pair<vertex_t, uint32_t>> bfs_queue;
    bfs_queue.emplace_back(start_vertex, 0u);

    do {
        auto [vertex, bfs_depth] = bfs_queue.front();
        bfs_queue.pop_front();

        visited.at(vertex) = true;
        distances[vertex] = bfs_depth;

        ++bfs_depth;
        for (vertex_t neighbour_vertex : graph.at(vertex)) {
            if (!visited.at(neighbour_vertex)) {
                bfs_queue.emplace_back(neighbour_vertex, bfs_depth);
            }
        }
    } while (!bfs_queue.empty());

    return distances;
}

uint32_t GetTreeDiameter(const graph_t& tree) {
    constexpr vertex_t start_vertex = 1;
    std::vector<uint32_t> distances = GetDistancesFromVertex(tree, start_vertex);
    assert(distances.size() == tree.size() && distances[start_vertex] == 0);

    vertex_t farest_vertex = start_vertex;
    for (vertex_t vertex = start_vertex + 1, total_vertices = static_cast<uint32_t>(tree.size() - 1);
         vertex <= total_vertices; ++vertex) {
        if (distances.at(vertex) > distances.at(farest_vertex)) {
            farest_vertex = vertex;
        }
    }

    distances = GetDistancesFromVertex(tree, farest_vertex);
    farest_vertex = start_vertex;
    for (vertex_t vertex = start_vertex + 1, total_vertices = static_cast<uint32_t>(tree.size() - 1);
         vertex <= total_vertices; ++vertex) {
        if (distances[vertex] > distances[farest_vertex]) {
            farest_vertex = vertex;
        }
    }

    return distances[farest_vertex];
}

int main() {
    uint32_t n = 0;
    std::cin >> n;

    graph_t graph(n + 1);
    for (uint32_t i = 1; i < n; ++i) {
        vertex_t b1 = 0, b2 = 0;
        std::cin >> b1 >> b2;
        graph.at(b1).push_back(b2);
        graph.at(b2).push_back(b1);
    }

    std::cout << GetTreeDiameter(graph) + 1 << '\n';
}
