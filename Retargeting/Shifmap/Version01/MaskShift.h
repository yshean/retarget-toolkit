#pragma once
#include <cv.h>
#include "Label.h"
#include <highgui.h>

 

// a mask is a region which is computed by shift to an original source
// _mask: mask in output size
// _imageData: data we are refering too
class MaskShift
{
public:
	MaskShift(IplImage* mask, IplImage* imageData);
	~MaskShift(void);
	void SetMaskData(IplImage* image);
	int GetWidth();
	int GetHeight();
	void AddShift(CvPoint shift);
	void ChangeMask(IplImage* mask);
	bool IsMaskedPixel(CvPoint pixel);
	bool IsMaskedPixel(int x, int y);
	CvScalar GetPixelValue(CvPoint pixel);
	CvScalar GetPixelValue(int x, int y);
	CvScalar GetPixelValue(CvPoint pixel, IplImage* image);
	CvScalar GetPixelValue(int x, int y, IplImage* image);
	IplImage* _maskData; //temp
	IplImage* _mask;
protected:
	
	CvPoint _shift;
	
	IplImage* _imageData;
};

// same as create mask shift but create mask & mask_data only
void CreateMask(IplImage* input, IplImage* mask, CvPoint shift, CvSize outputSize, IplImage* maskOutput, IplImage* maskData);

// mask is of the same size as input
MaskShift* CreateMaskShift(IplImage* input, IplImage* mask, CvPoint shift, CvSize outputSize);
bool IsMaskedPixel(int x, int y, IplImage* image);