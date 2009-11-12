#pragma once
#include "stdafx.h"

// Include header files
#include "cv.h"
#include "highgui.h"
#include <math.h>
#include "CommonMath.h"

// refer to paper: "Video Collage - a novel presentation of video sequence" for more details
public class ROIBlend
{
public:

IplImage *Igray, *It, *Iat;

/*
Usage: 
1) Load n images of type IplImage (n<=12)
2) Call cvBlendImages passing the image variable as the parameter
3) Needs implementation: Writing the Gaussian value of the pixel to the image
4) Show the images through the cvShowManyImages passing the new image files as the parameter
*/

void cvShowManyImages(char* title, int nArgs, ...);


public:
	// calculate integral of function in calcFsofP from a to b
	double calcDefiniteIntergral(int mean, int variance, int a, int b);
private:
	// calculate Gaussian distribution fs(p).
	double calcFsofP(int p, int mean, int variance);
public:
	// blend 2 images - assumed to be of the SAME SIZE
	void BlendImages(IplImage* image1, IplImage* image2);

	void cvBlendImages(IplImage *blend1);
};

int TestROIBlend( int argc, char** argv );