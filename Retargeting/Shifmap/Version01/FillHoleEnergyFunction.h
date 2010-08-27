#pragma once
#include <cv.h>
#include <vector>
using namespace std;

#include "Label.h"
#include "EnergyFunction.h"
#include "MaskShift.h"

struct ForDataFH2
{
	MaskShift* mask;
	IplImage* saliency;	
	IplImage* input;
	IplImage* inputGradient;
	IplImage* maskNeighbor;
	vector<CvPoint*>* pointMapping;
	CvSize inputSize;
	CvSize shiftSize;
};

struct ForDataFH
{
	IplImage* saliency;
	IplImage* maskData;
	IplImage* maskDataGradient;
	IplImage* input;
	IplImage* inputGradient;
	IplImage* maskNeighbor;
	vector<CvPoint*>* pointMapping;
	CvSize inputSize;
	CvSize shiftSize;
};
 
struct ForSmoothFH
{
	IplImage* input;
	IplImage* inputGradient;
	vector<CvPoint*>* pointMapping;
	CvSize inputSize;
	CvSize shiftSize;
};

 
// customized
int dataFunctionFH(int pixel, int label, void *extraData);
int dataFunctionFH2(int pixel, int label, void *extraData);
// same as in shiftmap
int smoothFunctionFH(int pixel1, int pixel2, int label1, int label2, void* extraData);

// maskOutput is given with some pixels already have values
// outputPoint: chosen point (or node)
// inputPoint: 'label' of the chosen point (which is in the input)
// knownPoint: is in the output, which is already given
int smoothFunctionFHMask(CvPoint inputPoint, CvPoint outputPoint, CvPoint knownPoint, IplImage* input, IplImage* inputGradient,
				 IplImage* maskOutput, IplImage* maskOutputGradient);
int smoothFunctionFHMask2(CvPoint inputPoint, CvPoint outputPoint, CvPoint knownPoint, ForDataFH2* forData);
int SquareColorDifference(CvPoint point1, IplImage* image1, CvPoint poin2, IplImage* image2);

void SetMaskNeighbor(CvPoint current, CvPoint neighbor, CvScalar* value);
vector<CvPoint*>* GetMaskNeighbor(CvScalar value, CvPoint current);

int SquareColorDifference(CvPoint point, IplImage* image, CvPoint pointMask, MaskShift* mask, IplImage* maskSource);