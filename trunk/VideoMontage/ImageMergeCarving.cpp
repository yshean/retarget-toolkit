#include "StdAfx.h"
#include "ImageMergeCarving.h"

ImageMergeCarving::ImageMergeCarving(void)
{
}

ImageMergeCarving::~ImageMergeCarving(void)
{}

IplImage* ImageMergeCarving::GetMergedImage(IplImage* image1, IplImage* image2, int overlap_size)
{
	// get overlap area	
	// the overlap area of right image is shifted 1 pixel
	// so they are align one pixel shifted with each other
	// so 2 pixel which is neighbor (horizontally) will have the 
	// same coordinate in each overlap images.
	IplImage* overlap1 = extractor->GetRegion(image1, 
		cvPoint(image1->width - overlap_size, 0), 
		cvSize(overlap_size, image1->height));

	IplImage* overlap2 = extractor->GetRegion(image2,
		cvPoint(1, 0), 
		cvSize(overlap_size, image2->height));

	IplImage* overlap_cost = cvCreateImage(cvSize(overlap_size, image1->height), IPL_DEPTH_8U, 1);
	// get cost image when a pixel is chosen to be interface between 2 images
	for(int x = 0; x < overlap_size; x++)
		for(int y = 0; y < image1->height; y++)
	{
		CvScalar pixel_1 = cvGet2D(overlap1, y, x);
		CvScalar pixel_2 = cvGet2D(overlap2, y, x);
		double sum = 0;
		for(int i = 0; i < 4; i++)
		{
			sum += abs(pixel_1.val[i] - pixel_2.val[i]);
		}
		cvSet2D(overlap_cost, y, x, cvScalar(sum));
	}

	cvNamedWindow("overlap_cost");
	while(1)
	{
		cvShowImage("overlap_cost", overlap_cost);
		cvWaitKey(100);
	}


	
	IplImage* result;
	//SeamCarving* carving = new SeamCarving();
	return result;
}


void ImageMergeCarving::GetOverlapImages(IplImage *image1, IplImage *image2, 
										 IplImage **overlap1, IplImage **overlap2,
										 int overlap_size)
{
	*overlap1 = extractor->GetRegion(image1, 
		cvPoint(image1->width - overlap_size, 0), 
		cvSize(overlap_size, image1->height));

	*overlap2 = extractor->GetRegion(image2,
		cvPoint(1, 0), 
		cvSize(overlap_size, image2->height));
}

void TestOverlap()
{
	ImageMergeCarving* carving = new ImageMergeCarving();
	RegionExtractor* extractor = new RegionExtractor();
	carving->extractor = extractor;

	IplImage* image1 = cvLoadImage("test.jpg");
	IplImage* image2 = cvLoadImage("test.jpg");

	IplImage* overlap1 = 0;
	IplImage* overlap2 = 0;
	carving->GetOverlapImages(image1, image2, &overlap1, &overlap2, 200);

	cvNamedWindow("overlap1");
	cvNamedWindow("overlap2");
	while(1)
	{
		cvShowImage("overlap1", overlap1);
		cvShowImage("overlap2", overlap2);
		cvWaitKey(100);
	}
}

void TestImageMergeCarving()
{
	TestOverlap();
}
