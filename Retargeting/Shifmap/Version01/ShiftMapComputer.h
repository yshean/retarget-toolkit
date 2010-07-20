#pragma once
#include "Label.h"
#include "GCoptimization.h"
#include "EnergyFunction.h"
#include <cv.h>
 
/********************************************************
Implementation of Shift-map algorithm (Peleg ICCV2009)

 
********************************************************/


class ShiftMapComputer
{
public:
	ShiftMapComputer();
	~ShiftMapComputer(void);
	 
	void ComputeShiftMap(IplImage* input, IplImage* saliency, CvSize output, CvSize shiftSize);

protected:
	// range of label to search for
	int _shiftWidth; 
	int _shiftHeight;

	CvSize _inputSize;
	CvSize _outputSize;
	CvSize _shiftSize; // range of label to search for

	IplImage* _input;

	GCoptimization* _gc;
public:
	IplImage* GetRetargetImage();

	// get retargeted image by graphcut given a interpolation matrix from lower level
	// only 9 labels surrounding the guess is used.
	//void GetRetargetImageInitialGuess(IplImage* input, IplImage* saliency, int width, int height, IplImage* intialGuess);

};
