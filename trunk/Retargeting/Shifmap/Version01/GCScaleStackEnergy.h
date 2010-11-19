#pragma once
#include "GCEnergyFunction.h"
#include "MappingCubic.h"
#include "ScaleStackImageSource.h"

class GCScaleStackEnergy : public GCEnergyFunction
{
public:
	GCScaleStackEnergy(void);
	~GCScaleStackEnergy(void);
protected:
	MappingCubic* _mappingCubicData;
	MappingCubic* _mappingCubicShift;
	ScaleStackImageSource* _imageSource;
	ScaleStackImageSource* _imageSaliency;
public:
	void SetMapping(MappingCubic* data, MappingCubic* shift);
	void SetSource(ScaleStackImageSource* source, ScaleStackImageSource* saliency);
	virtual int GetDataCost(int labelId, int nodeId);
	virtual int GetSmoothCost(int labelId1, int labelId2, int nodeId1, int nodeId2);
};
