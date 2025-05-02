#ifndef GEOMETRYCLASSES__POINT_H_
#define GEOMETRYCLASSES__POINT_H_ 1

#include <cstdint>
#include <ostream>

#include "i_shape.h"
#include "vector.h"

namespace geometry {

class Point : public IShape {
private:
    int64_t x_coord_ = 0;
    int64_t y_coord_ = 0;

public:
    constexpr Point() noexcept = default;

    constexpr Point(int64_t x, int64_t y) noexcept : x_coord_{x}, y_coord_{y} {}

    constexpr int64_t GetX() const noexcept {
        return x_coord_;
    }

    constexpr int64_t GetY() const noexcept {
        return y_coord_;
    }

    constexpr void SetX(int64_t value) noexcept {
        x_coord_ = value;
    }

    constexpr void SetY(int64_t value) noexcept {
        y_coord_ = value;
    }

    Point& Move(const Vector&) noexcept override;

    bool ContainsPoint(const Point&) const noexcept override;

    bool CrossesSegment(const Segment&) const noexcept override;

    Point* Clone() const override;

    inline Vector operator-(const Point& other) const noexcept {
        return Vector(x_coord_ - other.x_coord_, y_coord_ - other.y_coord_);
    }

    inline friend std::ostream& operator<<(std::ostream& out, const Point& point) {
        out << '(' << point.x_coord_ << ';' << point.y_coord_ << ')';
        return out;
    }
};
}  // namespace geometry

#endif  // GEOMETRYCLASSES__POINT_H_
