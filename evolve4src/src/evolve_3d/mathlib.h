#pragma once
//-----------------------------------------------------------------------------
// Copyright (c) 2005-2006 dhpoware. All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------
//
// This is a stripped down version of the dhpoware 3D Math Library. To download
// the full version visit: http://www.dhpoware.com/source/mathlib.html
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Common math functions and constants.
//-----------------------------------------------------------------------------

class Math
{
public:
    static const float PI;
    static const float HALF_PI;
    static const float EPSILON;

    static bool closeEnough(float f1, float f2)
    {
        // Determines whether the two floating-point values f1 and f2 are
        // close enough together that they can be considered equal.

        return fabsf((f1 - f2) / ((f2 == 0.0f) ? 1.0f : f2)) < EPSILON;
    }

    static float degreesToRadians(float degrees)
    {
        return (degrees * PI) / 180.0f;
    }

    static float radiansToDegrees(float radians)
    {
        return (radians * 180.0f) / PI;
    }
};

//-----------------------------------------------------------------------------
// A 3-component vector class that represents a row vector.
//-----------------------------------------------------------------------------

class Vector3
{
    friend Vector3 operator*(float lhs, const Vector3 &rhs);
    friend Vector3 operator-(const Vector3 &v);

public:
    float x, y, z;

    static Vector3 cross(const Vector3 &p, const Vector3 &q);
    static float dot(const Vector3 &p, const Vector3 &q);

    Vector3() {}
    Vector3(float x_, float y_, float z_);
    ~Vector3() {}

    bool operator==(const Vector3 &rhs) const;
    bool operator!=(const Vector3 &rhs) const;

    Vector3 &operator+=(const Vector3 &rhs);
    Vector3 &operator-=(const Vector3 &rhs);
    Vector3 &operator*=(float scalar);
    Vector3 &operator/=(float scalar);

    Vector3 operator+(const Vector3 &rhs) const;
    Vector3 operator-(const Vector3 &rhs) const;
    Vector3 operator*(float scalar) const;
    Vector3 operator/(float scalar) const;

    float magnitude() const;
    float magnitudeSq() const;
    Vector3 inverse() const;
    void normalize();
    void set(float x_, float y_, float z_);
};

inline Vector3 operator*(float lhs, const Vector3 &rhs)
{
    return Vector3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}

inline Vector3 operator-(const Vector3 &v)
{
    return Vector3(-v.x, -v.y, -v.z);
}

inline Vector3 Vector3::cross(const Vector3 &p, const Vector3 &q)
{
    return Vector3((p.y * q.z) - (p.z * q.y),
        (p.z * q.x) - (p.x * q.z),
        (p.x * q.y) - (p.y * q.x));
}

inline float Vector3::dot(const Vector3 &p, const Vector3 &q)
{
    return (p.x * q.x) + (p.y * q.y) + (p.z * q.z);
}

inline Vector3::Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

inline Vector3 &Vector3::operator+=(const Vector3 &rhs)
{
    x += rhs.x, y += rhs.y, z += rhs.z;
    return *this;
}

inline bool Vector3::operator==(const Vector3 &rhs) const
{
    return Math::closeEnough(x, rhs.x) && Math::closeEnough(y, rhs.y)
        && Math::closeEnough(z, rhs.z);
}

inline bool Vector3::operator!=(const Vector3 &rhs) const
{
    return !(*this == rhs);
}

inline Vector3 &Vector3::operator-=(const Vector3 &rhs)
{
    x -= rhs.x, y -= rhs.y, z -= rhs.z;
    return *this;
}

inline Vector3 &Vector3::operator*=(float scalar)
{
    x *= scalar, y *= scalar, z *= scalar;
    return *this;
}

inline Vector3 &Vector3::operator/=(float scalar)
{
    x /= scalar, y /= scalar, z /= scalar;
    return *this;
}

inline Vector3 Vector3::operator+(const Vector3 &rhs) const
{
    Vector3 tmp(*this);
    tmp += rhs;
    return tmp;
}

inline Vector3 Vector3::operator-(const Vector3 &rhs) const
{
    Vector3 tmp(*this);
    tmp -= rhs;
    return tmp;
}

inline Vector3 Vector3::operator*(float scalar) const
{
    return Vector3(x * scalar, y * scalar, z * scalar);    
}

inline Vector3 Vector3::operator/(float scalar) const
{
    return Vector3(x / scalar, y / scalar, z / scalar);
}

