#pragma once
/* ------------------------------------------------------

 Axis Aligned Boxes - Lighthouse3D

  -----------------------------------------------------*/


#include "Vec3.h"

class Vec3;

class AABox 
{

public:

	Vec3 corner;
	float x,y,z;


	AABox::AABox( Vec3 &corner, float x, float y, float z);
	AABox::AABox(void);
	AABox::~AABox();

	void AABox::setBox( Vec3 &corner, float x, float y, float z);

	// for use in frustum computations
	Vec3 AABox::getVertexP(Vec3 &normal);
	Vec3 AABox::getVertexN(Vec3 &normal);


};
