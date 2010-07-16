#pragma once
#include "Label.h"
#include "GCoptimization.h"
#include "EnergyFunction.h"
#include <cv.h>
 
/********************************************************
Implementation of Shift-map algorithm (Peleg ICCV2009)

A width x height image will have width x height number of labels
Label index is assigned from left to right, then top to bottom

e.g: 4x4 image
1 2  3  4
5 6  7  8
9 10 11 12 
********************************************************/





class ShiftMapComputer
{
public:
	ShiftMapComputer(void);
	~ShiftMapComputer(void);

	IplImage* GetImage(IplImage* input,IplImage* saliency, int width, int height);

private:
	void GetRetargetImage(GCoptimizationGridGraph * gc, IplImage* input, IplImage* output);
	
};
