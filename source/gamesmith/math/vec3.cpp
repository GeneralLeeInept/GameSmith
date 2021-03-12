#include "gspch.h"

#include "vec3.h"

#include <gamesmith/core/debug.h>
#include <gamesmith/math/math.h>

namespace gs
{

vec3::vec3(float f)
    : vec3(f, f, f)
{
}

vec3::vec3(float x_, float y_, float z_)
    : x(x_)
    , y(y_)
    , z(z_)
{
}

float& vec3::operator[](int ord)
{
    GS_ASSERT(ord >= 0 && ord < 3);
    if (ord == 0) return x;
    if (ord == 1) return y;
    return z;
}

float vec3::operator[](int ord) const
{
    GS_ASSERT(ord >= 0 && ord < 3);
    if (ord == 0) return x;
    if (ord == 1) return y;
    return z;
}

vec3& vec3::operator*=(const vec3& rhs)
{
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;
    return *this;
}

vec3& vec3::operator/=(const vec3& rhs)
{
    x /= rhs.x;
    y /= rhs.y;
    z /= rhs.z;
    return *this;
}

vec3& vec3::operator+=(const vec3& rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
}

vec3& vec3::operator-=(const vec3& rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
}

vec3& vec3::operator*=(float s)
{
    x *= s;
    y *= s;
    z *= s;
    return *this;
}

vec3& vec3::operator/=(float s)
{
    float invS = 1.0f / s;
    x *= invS;
    y *= invS;
    z *= invS;
    return *this;
}

float vec3::length() const
{
    return std::sqrt(x * x + y * y + z * z);
}

float vec3::lengthSq() const
{
    return x * x + y * y + z * z;
}

vec3 operator-(const vec3& u)
{
    return vec3{ -u.x, -u.y, -u.z };
}

bool operator==(const vec3& lhs, const vec3& rhs)
{
    return (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z);
}

vec3 operator*(const vec3& u, const vec3& v)
{
    return vec3{ u.x * v.x, u.y * v.y, u.z * v.z };
}

vec3 operator/(const vec3& u, const vec3& v)
{
    return vec3{ u.x / v.x, u.y / v.y, u.z / v.z };
}

vec3 operator+(const vec3& u, const vec3& v)
{
    return vec3{ u.x + v.x, u.y + v.y, u.z + v.z };
}

vec3 operator-(const vec3& u, const vec3& v)
{
    return vec3{ u.x - v.x, u.y - v.y, u.z - v.z };
}

vec3 operator*(const vec3& u, float s)
{
    return vec3{ u.x * s, u.y * s, u.z * s };
}

vec3 operator/(const vec3& u, float s)
{
    float invS = 1.0f / s;
    return vec3{ u.x * invS, u.y * invS, u.z * invS };
}

vec3 operator*(float s, const vec3& u)
{
    return vec3{ u.x * s, u.y * s, u.z * s };
}

vec3 normalize(const vec3& u)
{
    float m = std::sqrt(u.x * u.x + u.y * u.y + u.z * u.z);
    float s = 1.0f / m;
    return vec3{ u.x * s, u.y * s, u.z * s };
}

vec3 cross(const vec3& u, const vec3& v)
{
    return vec3{ u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x };
}

float dot(const vec3& u, const vec3& v)
{
    return u.x * v.x + u.y * v.y + u.z * v.z;
}

} // namespace gs
