// FacedetectImg.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

// Include header files
#include "cv.h"
#include "highgui.h"
/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>*/
#include "math.h"
IplImage *Igray=0, *It = 0, *Iat;

void drawImage(IplImage* target, IplImage* source, int x, int y) {
    for (int ix=0; x<source->width; x++) {
        for (int iy=0; y<source->height; y++) {
            int r = source->imageData[ix*3 + source->width * (source->height - iy - 1)*3 + 2];
            int g = source->imageData[ix*3 + source->width * (source->height - iy - 1)*3 + 1];
            int b = source->imageData[ix*3 + source->width * (source->height - iy - 1)*3 + 0];
            target->imageData[(ix+x)*3 + source->width * (source->height - iy + y - 1)*3 + 2] = r;
            target->imageData[(ix+x)*3 + source->width * (source->height - iy + y - 1)*3 + 1] = g;
            target->imageData[(ix+x)*3 + source->width * (source->height - iy + y - 1)*3 + 0] = b;
        }
    }
}

// Main function, defines the entry point for the program.
int main( int argc, char** argv )
{
	IplImage *src, *dst;
	int srcWidth=0, srcHeight=0, dstWidth=0, dstHeight=0;
	if(((src=cvLoadImage(argv[1],1)) != 0)&&((dst=cvLoadImage(argv[2],1)) != 0 ))
	{
		srcWidth = src->width;
		srcHeight = src->height;
		dstWidth = dst->width;
		dstHeight = dst->height;		
	}
	IplImage *combined = cvCreateImage(cvSize(srcWidth+dstWidth,srcHeight), IPL_DEPTH_8U, 3);
	cvCopy(src, combined);

	cvShowImage( "combined", combined);
	cvWaitKey();

	cvReleaseImage(&src);
	cvReleaseImage(&dst);
	cvReleaseImage(&combined);
	//if(argc != 7){return -1; }
	////Command line
	//double threshold = (double)atof(argv[1]);
	//int threshold_type = atoi(argv[2]) ?
	//CV_THRESH_BINARY : CV_THRESH_BINARY_INV;
	//int adaptive_method = atoi(argv[3]) ?
	//CV_ADAPTIVE_THRESH_MEAN_C : CV_ADAPTIVE_THRESH_GAUSSIAN_C;
	//int block_size = atoi(argv[4]);
	//double offset = (double)atof(argv[5]);
	////Read in gray image
	//if((Igray = cvLoadImage( argv[6], CV_LOAD_IMAGE_GRAYSCALE)) == 0){
	//return -1;}
	//// Create the grayscale output images
	//It = cvCreateImage(cvSize(Igray->width,Igray->height),
	//IPL_DEPTH_8U, 1);
	//Iat = cvCreateImage(cvSize(Igray->width,Igray->height),
	//IPL_DEPTH_8U, 1);
	////Threshold
	//cvThreshold(Igray,It,threshold,255,threshold_type);
	//cvAdaptiveThreshold(Igray, Iat, 255, adaptive_method,
	//threshold_type, block_size, offset);
	////PUT UP 2 WINDOWS
	//cvNamedWindow("Raw",1);
	//cvNamedWindow("Threshold",1);
	//cvNamedWindow("Adaptive Threshold",1);
	////Show the results
	//cvShowImage("Raw",Igray);
	//cvShowImage("Threshold",It);
	//cvShowImage("Adaptive Threshold",Iat);
	//cvWaitKey(0);
	////Clean up
	//cvReleaseImage(&Igray);
	//cvReleaseImage(&It);
	//cvReleaseImage(&Iat);
	//cvDestroyWindow("Raw");
	//cvDestroyWindow("Threshold");
	//cvDestroyWindow("Adaptive Threshold");
	return 0;
}