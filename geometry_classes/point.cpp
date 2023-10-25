#include "point.h"

#include "segment.h"

namespace geometry {

Point& Point::Move(const Vector& destination_vec) noexcept {
    x_coord_ += destination_vec.GetX();
    y_coord_ += destination_vec.GetY();
    return *this;
}

bool Point::ContainsPoint(const Point& other) const noexcept {
    return x_coord_ == other.x_coord_ && y_coord_ == other.y_coord_;
}

bool Point::CrossesSegment(const Segment& segment) const noexcept {
    auto seg_start = segment.GetStart();
    auto seg_end = segment.GetEnd();
    int64_t x0 = std::min(seg_start.x_coord_, seg_end.x_coord_);
    int64_t x2 = seg_start.x_coord_ + seg_end.x_coord_ - x0;
    int64_t y0 = std::min(seg_start.y_coord_, seg_end.y_coord_);
    int64_t y2 = seg_start.y_coord_ + seg_end.y_coord_ - y0;
    return x0 <= x_coord_ && x_coord_ <= x2 && y0 <= y_coord_ && y_coord_ <= y2 &&
           static_cast<uint64_t>(x2 - x_coord_) * static_cast<uint64_t>(y_coord_ - y0) ==
               static_cast<uint64_t>(y2 - y_coord_) * static_cast<uint64_t>(x_coord_ - x0);
}

Point* Point::Clone() const {
    return new Point(x_coord_, y_coord_);
}

}  // namespace geometry
