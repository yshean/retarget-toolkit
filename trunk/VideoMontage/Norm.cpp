#include "StdAfx.h"
#include "Norm.h"

// calculate the norm using pow and sqrt
double vrNormL2(Point3D point1, Point3D point2)
{
	double sum = pow(point1.x - point2.x, 2.0) + 
		pow(point1.y - point2.y, 2.0) + 
		pow(point1.z - point2.z, 2.0);
	return sqrt(sum);
}