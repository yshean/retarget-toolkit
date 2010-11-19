#include "StdAfx.h"
#include "DebugTool.h"
 void DisplayImage(IplImage* image, char* windowName)
 {
	 cvNamedWindow(windowName);
	 while(1)
	 {
		 cvShowImage(windowName, image);
		 int key = cvWaitKey(100);
		 if(key == 32)
			 break;
	 }
 }