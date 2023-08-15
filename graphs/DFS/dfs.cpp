#include <cstdint>
#include <vector>
#include <iostream>

using std::vector;

inline void dfs(uint32_t v, vector<vector<uint32_t>> const& graph, vector<bool>& visited) {
    visited[v] = true;
    for (uint32_t child : graph[v]) {
        if (!visited[child]) {
            dfs(child, graph, visited);
        }
    }
}

int main(void) {
    vector<vector<uint32_t>> graph = {
        {1, 2, 3},
        {0, 2, 3},
        {0, 1, 4, 6},
        {0, 1, 5},
        {2},
        {3, 7},
        {2, 7},
        {5, 6}
    };

    vector<bool> visited(graph.size(), false);
    dfs(1, graph, visited);
}
