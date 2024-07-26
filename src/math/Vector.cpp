//
// Created by 厉猛 on 2024-07-17.
//
#include <cmath>
#include "Vector.hpp"
#include "Matrix.hpp"
#include "base.hpp"

Vector2 Vector2::ZERO(0, 0);

Vector2::Vector2(float x, float y) noexcept : x(x), y(y) {}

float Vector2::length() const {
    return sqrt(lengthSq());
}

float Vector2::lengthSq() const {
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

Vector2 Vector2::normalize(const Vector2 &vec) {
    Vector2 t(vec);
    t.normalize();
    return t;
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

Vector2 & Vector2::operator+=(const Vector2 &o) {
    x += o.x;
    y += o.y;
    return *this;
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
    return a * (1 / n);
}

bool operator==(const Vector2 &a, const Vector2 &b) {
    return a.x == b.x && a.y == b.y;
}

Vector3::Vector3(float x, float y, float z) noexcept: x(x), y(y), z(z) {}

void Vector3::normalize() {
    if (isZero()) {
        return;
    }
    auto l = length();
    x /= l;
    y /= l;
    z /= l;
}

bool Vector3::isZero() const {
    return x == 0 && y == 0 && z == 0;
}

float Vector3::lengthSq() const {
    return x * x + y * y + z * z;
}

float Vector3::length() const {
    return sqrt(lengthSq());
}

Vector3 Vector3::normalize(const Vector3 &vec) {
    Vector3 t(vec);
    t.normalize();
    return t;
}

Vector3 Vector3::cross(const Vector3 &a, const Vector3 &b) {
    Vector3 temp;
    temp.x = a.y * b.z - a.z * b.y;
    temp.y = a.z * b.x - a.x * b.z;
    temp.z = a.x * b.y - a.y * b.x;
    return temp;
}

Vector3 Vector3::cross(const Vector3 &b) const {
    return Vector3::cross(*this, b);
}

float Vector3::dot(const Vector3 &a, const Vector3 &b) {
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

float Vector3::dot(const Vector3 &b) const {
    return Vector3::dot(*this, b);
}

Vector3 Vector3::transform(const Vector3 &vec, const struct Matrix4 &mat, float w) {
    Vector3 retVal;
    retVal.x = vec.x * mat.mat[0][0] + vec.y * mat.mat[1][0] +
               vec.z * mat.mat[2][0] + w * mat.mat[3][0];
    retVal.y = vec.x * mat.mat[0][1] + vec.y * mat.mat[1][1] +
               vec.z * mat.mat[2][1] + w * mat.mat[3][1];
    retVal.z = vec.x * mat.mat[0][2] + vec.y * mat.mat[1][2] +
               vec.z * mat.mat[2][2] + w * mat.mat[3][2];
    //ignore w since we aren't returning a new value for it...
    return retVal;
}

Vector3 Vector3::transformWithPerspDiv(const Vector3 &vec, const Matrix4 &mat, float w) {
    Vector3 retVal;
    retVal.x = vec.x * mat.mat[0][0] + vec.y * mat.mat[1][0] +
               vec.z * mat.mat[2][0] + w * mat.mat[3][0];
    retVal.y = vec.x * mat.mat[0][1] + vec.y * mat.mat[1][1] +
               vec.z * mat.mat[2][1] + w * mat.mat[3][1];
    retVal.z = vec.x * mat.mat[0][2] + vec.y * mat.mat[1][2] +
               vec.z * mat.mat[2][2] + w * mat.mat[3][2];
    float transformedW = vec.x * mat.mat[0][3] + vec.y * mat.mat[1][3] +
                         vec.z * mat.mat[2][3] + w * mat.mat[3][3];
    if (!nearZero(transformedW)) {
        transformedW = 1.0f / transformedW;
        retVal *= transformedW;
    }
    return retVal;
}

Vector3 &Vector3::operator*=(float n) {
    x *= n;
    y *= n;
    z *= n;
    return *this;
}

Vector3 operator-(const Vector3 &a) {
    return Vector3(-a.x, -a.y, -a.z);
}

Vector3 operator+(const Vector3 &a, const Vector3 &b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vector3 operator-(const Vector3 &a, const Vector3 &b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vector3 operator*(const Vector3 &a, float n) {
    return Vector3(a.x * n, a.y * n, a.z * n);
}

Vector3 operator/(const Vector3 &a, float n) {
    return a * (1 / n);
}

bool operator==(const Vector3 &a, const Vector3 &b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}
