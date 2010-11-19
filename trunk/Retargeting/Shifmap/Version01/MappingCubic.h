#pragma once
#include <cv.h>
#include "Mapping2D.h"
#include "Common3D.h"
 
 

class MappingCubic
{
public:
	MappingCubic(void);
	~MappingCubic(void);
protected:
	 
	Mapping2D* _mapping2D;
	int _width;
	int _height;
	int _pixelLevelCount; // no of pixels in 1 level
	int _stack_height;
public:
	int GetWidth();
	int GetHeight();
public:
	void InitializeMapping(int width, int height, int stack_height);
	void IsShift(bool isShift);
	Point3D GetMappedPoint(int id);
	int PointCount(); // number of points inside the cube
};
