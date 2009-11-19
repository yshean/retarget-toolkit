#pragma once
#include <Math.h>
#include <cv.h>
#include "Point.h"

// to calculate all kinds of norm
class Norm
{
public:
	Norm(void);
	~Norm(void);

	// return 0 by default so child does not have to implement anything
	virtual double CalculateNorm(Point3D point1, Point3D point2);

	// return 0 by default so child does not have to implement anything
	virtual double CalculateNorm(CvScalar value1, CvScalar value2);	

	// return 0 by default so child does not have to implement anything
	virtual double CalculateNorm(CvMat matrix1, CvMat matrix2);
};