inline float Vector3::magnitude() const
{
    return sqrtf((x * x) + (y * y) + (z * z));
}

inline float Vector3::magnitudeSq() const
{
    return (x * x) + (y * y) + (z * z);
}

inline Vector3 Vector3::inverse() const
{
    return Vector3(-x, -y, -z);
}

inline void Vector3::normalize()
{
    float invMag = 1.0f / magnitude();
    x *= invMag, y *= invMag, z *= invMag;
}

inline void Vector3::set(float x_, float y_, float z_)
{
    x = x_, y = y_, z = z_;
}

//-----------------------------------------------------------------------------
// A homogeneous row-major 4x4 matrix class.
//
// Matrices are concatenated in a left to right order.
// Multiplies Vector3s to the left of the matrix.
//-----------------------------------------------------------------------------

class Matrix4
{
    friend Vector3 operator*(const Vector3 &lhs, const Matrix4 &rhs);
    friend Matrix4 operator*(float scalar, const Matrix4 &rhs);

public:
    static const Matrix4 IDENTITY;

    Matrix4() {}
    Matrix4(float m11, float m12, float m13, float m14,
            float m21, float m22, float m23, float m24,
            float m31, float m32, float m33, float m34,
            float m41, float m42, float m43, float m44);
    ~Matrix4() {}

    float *operator[](int row);
    const float *operator[](int row) const;

    bool operator==(const Matrix4 &rhs) const;
    bool operator!=(const Matrix4 &rhs) const;

    Matrix4 &operator+=(const Matrix4 &rhs);
    Matrix4 &operator-=(const Matrix4 &rhs);
    Matrix4 &operator*=(const Matrix4 &rhs);
    Matrix4 &operator*=(float scalar);
    Matrix4 &operator/=(float scalar);

    Matrix4 operator+(const Matrix4 &rhs) const;
    Matrix4 operator-(const Matrix4 &rhs) const;
    Matrix4 operator*(const Matrix4 &rhs) const;
    Matrix4 operator*(float scalar) const;
    Matrix4 operator/(float scalar) const;

    void fromHeadPitchRoll(float headDegrees, float pitchDegrees, float rollDegrees);
    void identity();
    void rotate(const Vector3 &axis, float degrees);
    void toHeadPitchRoll(float &headDegrees, float &pitchDegrees, float &rollDegrees) const;

private:
    float mtx[4][4];
};

inline Vector3 operator*(const Vector3 &lhs, const Matrix4 &rhs)
{
    return Vector3((lhs.x * rhs.mtx[0][0]) + (lhs.y * rhs.mtx[1][0]) + (lhs.z * rhs.mtx[2][0]),
        (lhs.x * rhs.mtx[0][1]) + (lhs.y * rhs.mtx[1][1]) + (lhs.z * rhs.mtx[2][1]),
        (lhs.x * rhs.mtx[0][2]) + (lhs.y * rhs.mtx[1][2]) + (lhs.z * rhs.mtx[2][2]));
}

inline Matrix4 operator*(float scalar, const Matrix4 &rhs)
{
    return rhs * scalar;
}

inline Matrix4::Matrix4(float m11, float m12, float m13, float m14,
                      float m21, float m22, float m23, float m24,
                      float m31, float m32, float m33, float m34,
                      float m41, float m42, float m43, float m44)
{
    mtx[0][0] = m11, mtx[0][1] = m12, mtx[0][2] = m13, mtx[0][3] = m14;
    mtx[1][0] = m21, mtx[1][1] = m22, mtx[1][2] = m23, mtx[1][3] = m24;
    mtx[2][0] = m31, mtx[2][1] = m32, mtx[2][2] = m33, mtx[2][3] = m34;
    mtx[3][0] = m41, mtx[3][1] = m42, mtx[3][2] = m43, mtx[3][3] = m44;
}

inline float *Matrix4::operator[](int row)
{
    return mtx[row];
}

inline const float *Matrix4::operator[](int row) const
{
    return mtx[row];
}

