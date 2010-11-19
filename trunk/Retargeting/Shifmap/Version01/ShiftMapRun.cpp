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
#include "GCScaleStackShiftMap.h"
#include "HorizontalScaleSM.h"
#include "ScaleSM.h"
#include "FeedbackEnergyFunction.h"
#include <string>
#include <iostream>
#include <fstream>
#include "ConfigFile.h"
#include "Similarity.h"
using namespace std;
#include <vector>
#include <time.h>
// get commands from file

struct AlgoSetting
{
	CvSize outputSize;	 
	char* folder;
	char* inputName;
	char* saliencyName;
	IplImage* input;
	IplImage* saliency; // 0 if no saliency file is specified
	IplImage* targetSample;
	int scaleCount;
	double scaleX;
	double scaleY;
	char* smoothCost;
	char* dataCost;
	int smoothThreshold;
	char* type;
};


AlgoSetting GetAlgoSetting(char* commandFileName)
{
	AlgoSetting algoSetting;
	
	ConfigFile config(commandFileName);
	
	// *****************************************************************
	// SETTING saliency
	int noSaliency;
	int saliencySetting;
	config.readInto(saliencySetting, "saliency");
	if(saliencySetting == 0)
		noSaliency = true;
	else
		noSaliency = false;

	// *****************************************************************
	// SETTING output size	
	config.readInto(algoSetting.outputSize.width, "width");
	config.readInto(algoSetting.outputSize.height, "height");

	// *****************************************************************
	// SETTING input files
	string file;
	config.readInto(file, "file");	
	string folder;
	config.readInto(folder, "folder");	
	string filepath = folder + "/" + file;
	char* filename = (char*)filepath.c_str();
	IplImage* input = cvLoadImage(filename);
	algoSetting.input = input;
	algoSetting.inputName = new char[200];
	algoSetting.folder = new char[200];
	strcpy(algoSetting.folder, (char*)folder.c_str());
	strcpy(algoSetting.inputName, (char*)file.c_str());

	config.readInto(file, "targetSample");
	filepath = folder + "/" + file;
	filename = (char*)filepath.c_str();
	IplImage* targetSample = cvLoadImage(filename);
	algoSetting.targetSample = targetSample;

	// *****************************************************************
	// LOAD saliency
	double maxSal = 0; 
	IplImage* saliency;
	if(noSaliency)
	{
		algoSetting.saliencyName = "none";
		saliency = cvCloneImage(input);
		for(int i = 0; i < saliency->width; i++)
			for(int j = 0; j < saliency->height; j++)
			{
				cvSet2D(saliency, j, i, cvScalar(0,0,0));
			}			
	}
	else
	{
		string saliency_file;
		config.readInto(saliency_file, "saliencyfile");
		string saliency_path = folder+ "/" + saliency_file;
		char* filename_sal = (char*)saliency_path.c_str();			
		algoSetting.saliencyName = new char[200];
		strcpy(algoSetting.saliencyName, (char*)saliency_file.c_str());
		saliency = cvLoadImage(filename_sal);		
	/*	for(int i = 0; i < saliency->width; i++)
			for(int j = 0; j < saliency->height; j++)
			{
				CvScalar value = cvGet2D(saliency, j, i);
				for(int k = 0; k < 4; k++)
				{
					value.val[k] = 255 - value.val[k];
				}
				cvSet2D(saliency, j, i, value);
				if(maxSal < value.val[0] + value.val[1] + value.val[2])
					maxSal = value.val[0] + value.val[1] + value.val[2];
			}*/
	}
	algoSetting.saliency = saliency;
	
	// *****************************************************************
	// SETTING shiftmap type
	string type;
	config.readInto(type, "SmoothCost");
	algoSetting.smoothCost = new char[200];
	strcpy(algoSetting.smoothCost, (char*)type.c_str());
	config.readInto(type, "DataCost");
	algoSetting.dataCost = new char[200];
	strcpy(algoSetting.dataCost, (char*)type.c_str());	config.readInto(type, "SmoothCost");
	algoSetting.type = new char[200];
	strcpy(algoSetting.type, (char*)type.c_str());

	config.readInto(algoSetting.smoothThreshold, "smoothThreshold");
	// *****************************************************************
	// SETTING scale
	int scaleCount;
	double scaleX;
	double scaleY;
	config.readInto(algoSetting.scaleCount, "scaleCount");
	config.readInto(algoSetting.scaleX, "scaleX");
	config.readInto(algoSetting.scaleY, "scaleY");
	
	return algoSetting;
}

