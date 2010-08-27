#pragma once

 #pragma once
#include <cv.h>
#include <vector>
using namespace std;

#include "Label.h"
#include "EnergyFunction.h"
#include "FillHoleEnergyFunction.h"

struct ForDataFHHierarchy
{
	IplImage* saliency;
	IplImage* maskData;
	IplImage* maskDataGradient;
	IplImage* input;
	IplImage* inputGradient;
	IplImage* maskNeighbor;
	CvMat* guess;
	vector<CvPoint*>* pointMapping;
	CvSize inputSize;
	CvSize shiftSize;
};

struct ForSmoothFHHierarchy
{
	IplImage* input;
	IplImage* inputGradient;
	CvMat* guess;
	vector<CvPoint*>* pointMapping;
	CvSize inputSize;
	CvSize shiftSize;
};

 
// customized
int dataFunctionFHHierarchy(int pixel, int label, void *extraData);
// same as in shiftmap
int smoothFunctionFHHierarchy(int pixel1, int pixel2, int label1, int label2, void* extraData);

// maskOutput is given with some pixels already have values
// outputPoint: chosen point (or node)
// inputPoint: 'label' of the chosen point (which is in the input)
// knownPoint: is in the output, which is already given
int smoothFunctionFHMask(CvPoint inputPoint, CvPoint outputPoint, CvPoint knownPoint, IplImage* input, IplImage* inputGradient,
				 IplImage* maskOutput, IplImage* maskOutputGradient);

 