inline bool Matrix4::operator==(const Matrix4 &rhs) const
{
    return Math::closeEnough(mtx[0][0], rhs.mtx[0][0])
        && Math::closeEnough(mtx[0][1], rhs.mtx[0][1])
        && Math::closeEnough(mtx[0][2], rhs.mtx[0][2])
        && Math::closeEnough(mtx[0][3], rhs.mtx[0][3])
        && Math::closeEnough(mtx[1][0], rhs.mtx[1][0])
        && Math::closeEnough(mtx[1][1], rhs.mtx[1][1])
        && Math::closeEnough(mtx[1][2], rhs.mtx[1][2])
        && Math::closeEnough(mtx[1][3], rhs.mtx[1][3])
        && Math::closeEnough(mtx[2][0], rhs.mtx[2][0])
        && Math::closeEnough(mtx[2][1], rhs.mtx[2][1])
        && Math::closeEnough(mtx[2][2], rhs.mtx[2][2])
        && Math::closeEnough(mtx[2][3], rhs.mtx[2][3])
        && Math::closeEnough(mtx[3][0], rhs.mtx[3][0])
        && Math::closeEnough(mtx[3][1], rhs.mtx[3][1])
        && Math::closeEnough(mtx[3][2], rhs.mtx[3][2])
        && Math::closeEnough(mtx[3][3], rhs.mtx[3][3]);
}

inline bool Matrix4::operator!=(const Matrix4 &rhs) const
{
    return !(*this == rhs);
}

inline Matrix4 &Matrix4::operator+=(const Matrix4 &rhs)
{
    mtx[0][0] += rhs.mtx[0][0], mtx[0][1] += rhs.mtx[0][1], mtx[0][2] += rhs.mtx[0][2], mtx[0][3] += rhs.mtx[0][3];
    mtx[1][0] += rhs.mtx[1][0], mtx[1][1] += rhs.mtx[1][1], mtx[1][2] += rhs.mtx[1][2], mtx[1][3] += rhs.mtx[1][3];
    mtx[2][0] += rhs.mtx[2][0], mtx[2][1] += rhs.mtx[2][1], mtx[2][2] += rhs.mtx[2][2], mtx[2][3] += rhs.mtx[2][3];
    mtx[3][0] += rhs.mtx[3][0], mtx[3][1] += rhs.mtx[3][1], mtx[3][2] += rhs.mtx[3][2], mtx[3][3] += rhs.mtx[3][3];
    return *this;
}

inline Matrix4 &Matrix4::operator-=(const Matrix4 &rhs)
{
    mtx[0][0] -= rhs.mtx[0][0], mtx[0][1] -= rhs.mtx[0][1], mtx[0][2] -= rhs.mtx[0][2], mtx[0][3] -= rhs.mtx[0][3];
    mtx[1][0] -= rhs.mtx[1][0], mtx[1][1] -= rhs.mtx[1][1], mtx[1][2] -= rhs.mtx[1][2], mtx[1][3] -= rhs.mtx[1][3];
    mtx[2][0] -= rhs.mtx[2][0], mtx[2][1] -= rhs.mtx[2][1], mtx[2][2] -= rhs.mtx[2][2], mtx[2][3] -= rhs.mtx[2][3];
    mtx[3][0] -= rhs.mtx[3][0], mtx[3][1] -= rhs.mtx[3][1], mtx[3][2] -= rhs.mtx[3][2], mtx[3][3] -= rhs.mtx[3][3];
    return *this;
}

