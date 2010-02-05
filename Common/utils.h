
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
#include "picturelist.h"
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

typedef struct {
	PictureList *Lists;
	int Levels;
} listPyramidType;

typedef struct {
	Matrix *dx;
	Matrix *dy;
	double total_dx;
	double total_dy;
} gradient2D;

typedef struct {
	Matrix *dx;
	Matrix *dy;
	Matrix *dt;
	double *total_dx;
	double *total_dy;
	double *total_dt;
} gradient3D;

// gradient structure for seam carving
typedef struct {
	Matrix *dx;
	Matrix *dy;
} gradient2D_L1;

typedef struct {
	Matrix *LR;
	Matrix *pLU;
	Matrix *nLU;
} gradient2D_FE;

typedef struct {
	Matrix *LR;
	Matrix *pLU;
	Matrix *nLU;
	Matrix *temp_pLU;
	Matrix *temp_nLU;
} gradient3D_FE;


#define MAXIMUM_ITERATIONS	100
#define SMALLEST_ERROR_CHANGE	0.01
#define MINIMUM_LAMBDA_VALUE	pow((double)(10), (double)(-15))
#define MAXIMUM_LAMBDA_VALUE	pow((double)(10), (double)(15))

// const for infinity energy cost
#define MAX_COST_VALUE 10000.0//10000.0
#define PICTURE_FRAME_EXT "ppm"
#define VIDEO_FRAME_EXT "ppm"

#define SAMPLING_QUANTIZATION 5

double Gaussian(double x, double mean);
double Intensity(pixelType p);
double ColorContrast(Video *src, int x, int y, int t);
double GradientContrast(gradient3D *gradient, int time,int x, int y, int t);

int Convolve_Pixel(Picture *src, int x, int y, int kernel[][3]);
Picture *DrawImage(Picture *I1, Picture *I2, Matrix *M, bool UseMultiresolutionSpline);
Matrix *Register(Picture *I1, Picture *I2, pointType InitialPoints[2][4]);

Picture *Reduce(Picture *src);
Picture *Expand(Picture *src);
Picture *Laplacian(Picture *g1, Picture *g0);
pyramidType *GaussianPyramid(Picture *src);
Matrix *LocalScaleMap(pyramidType *pyramid, int level);
pyramidType *LaplacianPyramid(pyramidType *gaussianPyramid);

Picture *Collapse(pyramidType *LaplacianPyramid);
Picture *Combine(Picture *I1, Picture *I2,
                 pointType Max1, pointType Min1,
                 pointType Max2, pointType Min2,
                 pointType overlapMax, pointType overlapMin);


vector<string> Get_FrameNames(const char *foldername, const char *frame_ext);
videoPyramidType *VideoPyramid(Video *src);
Video *TemporalReduce(Video *src);

int *CalcMotionComponent(gradient3D *gradient, int source_time, 
						 int target_time, double &aveMotion);
Picture *InterpolateFrame(Picture *left, double lweight, Picture *right, double rweight);
Video *ReduceVideo(Video *src);
Matrix *FrameDifference(Picture *left, Picture *right, double &total_dt, double threshold=0);

gradient2D *Gradient(Picture *src);
gradient3D *Gradient_3D(Video *src, double threshold=0.0);
gradient2D *Naturality_2D(Picture *src, double threshold=0.0);
gradient2D *Diff_2D(Picture *src, double threshold=0.0);
gradient3D *Naturality_3D(PictureList *src, double threshold=0.0);
gradient3D *Diff_3D(PictureList *src, double threshold=0.0);
gradient3D *ScalingNaturality_3D(PictureList *src, double threshold=0.0);
Matrix *Contrast_3D(Video *src, gradient3D *gradient);

listPyramidType *ListPyramid(PictureList *src, int levels);


int DownsamplingIndex(int p, imageSize &target_size, 
						imageSize &previous_size, double ratio);
int Downsampling3DIndex(int p, videoSize &target_size, 
						videoSize &previous_size, double ratio);

/*
 * upsampling functions
 */
float simpleGauss(float x, float sigma, float mu);
int *SimpleUpsamplingMap(int *result, imageSize size, double ratio);
int *SimpleUpsamplingMapList(int *result, videoSize size, double ratio);
int *JointBilateralUpsampling(int *result, int s_width, int s_height, 
							  int s_time, PictureList *ref, double ratio);

Matrix *ReduceMatrix(Matrix *src);

/*
 * utility functions for seam carving
 */
Matrix *Gradient_xy(PictureList *src);
gradient2D_L1 *Gradient2D_L1(Picture *src);
Matrix *Rgb2Gray(Picture *src);
gradient2D_FE *Gradient2D_FE(Picture *src);
gradient3D_FE *Gradient3D_FE(PictureList *src);
