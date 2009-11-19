// VideoMontage.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cv.h>
#include <highgui.h>
#include <list>

#include "VideoCollage.h"

#include "NormL2.h"
#include "ColorConverter.h"
#include "Blur.h"
#include "median.h"

#include "VideoSequence.h"

using namespace std;

#pragma region Unit Testing

#pragma region Volume3D
#include "Volume3D.h"
// scanning through a volume3D and display its
void PlayVolume3D(Volume3D* volume)
{
	int length = volume->_length;
	cvNamedWindow("Volume3D");

	for(int i = 0; i < length; i++)
	{
		IplImage* image = volume->GetFrame(i);
		cvShowImage("Volume3D", image);
		int key = cvWaitKey(200);
		if (key == 27) break;
	}
}

// loading a volume3D then play it
void LoadVolume3D()
{
	int length = 16;
	Volume3D* volume = new Volume3D(16);

	char filename[100];
	 
	for(int i = 1; i <= length; i++)
	{
		sprintf(filename, "C:/Documents and Settings/minhchau/My Documents/Visual Studio 2005/Projects/VideoMontage/%i.jpg",i);
		IplImage* image = cvLoadImage(filename);		 
		// frame start at 0
		volume->AssignFrame(image, i - 1);			
	} 

	PlayVolume3D(volume);
}
#pragma endregion

#pragma region Matrix3D
#include "Matrix3D.h"

#pragma region Save and Load
void SaveMatrix3D(Matrix3D* matrix, char* name)
{
	char filename[100];
	char num[20];
	
	IplImage* image = cvCreateImage(cvSize(matrix->_width, matrix->_height), matrix->_type, matrix->_channel);
	IplImage* image2 = cvCreateImage(cvSize(matrix->_width, matrix->_height), IPL_DEPTH_8U, matrix->_channel);

	for(int i = 0; i < matrix->_length; i++)
	{
		sprintf(num, "%i.jpg", i);
		strcpy(filename, name);
		strcat(filename, num);
		matrix->GetIplImageZ(i, image);		
		cvCvtScale(image, image2);
		cvSaveImage(filename, image2);
	}
}

void PlayMatrix3D(Matrix3D* matrix, int interval)
{
	printf("Playing ... ");
	int length = matrix->_length;
	cvNamedWindow("Matrix3D");

	IplImage* image = cvCreateImage(cvSize(matrix->_width, matrix->_height), IPL_DEPTH_8U, matrix->_channel);
	
	for(int i = 0; i < length; i++)
	{
		printf("Displaying frame number: %i \n", i);
		matrix->GetIplImageZ(i, image);
		cvShowImage("Matrix3D", image);
		int key = cvWaitKey(interval);				 
		if (key == 27) break;
	}
}

Matrix3D* LoadMatrix3D(int start, int end)
{
	char filename[100];
	 
	sprintf(filename, "C:/Documents and Settings/minhchau/My Documents/Visual Studio 2005/Projects/VideoMontage/%i.jpg",start);
	IplImage* image = cvLoadImage(filename, 1);
	IplImage* image2 = cvCloneImage(image);
	
	printf("Loading... \n");
	
	int length = end - start + 1;

	Matrix3DChar* matrix = new Matrix3DChar(image->width, image->height, length, image->nChannels);
	matrix->SetIplImageZ(0, image);
	cvReleaseImage(&image);

	cvNamedWindow("Load");
	
	

	for(int i = start + 1; i <= end; i++)
	{
		sprintf(filename, "C:/Documents and Settings/minhchau/My Documents/Visual Studio 2005/Projects/VideoMontage/%i.jpg",i);
		image = cvLoadImage(filename, 1);		 
		matrix->SetIplImageZ(i - start, image);
		image2 = cvCloneImage(image);
		//matrix->GetIplImageZ(i - 1, image2);
		cvShowImage("Load", image2);
		cvWaitKey(100);
		cvReleaseImage(&image);
	} 

	return matrix;
}

void TestLoadAndPlay()
{
	Matrix3D* matrix = LoadMatrix3D(1,14);
	PlayMatrix3D(matrix, 200);
}

#pragma endregion

#pragma region Scaling
#include "Scaling.h"

void TestLoadThenScale()
{
	Matrix3D* matrix = LoadMatrix3D(1,15);
	Scaling* scaling = new Scaling();
	
	// scale in width and height
	//Matrix3D* output = new Matrix3DChar(matrix->_width, matrix->_height / 2, matrix->_length/2, matrix->_channel);
	//scaling->PyrDownZ(matrix, output);
	//PlayMatrix3D(output);
	
	// scale in height and temporal
	Matrix3D* output = new Matrix3DChar(matrix->_width, matrix->_height / 2, matrix->_length/2, matrix->_channel);
	scaling->PyrDownX(matrix, output);
	PlayMatrix3D(output, 1000);
}

#pragma endregion Scaling

#pragma region Saliency 
#include "MontageSaliencyMap.h"
#include "SobelSaliencyMap.h"


