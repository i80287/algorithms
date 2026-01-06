#pragma once
#include <cstdint>
#include <utility>
#include <vector>

using std::vector;

constexpr int64_t inf = 1ll << 60;

template <class weight_t>
vector<vector<weight_t>> min_dists_floyd_warshall(const vector<vector<weight_t>>& w) {
    vector<vector<weight_t>> dist = w;
    const auto n = w.size();
    for (size_t i = 0; i < n; i++) {
        dist[i][i] = 0;
    }

    for (size_t k = 0; k < n; k++) {
        for (size_t i = 0; i < n; i++) {
            for (size_t j = 0; j < n; j++) {
                dist[i][j] = std::min(dist[i][j], dist[i][k] + dist[k][j]);
            }
        }
    }

    return dist;
}
