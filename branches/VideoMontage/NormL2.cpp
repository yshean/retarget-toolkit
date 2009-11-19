#include "StdAfx.h"
#include "NormL2.h"

NormL2::NormL2(void)
{
}

NormL2::~NormL2(void)
{
}

double NormL2::CalculateNorm(Point3D point1, Point3D point2)
{
	double sum = pow(point1.x - point2.x, 2.0) + 
	pow(point1.y - point2.y, 2.0) + 
	pow(point1.z - point2.z, 2.0);
	return sqrt(sum);
}


double NormL2::CalculateNorm(CvScalar value1, CvScalar value2)
{
	double sum = 0;
	for(int i = 0; i < 4; i++)
	{
		sum += pow(value1.val[i] - value2.val[i], 2.0);
	}
	return sqrt(sum);
	//return sum;
}
