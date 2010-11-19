#include "StdAfx.h"
#include "Mapping2D.h"

Mapping2D::Mapping2D(void)
{
}

Mapping2D::~Mapping2D(void)
{
}

void Mapping2D::InitializeMapping(int width, int height)
{
	_width = width;
	_height = height;
	_idCount = width * height;
}
void Mapping2D::IsShift(bool isShift)
{
	_isShift = isShift;
}
CvPoint Mapping2D::GetMappedPoint(int id)
{
	if(id >= _idCount)
		return cvPoint(-1, -1);
	
	CvPoint point;
	int y = id / _width;
	int x = id % _width;
	
	if(_isShift)
	{
		point.x = x - _width / 2;
		point.y = y - _height / 2;	
	}
	else
	{
		point.x = x;
		point.y = y;
	}
	
	return point;
}

int Mapping2D::GetPointId(CvPoint point)
{
	return point.y * _width + point.x;
}