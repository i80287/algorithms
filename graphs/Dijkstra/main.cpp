#include <cassert>
#include <cstdint>
#include <cstddef>
#include <vector>

namespace dijkstra {

using std::vector;
using vertex_t = size_t;
using weight_t = uint64_t;
using graph_t = vector<vector<std::pair<vertex_t, weight_t>>>;

inline constexpr weight_t kInfDist = weight_t(-1);
inline constexpr vertex_t kNoVertex = vertex_t(-1);

class DHeap {
public:
    using Node = std::pair<weight_t, vertex_t>;

    static constexpr weight_t Dist(const Node& node) noexcept {
        return node.first;
    }

    constexpr void reserve(size_t size) {
        heap_nodes_.reserve(size);
    }

    constexpr size_t size() const noexcept {
        return heap_nodes_.size();
    }

    constexpr bool empty() const noexcept {
        return heap_nodes_.empty();
    }

    constexpr Node& Top() noexcept {
        return heap_nodes_.front();
    }

    constexpr const Node& Top() const noexcept {
        return heap_nodes_.front();
    }

    constexpr const vector<Node>& Nodes() const noexcept {
        return heap_nodes_;
    }

    constexpr vector<Node>& Nodes() noexcept {
        return heap_nodes_;
    }

    void Add(weight_t dist, vertex_t vertex) {
        // Add elem to the end of the heap and sift upper.

        size_t elem_index = size();
        size_t parent_index = ParentIndex(elem_index);
        heap_nodes_.emplace_back();
        while (elem_index != 0 && Dist(heap_nodes_[parent_index]) > dist) {
            heap_nodes_[elem_index] = heap_nodes_[parent_index];
            elem_index = parent_index;
            parent_index = ParentIndex(elem_index);
        }

        heap_nodes_[elem_index] = { dist, vertex };
    }

    void PopTop() noexcept {
        // Remove root, place last element on it's place
        // and return pyramide (heap) to the balanced state.
        assert(!empty());
        Node sifting_elem = heap_nodes_.front() = heap_nodes_.back();
        size_t parent_index = 0;
        size_t son_index = LeftSonIndex(parent_index);
        while (son_index + 1 < size()) {
            if (Dist(heap_nodes_[son_index]) > Dist(heap_nodes_[son_index + 1])) {
                son_index++;
            }

            if (Dist(sifting_elem) > Dist(heap_nodes_[son_index])) {
                heap_nodes_[parent_index] = heap_nodes_[son_index];
                heap_nodes_[son_index] = sifting_elem;
                parent_index = son_index;
                son_index = LeftSonIndex(parent_index);
            }
            else {
                break;
            }
        }

        heap_nodes_.pop_back();
    }

    void Build(size_t graph_size, vertex_t start_vertex) {
        heap_nodes_.resize(graph_size);
        for (vertex_t vertex = 1; size_t(vertex) < graph_size; vertex++) {
            heap_nodes_[vertex].first = kInfDist;
            heap_nodes_[vertex].second = vertex;
        }
        heap_nodes_[start_vertex].second = 0;
        heap_nodes_[0].first = 0;
        heap_nodes_[0].second = start_vertex;
    }

    constexpr void SetTopDist(weight_t dist) noexcept {
        SetDist(0, dist);
    }

    constexpr void DecreaseDist(size_t node_index, weight_t dist) noexcept {
        SetDist(node_index, Dist(heap_nodes_[node_index]) - dist);
    }

    constexpr void SetDist(size_t node_index, weight_t dist) noexcept {
        heap_nodes_[node_index].first = dist;
        RebalanceHeap(node_index);
    }
protected:
    constexpr void RebalanceHeap(size_t index) noexcept {
        size_t son_index = LeftSonIndex(index);
        if (son_index >= size()) {
            return;
        }

        size_t parent_index = index;
        Node sifting_elem = heap_nodes_[index];
        if (son_index + 1 != size() && Dist(heap_nodes_[son_index]) > Dist(heap_nodes_[son_index + 1])) {
            son_index++;
        }

        while (Dist(sifting_elem) > Dist(heap_nodes_[son_index])) {
            heap_nodes_[parent_index] = heap_nodes_[son_index];
            heap_nodes_[son_index] = sifting_elem;

            parent_index = son_index;
            son_index = LeftSonIndex(son_index);
            if (son_index >= size()) {
                return;
            }

            if (son_index + 1 != size() && Dist(heap_nodes_[son_index]) > Dist(heap_nodes_[son_index + 1])) {
                son_index++;
            }
        }
    }

    static constexpr size_t ParentIndex(size_t node_index) noexcept {
        return (node_index - 1) >> 1;
    }

    static constexpr size_t LeftSonIndex(size_t node_index) noexcept {
        return node_index << 1 | 1;
    }

