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

#include "stdafx.h"

const float ThirdPersonCamera::DEFAULT_FOVX = 80.0f;
const float ThirdPersonCamera::DEFAULT_ZFAR = 1000.0f;
const float ThirdPersonCamera::DEFAULT_ZNEAR = 1.0f;

const Vector3 ThirdPersonCamera::WORLD_XAXIS(1.0f, 0.0f, 0.0f);
const Vector3 ThirdPersonCamera::WORLD_YAXIS(0.0f, 1.0f, 0.0f);
const Vector3 ThirdPersonCamera::WORLD_ZAXIS(0.0f, 0.0f, 1.0f);

ThirdPersonCamera::ThirdPersonCamera()
{
    fovx = DEFAULT_FOVX;
    znear = DEFAULT_ZNEAR;
    zfar = DEFAULT_ZFAR;

    eye.set(0.0f, 0.0f, 0.0f);
    at.set(0.0f, 0.0f, 0.0f);
    offset.set(0.0f, 0.0f, 0.0f);

    xAxis.set(1.0f, 0.0f, 0.0f);
    yAxis.set(0.0f, 1.0f, 0.0f);
    zAxis.set(0.0f, 0.0f, 1.0f);

    viewMatrix.identity();
    projMatrix.identity();
    orientation.identity();

    m_longitudeDegrees = 0.0f;
    m_latitudeDegrees = 0.0f;

    p = Vec3(eye.x, eye.y, eye.z);
    l = Vec3(at.x, at.y, at.z);
    u = Vec3(0.0, 1.0, 0.0);
}

ThirdPersonCamera::~ThirdPersonCamera()
{
}

void ThirdPersonCamera::lookAt(const Vector3 &eye, const Vector3 &at, const Vector3 &up)
{
    this->eye = eye;
    this->at = at;

    // The offset vector is the vector from the target 'at' position to the
    // 'eye' position. This happens to also be the local z axis of the camera.
    // Notice that the offset vector is always relative to the 'at' position.

    offset = zAxis = eye - at;
    zAxis.normalize();

    xAxis = Vector3::cross(up, zAxis);
    xAxis.normalize();

    yAxis = Vector3::cross(zAxis, xAxis);
    yAxis.normalize();
    xAxis.normalize();

    viewMatrix[0][0] = xAxis.x;
    viewMatrix[1][0] = xAxis.y;
    viewMatrix[2][0] = xAxis.z;
    viewMatrix[3][0] = -Vector3::dot(xAxis, eye);

    viewMatrix[0][1] = yAxis.x;
    viewMatrix[1][1] = yAxis.y;
    viewMatrix[2][1] = yAxis.z;
    viewMatrix[3][1] = -Vector3::dot(yAxis, eye);

    viewMatrix[0][2] = zAxis.x;
    viewMatrix[1][2] = zAxis.y;
    viewMatrix[2][2] = zAxis.z;    
    viewMatrix[3][2] = -Vector3::dot(zAxis, eye);

    orientation.fromMatrix(viewMatrix);

    p = Vec3(eye.x, eye.y, eye.z);
    l = Vec3(at.x, at.y, at.z);
    u = Vec3(up.x, up.y, up.z);
}

void ThirdPersonCamera::perspective(float fovx, float aspect, float znear, float zfar)
{
    // We construct a projection matrix based on the horizontal field of view
    // 'fovx' rather than the more traditional 'fovy' used in gluPerspective().

    float e = 1.0f / tanf(Math::degreesToRadians(fovx) / 2.0f);
    float aspectInv = 1.0f / aspect;
    float fovy = 2.0f * atanf(aspectInv / e);
    float xScale = 1.0f / tanf(0.5f * fovy);
    float yScale = xScale / aspectInv;

    projMatrix[0][0] = xScale;
    projMatrix[0][1] = 0.0f;
    projMatrix[0][2] = 0.0f;
    projMatrix[0][3] = 0.0f;

    projMatrix[1][0] = 0.0f;
    projMatrix[1][1] = yScale;
    projMatrix[1][2] = 0.0f;
    projMatrix[1][3] = 0.0f;

    projMatrix[2][0] = 0.0f;
    projMatrix[2][1] = 0.0f;
    projMatrix[2][2] = (zfar + znear) / (znear - zfar);
    projMatrix[2][3] = -1.0f;

    projMatrix[3][0] = 0.0f;
    projMatrix[3][1] = 0.0f;
    projMatrix[3][2] = (2.0f * zfar * znear) / (znear - zfar);
    projMatrix[3][3] = 0.0f;

    this->fovx = fovx;
    this->znear = znear;
    this->zfar = zfar;
}

