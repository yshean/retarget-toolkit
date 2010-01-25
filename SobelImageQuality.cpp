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
	IplImage* clone;
	IplImage* gray = cvCreateImage(cvSize(image->width, image->height), image->depth, 1);
	cvCvtColor(image, gray, CV_RGB2GRAY);
	if(image->depth = IPL_DEPTH_8U)
	{	
		clone = cvCreateImage(cvSize(image->width, image->height), IPL_DEPTH_16S, 1);
		
		cvSobel(gray, clone, 2, 2);
		
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
	else
	{
		printf("need IPL_DEPTH_8U image");
		return -1;
	}
}
 