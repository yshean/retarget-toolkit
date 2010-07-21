// ShiftMap.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Label.h"
#include "ShiftMapComputer.h"
#include <cv.h>
#include <highgui.h>
#include "example.h"

int _tmain(int argc, _TCHAR* argv[])
{
	// GridGraph_DfnSfn(10, 10, 100, 20);
	
		CvPoint point;
	point.x = 0;
	point.y = 1;
	point = GetPoint(50, 25, 10);
 
	
	
	IplImage* input = cvLoadImage("boatman3.jpg");
	IplImage* saliency = cvLoadImage("boatman3S.jpg");
	ShiftMapComputer* computer = new ShiftMapComputer();
	

	CvSize outputSize;
	outputSize.width = 50;
	outputSize.height = 54;
		
	// initial guess map to same position
	// this for the case applying gc to all image
	// not just few labels
	IplImage* initialGuess = cvCreateImage(outputSize, IPL_DEPTH_8U, 3);
	for(int i = 0; i < outputSize.width; i++)
		for(int j = 0; j < outputSize.height; j++)
		{
			SetLabel(cvPoint(i,j), cvPoint(i,j), initialGuess);
		}

	//////	computer->ComputeShiftMap(input, saliency, initialGuess, outputSize, cvSize(input->width, input->height));
	//////IplImage* output = computer->GetRetargetImage();
	//IplImage* input = cvLoadImage("a.jpg");
	//IplImage* saliency = cvLoadImage("aS.jpg");
	//IplImage* output = computer->GetImage(input, saliency, 10, 10);
	
		
	computer->ComputeShiftMap(input, saliency, initialGuess, outputSize, cvSize(54,54));
	//computer->ComputeFastShiftMap(input, saliency, outputSize);
	IplImage* output = computer->GetRetargetImage();
	cvNamedWindow("test");
	while(1)
	{
		cvShowImage("test", output);
		cvWaitKey(100);
	}
	return 0;
}

