#pragma once
#include "cv.h"
#include "Label.h"
#include "highgui.h"
// this class is to visualize the shiftmap result
class ShiftMapVisualizer
{
public:
	ShiftMapVisualizer(void);
	~ShiftMapVisualizer(void);

	void Visualize(CvMat* labelMap, IplImage* source, IplImage* map, CvSize shiftSize);
	int ComputeEnergy(CvMat* labelMap);
};
