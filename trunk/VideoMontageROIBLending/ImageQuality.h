#pragma once
#include <cv.h>

public class ImageQuality
{
public:
	ImageQuality(void){}
	~ImageQuality(void){}

	// Return the quality of the image
	virtual double GetImageQuality(IplImage* image) = 0;
};