string GenerateName(AlgoSetting setting)
{
	string result = "SM";

	result += setting.folder;
	result += setting.inputName;
	result += setting.saliencyName;	
	 
	result += setting.smoothCost;
	result += setting.dataCost;
	char b[100];
	sprintf(b, "%i%i-%4.2f-%4.2f-%i%i", setting.outputSize.width, setting.outputSize.height,
		setting.scaleX, setting.scaleY, setting.scaleCount, setting.smoothThreshold);
	result += b;
	return result;
}
char* GenerateImageName(AlgoSetting setting)
{
	string result = GenerateName(setting);
	result += ".jpg";
	char* filename = new char[200];
	strcpy(filename, (char*)result.c_str());
	return filename;
}
char* GenerateFileName(AlgoSetting setting)
{
	string result = GenerateName(setting);
	result += ".txt";
	char* filename = new char[200];
	strcpy(filename, (char*)result.c_str());
	return filename;
}

void WriteSetting(AlgoSetting setting)
{
	ofstream myfile;

	char* filename = GenerateFileName(setting);
	myfile.open(filename);	
	//myfile.open("SMSource Imagefunnyppl150-156.jpgfunnyppl150-156-Sal-Manual.jpgthresholdorigin1501560.200.00210.txt");
	char b[100];	
	if (myfile.is_open())	
	{
		myfile << "Folder: ";
		myfile << setting.folder;
		myfile << "\n";
		myfile << "Input file: ";
		myfile << setting.inputName;
		myfile << "\n";	
		myfile << "Saliency file: ";
		myfile << setting.saliencyName;
		myfile << "\n";
		myfile << "Width: ";
		myfile << setting.outputSize.width;
		myfile << "\n";
		myfile << "Height: ";
		myfile << setting.outputSize.height;
		myfile << "\n";
		myfile << "Smooth cost: ";
		myfile << setting.smoothCost;
		myfile << "\n";
		myfile << "Data cost: ";
		myfile << setting.dataCost;
		myfile << "\n";
		myfile << "Scale count: ";
		myfile << setting.scaleCount;
		myfile << "\n";
		myfile << "Scale X: ";
		myfile << setting.scaleX;
		myfile << "\n";
		myfile << "Scale Y: ";
		myfile << setting.scaleY;
		myfile << "\n";
		myfile << "Smooth Threshold: ";
		myfile << setting.smoothThreshold;
		myfile << "\n";
		
		myfile.close();
	}
}

vector<char*>* BreakStringList(string listOfString);
void TestMultipleScaleAutoResize(char* commandFileName);
// create zero saliency image
vector<IplImage*>* CreateZeroSaliencyList(vector<IplImage*>* inputList);
// larger image receive more saliency according to size
vector<IplImage*>* CreateAdaptiveSaliencyList(vector<IplImage*>* inputList);
void TestScaleShiftMapConfig(char* commandFileName);
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

void TestScaleShiftMap(char* inputFile1, char* saliencyname1, char* inputFile2, char* saliencyname2, char* outputName, int width, int height)
{	
	IplImage* input = cvLoadImage(inputFile1);
	IplImage* saliency = cvLoadImage(saliencyname1);
	
	// reverse saliency
	for(int i = 0; i < saliency->width; i++)
		for(int j = 0; j < saliency->height; j++)
		{
			CvScalar value = cvGet2D(saliency, j, i);
			value.val[0] = 255 - value.val[0];
			value.val[1] = 255 - value.val[1];
			value.val[2] = 255 - value.val[2];
			cvSet2D(saliency, j, i, value);
		}

	IplImage* input2 = cvLoadImage(inputFile2);
	IplImage* saliency2 = cvLoadImage(saliencyname2);
	
	for(int i = 0; i < saliency2->width; i++)
		for(int j = 0; j < saliency2->height; j++)
		{
			CvScalar value = cvGet2D(saliency2, j, i);
			value.val[0] = 255 - value.val[0];
			value.val[1] = 255 - value.val[1];
			value.val[2] = 255 - value.val[2];
			cvSet2D(saliency2, j, i, value);
		}

	ScaleStackImageSource* imageSource = CreateScaleStackFromList(input, input2);
	ScaleStackImageSource* saliencySource = CreateScaleStackFromList(saliency, saliency2);

	GCScaleStackShiftMap* shiftMap = new GCScaleStackShiftMap();
	// IplImage* image = shiftMap->GetRetargetImage(input, saliency, cvSize(50, 50));
	

	IplImage* image = shiftMap->GetRetargetImage2(input, saliency, imageSource, saliencySource, cvSize(width, height));
	cvSaveImage(outputName, image);
	
	//cvNamedWindow("Res");
	//while(1)
	//{
	//	cvShowImage("Test", image);
	//	cvWaitKey(100);
	//}
}


