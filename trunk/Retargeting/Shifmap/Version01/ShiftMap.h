#pragma once
#include "highgui.h"
#include "Label.h"
#include "GCoptimization.h"
#include "EnergyFunction.h"
#include <cv.h> 
#include "ImageRetargeter.h" 
 
#include <vector>
using namespace std;

 
/********************************************************
Implementation of Shift-map algorithm (Peleg ICCV2009)

 
********************************************************/


class ShiftMap : public ImageRetargeter
{
public:
	ShiftMap();
	~ShiftMap(void);
	 
public:
	// implement base class
	virtual IplImage* GetRetargetImage(IplImage* input, IplImage* saliency, CvSize outputSize);
 
protected:
	virtual void ComputeShiftMap(IplImage* input, IplImage* saliency, CvSize output, CvSize shiftSize);	
	// delete memory of graphcut object
	void ClearGC();
protected:
	CvSize _inputSize;
	CvSize _outputSize;
	CvSize _shiftSize; // range of label to search for

	IplImage* _input;
	GCoptimizationGridGraph* _gc;     
public:
	virtual IplImage* CalculateRetargetImage();		
	CvMat* CalculateLabelMap();
protected: 
	// get image from label
	IplImage* GetImageFromLabelMap(CvMat* map, IplImage* image);

};
