#pragma once
#include "highgui.h"
#include "Label.h"
#include "GCoptimization.h"
#include "EnergyFunction.h"
#include <cv.h> 
 
 
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
	void ComputeFastShiftMap(IplImage* input, IplImage* saliency, CvSize output);
	// compute shift-map using an initial guess
	void ComputeShiftMap(IplImage* input, IplImage* saliency, IplImage* initialGuess, CvSize output, CvSize shiftSize);
	// delete memory of graphcut object
	void ClearGC();
public:	
	// data to analyze performance of algorithm
	vector<IplImage*>* _imageList;
	vector<IplImage*>* _labelMapList;

protected:

	CvSize _inputSize;
	CvSize _outputSize;
	CvSize _shiftSize; // range of label to search for

	IplImage* _input;
	IplImage* _initialGuess; // for hierarchical shift-map
	int _level;
	GCoptimizationGridGraph* _gc; 
    
public:
	int GetLevelCount();
	IplImage* GetOriginalImage(int level);
	IplImage* CalculateRetargetImage();
	IplImage* GetRetargetImageH();
	// get retarget image in a specifica level
	IplImage* GetRetargetImage(int level);
	// get label map of a specific level
	IplImage* GetLabelMap(int level);
	// without guess
	IplImage* CalculateLabelMap();
	// with initialGuess
	IplImage* CalculateLabelMap2();
protected:
	// get a interpolation of lower map by x2 times value of shift in the lowermap	
	void GetInterpolationMap(IplImage* lowerMap, IplImage* higherMap);
 
	// get retargeted image by graphcut given a interpolation matrix from lower level
	// only 9 labels surrounding the guess is used.
	//void GetRetargetImageInitialGuess(IplImage* input, IplImage* saliency, int width, int height, IplImage* intialGuess);

	// get image from label
	IplImage* GetImageFromLabelMap(IplImage* map, IplImage* image);

	void DownSampling(IplImage* source, IplImage* dst);
};
