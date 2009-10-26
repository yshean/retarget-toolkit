#include "StdAfx.h"
#include "PolarTransform.h"

Zooming::PolarTransform::PolarTransform(double unitAngle, int resolution)
{
	 width = resolution;
	 height = 360 / unitAngle;
	 this->unitAngle = unitAngle;
}

IplImage* Zooming::PolarTransform::CreatePolarImage(
	IplImage *image, 
	Zooming::Point2D center, 
	int outerRadius,
	int innerRadius)
{
	int inputWidth = image->width;
	int inputHeight = image->height;

	IplImage* result = cvCreateImage(cvSize(width, height), image->depth, image->nChannels);

	double increment = (outerRadius - innerRadius) / (width - 1);

	for(int x = 0; x < width; x++)
		for(int y = 0; y < height; y++)
		{
			double angle = y * unitAngle;
			double radius = innerRadius + x * increment;

			// get target pixel
			Point2D target = GetCartersianCoordination(angle, radius);
			// shift
			target.x += center.x;
			target.y += center.y;		
			
			// get the border pixel if the region is out of image
			if(target.x < 0)
				target.x = 0;
			if(target.x > inputWidth - 1)
				target.x = inputWidth - 1;
			if(target.y < 0)
				target.y = 0;
			if(target.y > inputHeight - 1)
				target.y = inputHeight - 1;

			CvScalar value = cvGet2D(image, target.y, target.x);
			cvSet2D(result, y, x, value);
		}
	return result;
}


Zooming::Path* Zooming::PolarTransform::GetOriginalPath(Point2D center, int outerRadius, 
														int innerRadius, 
														Path* path)
{
	Path* result = new Path();
	result->pixelList = new Pixel[path->size];
	result->size = path->size;

	Pixel* pixelList = path->pixelList;
	double increment = (outerRadius - innerRadius) / (width - 1);

	for(int i = 0; i < path->size; i++)
	{
		int x = pixelList[i].x;
		int y = pixelList[i].y;

		double angle = y * unitAngle;
		double radius = innerRadius + x * increment;

		// get target pixel
		Point2D target = GetCartersianCoordination(angle, radius);
		// shift
		target.x += center.x;
		target.y += center.y;
		
		result->pixelList[i].x = target.x;
		result->pixelList[i].y = target.y;
	}
	return result;
}

Zooming::PointDouble2D Zooming::pointDouble2D(double x, double y)
{
	PointDouble2D point;
	point.x = x;
	point.y = y;
	return point;
}


Zooming::Point2D Zooming::point2D(int x, int y)
{
	Point2D point;
	point.x = x;
	point.y = y;
	return point;
}


 