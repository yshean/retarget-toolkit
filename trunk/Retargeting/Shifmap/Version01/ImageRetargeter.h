#pragma once
#include <cv.h>

class ImageRetargeter
{
public:
	virtual IplImage* GetRetargetImage(IplImage* input, IplImage* saliency, CvSize outputSize) = 0;
};
