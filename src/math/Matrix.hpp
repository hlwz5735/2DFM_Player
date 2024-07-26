//
// Created by 厉猛 on 2024-07-26.
//

#ifndef INC_2DFM_PLAYER_MATRIX4_HPP
#define INC_2DFM_PLAYER_MATRIX4_HPP

#include "Vector.hpp"
#include <cstring>
#include <cmath>

// 4x4 Matrix
class Matrix4 {
public:
    float mat[4][4];

    Matrix4() {
        *this = Matrix4::Identity;
    }

    explicit Matrix4(float inMat[4][4]) {
        memcpy(mat, inMat, 16 * sizeof(float));
    }

    // Cast to a const float pointer
    const float *getAsFloatPtr() const {
        return reinterpret_cast<const float *>(&mat[0][0]);
    }

    // Matrix multiplication (a * b)
    friend Matrix4 operator*(const Matrix4 &a, const Matrix4 &b) {
        Matrix4 retVal;
        // row 0
        retVal.mat[0][0] =
                a.mat[0][0] * b.mat[0][0] +
                a.mat[0][1] * b.mat[1][0] +
                a.mat[0][2] * b.mat[2][0] +
                a.mat[0][3] * b.mat[3][0];

        retVal.mat[0][1] =
                a.mat[0][0] * b.mat[0][1] +
                a.mat[0][1] * b.mat[1][1] +
                a.mat[0][2] * b.mat[2][1] +
                a.mat[0][3] * b.mat[3][1];

        retVal.mat[0][2] =
                a.mat[0][0] * b.mat[0][2] +
                a.mat[0][1] * b.mat[1][2] +
                a.mat[0][2] * b.mat[2][2] +
                a.mat[0][3] * b.mat[3][2];

        retVal.mat[0][3] =
                a.mat[0][0] * b.mat[0][3] +
                a.mat[0][1] * b.mat[1][3] +
                a.mat[0][2] * b.mat[2][3] +
                a.mat[0][3] * b.mat[3][3];

        // row 1
        retVal.mat[1][0] =
                a.mat[1][0] * b.mat[0][0] +
                a.mat[1][1] * b.mat[1][0] +
                a.mat[1][2] * b.mat[2][0] +
                a.mat[1][3] * b.mat[3][0];

        retVal.mat[1][1] =
                a.mat[1][0] * b.mat[0][1] +
                a.mat[1][1] * b.mat[1][1] +
                a.mat[1][2] * b.mat[2][1] +
                a.mat[1][3] * b.mat[3][1];

        retVal.mat[1][2] =
                a.mat[1][0] * b.mat[0][2] +
                a.mat[1][1] * b.mat[1][2] +
                a.mat[1][2] * b.mat[2][2] +
                a.mat[1][3] * b.mat[3][2];

        retVal.mat[1][3] =
                a.mat[1][0] * b.mat[0][3] +
                a.mat[1][1] * b.mat[1][3] +
                a.mat[1][2] * b.mat[2][3] +
                a.mat[1][3] * b.mat[3][3];

        // row 2
        retVal.mat[2][0] =
                a.mat[2][0] * b.mat[0][0] +
                a.mat[2][1] * b.mat[1][0] +
                a.mat[2][2] * b.mat[2][0] +
                a.mat[2][3] * b.mat[3][0];

        retVal.mat[2][1] =
                a.mat[2][0] * b.mat[0][1] +
                a.mat[2][1] * b.mat[1][1] +
                a.mat[2][2] * b.mat[2][1] +
                a.mat[2][3] * b.mat[3][1];

        retVal.mat[2][2] =
                a.mat[2][0] * b.mat[0][2] +
                a.mat[2][1] * b.mat[1][2] +
                a.mat[2][2] * b.mat[2][2] +
                a.mat[2][3] * b.mat[3][2];

        retVal.mat[2][3] =
                a.mat[2][0] * b.mat[0][3] +
                a.mat[2][1] * b.mat[1][3] +
                a.mat[2][2] * b.mat[2][3] +
                a.mat[2][3] * b.mat[3][3];

        // row 3
        retVal.mat[3][0] =
                a.mat[3][0] * b.mat[0][0] +
                a.mat[3][1] * b.mat[1][0] +
                a.mat[3][2] * b.mat[2][0] +
                a.mat[3][3] * b.mat[3][0];

        retVal.mat[3][1] =
                a.mat[3][0] * b.mat[0][1] +
                a.mat[3][1] * b.mat[1][1] +
                a.mat[3][2] * b.mat[2][1] +
                a.mat[3][3] * b.mat[3][1];

        retVal.mat[3][2] =
                a.mat[3][0] * b.mat[0][2] +
                a.mat[3][1] * b.mat[1][2] +
                a.mat[3][2] * b.mat[2][2] +
                a.mat[3][3] * b.mat[3][2];

        retVal.mat[3][3] =
                a.mat[3][0] * b.mat[0][3] +
                a.mat[3][1] * b.mat[1][3] +
                a.mat[3][2] * b.mat[2][3] +
                a.mat[3][3] * b.mat[3][3];

        return retVal;
    }

    Matrix4 &operator*=(const Matrix4 &right) {
        *this = *this * right;
        return *this;
    }

    // Invert the matrix - super slow
    void invert();

    // Get the translation component of the matrix
    Vector3 getTranslation() const {
        return Vector3(mat[3][0], mat[3][1], mat[3][2]);
    }

