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
 