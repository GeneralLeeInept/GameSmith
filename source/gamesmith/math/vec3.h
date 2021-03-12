#pragma once

namespace gs
{

struct vec3
{
    float x, y, z;

    vec3() = default;
    explicit vec3(float);
    vec3(float, float, float);
    vec3(const vec3&) = default;
    vec3(vec3&&) = default;

    vec3& operator=(const vec3&) = default;
    vec3& operator=(vec3&&) = default;

    float& operator[](int);
    float operator[](int) const;

    vec3& operator*=(const vec3&);
    vec3& operator/=(const vec3&);
    vec3& operator+=(const vec3&);
    vec3& operator-=(const vec3&);
    vec3& operator*=(float);
    vec3& operator/=(float);

    float length() const;
    float lengthSq() const;
};

bool operator==(const vec3&, const vec3&);

vec3 operator-(const vec3&);
vec3 operator*(const vec3&, const vec3&);
vec3 operator/(const vec3&, const vec3&);
vec3 operator+(const vec3&, const vec3&);
vec3 operator-(const vec3&, const vec3&);
vec3 operator*(const vec3&, float);
vec3 operator/(const vec3&, float);
vec3 operator*(float, const vec3&);

vec3 normalize(const vec3&);
vec3 cross(const vec3&, const vec3&);
float dot(const vec3&, const vec3&);

} // namespace gs