    // Get the X axis of the matrix (forward)
    Vector3 getXAxis() const {
        return Vector3::normalize(Vector3(mat[0][0], mat[0][1], mat[0][2]));
    }

    // Get the Y axis of the matrix (left)
    Vector3 getYAxis() const {
        return Vector3::normalize(Vector3(mat[1][0], mat[1][1], mat[1][2]));
    }

    // Get the Z axis of the matrix (up)
    Vector3 getZAxis() const {
        return Vector3::normalize(Vector3(mat[2][0], mat[2][1], mat[2][2]));
    }

    // Extract the scale component from the matrix
    Vector3 getScale() const {
        Vector3 retVal;
        retVal.x = Vector3(mat[0][0], mat[0][1], mat[0][2]).length();
        retVal.y = Vector3(mat[1][0], mat[1][1], mat[1][2]).length();
        retVal.z = Vector3(mat[2][0], mat[2][1], mat[2][2]).length();
        return retVal;
    }

    // Create a scale matrix with x, y, and z scales
    static Matrix4 createScale(float xScale, float yScale, float zScale) {
        float temp[4][4] =
                {
                        {xScale, 0.0f,   0.0f,   0.0f},
                        {0.0f,   yScale, 0.0f,   0.0f},
                        {0.0f,   0.0f,   zScale, 0.0f},
                        {0.0f,   0.0f,   0.0f,   1.0f}
                };
        return Matrix4(temp);
    }

    static Matrix4 createScale(const Vector3 &scaleVector) {
        return createScale(scaleVector.x, scaleVector.y, scaleVector.z);
    }

    // Create a scale matrix with a uniform factor
    static Matrix4 createScale(float scale) {
        return createScale(scale, scale, scale);
    }

    // Rotation about x-axis
    static Matrix4 createRotationX(float theta) {
        float temp[4][4] =
                {
                        {1.0f, 0.0f,        0.0f,       0.0f},
                        {0.0f, cos(theta),  sin(theta), 0.0f},
                        {0.0f, -sin(theta), cos(theta), 0.0f},
                        {0.0f, 0.0f,        0.0f,       1.0f},
                };
        return Matrix4(temp);
    }

    // Rotation about y-axis
    static Matrix4 createRotationY(float theta) {
        float temp[4][4] =
                {
                        {cos(theta), 0.0f, -sin(theta), 0.0f},
                        {0.0f,       1.0f, 0.0f,        0.0f},
                        {sin(theta), 0.0f, cos(theta),  0.0f},
                        {0.0f,       0.0f, 0.0f,        1.0f},
                };
        return Matrix4(temp);
    }

    // Rotation about z-axis
    static Matrix4 createRotationZ(float theta) {
        float temp[4][4] =
                {
                        {cos(theta),  sin(theta), 0.0f, 0.0f},
                        {-sin(theta), cos(theta), 0.0f, 0.0f},
                        {0.0f,        0.0f,       1.0f, 0.0f},
                        {0.0f,        0.0f,       0.0f, 1.0f},
                };
        return Matrix4(temp);
    }

    static Matrix4 createTranslation(const Vector3 &trans) {
        float temp[4][4] =
                {
                        {1.0f,    0.0f,    0.0f,    0.0f},
                        {0.0f,    1.0f,    0.0f,    0.0f},
                        {0.0f,    0.0f,    1.0f,    0.0f},
                        {trans.x, trans.y, trans.z, 1.0f}
                };
        return Matrix4(temp);
    }

    static Matrix4 createLookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up) {
        Vector3 zaxis = Vector3::normalize(target - eye);
        Vector3 xaxis = Vector3::normalize(Vector3::cross(up, zaxis));
        Vector3 yaxis = Vector3::normalize(Vector3::cross(zaxis, xaxis));
        Vector3 trans;
        trans.x = -Vector3::dot(xaxis, eye);
        trans.y = -Vector3::dot(yaxis, eye);
        trans.z = -Vector3::dot(zaxis, eye);

        float temp[4][4] =
                {
                        {xaxis.x, yaxis.x, zaxis.x, 0.0f},
                        {xaxis.y, yaxis.y, zaxis.y, 0.0f},
                        {xaxis.z, yaxis.z, zaxis.z, 0.0f},
                        {trans.x, trans.y, trans.z, 1.0f}
                };
        return Matrix4(temp);
    }

    static Matrix4 createOrtho(float width, float height, float near, float far) {
        float temp[4][4] =
                {
                        {2.0f / width, 0.0f,          0.0f,                0.0f},
                        {0.0f,         2.0f / height, 0.0f,                0.0f},
                        {0.0f,         0.0f,          1.0f / (far - near), 0.0f},
                        {0.0f,         0.0f,          near / (near - far), 1.0f}
                };
        return Matrix4(temp);
    }

    // Create "Simple" View-Projection Matrix from Chapter 6
    static Matrix4 createSimpleViewProj(float width, float height) {
        float temp[4][4] =
                {
                        {2.0f / width, 0.0f,          0.0f, 0.0f},
                        {0.0f,         2.0f / height, 0.0f, 0.0f},
                        {0.0f,         0.0f,          1.0f, 0.0f},
                        {0.0f,         0.0f,          1.0f, 1.0f}
                };
        return Matrix4(temp);
    }

    static const Matrix4 Identity;
};

#endif //INC_2DFM_PLAYER_MATRIX4_HPP
