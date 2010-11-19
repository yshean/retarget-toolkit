#pragma once

// generalized class for graph cut cost
class GCEnergyFunction
{
public:
	GCEnergyFunction(void);
	~GCEnergyFunction(void);

	virtual int GetDataCost(int labelId, int nodeId) = 0;
	virtual int GetSmoothCost(int labelId1, int label2, int nodeI1, int nodeId2) = 0;
};
