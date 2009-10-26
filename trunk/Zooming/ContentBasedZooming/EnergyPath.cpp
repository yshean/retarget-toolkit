#include "StdAfx.h"
#include "EnergyPath.h"
 

Zooming::SeamCarving::~SeamCarving()
{
}

Zooming::Path* Zooming::SeamCarving::GetEnergyPath()
{		
	int width = energy->width;
	int height = energy->height;

	IplImage* energyMap = cvCloneImage(energy);
	IplImage* pathInfo = cvCreateImage(cvSize(width, height), IPL_DEPTH_8S, 1);
	
	PreprocessEnergyMap(energy, energyMap, pathInfo);
	
	int minPosition = GetMinPosition(energyMap);

	Path* path = GeneratePath(minPosition, pathInfo);
	
	cvReleaseImage(&energyMap);
	cvReleaseImage(&pathInfo);

	return path;
}

void Zooming::SeamCarving::PreprocessEnergyMap(IplImage *energy, IplImage *energyMap, IplImage *pathInfo)
{
	int width = energy->width;
	int height = energy->height;

	//energyMap = cvCreateImage(cvSize(width, height), inputEnergy->depth, inputEnergy->nChannels);
	//energyMap = cvCloneImage(inputEnergy);

	//if(pathInfo != 0) cvReleaseImage(&pathInfo);
	//cvReleaseImage(&pathInfo);
	//pathInfo = cvCreateImage(cvSize(width, height), IPL_DEPTH_8S, 1);

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

			double energyValue = min + GetValue(energy, x, y);
			SetValue(energyMap, x, y, energyValue);

			//position = position * 40;
			position = position - 2; // make it to the range -1 0 1	 
			SetValue(pathInfo, x, y, position);
		}
	}
}
Zooming::Path* Zooming::SeamCarving::GeneratePath(int position, IplImage *pathInfo)
{
	// from bottom up
	Path* path = new Path();
	path->size = pathInfo->height;
	Pixel* pixelList = new Pixel[path->size];

	pixelList[0].y = path->size - 1;
	pixelList[0].x = position;

	for(int i = 1; i < path->size; i++)
	{
		int currentY = path->size - i;
		pixelList[i].y = currentY;				

		int increment = (int) GetValue(pathInfo, position, currentY);
		//printf("position: %i Y: %i", position, currentY);
		//printf("Incre: %i \n" , increment);
		//if(i == path->size - 1)
		//	printf("end \n");
		
		double value = GetValue(pathInfo, position, currentY);		
		position += value;
		pixelList[i].x = position;
	}

	path->pixelList = pixelList;
	return path;
}

int Zooming::SeamCarving::GetMinPosition(IplImage* energyMap)
{
	int width = energyMap->width;			
	int height = energyMap->height;

	double min = double::MaxValue;
	int position;
	for(int i = 0; i < width; i++)
	{
		double value = GetValue(energyMap, i, height - 1);
		if(value < min) 
		{
			min = value;
			position = i;
		}
	}
	return position;
}