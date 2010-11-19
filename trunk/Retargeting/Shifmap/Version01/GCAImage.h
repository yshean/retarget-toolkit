#pragma once
#include "GCAlgorithm.h"
#include "MappingCubic.h"

// GC used to retarget an image
class GCAImage : public GCAlgorithm
{
public:
	GCAImage(GCEnergyFunction* energyFunction);
	~GCAImage(void);
protected:
	MappingCubic* _shift;
	
public:
	void Initialize(int width, int height, MappingCubic* shift);
};
