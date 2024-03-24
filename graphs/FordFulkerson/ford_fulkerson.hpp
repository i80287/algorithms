#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <utility>
#include <vector>

using std::vector;

using vertex_t = int64_t;
using weight_t = int64_t;

weight_t ford_fulkerson(const vector<vector<weight_t>>& capacity) {
    const size_t n = capacity.size();
    vector<vector<weight_t>> flow(n, vector<weight_t>(n));

    vector<vertex_t> q(n);
    vector<vertex_t> parent(n);
    for (;;) {
        std::fill(parent.begin(), parent.end(), vertex_t(-1));
        size_t h  = 0;
        size_t t  = 0;
        q[t++]    = 0;
        parent[0] = 0;
        while (h < t) {
            vertex_t u = q[h++];
            for (vertex_t v = 0; v < n; v++) {
                if (parent[v] == vertex_t(-1) && flow[u][v] < capacity[u][v]) {
                    q[t++]    = v;
                    parent[v] = u;
                }
            }
        }

        if (parent[n - 1] == vertex_t(-1)) {
            break;
        }

        weight_t curflow = std::numeric_limits<weight_t>::max();
        for (vertex_t v = n - 1; v > 0;) {
            vertex_t p = parent[v];
            curflow    = std::min(curflow, capacity[p][v] - flow[p][v]);
            v          = p;
        }
        for (vertex_t v = n - 1; v != 0;) {
            vertex_t p = parent[v];
            flow[p][v] += curflow;
            flow[v][p] -= curflow;
            v = p;
        }
    }

    weight_t maxflow = 0;
    for (vertex_t v = 0; v < n; v++) {
        if (capacity[0][v] > 0) {
            maxflow += flow[0][v];
        }
    }
    return maxflow;
}
