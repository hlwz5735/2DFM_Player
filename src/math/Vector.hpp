//
// Created by 厉猛 on 2024-07-17.
//

#ifndef INC_2DFM_PLAYER_VECTOR_HPP
#define INC_2DFM_PLAYER_VECTOR_HPP

class Vector2 {
public:
    explicit Vector2(float x = 0, float y = 0) noexcept;

    float length() const;
    float lengthSq() const;
    void normalize();
    static Vector2 normalize(const Vector2 &vec);
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

class Vector3 {
public:
    explicit Vector3(float x = 0, float y = 0, float z = 0) noexcept;
    Vector3(const Vector2 &vec2): x(vec2.x), y(vec2.y), z(0) {}

    bool isZero() const;

    float length() const;
    float lengthSq() const;

    void normalize();
    static Vector3 normalize(const Vector3 &vec);
    Vector3 cross(const Vector3 &b) const;
    static Vector3 cross(const Vector3 &a, const Vector3 &b);
    float dot(const Vector3 &b) const;
    static float dot(const Vector3 &a, const Vector3 &b);

    static Vector3 transform(const Vector3& vec, const class Matrix4& mat, float w = 1.0f);
    // This will transform the vector and renormalize the w component
    static Vector3 transformWithPerspDiv(const Vector3& vec, const class Matrix4& mat, float w = 1.0f);

    Vector3 &operator *=(float n);
public:
    float x;
    float y;
    float z;
};

Vector3 operator-(const Vector3 &a);
Vector3 operator+(const Vector3 &a, const Vector3 &b);
Vector3 operator-(const Vector3 &a, const Vector3 &b);
Vector3 operator*(const Vector3 &a, float n);
Vector3 operator/(const Vector3 &a, float n);
bool operator==(const Vector3 &a, const Vector3 &b);

#endif //INC_2DFM_PLAYER_VECTOR_HPP
