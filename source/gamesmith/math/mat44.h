#pragma once

#include <gamesmith/math/vec4.h>

namespace gs
{

// Column-major 4x4 matrix
struct Mat44
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

Mat44 operator*(const Mat44&, const Mat44&);
Vec4 operator*(const Mat44&, const Vec4&);
Mat44 operator*(const Mat44&, float);
Mat44 operator*(float, const Mat44&);
Mat44 operator/(const Mat44&, float);

Mat44 Transpose(const Mat44&);
Mat44 Inverse(const Mat44&);
Mat44 InverseFast(const Mat44&);
}