inline Matrix4 &Matrix4::operator*=(const Matrix4 &rhs)
{
    Matrix4 tmp;

    // Row 1.
    tmp.mtx[0][0] = (mtx[0][0] * rhs.mtx[0][0]) + (mtx[0][1] * rhs.mtx[1][0]) + (mtx[0][2] * rhs.mtx[2][0]) + (mtx[0][3] * rhs.mtx[3][0]);
    tmp.mtx[0][1] = (mtx[0][0] * rhs.mtx[0][1]) + (mtx[0][1] * rhs.mtx[1][1]) + (mtx[0][2] * rhs.mtx[2][1]) + (mtx[0][3] * rhs.mtx[3][1]);
    tmp.mtx[0][2] = (mtx[0][0] * rhs.mtx[0][2]) + (mtx[0][1] * rhs.mtx[1][2]) + (mtx[0][2] * rhs.mtx[2][2]) + (mtx[0][3] * rhs.mtx[3][2]);
    tmp.mtx[0][3] = (mtx[0][0] * rhs.mtx[0][3]) + (mtx[0][1] * rhs.mtx[1][3]) + (mtx[0][2] * rhs.mtx[2][3]) + (mtx[0][3] * rhs.mtx[3][3]);

    // Row 2.
    tmp.mtx[1][0] = (mtx[1][0] * rhs.mtx[0][0]) + (mtx[1][1] * rhs.mtx[1][0]) + (mtx[1][2] * rhs.mtx[2][0]) + (mtx[1][3] * rhs.mtx[3][0]);
    tmp.mtx[1][1] = (mtx[1][0] * rhs.mtx[0][1]) + (mtx[1][1] * rhs.mtx[1][1]) + (mtx[1][2] * rhs.mtx[2][1]) + (mtx[1][3] * rhs.mtx[3][1]);
    tmp.mtx[1][2] = (mtx[1][0] * rhs.mtx[0][2]) + (mtx[1][1] * rhs.mtx[1][2]) + (mtx[1][2] * rhs.mtx[2][2]) + (mtx[1][3] * rhs.mtx[3][2]);
    tmp.mtx[1][3] = (mtx[1][0] * rhs.mtx[0][3]) + (mtx[1][1] * rhs.mtx[1][3]) + (mtx[1][2] * rhs.mtx[2][3]) + (mtx[1][3] * rhs.mtx[3][3]);

    // Row 3.
    tmp.mtx[2][0] = (mtx[2][0] * rhs.mtx[0][0]) + (mtx[2][1] * rhs.mtx[1][0]) + (mtx[2][2] * rhs.mtx[2][0]) + (mtx[2][3] * rhs.mtx[3][0]);
    tmp.mtx[2][1] = (mtx[2][0] * rhs.mtx[0][1]) + (mtx[2][1] * rhs.mtx[1][1]) + (mtx[2][2] * rhs.mtx[2][1]) + (mtx[2][3] * rhs.mtx[3][1]);
    tmp.mtx[2][2] = (mtx[2][0] * rhs.mtx[0][2]) + (mtx[2][1] * rhs.mtx[1][2]) + (mtx[2][2] * rhs.mtx[2][2]) + (mtx[2][3] * rhs.mtx[3][2]);
    tmp.mtx[2][3] = (mtx[2][0] * rhs.mtx[0][3]) + (mtx[2][1] * rhs.mtx[1][3]) + (mtx[2][2] * rhs.mtx[2][3]) + (mtx[2][3] * rhs.mtx[3][3]);

    // Row 4.
    tmp.mtx[3][0] = (mtx[3][0] * rhs.mtx[0][0]) + (mtx[3][1] * rhs.mtx[1][0]) + (mtx[3][2] * rhs.mtx[2][0]) + (mtx[3][3] * rhs.mtx[3][0]);
    tmp.mtx[3][1] = (mtx[3][0] * rhs.mtx[0][1]) + (mtx[3][1] * rhs.mtx[1][1]) + (mtx[3][2] * rhs.mtx[2][1]) + (mtx[3][3] * rhs.mtx[3][1]);
    tmp.mtx[3][2] = (mtx[3][0] * rhs.mtx[0][2]) + (mtx[3][1] * rhs.mtx[1][2]) + (mtx[3][2] * rhs.mtx[2][2]) + (mtx[3][3] * rhs.mtx[3][2]);
    tmp.mtx[3][3] = (mtx[3][0] * rhs.mtx[0][3]) + (mtx[3][1] * rhs.mtx[1][3]) + (mtx[3][2] * rhs.mtx[2][3]) + (mtx[3][3] * rhs.mtx[3][3]);

    *this = tmp;
    return *this;
}

inline Matrix4 &Matrix4::operator*=(float scalar)
{
    mtx[0][0] *= scalar, mtx[0][1] *= scalar, mtx[0][2] *= scalar, mtx[0][3] *= scalar;
    mtx[1][0] *= scalar, mtx[1][1] *= scalar, mtx[1][2] *= scalar, mtx[1][3] *= scalar;
    mtx[2][0] *= scalar, mtx[2][1] *= scalar, mtx[2][2] *= scalar, mtx[2][3] *= scalar;
    mtx[3][0] *= scalar, mtx[3][1] *= scalar, mtx[3][2] *= scalar, mtx[3][3] *= scalar;
    return *this;
}

inline Matrix4 &Matrix4::operator/=(float scalar)
{
    mtx[0][0] /= scalar, mtx[0][1] /= scalar, mtx[0][2] /= scalar, mtx[0][3] /= scalar;
    mtx[1][0] /= scalar, mtx[1][1] /= scalar, mtx[1][2] /= scalar, mtx[1][3] /= scalar;
    mtx[2][0] /= scalar, mtx[2][1] /= scalar, mtx[2][2] /= scalar, mtx[2][3] /= scalar;
    mtx[3][0] /= scalar, mtx[3][1] /= scalar, mtx[3][2] /= scalar, mtx[3][3] /= scalar;
    return *this;
}

