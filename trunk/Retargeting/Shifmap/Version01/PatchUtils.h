#pragma once
#include <cv.h>
#include "EnergyFunction.h" 
#include "HorizontalScaleLabelMapping.h"


struct DoublePoint
{
	double x;
	double y;
};

DoublePoint doublePoint(double x, double y);

// outside edge is consider 0
int GetPatchDifference(CvPoint point1, CvPoint point2, int labelId1, int labelId2, IplImage* image, HorizontalScaleLabelMapping* labelMapping);

// return 0 if outside
CvScalar GetValue(double x, int y, IplImage* image);

CvScalar GetInterpolatedValue(DoublePoint point, IplImage* image);
CvScalar GetInterpolatedValue(double x1, int y, IplImage* image);

int GetColorDifference(DoublePoint point1, DoublePoint point2, IplImage* image);
int GetColorDifference(double x1, int y1, double x2, int y2, IplImage* image);

bool IsInside(DoublePoint point, CvSize imageSize);