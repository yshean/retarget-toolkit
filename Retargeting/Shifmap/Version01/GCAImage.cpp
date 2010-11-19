#include "StdAfx.h"
#include "GCAImage.h"

GCAImage::GCAImage(GCEnergyFunction* energyFunction)
{
	_gcEnergyFunction = energyFunction;
}

GCAImage::~GCAImage(void)
{
}

void GCAImage::Initialize(int width, int height, MappingCubic* shift)
{
	int num_labels = shift->PointCount();
	 
	_gc = new GCoptimizationGridGraph(width, height, num_labels);	
}