void ThirdPersonCamera::rotate(float longitudeDegrees, float latitudeDegrees)
{
    // Both 'longitudeDegrees' and 'latitudeDegrees' represents the maximum
    // number of degrees of rotation per second.

    m_latitudeDegrees = latitudeDegrees;
    m_longitudeDegrees = longitudeDegrees;
}


void ThirdPersonCamera::climb(float amount)
{
	//
	// move eye up or down by 'amount'
	//
	Vector3 newCameraPosition = eye;
	float y;

	y = newCameraPosition.y + amount;

	if( y < 2.0 )
		y = 2.0;
	else if( y > 20.0 )
		y = 20.0;

	newCameraPosition.y = y;

	lookAt(newCameraPosition, at, WORLD_YAXIS);
}

void ThirdPersonCamera::setTargetPosition(const Vector3 &at)
{
    this->at = at;
}

void ThirdPersonCamera::update(float elapsedTimeSec)
{
    // This method must be called once per frame to rebuild the view matrix.
    // The most important part of this update() method is the camera's offset
    // vector. Everything depends on it. The offset vector describes the
    // camera's position relative to the camera's current look at position.
    // The offset vector is always relative to the current look at position.
    // Adding the offset vector to the current look at position will give us
    // the correct camera eye position. So applying rotations to the camera
    // really means rotating the offset vector. What we are basically doing
    // is orbiting the eye position about the look at position.

    // Determine how many degrees of rotation to apply based on current time.
    
    float latitudeElapsed = m_latitudeDegrees * elapsedTimeSec;
    float longitudeElapsed = m_longitudeDegrees * elapsedTimeSec;

    // Rotate the offset vector based on the current camera rotation.
    // We use the quaternion triple product here to rotate the offset vector.

    Quaternion rotation(longitudeElapsed, latitudeElapsed, 0.0f);
    Quaternion offsetVector(0.0f, offset.x, offset.y, offset.z);
    Quaternion result = rotation.conjugate() * offsetVector * rotation;

    // Once the offset vector has been rotated into its new orientation we
    // use the transformed offset vector to calculate the new camera 'eye'
    // position based on the camera's current target 'at' position. We do this
    // to ensure that the camera is always at the required distance from the
    // target 'at' position.

    Vector3 transformedOffsetVector(result.x, result.y, result.z);
    Vector3 newCameraPosition = transformedOffsetVector + at;

    // Rebuild the view matrix.

    lookAt(newCameraPosition, at, WORLD_YAXIS);
}

double ThirdPersonCamera::heading()
{
	double dx, dy, angle;

	dx = l.x - p.x;
	dy = p.z - l.z;

	if( dy != 0.0 ) {
		angle = atan( dx/dy );
		angle = angle * 57.2957795;

		if( dx >= 0.0 && dy >= 0.0 ) {
			// I
			angle = 0 + angle;
		} else if( dx >= 0.0 && dy < 0.0 ) {
			// II
			angle = 180 + angle;
		} else if( dx < 0.0 && dy < 0.0 ) {
			// III
			angle = 180 + angle;
		} else if( dx < 0.0 && dy >= 0.0 ) {
			// IV
			angle = 360 + angle;
		} else {
			ASSERT(0);
		}

		return angle;

	} else {
		if( dx >= 0.0 ) {
			return 90;
		} else {
			return 270;
		}
	}
	
}
