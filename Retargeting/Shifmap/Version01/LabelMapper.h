#pragma once
#include "Common3D.h"
#include "MappingCubic.h"

// provide which point3D is mapped to which point3D
class LabelMapper
{
public:
	LabelMapper(void);
	~LabelMapper(void);
protected:
	MappingCubic* _mappingCubicData;
	MappingCubic* _mappingCubicShift;
public:
	void SetMappingCubic(MappingCubic* data, MappingCubic* shift);
	virtual Point3D GetMappedPoint(int labelId, int pixelId);
	virtual Point3D GetPoint(int pixelId);
};
