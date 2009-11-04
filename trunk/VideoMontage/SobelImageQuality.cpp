#include "StdAfx.h"
#include "SobelImageQuality.h"

SobelImageQuality::SobelImageQuality(void)
{
}

SobelImageQuality::~SobelImageQuality(void)
{
}

double SobelImageQuality::GetImageQuality(IplImage* image)
{
	IplImage* clone = cvCloneImage(image);
	cvSobel(image, clone, 2, 2);
	
	int width = clone->width;
	int height = clone->height;

	double max = 0;

	for(int x = 0; x < width; x++)
		for(int y = 0; y < height; y++)
		{
			CvScalar value = cvGet2D(clone, y, x);
			double tempMax = abs(value.val[0]) + abs(value.val[1]) 
				+ abs(value.val[2]) + abs(value.val[3]);
			if(tempMax > max)
				max = tempMax;			
		}
	return max;
}
 