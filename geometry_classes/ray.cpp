#include "ray.h"

#include "line.h"
#include "segment.h"

namespace geometry {

Ray& Ray::Move(const Vector& vector) noexcept {
    start_.Move(vector);
    end_.Move(vector);
    return *this;
}

bool Ray::ContainsPoint(const Point& point) const noexcept {
    int64_t start_x = start_.GetX();
    int64_t end_x = end_.GetX();
    int64_t point_x = point.GetX();
    if (start_x < end_x) {
        if (!(start_x <= point_x)) {
            return false;
        }
    } else {
        if (!(start_x >= point_x)) {
            return false;
        }
    }

    int64_t start_y = start_.GetY();
    int64_t end_y = end_.GetY();
    int64_t point_y = point.GetY();
    if (start_y < end_y) {
        if (!(start_y <= point_y)) {
            return false;
        }
    } else {
        if (!(start_y >= point_y)) {
            return false;
        }
    }

    Line l(start_, end_);
    return l.GetA() * point_x + l.GetB() * point_y + l.GetC() == 0;
}

bool Ray::CrossesSegment(const Segment& segment) const noexcept {
    Line line1(start_, end_);
    Point segment_start = segment.GetStart();
    Point segment_end = segment.GetEnd();
    Line line2(segment_start, segment_end);

    int64_t a1 = line1.GetA();
    int64_t a2 = line2.GetA();
    int64_t b1 = line1.GetB();
    int64_t b2 = line2.GetB();
    int64_t c1 = line1.GetC();
    int64_t c2 = line2.GetC();
    int64_t det = a1 * b2 - b1 * a2;

    if (det != 0) {
        int64_t det_x = c2 * b1 - c1 * b2;
        int64_t det_y = c1 * a2 - c2 * a1;
        if (det < 0) {
            det = -det;
            det_x = -det_x;
            det_y = -det_y;
        }

        {
            int64_t start1_x = start_.GetX();
            int64_t end1_x = end_.GetX();
            if (start1_x < end1_x) {
                if (!(start1_x * det <= det_x)) {
                    return false;
                }
            } else {
                if (!(start1_x * det >= det_x)) {
                    return false;
                }
            }
        }

        {
            int64_t start1_y = start_.GetY();
            int64_t end1_y = end_.GetY();
            if (start1_y < end1_y) {
                if (!(start1_y * det <= det_y)) {
                    return false;
                }
            } else {
                if (!(start1_y * det >= det_y)) {
                    return false;
                }
            }
        }

        int64_t start2_x = segment_start.GetX();
        int64_t end2_x = segment_end.GetX();
        int64_t start2_y = segment_start.GetY();
        int64_t end2_y = segment_end.GetY();

        if (end2_x < start2_x) {
            std::swap(start2_x, end2_x);
        }

        if (end2_y < start2_y) {
            std::swap(start2_y, end2_y);
        }

        return start2_x * det <= det_x && det_x <= end2_x * det && start2_y * det <= det_y && det_y <= end2_y * det;
    }

    return segment.ContainsPoint(start_) || segment.ContainsPoint(end_) || ContainsPoint(segment.GetStart()) ||
           ContainsPoint(segment.GetEnd());
}

Ray* Ray::Clone() const {
    return new Ray(start_, end_);
}

}  // namespace geometry
