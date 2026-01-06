#pragma once
#include <set>

#include "../../DisjointSetUnion/dsu.hpp"

using vertex_t = int64_t;
using edge_t = std::pair<vertex_t, vertex_t>;

template <class Comp>
[[nodiscard]] inline std::vector<edge_t> kruskal_mst(const std::set<edge_t, Comp>& edges, const size_t n) {
    dsu_t dsu = dsu_t::with_nodes_count(n);
    std::vector<edge_t> mst;
    mst.reserve(n - 1);
    for (auto [u, v] : edges) {
        if (!dsu.equal(u, v)) {
            mst.push_back({u, v});
            dsu.unite(u, v);
        }
    }
    return mst;
}
