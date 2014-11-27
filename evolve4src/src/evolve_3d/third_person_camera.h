#pragma once
//-----------------------------------------------------------------------------
// Copyright (c) 2006 dhpoware. All Rights Reserved.
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


//-----------------------------------------------------------------------------
// A quaternion based third person camera class.
//
// Call the lookAt() method to establish the distance and position of the
// camera relative to the target look at position. The lookAt() method stores
// this relationship to the target look at position in the offset vector. The
// look at position can be changed using the setTargetPosition() method.
//
// Aside from the lookAt() method there is no way to move the camera's eye
// position. The camera's eye position is automatically calculated and updated
// in the update() method using the offset vector.
//
// Rotating the camera using the rotate() method allows the camera to be
// orbited around the the look at position. 
//-----------------------------------------------------------------------------

class ThirdPersonCamera
{
public:
    static const float DEFAULT_FOVX;
    static const float DEFAULT_ZFAR;
    static const float DEFAULT_ZNEAR;

    static const Vector3 WORLD_XAXIS;
    static const Vector3 WORLD_YAXIS;
    static const Vector3 WORLD_ZAXIS;

    float fovx, znear, zfar;
    Vector3 eye, at, offset;
    Vector3 xAxis, yAxis, zAxis;
    Matrix4 viewMatrix, projMatrix;
    Quaternion orientation;

    ThirdPersonCamera();
    ~ThirdPersonCamera();

    void lookAt(const Vector3 &eye, const Vector3 &at, const Vector3 &up);
    void perspective(float fovx, float aspect, float znear, float zfar);
    void rotate(float longitudeDegrees, float latitudeDegrees);
    void climb(float amount);
    void setTargetPosition(const Vector3 &at);
    void update(float elapsedTimeSec);
    double heading();

    // these vectors are set after calling lookAt
    Vec3 p;
    Vec3 l;
    Vec3 u;

private:
    float m_longitudeDegrees;
    float m_latitudeDegrees;

};

