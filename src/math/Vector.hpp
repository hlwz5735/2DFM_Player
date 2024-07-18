//
// Created by 厉猛 on 2024-07-17.
//

#ifndef INC_2DFM_PLAYER_VECTOR_HPP
#define INC_2DFM_PLAYER_VECTOR_HPP

class Vector2 {
public:
    explicit Vector2(float x = 0, float y = 0) noexcept;

    float length() const;
    float lengthSqrt() const;
    void normalize();
    /// 点乘
    float dot(const Vector2 &o);
    /// 计算夹角
    float angle(const Vector2 &o);
    bool isZero() const;
public:
    float x;
    float y;

    static Vector2 ZERO;
};

Vector2 operator-(const Vector2 &a);
Vector2 operator+(const Vector2 &a, const Vector2 &b);
Vector2 operator-(const Vector2 &a, const Vector2 &b);
Vector2 operator*(const Vector2 &a, float n);
Vector2 operator/(const Vector2 &a, float n);
bool operator==(const Vector2 &a, const Vector2 &b);

//class Vector3 {
//public:
//    float x;
//    float y;
//    float z;
//};

#endif //INC_2DFM_PLAYER_VECTOR_HPP