inline Matrix4 Matrix4::operator+(const Matrix4 &rhs) const
{
    Matrix4 tmp(*this);
    tmp += rhs;
    return tmp;
}

inline Matrix4 Matrix4::operator-(const Matrix4 &rhs) const
{
    Matrix4 tmp(*this);
    tmp -= rhs;
    return tmp;
}

inline Matrix4 Matrix4::operator*(const Matrix4 &rhs) const
{
    Matrix4 tmp(*this);
    tmp *= rhs;
    return tmp;
}

inline Matrix4 Matrix4::operator*(float scalar) const
{
    Matrix4 tmp(*this);
    tmp *= scalar;
    return tmp;
}

inline Matrix4 Matrix4::operator/(float scalar) const
{
    Matrix4 tmp(*this);
    tmp /= scalar;
    return tmp;
}

inline void Matrix4::identity()
{
    mtx[0][0] = 1.0f, mtx[0][1] = 0.0f, mtx[0][2] = 0.0f, mtx[0][3] = 0.0f;
    mtx[1][0] = 0.0f, mtx[1][1] = 1.0f, mtx[1][2] = 0.0f, mtx[1][3] = 0.0f;
    mtx[2][0] = 0.0f, mtx[2][1] = 0.0f, mtx[2][2] = 1.0f, mtx[2][3] = 0.0f;
    mtx[3][0] = 0.0f, mtx[3][1] = 0.0f, mtx[3][2] = 0.0f, mtx[3][3] = 1.0f;
}

//-----------------------------------------------------------------------------
// This Quaternion class will concatenate quaternions in a left to right order.
// The reason for this is to maintain the same multiplication semantics as the
// Matrix4 classes.
//-----------------------------------------------------------------------------

class Quaternion
{
    friend Quaternion operator*(float lhs, const Quaternion &rhs);

public:
    static const Quaternion IDENTITY;

    float w, x, y, z;

    Quaternion() {}
    Quaternion(float w_, float x_, float y_, float z_);
    Quaternion(float headDegrees, float pitchDegrees, float rollDegrees);
    Quaternion(const Vector3 &axis, float degrees);
    explicit Quaternion(const Matrix4 &m);
    ~Quaternion() {}

    bool operator==(const Quaternion &rhs) const;
    bool operator!=(const Quaternion &rhs) const;

    Quaternion &operator+=(const Quaternion &rhs);
    Quaternion &operator-=(const Quaternion &rhs);
    Quaternion &operator*=(const Quaternion &rhs);
    Quaternion &operator*=(float scalar);
    Quaternion &operator/=(float scalar);

    Quaternion operator+(const Quaternion &rhs) const;
    Quaternion operator-(const Quaternion &rhs) const;
    Quaternion operator*(const Quaternion &rhs) const;
    Quaternion operator*(float scalar) const;
    Quaternion operator/(float scalar) const;

    Quaternion conjugate() const;
    void fromAxisAngle(const Vector3 &axis, float degrees);
    void fromHeadPitchRoll(float headDegrees, float pitchDegrees, float rollDegrees);
    void fromMatrix(const Matrix4 &m);
    void identity();
    Quaternion inverse() const;
    float magnitude() const;
    void normalize();
    void set(float w_, float x_, float y_, float z_);
    void toAxisAngle(Vector3 &axis, float &degrees) const;
    void toHeadPitchRoll(float &headDegrees, float &pitchDegrees, float &rollDegrees) const;
    Matrix4 toMatrix4() const;
};

inline Quaternion operator*(float lhs, const Quaternion &rhs)
{
    return rhs * lhs;
}

inline Quaternion::Quaternion(float w_, float x_, float y_, float z_)
    : w(w_), x(x_), y(y_), z(z_) {}

inline Quaternion::Quaternion(float headDegrees, float pitchDegrees, float rollDegrees)
{
    fromHeadPitchRoll(headDegrees, pitchDegrees, rollDegrees);
}

inline Quaternion::Quaternion(const Vector3 &axis, float degrees)
{
    fromAxisAngle(axis, degrees);
}

inline Quaternion::Quaternion(const Matrix4 &m)
{
    fromMatrix(m);
}

inline bool Quaternion::operator==(const Quaternion &rhs) const
{
    return Math::closeEnough(w, rhs.w) && Math::closeEnough(x, rhs.x)
        && Math::closeEnough(y, rhs.y) && Math::closeEnough(z, rhs.z);
}