void TestMultipleScaleHorizontalResize(char* commandFileName)
{
// get command
	//vector<char*>* arguments = GetCommands(commandFileName);
	ConfigFile config(commandFileName);
	
	// SETTING saliency
	int noSaliency;
	int saliencySetting;
	config.readInto(saliencySetting, "saliency");
	if(saliencySetting == 0)
		noSaliency = true;

	// SETTING output size
	int width;
	int height;
	config.readInto(width, "width");
	config.readInto(height, "height");

	// SETTING input files
	string file;
	config.readInto(file, "file");
	char* filename = (char*)file.c_str();
	
	IplImage* input = cvLoadImage(filename);
	// create empty saliency
	IplImage* saliency;
	if(noSaliency)
	{
		saliency = cvCloneImage(input);
		for(int i = 0; i < saliency->width; i++)
			for(int j = 0; j < saliency->height; j++)
			{
				cvSet2D(saliency, j, i, cvScalar(0,0,0));
			}
	}

	int scaleCount;
	double scale;

	config.readInto(scaleCount, "scaleCount");
	config.readInto(scale, "scale");

	HorizontalScaleSM* shiftMap = new HorizontalScaleSM();
	shiftMap->SetScaleSetting(scaleCount, scale);
	shiftMap->ComputeOptimalRetargetMapping(input, saliency, cvSize(width, height));
	IplImage* image = shiftMap->RenderRetargetImage();
	IplImage* visualizedImage = shiftMap->RenderStackMapVisualisation();

	IplImage* visualedSmooth = shiftMap->RenderSmoothCostMapVisualisation();

	// SAVING result
	// creating filename
	string result = "SMFrom-";

	result += filename;

	if(noSaliency)
	{
		result += "-NoSaliency-SmoothMin";
		char b[100];
		sprintf(b, "-Scale %f scaleCount %i-", scale, scaleCount);
		result += b;
	}
	char b[100];
	sprintf(b, "To%i-%i", width, height);
	result+= b;
	


	string comment;
	config.readInto(comment, "comment");
	result += comment;

	string imageNameStr = result + ".jpg";
	char* imageName = (char*)imageNameStr.c_str();
	string visualImageNameStr = result + "-visualized.jpg";
	char* visualImageName = (char*)visualImageNameStr.c_str();
	result += shiftMap->GetCost();
	result += ".jpg";
	string visualSmooth = result + "-smoothVisualed.jpg";
	
	char* visualSmoothName = (char*)visualSmooth.c_str();
	// saving
	cvSaveImage(imageName, image);
	cvSaveImage(visualImageName, visualizedImage);	
	cvSaveImage(visualSmoothName, visualedSmooth);
}
void TestMultipleScaleAutoResize(char* commandFileName)
{
	// get command
	//vector<char*>* arguments = GetCommands(commandFileName);
	ConfigFile config(commandFileName);
	
	// SETTING saliency
	int noSaliency;
	int saliencySetting;
	config.readInto(saliencySetting, "saliency");
	if(saliencySetting == 0)
		noSaliency = true;

	// SETTING output size
	int width;
	int height;
	config.readInto(width, "width");
	config.readInto(height, "height");

	// SETTING input files
	string files;
	config.readInto(files, "files");
	vector<char*>* filesList = BreakStringList(files);	
	vector<IplImage*>* inputList = new vector<IplImage*>();
	for(int i = 0; i < filesList->size(); i++)
	{
		IplImage* image = cvLoadImage((*filesList)[i]);
		inputList->push_back(image);
	}

	// SHIFTMAP SETTING and algorithm
	ScaleStackImageSource* imageSource = CreateScaleStackFromList(inputList);
	ScaleStackImageSource* saliencySource;
	if(noSaliency)
		//saliencySource = CreateScaleStackFromList(CreateZeroSaliencyList(inputList));
		saliencySource = CreateScaleStackFromList(CreateAdaptiveSaliencyList(inputList));

	GCScaleStackShiftMap* shiftMap = new GCScaleStackShiftMap();
	shiftMap->ComputeOptimalRetargetMapping(imageSource, saliencySource, cvSize(width, height));
	IplImage* image = shiftMap->RenderRetargetImage();
	IplImage* visualizedImage = shiftMap->RenderStackMapVisualisation();

	// SAVING result
	// creating filename
	string result = "SMFrom-";

	for(int i = 0; i < filesList->size(); i++)
	{
		result += (*filesList)[i];
		result += "#";
	}
	if(noSaliency)
		result += "-AdaptiveSaliencyScaleby20-";
	char b[100];
	sprintf(b, "To%i-%i", width, height);
	result+= b;
	string imageNameStr = result + ".jpg";
	char* imageName = (char*)imageNameStr.c_str();
	string visualImageNameStr = result + "-visualized.jpg";
	char* visualImageName = (char*)visualImageNameStr.c_str();
	// saving
	cvSaveImage(imageName, image);
	cvSaveImage(visualImageName, visualizedImage);	

}