// calculate the diff of a pixel with its neighbors
void TestCalculateDiffImage()
{
	Matrix3D* matrix = LoadMatrix3D(1,8);

	IplImage* image = cvCreateImage(cvSize(matrix->_width, matrix->_height), IPL_DEPTH_8U, 3);
	//IplImage* image2 = cvCreateImage(cvSize(20,20), IPL_DEPTH_8U, 3);
	//cvSmooth(image, image);
	
	cvSobel(image, image, 1, 1);
	
	Matrix3D* matrix_small = new Matrix3DChar(matrix->_width/2, matrix->_height/2, matrix->_length, 3);
	
	Matrix3D* matrix_small2 = new Matrix3DChar(matrix->_width/4, matrix->_height/4, matrix->_length, 3);
	
	Matrix3D* matrix_smooth = new Matrix3DChar(matrix->_width/4, matrix->_height/4, matrix->_length, 3);
	
	Matrix3D* matrix_saliency = new Matrix3DFloat(matrix->_width/4, matrix->_height/4, matrix->_length, 1);
	
	Scaling* scale = new Scaling();
	scale->PyrDownZ(matrix, matrix_small);
	scale->PyrDownZ(matrix_small, matrix_small2);
	
	
	// convert to LUV
	ColorConverter* colorConverter = new ColorConverter();
	colorConverter->ConvertRGB2LUV(matrix_small2, matrix_small2);

	PlayMatrix3D(matrix_small2, 400);
	
	Blur* blur = new Blur();
	blur->DoBlur(matrix_small2, matrix_smooth);

	MontageSaliencySetting setting;
	setting.norm = new NormL2();
	setting.scaling = scale;
	SaliencyMap* saliency = new MontageSaliencyMap(&setting);
	//SaliencyMap* saliency = new SobelSaliencyMap();
	saliency->CalculateSaliencyMap(matrix_small2, matrix_saliency);
	
	SaveMatrix3D(matrix_saliency, "MontageSaliencyBlock2");
	PlayMatrix3D(matrix_saliency, 1000);
}

#pragma endregion Saliency

#pragma endregion Matrix3D

#pragma region ImageMergeCarving
#include "RegionExtractor.h"
#include "ImageMergeCarving.h"
#pragma endregion

#pragma region Background Extractor
#include "MedianBackgroundExtractor.h"
void TestBackGroundExtractor()
{
	Median* median = new Median();
	MedianBackgroundExtractor* extractor = new MedianBackgroundExtractor(median);

	Matrix3D* matrix = LoadMatrix3D(63, 79);
	ColorConverter* converter = new ColorConverter();
	Matrix3D* gray_matrix = new Matrix3DInt(matrix->_width, matrix->_height, matrix->_length, 1);
	converter->ConvertRGB2GRAY(matrix, gray_matrix);

	PlayMatrix3D(gray_matrix, 500);
	IplImage* bkgrnd = extractor->ExtractBackground(gray_matrix);
	

	cvNamedWindow("Background");
	while(1)
	{
		cvShowImage("Background", bkgrnd);
		cvWaitKey(100);
	}
}

#pragma endregion

#pragma region Frequency-Tuned Saliency
#include "Saliency.h"
#include "FrequencyTunedSaliency.h"

void TestFrequencyTunedSaliency()
{
	Saliency sal;
	IplImage* image = cvLoadImage("test.jpg");
	int width = image->width;
	int height = image->height;
	// convert image to vector
	vector<unsigned int> input(height * width * 4);

	for(int x = 0; x < image->width; x++)
		for(int y = 0; y < image->height; y++)
		{
			CvScalar value = cvGet2D(image, y, x);
			input[y * width + x] = 0;
			input[y * width + x + 1] = value.val[0];
			input[y * width + x + 2] = value.val[1];
			input[y * width + x + 3] = value.val[2];
		}

	// detect saliency
	vector<double> saliency(0);
	
	sal.GetSaliencyMap(input, image->width, image->height, saliency);

	int size = saliency.size();
	// convert back to image
	IplImage* saliency_image = cvCreateImage(cvSize(image->width, image->height), IPL_DEPTH_32F, 1);
	for(int x = 0; x < image->width; x++)
		for(int y = 0; y < image->height; y++)
		{
			cvSet2D(saliency_image, y, x, cvScalar(saliency[y * image->width + x]));
		}
	
	// display image
	cvNamedWindow("FrequencySaliency");
	cvNamedWindow("Original");
	while(1)
	{
		cvShowImage("FrequencySaliency", saliency_image);
		cvShowImage("Original", image);
		cvWaitKey(100);
	}
}


#pragma endregion

#pragma region Shot Info 
#include "VideoSequence.h"
#pragma endregion

#pragma region ROI Blending
#include "ROIBlend.h"
#pragma endregion
#pragma endregion Unit Testing

//
int _tmain(int argc, _TCHAR* argv[])
{
	
	// LoadVolume3D();
	// TestLoadAndPlay();
	// TestLoadThenScale();
	//TestCalculateDiffImage();
	
	//TestRegionExtractor();
	//TestImageMergeCarving();
	//TestBackGroundExtractor();
	//TestFrequencyTunedSaliency();
	//TestFrequencyTunedSaliencyClass();
	//TestCreateSequence();
	//TestSaveLoadShot();

	//*********** ROIBlend ************
	//char* image[3];
	//image[0] = "test1.jpg";
	//image[1] = "test2.jpg";
	//image[2] = "test3.jpg";
	//TestROIBlend(3, image );
	//TestCombineImages();
	TestBlendImages();
	//*********** ROIBlend END ************


	//IplImage* frame;
	//long* begin;
	//begin = (long*)malloc(10 * sizeof(long));
	//long* iter = begin; 

	//for(int i = 0; i < 10; i++)
	//{
	//	frame = cvLoadImage("17033.jpg");
	//	*iter = (long)frame;
	//	iter++;
	//}
	//
	//// move pointer to begining
	//iter = begin;
	// 
	//cvNamedWindow("Image");
	//for(int i = 0; i < 10; i++)
	//{
	//	cvShowImage("Image", (IplImage*)(iter[i]));		 
 //
	//	printf("Number: %i", iter[i]);
	//	cvWaitKey(1000);
	//}
	//return 0;
}
//

