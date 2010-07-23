#pragma once
#include "highgui.h"
#include "Label.h"
#include "GCoptimization.h"
#include "EnergyFunction.h"
#include <cv.h>
 
#include "highgui.h"
 
#include <vector>
using namespace std;

 
/********************************************************
Implementation of Shift-map algorithm (Peleg ICCV2009)

 
********************************************************/


class ShiftMapComputer
{
public:
	ShiftMapComputer();
	~ShiftMapComputer(void);
	 
	void ComputeShiftMap(IplImage* input, IplImage* saliency,   CvSize output, CvSize shiftSize);

	// fast shift by downsampling
	//void ComputeFastShiftMap(IplImage* input, IplImage* saliency, CvSize output);
	// delete memory of graphcut object
	void ClearGC();
protected:
	// range of label to search for
	int _shiftWidth; 
	int _shiftHeight;

	CvSize _inputSize;
	CvSize _outputSize;
	CvSize _shiftSize; // range of label to search for

	IplImage* _input;
 
 
	GCoptimizationGridGraph* _gc;
 
  
public:
	IplImage* GetRetargetImage();
 
	IplImage* GetLabelMap();
protected:
	// get a interpolation of lower map by x2 times value of shift in the lowermap	
	//void GetInterpolationMap(IplImage* lowerMap, IplImage* higherMap);
 
	// get retargeted image by graphcut given a interpolation matrix from lower level
	// only 9 labels surrounding the guess is used.
	//void GetRetargetImageInitialGuess(IplImage* input, IplImage* saliency, int width, int height, IplImage* intialGuess);

	// get image from label
	IplImage* GetImageFromLabelMap(IplImage* map, IplImage* image);
};
