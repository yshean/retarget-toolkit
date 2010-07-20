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
 
	
	
	IplImage* input = cvLoadImage("boatman.jpg");
	IplImage* saliency = cvLoadImage("boatmanS.jpg");
	ShiftMapComputer* computer = new ShiftMapComputer();
	computer->ComputeShiftMap(input, saliency, cvSize(54,54), cvSize(54,54));
	IplImage* output = computer->GetRetargetImage();
	//IplImage* input = cvLoadImage("a.jpg");
	//IplImage* saliency = cvLoadImage("aS.jpg");
	//IplImage* output = computer->GetImage(input, saliency, 10, 10);

	cvNamedWindow("test");
	while(1)
	{
		cvShowImage("test", output);
		cvWaitKey(100);
	}
	return 0;
}

