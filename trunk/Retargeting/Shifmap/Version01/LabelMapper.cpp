#include "StdAfx.h"
#include "LabelMapper.h"

LabelMapper::LabelMapper(void)
{
}

LabelMapper::~LabelMapper(void)
{
}

void LabelMapper::SetMappingCubic(MappingCubic* data, MappingCubic* shift)
{
	_mappingCubicData = data;
	_mappingCubicShift = shift;
}
Point3D LabelMapper::GetPoint(int pixelId)
{
	return _mappingCubicData->GetMappedPoint(pixelId);
}

Point3D LabelMapper::GetMappedPoint(int labelId, int pixelId)
{
	 Point3D pixel = _mappingCubicData->GetMappedPoint(pixelId);
	 Point3D shift = _mappingCubicShift->GetMappedPoint(labelId);
	 return point3D(pixel.x + shift.x, pixel.y + shift.y, pixel.z + shift.z);
}