vector<char*>* BreakStringList(string listOfString)
{
	vector<char*>* stringList = new vector<char*>();
	int current = 0;
	int start;
	int end;
	while(true)
	{
		start = listOfString.find('@', current);
		end = listOfString.find('%', current);
		if(start != string::npos && end != string::npos)
		{
			string subString = listOfString.substr(start + 1, end - start - 1);
			char* test = new char[200];
			strcpy(test, (char*)subString.c_str());
			stringList->push_back(test);
			current = end + 1; // carry on from next char
		}
		else
			break;
	}
	return stringList;
}
vector<IplImage*>* CreateZeroSaliencyList(vector<IplImage*>* inputList)
{
	vector<IplImage*>* saliencyList = new vector<IplImage*>();
	for(int i = 0; i < inputList->size(); i++)
	{
		IplImage* saliency = cvCloneImage((*inputList)[i]);
		for(int i = 0; i < saliency->width; i++)
			for(int j = 0; j < saliency->height; j++)
			{
				cvSet2D(saliency, j, i, cvScalar(0));
			}
		saliencyList->push_back(saliency);
	}
	return saliencyList;
}
vector<IplImage*>* CreateAdaptiveSaliencyList(vector<IplImage*>* inputList)
{
	vector<IplImage*>* saliencyList = new vector<IplImage*>();
	
	// get max size
	CvSize maxSize = cvSize(1,1);
	for(int i = 0; i < inputList->size(); i++)
	{
		IplImage* image = (*inputList)[i];
		if(image->width * image->height > maxSize.width * maxSize.height)
		{
			maxSize.width = image->width;
			maxSize.height = image->height;
		}
	}

	for(int i = 0; i < inputList->size(); i++)
	{
		IplImage* saliency = cvCloneImage((*inputList)[i]);
		double normalizedSaliency = saliency->width * saliency->height / (maxSize.width * maxSize.height);
		for(int i = 0; i < saliency->width; i++)
			for(int j = 0; j < saliency->height; j++)
			{
				cvSet2D(saliency, j, i, cvScalar((1 - normalizedSaliency) * 5));
			}
		saliencyList->push_back(saliency);
	}
	return saliencyList;
}
void TestScaleShiftMapConfig(char* commandFileName)
{ 
	ConfigFile config(commandFileName);
	
	// SETTING saliency
	int noSaliency;
	int saliencySetting;
	config.readInto(saliencySetting, "saliency");
	if(saliencySetting == 0)
		noSaliency = true;
	else
		noSaliency = false;

	// SETTING output size
	int width;
	int height;
	config.readInto(width, "width");
	config.readInto(height, "height");

	// SETTING input files
	string file;
	config.readInto(file, "file");
	
	string folder;
	config.readInto(folder, "folder");	
	string filepath = folder + file;
	char* filename = (char*)filepath.c_str();
	IplImage* input = cvLoadImage(filename);
	
	double maxSal = 0; 

	IplImage* saliency;
	if(noSaliency)
	{
		saliency = cvCloneImage(input);
		for(int i = 0; i < saliency->width; i++)
			for(int j = 0; j < saliency->height; j++)
			{
				cvSet2D(saliency, j, i, cvScalar(0,0,0));
			}
	}
	else
	{
		string saliency_file;
		config.readInto(saliency_file, "saliencyfile");
		string saliency_path = folder+ saliency_file;
		char* filename_sal = (char*)saliency_path.c_str();	
		saliency = cvLoadImage(filename_sal);
		
		for(int i = 0; i < saliency->width; i++)
			for(int j = 0; j < saliency->height; j++)
			{
				CvScalar value = cvGet2D(saliency, j, i);
				for(int k = 0; k < 4; k++)
				{
					value.val[k] = 255 - value.val[k];
				}
				cvSet2D(saliency, j, i, value);
				if(maxSal < value.val[0] + value.val[1] + value.val[2])
					maxSal = value.val[0] + value.val[1] + value.val[2];
			}
	}

	// DisplayImage(saliency, "saliency");

	int scaleCount;
	double scaleX;
	double scaleY;
	config.readInto(scaleCount, "scaleCount");
	config.readInto(scaleX, "scaleX");
	config.readInto(scaleY, "scaleY");

	ScaleSM* shiftMap = new ScaleSM();	 	
	shiftMap->SetScaleSetting(scaleCount, scaleX, scaleY);
	shiftMap->SetMaxSaliency(maxSal);
	
	shiftMap->ComputeOptimalRetargetMapping(input, saliency, cvSize(width, height));
	IplImage* image = shiftMap->RenderRetargetImage();
	IplImage* visualizedImage = shiftMap->RenderStackMapVisualisation();

	//IplImage* visualedSmooth = shiftMap->RenderSmoothCostMapVisualisation();

	// SAVING result
	// creating filename
	string result = "SMFrom-";

	result += file;

	if(noSaliency)
	{
		result += "-NoSaliency-SmoothPatch-DataOrigin-PreventCrossingPatch";
		char b[100];
		sprintf(b, "-ScaleX %f ScaleY %f scaleCount %i-", scaleX, scaleY, scaleCount);
		result += b;
	}
	else
	{
		result += "-ReverseThresSal-SmoothOrigin-DataOrigin-PreventCrossingPatch";
		char b[100];
		sprintf(b, "-ScaleX %f ScaleY %f scaleCount %i-", scaleX, scaleY, scaleCount);
		result += b;
	}

	char b[100];
	sprintf(b, "To%i-%i", width, height);
	result+= b;
	


	string comment;
	config.readInto(comment, "comment");
	result += comment;

	string imageNameStr = result + ".jpg";
	char* imageName = (char*)imageNameStr.c_str();
	string visualImageNameStr = result + "-visualized.jpg";
	char* visualImageName = (char*)visualImageNameStr.c_str();
	// result += shiftMap->GetCost();
	result += ".jpg";
	string visualSmooth = result + "-smoothVisualed.jpg";
	
	char* visualSmoothName = (char*)visualSmooth.c_str();
	// saving
	printf("Saving ");
	printf(imageName);
	printf("\n");
	cvSaveImage(imageName, image);
	cvSaveImage(visualImageName, visualizedImage);	
	//cvSaveImage(visualSmoothName, visualedSmooth);
}



