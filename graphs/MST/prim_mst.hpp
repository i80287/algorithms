#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>
#include <set>
#include <vector>

using std::vector;
using vertex_t = int64_t;
using weight_t = int64_t;
using graph_t = vector<vector<vertex_t>>;

vector<vertex_t> prim_mst(const graph_t& g, const vector<vector<weight_t>>& weights) {
    const size_t n = g.size();
    vector<uint8_t> in_queue(n, true);
    vector<vertex_t> prnt(n, vertex_t(-1));
    vector<weight_t> key(n, std::numeric_limits<weight_t>::max());
    key[0] = 0;
    std::set<std::pair<weight_t, vertex_t>> q;
    for (vertex_t v = 0; v < n; v++) {
        auto [_, inserted] = q.insert({key[v], v});
    }

    do {
        auto min_iter = q.begin();
        vertex_t v = min_iter->second;
        assert(min_iter->first == key[v]);
        q.extract(min_iter);
        in_queue[v] = false;
        for (vertex_t u : g[v]) {
            if (in_queue[u] && key[u] > weights[v][u]) {
                prnt[u] = v;
                auto node = q.extract({key[u], u});
                assert(!node.empty());
                assert(node.value().first == key[u]);
                assert(node.value().second == u);
                key[u] = weights[v][u];
                q.insert({key[u], u});
            }
        }
    } while (!q.empty());

    return prnt;
}
