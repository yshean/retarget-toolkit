#include "StdAfx.h"
#include "Similarity.h"

double GetAsymmetriaDTW(double* signal1, int length1, double* signal2, int length2)
{
	double* M = new double[(length1+1)*(length2+1)];
	int width = length1 + 1;
	int height = length2 + 1;
	M[0] = 0;
	double infinity = 10000000;
	for(int i = 1; i < width; i++)
	{
		M[i] = infinity;
	}
	for(int i = 1; i < width; i++)
	{
		M[i * height] = 0;
	}

	for(int i = 1; i < width; i++)
		for(int j = 1; j < height; j++)
		{
			//if (i == length1 && j == length2)
			//	printf("test");
			//double test1 = pow((signal1[i - 1]- signal2[j - 1]),2);
			//double test2 = min(
			//	min(M[(i-1) * height + (j-1)] , 
			//	M[i * height + (j-1)]), 
			//	M[(i-1) * height + j]);

			M[i * height + j] = pow((signal1[i - 1] - signal2[j - 1]),2) + min(
				min(M[(i-1) * height + (j-1)] , 
				M[i * height + (j-1)]), 
				M[(i-1) * height + j]);
		}

	 
	return M[length1 * height + length2];
}

double GetPatchDTW(CvPoint point1, CvPoint point2, IplImage* image1, IplImage* image2, int patch_size)
{
	int length = patch_size * patch_size;
	double* patch1 = new double[length];
	double* patch2 = new double[length];
	double dtw = 0;
	for(int ch = 0; ch < image1->nChannels; ch++)
	{
		for(int i = 0; i < patch_size; i++)
		for(int j = 0; j < patch_size; j++)
		{
			CvScalar value1 = cvGet2D(image1, point1.y + j, point1.x + i);
			CvScalar value2 = cvGet2D(image2, point2.y + j, point2.x + i);
			patch1[i * patch_size + j] = value1.val[ch];
			patch2[i * patch_size + j] = value2.val[ch];
		}
		double test = patch2[8];
		dtw += GetAsymmetriaDTW(patch1, length, patch2, length);
	}
	return dtw;
}

// helper = no boundary checking
double GetPatchDifference(IplImage* img1, IplImage* img2, CvPoint point1, CvPoint point2, int patchSize)
{
	double diff = 0;

	for(int i = 0; i < patchSize - 1; i++)
		for(int j = 0; j < patchSize - 1; j++)
		{
			CvScalar value1 = cvGet2D(img1, point1.y + j, point1.x + i);
			CvScalar value2 = cvGet2D(img2, point2.y + j, point2.x + i);
			diff += SquareDifference(value1, value2);
		}
	return diff;
}
 

IplImage* GetScaleSimilar(IplImage* input, double scale)
{
	int width = input->width;
	int height = input->height;
	int newWidth = width * scale;
	int newHeight = height * scale;

	// create new scaled down image	
	IplImage* scale_img = cvCreateImage(cvSize(newWidth, newHeight), input->depth, input->nChannels);	
	IplImage* result = cvCloneImage(scale_img);
	cvResize(input, scale_img);

	// each patch compare similarity with original
	int patchsize = 3;
	int patchsize2 = patchsize / scale;
	for(int i = 0; i < scale_img->width - patchsize ; i++)
		for(int j = 0; j < scale_img->height - patchsize ; j++)
		{
			// find correspondence patch
			int i2 = i / scale;
			int j2 = j / scale;
			
			// find max diff
			double minDiff = 1000000;

			for(int ipatch = 0; ipatch < patchsize2 - patchsize + 1; ipatch++)
				for(int jpatch = 0; jpatch < patchsize2 - patchsize + 1; jpatch++)
				{					 
					double diff = GetPatchDifference(scale_img, input,
					cvPoint(i,j), cvPoint(i2 + ipatch, j2 + jpatch), patchsize);
					
					//double diff = GetPatchDTW(cvPoint(i,j), cvPoint(i2 + ipatch, j2 + jpatch),
					//	scale_img, input, patchsize);
					if(diff < minDiff)
						minDiff = diff;
				}
			cvSet2D(result, j, i, cvScalar(minDiff));
		}	
	return result;
}