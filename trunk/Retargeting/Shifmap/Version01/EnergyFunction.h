#pragma once
#include <cv.h>
#include "Label.h"
/********************************************************
Implementation of Shift-map algorithm (Peleg ICCV2009)

This class implement data term & smooth term for graphcut used in retargeting

A width x height image will have width x height number of labels
Label index is assigned from left to right, then top to bottom

e.g: 4x4 image
1 2  3  4
5 6  7  8
9 10 11 12 

********************************************************/
struct ForDataFunction
{
	// output image
	int outWidth;
	int outHeight;
	// shift size
	int shiftWidth;	
	int shiftHeight;
	// input
	int inWidth;
	int inHeight;
	IplImage* saliency; 
};

// for hierarchical shift-map
struct ForDataFunctionH
{
	// output image
	int outWidth;
	int outHeight;
	// shift size
	int shiftWidth;	
	int shiftHeight;
	// input
	int inWidth;
	int inHeight;
	IplImage* saliency; 
	IplImage* initialGuess;
};

struct ForSmoothFunction
{
	int outWidth;
	int outHeight;
	// shift size
	int shiftWidth;	
	int shiftHeight;
	// input
	int inWidth;
	int inHeight;
	IplImage* image;
	IplImage* gradient;
}; 

// for hierarchical shift-map
struct ForSmoothFunctionH
{
	int outWidth;
	int outHeight;
	// shift size
	int shiftWidth;	
	int shiftHeight;
	// input
	int inWidth;
	int inHeight;
	IplImage* image;
	IplImage* gradient;
	// initial guess
	IplImage* initialGuess;
};

// straight shiftmap
int dataFunctionShiftmap(int pixel, int label, void *extraData);
int smoothFunctionShiftmap(int pixel1, int pixel2, int label1, int label2, void* extraData);

// hierarchical shift-map
int dataFunctionShiftmapH(int pixel, int label, void *extraData);
int smoothFunctionShiftmapH(int pixel1, int pixel2, int label1, int label2, void* extraData);

bool IsNeighbor(CvPoint point1, CvPoint point2);

CvScalar ColorDiffenrece(CvPoint point1, CvPoint point2, IplImage* image);

// get Euclide distance of color between 2 point
int SquareColorDifference(CvPoint point1, CvPoint point2, IplImage* image);

int SquareDifference(CvScalar value1, CvScalar value2);

// Get the neighbor of label which have the some neighborhood properties 
// with pixel1 and pixel2
// or label -> result ~ pixel1 -> pixel2
CvPoint GetNeighbor(CvPoint pixel1, CvPoint pixel2, CvPoint label);

bool IsInsideImage(CvPoint point, int width, int height);

//bool IsCorrectMap(CvPoint pixel1, CvPoint pixel2, CvPoint label1, CvPoint label2);


struct ForDataFunctionInitialGuess
{
	// output image
	int outWidth;
	int outHeight;
	// input image
	int inWidth;	
	int inHeight;
	IplImage* saliency;
	IplImage* initialGuess; // initial guess
};

struct ForSmoothFunctionInitialGuess
{
	int outWidth;
	int outHeight;
	// input image
	int inWidth;	
	int inHeight;
	IplImage* image;
	IplImage* gradient;
	IplImage* initialGuess;
};


//int dataFunctionInitialGuess(int pixel, int label, void *extraData);
//int smoothFunctionInitialGuess(int pixel1, int pixel2, int label1, int label2, void* extraData);