#pragma once
#include "GCScaleStackEnergy.h"

class GCScaleStackHorizontalEnergy : public GCScaleStackEnergy
{
public:
	GCScaleStackHorizontalEnergy(void);
	~GCScaleStackHorizontalEnergy(void);
public:
	virtual int GetDataCost(int labelId, int nodeId);
// flavor jump which is actually near in 2D, doesn't matter it's from different scale or not
	virtual int GetSmoothCost2(int labelId1, int labelId2, int nodeId1, int nodeId2);
protected:
	// ensure border come from one image of the stack
	int GetDataCost1(int labelId, int nodeId);
	// ensure border come from the largest image only
	int GetDataCost2(int labelId, int nodeId);
	// ensure border come from the largest image only - prefer large imag
	int GetDataCost3(int labelId, int nodeId);
	
	
};
