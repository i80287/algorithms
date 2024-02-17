#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace kosaraju_algorithm {

using std::vector;

// Min vertex is 0, max is g.size() - 1
using vertex_t = std::size_t;
using graph_t  = vector<vector<vertex_t>>;

namespace impl {

static void fill_stack_dfs(const graph_t& g, vector<uint8_t>& visited,
                           vector<vertex_t>& order_stack, vertex_t v) noexcept {
    visited[v] = true;
    for (vertex_t u : g[v]) {
        if (!visited[u]) {
            fill_stack_dfs(g, visited, order_stack, u);
        }
    }
    assert(order_stack.capacity() >= g.size());
    order_stack.push_back(v);
}

static void form_component_dfs(const graph_t& tr_g, vector<uint8_t>& visited,
                               vector<vertex_t>& component, vertex_t v) noexcept {
    visited[v] = true;
    assert(component.capacity() >= tr_g.size());
    component.push_back(v);
    for (vertex_t u : tr_g[v]) {
        if (!visited[u]) {
            form_component_dfs(tr_g, visited, component, u);
        }
    }
}

}  // namespace impl

template <bool ShrinkComponents = false>
graph_t strongly_connected_components(const graph_t& g) {
    graph_t components;
    const auto n = g.size();

    vector<std::uint8_t> visited(n, false);
    vector<vertex_t> order_stack;
    order_stack.reserve(n);
    for (vertex_t v = 0; v < n; v++) {
        if (!visited[v]) {
            impl::fill_stack_dfs(g, visited, order_stack, v);
        }
    }
    assert(order_stack.size() == n);
    std::fill(visited.begin(), visited.end(), false);

    graph_t transposed_g(n);

    std::vector<vertex_t> current_component;
    current_component.reserve(n);

    for (vertex_t v = 0; v < n; v++) {
        for (vertex_t u : g[v]) {
            transposed_g[u].push_back(v);
        }
    }

    for (auto iter = order_stack.crbegin(), end = order_stack.crend(); iter != end;
         ++iter) {
        vertex_t v = *iter;
        if (!visited[v]) {
            impl::form_component_dfs(transposed_g, visited, current_component, v);
            // Use O(E) memory instead of O(V^2)
            std::vector<vertex_t>& new_component =
                components.emplace_back(current_component.size());
            std::copy(current_component.cbegin(), current_component.cend(),
                      new_component.begin());
            current_component.clear();
        }
    }

    return components;
}

}  // namespace kosaraju_algorithm

#include <algorithm>

int main() {
    // 0 <-> 2
    //  \   ^
    //  v  /
    //   1
    //
    // 3 <- 5
    //  \  ^
    //  v /
    //   4
    //
    // 6
    kosaraju_algorithm::graph_t g = {
        {1, 2}, {2}, {0}, {4}, {5}, {3}, {},
    };
    auto components = kosaraju_algorithm::strongly_connected_components(g);
    assert(components.size() == 3);
    for (auto& c : components) {
        std::sort(c.begin(), c.end());
    }
    std::sort(components.begin(), components.end());
    bool r = components[0] == std::vector<kosaraju_algorithm::vertex_t>{ 0, 1, 2 };
    assert(r);
    r = components[1] == std::vector<kosaraju_algorithm::vertex_t>{ 3, 4, 5 };
    assert(r);
    r = components[2] == std::vector<kosaraju_algorithm::vertex_t>{ 6 };
    assert(r);
}
