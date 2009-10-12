// VideoMontage.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cv.h>
#include <highgui.h>
#include <list>




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

void PlayMatrix3D(Matrix3D* matrix)
{
	int length = matrix->_length;
	cvNamedWindow("Matrix3D");

	IplImage* image = cvCreateImage(cvSize(matrix->_width, matrix->_height), IPL_DEPTH_8U, matrix->_channel);
	
	for(int i = 0; i < length; i++)
	{
		matrix->GetIplImageZ(i, image);
		cvShowImage("Matrix3D", image);
		int key = cvWaitKey(200);				 
		if (key == 27) break;
	}
}

Matrix3D* LoadMatrix3D()
{
	int length = 16;
	char filename[100];
	 
	sprintf(filename, "C:/Documents and Settings/minhchau/My Documents/Visual Studio 2005/Projects/VideoMontage/%i.jpg",1);
	IplImage* image = cvLoadImage(filename);
	IplImage* image2 = cvCloneImage(image);
	
	
	Matrix3DChar* matrix = new Matrix3DChar(image->width, image->height, length, image->nChannels);
	matrix->SetIplImageZ(0, image);
	cvReleaseImage(&image);

	cvNamedWindow("Load");

	for(int i = 2; i <= length; i++)
	{
		sprintf(filename, "C:/Documents and Settings/minhchau/My Documents/Visual Studio 2005/Projects/VideoMontage/%i.jpg",i);
		image = cvLoadImage(filename);		 
		matrix->SetIplImageZ(i - 1, image);
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
	Matrix3D* matrix = LoadMatrix3D();
	PlayMatrix3D(matrix);
}

#pragma endregion

#pragma region Scaling
#include "Scaling.h"

void TestLoadThenScale()
{
	Matrix3D* matrix = LoadMatrix3D();
	Scaling* scaling = new Scaling();
	Matrix3D* output = new Matrix3DChar(matrix->_width / 2, matrix->_height / 2, matrix->_length, matrix->_channel);

	scaling->PyrDown(matrix, output);
	PlayMatrix3D(output);
}

#pragma endregion Scaling

#pragma endregion Matrix3D



#pragma endregion Unit Testing

int _tmain(int argc, _TCHAR* argv[])
{
	
	// LoadVolume3D();
	// TestLoadAndPlay();
	TestLoadThenScale();

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


