#pragma once
#include "Volume3D.h"
#include "Matrix3D.h"

class SaliencyMap
{
public:
	SaliencyMap(void);
	~SaliencyMap(void);

#pragma region Saliency Interface
	// interface to calculate saliency for 3D volume
	// do nothing by default
	virtual void CalculateSaliencyMap(Volume3D* volume, Volume3D* output){}

	// saliency on image or other data type can be added here
	// default implementation should provided so that child 
	// does not have to implement anything
	virtual void CalculateSaliencyMap(Matrix3D* matrix, Matrix3D* output){}
#pragma endregion

};

 