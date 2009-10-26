#include "StdAfx.h"
#include "ZoomingTester.h"

Zooming::ZoomingTester::ZoomingTester(void)
{ 
	image = cvLoadImage("image.jpg");	
}

Zooming::ZoomingTester::~ZoomingTester(void)
{
}
/*
void Zooming::ZoomingTester::TestEnergyGenerate()
{ 
	if(image == 0) 
	{
		printf("no image found");
		return;
	}
	

	SeamAlgorithm* algorithm = new SeamAlgorithm(image->width, image->height);
	int width = image->width;
	int height = image->height;
	//IplImage* dst = cvCreateImage(cvSize(width, height), IPL_DEPTH_16U, 3);
	IplImage* dst = cvCloneImage(image);
	cvSobel(image, dst, 2, 2);

	// average 3 channels to 1
	IplImage* energy = cvCreateImage(cvSize(width, height), IPL_DEPTH_16U, 1);
	
	char* dstData = dst->imageData;
	short* energyData = (short*)energy->imageData;

	for(int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j++)
		{
			CvScalar input = cvGet2D(dst, i, j); 
			CvScalar output = cvScalar( abs(input.val[0] + input.val[1] + input.val[2]) * 200 / 3);
			cvSet2D(energy, i, j, output);
		}
		//dstData += dst->widthStep;
		//energyData += energy->width;
	}



	algorithm->PreprocessEnergyPath<short>(energy);

	double min = 10000000;
	int minPosition;
	// find the min position
	for(int i = 0; i < width; i++ )
	{
		CvScalar pixel = cvGet2D(algorithm->GetMap(), height - 1, i);
		double value = pixel.val[0];
		if( value <= min )
		{		
			min = value;
			minPosition = i;
		}
	}



	cvNamedWindow("Result");
	cvNamedWindow("Sobel");
	cvNamedWindow("PreprocessedMap");
	cvNamedWindow("PathInfo");
	cvNamedWindow("Energy");

	minPosition = 200;

	while(1)
	{	Path* path = algorithm->GenerateEneryPath(minPosition);
		IplImage* image2 = cvCloneImage(image);
		// draw the path to the original image
		//for(int i = 0; i < path->size; i++)
		//{
		//	CvScalar pixel = cvScalar(0, 255, 255);		
		//	cvSet2D(image2, path->pixelList[i].y, path->pixelList[i].x, pixel);
		//}

		CvScalar pixel = cvScalar(0, 0, 255);
		for(int i = 0; i < 100; i++)
		{
			cvSet2D(image2, 0, i, pixel);
		}

		cvShowImage("Sobel", dst);
		cvShowImage("Energy", energy);
		cvShowImage("PreprocessedMap", algorithm->GetMap());
		cvShowImage("PathInfo", algorithm->GetPathInfo());
		cvShowImage("Result", image2);
		cvReleaseImage(&image2);
		int key = cvWaitKey(100);
		if(key == 97) minPosition--;
		if(key == 100) minPosition++;
	}

}



*/
IplImage* Zooming::ZoomingTester::GetEnergyImage(IplImage* input)
{
	if(input == 0) 
	{
		printf("no image");
		return 0;
	}	 	 
 
	IplImage* dstX = cvCloneImage(input);	
	IplImage* dstY = cvCloneImage(input);
	cvSobel(input, dstY, 0, 1);
	cvSobel(input, dstX, 1, 0);

	int width = input->width;
	int height = input->height;

	// average 3 channels to 1
	IplImage* energy = cvCreateImage(cvSize(width, height), IPL_DEPTH_16U, 1);
	//
	//char* dstData = dstX->imageData;
	//short* energyData = (short*)energy->imageData;

	for(int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j++)
		{
			CvScalar inputPixelX = cvGet2D(dstX, i, j); 
			CvScalar inputPixelY = cvGet2D(dstY, i, j);
 
			CvScalar outputX = cvScalar( abs(inputPixelX.val[0] + inputPixelX.val[1] + inputPixelX.val[2]) / 3);
			CvScalar outputY = cvScalar( abs(inputPixelY.val[0] + inputPixelY.val[1] + inputPixelY.val[2]) / 3);
			
			CvScalar output;
			output.val[0] = outputX.val[0] + outputY.val[0];

			cvSet2D(energy, i, j, output);
		}
		//dstData += dst->widthStep;
		//energyData += energy->width;
	}
	return energy;
}
//
//void Zooming::ZoomingTester::TestNewPathGenerate()
//{
//	SeamCarver* algorithm = new SeamCarver();
//	IplImage* energy = GetEnergyImage(image);
//
//	algorithm->PreprocessEnergy(energy);	
//	//algorithm->PreprocessEnergyMax(energy);
//	
//	cvNamedWindow("Result");
//	cvNamedWindow("Energy");
//	cvNamedWindow("Path");
//	cvNamedWindow("Map");
//	//int position = 200;
//	int position = algorithm->GetMinPosition();
//	//int position = algorithm->GetMaxPosition();
//
//	while(1)
//	{
//		IplImage* image2 = cvCloneImage(image);
//		
//		Path* path = algorithm->GeneratePath(position);
//
//		// draw the path to the original image
//		for(int i = 0; i < path->size; i++)
//		{
//			CvScalar pixel = cvScalar(0, 0, 255);		
//			cvSet2D(image2, path->pixelList[i].y, path->pixelList[i].x, pixel);
//		}
//
//		cvShowImage("Result", image2);
//		cvShowImage("Energy", energy);
//		cvShowImage("Path", algorithm->pathInfo);
//		cvShowImage("Map", algorithm->energyMap);
//
//		int key = cvWaitKey(100);
//		if(key == 97) position--;
//		if(key == 100) position++;
//
//		cvReleaseImage(&image2);
//	}
//}
//
//void Zooming::ZoomingTester::TestPolarImageGenerator()
//{
//	PolarTransform* polarTrans = new PolarTransform(2, 20);	
//
//	cvNamedWindow("Result");
//	cvNamedWindow("Original");
//
//	int positionX = 300;
//	int positionY = 300;
//	while(1)
//	{
//		IplImage* display = polarTrans->CreatePolarImage(image, point2D(positionX, positionY), 20, 50);
//		cvShowImage("Result", display);		
//		cvShowImage("Original", image);
//		int key = cvWaitKey(100);
//		if(key == 97) positionX--;
//		if(key == 100) positionX++;
//		if(key == 119) positionY--;
//		if(key == 115) positionY++;
//		cvReleaseImage(&display);
//	}
//}
//
//void Zooming::ZoomingTester::TestPathPolar()
//{
//	PolarTransform* polarTrans = new PolarTransform(3, 50);	
//	SeamCarver* algorithm = new SeamCarver();
//
//	cvNamedWindow("PolarPath");
//	cvNamedWindow("PolarEnergy");
//	cvNamedWindow("PreprocessedPolarEnergy");
//	cvNamedWindow("PolarPath");
//	cvNamedWindow("PolarClone");
//	cvNamedWindow("CloneImage");
//		
//
//	int positionX = 300;
//	int positionY = 200;
//
//
//
//
//
//	while(1)
//	{
//		IplImage* polarImage = polarTrans->CreatePolarImage(image, point2D(positionX, positionY), 120, 40);		
//		IplImage* polarEnergy = GetEnergyImage(polarImage);	
//		IplImage* polarClone = cvCloneImage(polarImage);
//		IplImage* imageClone = cvCloneImage(image);
//		algorithm->PreprocessEnergy(polarEnergy);			
//		int minPosition = algorithm->GetMinPosition(); 
//		Path* path = algorithm->GeneratePath(minPosition);
//
//
//		Path* originPath = polarTrans->GetOriginalPath(point2D(positionX, positionY), 120, 40, path);
//		
//		
//		// draw the path to the original image		
//		//IplImage* result = cvCloneImage(image);
//		
//		//for(int i = 0; i < originPath->size; i++)
//		//{				
//		//	cvSet2D(result, originPath->pixelList[i].y, originPath->pixelList[i].x, pixel);
//		//}
//
//		// draw path to polar image		
//		DrawPath(polarClone, path);
//		DrawPath(imageClone, originPath);
//
//		CvScalar pixel = cvScalar(255, 0, 0);
//		cvDrawCircle(imageClone, cvPoint(positionX, positionY), 40, pixel);
//		cvDrawCircle(imageClone, cvPoint(positionX, positionY), 120, pixel);
//
//		cvShowImage("PolarEnergy", polarEnergy);
//		cvShowImage("CloneImage", imageClone);		
//		cvShowImage("PolarClone", polarClone);
//		cvShowImage("PreprocessedPolarEnergy", algorithm->energyMap);
//		cvShowImage("PolarPath", algorithm->pathInfo);
//
//		//cvShowImage("Original", result);
//
//		int key = cvWaitKey(100);
//		if(key == 97) positionX--;
//		if(key == 100) positionX++;
//		if(key == 119) positionY--;
//		if(key == 115) positionY++;
//		cvReleaseImage(&polarImage);
//		cvReleaseImage(&polarEnergy);		
//		cvReleaseImage(&polarClone);
//		
//		//cvReleaseImage(&result);
//	}
//
//
//}
//
//void Zooming::ZoomingTester::TestGetEnergy()
//{
//	IplImage* energy = GetEnergyImage(image);
//
//	cvNamedWindow("Energy");
//	while(1)
//	{
//		cvShowImage("Energy", energy);
//		cvWaitKey(100);
//	}
//}

void Zooming::ZoomingTester::DrawPath(IplImage* image, Path* path)
{
	CvScalar pixel = cvScalar(0, 0, 255);	
	for(int i = 0; i < path->size; i++)
	{				
		cvSet2D(image, path->pixelList[i].y, path->pixelList[i].x, pixel);
	}
}