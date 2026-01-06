#pragma once

#include <cstdint>
#include <utility>
#include <vector>

using std::pair;
using std::vector;
using vertex_t = int64_t;
using weight_t = int64_t;

constexpr weight_t inf = 1ll << 60;

vector<weight_t> bellman_ford(const vector<vector<pair<vertex_t, weight_t>>>& g, vertex_t start) {
    const size_t n = g.size();
    vector<weight_t> dist(n, inf);
    dist[start] = 0;
    for (size_t i = 0; i < n; i++) {
        for (size_t u = 0; u < n; u++) {
            for (auto [v, w] : g[u]) {
                dist[v] = std::min(dist[v], dist[u] + w);
            }
        }
    }

    for (size_t u = 0; u < n; u++) {
        for (auto [v, w] : g[u]) {
            if (dist[u] + w < dist[v]) {
                dist.clear();
                u = n;
                break;
            }
        }
    }

    return dist;
}
