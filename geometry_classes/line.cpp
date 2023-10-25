#include "line.h"

#include <cassert>
#include <cstdint>

#include "segment.h"

namespace geometry {

double Line::Distance(const Point& point) const noexcept {
    auto se_vect = end_ - start_;
    auto ps_vect = start_ - point;
    int64_t vector_mult_length = VectorMult(ps_vect, se_vect);
    assert(!(end_.GetX() == start_.GetX() && end_.GetY() == start_.GetY()));
    return static_cast<double>(vector_mult_length) / Length(se_vect);
}

Line& Line::Move(const Vector& vector) noexcept {
    start_.Move(vector);
    end_.Move(vector);
    return *this;
}

bool Line::ContainsPoint(const Point& point) const noexcept {
    int64_t x0 = start_.GetX();
    int64_t x2 = end_.GetX();
    int64_t y0 = start_.GetY();
    int64_t y2 = end_.GetY();

    return (x2 - point.GetX()) * (point.GetY() - y0) == (y2 - point.GetY()) * (point.GetX() - x0);
}

bool Line::CrossesSegment(const Segment& segment) const noexcept {
    Vector l = end_ - start_;

    Point p1 = segment.GetStart();
    Point p2(p1);
    p2.Move(l);
    Line line1(p1, p2);

    Point p3 = segment.GetEnd();
    Point p4(p3);
    p4.Move(l);
    Line line2(p3, p4);

    int64_t c1 = line1.GetC();
    int64_t c2 = line2.GetC();
    if (c1 > c2) {
        c1 ^= c2;
        c2 ^= c1;
        c1 ^= c2;
    }

    int64_t c = GetC();
    return c1 <= c && c <= c2;
}

Line* Line::Clone() const {
    return new Line(start_, end_);
}

}  // namespace geometry
