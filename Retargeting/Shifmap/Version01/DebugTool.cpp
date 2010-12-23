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

 void DisplayImage(CvMat* mat, char* windowName)
 {
	 double max = 0;
	 double min = 100000;
	 for(int i = 0; i < mat->cols; i++)
		 for(int j = 0; j < mat->rows; j++)
		 {
			 CvScalar value = cvGet2D(mat, j, i);
			 if(value.val[0] > max)
				 max = value.val[0];		
			 if(value.val[0] < min)
				min = value.val[0];			
		 }
	 IplImage* image = cvCreateImage(cvSize(mat->cols, mat->rows), IPL_DEPTH_8U, 1);
	 for(int i = 0; i < mat->cols; i++)
		 for(int j = 0; j < mat->rows; j++)
		 {
			 CvScalar value = cvGet2D(mat, j, i);
			 value.val[0] = 255 * (value.val[0] - min) / max;
			 cvSet2D(image, j, i, value);
		 }
	 DisplayImage(image, windowName);
 }