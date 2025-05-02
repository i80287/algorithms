#pragma once

#include <cmath>
#include <cstdint>

namespace geometry {

class Vector {
private:
    int64_t x_coord_;
    int64_t y_coord_;

public:
    Vector() noexcept : x_coord_{0}, y_coord_{0} {}

    Vector(int64_t x, int64_t y) noexcept : x_coord_{x}, y_coord_{y} {}

    int64_t GetX() const noexcept {
        return x_coord_;
    }

    int64_t GetY() const noexcept {
        return y_coord_;
    }

    Vector operator+(const Vector& other) const noexcept;

    Vector operator-(const Vector& other) const noexcept;

    Vector operator+() const noexcept;

    Vector operator-() const noexcept;

    Vector operator*(int64_t number) const noexcept;

    Vector operator/(int64_t number) const noexcept;

    Vector& operator+=(const Vector& other) noexcept;

    Vector& operator-=(const Vector& other) noexcept;

    Vector& operator*=(int64_t number) noexcept;

    Vector& operator/=(int64_t number) noexcept;

    bool operator==(const Vector& other) const noexcept;

    friend int64_t ScalarMult(const Vector& first, const Vector& second) noexcept;

    friend int64_t VectorMult(const Vector& first, const Vector& second) noexcept;

    friend double Length(const Vector& vector) noexcept;
};

int64_t ScalarMult(const Vector& first, const Vector& second) noexcept;

int64_t VectorMult(const Vector& first, const Vector& second) noexcept;

double Length(const Vector& vector) noexcept;

}  // namespace geometry
