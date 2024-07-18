//
// Created by 厉猛 on 2024-07-17.
//
#include <cmath>
#include "Vector.hpp"

Vector2 Vector2::ZERO(0, 0);

Vector2::Vector2(float x, float y) noexcept : x(x), y(y) {}

float Vector2::length() const {
    return sqrt(lengthSqrt());
}

float Vector2::lengthSqrt() const {
    return x * x + y * y;
}

void Vector2::normalize() {
    if (isZero()) {
        return;
    }
    auto l = length();
    x /= l;
    y /= l;
}

float Vector2::dot(const Vector2 &o) {
    return x * o.x + y * o.y;
}

float Vector2::angle(const Vector2 &o) {
    if (isZero() || o.isZero()) {
        return -1;
    }
    auto cosT = dot(o) / length() * o.length();
    return acos(cosT);
}

bool Vector2::isZero() const {
    return x == 0 && y == 0;
}

Vector2 operator-(const Vector2 &a) {
    return Vector2(-a.x, -a.y);
}

Vector2 operator+(const Vector2 &a, const Vector2 &b) {
    return Vector2(a.x + b.x, a.y + b.y);
}

Vector2 operator-(const Vector2 &a, const Vector2 &b) {
    return Vector2(a.x - b.x, a.y - b.y);
}

Vector2 operator*(const Vector2 &a, float n) {
    return Vector2(a.x * n, a.y * n);
}

Vector2 operator/(const Vector2 &a, float n) {
    auto factor = 1 / n;
    return a * factor;
}

bool operator==(const Vector2 &a, const Vector2 &b) {
    return a.x == b.x && a.y == b.y;
}
