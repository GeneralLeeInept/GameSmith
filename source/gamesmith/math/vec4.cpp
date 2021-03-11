#include "gspch.h"

#include "vec4.h"

#include <gamesmith/core/debug.h>
#include <gamesmith/math/math.h>
#include <gamesmith/math/vec3.h>

namespace gs
{

Vec4::Vec4(float f)
    : Vec4(f, f, f, f)
{
}

constexpr Vec4::Vec4(float x_, float y_, float z_, float w_)
    : x(x_)
    , y(y_)
    , z(z_)
    , w(w_)
{
}

Vec4::Vec4(const Vec3& u, float w_)
    : Vec4(u.x, u.y, u.z, w_)
{
}

float& Vec4::operator[](int ord)
{
    GS_ASSERT(ord >= 0 && ord < 4);
    if (ord == 0) return x;
    if (ord == 1) return y;
    if (ord == 2) return z;
    return w;
}

float Vec4::operator[](int ord) const
{
    GS_ASSERT(ord >= 0 && ord < 4);
    if (ord == 0) return x;
    if (ord == 1) return y;
    if (ord == 2) return z;
    return w;
}

Vec4& Vec4::operator*=(const Vec4& rhs)
{
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;
    w *= rhs.w;
    return *this;
}

Vec4& Vec4::operator/=(const Vec4& rhs)
{
    x /= rhs.x;
    y /= rhs.y;
    z /= rhs.z;
    w /= rhs.w;
    return *this;
}

Vec4& Vec4::operator+=(const Vec4& rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;
    return *this;
}

Vec4& Vec4::operator-=(const Vec4& rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    w -= rhs.w;
    return *this;
}

Vec4& Vec4::operator*=(float s)
{
    x *= s;
    y *= s;
    z *= s;
    w *= s;
    return *this;
}

Vec4& Vec4::operator/=(float s)
{
    float invS = 1.0f / s;
    x *= invS;
    y *= invS;
    z *= invS;
    w *= invS;
    return *this;
}

float Vec4::Length() const
{
    return std::sqrt(x * x + y * y + z * z + w * w);
}

float Vec4::LengthSq() const
{
    return x * x + y * y + z * z + w * w;
}

Vec4 operator-(const Vec4& u)
{
    return Vec4{ -u.x, -u.y, -u.z, -u.w };
}

bool operator==(const Vec4& lhs, const Vec4& rhs)
{
    return (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w);
}

Vec4 operator*(const Vec4& u, const Vec4& v)
{
    return Vec4{ u.x * v.x, u.y * v.y, u.z * v.z, u.w * v.w };
}

Vec4 operator/(const Vec4& u, const Vec4& v)
{
    return Vec4{ u.x / v.x, u.y / v.y, u.z / v.z, u.w / v.w };
}

Vec4 operator+(const Vec4& u, const Vec4& v)
{
    return Vec4{ u.x + v.x, u.y + v.y, u.z + v.z, u.w + v.w };
}

Vec4 operator-(const Vec4& u, const Vec4& v)
{
    return Vec4{ u.x - v.x, u.y - v.y, u.z - v.z, u.w - v.w };
}

Vec4 operator*(const Vec4& u, float s)
{
    return Vec4{ u.x * s, u.y * s, u.z * s, u.w * s };
}

Vec4 operator/(const Vec4& u, float s)
{
    float invS = 1.0f / s;
    return Vec4{ u.x * invS, u.y * invS, u.z * invS, u.w * invS };
}

Vec4 operator*(float s, const Vec4& u)
{
    return Vec4{ u.x * s, u.y * s, u.z * s, u.w * s };
}

Vec4 Normalize(const Vec4& u)
{
    float m = std::sqrt(u.x * u.x + u.y * u.y + u.z * u.z + u.w * u.w);
    float s = 1.0f / m;
    return Vec4{ u.x * s, u.y * s, u.z * s, u.w * s };
}

float Dot(const Vec4& u, const Vec4& v)
{
    return u.x * v.x + u.y * v.y + u.z * v.z + u.w * v.w;
}

} // namespace gs
