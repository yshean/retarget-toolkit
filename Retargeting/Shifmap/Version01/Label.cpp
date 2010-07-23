#include "stdafx.h" 
#include "Label.h"

int GetLabel(CvPoint point, CvSize imageSize)
{
	if(point.x < 0 || point.x>= imageSize.width || point.y < 0 || point.y >= imageSize.height)
		return -1;
	
	return point.y * imageSize.width + point.x;
}

CvPoint GetPoint(int label, CvSize imageSize)
{
	if(label + 1 > imageSize.width * imageSize.height)
		return cvPoint(-1, -1);
	
	CvPoint point;
	int y = label / imageSize.width;
	int x = label % imageSize.width;
	point.x = x;
	point.y = y;
	return point;
}
 
bool IsOutside(CvPoint point, CvSize imageSize)
{
	if(point.x < 0 || point.y < 0 || point.x >= imageSize.width || point.y >= imageSize.height)
		return true;
	return false;
}

CvPoint GetShift(int label, CvSize shiftSize)
{
	CvPoint shift = GetPoint(label, shiftSize);
	shift.x -= shiftSize.width / 2;
	shift.y -= shiftSize.height / 2;
	return shift;
}

CvPoint GetMappedPoint(int pixel, int label, CvSize output, CvSize shiftSize)
{
	CvPoint result;

	CvPoint shift = GetShift(label, shiftSize);
	CvPoint pixelPoint = GetPoint(pixel, output);

	result.x = pixelPoint.x + shift.x;
	result.y = pixelPoint.y + shift.y;
	
	return result;
}

CvPoint GetMappedPointInitialGuess(int pixel, int label, CvSize output, CvSize shiftSize, CvMat* initialGuess)
{
	CvPoint result;

	CvPoint shift = GetShift(label, shiftSize);
	CvPoint pixelPoint = GetPoint(pixel, output);	
	CvPoint guessShift = GetLabel(pixelPoint, initialGuess);
	 
	result.x = pixelPoint.x + guessShift.x + shift.x;
	result.y = pixelPoint.y + guessShift.y + shift.y;

	return result;
}

// set a label to a label map
void SetLabel(CvPoint point, CvPoint shiftLabel, CvMat* labelMap)
{	
	cvSet2D(labelMap, point.y, point.x, cvScalar(shiftLabel.x, shiftLabel.y));  
}

// get a label from a label map
CvPoint GetLabel(CvPoint point, CvMat* labelMap)
{
	CvPoint shift;
	CvScalar value = cvGet2D(labelMap, point.y, point.x);
	shift.x = value.val[0];
	shift.y = value.val[1];
	return shift;
}
