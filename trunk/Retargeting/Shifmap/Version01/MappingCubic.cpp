#include "StdAfx.h"
#include "MappingCubic.h"


MappingCubic::MappingCubic(void)
{
	_mapping2D = new Mapping2D();
}

MappingCubic::~MappingCubic(void)
{
}


void MappingCubic::InitializeMapping(int width, int height, int stack_height)
{
	_width = width;
	_height = height;
	_stack_height = stack_height;
	_pixelLevelCount = _width * _height;
	_mapping2D->InitializeMapping(width, height);
}

void MappingCubic::IsShift(bool isShift)
{	
	_mapping2D->IsShift(isShift);
}

Point3D MappingCubic::GetMappedPoint(int id)
{ 
		int level = id / _pixelLevelCount;
		int level_id = id % _pixelLevelCount;
		CvPoint point = _mapping2D->GetMappedPoint(level_id);
		return point3D(point.x, point.y, level); 
}

int MappingCubic::PointCount()
{
	return _width * _height * _stack_height;
}

int MappingCubic::GetWidth()
{
	return _width;
}
int MappingCubic::GetHeight()
{
	return _height;
}