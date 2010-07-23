// ShiftMap.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Label.h"
#include "ShiftMapComputer.h"
#include <cv.h>
#include <highgui.h>
#include "example.h"
#include "ShiftMapVisualizer.h"

int _tmain(int argc, _TCHAR* argv[])
{
	// GridGraph_DfnSfn(10, 10, 100, 20);
	
	
	
 

	
	IplImage* input = cvLoadImage("boatman3.jpg");
	IplImage* saliency = cvLoadImage("boatman3S.JPG");
  
 
 
	ShiftMapComputer* computer = new ShiftMapComputer();
 
	computer->ComputeShiftMap(input, saliency, cvSize(40, 54), cvSize(54, 54));

	IplImage* labelMap = computer->GetLabelMap();

	ShiftMapVisualizer* visualizer = new ShiftMapVisualizer();

	IplImage* map = cvCreateImage(cvSize(labelMap->width, labelMap->height), IPL_DEPTH_8U, 3);
	IplImage* source = cvCreateImage(cvSize(input->width, input->height), IPL_DEPTH_8U, 3);
	visualizer->Visualize(labelMap, source, map);

	// test smooth energy
	int energy = visualizer->ComputeEnergy(labelMap);
	printf("energy: %i" ,energy);

	IplImage* output = computer->GetRetargetImage();
 
	

	CvSize outputSize;
	outputSize.width = 50;
	outputSize.height = 54;
		
	// initial guess map to same position
	// this for the case applying gc to all image
	// not just few labels
	//IplImage* initialGuess = cvCreateImage(outputSize, IPL_DEPTH_8U, 3);
	//for(int i = 0; i < outputSize.width; i++)
	//	for(int j = 0; j < outputSize.height; j++)
	//	{
	//		SetLabel(cvPoint(i,j), cvPoint(i,j), initialGuess);
	//	}

	//////	computer->ComputeShiftMap(input, saliency, initialGuess, outputSize, cvSize(input->width, input->height));
	//////IplImage* output = computer->GetRetargetImage();
 
	//IplImage* input = cvLoadImage("a.jpg");
	//IplImage* saliency = cvLoadImage("aS.jpg");
	//IplImage* output = computer->GetImage(input, saliency, 10, 10);
 
	cvNamedWindow("Source");
	cvNamedWindow("Map");
	cvNamedWindow("Result");	
		
 
	//computer->ComputeFastShiftMap(input, saliency, outputSize);
  
 
	while(1)
	{
		cvShowImage("Source", source);
		cvShowImage("Map", map);
		cvShowImage("Result", output);
		cvWaitKey(100);
	}
	return 0;


}