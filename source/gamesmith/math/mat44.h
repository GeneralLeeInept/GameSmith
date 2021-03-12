#pragma once

#include <gamesmith/math/vec4.h>

namespace gs
{

// Column-major 4x4 matrix
struct alignas(16) mat44
{
    Vec4 X;
    Vec4 Y;
    Vec4 Z;
    Vec4 P;

    mat44();
    mat44(const Vec4&, const Vec4&, const Vec4&, const Vec4&);
    mat44(float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float);
    mat44(mat44&) = default;
    mat44(mat44&&) = default;

    mat44& operator=(const mat44&) = default;
    mat44& operator=(mat44&&) = default;

    Vec4& operator[](int);
    Vec4 operator[](int) const;

    mat44& operator*=(const mat44&);
    mat44& operator*=(float);
    mat44& operator/=(float);

    float Determinant() const;
};

mat44 operator*(const mat44& a, const mat44& b);
Vec4 operator*(const mat44& m, const Vec4& v);
mat44 operator*(const mat44& m, float s);
mat44 operator*(float s, const mat44& m);
mat44 operator/(const mat44& m, float s);

mat44 transpose(const mat44& m);
mat44 inverse(const mat44& m);
mat44 invertOrthogonal(const mat44& m);

mat44 rotateX(float r);
mat44 rotateY(float r);
mat44 rotateZ(float r);
mat44 rotate(float pitch, float yaw, float roll);
mat44 translate(const Vec4& p);
mat44 scale(float s);

mat44 perspective(float fovy, float aspect, float znear, float zfar);
mat44 orthographic(float left, float top, float right, float bottom, float znear, float zfar);
mat44 lookAt(const vec3& from, const vec3& at, const vec3& up);

} // namespace gs
