#pragma once
#include "Volume3D.h"

class SaliencyMap
{
public:
	SaliencyMap(void);

	~SaliencyMap(void);

	// compute the saliency map of a volume
	// this is general interface
	virtual Volume3D* ComputeMap(Volume3D* volume) = 0;
	
};
