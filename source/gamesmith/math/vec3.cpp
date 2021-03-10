#include "gspch.h"

#include "vec3.h"

#include <gamesmith/core/debug.h>
#include <gamesmith/math/math.h>

namespace gs
{

Vec3::Vec3(float f)
    : Vec3(f, f, f)
{
}

Vec3::Vec3(float x_, float y_, float z_)
    : x(x_)
    , y(y_)
    , z(z_)
{
}

float& Vec3::operator[](int ord)
{
    GS_ASSERT(ord >= 0 && ord < 3);
    if (ord == 0) return x;
    if (ord == 1) return y;
    return z;
}

float Vec3::operator[](int ord) const
{
    GS_ASSERT(ord >= 0 && ord < 3);
    if (ord == 0) return x;
    if (ord == 1) return y;
    return z;
}

Vec3& Vec3::operator*=(const Vec3& rhs)
{
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;
    return *this;
}

Vec3& Vec3::operator/=(const Vec3& rhs)
{
    x /= rhs.x;
    y /= rhs.y;
    z /= rhs.z;
    return *this;
}

Vec3& Vec3::operator+=(const Vec3& rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
}

Vec3& Vec3::operator-=(const Vec3& rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
}

Vec3& Vec3::operator*=(float s)
{
    x *= s;
    y *= s;
    z *= s;
    return *this;
}

Vec3& Vec3::operator/=(float s)
{
    float invS = 1.0f / s;
    x *= invS;
    y *= invS;
    z *= invS;
    return *this;
}

float Vec3::Length() const
{
    return std::sqrt(x * x + y * y + z * z);
}

float Vec3::LengthSq() const
{
    return x * x + y * y + z * z;
}

Vec3 operator-(const Vec3& u)
{
    return Vec3{ -u.x, -u.y, -u.z };
}

bool operator==(const Vec3& lhs, const Vec3& rhs)
{
    return (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z);
}

Vec3 operator*(const Vec3& u, const Vec3& v)
{
    return Vec3{ u.x * v.x, u.y * v.y, u.z * v.z };
}

Vec3 operator/(const Vec3& u, const Vec3& v)
{
    return Vec3{ u.x / v.x, u.y / v.y, u.z / v.z };
}

Vec3 operator+(const Vec3& u, const Vec3& v)
{
    return Vec3{ u.x + v.x, u.y + v.y, u.z + v.z };
}

Vec3 operator-(const Vec3& u, const Vec3& v)
{
    return Vec3{ u.x - v.x, u.y - v.y, u.z - v.z };
}

Vec3 operator*(const Vec3& u, float s)
{
    return Vec3{ u.x * s, u.y * s, u.z * s };
}

Vec3 operator/(const Vec3& u, float s)
{
    float invS = 1.0f / s;
    return Vec3{ u.x * invS, u.y * invS, u.z * invS };
}

Vec3 operator*(float s, const Vec3& u)
{
    return Vec3{ u.x * s, u.y * s, u.z * s };
}

Vec3 Normalize(const Vec3& u)
{
    float m = std::sqrt(u.x * u.x + u.y * u.y + u.z * u.z);
    float s = 1.0f / m;
    return Vec3{ u.x * s, u.y * s, u.z * s };
}

Vec3 Cross(const Vec3& u, const Vec3& v)
{
    return Vec3{ u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x };
}

float Dot(const Vec3& u, const Vec3& v)
{
    return u.x * v.x + u.y * v.y + u.z * v.z;
}

} // namespace gs
