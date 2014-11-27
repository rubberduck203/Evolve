#pragma once
//***************************************************************************
//
// Advanced CodeColony Camera
// Philipp Crocoll, 2003
//
// HACKED BY KENNY
//
// Note: All angles in degrees
//
//
//***************************************************************************

#include "stdafx.h"

struct SF3dVector
{
	GLfloat x,y,z;
};

struct SF2dVector
{
	GLfloat x,y;
};

SF3dVector F3dVector ( GLfloat x, GLfloat y, GLfloat z );

class CCamera
{
private:
	SF3dVector ViewDir;
	SF3dVector RightVector;	
	SF3dVector UpVector;
	SF3dVector Position;

	GLfloat RotatedX, RotatedY, RotatedZ;	
	
public:
	// inits the values (Position: (0|0|0) Target: (0|0|-1) )
	CCamera();

	// executes some glRotates and a glTranslate command
	// Note: You should call glLoadIdentity before using Render
	void Render ( void );

	void Move ( SF3dVector Direction );
	void RotateX ( GLfloat Angle );
	void RotateY ( GLfloat Angle );
	void RotateZ ( GLfloat Angle );

	void MoveForward ( GLfloat Distance );
	void MoveUpward ( GLfloat Distance );
	void StrafeRight ( GLfloat Distance );

	SF3dVector GetPosition() { return Position; }

	void CorrectPosition(SF3dVector &pos);

	// these vectors are set after calling Render()
	Vec3 p;
	Vec3 l;
	Vec3 u;

};


