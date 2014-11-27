
#include "stdafx.h"

#define PI		3.1415926535897932384626433832795
#define PIdiv180	(PI/180.0)

#define SQR(x) (x*x)

#define NULL_VECTOR F3dVector(0.0f,0.0f,0.0f)

SF3dVector F3dVector ( GLfloat x, GLfloat y, GLfloat z )
{
	SF3dVector tmp;
	tmp.x = x;
	tmp.y = y;
	tmp.z = z;
	return tmp;
}

GLfloat GetF3dVectorLength( SF3dVector * v)
{
	return (GLfloat)(sqrt(SQR(v->x)+SQR(v->y)+SQR(v->z)));
}

SF3dVector Normalize3dVector( SF3dVector v)
{
	SF3dVector res;
	float l = GetF3dVectorLength(&v);
	if (l == 0.0f) return NULL_VECTOR;
	res.x = v.x / l;
	res.y = v.y / l;
	res.z = v.z / l;
	return res;
}

SF3dVector operator+ (SF3dVector v, SF3dVector u)
{
	SF3dVector res;
	res.x = v.x+u.x;
	res.y = v.y+u.y;
	res.z = v.z+u.z;
	return res;
}
SF3dVector operator- (SF3dVector v, SF3dVector u)
{
	SF3dVector res;
	res.x = v.x-u.x;
	res.y = v.y-u.y;
	res.z = v.z-u.z;
	return res;
}


SF3dVector operator* (SF3dVector v, float r)
{
	SF3dVector res;
	res.x = v.x*r;
	res.y = v.y*r;
	res.z = v.z*r;
	return res;
}

SF3dVector CrossProduct (SF3dVector * u, SF3dVector * v)
{
	SF3dVector resVector;
	resVector.x = u->y*v->z - u->z*v->y;
	resVector.y = u->z*v->x - u->x*v->z;
	resVector.z = u->x*v->y - u->y*v->x;

	return resVector;
}
float operator* (SF3dVector v, SF3dVector u)	//dot product
{
	return v.x*u.x+v.y*u.y+v.z*u.z;
}


/***************************************************************************************/

CCamera::CCamera()
{
	// Init with standard OGL values:
	Position = F3dVector (0.0, 0.0,	0.0);

	ViewDir = F3dVector( 0.0, 0.0, -1.0);
	RightVector = F3dVector (1.0, 0.0, 0.0);
	UpVector = F3dVector (0.0, 1.0, 0.0);

	//Only to be sure:
	RotatedX = RotatedY = RotatedZ = 0.0;
}

void CCamera::Move (SF3dVector Direction)
{
	SF3dVector NewPosition;
	NewPosition = Position + Direction;
	CorrectPosition(NewPosition);
	Position = NewPosition;
}

void CCamera::RotateX (GLfloat Angle)
{
	RotatedX += Angle;
	
	//Rotate viewdir around the right vector:
	ViewDir = Normalize3dVector(ViewDir*cos(Angle*PIdiv180)
					+ UpVector*sin(Angle*PIdiv180));

	// now compute the new UpVector (by cross product)
	// (Disabled by KJS).
	// UpVector = CrossProduct(&ViewDir, &RightVector)*-1;

	
}

void CCamera::RotateY (GLfloat Angle)
{
	RotatedY += Angle;
	
	// Rotate viewdir around the up vector:
	ViewDir = Normalize3dVector(ViewDir*cos(Angle*PIdiv180)
					- RightVector*sin(Angle*PIdiv180));

	// now compute the new RightVector (by cross product)
	RightVector = CrossProduct(&ViewDir, &UpVector);
}

void CCamera::RotateZ (GLfloat Angle)
{
	RotatedZ += Angle;
	
	// Rotate viewdir around the right vector:
	RightVector = Normalize3dVector(RightVector*cos(Angle*PIdiv180)
					+ UpVector*sin(Angle*PIdiv180));

	//now compute the new UpVector (by cross product)
	UpVector = CrossProduct(&ViewDir, &RightVector)*-1;
}

void CCamera::Render( void )
{

	// The point at which the camera looks:
	SF3dVector ViewPoint = Position+ViewDir;

	p = Vec3(Position.x, Position.y, Position.z);
	l = Vec3(ViewPoint.x, ViewPoint.y, ViewPoint.z);
	u = Vec3(UpVector.x, UpVector.y, UpVector.z);

}

void CCamera::MoveForward( GLfloat Distance )
{
	SF3dVector NewPosition;
	NewPosition = Position + (ViewDir*-Distance);
	CorrectPosition(NewPosition);
	Position = NewPosition;
}

void CCamera::StrafeRight ( GLfloat Distance )
{
	SF3dVector NewPosition;

	NewPosition = Position + (RightVector*Distance);
	CorrectPosition(NewPosition);
	Position = NewPosition;
}

void CCamera::MoveUpward( GLfloat Distance )
{
	SF3dVector NewPosition;

	NewPosition = Position + (UpVector*Distance);
	CorrectPosition(NewPosition);
	Position = NewPosition;
}

void CCamera::CorrectPosition(SF3dVector &pos)
{
	int x, y;
	UNIVERSE_GRID ugrid;

	if( pos.y < 1.5 ) {
		pos.y = 1.5;
	}

}