void TestRelaxSmoothCost(char* commandFileName)
{
	AlgoSetting setting = GetAlgoSetting(commandFileName);
	char* output = GenerateImageName(setting);
	char* settingoutput = GenerateFileName(setting);
	WriteSetting(setting);
	ScaleSM* shiftMap = new ScaleSM();	 
	
	if(strcmp(setting.smoothCost, "threshold") == 0)
		shiftMap->SetSmoothThreshold(setting.smoothThreshold);
	
	shiftMap->SetScaleSetting(setting.scaleCount, setting.scaleX, setting.scaleY);
	// shiftMap->SetMaxSaliency(maxSal);
	
	shiftMap->ComputeOptimalRetargetMapping(setting.input, setting.saliency, 
		cvSize(setting.outputSize.width, setting.outputSize.height));
	IplImage* image = shiftMap->RenderRetargetImage();
	IplImage* visualizedImage = shiftMap->RenderStackMapVisualisation();

	cvSaveImage(output, image);
	cvSaveImage("visualized.jpg", visualizedImage);
}

void TestFeedbackRetargeting(char* commandFileName)
{
	AlgoSetting setting = GetAlgoSetting(commandFileName);
	char* output = GenerateImageName(setting);
	char* settingoutput = GenerateFileName(setting);
	WriteSetting(setting);
	ScaleSM* shiftMap = new ScaleSM();	 
	
	if(strcmp(setting.smoothCost, "threshold") == 0)
		shiftMap->SetSmoothThreshold(setting.smoothThreshold);
	
	shiftMap->SetScaleSetting(setting.scaleCount, setting.scaleX, setting.scaleY);
	// shiftMap->SetMaxSaliency(maxSal);
	
	//shiftMap->ComputeOptimalRetargetMapping(setting.input, setting.saliency, 
	//	cvSize(setting.outputSize.width, setting.outputSize.height));
	
	shiftMap->InitGraphCut(setting.input, setting.saliency, 
		cvSize(setting.outputSize.width,setting.outputSize.height));
	shiftMap->ComputeGraphCut();
	
	IplImage* image = shiftMap->RenderRetargetImage();
	IplImage* visualizedImage = shiftMap->RenderStackMapVisualisation();

	
	// feedback part
	shiftMap = new ScaleSM();
	
	IplImage* gradient = cvCloneImage(image);			
	cvSobel(image, gradient, 1, 1);	
	
	shiftMap->SetScaleSetting(1, 0.1, 0.1);

	ScaleLabelMapping* labelMapping = new ScaleLabelMapping();
	CvSize outputSize = cvSize(setting.input->width, setting.input->height);
	labelMapping->InitScaleRange(cvSize(image->width, image->height), 
		outputSize, 1, setting.scaleX, setting.scaleY);	 

	Mapping2D* mapping2D = new Mapping2D();
	mapping2D->InitializeMapping(outputSize.width, outputSize.height);
	mapping2D->IsShift(false);

	IplImage* saliency = cvCloneImage(image);	
	for(int i = 0; i < saliency->width; i++)
		for(int j = 0; j < saliency->height; j++)
		{
			cvSet2D(saliency, j, i, cvScalar(0,0,0));
		}

	FeedbackEnergyFunction* energyFunction = new FeedbackEnergyFunction();	
	energyFunction->SetInput(image, gradient, setting.saliency);
	energyFunction->SetRetargetSize(cvSize(setting.input->width, setting.input->height), setting.outputSize);
	energyFunction->SetLabelMapping(labelMapping);
	energyFunction->SetMapping2D(mapping2D);
	energyFunction->SetTargetSample(setting.input);
	// energyFunction->SetSmoothCostPatchSize(3);
	

	shiftMap->SetEnergyFunction(energyFunction);
	shiftMap->InitGraphCut(image, saliency, 
		outputSize);	
	shiftMap->ComputeGraphCut();
	//
	//// shiftMap->ComputeOptimalRetargetMapping(setting.input, setting.saliency, 
	////	cvSize(setting.outputSize.width, setting.outputSize.height));
	//
	IplImage* fbimage = shiftMap->RenderRetargetImage();
	//IplImage* visualizedImage = shiftMap->RenderStackMapVisualisation();
	//DisplayImage(fbimage, "Test");
	//IplImage* image1 = cvLoadImage("1.jpg");
	//IplImage* image2 = cvLoadImage("2.jpg");
	IplImage* image1 = cvCloneImage(fbimage);
	for(int i = 0; i < image1->width; i++)
		for(int j = 0; j < image1->height; j++)
		{
			CvScalar value1 = cvGet2D(fbimage, j, i);
			CvScalar value2 = cvGet2D(setting.input, j, i);
			int diff = SquareDifference(value1, value2);
			cvSet2D(image1, j, i, cvScalar(diff));
		}

	DisplayImage(image1, "Test");

	//cvSaveImage(output, image);
	
}
void TestAreaRetargeting(char* commandFileName)
{
	AlgoSetting setting = GetAlgoSetting(commandFileName);
	char* output = GenerateImageName(setting);
	char* settingoutput = GenerateFileName(setting);
	WriteSetting(setting);
	ScaleSM* shiftMap = new ScaleSM();	 
	
	if(strcmp(setting.smoothCost, "threshold") == 0)
		shiftMap->SetSmoothThreshold(setting.smoothThreshold);
	 
	shiftMap = new ScaleSM();
	shiftMap->SetScaleSetting(setting.scaleCount, setting.scaleX, setting.scaleY);
	IplImage* gradient = cvCloneImage(setting.input);			
	cvSobel(setting.input, gradient, 1, 1);	

	// new code 19 Nov for warp 
	vector<double>* scaleListX = new vector<double>();
	vector<double>* scaleListY = new vector<double>();
	for(int i = 0; i < setting.scaleCount; i++)
	{
		double scaleX = 1 - i * setting.scaleX;
		double scaleY = 1 - i * setting.scaleY;
		scaleListX->push_back(scaleX);
		scaleListY->push_back(scaleY);
	}
	
	ScaleLabelMapping* labelMapping = new ScaleLabelMapping();	
	//labelMapping->InitScaleRange(cvSize(setting.input->width, setting.input->height),
	//	setting.outputSize, setting.scaleCount, setting.scaleX, setting.scaleY);	 
	labelMapping->InitWarpScaleRange(cvSize(setting.input->width, setting.input->height),
		setting.outputSize, scaleListX, scaleListY);

	Mapping2D* mapping2D = new Mapping2D();
	mapping2D->InitializeMapping(setting.outputSize.width, setting.outputSize.height);
	mapping2D->IsShift(false);

	IplImage* saliency = cvCloneImage(setting.input);	
	for(int i = 0; i < saliency->width; i++)
		for(int j = 0; j < saliency->height; j++)
		{
			cvSet2D(saliency, j, i, cvScalar(0,0,0));
		}

	ScaleEnergyFunction* energyFunction = new ScaleEnergyFunction();
	energyFunction->SetInput(setting.input, gradient, setting.saliency);
	energyFunction->SetRetargetSize(cvSize(setting.input->width, setting.input->height), setting.outputSize);
	energyFunction->SetLabelMapping(labelMapping);
	energyFunction->SetMapping2D(mapping2D);
	energyFunction->SetupAreaCost(1000, 100, 100);
  
	

	shiftMap->SetEnergyFunction(energyFunction);
	shiftMap->SetLabelMapping(labelMapping);
	shiftMap->SetMapping2D(mapping2D);
	shiftMap->InitGraphCut(setting.input, saliency, 
		setting.outputSize);	
	shiftMap->ComputeGraphCut();
	//
	//// shiftMap->ComputeOptimalRetargetMapping(setting.input, setting.saliency, 
	////	cvSize(setting.outputSize.width, setting.outputSize.height));
	//
	IplImage* rtimage = shiftMap->RenderRetargetImage();
	IplImage* viimage = shiftMap->RenderStackMapVisualisation();
	//IplImage* visualizedImage = shiftMap->RenderStackMapVisualisation();
	//DisplayImage(fbimage, "Test");
	//IplImage* image1 = cvLoadImage("1.jpg");
	//IplImage* image2 = cvLoadImage("2.jpg");
	//IplImage* image1 = cvCloneImage(fbimage);
	//for(int i = 0; i < image1->width; i++)
	//	for(int j = 0; j < image1->height; j++)
	//	{
	//		CvScalar value1 = cvGet2D(fbimage, j, i);
	//		CvScalar value2 = cvGet2D(setting.input, j, i);
	//		int diff = SquareDifference(value1, value2);
	//		cvSet2D(image1, j, i, cvScalar(diff));
	//	}

	DisplayImage(rtimage, "Test");
  	DisplayImage(viimage, "Test2");
	//cvSaveImage(output, image);
	
}
int main(int argc, char* argv[])
{
	// ConfigFile config( "example.txt" );
	//TestMultipleScaleAutoResize("command.txt");
	 //TestMultipleScaleHorizontalResize("command.txt");
	// TestScaleShiftMapConfig("command.txt");
	//TestRelaxSmoothCost("command.txt");
	// TestFeedbackRetargeting("command.txt");
	TestAreaRetargeting("command.txt");
	//IplImage* image = cvLoadImage("building.jpg");

	//IplImage* result =  GetScaleSimilar(image, 0.5);	

	//DisplayImage(result, "Result");
	//char* input_name1 = (*arguments)[0];
	//char* saliency1 = (*arguments)[1];
	//char* input_name2 = (*arguments)[2];
	//char* saliency2 = (*arguments)[3];
	//char* output_name = (*arguments)[4];
	//int width = atoi((*arguments)[5]);
	//int height = atoi((*arguments)[6]);
	//TestScaleShiftMap(input_name1, saliency1, input_name2, saliency2, output_name, width, height);
	//TestHierarchyFHShiftMap2();
	//TestNewFillHoleShiftMap();
	//TestHierarchyFHShiftMap();
	//TestFillHoleShiftMap();
	//TestShiftMap();
	//TestHierarchyShiftMap();
}