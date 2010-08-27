// ShiftMap.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Label.h"
#include "ShiftMap.h"
#include "ShiftMapHierarchy.h"
#include "ShiftMapCoarse.h"
#include <cv.h>
#include <highgui.h>
#include "example.h"
#include "ShiftMapVisualizer.h"
#include "ClusterShiftMap.h"
#include "FillHoleShiftMap.h"
#include "HierarchyFHShiftMap.h"
#include "MaskShift.h"
#include <time.h>

void TestShiftMap()
{	
	time_t start, end;
	time(&start);
	 
	IplImage* input = cvLoadImage("boatman40.jpg");
	IplImage* saliency = cvLoadImage("boatmanCS40.jpg");

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
	CvSize outputSize = cvSize(32, 40);
	ShiftMap* computer = new ShiftMap();
	IplImage* result = computer->GetRetargetImage(input, saliency, outputSize)	;

	time(&end);
	double dif = difftime (end,start);	
	printf ("It took you %.2lf seconds.\n", dif );

	cvNamedWindow("Result");
	while(1)
	{
		cvShowImage("Result", result);
		cvWaitKey(100);
	}

}
void TestHierarchyShiftMap()
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
 
	ShiftMapHierarchy* computer = new ShiftMapHierarchy();
	 
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
		cvSaveImage("boatman180180to230180blur5x5.jpg", (*result)[index]);
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
	
}
void TestCoarseShiftMap()
{
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

	ShiftMap* computer = new ShiftMapCoarse();
	IplImage* result = computer->GetRetargetImage(input, saliency, cvSize(150,180));

	cvNamedWindow("Result");
	while(1)
	{
		cvShowImage("Result", result);
		cvWaitKey(100);
	}
}

void TestClusterShiftMap()
{
	IplImage* mask = cvLoadImage("boatmanmask.jpg");
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

	ClusterShiftMap* shmap = new ClusterShiftMap();
	shmap->ComputeShiftMap(input, saliency, mask, cvSize(mask->width, mask->height), cvSize(input->width, input->height));
}

void TestFillHoleShiftMap()
{
	time_t start, end;
	time(&start);

	//IplImage* maskData = cvLoadImage("boatmanOutput75.jpg");
	//IplImage* mask = cvLoadImage("boatmanOutputMask75.jpg");
	//IplImage* input = cvLoadImage("boatman75.jpg");
	//IplImage* saliency = cvLoadImage("boatmanCS75.jpg");
	IplImage* maskData = cvLoadImage("testMaskData.jpg");
	IplImage* mask = cvLoadImage("testMask.jpg");
	IplImage* input = cvLoadImage("test.jpg");
	IplImage* saliency = cvLoadImage("testS.jpg");


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
	
	FillHoleShiftMap* shiftMap = new FillHoleShiftMap();
	shiftMap->SetMask(mask, maskData);	
	IplImage* result = shiftMap->GetRetargetImage(input, saliency, cvSize(maskData->width, mask->height));
	
	time(&end);
	double dif = difftime (end,start);	
	printf ("It took you %.2lf seconds.\n", dif );

	cvNamedWindow("Result");
	while(1)
	{
		cvShowImage("Result", result);
		cvWaitKey(100);
	}
}

void TestHierarchyFHShiftMap()
{
	IplImage* input = cvLoadImage("boatman.jpg");
	IplImage* saliency = cvLoadImage("boatmanCS.JPG");
	IplImage* mask = cvLoadImage("boatmanMask.jpg");
	IplImage* maskData = cvLoadImage("boatmanMaskData.jpg");
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
 
	CvSize outputSize = cvSize(150, 180);
 
	HierarchyFHShiftMap* computer = new HierarchyFHShiftMap();
	computer->SetMask(mask, maskData);
	computer->ComputeFastShiftMap(input, saliency);
	
	
	cvNamedWindow("Result");
	vector<IplImage*>* result = new vector<IplImage*>();

	
	int levelCount = computer->GetLevelCount();
	int index = levelCount - 1;

	for(int i = 0; i < levelCount; i++)
	{
		result->push_back(computer->GetRetargetedImage(i));
	}
	while(1)
	{
		 
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
				index = computer->GetLevelCount() - 1;
			else
				index--;
		}
	}
}

void TestHierarchyFHShiftMap2()
{
	IplImage* input = cvLoadImage("boatman.jpg");
	IplImage* saliency = cvLoadImage("boatmanCS.JPG");
	IplImage* mask = cvLoadImage("boatmanInputMask.jpg");
	
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
 
	CvSize outputSize = cvSize(150, 180);
 
	HierarchyFHShiftMap* computer = new HierarchyFHShiftMap();
	// computer->SetMask(mask, maskData);
	computer->ComputeFastShiftMap2(input, saliency, mask, cvPoint(20,0), outputSize);
	
	
	cvNamedWindow("Result");
	vector<IplImage*>* result = new vector<IplImage*>();

	
	int levelCount = computer->GetLevelCount();
	int index = levelCount - 1;

	for(int i = 0; i < levelCount; i++)
	{
		result->push_back(computer->GetRetargetedImage(i));
	}
	while(1)
	{
		 
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
				index = computer->GetLevelCount() - 1;
			else
				index--;
		}
	}
}
void TestNewFillHoleShiftMap()
{
	IplImage* maskData = cvLoadImage("testMaskData.jpg");
	IplImage* mask = cvLoadImage("boatmanmaskinput40.jpg");
	IplImage* input = cvLoadImage("boatman40.jpg");
	IplImage* saliency = cvLoadImage("boatmanCS40.jpg");

	MaskShift* maskShift = CreateMaskShift(input, mask, cvPoint(5, 0), cvSize(35,40));
	
	FillHoleShiftMap* computer = new FillHoleShiftMap();
	computer->SetMask(maskShift->_mask, maskShift->_maskData);
	IplImage* image = computer->GetRetargetImage(input, saliency, cvSize(35,40));
	cvNamedWindow("Test");
	while(1)
	{
		cvShowImage("Test", image);
		cvWaitKey(100);
	}
}
int _tmain(int argc, _TCHAR* argv[])
{

	TestHierarchyFHShiftMap2();
	//TestNewFillHoleShiftMap();
	//TestHierarchyFHShiftMap();
	//TestFillHoleShiftMap();
	//TestShiftMap();
	//TestHierarchyShiftMap();
}