#pragma once

#include <cmath>

namespace gs
{
namespace m
{
static constexpr float pi = 3.14159265358979323846f;
}

constexpr float radToDeg(float r)
{
    return r * 180.0f / m::pi;
}

constexpr float degToRad(float d)
{
    return d * m::pi / 180.0f;
}

constexpr float lerp(float a, float b, float t)
{
    return a * (1.0f - t) + b * t;
}

} // namespace gs
