#pragma once
#include "ImageQuality.h"

// Get Image Quality by taking max value in Sobel saliency map
class SobelImageQuality : public ImageQuality
{
public:
	SobelImageQuality(void);
	~SobelImageQuality(void);

	virtual double GetImageQuality(IplImage* image);
};
