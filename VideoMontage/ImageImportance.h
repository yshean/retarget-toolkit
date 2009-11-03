#pragma once
#include <cv.h>

public class ImageImportance
{
public:
	ImageImportance(void){}
	~ImageImportance(void){}

	// Return the important of the image
	virtual double GetImageImportance(IplImage* image) = 0;
};
