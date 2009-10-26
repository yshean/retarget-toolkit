#pragma once
#include <cv.h>
#include <highgui.h>
#include <vector>
using namespace std;

// this class provide a easy solution to display image
public class ImageViewer
{
public:
	ImageViewer()
	{
		//_imageList = new vector<IplImage*>();
		//_windowList = new vector<char*>();
	}

	~ImageViewer()
	{
		printf("Destroy");
	}

private:
	vector<IplImage*> _imageList;
	vector<char*> _windowList;
private:
	void CheckDuplicatedWindow(char* windowName);
public:
	// push an image to list, an window will be created to display it
	void PushImage(IplImage* image, char* windowName);	

	void ShowWindow(char* windowName);

	// show all windows
	void ShowAllWindows();

	
};