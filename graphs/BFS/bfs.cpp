#include <iostream>
#include <cstdint>
#include <vector>
#include <deque>

using std::vector;

uint32_t find_distance(vector<vector<uint32_t>> const& graph, uint32_t start, uint32_t end) {
    std::deque<uint32_t> queue;

    vector<uint32_t> distances(graph.size(), UINT32_MAX);
    distances[start] = 0;
    queue.push_back(start);

    do {
        uint32_t v = queue.front();
        queue.pop_front();
        uint32_t distance_to_v = distances[v];
        
        for (uint32_t child : graph[v]) {
            if (distances[child] == UINT32_MAX) {
                distances[child] = distance_to_v + 1;
                queue.push_back(child);
            }
        }
    } while (!queue.empty());

    return distances[end];
}

int main() {
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

    std::cout << find_distance(graph, 1, 7);
}