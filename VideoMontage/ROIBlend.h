#include "stdafx.h"

// Include header files
#include "cv.h"
#include "highgui.h"
#include <math.h>

IplImage *Igray=0, *It = 0, *Iat;

/*
Usage: 
1) Load n images of type IplImage (n<=12)
2) Call cvBlendImages passing the image variable as the parameter
3) Needs implementation: Writing the Gaussian value of the pixel to the image
4) Show the images through the cvShowManyImages passing the new image files as the parameter
*/

void cvShowManyImages(char* title, int nArgs, ...);
int TestROIBlend( int argc, char** argv );
void calcFsofP(int val, int variance, char chan);
void cvBlendImages(IplImage *blend1);