#include "stdafx.h" 
#include "Label.h"

int GetLabel(CvPoint point, int width, int height)
{
	if(point.x < 0 || point.x>= width || point.y < 0 || point.y >= height)
		return -1;
	
	return point.y * width + point.x;
}

CvPoint GetPoint(int label, int width, int height)
{
	if(label + 1 > width * height)
		return cvPoint(-1, -1);
	
	CvPoint point;
	int y = label / width;
	int x = label % width;
	point.x = x;
	point.y = y;
	return point;
}
 
bool IsOutside(CvPoint point, int width, int height)
{
	if(point.x < 0 || point.y < 0 || point.x >= width || point.y >= height)
		return true;
	return false;
}

CvPoint GetMappedPoint(CvPoint pixel, int label, int width, int height)
{
	CvPoint shift = GetPoint(label, width, height);
	// shift
	shift.x -= width / 2;
	shift.y -= height / 2;
	CvPoint result;
	result.x = pixel.x + shift.x;
	result.y = pixel.y + shift.y;
	
	return result;
}

CvPoint GetMappedPointInitialGuess(CvPoint pixel, int label, int width, int height, IplImage* initialGuess)
{
	CvPoint shift = GetPoint(label, width, height);	

	// shift
	shift.x -= width / 2;
	shift.y -= height / 2;

	CvPoint guessPoint = GetLabel(pixel, initialGuess);

	CvPoint result;
	result.x = guessPoint.x + shift.x;
	result.y = guessPoint.y + shift.y;

	if(IsOutside(result, width, height))
		return cvPoint(-1,-1);
	return result;
}

// set a label to a label map
CvPoint SetLabel(CvPoint point, CvPoint shiftLabel, IplImage* labelMap)
{
	cvSet2D(labelMap, point.y, point.x, cvScalar(shiftLabel.x, shiftLabel.y));
}

// get a label from a label map
CvPoint GetLabel(CvPoint point, IplImage* labelMap)
{
	CvPoint shift;
	CvScalar value = cvGet2D(labelMap, point.y, point.x);
	shift.x = value.val[0];
	shift.y = value.val[1];
	return shift;
}
