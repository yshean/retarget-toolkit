#pragma once
#include <cv.h>
#include "Common3D.h"

// general class for getting image model data source
class ImageSource
{
public:
	ImageSource(void);
	~ImageSource(void);
public:
	virtual CvScalar GetImageValue(Point3D point) = 0;
	virtual CvScalar GetGradientValue(Point3D point) = 0;	
};
