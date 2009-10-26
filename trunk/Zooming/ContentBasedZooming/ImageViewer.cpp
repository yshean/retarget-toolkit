#include "stdafx.h"
#include "ImageViewer.h"

void ImageViewer::PushImage(IplImage *image, char *windowName)
{

	_imageList.push_back(image);
	_windowList.push_back(windowName);
	cvNamedWindow(windowName);
}

void ImageViewer::ShowAllWindows()
{
	int size = _windowList.size();
	for(int i = 0; i < size; i++)
	{
		cvShowImage(_windowList[i], _imageList[i]);
	}
}

void ImageViewer::ShowWindow(char *windowName)
{
}

void ImageViewer::CheckDuplicatedWindow(char* windowName)
{
	int size = _windowList.size();
	for(int i = 0; i < size; i++)
	{
		 
	}
}