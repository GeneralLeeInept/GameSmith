#pragma once

namespace gs
{

struct Vec3
{
    float x, y, z;

    Vec3() = default;
    explicit Vec3(float);
    Vec3(float, float, float);
    Vec3(const Vec3&) = default;
    Vec3(Vec3&&) = default;

    Vec3& operator=(const Vec3&) = default;
    Vec3& operator=(Vec3&&) = default;

    float& operator[](int);
    float operator[](int) const;

    Vec3& operator*=(const Vec3&);
    Vec3& operator/=(const Vec3&);
    Vec3& operator+=(const Vec3&);
    Vec3& operator-=(const Vec3&);
    Vec3& operator*=(float);
    Vec3& operator/=(float);

    float Length() const;
    float LengthSq() const;
};

bool operator==(const Vec3&, const Vec3&);

Vec3 operator-(const Vec3&);
Vec3 operator*(const Vec3&, const Vec3&);
Vec3 operator/(const Vec3&, const Vec3&);
Vec3 operator+(const Vec3&, const Vec3&);
Vec3 operator-(const Vec3&, const Vec3&);
Vec3 operator*(const Vec3&, float);
Vec3 operator/(const Vec3&, float);
Vec3 operator*(float, const Vec3&);

Vec3 Normalize(const Vec3&);
Vec3 Cross(const Vec3&, const Vec3&);
float Dot(const Vec3&, const Vec3&);

} // namespace gs