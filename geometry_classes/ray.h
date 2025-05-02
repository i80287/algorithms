#pragma once

#include "i_shape.h"
#include "point.h"

namespace geometry {
class Ray : public IShape {
private:
    Point start_;
    Point end_;

public:
    Ray() : start_{0, 0}, end_{0, 0} {}

    Ray(const Point& first, const Point& second) : start_{first}, end_{second} {}

    Ray& Move(const Vector& vector) noexcept override;

    bool ContainsPoint(const Point& point) const noexcept override;

    bool CrossesSegment(const Segment& segment) const noexcept override;

    Ray* Clone() const override;

    Point GetStart() const noexcept {
        return start_;
    }

    Point GetEnd() const noexcept {
        return end_;
    }
};
}  // namespace geometry
