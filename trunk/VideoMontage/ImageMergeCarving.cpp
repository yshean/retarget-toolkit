#include "StdAfx.h"
#include "ImageMergeCarving.h"

ImageMergeCarving::ImageMergeCarving(void)
{
}

ImageMergeCarving::~ImageMergeCarving(void)
{}

IplImage* ImageMergeCarving::GetMergedImage(IplImage* image1, IplImage* image2, int overlap_size)
{
	IplImage* overlap1;
	IplImage* overlap2;

	GetOverlapImages(image1, image2, &overlap1, &overlap2, overlap_size);

	IplImage* overlap_cost = CreateOverlapImageCost(overlap1, overlap2);
	
	MinEnergyPath* minEnergyPath = new SeamCarving(overlap_cost);
	Path* path = minEnergyPath->GetEnergyPath();
	DrawPath(overlap_cost, path);

	IplImage* result;
	//SeamCarving* carving = new SeamCarving();
	return result;
}

IplImage* ImageMergeCarving::CreateOverlapImageCost(IplImage* overlap1, IplImage* overlap2)
{
	int height = overlap1->height;
	int width = overlap2->width;

	IplImage* overlap_cost = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	// get cost image when a pixel is chosen to be interface between 2 images
	for(int x = 1; x < width; x++)
		for(int y = 0; y < height; y++)
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
	return overlap_cost;
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


void TestOverlapMinPath()
{
#pragma region Load Images
	ImageMergeCarving* carving = new ImageMergeCarving();
	RegionExtractor* extractor = new RegionExtractor();
	carving->extractor = extractor;

	IplImage* image1 = cvLoadImage("test.jpg");
	IplImage* image2 = cvLoadImage("test.jpg");
#pragma endregion
#pragma region Get Overlap Cost Image
	IplImage* overlap1 = 0;
	IplImage* overlap2 = 0;
	carving->GetOverlapImages(image1, image2, &overlap1, &overlap2, 30);

	IplImage* overlap_cost = carving->CreateOverlapImageCost(overlap1, overlap2);
#pragma endregion
	
	// get min path
	//IplImage* input = image1;
	//IplImage* dstX = cvCloneImage(input);	
	//IplImage* dstY = cvCloneImage(input);
	//cvSobel(input, dstY, 0, 1);
	//cvSobel(input, dstX, 1, 0);

	//int width = input->width;
	//int height = input->height;

	//// average 3 channels to 1
	//IplImage* energy = cvCreateImage(cvSize(width, height), IPL_DEPTH_16U, 1);
	////
	////char* dstData = dstX->imageData;
	////short* energyData = (short*)energy->imageData;

	//for(int i = 0; i < height; i++)
	//{
	//	for(int j = 0; j < width; j++)
	//	{
	//		CvScalar inputPixelX = cvGet2D(dstX, i, j); 
	//		CvScalar inputPixelY = cvGet2D(dstY, i, j);
 //
	//		CvScalar outputX = cvScalar( abs(inputPixelX.val[0] + inputPixelX.val[1] + inputPixelX.val[2]) / 3);
	//		CvScalar outputY = cvScalar( abs(inputPixelY.val[0] + inputPixelY.val[1] + inputPixelY.val[2]) / 3);
	//		
	//		CvScalar output;
	//		output.val[0] = outputX.val[0] + outputY.val[0];

	//		cvSet2D(energy, i, j, output);
	//	}
	//}

	//MinEnergyPath* minEnergyPath = new SeamCarving(energy);
	//Path* path = minEnergyPath->GetEnergyPath();
	//DrawPath(image1, path);

	MinEnergyPath* minEnergyPath = new SeamCarving(overlap_cost);
	Path* path = minEnergyPath->GetEnergyPath();
	DrawPath(overlap_cost, path);
	
	// drawing
	cvNamedWindow("overlap_cost");
	while(1)
	{
		cvShowImage("overlap_cost", overlap_cost);
		cvWaitKey(100);
	}

}
void TestOverlapCost()
{
#pragma region Load Images
	ImageMergeCarving* carving = new ImageMergeCarving();
	RegionExtractor* extractor = new RegionExtractor();
	carving->extractor = extractor;

	IplImage* image1 = cvLoadImage("test.jpg");
	IplImage* image2 = cvLoadImage("test.jpg");
#pragma endregion

	IplImage* overlap1 = 0;
	IplImage* overlap2 = 0;
	carving->GetOverlapImages(image1, image2, &overlap1, &overlap2, 30);

	IplImage* overlap_cost = carving->CreateOverlapImageCost(overlap1, overlap2);

	cvNamedWindow("overlap_cost");
	while(1)
	{
		cvShowImage("overlap_cost", overlap_cost);
		cvWaitKey(100);
	}

}

void TestOverlap()
{
#pragma region Load Images
	ImageMergeCarving* carving = new ImageMergeCarving();
	RegionExtractor* extractor = new RegionExtractor();
	carving->extractor = extractor;

	IplImage* image1 = cvLoadImage("test.jpg");
	IplImage* image2 = cvLoadImage("test.jpg");
#pragma endregion

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
	// TestOverlap();
	// TestOverlapCost();
	TestOverlapMinPath();
}
