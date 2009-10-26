#pragma once
#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include "SeamCarvingOld.h"
#include "EnergyPath.h"
#include "PolarTransform.h"
#include "EnergyCalculator.h"
#include <math.h>
#include "ImageViewer.h"
#include "DrawingFunctions.h"

// EPF Header 
#include "ISurface.h"
#include "LinearLensSurface.h"
#include "DistanceMetric.h"
#include "DistanceMetricL.h"
#include "Point.h"
#include "BinarySearchIntersectAlgorithm.h"
#include "LineSegment.h"
#include "Camera.h"
#include "BilinearImageInterpolation.h"
#include "IntersectZoomAlgorithm.h"
#include "HemisphereSurface.h"
#include "GaussianLensSurface.h"
#include "DistanceMetricX.h"
#include "BufferTransformation.h"
#include "LensBufferIO.h"

using namespace ElasticPresentationFramework;

struct MousePosition
{
	int x;
	int y;
};


void mouseHandler(int event, int x, int y, int flags, void* param)
{
	switch (event)
	{
	case CV_EVENT_MOUSEMOVE:
		// update mousePosition
		MousePosition* pos = (MousePosition*) param;
		pos->x = x;
		pos->y = y;
		break;
	}
}

namespace Zooming
{
	// testing with mouse event
	public ref class MouseTester
	{
	private:
		MousePosition* mousePosition;
		static char* imagePath = "image.jpg";
	public:
		MouseTester(void)
		{
			cvNamedWindow("Result");			
			mousePosition = new MousePosition();
			mousePosition->x = 200;
			mousePosition->y = 200;
			cvSetMouseCallback("Result", mouseHandler, mousePosition); 
		}
		
		// hover the mouse over a window to detect best focus region
		void TestFocusRegionWithMouse()
		{
			// setting for polar transform
			int unitAngle = 2;
			int resolution = 40;
			int outerRadius = 140;
			int innerRadius = 40; 	
			// initial position
			int positionX;
			int positionY;

			IplImage* input = cvLoadImage("image.jpg");
			EnergyCalculator* sobel = new SobelEnergy(1,1);			

			MinEnergyPath* energyPath;

			PolarTransform* polar = new PolarTransform(unitAngle, resolution);

			cvNamedWindow("Result");
			cvNamedWindow("Polar");

			while(1)
			{
				positionX = mousePosition->x;
				positionY = mousePosition->y;
				IplImage* polarImage = polar->CreatePolarImage(input, 
					point2D(positionX, positionY), outerRadius, innerRadius);

				IplImage* energy = sobel->GetEnergyImage(polarImage);
				
				energyPath = new SeamCarving(energy);
				Path* path = energyPath->GetEnergyPath();
				Path* originPath = polar->GetOriginalPath(point2D(positionX, positionY), outerRadius,
					innerRadius, path);
				
				IplImage* inputClone = cvCloneImage(input);
				DrawPath(inputClone, originPath);
				IplImage* polarClone = cvCloneImage(polarImage);
				DrawPath(polarClone, path);

				cvShowImage("Result", inputClone);
				cvShowImage("Polar", polarClone);	 

				cvWaitKey(100);

				// tidy up
				cvReleaseImage(&polarImage);
				cvReleaseImage(&energy);
				cvReleaseImage(&inputClone);
				ReleasePath(path);
				ReleasePath(originPath);
			}
		}

		
		// hover the mouse over a window to and apply a lens
		void TestSeamCarvingLens()
		{
			IplImage* input = cvLoadImage("image.jpg");
			// setting for polar transform
			int unitAngle = 2;
			int resolution = 40;
			int outerRadius = 140;
			int innerRadius = 40; 	 
			
			EnergyCalculator* energyCalculator = new SobelEnergy(1,1);					

			PolarTransform* polar = new PolarTransform(unitAngle, resolution);			 			

			while(1)
			{
				Path* path = GetEnergyPath(input, polar, energyCalculator, 
					mousePosition->x, mousePosition->y, outerRadius, innerRadius);

				cvWaitKey(100);
				ReleasePath(path);
			}
		}
 
		// Get the path in the origin image
		Path* GetEnergyPath(IplImage* input, PolarTransform* polar, EnergyCalculator* energyCalulator,
			int positionX, int positionY, int outerRadius, int innerRadius)
		{			
			IplImage* polarImage = polar->CreatePolarImage(input, 
				point2D(positionX, positionY), outerRadius, innerRadius);

			IplImage* energy = energyCalulator->GetEnergyImage(polarImage);
			
			MinEnergyPath* energyPath = new SeamCarving(energy);
			Path* path = energyPath->GetEnergyPath();
			Path* originPath = polar->GetOriginalPath(point2D(positionX, positionY), outerRadius,
				innerRadius, path);
			ReleasePath(path);
			return originPath;
		}

		// Initialize lens
		void InitializeLens()
		{
			LensTransformationBuffer* lensBuffer;
			DistanceMetric2D *distanceMetric = new DistanceMetric2DL();
			IntersectAlgorithm* intersectAlgorithm = new BinarySearchIntersectAlgorithm();			

			LensData* lensData = new LensData[15];
			LensBufferIO* io = new LensBufferIO();

			// initialize abt 15 lens
			for(int i = 20; i < 40; i++)
			{			
				double radius = i;	
				double deviation = i * 5 + 500;
				double height = 25; 
				double cameraHeight = 60; // magnification is always 2x
				

				double bufferSize = radius + 200;
				int centerPosition = bufferSize / 2;
				ElasticPresentationFramework::Point2D center = ElasticPresentationFramework::point2D(centerPosition, centerPosition);

				printf("Process buffer with radius: %i\n", i);
				
				// set up buffer
				ISurface* surface = new GaussianLensSurface(deviation, height, radius, distanceMetric, center);				
				lensBuffer = new LensTransformationBuffer(surface, intersectAlgorithm, cameraHeight);	
				lensBuffer->InitializeBuffer(bufferSize, bufferSize);	
				
				char fileName[20];
				strcpy(fileName, "radius");
				char num[5];
				sprintf(num, "%i", i);
				strcat(fileName, num);
				strcat(fileName, ".len");
				io->SaveLens(lensBuffer, fileName);
				printf("done \n");
			}
		}
		void TestZoomContentAware()
		{
			// setting for polar transform
			int unitAngle = 2;
			int resolution = 60;
			int outerRadius = 140;
			int innerRadius = 20; 	
			// initial position
			int positionX;
			int positionY;

			IplImage* input = cvLoadImage("image.jpg");
			EnergyCalculator* sobel = new SobelEnergy(1,1);			

			MinEnergyPath* energyPath;

			PolarTransform* polar = new PolarTransform(unitAngle, resolution);

			ImageInterpolation* interpolationAlgorithm = new BilinearImageInterpolation();

			cvNamedWindow("Result");
			cvNamedWindow("Polar");

			while(1)
			{
				positionX = mousePosition->x;
				positionY = mousePosition->y;
				IplImage* polarImage = polar->CreatePolarImage(input, 
					point2D(positionX, positionY), outerRadius, innerRadius);

				IplImage* energy = sobel->GetEnergyImage(polarImage);
				
				energyPath = new SeamCarving(energy);
				Path* path = energyPath->GetEnergyPath();
				Path* originPath = polar->GetOriginalPath(point2D(positionX, positionY), outerRadius,
					innerRadius, path);
				
				//IplImage* inputClone = cvCloneImage(input);
				//DrawPath(inputClone, originPath);
				//IplImage* polarClone = cvCloneImage(polarImage);
				//DrawPath(polarClone, path);

				// put a lens around the path
				
				MousePosition* lensPosition = new MousePosition();
				int radius;
				GetLensFromPath(originPath, lensPosition, &radius);

				if(radius < 20) radius = 20;
				if(radius > 60) radius = 60;
 
				BufferTransformation* buffer = new BufferTransformation(GetLens(radius), interpolationAlgorithm);
				IplImage* inputClone = buffer->GetTransformedImage(input, lensPosition->x, lensPosition->y);				

				cvShowImage("Result", inputClone);
				//cvShowImage("Polar", polarClone);	 

				cvWaitKey(100);

				// tidy up
				cvReleaseImage(&polarImage);
				cvReleaseImage(&energy);
				cvReleaseImage(&inputClone);
				ReleasePath(path);
				ReleasePath(originPath);
			}
		}

		void TestZoomVideoContentAware()
		{

			// setting for polar transform
			int unitAngle = 2;
			int resolution = 60;
			int outerRadius = 140;
			int innerRadius = 20; 	
			// initial position
			int positionX;
			int positionY;

			 
			EnergyCalculator* sobel = new SobelEnergy(1,1);			

			MinEnergyPath* energyPath;

			PolarTransform* polar = new PolarTransform(unitAngle, resolution);

			ImageInterpolation* interpolationAlgorithm = new BilinearImageInterpolation();

			cvNamedWindow("Result");
			cvNamedWindow("Polar");

			// setup frame capture =========================
			CvCapture* capture = cvCreateFileCapture("video.avi"); 
			IplImage* input = 0; 
			if(!cvGrabFrame(capture)){              // capture a frame 
			  printf("Could not grab a frame\n\7");
			  exit(0);
			}

			int numFrames = (int) cvGetCaptureProperty(capture,  CV_CAP_PROP_FRAME_COUNT);

			// setup video and mouse
			cvNamedWindow("video"); 
		 
			for(int i=0; i<numFrames;i++)
			{		 
				input = cvRetrieveFrame(capture);
				cvGrabFrame(capture);
				//result = buffer->GetTransformedImage(img, pos.x, pos.y);

				positionX = mousePosition->x;
				positionY = mousePosition->y;
				IplImage* polarImage = polar->CreatePolarImage(input, 
					point2D(positionX, positionY), outerRadius, innerRadius);

				IplImage* energy = sobel->GetEnergyImage(polarImage);
				
				energyPath = new SeamCarving(energy);
				Path* path = energyPath->GetEnergyPath();
				Path* originPath = polar->GetOriginalPath(point2D(positionX, positionY), outerRadius,
					innerRadius, path);
				
				//IplImage* inputClone = cvCloneImage(input);
				//DrawPath(inputClone, originPath);
				//IplImage* polarClone = cvCloneImage(polarImage);
				//DrawPath(polarClone, path);

				// put a lens around the path
				
				MousePosition* lensPosition = new MousePosition();
				int radius;
				GetLensFromPath(originPath, lensPosition, &radius);

				if(radius < 20) radius = 20;
				if(radius > 60) radius = 60;
 
				BufferTransformation* buffer = new BufferTransformation(GetLens(radius), interpolationAlgorithm);
				IplImage* inputClone = buffer->GetTransformedImage(input, lensPosition->x, lensPosition->y);				

				cvShowImage("Result", inputClone);
				//cvShowImage("Polar", polarClone);	 

				cvWaitKey(100);

				// tidy up
				cvReleaseImage(&polarImage);
				cvReleaseImage(&energy);
				cvReleaseImage(&inputClone);
				ReleasePath(path);
				ReleasePath(originPath);
				 
			}
		}
		
		// 
		void GetLensFromPath(Path* path, MousePosition* position, int* radius)
		{
			int minX = int::MaxValue;
			int maxX = int::MinValue;
			int minY = int::MaxValue;
			int maxY = int::MinValue;

			for(int i = 0; i < path->size; i++)
			{
				int x = path->pixelList[i].x;
				int y = path->pixelList[i].y;
				
				if(x < minX) minX = x;
				if(x > maxX) maxX = x;
				if(y < minY) minY = y;
				if(y > maxY) maxY = y;
			}

			int width = maxX - minX;
			int height = maxY - minY;
			
			position->x = minX + width/2;
			position->y = minY + height/2;

			// the greater range
			if(width > height) *radius = width/2;
			else *radius = height/2;

		}

		void TestLoadLens()
		{
			ImageInterpolation* interpolationAlgorithm = new BilinearImageInterpolation();
			IplImage* image = cvLoadImage("image.jpg");
			BufferTransformation* buffer = new BufferTransformation(GetLens(60), interpolationAlgorithm);

			cvNamedWindow("Result");

			while(1)
			{
				IplImage* result = buffer->GetTransformedImage(image, mousePosition->x, mousePosition->y);
				cvShowImage("Result", result);
				cvWaitKey(100);
				cvReleaseImage(&result);
			}
	
		}
		LensData* GetLens(int radius)
		{
			LensBufferIO* io = new LensBufferIO();
			char fileName[20];
			strcpy(fileName, "radius");
			char num[5];
			sprintf(num, "%i", radius);
			strcat(fileName, num);
			strcat(fileName, ".len");			 
			return io->LoadLens(fileName);
		}
	};
}


