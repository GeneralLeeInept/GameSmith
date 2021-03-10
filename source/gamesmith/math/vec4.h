#pragma once

namespace gs
{

struct Vec3;

struct Vec4
{
    float x, y, z, w;

    Vec4() = default;
    explicit Vec4(float);
    Vec4(float, float, float, float);
    Vec4(const Vec3&, float);
    Vec4(const Vec4&) = default;
    Vec4(Vec4&&) = default;

    Vec4& operator=(const Vec4&) = default;
    Vec4& operator=(Vec4&&) = default;

    float& operator[](int);
    float operator[](int) const;

    Vec4& operator*=(const Vec4&);
    Vec4& operator/=(const Vec4&);
    Vec4& operator+=(const Vec4&);
    Vec4& operator-=(const Vec4&);
    Vec4& operator*=(float);
    Vec4& operator/=(float);

    float Length() const;
    float LengthSq() const;
};

bool operator==(const Vec4&, const Vec4&);

Vec4 operator-(const Vec4&);
Vec4 operator*(const Vec4&, const Vec4&);
Vec4 operator/(const Vec4&, const Vec4&);
Vec4 operator+(const Vec4&, const Vec4&);
Vec4 operator-(const Vec4&, const Vec4&);
Vec4 operator*(const Vec4&, float);
Vec4 operator/(const Vec4&, float);
Vec4 operator*(float, const Vec4&);

Vec4 Normalize(const Vec4&);
float Dot(const Vec4&, const Vec4&);

} // namespace gs