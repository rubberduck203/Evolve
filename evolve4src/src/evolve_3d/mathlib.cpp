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

#include "stdafx.h"

const float Math::PI = 3.1415926f;
const float Math::HALF_PI = Math::PI / 2.0f;
const float Math::EPSILON = 1e-6f;

//-----------------------------------------------------------------------------
// Matrix4.
//-----------------------------------------------------------------------------

const Matrix4 Matrix4::IDENTITY(1.0f, 0.0f, 0.0f, 0.0f,
                              0.0f, 1.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, 1.0f, 0.0f,
                              0.0f, 0.0f, 0.0f, 1.0f);

void Matrix4::fromHeadPitchRoll(float headDegrees, float pitchDegrees, float rollDegrees)
{
    // Constructs a rotation matrix based on a Euler Transform.
    // I use the popular NASA standard airplane convention of 
    // heading-pitch-roll (i.e., RzRxRy).

    headDegrees = Math::degreesToRadians(headDegrees);
    pitchDegrees = Math::degreesToRadians(pitchDegrees);
    rollDegrees = Math::degreesToRadians(rollDegrees);

    float cosH = cosf(headDegrees);
    float cosP = cosf(pitchDegrees);
    float cosR = cosf(rollDegrees);
    float sinH = sinf(headDegrees);
    float sinP = sinf(pitchDegrees);
    float sinR = sinf(rollDegrees);

    mtx[0][0] = cosR * cosH - sinR * sinP * sinH;
    mtx[0][1] = sinR * cosH + cosR * sinP * sinH;
    mtx[0][2] = -cosP * sinH;
    mtx[0][3] = 0.0f;

    mtx[1][0] = -sinR * cosP;
    mtx[1][1] = cosR * cosP;
    mtx[1][2] = sinP;
    mtx[1][3] = 0.0f;

    mtx[2][0] = cosR * sinH + sinR * sinP * cosH;
    mtx[2][1] = sinR * sinH - cosR * sinP * cosH;
    mtx[2][2] = cosP * cosH;
    mtx[2][3] = 0.0f;

    mtx[3][0] = 0.0f;
    mtx[3][1] = 0.0f;
    mtx[3][2] = 0.0f;
    mtx[3][3] = 1.0f;
}

void Matrix4::rotate(const Vector3 &axis, float degrees)
{
    // Creates a rotation matrix about the specified axis.
    // The axis must be a unit vector. The angle must be in degrees.
    //
    // Let u = axis of rotation = (x, y, z)
    //
    //             | x^2(1 - c) + c  xy(1 - c) + zs  xz(1 - c) - ys   0 |
    // Ru(angle) = | yx(1 - c) - zs  y^2(1 - c) + c  yz(1 - c) + xs   0 |
    //             | zx(1 - c) - ys  zy(1 - c) - xs  z^2(1 - c) + c   0 |
    //             |      0              0                0           1 |
    //
    // where,
    //	c = cos(angle)
    //  s = sin(angle)

    degrees = Math::degreesToRadians(degrees);

    float x = axis.x;
    float y = axis.y;
    float z = axis.z;
    float c = cosf(degrees);
    float s = sinf(degrees);

    mtx[0][0] = (x * x) * (1.0f - c) + c;
    mtx[0][1] = (x * y) * (1.0f - c) + (z * s);
    mtx[0][2] = (x * z) * (1.0f - c) - (y * s);
    mtx[0][3] = 0.0f;

    mtx[1][0] = (y * x) * (1.0f - c) - (z * s);
    mtx[1][1] = (y * y) * (1.0f - c) + c;
    mtx[1][2] = (y * z) * (1.0f - c) + (x * s);
    mtx[1][3] = 0.0f;

    mtx[2][0] = (z * x) * (1.0f - c) + (y * s);
    mtx[2][1] = (z * y) * (1.0f - c) - (x * s);
    mtx[2][2] = (z * z) * (1.0f - c) + c;
    mtx[2][3] = 0.0f;

    mtx[3][0] = 0.0f;
    mtx[3][1] = 0.0f;
    mtx[3][2] = 0.0f;
    mtx[3][3] = 1.0f;
}

