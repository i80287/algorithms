#include "vector.h"

#include <cstdint>

namespace geometry {

Vector Vector::operator+(const Vector& other) const noexcept {
    return Vector(x_coord_ + other.x_coord_, y_coord_ + other.y_coord_);
}

Vector Vector::operator-(const Vector& other) const noexcept {
    return Vector(x_coord_ - other.x_coord_, y_coord_ - other.y_coord_);
}

Vector Vector::operator+() const noexcept {
    return Vector(x_coord_, y_coord_);
}

Vector Vector::operator-() const noexcept {
    return Vector(-x_coord_, -y_coord_);
}

Vector Vector::operator*(int64_t number) const noexcept {
    return Vector(x_coord_ * number, y_coord_ * number);
}

Vector Vector::operator/(int64_t number) const noexcept {
    return number != 0 ? Vector(x_coord_ / number, y_coord_ / number) : Vector();
}

Vector& Vector::operator+=(const Vector& other) noexcept {
    x_coord_ += other.x_coord_;
    y_coord_ += other.y_coord_;
    return *this;
}

Vector& Vector::operator-=(const Vector& other) noexcept {
    x_coord_ -= other.x_coord_;
    y_coord_ -= other.y_coord_;
    return *this;
}

Vector& Vector::operator*=(int64_t number) noexcept {
    x_coord_ *= number;
    y_coord_ *= number;
    return *this;
}

Vector& Vector::operator/=(int64_t number) noexcept {
    if (number != 0) {
        x_coord_ /= number;
        y_coord_ /= number;
    }

    return *this;
}

bool Vector::operator==(const Vector& other) const noexcept {
    return x_coord_ == other.x_coord_ && y_coord_ == other.y_coord_;
}

int64_t ScalarMult(const Vector& first, const Vector& second) noexcept {
    return first.x_coord_ * second.x_coord_ + first.y_coord_ * second.y_coord_;
}

int64_t VectorMult(const Vector& first, const Vector& second) noexcept {
    return first.x_coord_ * second.y_coord_ - first.y_coord_ * second.x_coord_;
}

double Length(const Vector& vector) noexcept {
    double x = static_cast<double>(vector.GetX());
    double y = static_cast<double>(vector.GetY());
    return std::sqrt(x * x + y * y);
}

}  // namespace geometry
