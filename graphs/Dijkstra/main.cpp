#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace dijkstra {

using std::vector;
using vertex_t = size_t;
using weight_t = uint64_t;
using graph_t = vector<vector<std::pair<vertex_t, weight_t>>>;

inline constexpr weight_t kInfDist = weight_t(-1);
inline constexpr vertex_t kNoVertex = vertex_t(-1);

class heap {
public:
    struct node_t {
        weight_t dist;
        vertex_t vertex;
    };

    static constexpr heap from_graph(size_t graph_size, vertex_t start_vertex) {
        std::vector<node_t> heap_nodes;
        heap_nodes.reserve(graph_size);
        heap_nodes.resize(1);
        heap_nodes[0].dist = 0;
        heap_nodes[0].vertex = start_vertex;
        return heap(std::move(heap_nodes));
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

    constexpr node_t& top() noexcept {
        return heap_nodes_.front();
    }

    constexpr const node_t& top() const noexcept {
        return heap_nodes_.front();
    }

    void push(weight_t v_dist, vertex_t vertex) {
        // push elem to the end of the heap and sift upper.

        size_t elem_index = size();
        size_t parent_index = parentIndex(elem_index);
        heap_nodes_.emplace_back();
        while (elem_index != 0 && heap_nodes_[parent_index].dist > v_dist) {
            heap_nodes_[elem_index] = heap_nodes_[parent_index];
            elem_index = parent_index;
            parent_index = parentIndex(elem_index);
        }

        heap_nodes_[elem_index] = {v_dist, vertex};
    }

    void pop_top() noexcept {
        // Remove root, place last element on it's place
        // and return pyramide (heap) to the balanced state.
        assert(!empty());
        node_t sifting_elem = heap_nodes_.front() = heap_nodes_.back();
        size_t parent_index = 0;
        size_t son_index = leftSonIndex(parent_index);
        while (son_index + 1 < size()) {
            if (heap_nodes_[son_index].dist > heap_nodes_[son_index + 1].dist) {
                son_index++;
            }

            if (sifting_elem.dist <= heap_nodes_[son_index].dist) {
                break;
            }
            heap_nodes_[parent_index] = heap_nodes_[son_index];
            heap_nodes_[son_index] = sifting_elem;
            parent_index = son_index;
            son_index = leftSonIndex(parent_index);
        }

        heap_nodes_.pop_back();
    }

    constexpr void decrease_top_key(weight_t dist) noexcept {
        decrease_key(0, dist);
    }

    constexpr void decrease_key(size_t node_index, weight_t dist) noexcept {
        heap_nodes_[node_index].dist = dist;
        rebalance_heap(node_index);
    }

private:
    constexpr heap(std::vector<node_t>&& heap_nodes) noexcept : heap_nodes_(std::move(heap_nodes)) {}

    constexpr void rebalance_heap(size_t index) noexcept {
        size_t son_index = leftSonIndex(index);
        if (son_index >= size()) {
            return;
        }

        size_t parent_index = index;
        node_t sifting_elem = heap_nodes_[index];
        if (son_index + 1 != size() && heap_nodes_[son_index].dist > heap_nodes_[son_index + 1].dist) {
            son_index++;
        }

        while (sifting_elem.dist > heap_nodes_[son_index].dist) {
            heap_nodes_[parent_index] = heap_nodes_[son_index];
            heap_nodes_[son_index] = sifting_elem;

            parent_index = son_index;
            son_index = leftSonIndex(son_index);
            if (son_index >= size()) {
                return;
            }

            if (son_index + 1 != size() && heap_nodes_[son_index].dist > heap_nodes_[son_index + 1].dist) {
                son_index++;
            }
        }
    }

    static constexpr size_t parentIndex(size_t node_index) noexcept {
        return (node_index - 1) / 2;
    }

    static constexpr size_t leftSonIndex(size_t node_index) noexcept {
        return node_index * 2 | 1;
    }

    std::vector<node_t> heap_nodes_;
};

/// @brief O(|V|^2 + |E|) = O(|V|^2), better use when |E| = \Theta(|V^2|)
/// @param g graph = (V, E) with weight function
/// @param dist
/// @param ancestors
/// @param from start vertex, 0 <= from < |V|
static void find_shr_pths_highdensity(const graph_t& g,
                                      vector<weight_t>& dist,
                                      vector<vertex_t>& ancestors,
                                      uint32_t from) {
    const size_t n = g.size();
    assert(dist.size() == n && ancestors.size() == n);
    assert(from < n);
    vector<bool> visited(n);
    dist[from] = 0;
    while (true) {
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
static void find_shr_pths_lowdensity(const graph_t& g,
                                     vector<weight_t>& dist,
                                     vector<vertex_t>& ancestors,
                                     uint32_t from) {
    const size_t n = g.size();
    assert(dist.size() == n && ancestors.size() == n);
    assert(from < n);
    dist[from] = 0;
    vector<bool> visited(n);
    heap not_visited = heap::from_graph(n, from);
    assert(!not_visited.empty());
    do {
        weight_t vertex_dist;
        vertex_t vertex;
        do {
            vertex_dist = not_visited.top().dist;
            vertex = not_visited.top().vertex;
            not_visited.pop_top();
        } while (visited[vertex] && !not_visited.empty());
        if (visited[vertex]) {
            break;
        }

        for (auto [neighbour, weight] : g[vertex]) {
            weight_t path_via_vertex = vertex_dist + weight;
            if (dist[neighbour] > path_via_vertex) {
                dist[neighbour] = path_via_vertex;
                ancestors[neighbour] = vertex;
                not_visited.push(path_via_vertex, neighbour);
            }
        }

        visited[vertex] = true;
    } while (!not_visited.empty());
}

static uint32_t log2_floor(size_t n) noexcept {
    return 63 ^ uint32_t(__builtin_clzll(n | 1));
}

template <size_t C = 2>
std::pair<vector<weight_t>, vector<vertex_t>> shortest_paths(const graph_t& g, uint32_t from) {
    const size_t n = g.size();
    size_t edges = 0;
    for (const auto& neighbours : g) {
        edges += neighbours.size();
    }
    vector<weight_t> dist(n, kInfDist);
    vector<vertex_t> ancestors(n, kNoVertex);
    if (edges * log2_floor(n) <= n * n * C) {
        find_shr_pths_lowdensity(g, dist, ancestors, from);
    } else {
        find_shr_pths_highdensity(g, dist, ancestors, from);
    }
    return {dist, ancestors};
}

}  // namespace dijkstra

int main() {
    using namespace dijkstra;
    graph_t g = {
        {{1, 1}, {2, 1}},
        {{0, 4}, {2, 1}},
        {{0, 2}, {1, 1}},
    };

    {
        auto [dist, ancestors] = shortest_paths(g, 0);
        assert(dist[0] == 0);
        assert(dist[1] == 1);
        assert(dist[2] == 1);
        assert(ancestors[0] == kNoVertex);
        assert(ancestors[1] == 0);
        assert(ancestors[2] == 0);
    }
    {
        auto [dist, ancestors] = shortest_paths(g, 1);
        assert(dist[0] == 3);
        assert(dist[1] == 0);
        assert(dist[2] == 1);
        assert(ancestors[0] == 2);
        assert(ancestors[1] == kNoVertex);
        assert(ancestors[2] == 1);
    }
    {
        auto [dist, ancestors] = shortest_paths(g, 2);
        assert(dist[0] == 2);
        assert(dist[1] == 1);
        assert(dist[2] == 0);
        assert(ancestors[0] == 2);
        assert(ancestors[1] == 2);
        assert(ancestors[2] == kNoVertex);
    }

    g.clear();
    g.resize(6);
    for (auto [u, v, w] : std::vector<std::tuple<uint32_t, uint32_t, uint32_t>>{
             {1, 2, 7},
             {2, 4, 8},
             {4, 5, 1},
             {4, 3, 100},
         }) {
        u--;
        v--;
        g[u].emplace_back(vertex_t(v), weight_t(w));
        g[v].emplace_back(vertex_t(u), weight_t(w));
    }
    {
        auto [dist, ancestors] = shortest_paths(g, 2);
        assert(dist[0] == 115);
    }
}
