#pragma once
#include "Norm.h"

class NormL2 : public Norm
{
public:
	NormL2(void);
	~NormL2(void);

	// Calculate L2 Norm between 2 Point3D
	virtual double CalculateNorm(Point3D point1, Point3D point2);
	
	// Calculate L2 Norm between 2 CvScalar 
	virtual double CalculateNorm(CvScalar value1, CvScalar value2);	
	
};
