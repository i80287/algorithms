#include "polygon.h"

#include <cassert>
#include <climits>
#include <cmath>

#include "segment.h"

#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

namespace geometry {

constexpr uint64_t Uabs(int64_t value) noexcept {
    return static_cast<uint64_t>(value >= 0 ? value : -value);
}

Polygon& Polygon::Move(const Vector& vector) noexcept {
    for (Point& vertex : vertexes_) {
        vertex.Move(vector);
    }

    return *this;
}

bool Polygon::ContainsPoint(const Point& point) const noexcept {
    bool contains_point = false;
    const size_t n = vertexes_.size();
    for (size_t i = 0; i < n; ++i) {
        contains_point = false;
        size_t next_i = i + 1 < n ? i + 1 : 0;
        const size_t termination_index = next_i;

        while (!contains_point) {
            size_t next_next_i = next_i + 1 < n ? next_i + 1 : 0;
            if (next_next_i == termination_index) {
                break;
            }

            Vector vec_i_next_i = vertexes_[next_i] - vertexes_[i];
            Vector vec_next_i_next_next_i = vertexes_[next_next_i] - vertexes_[next_i];
            Vector vec_next_next_i_p = point - vertexes_[next_next_i];
            Vector vec_i_p = vertexes_[i] - point;

            uint64_t double_total_square = Uabs(VectorMult(vec_i_next_i, vec_next_i_next_next_i));
            uint64_t double_square1 = Uabs(VectorMult(vec_i_p, vec_i_next_i));
            uint64_t double_square2 = Uabs(VectorMult(vec_next_i_next_next_i, vec_next_next_i_p));
            uint64_t double_square3 = Uabs(VectorMult(vec_next_next_i_p, vec_i_p));
            if (double_square1 + double_square2 + double_square3 == double_total_square) {
                contains_point = true;
                break;
            }

            next_i = next_i + 1 < n ? next_i + 1 : 0;
        }

        if (!contains_point) {
            break;
        }
    }

    return contains_point;
}

bool Polygon::SimplePolygonContainsPoint(const Point& point) const noexcept {
    int64_t p_x = point.GetX();
    int64_t p_y = point.GetY();
    int64_t max_x = LONG_LONG_MIN;
    for (const Point& vertex : vertexes_) {
        max_x = std::max(vertex.GetX(), max_x);
        if (unlikely(vertex.GetX() == p_x && vertex.GetY() == p_y)) {
            return true;
        }
    }

    if (p_x > max_x) {
        return false;
    }

    Segment points_long_segment(point, Point(max_x + 1, p_y + 1));
    size_t count = 0;
    for (size_t i = 0, n = vertexes_.size(); i < n; i++) {
        Segment edge(vertexes_[i], vertexes_[(i + 1) % n]);
        if (unlikely(edge.ContainsPoint(point))) {
            return true;
        }

        bool crosses = points_long_segment.CrossesSegment(edge);
        count += crosses;
    }

    return count % 2 != 0;
}

static bool SimpleConvexPolygonContainsPointRecursive(const std::vector<Point>& vertexes,
                                                      const Point& point,
                                                      size_t l,
                                                      size_t r) noexcept {
    assert(l <= r);
    switch (r - l) {
        case 0: {
            return Segment(vertexes[0], vertexes[l]).ContainsPoint(point);
        }
        case 1: {
            int64_t max_x = LONG_LONG_MIN;
            int64_t p_x = point.GetX();
            int64_t p_y = point.GetY();

            {
                int64_t v_x = vertexes[0].GetX();
                int64_t v_y = vertexes[0].GetY();
                max_x = std::max(v_x, max_x);
                if (unlikely(v_x == p_x && v_y == p_y)) {
                    return true;
                }
            }
            {
                int64_t v_x = vertexes[l].GetX();
                int64_t v_y = vertexes[l].GetY();
                max_x = std::max(v_x, max_x);
                if (unlikely(v_x == p_x && v_y == p_y)) {
                    return true;
                }
            }
            {
                int64_t v_x = vertexes[l + 1].GetX();
                int64_t v_y = vertexes[l + 1].GetY();
                max_x = std::max(v_x, max_x);
                if (unlikely(v_x == p_x && v_y == p_y)) {
                    return true;
                }
            }

            if (p_x > max_x) {
                return false;
            }

            Segment points_long_segment(point, Point(max_x + 1, p_y + 1));
            size_t count = 0;
            Segment edge(vertexes[0], vertexes[l]);
            if (unlikely(edge.ContainsPoint(point))) {
                return true;
            }
            count += points_long_segment.CrossesSegment(edge);
            edge = Segment(vertexes[l], vertexes[l + 1]);
            if (unlikely(edge.ContainsPoint(point))) {
                return true;
            }
            count += points_long_segment.CrossesSegment(edge);
            edge = Segment(vertexes[l + 1], vertexes[0]);
            if (unlikely(edge.ContainsPoint(point))) {
                return true;
            }
            count += points_long_segment.CrossesSegment(edge);
            return count % 2 != 0;
        }
    }

    size_t m = (l + r + 1) / 2;
    Vector vec_zrm = vertexes[m] - vertexes[0];
    Vector vec_zrp = point - vertexes[0];
    if (VectorMult(vec_zrp, vec_zrm) >= 0) {
        return SimpleConvexPolygonContainsPointRecursive(vertexes, point, l, m);
    } else {
        return SimpleConvexPolygonContainsPointRecursive(vertexes, point, m, r);
    }
}

bool Polygon::SimpleConvexPolygonContainsPoint(const Point& point) const noexcept {
    if (unlikely(vertexes_.empty())) {
        return false;
    }

    return SimpleConvexPolygonContainsPointRecursive(vertexes_, point, 0, vertexes_.size() - 1);
}

bool Polygon::CrossesSegment(const Segment& segment) const noexcept {
    const size_t n = vertexes_.size();
    for (size_t i = 1; i < n; ++i) {
        if (Segment(vertexes_[i - 1], vertexes_[i]).CrossesSegment(segment)) {
            return true;
        }
    }

    return false;
}

Polygon* Polygon::Clone() const {
    return new Polygon(vertexes_);
}

}  // namespace geometry
