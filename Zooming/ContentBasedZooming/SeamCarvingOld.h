#pragma once
#include <cv.h>
#include <math.h>
#include "MinEnergyPath.h"
#include "MinMax.h"
#include "IplWrapper.h"
#include "OpenCVEx.h"

//using namespace ImageEnergy;
using namespace OpenCV;
using namespace Zooming;
using namespace CVMath;

namespace SeamCarving
{	 
	using namespace OpenCVEx;

	public class SeamCarver
	{
	public:
		SeamCarver()
		{
			// dummy creation to release them later
			energyMap = cvCreateImage(cvSize(100, 100), IPL_DEPTH_8U, 3);
			pathInfo = cvCreateImage(cvSize(100, 100), IPL_DEPTH_8U, 3);
		}
		// IPL_DEPTH_32F
		IplImage* energyMap;
		IplImage* pathInfo;
	public:
		// First step in seam carving
		// return an image with same size, each pixel denote its energy
		// notes that a region of an image can also be processed by changing the width/widthstep
		// input: IplImage Energy Input depth 32F, 1 channel
		// return a IplImage with depth 32F, 1 channel		
		void PreprocessEnergy(IplImage* inputEnergy)
		{
			int width = inputEnergy->width;
			int height = inputEnergy->height;

			//if(energyMap != NULL) 
			//{
			//	cvReleaseImage(&energyMap);
			//}
			cvReleaseImage(&energyMap);
			energyMap = cvCreateImage(cvSize(width, height), inputEnergy->depth, inputEnergy->nChannels);
			energyMap = cvCloneImage(inputEnergy);

			//if(pathInfo != 0) cvReleaseImage(&pathInfo);
			cvReleaseImage(&pathInfo);
			pathInfo = cvCreateImage(cvSize(width, height), IPL_DEPTH_8S, 1);

			for(int y = 1; y < height; y++)
			{
				for(int x = 0; x < width; x++)
				{
					int position;
					// favor the directly above pixel
					double min = GetMinOfThree<double>(
						GetValue(energyMap, x - 1, y - 1), 						
						GetValue(energyMap, x, y - 1),
						GetValue(energyMap, x + 1, y - 1), 						
						&position);

					double energyValue = min + GetValue(inputEnergy, x, y);
					SetValue(energyMap, x, y, energyValue);

					//position = position * 40;
					position = position - 2; // make it to the range -1 0 1			
					SetValue(pathInfo, x, y, position);
				}
			}
		}


		// This is to find the max seam instead of a min
		void PreprocessEnergyMax(IplImage* inputEnergy)
		{
			int width = inputEnergy->width;
			int height = inputEnergy->height;

			cvReleaseImage(&energyMap);
			energyMap = cvCreateImage(cvSize(width, height), inputEnergy->depth, inputEnergy->nChannels);
			energyMap = cvCloneImage(inputEnergy);

			//if(pathInfo != 0) cvReleaseImage(&pathInfo);
			cvReleaseImage(&pathInfo);
			pathInfo = cvCreateImage(cvSize(width, height), IPL_DEPTH_8S, 1);

			for(int y = 1; y < height; y++)
			{
				for(int x = 0; x < width; x++)
				{
					int position;
					// favor the directly above pixel
					double min = GetMaxOfThree<double>(
						GetValueEx(inputEnergy, x - 1, y -1), 						
						GetValueEx(inputEnergy, x, y-1),
						GetValueEx(inputEnergy, x+1, y-1), 						
						&position);

					double energyValue = min + GetValueEx(inputEnergy, x, y);
					SetValue(energyMap, x, y, energyValue);

					position = position - 2; // make it to the range -1 0 1
					SetValue(pathInfo, x, y, position);
				}
			}
		}
		// generate the path
		Path* GeneratePath(int position)
		{			 
			// from bottom up
			Path* path = new Path();
			path->size = pathInfo->height;
			Pixel* pixelList = new Pixel[path->size];

			pixelList[0].y = path->size - 1;
			pixelList[0].x = position;

			for(int i = 1; i < path->size; i++)
			{
				int currentY = path->size - i - 1;
				pixelList[i].y = currentY;				

				int increment = (int) GetValue(pathInfo, position, currentY);
				//printf("position: %i Y: %i", position, currentY);
				//printf("Incre: %i \n" , increment);
				//if(i == path->size - 1)
				//	printf("end \n");
				position = position + (int) GetValue(pathInfo, position, currentY);
				pixelList[i].x = position;
			}

			path->pixelList = pixelList;
			return path;
		}



		
		// Get the min position of the pathinfo
		int GetMinPosition()
		{
			int width = pathInfo->width;			
			int height = pathInfo->height;

			double min = double::MaxValue;
			int position;
			for(int i = 0; i < width; i++)
			{
				double value = GetValue(pathInfo, i, height - 1);
				if(value < min) 
				{
					min = value;
					position = i;
				}
			}
			return position;
		}

		int GetMaxPosition()
		{
			int width = pathInfo->width;			
			int height = pathInfo->height;

			double max = double::MinValue;
			int position;
			for(int i = 0; i < width; i++)
			{
				double value = GetValueEx(pathInfo, i, height - 1);
				if(value > max) 
				{
					max = value;
					position = i;
				}
			}
			return position;
		}


	 

 
	}; 
}
 