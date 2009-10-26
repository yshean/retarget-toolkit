#pragma once
#include <cv.h>
#include <highgui.h>
#include "SeamCarvingOld.h"
#include "EnergyPath.h"
#include "PolarTransform.h"
#include "EnergyCalculator.h"
#include <math.h>
#include "ImageViewer.h"

namespace Zooming
{ 
	public ref class ZoomingTester
	{
	private:
		IplImage* image;
	public:
		ZoomingTester(void);
	public:
		~ZoomingTester(void);
		
	private:
		IplImage* GetEnergyImage(IplImage* input);
		void DrawPath(IplImage* image, Path* path);
	public:		 
	
		//void TestGetEnergy();

		//// rewrite algo
		//void TestNewPathGenerate();

		//// create an polar image according to selected position
		//void TestPolarImageGenerator();

		//// Test generate path on the polar image
		//void TestPathPolar();

		// Test Image Viewer
		void TestImageViewer()
		{

			for(int i = 0; i < 1; i++)
			{
				IplImage* image = cvLoadImage("image.jpg");
				ImageViewer* viewer = new ImageViewer();
				viewer->PushImage(image, "image");
			}
			while(1)
			{
				//viewer->ShowAllWindows();
				cvWaitKey();
			}
		}


		
		// take image.jpg and draw the minimized seam across the image
		void TestSeamCarving()
		{
			IplImage* input = cvLoadImage("image.jpg");

			IplImage* energy = GetEnergyImage(input);

			SeamCarving* seamCarving = new SeamCarving(energy);

			Path* path = seamCarving->GetEnergyPath();

			cvNamedWindow("Result");

			while(1)
			{
				// clone input so that old path is deleted
				IplImage* inputClone = cvCloneImage(input);
				DrawPath(inputClone, path);
				cvShowImage("Result", inputClone);
				cvWaitKey(100);
				cvReleaseImage(&inputClone);				
			}

			cvReleaseImage(&energy);			
			cvDestroyWindow("Result");
		}

		// draw a circle denote the focus region by process in polar coordination
		void TestPolarSeamCarving()
		{
			// setting for polar transform
			int unitAngle = 2;
			int resolution = 40;
			int outerRadius = 140;
			int innerRadius = 40;
			// setting for focus region			
			int positionX = 200;
			int positionY = 200;			 

			IplImage* input = cvLoadImage("image.jpg");
			EnergyCalculator* sobel = new SobelEnergy(1,1);			

			MinEnergyPath* energyPath;

			PolarTransform* polar = new PolarTransform(unitAngle, resolution);

			cvNamedWindow("Result");
			cvNamedWindow("Polar");

			while(1)
			{
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
				
				// changing focus region
				int key = cvWaitKey(100);
				if(key == 97) positionX--;
				if(key == 100) positionX++;
				if(key == 119) positionY--;
				if(key == 115) positionY++;

				// tidy up
				cvReleaseImage(&polarImage);
				cvReleaseImage(&energy);
				cvReleaseImage(&inputClone);
				ReleasePath(path);
				ReleasePath(originPath);
			}
		}
	};
}