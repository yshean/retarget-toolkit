#pragma once
#include "GCEnergyFunction.h"
#include "GCOptimization.h"
#include "GCScaleStackEnergy.h"
int dataFunctionGCA(int pixel, int label, void *extraData);
int smoothFunctionGCA(int pixel1, int pixel2, int label1, int label2, void* extraData);


// generalized class for graph cut
class GCAlgorithm
{
public:
	GCAlgorithm(void);
	GCAlgorithm(GCEnergyFunction* energyFunction);
	~GCAlgorithm(void);
protected:
	GCEnergyFunction* _gcEnergyFunction;
	GCoptimization* _gc;
public:
	int GetSitesCount();
	void SetupGC(int numSites, int numLabel);
	void SetEnergyFunction(GCEnergyFunction* energyFunction);
	void ComputeGC();
	GCoptimization* GetGCoptimization();
	int GetSmoothEnergy();
	int GetDataEnergy();
};