inline bool Quaternion::operator!=(const Quaternion &rhs) const
{
    return !(*this == rhs);
}

inline Quaternion &Quaternion::operator+=(const Quaternion &rhs)
{
    w += rhs.w, x += rhs.x, y += rhs.y, z += rhs.z;
    return *this;
}

inline Quaternion &Quaternion::operator-=(const Quaternion &rhs)
{
    w -= rhs.w, x -= rhs.x, y -= rhs.y, z -= rhs.z;
    return *this;
}

inline Quaternion &Quaternion::operator*=(const Quaternion &rhs)
{
    // Multiply so that rotations are applied in a left to right order.
    Quaternion tmp(
        (w * rhs.w) - (x * rhs.x) - (y * rhs.y) - (z * rhs.z),
        (w * rhs.x) + (x * rhs.w) - (y * rhs.z) + (z * rhs.y),
        (w * rhs.y) + (x * rhs.z) + (y * rhs.w) - (z * rhs.x),
        (w * rhs.z) - (x * rhs.y) + (y * rhs.x) + (z * rhs.w));

    /*
    // Multiply so that rotations are applied in a right to left order.
    Quaternion tmp(
    (w * rhs.w) - (x * rhs.x) - (y * rhs.y) - (z * rhs.z),
    (w * rhs.x) + (x * rhs.w) + (y * rhs.z) - (z * rhs.y),
    (w * rhs.y) - (x * rhs.z) + (y * rhs.w) + (z * rhs.x),
    (w * rhs.z) + (x * rhs.y) - (y * rhs.x) + (z * rhs.w));
    */

    *this = tmp;
    return *this;
}

inline Quaternion &Quaternion::operator*=(float scalar)
{
    w *= scalar, x *= scalar, y *= scalar, z *= scalar;
    return *this;
}

inline Quaternion &Quaternion::operator/=(float scalar)
{
    w /= scalar, x /= scalar, y /= scalar, z /= scalar;
    return *this;
}

inline Quaternion Quaternion::operator+(const Quaternion &rhs) const
{
    Quaternion tmp(*this);
    tmp += rhs;
    return tmp;
}

inline Quaternion Quaternion::operator-(const Quaternion &rhs) const
{
    Quaternion tmp(*this);
    tmp -= rhs;
    return tmp;
}

inline Quaternion Quaternion::operator*(const Quaternion &rhs) const
{
    Quaternion tmp(*this);
    tmp *= rhs;
    return tmp;
}

inline Quaternion Quaternion::operator*(float scalar) const
{
    Quaternion tmp(*this);
    tmp *= scalar;
    return tmp;
}

inline Quaternion Quaternion::operator/(float scalar) const
{
    Quaternion tmp(*this);
    tmp /= scalar;
    return tmp;
}

inline Quaternion Quaternion::conjugate() const
{
    Quaternion tmp(w, -x, -y, -z);
    return tmp;
}

inline void Quaternion::fromAxisAngle(const Vector3 &axis, float degrees)
{
    float halfTheta = Math::degreesToRadians(degrees) * 0.5f;
    float s = sinf(halfTheta);
    w = cosf(halfTheta), x = axis.x * s, y = axis.y * s, z = axis.z * s;
}

inline void Quaternion::fromHeadPitchRoll(float headDegrees, float pitchDegrees, float rollDegrees)
{
    Matrix4 m;
    m.fromHeadPitchRoll(headDegrees, pitchDegrees, rollDegrees);
    fromMatrix(m);
}

inline void Quaternion::identity()
{
    w = 1.0f, x = y = z = 0.0f;
}

inline Quaternion Quaternion::inverse() const
{
    float invMag = 1.0f / magnitude();
    return conjugate() * invMag;
}

inline float Quaternion::magnitude() const
{
    return sqrtf(w * w + x * x + y * y + z * z);
}

inline void Quaternion::normalize()
{
    float invMag = 1.0f / magnitude();
    w *= invMag, x *= invMag, y *= invMag, z *= invMag;
}

inline void Quaternion::set(float w_, float x_, float y_, float z_)
{
    w = w_, x = x_, y = y_, z = z_;
}

inline void Quaternion::toHeadPitchRoll(float &headDegrees, float &pitchDegrees, float &rollDegrees) const
{
    Matrix4 m = toMatrix4();
    m.toHeadPitchRoll(headDegrees, pitchDegrees, rollDegrees);
}

