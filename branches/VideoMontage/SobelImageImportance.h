#pragma once
#include "ImageImportance.h"

// Get ImageImportance by calculate the sum of saliency map created using Sobel
// operator.
class SobelImageImportance : public ImageImportance
{
public:
	SobelImageImportance(void);
	~SobelImageImportance(void);

	virtual double GetImageImportance(IplImage* image);
};
