#include "segment.h"

#include <cassert>
#include <utility>

#include "line.h"

namespace geometry {

double Segment::Distance(const Point& point) const noexcept {
    Vector se_vect = end_ - start_;
    Vector ps_vect = start_ - point;
    int64_t vector_mult_length = VectorMult(ps_vect, se_vect);
    return static_cast<double>(vector_mult_length) / Length(se_vect);
}

double Segment::NonProjectingDistance(const Point& point) const noexcept {
    Vector ap_vec = point - start_;
    Vector ab_vec = end_ - start_;
    Vector bp_vec = point - end_;
    Vector ba_vec = start_ - end_;
    int64_t ap_ab_scalar_mul = ScalarMult(ap_vec, ab_vec);
    int64_t bp_ba_scalar_mul = ScalarMult(bp_vec, ba_vec);
    if (ap_ab_scalar_mul >= 0 && bp_ba_scalar_mul >= 0) {
        return Distance(point);
    }

    if (ap_ab_scalar_mul < 0) {
        return Length(ba_vec);
    }

    return Length(bp_vec);
}

double Segment::Distance(const Segment& other) const noexcept {
    if (CrossesSegment(other)) {
        return 0;
    }

    double d1 = other.Distance(start_);
    double d2 = other.Distance(end_);
    double d3 = Distance(other.start_);
    double d4 = Distance(other.end_);
    return std::min(std::min(std::min(d1, d2), d3), d4);
}

bool Segment::IsDegenerate() const noexcept {
    return start_.GetX() == end_.GetX() && start_.GetY() == end_.GetY();
}

Segment& Segment::Move(const Vector& vector) noexcept {
    start_.Move(vector);
    end_.Move(vector);
    return *this;
}

bool Segment::ContainsPoint(const Point& point) const noexcept {
    int64_t start_x = start_.GetX();
    int64_t end_x = end_.GetX();
    if (end_x < start_x) {
        std::swap(start_x, end_x);
    }

    int64_t start_y = start_.GetY();
    int64_t end_y = end_.GetY();
    if (end_y < start_y) {
        std::swap(start_y, end_y);
    }

    int64_t point_x = point.GetX();
    int64_t point_y = point.GetY();

    if (!(start_x <= point_x && point_x <= end_x && start_y <= point_y && point_y <= end_y)) {
        return false;
    }

    Line l(start_, end_);
    return l.GetA() * point_x + l.GetB() * point_y + l.GetC() == 0;
}

bool Segment::CrossesSegment(const Segment& segment) const noexcept {
    if (IsDegenerate()) {
        return segment.IsDegenerate() || segment.ContainsPoint(start_);
    }

    if (segment.IsDegenerate()) {
        return ContainsPoint(segment.start_);
    }

    Line line1(start_, end_);
    Line line2(segment.start_, segment.end_);

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

        int64_t start1_x = start_.GetX();
        int64_t end1_x = end_.GetX();
        int64_t start1_y = start_.GetY();
        int64_t end1_y = end_.GetY();
        int64_t start2_x = segment.start_.GetX();
        int64_t end2_x = segment.end_.GetX();
        int64_t start2_y = segment.start_.GetY();
        int64_t end2_y = segment.end_.GetY();
        if (end1_y < start1_y) {
            std::swap(start1_y, end1_y);
        }

        if (end1_x < start1_x) {
            std::swap(start1_x, end1_x);
        }

        if (end2_x < start2_x) {
            std::swap(start2_x, end2_x);
        }

        if (end2_y < start2_y) {
            std::swap(start2_y, end2_y);
        }

        int64_t left_x_border = std::max(start1_x, start2_x);
        int64_t left_y_border = std::max(start1_y, start2_y);
        int64_t right_x_border = std::min(end1_x, end2_x);
        int64_t right_y_border = std::min(end1_y, end2_y);

        return left_x_border * det <= det_x && det_x <= right_x_border * det &&
               left_y_border * det <= det_y && det_y <= right_y_border * det;
    }

    return segment.ContainsPoint(start_) || segment.ContainsPoint(end_) ||
           ContainsPoint(segment.start_) || ContainsPoint(segment.end_);
}

Segment* Segment::Clone() const {
    return new Segment(start_, end_);
}

}  // namespace geometry
