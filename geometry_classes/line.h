#ifndef GEOMETRYCLASSES__LINE_H_
#define GEOMETRYCLASSES__LINE_H_

#include "i_shape.h"
#include "point.h"

namespace geometry {
class Line : public IShape {
private:
    Point start_;
    Point end_;

public:
    Line() noexcept : start_{0, 0}, end_{0, 0} {
    }

    Line(const Point& start, const Point& end) noexcept : start_{start}, end_{end} {
    }

    int64_t GetA() const noexcept {
        return end_.GetY() - start_.GetY();
    }

    int64_t GetB() const noexcept {
        return start_.GetX() - end_.GetX();
    }

    int64_t GetC() const noexcept {
        return -end_.GetY() * start_.GetX() + start_.GetY() * end_.GetX();
    }

    double Distance(const Point&) const noexcept;

    Line& Move(const Vector& vector) noexcept override;

    bool ContainsPoint(const Point& point) const noexcept override;

    bool CrossesSegment(const Segment& segment) const noexcept override;

    Line* Clone() const override;

    inline friend std::ostream& operator<<(std::ostream& out, const Line& line) {
        out << line.GetA() << " * x + " << line.GetB() << " * y + " << line.GetC() << " = 0";
        return out;
    }
};
}  // namespace geometry

#endif  // GEOMETRYCLASSES__LINE_H_
