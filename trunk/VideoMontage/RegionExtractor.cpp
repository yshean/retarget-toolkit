#include "StdAfx.h"
#include "RegionExtractor.h"

RegionExtractor::RegionExtractor(void)
{
}

RegionExtractor::~RegionExtractor(void)
{
}

IplImage* RegionExtractor::GetRegion(IplImage *image, CvPoint position, CvSize size)
{
	// cropping
	if(position.x + size.width >= image->width)
	{
		size.width = image->width - position.x;
	}
	if(position.y + size.height >= image->height)
	{
		size.height = image->height - position.y;
	}

	IplImage* result = cvCreateImage(size, image->depth, image->nChannels);
	for(int x = 0; x < size.width; x++)
		for(int y = 0; y < size.height; y++)
		{
			CvScalar value = cvGet2D(image, y + position.y, x + position.x);
			cvSet2D(result, y, x, value);
		}
	return result;
}

//**********************************************
// TESTING
void TestRegionExtractor()
{
	IplImage* image = cvLoadImage("test.jpg");
	if(image == 0)
	{
		printf("Can not find test image test.jpg");
		exit(-1);
	}

	RegionExtractor* extractor = new RegionExtractor();
	int x, y, width, height;
	x = 100;
	y = 300;
	width = 200;
	height = 900;
	IplImage* result = extractor->GetRegion(image, cvPoint(x, y), cvSize(width, height));
	printf("Extracted with position x = %i, y = %i, width = %i, height = %i \n", x, y, width, height);

	cvNamedWindow("Origin");
	cvNamedWindow("Result");

	while(1)
	{
		cvShowImage("Origin", image);
		cvShowImage("Result", result);
		int key = cvWaitKey(100);
		if(key == 27) break;
	}
}
