// Plane.cpp
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Plane.h"

Plane::Plane( Vec3 &v1,  Vec3 &v2,  Vec3 &v3) {

	set3Points(v1,v2,v3);
}


Plane::Plane() {}

Plane::~Plane() {}


void Plane::set3Points( Vec3 &v1,  Vec3 &v2,  Vec3 &v3) {


	Vec3 aux1, aux2;

	aux1 = v1 - v2;
	aux2 = v3 - v2;

	normal = aux2 * aux1;

	normal.normalize();
	point.copy(v2);
	d = -(normal.innerProduct(point));
}

void Plane::setNormalAndPoint(Vec3 &normal, Vec3 &point) {

	this->normal.copy(normal);
	this->normal.normalize();
	d = -(this->normal.innerProduct(point));
}

void Plane::setCoefficients(float a, float b, float c, float d) {

	// set the normal vector
	normal.set(a,b,c);
	//compute the lenght of the vector
	float l = normal.length();
	// normalize the vector
	normal.set(a/l,b/l,c/l);
	// and divide d by th length as well
	this->d = d/l;
}


	

float Plane::distance(Vec3 &p) {

	return (d + normal.innerProduct(p));
}

void Plane::print() {

	printf("Plane(");normal.print();printf("# %f)",d);
}
