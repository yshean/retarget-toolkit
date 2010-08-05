#pragma once
#include "Common3D.h"
#include "Label.h"

struct ForDataFunction3D
{
	Volume3D outputSize;
	Volume3D shiftSize;
	Volume3D inputSize;
	IplImage* saliency; 
};
// straight shiftmap
int dataFunctionShiftmap3D(int pixel, int label, void *extraData);