void Matrix4::toHeadPitchRoll(float &headDegrees, float &pitchDegrees, float &rollDegrees) const
{
    // Extracts the Euler angles from a rotation matrix. The returned
    // angles are in degrees. This method might suffer from numerical
    // imprecision for ill defined rotation matrices.
    //
    // This function only works for rotation matrices constructed using
    // the popular NASA standard airplane convention of heading-pitch-roll 
    // (i.e., RzRxRy).
    //
    // The algorithm used is from:
    //  David Eberly, "Euler Angle Formulas", Geometric Tools web site,
    //  http://www.geometrictools.com/Documentation/EulerAngles.pdf.

    float thetaX = asinf(mtx[1][2]);
    float thetaY = 0.0f;
    float thetaZ = 0.0f;

    if (thetaX < Math::HALF_PI)
    {
        if (thetaX > -Math::HALF_PI)
        {
            thetaZ = atan2f(-mtx[1][0], mtx[1][1]);
            thetaY = atan2f(-mtx[0][2], mtx[2][2]);
        }
        else
        {
            // Not a unique solution.
            thetaZ = -atan2f(mtx[2][0], mtx[0][0]);
            thetaY = 0.0f;
        }
    }
    else
    {
        // Not a unique solution.
        thetaZ = atan2f(mtx[2][0], mtx[0][0]);
        thetaY = 0.0f;
    }

    headDegrees = Math::radiansToDegrees(thetaY);
    pitchDegrees = Math::radiansToDegrees(thetaX);
    rollDegrees = Math::radiansToDegrees(thetaZ);
}

//-----------------------------------------------------------------------------
// Quaternion.
//-----------------------------------------------------------------------------

const Quaternion Quaternion::IDENTITY(1.0f, 0.0f, 0.0f, 0.0f);

void Quaternion::fromMatrix(const Matrix4 &m)
{
    // Creates a quaternion from a rotation matrix. 
    // The algorithm used is from Allan and Mark Watt's "Advanced 
    // Animation and Rendering Techniques" (ACM Press 1992).

    float s = 0.0f;
    float q[4] = {0.0f};
    float trace = m[0][0] + m[1][1] + m[2][2];

    if (trace > 0.0f)
    {
        s = sqrtf(trace + 1.0f);
        q[3] = s * 0.5f;
        s = 0.5f / s;
        q[0] = (m[1][2] - m[2][1]) * s;
        q[1] = (m[2][0] - m[0][2]) * s;
        q[2] = (m[0][1] - m[1][0]) * s;
    }
    else
    {
        int nxt[3] = {1, 2, 0};
        int i = 0, j = 0, k = 0;

        if (m[1][1] > m[0][0])
            i = 1;

        if (m[2][2] > m[i][i])
            i = 2;

        j = nxt[i];
        k = nxt[j];
        s = sqrtf((m[i][i] - (m[j][j] + m[k][k])) + 1.0f);

        q[i] = s * 0.5f;
        s = 0.5f / s;
        q[3] = (m[j][k] - m[k][j]) * s;
        q[j] = (m[i][j] + m[j][i]) * s;
        q[k] = (m[i][k] + m[k][i]) * s;
    }

    x = q[0], y = q[1], z = q[2], w = q[3];
}

void Quaternion::toAxisAngle(Vector3 &axis, float &degrees) const
{
    // Converts this quaternion to an axis and an angle.

    float sinHalfThetaSq = 1.0f - w * w;

    // Guard against numerical imprecision and identity quaternions.
    if (sinHalfThetaSq <= 0.0f)
    {
        axis.x = 1.0f, axis.y = axis.z = 0.0f;
        degrees = 0.0f;
    }
    else
    {
        float invSinHalfTheta = 1.0f / sqrtf(sinHalfThetaSq);

        axis.x = x * invSinHalfTheta;
        axis.y = y * invSinHalfTheta;
        axis.z = z * invSinHalfTheta;
        degrees = Math::radiansToDegrees(2.0f * acosf(w));
    }
}

Matrix4 Quaternion::toMatrix4() const
{
    // Converts this quaternion to a rotation matrix.
    //
    //  | 1 - 2(y^2 + z^2)	2(xy + wz)			2(xz - wy)			0  |
    //  | 2(xy - wz)		1 - 2(x^2 + z^2)	2(yz + wx)			0  |
    //  | 2(xz + wy)		2(yz - wx)			1 - 2(x^2 + y^2)	0  |
    //  | 0					0					0					1  |

    float x2 = x + x; 
    float y2 = y + y; 
    float z2 = z + z;
    float xx = x * x2;
    float xy = x * y2;
    float xz = x * z2;
    float yy = y * y2;
    float yz = y * z2;
    float zz = z * z2;
    float wx = w * x2;
    float wy = w * y2;
    float wz = w * z2;

    Matrix4 m;

    m[0][0] = 1.0f - (yy + zz);
    m[0][1] = xy + wz;
    m[0][2] = xz - wy;
    m[0][3] = 0.0f;

    m[1][0] = xy - wz;
    m[1][1] = 1.0f - (xx + zz);
    m[1][2] = yz + wx;
    m[1][3] = 0.0f;

    m[2][0] = xz + wy;
    m[2][1] = yz - wx;
    m[2][2] = 1.0f - (xx + yy);
    m[2][3] = 0.0f;

    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;

    return m;
}

