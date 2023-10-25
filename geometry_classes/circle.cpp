#include "circle.h"
#include "line.h"
#include "segment.h"

namespace geometry {

constexpr uint64_t Uabs(int64_t val) noexcept {
    return static_cast<uint64_t>(val >= 0 ? val : -val);
}

Circle::Circle() noexcept : centre_{0, 0}, radius_{0} {
}

Circle::Circle(Point centre, uint64_t radius) noexcept : centre_{centre}, radius_{radius} {
}

Point Circle::GetCenter() const noexcept {
    return centre_;
}

uint64_t Circle::GetRadius() const noexcept {
    return radius_;
}

Circle& Circle::Move(const Vector& vector) noexcept {
    centre_.Move(vector);
    return *this;
}

bool Circle::ContainsPoint(const Point& point) const noexcept {
    uint64_t x = Uabs(point.GetX() - centre_.GetX());
    uint64_t y = Uabs(point.GetY() - centre_.GetY());
    return x * x + y * y <= radius_ * radius_;
}

bool Circle::CrossesSegment(const Segment& segment) const noexcept {
    int64_t cx = centre_.GetX();
    int64_t cy = centre_.GetY();

    Point start = segment.GetStart();
    uint64_t x = Uabs(start.GetX() - cx);
    uint64_t y = Uabs(start.GetY() - cy);
    uint64_t length_start_square = x * x + y * y;

    Point end = segment.GetEnd();
    x = Uabs(end.GetX() - cx);
    y = Uabs(end.GetY() - cy);
    uint64_t length_end_square = x * x + y * y;

    uint64_t radius_square = radius_ * radius_;

    if ((length_start_square <= radius_square && length_end_square >= radius_square) ||
        (length_start_square >= radius_square && length_end_square <= radius_square)) {
        return true;
    }

    if (length_start_square < radius_square && length_end_square < radius_square) {
        return false;
    }

    Line l(start, end);
    uint64_t a = Uabs(l.GetA());
    uint64_t b = Uabs(l.GetB());
    uint64_t c = Uabs(l.GetC());

    return (a * a + b * b) * radius_square >= c * c;
}

Circle* Circle::Clone() const {
    return new Circle(centre_, radius_);
}

}  // namespace geometry
