#include "gspch.h"

#include "mat44.h"

#include <gamesmith/core/debug.h>
#include <gamesmith/math/math.h>

namespace gs
{

Mat44::Mat44()
    : Mat44(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f)
{
}

Mat44::Mat44(const Vec4& X_, const Vec4& Y_, const Vec4& Z_, const Vec4& P_)
    : X(X_)
    , Y(Y_)
    , Z(Z_)
    , P(P_)
{
}

Mat44::Mat44(float Xx, float Xy, float Xz, float Xw, float Yx, float Yy, float Yz, float Yw, float Zx, float Zy, float Zz, float Zw, float Px,
             float Py, float Pz, float Pw)
    : X(Xx, Xy, Xz, Xw)
    , Y(Yx, Yy, Yz, Yw)
    , Z(Zx, Zy, Zz, Zw)
    , P(Px, Py, Pz, Pw)
{
}

Vec4& Mat44::operator[](int ord)
{
    GS_ASSERT(ord >= 0 && ord < 4);
    if (ord == 0) return X;
    if (ord == 1) return Y;
    if (ord == 2) return Z;
    return P;
}

Vec4 Mat44::operator[](int ord) const
{
    GS_ASSERT(ord >= 0 && ord < 4);
    if (ord == 0) return X;
    if (ord == 1) return Y;
    if (ord == 2) return Z;
    return P;
}

Mat44& Mat44::operator*=(const Mat44& m)
{
    float Xx = X.x * m.X.x + Y.x * m.X.y + Z.x * m.X.z + P.x * m.X.w;
    float Yx = X.x * m.Y.x + Y.x * m.Y.y + Z.x * m.Y.z + P.x * m.Y.w;
    float Zx = X.x * m.Z.x + Y.x * m.Z.y + Z.x * m.Z.z + P.x * m.Z.w;
    float Px = X.x * m.P.x + Y.x * m.P.y + Z.x * m.P.z + P.x * m.P.w;
    float Xy = X.y * m.X.x + Y.y * m.X.y + Z.y * m.X.z + P.y * m.X.w;
    float Yy = X.y * m.Y.x + Y.y * m.Y.y + Z.y * m.Y.z + P.y * m.Y.w;
    float Zy = X.y * m.Z.x + Y.y * m.Z.y + Z.y * m.Z.z + P.y * m.Z.w;
    float Py = X.y * m.P.x + Y.y * m.P.y + Z.y * m.P.z + P.y * m.P.w;
    float Xz = X.z * m.X.x + Y.z * m.X.y + Z.z * m.X.z + P.z * m.X.w;
    float Yz = X.z * m.Y.x + Y.z * m.Y.y + Z.z * m.Y.z + P.z * m.Y.w;
    float Zz = X.z * m.Z.x + Y.z * m.Z.y + Z.z * m.Z.z + P.z * m.Z.w;
    float Pz = X.z * m.P.x + Y.z * m.P.y + Z.z * m.P.z + P.z * m.P.w;
    float Xw = X.w * m.X.x + Y.w * m.X.y + Z.w * m.X.z + P.w * m.X.w;
    float Yw = X.w * m.Y.x + Y.w * m.Y.y + Z.w * m.Y.z + P.w * m.Y.w;
    float Zw = X.w * m.Z.x + Y.w * m.Z.y + Z.w * m.Z.z + P.w * m.Z.w;
    float Pw = X.w * m.P.x + Y.w * m.P.y + Z.w * m.P.z + P.w * m.P.w;
    X = { Xx, Xy, Xz, Xw };
    Y = { Yx, Yy, Yz, Yw };
    Z = { Zx, Zy, Zz, Zw };
    P = { Px, Py, Pz, Pw };
    return *this;
}

Mat44& Mat44::operator*=(float s)
{
    X *= s;
    Y *= s;
    Z *= s;
    P *= s;
    return *this;
}

Mat44& Mat44::operator/=(float s)
{
    float invS = 1.0f / s;
    X *= invS;
    Y *= invS;
    Z *= invS;
    P *= invS;
    return *this;
}


float Mat44::Determinant() const
{
    float minorXx = (Y.y * Z.z * P.w + Z.y * P.z * Y.w + P.y * Y.z * Z.w) - (P.y * Z.z * Y.w + Z.y * Y.z * P.w + Y.y * P.z * Z.w);
    float minorXy = (Y.x * Z.z * P.w + Z.x * P.z * Y.w + P.x * Y.z * Z.w) - (P.x * Z.z * Y.w + Z.x * Y.z * P.w + Y.x * P.z * Z.w);
    float minorXz = (Y.x * Z.y * P.w + Z.x * P.y * Y.w + P.x * Y.y * Z.w) - (P.x * Z.y * Y.w + Z.x * Y.y * P.w + Y.x * P.y * Z.w);
    float minorXw = (Y.x * Z.y * P.z + Z.x * P.y * Y.z + P.x * Y.y * Z.z) - (P.x * Z.y * Y.z + Z.x * Y.y * P.z + Y.x * P.y * Z.z);
    return X.x * minorXx - X.y * minorXy + X.z * minorXz - X.w * minorXw;
}

Mat44 operator*(const Mat44& a, const Mat44& b)
{
    float Xx = a.X.x * b.X.x + a.Y.x * b.X.y + a.Z.x * b.X.z + a.P.x * b.X.w;
    float Yx = a.X.x * b.Y.x + a.Y.x * b.Y.y + a.Z.x * b.Y.z + a.P.x * b.Y.w;
    float Zx = a.X.x * b.Z.x + a.Y.x * b.Z.y + a.Z.x * b.Z.z + a.P.x * b.Z.w;
    float Px = a.X.x * b.P.x + a.Y.x * b.P.y + a.Z.x * b.P.z + a.P.x * b.P.w;
    float Xy = a.X.y * b.X.x + a.Y.y * b.X.y + a.Z.y * b.X.z + a.P.y * b.X.w;
    float Yy = a.X.y * b.Y.x + a.Y.y * b.Y.y + a.Z.y * b.Y.z + a.P.y * b.Y.w;
    float Zy = a.X.y * b.Z.x + a.Y.y * b.Z.y + a.Z.y * b.Z.z + a.P.y * b.Z.w;
    float Py = a.X.y * b.P.x + a.Y.y * b.P.y + a.Z.y * b.P.z + a.P.y * b.P.w;
    float Xz = a.X.z * b.X.x + a.Y.z * b.X.y + a.Z.z * b.X.z + a.P.z * b.X.w;
    float Yz = a.X.z * b.Y.x + a.Y.z * b.Y.y + a.Z.z * b.Y.z + a.P.z * b.Y.w;
    float Zz = a.X.z * b.Z.x + a.Y.z * b.Z.y + a.Z.z * b.Z.z + a.P.z * b.Z.w;
    float Pz = a.X.z * b.P.x + a.Y.z * b.P.y + a.Z.z * b.P.z + a.P.z * b.P.w;
    float Xw = a.X.w * b.X.x + a.Y.w * b.X.y + a.Z.w * b.X.z + a.P.w * b.X.w;
    float Yw = a.X.w * b.Y.x + a.Y.w * b.Y.y + a.Z.w * b.Y.z + a.P.w * b.Y.w;
    float Zw = a.X.w * b.Z.x + a.Y.w * b.Z.y + a.Z.w * b.Z.z + a.P.w * b.Z.w;
    float Pw = a.X.w * b.P.x + a.Y.w * b.P.y + a.Z.w * b.P.z + a.P.w * b.P.w;
    return Mat44({ Xx, Xy, Xz, Xw }, { Yx, Yy, Yz, Yw }, { Zx, Zy, Zz, Zw }, { Px, Py, Pz, Pw });
}

Vec4 operator*(const Mat44& m, const Vec4& v)
{
    float x = m.X.x * v.x + m.Y.x * v.y + m.Z.x * v.z + m.P.x * v.w;
    float y = m.X.y * v.x + m.Y.y * v.y + m.Z.y * v.z + m.P.y * v.w;
    float z = m.X.z * v.x + m.Y.z * v.y + m.Z.z * v.z + m.P.z * v.w;
    float w = m.X.w * v.x + m.Y.w * v.y + m.Z.w * v.z + m.P.w * v.w;
    return Vec4(x, y, z, w);
}

Mat44 operator*(const Mat44& m, float s)
{
    return Mat44{ m.X * s, m.Y * s, m.Z * s, m.P * s };
}

Mat44 operator*(float s, const Mat44& m)
{
    return Mat44{ m.X * s, m.Y * s, m.Z * s, m.P * s };
}

Mat44 operator/(const Mat44& m, float s)
{
    float invS = 1.0f / s;
    return Mat44{ m.X * invS, m.Y * invS, m.Z * invS, m.P * invS };
}

Mat44 Transpose(const Mat44& m)
{
    return Mat44({ m.X.x, m.Y.x, m.Z.x, m.P.x }, { m.X.y, m.Y.y, m.Z.y, m.P.y }, { m.X.z, m.Y.z, m.Z.z, m.P.z }, { m.X.w, m.Y.w, m.Z.w, m.P.w });
}

Mat44 Inverse(const Mat44& m)
{
    float minorXx = (m.Y.y * m.Z.z * m.P.w + m.Z.y * m.P.z * m.Y.w + m.P.y * m.Y.z * m.Z.w) -
                    (m.P.y * m.Z.z * m.Y.w + m.Z.y * m.Y.z * m.P.w + m.Y.y * m.P.z * m.Z.w);
    float minorXy = (m.Y.x * m.Z.z * m.P.w + m.Z.x * m.P.z * m.Y.w + m.P.x * m.Y.z * m.Z.w) -
                    (m.P.x * m.Z.z * m.Y.w + m.Z.x * m.Y.z * m.P.w + m.Y.x * m.P.z * m.Z.w);
    float minorXz = (m.Y.x * m.Z.y * m.P.w + m.Z.x * m.P.y * m.Y.w + m.P.x * m.Y.y * m.Z.w) -
                    (m.P.x * m.Z.y * m.Y.w + m.Z.x * m.Y.y * m.P.w + m.Y.x * m.P.y * m.Z.w);
    float minorXw = (m.Y.x * m.Z.y * m.P.z + m.Z.x * m.P.y * m.Y.z + m.P.x * m.Y.y * m.Z.z) -
                    (m.P.x * m.Z.y * m.Y.z + m.Z.x * m.Y.y * m.P.z + m.Y.x * m.P.y * m.Z.z);
    float minorYx = (m.X.y * m.Z.z * m.P.w + m.Z.y * m.P.z * m.X.w + m.P.y * m.X.z * m.Z.w) -
                    (m.P.y * m.Z.z * m.X.w + m.Z.y * m.X.z * m.P.w + m.X.y * m.P.z * m.Z.w);
    float minorYy = (m.X.x * m.Z.z * m.P.w + m.Z.x * m.P.z * m.X.w + m.P.x * m.X.z * m.Z.w) -
                    (m.P.x * m.Z.z * m.X.w + m.Z.x * m.X.z * m.P.w + m.X.x * m.P.z * m.Z.w);
    float minorYz = (m.X.x * m.Z.y * m.P.w + m.Z.x * m.P.y * m.X.w + m.P.x * m.X.y * m.Z.w) -
                    (m.P.x * m.Z.y * m.X.w + m.Z.x * m.X.y * m.P.w + m.X.x * m.P.y * m.Z.w);
    float minorYw = (m.X.x * m.Z.y * m.P.z + m.Z.x * m.P.y * m.X.z + m.P.x * m.X.y * m.Z.z) -
                    (m.P.x * m.Z.y * m.X.z + m.Z.x * m.X.y * m.P.z + m.X.x * m.P.y * m.Z.z);
    float minorZx = (m.X.y * m.Y.z * m.P.w + m.Y.y * m.P.z * m.X.w + m.P.y * m.X.z * m.Y.w) -
                    (m.P.y * m.Y.z * m.X.w + m.Y.y * m.X.z * m.P.w + m.X.y * m.P.z * m.Y.w);
    float minorZy = (m.X.x * m.Y.z * m.P.w + m.Y.x * m.P.z * m.X.w + m.P.x * m.X.z * m.Y.w) -
                    (m.P.x * m.Y.z * m.X.w + m.Y.x * m.X.z * m.P.w + m.X.x * m.P.z * m.Y.w);
    float minorZz = (m.X.x * m.Y.y * m.P.w + m.Y.x * m.P.y * m.X.w + m.P.x * m.X.y * m.Y.w) -
                    (m.P.x * m.Y.y * m.X.w + m.Y.x * m.X.y * m.P.w + m.X.x * m.P.y * m.Y.w);
    float minorZw = (m.X.x * m.Y.y * m.P.z + m.Y.x * m.P.y * m.X.z + m.P.x * m.X.y * m.Y.z) -
                    (m.P.x * m.Y.y * m.X.z + m.Y.x * m.X.y * m.P.z + m.X.x * m.P.y * m.Y.z);
    float minorPx = (m.X.y * m.Y.z * m.Z.w + m.Y.y * m.Z.z * m.X.w + m.Z.y * m.X.z * m.Y.w) -
                    (m.Z.y * m.Y.z * m.X.w + m.Y.y * m.X.z * m.Z.w + m.X.y * m.Z.z * m.Y.w);
    float minorPy = (m.X.x * m.Y.z * m.Z.w + m.Y.x * m.Z.z * m.X.w + m.Z.x * m.X.z * m.Y.w) -
                    (m.Z.x * m.Y.z * m.X.w + m.Y.x * m.X.z * m.Z.w + m.X.x * m.Z.z * m.Y.w);
    float minorPz = (m.X.x * m.Y.y * m.Z.w + m.Y.x * m.Z.y * m.X.w + m.Z.x * m.X.y * m.Y.w) -
                    (m.Z.x * m.Y.y * m.X.w + m.Y.x * m.X.y * m.Z.w + m.X.x * m.Z.y * m.Y.w);
    float minorPw = (m.X.x * m.Y.y * m.Z.z + m.Y.x * m.Z.y * m.X.z + m.Z.x * m.X.y * m.Y.z) -
                    (m.Z.x * m.Y.y * m.X.z + m.Y.x * m.X.y * m.Z.z + m.X.x * m.Z.y * m.Y.z);

    Mat44 adjugate({ +minorXx, -minorYx, +minorZx, -minorPx }, { -minorXy, +minorYy, -minorZy, +minorPy }, { +minorXz, -minorYz, +minorZz, -minorPz },
                   { -minorXw, +minorYw, -minorZw, +minorPw });

    float determinant = m.X.x * minorXx - m.X.y * minorXy + m.X.z * minorXz - m.X.w * minorXw;
    GS_ASSERT(determinant != 0.0f);

    return (1.0f / determinant) * adjugate;
}

Mat44 InverseFast(const Mat44& m)
{
    GS_ASSERT(!"Not implemented");
    return Inverse(m);
}

} // namespace gs
