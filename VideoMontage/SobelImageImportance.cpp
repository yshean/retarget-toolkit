#include "StdAfx.h"
#include "SobelImageImportance.h"

SobelImageImportance::SobelImageImportance(void)
{
}

SobelImageImportance::~SobelImageImportance(void)
{
}

double SobelImageImportance::GetImageImportance(IplImage* image)
{
	IplImage* clone;
	if(image->depth == IPL_DEPTH_8U)
	{
		clone = cvCreateImage(cvSize(image->width, image->height), IPL_DEPTH_16S, image->nChannels);	
		cvSobel(image, clone, 2, 2);		
		double sum = 0;
		for(int x = 0; x < image->width; x++)
			for(int y = 0; y < image->height; y++)
			{
				CvScalar value = cvGet2D(clone, y, x);
				sum += abs(value.val[0]) + abs(value.val[1]) + abs(value.val[2]);
			}
		return sum;
	}
	else
	{
		printf("need IPL_DEPTH_8U image");
		return -1;
	}
}
