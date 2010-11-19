#pragma once
#include <cv.h>

class Mapping2D
{
public:
	Mapping2D(void);
	~Mapping2D(void);
	void InitializeMapping(int width, int height);
	void IsShift(bool isShift);
	CvPoint GetMappedPoint(int id);
	int GetPointId(CvPoint point);
protected:
	bool _isShift;
	int _width;
	int _height;
	int _idCount;
};
