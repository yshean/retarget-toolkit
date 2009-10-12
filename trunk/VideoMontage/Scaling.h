#pragma once
#include "Matrix3D.h"

// this class is for scaling
class Scaling
{
public:
	Scaling(void);
	~Scaling(void);

public:
	// scale down with same number of frames
	// output should be 2 times smaller than input
	virtual void PyrDown(Matrix3D* input, Matrix3D* output);
};
