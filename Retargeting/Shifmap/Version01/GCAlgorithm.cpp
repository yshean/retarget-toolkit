#include "StdAfx.h"
#include "GCAlgorithm.h"

int dataFunctionGCA(int pixel, int label, void *extraData)
{
	// GCEnergyFunction* energyFunction = (GCScaleStackEnergy*) extraData;
	GCEnergyFunction* energyFunction = (GCEnergyFunction*) extraData;
	int energy = energyFunction->GetDataCost(label, pixel);
	return energy;
}

int smoothFunctionGCA(int pixel1, int pixel2, int label1, int label2, void* extraData)
{
	GCEnergyFunction* energyFunction = (GCScaleStackEnergy*) extraData;
	return energyFunction->GetSmoothCost(label1, label2, pixel1, pixel2);
}

GCAlgorithm::GCAlgorithm(void)
{
}

GCAlgorithm::GCAlgorithm(GCEnergyFunction* energyFunction)
{
	_gcEnergyFunction = energyFunction;
}

GCAlgorithm::~GCAlgorithm(void)
{
}
int GCAlgorithm::GetSitesCount()
{
	return _gc->numSites();
}

void GCAlgorithm::SetEnergyFunction(GCEnergyFunction* energyFunction)
{
	_gcEnergyFunction = energyFunction;
}

void GCAlgorithm::ComputeGC()
{
	try{
		
		_gc->setDataCost(&dataFunctionGCA, _gcEnergyFunction);
		_gc->setSmoothCost(&smoothFunctionGCA, _gcEnergyFunction);

		printf("\nBefore optimization energy is %d \n", _gc->compute_energy());
		//gc->swap(20);
		_gc->expansion(1);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("\nAfter optimization energy is %d \n", _gc->compute_energy()); 		
	}
	catch (GCException e){
		e.Report();
	}
}

GCoptimization* GCAlgorithm::GetGCoptimization()
{
	return _gc;
}

int GCAlgorithm::GetSmoothEnergy()
{
	return _gc->giveSmoothEnergy();
}
int GCAlgorithm::GetDataEnergy()
{
	return _gc->giveDataEnergy();
	
}