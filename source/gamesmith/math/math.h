#pragma once

#include <cmath>

namespace gs
{
namespace m
{
static constexpr float pi = 3.14159265358979323846f;
}

constexpr float DegToRad(float r)
{
    return r * 180.0f / m::pi;
}

constexpr float RadToDeg(float d)
{
    return d * m::pi / 180.0f;
}

constexpr float Lerp(float a, float b, float t)
{
    return a * (1.0f - t) + b * t;
}

} // namespace gs
