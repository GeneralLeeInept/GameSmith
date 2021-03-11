#pragma once

namespace gs
{

struct Vec3;

struct alignas(16) Vec4
{
    float x, y, z, w;

    Vec4() = default;
    explicit Vec4(float);
    constexpr Vec4(float, float, float, float);
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

    constexpr static Vec4 UnitX() { return Vec4(1.f, 0.f, 0.f, 0.f); }
    constexpr static Vec4 UnitY() { return Vec4(0.f, 1.f, 0.f, 0.f); }
    constexpr static Vec4 UnitZ() { return Vec4(0.f, 0.f, 1.f, 0.f); }
    constexpr static Vec4 UnitW() { return Vec4(0.f, 0.f, 0.f, 1.f); }
};

bool operator==(const Vec4& u, const Vec4& v);

Vec4 operator-(const Vec4& u);
Vec4 operator*(const Vec4& u, const Vec4& v);
Vec4 operator/(const Vec4& u, const Vec4& v);
Vec4 operator+(const Vec4& u, const Vec4& v);
Vec4 operator-(const Vec4& u, const Vec4& v);
Vec4 operator*(const Vec4& u, float s);
Vec4 operator/(const Vec4& u, float s);
Vec4 operator*(float s, const Vec4& u);

Vec4 Normalize(const Vec4& u);
float Dot(const Vec4& u, const Vec4& v);

} // namespace gs