    std::vector<Node> heap_nodes_;
};

/// @brief O(|V|^2 + |E|) = O(|V|^2), better use when |E| = \Theta(|V^2|)
/// @param g graph = (V, E) with weight function
/// @param dist 
/// @param ancestors 
/// @param from start vertex, 0 <= from < |V|
static void FindShortestPathsHighDenisty(const graph_t& g, vector<weight_t>& dist, vector<vertex_t>& ancestors, uint32_t from) {
    const size_t n = g.size();
    assert(dist.size() == n && ancestors.size() == n);
    assert(from < n);
    vector<bool> visited(n);
    dist[from] = 0;
    while(true) {
        size_t vertex = n;
        weight_t min_dist = kInfDist;
        for (size_t v = 0; v < n; v++) {
            if (!visited[v] && dist[v] < min_dist) {
                min_dist = dist[v];
                vertex = v;
            }
        }

        if (vertex == n) {
            break;
        }

        for (auto [neighbour, weight] : g[vertex]) {
            weight_t path_via_vertex = min_dist + weight;
            if (dist[neighbour] > path_via_vertex) {
                dist[neighbour] = path_via_vertex;
                ancestors[neighbour] = vertex_t(vertex);
            }
        }

        visited[vertex] = true;
    }
}

/// @brief O(|V| + |E| log|E|) = O(|E| log|V|), better use when |E| = \O(|V^2| / log|V|)
/// @param g graph = (V, E) with weight function
/// @param dist 
/// @param ancestors 
/// @param from start vertex, 0 <= from < |V|
static void FindShortestPathsLowDenisty(const graph_t& g, vector<weight_t>& dist, vector<vertex_t>& ancestors, uint32_t from) {
    const size_t n = g.size();
    assert(dist.size() == n && ancestors.size() == n);
    assert(from < n);
    dist[from] = 0;
    vector<bool> visited(n);
    DHeap not_visited;
    not_visited.Build(n, from);
    assert(!not_visited.empty());
    while (true) {
        weight_t vertex_dist;
        vertex_t vertex;
        do {
            vertex_dist = not_visited.Top().first;
            vertex = not_visited.Top().second;
            not_visited.PopTop();
        } while (visited[vertex] && !not_visited.empty());

        if (vertex_dist == kInfDist || not_visited.empty()) {
            break;
        }

        for (auto [neighbour, weight] : g[vertex]) {
            weight_t path_via_vertex = vertex_dist + weight;
            if (dist[neighbour] > path_via_vertex) {
                dist[neighbour] = path_via_vertex;
                ancestors[neighbour] = vertex;
                not_visited.Add(path_via_vertex, neighbour);
            }
        }

        visited[vertex] = true;
    }
}

static uint32_t log2_floor(size_t n) noexcept {
    return 63 ^ uint32_t(__builtin_clzll(n | 1));
}

template <size_t C = 2>
std::pair<vector<weight_t>, vector<vertex_t>> FindShortestPaths(const graph_t& g, uint32_t from) {
    const size_t n = g.size();
    size_t edges = 0;
    for (const auto& neighbours : g) {
        edges += neighbours.size();
    }
    vector<weight_t> dist(n, kInfDist);
    vector<vertex_t> ancestors(n, kNoVertex);
    if (edges * log2_floor(n) <= n * n * C) {
        FindShortestPathsLowDenisty(g, dist, ancestors, from);
    }
    else {
        FindShortestPathsHighDenisty(g, dist, ancestors, from);
    }
    return { dist, ancestors };
}

} // namespace dijkstra

int main() {
    using namespace dijkstra;
    graph_t g = {
        { { 1, 1 }, { 2, 1 } },
        { { 0, 4 }, { 2, 1 } },
        { { 0, 2 }, { 1, 1 } },
    };

    {
        auto [dist, ancestors] = FindShortestPaths(g, 0);
        assert(dist[0] == 0);
        assert(dist[1] == 1);
        assert(dist[2] == 1);
        assert(ancestors[0] == kNoVertex);
        assert(ancestors[1] == 0);
        assert(ancestors[2] == 0);
    }
    {
        auto [dist, ancestors] = FindShortestPaths(g, 1);
        assert(dist[0] == 3);
        assert(dist[1] == 0);
        assert(dist[2] == 1);
        assert(ancestors[0] == 2);
        assert(ancestors[1] == kNoVertex);
        assert(ancestors[2] == 1);
    }
    {
        auto [dist, ancestors] = FindShortestPaths(g, 2);
        assert(dist[0] == 2);
        assert(dist[1] == 1);
        assert(dist[2] == 0);
        assert(ancestors[0] == 2);
        assert(ancestors[1] == 2);
        assert(ancestors[2] == kNoVertex);
    }
}
