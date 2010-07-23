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
	
	
	
 

	
	IplImage* input = cvLoadImage("boatman.jpg");
	IplImage* saliency = cvLoadImage("boatmanCS.JPG");
  	// reverse the saliency
	for(int i = 0; i < saliency->width; i++)
		for(int j = 0; j < saliency->height; j++)
		{
			CvScalar value = cvGet2D(saliency, j, i);
			value.val[0] = 255 - value.val[0];
			value.val[1] = 255 - value.val[1];
			value.val[2] = 255 - value.val[2];
			cvSet2D(saliency, j, i, value);
		}
	//cvNamedWindow("Test");
	//while(1)
	//{
	//	cvShowImage("Test", saliency);
	//	cvWaitKey(100);
	//}
	CvSize outputSize = cvSize(230, 180);
 
	ShiftMapComputer* computer = new ShiftMapComputer();
 
	//computer->ComputeShiftMap(input, saliency, outputSize, outputSize);
	computer->ComputeFastShiftMap(input, saliency, outputSize);
	CvMat* labelMap = computer->CalculateLabelMap();
	IplImage* map = cvCreateImage(cvSize(labelMap->width, labelMap->height), IPL_DEPTH_8U, 3);
	IplImage* source = cvCreateImage(cvSize(input->width, input->height), IPL_DEPTH_8U, 3);

	/*ShiftMapVisualizer* visualizer = new ShiftMapVisualizer();
	visualizer->Visualize(labelMap, source, map, cvSize(45,45));*/
	//IplImage* labelMap1 = computer->GetLabelMap(1);
	//IplImage* origin1 = computer->GetOriginalImage(1);	
	//IplImage* map1 = cvCreateImage(cvSize(labelMap1->width, labelMap1->height), IPL_DEPTH_8U, 3);
	//IplImage* source1 = cvCreateImage(cvSize(origin1->width, origin1->height), IPL_DEPTH_8U, 3);	
	//IplImage* output1 = computer->GetRetargetImage(1);
	//visualizer->Visualize(labelMap1, source1, map1);

	//IplImage* labelMap0 = computer->GetLabelMap(0);
	//IplImage* origin0 = computer->GetOriginalImage(0);	
	//IplImage* map0 = cvCreateImage(cvSize(labelMap0->width, labelMap0->height), IPL_DEPTH_8U, 3);
	//IplImage* source0 = cvCreateImage(cvSize(origin0->width, origin0->height), IPL_DEPTH_8U, 3);	
	//IplImage* output0 = computer->GetRetargetImage(0);
	//visualizer->Visualize(labelMap0, source0, map0);
	 
	cvNamedWindow("Source");
	cvNamedWindow("Map");
	cvNamedWindow("Result");	
	//cvNamedWindow("Source0");
	//cvNamedWindow("Map0");
	//cvNamedWindow("Result0");	
	
	//cvSaveImage("boatman4054.jpg", output0);
	
	//IplImage* result = computer->CalculateRetargetImage();
	////

	//while(1)
	//{
	//	cvShowImage("Result", result);
	//	cvShowImage("Source", source);
	//	cvShowImage("Map", map);
	//	cvWaitKey(100);
	//}
	//
	
	int index = computer->GetLevelCount();
	vector<IplImage*>* result = new vector<IplImage*>(0);
	for(int i = 0; i <= index;i++)
	{
		IplImage* image = computer->GetRetargetImage(i);
		result->push_back(image);
	}
	
	while(1)
	{

		cvShowImage("Source", (*(computer->_imageList))[index]);
		cvShowImage("Result", (*result)[index]);
		//cvShowImage("Source1", source1);
		//cvShowImage("Map1", map1);
		//cvShowImage("Result1", output1);
		//cvShowImage("Source0", source0);
		//cvShowImage("Map0", map0);
		//cvShowImage("Result0", output0);

		int key = cvWaitKey(100);
		if(key == 32)
		{
			if(index == 0)
				index = computer->GetLevelCount();
			else
				index--;
		}
	}
	return 0;


}