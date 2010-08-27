#pragma once
#include "ShiftMap.h"
#include <cv.h>



struct ForDataFnCoarse
{
	CvSize outputSize;
	CvSize shiftSize;
	CvSize inputSize;
	// size of processing patch
	CvSize patchSize;
	// image input
	IplImage* saliency; 
};

struct ForSmoothFnCoarse
{	
	CvSize outputSize;
	CvSize shiftSize;
	CvSize inputSize;
	// size of processing patch
	CvSize patchSize;
	// no of rows & cols in output
	int nRows;
	int nCols;
	// image input
	IplImage* image;
	IplImage* gradient;
};

// coarse shiftmap
int dataFunctionShiftmapCoarse(int pixel, int label, void *extraData);
int smoothFunctionShiftmapCoarse(int pixel1, int pixel2, int label1, int label2, void* extraData);

CvRect GetPatchLocation(CvPoint location, CvSize patchSize);
// support function for coarse shift map

// square difference between horizontal 2 lines, location is at the left most
int SquareDifferenceH(CvPoint location1, CvPoint location2, int width, IplImage* image);

// square difference between 2 vertical lines, location is at the top
int SquareDifferenceV(CvPoint location1, CvPoint location2, int height, IplImage* image);

// assume that location1 & location2 are neighbor
int GetNeighborRectDifference(CvPoint location1, CvPoint location2, CvSize patchSize, IplImage* image);

class ShiftMapCoarse : public ShiftMap
{
public:
	ShiftMapCoarse(void);
	~ShiftMapCoarse(void);
	// implement base class
	virtual IplImage* GetRetargetImage(IplImage* input, IplImage* saliency, CvSize outputSize);
	virtual void ComputeShiftMap(IplImage* input, IplImage* saliency, CvSize output, CvSize shiftSize);	
	virtual IplImage* CalculateRetargetImage();		
protected:
	CvSize _patchSize;
};
