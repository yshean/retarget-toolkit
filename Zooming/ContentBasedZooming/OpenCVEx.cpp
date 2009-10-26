#include "stdafx.h"
#include "OpenCVEx.h"

double OpenCVEx::GetValue(IplImage* image, int x, int y)
	{
		if(x < 0 || y < 0
			|| x > image->width - 1 || y > image->height - 1) 
		{			 
			return double::MaxValue;
		} 
		//printf("image info: width: %i height: %i", image->width, image->height);
		CvScalar value = cvGet2D(image, y, x);
		//if(value.val[0] < -1 || value.val[0] > 1)
		//	printf("end");
		return value.val[0];
	}

// return min possible value instead of max double
double OpenCVEx::GetValueEx(IplImage* image, int x, int y)
	{
		if(x < 0 || y < 0
			|| x > image->width - 1 || y > image->height - 1) return double::MinValue;
		CvScalar value = cvGet2D(image, y, x);
		return value.val[0];
	}

void OpenCVEx::SetValue(IplImage* image, int x, int y, double value)
	{
		if(x < 0 || y < 0
			|| x > image->width || y > image->height) return;

		cvSet2D(image, y, x, cvScalar(value));
	}