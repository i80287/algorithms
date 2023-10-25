#ifndef GEOMETRYCLASSES__CIRCLE_H_
#define GEOMETRYCLASSES__CIRCLE_H_

#include "i_shape.h"
#include "point.h"

namespace geometry {
class Circle : public IShape {
private:
    Point centre_;
    uint64_t radius_;

public:
    Circle() noexcept;

    Circle(Point, uint64_t) noexcept;

    Point GetCenter() const noexcept;

    uint64_t GetRadius() const noexcept;

    Circle& Move(const Vector& vector) noexcept override;

    bool ContainsPoint(const Point& point) const noexcept override;

    bool CrossesSegment(const Segment& segment) const noexcept override;

    Circle* Clone() const override;
};
}  // namespace geometry

#endif  // GEOMETRYCLASSES__CIRCLE_H_
