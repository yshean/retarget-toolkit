// VideoMontage.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cv.h>
#include <highgui.h>
#include <list>




using namespace std;

#pragma region unit testing


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

int _tmain(int argc, _TCHAR* argv[])
{
	
	LoadVolume3D();
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


