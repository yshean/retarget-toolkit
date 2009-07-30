
/*
 *    Utility function prototypes
 *
 *    Author: Edward Alston Anthony
 *
 */

#include <math.h>
#include <stdio.h>
#include <vector>
#include <string>
#include "picture.h"
#include "video.h"
#include "matrix.h"

using namespace std;

#define PI 3.14159

#ifndef max
#define max(a, b) (a > b ? a : b)
#endif

#ifndef min
#define min(a, b) (a < b ? a : b)
#endif

typedef struct {
	int width;
	int height;
} imageSize;

typedef struct {
	int width;
	int height;
	int time;
} videoSize;

typedef struct {
  Picture *Images;
  int Levels;
} pyramidType;

typedef struct {
  Video *Videos;
  int Levels;
} videoPyramidType;

#define MAXIMUM_ITERATIONS	100
#define SMALLEST_ERROR_CHANGE	0.01
#define MINIMUM_LAMBDA_VALUE	pow((double)(10), (double)(-15))
#define MAXIMUM_LAMBDA_VALUE	pow((double)(10), (double)(15))

// const for infinity energy cost
#define MAX_COST_VALUE 1000.0//10000.0
#define VIDEO_FRAME_EXT "ppm"

double Gaussian(double x, double mean);
double Intensity(pixelType p);
int Convolve_Pixel(Picture *src, int x, int y, int kernel[][3]);
Matrix *Gradient(Picture *src);
Picture *DrawImage(Picture *I1, Picture *I2, Matrix *M, bool UseMultiresolutionSpline);
Matrix *Register(Picture *I1, Picture *I2, pointType InitialPoints[2][4]);
Picture *Reduce(Picture *src);
Picture *Expand(Picture *src);
Picture *Laplacian(Picture *g1, Picture *g0);
pyramidType *GaussianPyramid(Picture *src);
pyramidType *LaplacianPyramid(pyramidType *gaussianPyramid);
Picture *Collapse(pyramidType *LaplacianPyramid);
Picture *Combine(Picture *I1, Picture *I2,
                 pointType Max1, pointType Min1,
                 pointType Max2, pointType Min2,
                 pointType overlapMax, pointType overlapMin);


vector<string> Get_FrameNames(const char *foldername, const char *frame_ext);
videoPyramidType *VideoPyramid(Video *src);
Video *TemporalReduce(Video *src);
Video *ReduceVideo(Video *src);
Matrix *FrameDifference(Picture *left, Picture *right);
Matrix *Gradient_3D(Video *src);
int DownsamplingIndex(int p, imageSize &target_size, 
						imageSize &previous_size, double ratio);
int Downsampling3DIndex(int p, videoSize &target_size, 
						videoSize &previous_size, double ratio);

