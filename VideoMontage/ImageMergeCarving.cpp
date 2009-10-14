#include "StdAfx.h"
#include "ImageMergeCarving.h"

ImageMergeCarving::ImageMergeCarving(void)
{
}

ImageMergeCarving::~ImageMergeCarving(void)
{}

IplImage* ImageMergeCarving::GetMergedImage(IplImage* image1, IplImage* image2, int overlap_size)
{
	IplImage* result;

	// get overlap area	
	IplImage* overlap1 = extractor->GetRegion(image1, 
		cvPoint(image1->width - overlap_size, 0), 
		cvSize(overlap_size, image1->height));

	IplImage* overlap2 = extractor->GetRegion(image2,
		cvPoint(0, 0), 
		cvSize(overlap_size, image2->height));

	cvNamedWindow("overlap1");
	cvNamedWindow("overlap2");
	while(1)
	{
		cvShowImage("overlap1", overlap1);
		cvShowImage("overlap2", overlap2);
		cvWaitKey(100);
	}

	//SeamCarving* carving = new SeamCarving();
	return result;
}

void TestImageMergeCarvingShowOverlap()
{
	ImageMergeCarving* carving = new ImageMergeCarving();
	RegionExtractor* extractor = new RegionExtractor();
	carving->extractor = extractor;

	IplImage* image1 = cvLoadImage("test.jpg");
	IplImage* image2 = cvLoadImage("test.jpg");

	carving->GetMergedImage(image1, image2, 200);
}