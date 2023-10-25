#pragma once
#include <iostream>
#include <vector>

#include "point.h"
#include "vector.h"

namespace geometry {
class Polygon : public IShape {
protected:
    std::vector<Point> vertexes_;
public:
    Polygon() noexcept = default;

    explicit Polygon(const std::vector<Point>& vertexes) : vertexes_{vertexes} {
    }

    Polygon& Move(const Vector& vector) noexcept override;

    /// @brief In worst case works in O(n^2), where n is number of vertexes
    /// @return true if this polygon contains point and false otherwise
    bool ContainsPoint(const Point& point) const noexcept override;

    /// @brief Works if and only if this polygon is simple (e.g. without self intersections and holes inside)
    ///        This method works in O(n), where n is number of vertexes
    /// @return true if this polygon contains point and false otherwise
    bool SimplePolygonContainsPoint(const Point& point) const noexcept;

    /// @brief Works if and only if this polygon is simple and convex
    ///        This method works in O(log(n)), where n is number of vertexes
    /// @return true if this polygon contains point and false otherwise
    bool SimpleConvexPolygonContainsPoint(const Point& point) const noexcept;

    bool CrossesSegment(const Segment& segment) const noexcept override;

    Polygon* Clone() const override;

    const std::vector<Point>& GetVertexes() const noexcept {
        return vertexes_;
    }
};
}  // namespace geometry
