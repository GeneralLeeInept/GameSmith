#pragma once

#include <gamesmith/math/vec4.h>

namespace gs
{

// Column-major 4x4 matrix
struct alignas(16) Mat44
{
    Vec4 X;
    Vec4 Y;
    Vec4 Z;
    Vec4 P;

    Mat44();
    Mat44(const Vec4&, const Vec4&, const Vec4&, const Vec4&);
    Mat44(float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float);
    Mat44(Mat44&) = default;
    Mat44(Mat44&&) = default;

    Mat44& operator=(const Mat44&) = default;
    Mat44& operator=(Mat44&&) = default;

    Vec4& operator[](int);
    Vec4 operator[](int) const;

    Mat44& operator*=(const Mat44&);
    Mat44& operator*=(float);
    Mat44& operator/=(float);

    float Determinant() const;
};

Mat44 operator*(const Mat44& a, const Mat44& b);
Vec4 operator*(const Mat44& m, const Vec4& v);
Mat44 operator*(const Mat44& m, float s);
Mat44 operator*(float s, const Mat44& m);
Mat44 operator/(const Mat44& m, float s);

Mat44 Transpose(const Mat44& m);
Mat44 Inverse(const Mat44& m);
Mat44 InverseFast(const Mat44& m);

Mat44 RotateX(float r);
Mat44 RotateY(float r);
Mat44 RotateZ(float r);
Mat44 Rotate(float pitch, float yaw, float roll);
Mat44 Translate(const Vec4& p);
Mat44 Scale(float s);

Mat44 Perspective(float fovy, float aspect, float znear, float zfar);
Mat44 Orthographic(float left, float top, float right, float bottom, float znear, float zfar);
Mat44 LookAt(const Vec3& from, const Vec3& at, const Vec3& up);

} // namespace gs
