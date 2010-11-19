#include "StdAfx.h"
#include "PatchUtils.h"
#include "HorizontalScaleLabelMapping.h"

DoublePoint doublePoint(double x, double y)
{
	DoublePoint point;
	point.x = x;
	point.y = y;
	return point;
}

CvScalar GetValue(double x, int y, IplImage* image)
{
	if(x >= 0 && x <= image->width - 1 && y >= 0 && y < image->height)
	{
		return GetInterpolatedValue(x, y, image);
	}
	else
		return cvScalar(0,0,0);	
}

int GetPatchDifference(CvPoint point1, CvPoint point2, int labelId1, int labelId2, IplImage* image, HorizontalScaleLabelMapping* labelMapping)
{
	int diff = 0;
	for(int i = -1; i < 2; i++)
		for(int j = -1; j < 2; j++)
		{
			double x1 = labelMapping->GetMappedPoint(labelId1, point1.x + i);
			double x2 = labelMapping->GetMappedPoint(labelId2, point2.x + i);
			
			CvScalar value1 = GetValue(x1, point1.y, image);
			CvScalar value2 = GetValue(x2, point2.y, image);

			diff += SquareDifference(value1, value2);
			//diff += GetColorDifference(x1, point1.y + j, x2, point2.y + j, image);
		}
	return diff / 9;
}
 

CvScalar GetInterpolatedValue(DoublePoint point, IplImage* image)
{ 
	int yInt = (int)floor(point.y);
	double weight = point.y - yInt;

	CvScalar valueX1 = GetInterpolatedValue(point.x, yInt, image);
	CvScalar valueX2;
	if(weight != 0)
		valueX2 = GetInterpolatedValue(point.x, yInt + 1, image);
	else
		valueX2 = valueX1;
	
	CvScalar value;
	for(int i = 0; i < 4; i++)
		{
			value.val[i] = valueX1.val[i] * (1-weight) + valueX2.val[i] * weight;
		}
	
	return value;
}
CvScalar GetInterpolatedValue(double x1, int y, IplImage* image)
{	

	if(x1 < 0 || x1 > image->width - 1 || y < 0 || y > image->height - 1)
		return cvScalar(0,0,0);

	int x1Int = (int)floor(x1);
	double weight = x1 - x1Int;

	CvScalar value1;
	CvScalar value2;

	if(x1Int >= image->width)
		return cvScalar(255);

	if(weight != 0)
	{
		value1 = cvGet2D(image, y, x1Int);
	    value2 = cvGet2D(image, y, x1Int + 1);
	}
	else
	{
		value1 = cvGet2D(image, y, x1Int);
		value2 = value1;
	}
	CvScalar value;
 
	for(int i = 0; i < 4; i++)
	{
		value.val[i] = value1.val[i] * (1-weight) + value2.val[i] * weight;
	}
	
	return value;
}

int GetColorDifference(DoublePoint point1, DoublePoint point2, IplImage* image)
{
	CvScalar value1 = GetInterpolatedValue(point1, image);
	CvScalar value2 = GetInterpolatedValue(point2, image);
	return SquareDifference(value1, value2);
}

int GetColorDifference(double x1, int y1, double x2, int y2, IplImage* image)
{
	CvScalar value1 = GetInterpolatedValue(x1, y1, image);
	CvScalar value2 = GetInterpolatedValue(x2, y2, image);
	return SquareDifference(value1, value2);
}
bool IsInside(DoublePoint point, CvSize imageSize)
{
	if (point.x < 0 || point.x >imageSize.width - 1 || point.y < 0 || point.y > imageSize.height - 1)
		return false;
	return true;
}