#include "StdAfx.h"
#include "DrawingFunctions.h"

void Zooming::DrawPath(IplImage* image, Path* path)
{
	CvScalar pixel = cvScalar(0, 0, 255);	
	for(int i = 0; i < path->size; i++)
	{		
		int x = path->pixelList[i].x;
		int y = path->pixelList[i].y;

		if( x < 0 ) x = 0;
		if( x > image->width - 1) x = image->width - 1;
		if( y < 0 ) y = 0;
		if( y > image->height - 1) y = image->height - 1;

		cvSet2D(image, y, x, pixel);
	}
}