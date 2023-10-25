#ifndef GEOMETRYCLASSES__SEGMENT_H_
#define GEOMETRYCLASSES__SEGMENT_H_

#include "i_shape.h"
#include "point.h"
#include "vector.h"

namespace geometry {
class Segment : public IShape {
protected:
    Point start_;
    Point end_;

public:
    Segment() noexcept : start_{0, 0}, end_{0, 0} {
    }

    Segment(const Point& start, const Point& end) noexcept : start_{start}, end_{end} {
    }

    const Point& GetStart() const noexcept {
        return start_;
    }

    const Point& GetEnd() const noexcept {
        return end_;
    }

    void SetStart(const Point& start) noexcept {
        start_ = start;
    }

    void SetEnd(const Point& end) noexcept {
        end_ = end;
    }

    /// @brief Distance (projection length) from point to this segment
    /// @param point point
    /// @return length of the projection from point to this segment
    double Distance(const Point& point) const noexcept;

    double Distance(const Segment& point) const noexcept;

    /// @brief Distance (not always projection length) from point to this segment
    /// @param point point
    /// @return length of the shortest segment starting in point and ending in the point of this segment
    double NonProjectingDistance(const Point& point) const noexcept;

    bool IsDegenerate() const noexcept;

    Segment& Move(const Vector& vector) noexcept override;

    bool ContainsPoint(const Point& point) const noexcept override;

    bool CrossesSegment(const Segment& segment) const noexcept override;

    Segment* Clone() const override;
};
}  // namespace geometry

#endif  // GEOMETRYCLASSES__SEGMENT_H_
