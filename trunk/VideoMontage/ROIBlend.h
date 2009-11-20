#pragma once
#include "stdafx.h"

// Include header files
#include "cv.h"
#include "highgui.h"
#include <math.h>
#include "CommonMath.h"
 
/*
Usage: 
1) Load n images of type IplImage (n<=12)
2) Call cvBlendImages passing the image variable as the parameter
3) Needs implementation: Writing the Gaussian value of the pixel to the image
4) Show the images through the cvShowManyImages passing the new image files as the parameter
*/

// refer to paper: "Video Collage - a novel presentation of video sequence" for more details
public class __declspec(dllexport) ROIBlend
{
public:
	// scale: scale for calculating Gaussian mean 
	// variance: variance of the Gaussian mean 
	ROIBlend(double scale, double variance) 
	{
		_scale = scale;
		_variance = variance; 
	}

	// new implementation does not need any parameter
	ROIBlend()
	{
	}
protected:
	double _scale;
	double _variance;
	int _a;				// position of edge after combining 2 images - will be updated and BlendImages is called
	int _b;				// position of edge after combining 2 images
public:
	void cvShowManyImages(char* title, int nArgs, ...);
public:
	// calculate blending value
	double calcBlendingValue(int p);
	// calculate integral of function in calcFsofP from start to end
	// actually is the result of Ps(p)	
	double calcDefiniteIntergral(int p, int start, int end);
 
	CvScalar GetBlendScalar(CvScalar value, CvScalar seam, int p, bool is_S);
	
	// new implementation, put gaussian mean into the middle
	// each part has 1 gaussian seperately, and the deviation is  _a and _b
	CvScalar GetBlendScalar2(CvScalar value, CvScalar seam, int p, int deviation);
private:
	// calculate Gaussian distribution fs(p).
	double calcFsofP(int p, int mean);
	// calculate the mean depends on the position of the pixel
	// scale is proportional parameter
	double calcMean(int p, int a, int b, double scale);
public:
	// blend 2 images - assumed to be of the SAME HEIGHT and SAME TYPE
	// a - size of left image portion to blend
	// b - size of right image portion to blend
	IplImage* BlendImages(IplImage* image1, IplImage* image2, int a, int b);
	// blend 2 images - assumed to be of the SAME HEIGHT and SAME TYPE
	// new implementation, put gaussian mean into the middle
	IplImage* BlendImages2(IplImage* image1, IplImage* image2, int a, int b);
	// blend 2 images - assumed to be of the SAME HEIGHT and SAME TYPE
	// new implementation, put gaussian mean into the middle and overlap 2 images
	// - size: overlap area size
	// - std: deviation of gaussian.
	// put std < size allow smoother transition
	// mean of Gaussian is always in the middle of the overlap area
	IplImage* BlendImages3(IplImage* image1, IplImage* image2, int size, int std);
	
	// first resize all images to same height
	// blend multiple images using filenames
	// then show the result
	void TestBlendMultiple3(char** fileList, int count);
	
	// combine to image with same height side by side
	IplImage* CombineImages(IplImage* image1, IplImage* image2);
	void cvBlendImages(IplImage *blend1);
};
// find images named "test1.jpg" and "test2.jpg" 
// supposed to have same height and combine them into 1 image.
void TestCombineImages();

// find images named "test1.jpg" and "test2.jpg"
// supposed to have same height to blend together
void TestBlendImages();

int TestROIBlend( int argc, char** argv );