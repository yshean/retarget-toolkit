//////////////////////////////////////////////////////////////////////////////
// Improved Hybrid Shift-Map for Video Retargeting
//////////////////////////////////////////////////////////////////////////////
//
// Its graph labeling formulation:
// For every frame, we design a 2d grid graph to represent it. Every node in 
// the graph corresponds to a pixel in the frame and is connected to 4 nieg-
// boring nodes. The labels are the shift-offset of the spatial coordinates
// of the pixel from source to target.  
// Similar to original shift-map, the objective energy function of graph-cut
// consists of 2 terms:
// 1. Data terms: sum_p D(p,l_p)
// 2. Smoothness terms: sum_{p,q} V_{pq}(l_p,l_q)
// Instead of directly applying shift-map on individual frames, whose output
// is not motion consistent, we introduce the motion-consistent shift-map for
// video retargeting. The idea of motion-consistency is to ensure that the 
// temporal changes are as similar as possible to the source. We encode the 
// data term to ensure such mostion-consistency.
// Given the shift-map of previous frame, we use it to constrain the graph-cut
// of current frame. For every spatial location $p$ in target, we have their
// shifts (l_{t-1}(p),l_{t}(p)) in both previous and current frame. The temporal
// change in the target is |I_{t-1}(p+l_{t-1}(p))-I_{t}(p+l_{t}(p))|. By fixing 
// the mapping in the previous frame, the temporal change of this location in 
// the source is |I_{t-1}(p+l_{t-1}(p))-I_{t}(p+l_{t-1}(p))|. So the similarity
// of the temporal change at this location between target and source can be 
// decided by |I_{t}(p+l_{t-1}(p))-I_{t}(p+l_{t}(p))|.


#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include<fstream>

#include "../Common/picture.h"
#include "../Common/utils.h"
#include "../GCoptimization/GCoptimization.h"

using namespace std;

typedef struct {
	double dx;
	double dy;
} gradientType;

inline double round( double d )
{
	return floor( d + 0.5 );
}

/*
 * structure of data term for single image
 */
struct ForSingleImgDataFn
{
	Picture *src;
	Matrix *bias;
	bool subbanded;
	int *subband;
	int subband_idx;
	int subband_lbound;
	imageSize target_size;
};

/*
 * structure of smooth term for single image
 */
struct ForSingleImgSmoothFn
{
	Picture *src;
	gradient2D *gradient;
	gradient2D *naturality;
	Matrix *bias;
	imageSize target_size;
};

/*
 * structure of data term for image pair
 */
struct ForPairImgDataFn
{
	Picture *src;
	Matrix *gray;
	Picture *pre;
	Matrix *pre_gray;
	int *pre_labels;
	Matrix *bias;
	bool subbanded;
	int *subband;
	int subband_idx;
	int subband_lbound;
	imageSize target_size;
};

/*
 * structure of smooth term for image pair
 */
struct ForPairImgSmoothFn
{
	Picture *src;
	gradient3D *gradient;
	gradient2D *naturality;
	Matrix *bias;
	imageSize target_size;
};

/*
 * structure of data term for video
 */
struct ForVideoDataFn
{
	PictureList *src;
	Matrix *bias;
	videoSize target_size;
};

/*
 * structure of smooth term for video
 */
struct ForVideoSmoothFn
{
	PictureList *src;
	gradient3D *gradient;
	gradient3D *naturality;
	Matrix *bias;
	videoSize target_size;
};

pixelType SamplingPixel(Picture *frame, int x, int y, int label)
{
	pixelType result;
	double dLeftWeight, dRightWeight;
	if ((label/SAMPLING_QUANTIZATION)%1!=0)
	{
		dLeftWeight = 1-label/SAMPLING_QUANTIZATION+
					  floor(label/SAMPLING_QUANTIZATION);
		dRightWeight = 1-floor(label/SAMPLING_QUANTIZATION+0.5)+
					   label/SAMPLING_QUANTIZATION;
	}
	else
	{
		dLeftWeight = 1;
		dRightWeight = 0;
	}

	pixelType pLeft = frame->GetPixel(x+floor(label/SAMPLING_QUANTIZATION),y);
	pixelType pRight = frame->GetPixel(x+floor(label/SAMPLING_QUANTIZATION+0.5),y);

	result.r = dLeftWeight*pLeft.r+dRightWeight*pRight.r;
	result.g = dLeftWeight*pLeft.g+dRightWeight*pRight.g;
	result.b = dLeftWeight*pLeft.b+dRightWeight*pRight.b;

	return result;
}

double PairwiseColorDiff(intensityType &p1, intensityType &p2, double threshold)
{
	double result = 0.0;

	double r_diff = pow(p1.r-p2.r,2.0);
	r_diff = (r_diff<threshold) ? 0 : r_diff;
	double g_diff = pow(p1.g-p2.g,2.0);
	g_diff = (g_diff<threshold) ? 0 : g_diff;
	double b_diff = pow(p1.b-p2.b,2.0);
	b_diff = (b_diff<threshold) ? 0 : b_diff;

	result += r_diff+g_diff+b_diff;
	return result;
}

double PairwiseGradientDiff(gradientType &p1_grad, gradientType &p2_grad, double threshold)
{
	double result = 0.0;

	double dx_diff = pow(p1_grad.dx-p2_grad.dx,2.0);
	dx_diff = (dx_diff<threshold) ? 0 : dx_diff;
	double dy_diff = pow(p1_grad.dy-p2_grad.dy,2.0);
	dy_diff = (dy_diff<threshold) ? 0 : dy_diff;

	result += dx_diff+dy_diff;
	return result;
}


/*
 * data term calculation for single image
 */
double SingleImgDataFn(int p, int l, void *data)
{
	ForSingleImgDataFn *myData = (ForSingleImgDataFn *) data;
	int width = myData->target_size.width;
	int height = myData->target_size.height;

	int x = p % width;
	int y = p / width;

	double cost = 0.0;

	// pixel arrangement
	if (x==0 && l!=0)
		return 100000*MAX_COST_VALUE;
	if (x==width-1 && 
		l!=abs(myData->src->GetWidth()-width))
		return 100000*MAX_COST_VALUE;

	if (myData->subbanded)
	{
		int refine_offset = 2;
		if (x+myData->subband_lbound<myData->subband[myData->subband_idx+y]-refine_offset && l!=0)
			return 100000*MAX_COST_VALUE;
		if (x+myData->subband_lbound>myData->subband[myData->subband_idx+y]+refine_offset && l==0)
			return 100000*MAX_COST_VALUE;
	}

	//cost += 1000*myData->bias->Get(y+1,x+l+1);

	return cost;
}

double SingleImgColorDiff(Picture *src, Matrix *bias, int x1, int y1, int x2, 
						  int y2, int l1, int l2, int direction,double threshold)
{
	double diff = 0.0;
	int width = src->GetWidth();
	int height = src->GetHeight(); 
	int x_offset = x2-x1;
	int y_offset = y2-y1;

	//printf("ColorDiff: pixels: (%d,%d) and (%d,%d) labels: %d and %d\n",x2,y2+l2,x1+x_offset,y1+l1+y_offset,l2,l1);
	if (x2+l2>=0 && x2+l2<width && x1+l1+x_offset>=0 && x1+l1+x_offset<width &&
		x2+l2-x_offset>=0 && x2+l2-x_offset<width && x1+l1>=0 && x1+l1<width)
	{
		//double weight;
		pixelType pix1, pix2;
		double color_diff;
		//printf("(%d,%d) color components: %d,%d,%d\n",src->GetPixel(y2+l2,x2).r,src->GetPixel(y2+l2,x2).g,src->GetPixel(y2+l2,x2).b);
		if (direction==0)
		{
			color_diff = 0.0;
			pix1 = src->GetPixel(x2+l2,y2);
			pix2 = src->GetPixel(x1+l1+x_offset,y1+y_offset);
			//weight = bias->Get(y2+1,x2+l2+1)*bias->Get(y1+y_offset+1,x1+l1+x_offset+1);
			color_diff += pow((double)(pix1.r-pix2.r),2.0);
			color_diff += pow((double)(pix1.g-pix2.g),2.0);
			color_diff += pow((double)(pix1.b-pix2.b),2.0);
			//diff += 1000*weight;
			diff += (color_diff<threshold) ? 0 : color_diff;
		}
		if (direction==1)
		{
			color_diff = 0.0;
			pix1 = src->GetPixel(x2+l2-x_offset,y2-y_offset);
			pix2 = src->GetPixel(x1+l1,y1);
			//weight = bias->Get(y2-y_offset+1,x2+l2-x_offset+1)*bias->Get(y1+1,x1+l1+1);
			color_diff += pow((double)(pix1.r-pix2.r),2.0);		
			color_diff += pow((double)(pix1.g-pix2.g),2.0);	
			color_diff += pow((double)(pix1.b-pix2.b),2.0);
			//diff += 1000*weight;
			diff += (color_diff<threshold) ? 0 : color_diff;
		}
	} else
	{
		diff += 100000*MAX_COST_VALUE;
	}
	//printf("ColorDiff: finished\n");

	//printf("ColorDiff: cost between (%d,%d) and (%d,%d) with labels %d and %d is %d\n",x1,y1,x2,y2,l1,l2,diff);
	return diff;
}

double SingleImgGrayscaleDiff(Matrix *gray, Matrix *bias, int x1, int y1, 
							  int x2, int y2, int l1, int l2, int direction, double threshold)
{
	double diff = 0.0;
	int width = gray->NumOfCols();
	int height = gray->NumOfRows();
	int x_offset = x2-x1;
	int y_offset = y2-y1;

	if (x2+l2>0 && x2+l2<=width && x1+l1+x_offset>0 && x1+l1+x_offset<=width &&
		x2+l2-x_offset>0 && x2+l2-x_offset<=width && x1+l1>0 && x1+l1<=width)
	{
		//double weight;
		double grad_diff;
		if (direction==0)
		{
			//weight = bias->Get(y2,x2+l2)*bias->Get(y1+y_offset,x1+l1+x_offset);
			grad_diff = 0.0;
			grad_diff += pow((double)(gray->Get(y2,x2+l2)-gray->Get(y1+y_offset,x1+l1+x_offset)),2.0);
			//diff += 1000*weight;
			diff += (grad_diff<threshold) ? 0 : grad_diff;
		}
		if (direction==1)
		{
			//weight = bias->Get(y2-y_offset,x2+l2-x_offset)*bias->Get(y1,x1+l1);
			grad_diff = 0.0;
			grad_diff += pow((double)(gray->Get(y1,x1+l1)-gray->Get(y2-y_offset,x2+l2-x_offset)),2.0);
			//diff += 1000*weight;
			diff += (grad_diff<threshold) ? 0 : grad_diff;
		}
	} else
	{	
		diff += 100000*MAX_COST_VALUE;
	}

	return diff;
}

double SingleImgGradientDiff(gradient2D *gradient, Matrix *bias, int x1, int y1, 
							 int x2, int y2, int l1, int l2, int direction, double threshold)
{
	double diff = 0.0;
	int width = gradient->dx->NumOfCols();
	int height = gradient->dx->NumOfRows();
	int x_offset = x2-x1;
	int y_offset = y2-y1;

	if (x2+l2>0 && x2+l2<=width && x1+l1+x_offset>0 && x1+l1+x_offset<=width &&
		x2+l2-x_offset>0 && x2+l2-x_offset<=width && x1+l1>0 && x1+l1<=width)
	{
		//double weight;
		double grad_diff;
		if (direction==0)
		{
			//weight = bias->Get(y2,x2+l2)*bias->Get(y1+y_offset,x1+l1+x_offset);
			grad_diff = 0.0;
			grad_diff += pow((double)(gradient->dx->Get(y2,x2+l2)-gradient->dx->Get(y1+y_offset,x1+l1+x_offset)),2.0)+
						 pow((double)(gradient->dy->Get(y2,x2+l2)-gradient->dy->Get(y1+y_offset,x1+l1+x_offset)),2.0);
			//diff += 1000*weight;
			diff += (grad_diff<threshold) ? 0 : grad_diff;
		}
		if (direction==1)
		{
			//weight = bias->Get(y2-y_offset,x2+l2-x_offset)*bias->Get(y1,x1+l1);
			grad_diff = 0.0;
			grad_diff += pow((double)(gradient->dx->Get(y2-y_offset,x2+l2-x_offset)-gradient->dx->Get(y1,x1+l1)),2.0)+
						 pow((double)(gradient->dy->Get(y2-y_offset,x2+l2-x_offset)-gradient->dy->Get(y1,x1+l1)),2.0);
			//diff += 1000*weight;
			diff += (grad_diff<threshold) ? 0 : grad_diff;
		}
	} else
	{	
		diff += 100000*MAX_COST_VALUE;
	}

	return diff;
}

double SingleImgSmoothFn(int p1, int p2, int l1, int l2, void *data)
{
	if (l1==l2)
		return 0.0;

	ForSingleImgSmoothFn *myData = (ForSingleImgSmoothFn *) data;
	double cost = 0.0;
	//printf("smoothFn: calculating image gradient...\n");
	int width = myData->target_size.width;
	int height = myData->target_size.height;
	//printf("smoothFn: image width=%d and height=%d\n",width,height);
	int x1 = p1 % width;
	int y1 = p1 / width;
	int x2 = p2 % width;
	int y2 = p2 / width;
	//printf("smoothFn: (%d,%d) and (%d,%d)\n",x1,y1,x2,y2);

	if ((y1==y2) && ((x1<x2 && l1>l2) || (x1>x2 && l1<l2)))
		return 100000*MAX_COST_VALUE;

	/* connectivity */
	/*
	if ((abs(y1-y2)==1) && (abs(x1-x2)==1))
	{
		if (x1<x2 && l1>0 && l2==0)
			return 100000*MAX_COST_VALUE;
		if (x2<x1 && l2>0 && l1==0)
			return 100000*MAX_COST_VALUE;
	}
	*/

	/* enlarge */
	if (myData->src->GetWidth()<width)
	{
		if (x1==x2)
		{
			if (y1<y2 && x1>0)
				cost += myData->naturality->dy->Get(y1+1,x1);
			if (y2<y1 && x2>0)
				cost += myData->naturality->dy->Get(y2+1,x2);
		}
		if (y1==y2 && l1>0 && x1>0)
			cost += myData->gradient->dx->Get(y1+1,x1);
		if (y1==y2 && l2>0 && x2>0)
			cost += myData->gradient->dx->Get(y2+1,x2);

		/*
		if (l1>0 && x1>0)
			cost += myData->gradient->dx->Get(y1+1,x1);
		if (l2>0 && x2>0)
			cost += myData->gradient->dx->Get(y2+1,x2);
		*/
	}

	/* reduce */
	if (myData->src->GetWidth()>width)
	{
		if (y1==y2)
		{
			if (l1>0)
			{
				cost += myData->naturality->dx->Get(y1+1,x1+1);
				/* edgeness */
				//cost += myData->gradient->dx->Get(y2+1,x2+1);
				/*
				cost += 1*myData->gradient->dx->Get(y1+1,x1+1);
				cost += 1*myData->gradient->dx->Get(y2+1,x2+1);
				if (y1>0)
					cost += 1*myData->gradient->dy->Get(y1,x1+1);
				if (y1<height-1)
					cost += 1*myData->gradient->dy->Get(y1+1,x1+1);
				*/
			}
			if (l2>0)
			{
				cost += myData->naturality->dx->Get(y2+1,x2+1);
				/* edgeness */
				//cost += myData->gradient->dx->Get(y1+1,x1+1);
				/*
				cost += 1*myData->gradient->dx->Get(y1+1,x1+1);
				cost += 1*myData->gradient->dx->Get(y2+1,x2+1);
				if (y2>0)
					cost += 1*myData->gradient->dy->Get(y2,x2+1);
				if (y2<height-1)
					cost += 1*myData->gradient->dy->Get(y2+1,x2+1);
				*/
			}
		}
		if (x1==x2)
		{
			if (y1<y2)
			{
				cost += myData->naturality->dy->Get(y1+1,x1+1);
				//cost += myData->gradient->dy->Get(y1+1,x1+1);
			}
			if (y2<y1)
			{
				cost += myData->naturality->dy->Get(y2+1,x2+1);
				//cost += myData->gradient->dy->Get(y2+1,x2+1);
			}
		}
	}

	//printf("smoothFn: computing cost between (%d,%d) and (%d,%d) with labels %d and %d : %d\n",y1,x1,y2,x2,l1,l2,cost);
	return cost;
}

/*
 * data function for pair of images
 */
double PairImgDataFn(int p, int l, void *data)
{
	ForPairImgDataFn *myData = (ForPairImgDataFn *) data;

	int x = p % myData->target_size.width;
	int y = p / myData->target_size.width;

	double cost = 0.0;
	
	if (x==0 && l!=0)
		return 100000*MAX_COST_VALUE;
	if (x==myData->target_size.width-1 && 
		l!=abs(myData->src->GetWidth()-myData->target_size.width))
		return 100000*MAX_COST_VALUE;

	if (myData->subbanded)
	{
		int refine_offset = 2;
		if (x+myData->subband_lbound<myData->subband[myData->subband_idx+y]-refine_offset && l!=0)
			return 100000*MAX_COST_VALUE;
		if (x+myData->subband_lbound>myData->subband[myData->subband_idx+y]+refine_offset && l==0)
			return 100000*MAX_COST_VALUE;
	}
 
	// preserve temporal consistency
	/* enlarge */
	if (myData->src->GetWidth()<myData->target_size.width)
	{
		int cur_match = x-l;
		int pre_match = x+myData->pre_labels[p];

		if (cur_match==pre_match)
			return 0.0;

		double min_diff;
		if (cur_match>=0 && cur_match<myData->src->GetWidth())
		{
			min_diff = min(abs(myData->gray->Get(y+1,cur_match+1)-myData->gray->Get(y+1,pre_match+1)),
						   abs(myData->pre_gray->Get(y+1,cur_match+1)-myData->pre_gray->Get(y+1,pre_match+1)));
		}
		else
			min_diff = 100000*MAX_COST_VALUE;

		cost += 1*min_diff;
		//cost += 1000*myData->bias->Get(y+1,x+l+1);
	}

	/* reduce */
	if (myData->src->GetWidth()>myData->target_size.width)
	{
		int cur_match = x+l;
		int pre_match = x+myData->pre_labels[p];

		if (cur_match==pre_match)
			return 0.0;


		double min_diff;
		if (cur_match>=0 && cur_match<myData->src->GetWidth())
		{
			min_diff = min(abs(myData->gray->Get(y+1,cur_match+1)-myData->gray->Get(y+1,pre_match+1)),
						   abs(myData->pre_gray->Get(y+1,cur_match+1)-myData->pre_gray->Get(y+1,pre_match+1)));
			/*
			intensityType p1 = myData->src->GetPixelIntensity(cur_match,y);
			intensityType p2 = myData->src->GetPixelIntensity(pre_match,y);
			min_diff = PairwiseColorDiff(myData->src->GetPixelIntensity(cur_match,y),
										 myData->src->GetPixelIntensity(pre_match,y),0.0);
			*/

			/*
			min_diff += pow(myData->gradient->dx->Get(y+1,cur_match+1)-
							myData->gradient->dx->Get(y+1,pre_match+1),2.0);
			min_diff += pow(myData->gradient->dy->Get(y+1,cur_match+1)-
							myData->gradient->dy->Get(y+1,pre_match+1),2.0);
			*/
		}
		else
			min_diff = 100000*MAX_COST_VALUE;

		cost += 1*min_diff;
		//cost += 1000*myData->bias->Get(y+1,x+l+1);
	}
	
	return cost;
}

/*
 * Compute color difference between two mapped points
 */
double PairImgColorDiff(Picture *src, Matrix *bias, int x1, int y1, int x2, 
						int y2, int l1, int l2, int direction, double threshold)
{
	double diff = 0.0;
	int width = src->GetWidth();
	int height = src->GetHeight(); 
	int x_offset = x2-x1;
	int y_offset = y2-y1;

	//printf("ColorDiff: pixels: (%d,%d) and (%d,%d) labels: %d and %d\n",x2,y2+l2,x1+x_offset,y1+l1+y_offset,l2,l1);
	if (x2+l2>=0 && x2+l2<width && x1+l1+x_offset>=0 && x1+l1+x_offset<width &&
		x2+l2-x_offset>=0 && x2+l2-x_offset<width && x1+l1>=0 && x1+l1<width)
	{
		//double weight;
		pixelType pix1, pix2;
		double color_diff;
		//printf("(%d,%d) color components: %d,%d,%d\n",src->GetPixel(y2+l2,x2).r,src->GetPixel(y2+l2,x2).g,src->GetPixel(y2+l2,x2).b);
		if (direction==0)
		{
			//weight = bias->Get(y2+1,x2+l2+1)*bias->Get(y1+y_offset+1,x1+l1+x_offset+1);
			pix1 = src->GetPixel(x2+l2,y2);
			pix2 = src->GetPixel(x1+l1+x_offset,y1+y_offset);
			color_diff = 0.0;
			color_diff += pow((double)(pix1.r-pix2.r),2.0);
			color_diff += pow((double)(pix1.g-pix2.g),2.0);
			color_diff += pow((double)(pix1.b-pix2.b),2.0);
			//diff += 1000*weight;
			diff += (color_diff<threshold) ? 0 : color_diff;
		}
		
		if (direction==1)
		{
			//weight = bias->Get(y2-y_offset+1,x2+l2-x_offset+1)*bias->Get(y1+1,x1+l1+1);
			pix1 = src->GetPixel(x2+l2-x_offset,y2-y_offset);
			pix2 = src->GetPixel(x1+l1,y1);
			color_diff = 0.0;
			color_diff += pow((double)(pix1.r-pix2.r),2.0);	
			color_diff += pow((double)(pix1.g-pix2.g),2.0);
			color_diff += pow((double)(pix1.b-pix2.b),2.0);
			//diff += 1000*weight;
			diff += (color_diff<threshold) ? 0 : color_diff;
		}
	} else
	{
		diff += 100000*MAX_COST_VALUE;
	}
	//printf("ColorDiff: finished\n");

	//printf("ColorDiff: cost between (%d,%d) and (%d,%d) with labels %d and %d is %d\n",x1,y1,x2,y2,l1,l2,diff);
	return diff;
}

/*
 * Compute grayscale difference between two mapped points
 */
double PairImgGrayscaleDiff(Matrix *gray, Matrix *bias, int x1, int y1, 
						    int x2, int y2, int l1, int l2, int direction, double threshold)
{
	if (l1==l2)
		return 0.0;

	double diff = 0.0;
	int width = gray->NumOfCols();
	int height = gray->NumOfRows();
	int x_offset = x2-x1;
	int y_offset = y2-y1;

	if (x2+l2>0 && x2+l2<=width && x1+l1+x_offset>0 && x1+l1+x_offset<=width &&
		x2+l2-x_offset>0 && x2+l2-x_offset<=width && x1+l1>0 && x1+l1<=width)
	{
		//double weight;
		double grad_diff;
		if (direction==0)
		{
			//weight = bias->Get(y2,x2+l2)*bias->Get(y1+y_offset,x1+l1+x_offset);
			grad_diff = 0.0;
			grad_diff += pow((double)(gray->Get(y2,x2+l2)-gray->Get(y1+y_offset,x1+l1+x_offset)),2.0);
			//diff += 1000*weight;
			diff += (grad_diff<threshold) ? 0 : grad_diff;
		}
		if (direction==1)
		{
			//weight = bias->Get(y2-y_offset,x2+l2-x_offset)*bias->Get(y1,x1+l1);
			grad_diff = 0.0;
			grad_diff += pow((double)(gray->Get(y1,x1+l1)-gray->Get(y2-y_offset,x2+l2-x_offset)),2.0);
			//diff += 1000*weight;
			diff += (grad_diff<threshold) ? 0 : grad_diff;
		}
	} else
	{	
		diff += 100000*MAX_COST_VALUE;
	}

	return diff;
}

/*
 * Compute gradient difference between two mapped points
 */
double PairImgGradientDiff(gradient2D *gradient, Matrix *bias, int x1, int y1, 
						   int x2, int y2, int l1, int l2, int direction, double threshold)
{
	if (l1==l2)
		return 0.0;

	double diff = 0.0;
	int width = gradient->dx->NumOfCols();
	int height = gradient->dx->NumOfRows();
	int x_offset = x2-x1;
	int y_offset = y2-y1;

	if (x2+l2>0 && x2+l2<=width && x1+l1+x_offset>0 && x1+l1+x_offset<=width &&
		x2+l2-x_offset>0 && x2+l2-x_offset<=width && x1+l1>0 && x1+l1<=width)
	{
		//double weight;
		double grad_diff;
		if (direction==0)
		{
			//weight = bias->Get(y2,x2+l2)*bias->Get(y1+y_offset,x1+l1+x_offset);
			grad_diff = 0.0;
			grad_diff += pow((double)(gradient->dx->Get(y2,x2+l2)-gradient->dx->Get(y1+y_offset,x1+l1+x_offset)),2.0)+
						 pow((double)(gradient->dy->Get(y2,x2+l2)-gradient->dy->Get(y1+y_offset,x1+l1+x_offset)),2.0);
			//diff += 1000*weight;
			diff += (grad_diff<threshold) ? 0 : grad_diff;
		}
		if (direction==1)
		{
			//weight = bias->Get(y2-y_offset,x2+l2-x_offset)*bias->Get(y1,x1+l1);
			grad_diff = 0.0;
			grad_diff += pow((double)(gradient->dx->Get(y2-y_offset,x2+l2-x_offset)-gradient->dx->Get(y1,x1+l1)),2.0)+
						 pow((double)(gradient->dy->Get(y2-y_offset,x2+l2-x_offset)-gradient->dy->Get(y1,x1+l1)),2.0);
			//diff += 1000*weight;
			diff += (grad_diff<threshold) ? 0 : grad_diff;
		}
	} else
	{	
		diff += 100000*MAX_COST_VALUE;
	}

	return diff;
}

/*
 * Smoothness term calculation
 */
double PairImgSmoothFn(int p1, int p2, int l1, int l2, void *data)
{
	if (l1==l2)
		return 0.0;

	ForPairImgSmoothFn *myData = (ForPairImgSmoothFn *) data;
	double cost = 0.0;
	//printf("smoothFn: calculating image gradient...\n");
	int width = myData->target_size.width;
	int height = myData->target_size.height;
	//printf("smoothFn: image width=%d and height=%d\n",width,height);
	int x1 = p1 % width;
	int y1 = p1 / width;
	int x2 = p2 % width;
	int y2 = p2 / width;
	//printf("smoothFn: (%d,%d) and (%d,%d)\n",x1,y1,x2,y2);

	if ((y1==y2) && ((x1<x2 && l1>l2) || (x1>x2 && l1<l2)))
		return 100000*MAX_COST_VALUE;

	/* connectivity */
	/*
	if ((abs(y1-y2)==1) && (abs(x1-x2)==1))
	{
		if (x1<x2 && l1>0 && l2==0)
			return 100000*MAX_COST_VALUE;
		if (x2<x1 && l2>0 && l1==0)
			return 100000*MAX_COST_VALUE;
	}
	*/

	//cost += 100*abs(myData->gray->Get(y2+1,x2+1)-myData->gray->Get(y1+1,x1+1));
	/* enlarge */
	if (myData->src->GetWidth()<width)
	{
		if (x1==x2)
		{
			if (y1<y2 && x1>0)
				cost += myData->naturality->dy->Get(y1+1,x1);
			if (y2<y1 && x2>0)
				cost += myData->naturality->dy->Get(y2+1,x2);
		}
		if (y1==y2 && l1>0 && x1>0)
			cost += myData->gradient->dx->Get(y1+1,x1);
		if (y1==y2 && l2>0 && x2>0)
			cost += myData->gradient->dx->Get(y2+1,x2);

		/*
		if (l1>0 && x1>0)
			cost += myData->gradient->dx[1].Get(y1+1,x1);
		if (l2>0 && x2>0)
			cost += myData->gradient->dx[1].Get(y2+1,x2);
		*/
	}

	/* reduce */
	if (myData->src->GetWidth()>width)
	{
		if (y1==y2)
		{
			if (l1>0)
			{
				cost += 1*myData->naturality->dx->Get(y1+1,x1+1);
				/* edgeness */
				//cost += myData->gradient->dx->Get(y2+1,x2+1);
				/*
				cost += 1*myData->gradient->dt[0].Get(y1+1,x1+1);
				cost += 1*myData->gradient->dx->Get(y1+1,x1+1);
				cost += 1*myData->gradient->dx->Get(y2+1,x2+1);
				if (y1>0)
					cost += 1*myData->gradient->dy->Get(y1,x1+1);
				if (y1<height-1)
					cost += 1*myData->gradient->dy->Get(y1+1,x1+1);
				*/
			}
			if (l2>0)
			{
				cost += 1*myData->naturality->dx->Get(y2+1,x2+1);
				/* edgeness */
				//cost += myData->gradient->dx->Get(y1+1,x1+1);
				/*
				cost += 1*myData->gradient->dt[0].Get(y2+1,x2+1);
				cost += 1*myData->gradient->dx->Get(y1+1,x1+1);
				cost += 1*myData->gradient->dx->Get(y2+1,x2+1);
				if (y2>0)
					cost += 1*myData->gradient->dy->Get(y2,x2+1);
				if (y2<height-1)
					cost += 1*myData->gradient->dy->Get(y2+1,x2+1);
				*/
			}
		}
		if (x1==x2)
		{
			if (y1<y2)
			{
				cost += myData->naturality->dy->Get(y1+1,x1+1);
				//cost += myData->gradient->dy->Get(y1+1,x1+1);
			}
			if (y2<y1)
			{
				cost += myData->naturality->dy->Get(y2+1,x2+1);
				//cost += myData->gradient->dy->Get(y1+1,x1+1);
			}
		}
	}

	//printf("smoothFn: computing cost between (%d,%d) and (%d,%d) with labels %d and %d : %d\n",y1,x1,y2,x2,l1,l2,cost);
	return cost;
}

/*
 * Motion-guided data term calculation
 */
double VideoDataFn(int p, int l, void *data)
{
	ForVideoDataFn *myData = (ForVideoDataFn *) data;
	int width = myData->target_size.width;
	int height = myData->target_size.height;
	int time = myData->target_size.time;

	int	t = p / (width*height);
	int x = (p % (width*height)) % width;
	int y = (p % (width*height)) / width;

	double cost = 0.0;

	// pixel arrangement
	if (x==0 && l!=0)
		return 100000*MAX_COST_VALUE;
	if (x==myData->target_size.width-1 && l==0)
		return 100000*MAX_COST_VALUE;

	// data term for scaling
	if (l==2)
	{
		sCenter = 0.5*();
		cost = 
	}

	return cost;
}

/*
 * Compute grayscale difference between two mapped points
 */
double VideoGrayScaleDiff(Matrix *gray, int x1, int y1, int t1, int x2, int y2, 
						  int t2, int l1, int l2, int direction, double threshold)
{
	double diff = 0.0;
	int width = gray[0].NumOfCols();
	int height = gray[0].NumOfRows();
	int x_offset = x2-x1;
	int y_offset = y2-y1;
	int t_offset = t2-t1;

	if (x2+l2>0 && x2+l2<=width && x1+l1+x_offset>0 && x1+l1+x_offset<=width &&
		x2+l2-x_offset>0 && x2+l2-x_offset<=width  && x1+l1>0 && x1+l1<=width)
	{
		double grad_diff = 0.0;
		if (direction==0)
		{
			grad_diff += pow((double)(gray[t2].Get(y2,x2+l2)-gray[t1+t_offset].Get(y1+y_offset,x1+l1+x_offset)),2.0);
			diff += (grad_diff<threshold) ? 0 : grad_diff;
		}

		if (direction==1)
		{
			grad_diff += pow((double)(gray[t1].Get(y1,x1+l1)-gray[t2-t_offset].Get(y2-y_offset,x2+l2-x_offset)),2.0);
			diff += (grad_diff<threshold) ? 0 : grad_diff;
		}
	} else
	{	
		diff += 100000*MAX_COST_VALUE;
	}

	if (t1!=t2)
		return 1*diff;
	else
		return diff;
}

/*
 * Compute color difference between two mapped points
 */
double VideoColorDiff(PictureList *src, int x1, int y1, int t1, int x2, int y2, 
					  int t2, int l1, int l2, int direction, double threshold)
{
	double diff = 0.0;	
	int width = src->GetPicture(t1)->GetWidth();
	int height = src->GetPicture(t1)->GetHeight(); 
	int time = src->GetLength();
	int x_offset = x2-x1;
	int y_offset = y2-y1;
	int t_offset = t2-t1;

	if (x2+l2>=0 && x2+l2<width && x1+l1+x_offset>=0 && x1+l1+x_offset<width &&
		x2+l2-x_offset>=0 && x2+l2-x_offset<width && x1+l1>=0 && x1+l1<width)
	{
		pixelType pix1, pix2;
		double color_diff = 0.0;
		if (direction == 0)
		{
			pix1 = src->GetPicture(t2)->GetPixel(x2+l2,y2);
			pix2 = src->GetPicture(t1+t_offset)->GetPixel(x1+l1+x_offset,y1+y_offset);
			color_diff += pow((double)(pix1.r-pix2.r),2.0);
			color_diff += pow((double)(pix1.g-pix2.g),2.0);
			color_diff += pow((double)(pix1.b-pix2.b),2.0);
			diff += (color_diff<threshold) ? 0 : color_diff;
		}

		if (direction == 1)
		{
			pix1 = src->GetPicture(t2-t_offset)->GetPixel(x2+l2-x_offset,y2-y_offset);
			pix2 = src->GetPicture(t1)->GetPixel(x1+l1,y1); 
			color_diff += pow((double)(pix1.r-pix2.r),2.0);
			color_diff += pow((double)(pix1.g-pix2.g),2.0);
			color_diff += pow((double)(pix1.b-pix2.b),2.0);
			diff += (color_diff<threshold) ? 0 : color_diff;
		}

	} else
	{
		diff += 100000*MAX_COST_VALUE;
	}	
	//printf("ColorDiff: finished\n");

	//printf("ColorDiff: cost between (%d,%d) and (%d,%d) with labels %d and %d is %d\n",x1,y1,x2,y2,l1,l2,diff);
	if (t1!=t2)
		return 50*diff;
	else
		return diff;
}

/*
 * Compute gradient difference between two mapped points
 */
double VideoGradientDiff(gradient2D *gradient, int x1, int y1, int t1, int x2, int y2, 
						 int t2, int l1, int l2, int direction, double threshold)
{
	double diff = 0.0;
	int width = gradient[0].dx->NumOfCols();
	int height = gradient[0].dx->NumOfRows();
	int x_offset = x2-x1;
	int y_offset = y2-y1;
	int t_offset = t2-t1;

	if (x2+l2>0 && x2+l2<=width && x1+l1+x_offset>0 && x1+l1+x_offset<=width &&
		x2+l2-x_offset>0 && x2+l2-x_offset<=width && x1+l1>0 && x1+l1<=width)
	{
		double grad_diff = 0.0;
		if (direction==0)
		{
			grad_diff += pow((double)(gradient[t2].dx->Get(y2,x2+l2)-gradient[t1+t_offset].dx->Get(y1+y_offset,x1+l1+x_offset)),2.0);
			grad_diff += pow((double)(gradient[t2].dy->Get(y2,x2+l2)-gradient[t1+t_offset].dy->Get(y1+y_offset,x1+l1+x_offset)),2.0);
			diff += (grad_diff<threshold) ? 0 : grad_diff;
		}

		if (direction==1)
		{
			grad_diff += pow((double)(gradient[t1].dx->Get(y1,x1+l1)-gradient[t2-t_offset].dx->Get(y2-y_offset,x2+l2-x_offset)),2.0);
			grad_diff += pow((double)(gradient[t1].dy->Get(y1,x1+l1)-gradient[t2-t_offset].dy->Get(y2-y_offset,x2+l2-x_offset)),2.0);
			diff += (grad_diff<threshold) ? 0 : grad_diff;
		}
	} else
	{	
		diff += 100000*MAX_COST_VALUE;
	}

	if (t1!=t2)
		return 50*diff;
	else
		return diff;
}

/*
 * Smoothness term calculation
 */
double VideoSmoothFn(int p1, int p2, int l1, int l2, void *data)
{
	if (l1==l2)
		return 0.0;

	ForVideoSmoothFn *myData = (ForVideoSmoothFn *) data;
	double cost = 0.0;
	//printf("smoothFn: calculating image gradient...\n");
	int width = myData->target_size.width;
	int height = myData->target_size.height;
	int time = myData->target_size.time;
	//printf("smoothFn: image width=%d and height=%d\n",width,height);
	int	t1 = p1 / (width*height);
	int x1 = (p1 % (width*height)) % width;
	int y1 = (p1 % (width*height)) / width;

	int	t2 = p2 / (width*height);
	int x2 = (p2 % (width*height)) % width;
	int y2 = (p2 % (width*height)) / width;

	if ((y1==y2) && (t1==t2) && ((x1<x2 && l1>l2) || (x1>x2 && l1<l2)))
		return 100000*MAX_COST_VALUE;

	/* connectivity */
	/*
	if ((abs(t1-t2)==1) && (y1==y2) && (abs(x1-x2)==1))
	{
		if (x1<x2 && l1>0 && l2==0)
			return 100000*MAX_COST_VALUE;
		if (x2<x1 && l2>0 && l1==0)
			return 100000*MAX_COST_VALUE;
	}
	if ((t1==t2) && (abs(y1-y2)==1) && (abs(x1-x2)==1))
	{
		if (x1<x2 && l1>0 && l2==0)
			return 100000*MAX_COST_VALUE;
		if (x2<x1 && l2>0 && l1==0)
			return 100000*MAX_COST_VALUE;
	}
	*/

	/* expansion */
	if (myData->src->GetMaxWidth()<width)
	{
		if (t1==t2 && x1==x2)
		{
			if (y1<y2 && x1>0)
				cost += 1*myData->naturality->dy[t1].Get(y1+1,x1);
			if (y2<y1 && x2>0)
				cost += 1*myData->naturality->dy[t2].Get(y2+1,x2);
		}
		if (x1==x2 && y1==y2)
		{
			if (t1<t2 && x1>0)
				cost += 1*myData->naturality->dt[t1].Get(y1+1,x1);
			if (t2<t1 && x2>0)
				cost += 1*myData->naturality->dt[t2].Get(y2+1,x2);
		}
		if (t1==t2 && y1==y2 && l1>0 && x1>0)
			cost += 1*myData->gradient->dx[t1].Get(y1+1,x1);
		if (t1==t2 && y1==y2 && l2>0 && x2>0)
			cost += 1*myData->gradient->dx[t2].Get(y2+1,x2);
		/*
		if (l1>0 && x1>0)
			cost += 1*myData->gradient->dx[t1].Get(y1+1,x1);
		if (l2>0 && x2>0)
			cost += 1*myData->gradient->dx[t2].Get(y2+1,x2);
		*/
		if (t1==t2 && y1==y2)
		{
			if (x1<x2 && l2>0 && myData->bias[t1].Get(y1+1,x1+1)>0)
				return 100000*MAX_COST_VALUE;
			if (x2<x1 && l1>0 && myData->bias[t2].Get(y2+1,x2+1)>0)
				return 100000*MAX_COST_VALUE;
		}
	}

	/* reduce */
	if (myData->src->GetMaxWidth()>width)
	{
		if (t1==t2 && y1==y2)
		{
			if (l1>0)
			{
				//double weight = simpleGauss(x1,sigma,width/2);
				cost += 1*myData->naturality->dx[t1].Get(y1+1,x1+1);
				/* edgeness */
				//cost += 1*myData->gradient->dx[t2].Get(y2+1,x2+1);
				/*
				if (t1<time-1)
					cost += 1*myData->gradient->dt[t1].Get(y1+1,x1+1);
				else
					cost += 1*myData->gradient->dt[t1-1].Get(y1+1,x1+1);
				cost += 1*myData->gradient->dx[t1].Get(y1+1,x1+1);
				cost += 1*myData->gradient->dx[t2].Get(y2+1,x2+1);
				if (y1>0)
					cost += 1*myData->gradient->dy[t1].Get(y1,x1+1);
				if (y1<height-1)
					cost += 1*myData->gradient->dy[t1].Get(y1+1,x1+1);
				*/
			}
			if (l2>0)
			{
				//double weight = simpleGauss(x2,sigma,width/2);
				cost += 1*myData->naturality->dx[t2].Get(y2+1,x2+1);
				/* edgeness */
				//cost += myData->gradient->dx[t1].Get(y1+1,x1+1);
				/*
				if (t2<time-1)
					cost += 1*myData->gradient->dt[t2].Get(y2+1,x2+1);
				else
					cost += 1*myData->gradient->dt[t2-1].Get(y2+1,x2+1);
				cost += 1*myData->gradient->dx[t1].Get(y1+1,x1+1);
				cost += 1*myData->gradient->dx[t2].Get(y2+1,x2+1);
				if (y2>0)
					cost += 1*myData->gradient->dy[t2].Get(y2,x2+1);
				if (y2<height-1)
					cost += 1*myData->gradient->dy[t2].Get(y2+1,x2+1);
				*/
			}
		}
		if (t1==t2 && x1==x2)
		{
			if (y1<y2)
			{
				//double weight = simpleGauss(x1,sigma,width/2);
				cost += 1*myData->naturality->dy[t1].Get(y1+1,x1+1);
				//cost += myData->gradient->dy[t1].Get(y1+1,x1+1);
			}
			if (y2<y1)
			{
				//double weight = simpleGauss(x1,sigma,width/2);
				cost += 1*myData->naturality->dy[t2].Get(y2+1,x2+1);
				//cost += myData->gradient->dy[t2].Get(y2+1,x2+1);
			}
		}
		if (x1==x2 && y1==y2)
		{
			if (t1<t2)
			{
				//double weight = simpleGauss(x1,sigma,width/2);
				cost += 1*myData->naturality->dt[t1].Get(y1+1,x1+1);
				//cost += 1*myData->gradient->dt[t1].Get(y1+1,x1+1);
			}
			if (t2<t1)
			{
				//double weight = simpleGauss(x1,sigma,width/2);
				cost += 1*myData->naturality->dt[t2].Get(y2+1,x2+1);
				//cost += 1*myData->gradient->dt[t2].Get(y2+1,x2+1);
			}
		}
	}

	//printf("smoothFn: computing cost between (%d,%d) and (%d,%d) with labels %d and %d : %d\n",y1,x1,y2,x2,l1,l2,cost);
	return cost;
}

/*
 * Function to interpolate binary shift map of low resolution to higher resolution
 */
int *UpsamplingShiftMap(int *result, videoSize &size, double ratio, videoSize &up_size)
{
	int *band = new int[up_size.time*up_size.height];

	int cur_idx = 0;
    for (int t = 0; t < up_size.time; t++)
    {
        for (int y = 0; y < up_size.height; y++)
        {
            for (int x = 0; x < up_size.width; x++)
            {
                int p = t*up_size.width*up_size.height+y*up_size.width+x;
                int idx = Downsampling3DIndex(p,up_size,size,ratio);
				if (result[idx]!=0)
				{
					band[cur_idx] = x;
					cur_idx++;
					break;
				}
            } // end for x
        } // end for y
    }

	return band;
}

/*
 *
 */
int *JBUpsamplingShiftMap(int *result, int s_width, int s_height, 
						  int s_time, PictureList *ref, double ratio)
{
	int mu = 0;
	float sigma = 0.5;
	ratio = 1;
	
	int width = ref->GetMaxWidth();
	int height = ref->GetMaxHeight();
	int time = ref->GetLength();
	int *up_result = new int[time*width*height];

	for (int t = 0; t < time; t++)
	{
		for (int y = 0; y < height; y++)
		{
			int refPixLoc = t*(width*height)+y*width;
			for (int x = 0; x < width; x++)
			{	
				refPixLoc += x;
				pixelType refPix = ref->GetPicture(t)->GetPixel(x,y);
				
				float total_val = 0;
				float normalizing_factor = 0;
			
				//prevent black areas fro all rgausses being 0
				float norgauss = 0; 
				float norgauss_normalize = 0;

				/* coordinates in the source */
				float o_x = x/ratio;
				float o_y = y/ratio;

				for (int nn_y = -ratio-1; nn_y < ratio; nn_y++)
				{
					int r_y = (int)round(o_y + nn_y);
					r_y = (r_y > 0 ? (r_y < s_height ? r_y : s_height-1) : 0);
					int srcPixLoc = t*(s_width*s_height)+r_y*s_width;
					int neighborPixLoc = t*(width*height)+r_y*ratio*width;

					for (int nn_x = -ratio-1; nn_x < ratio; nn_x++)
					{
						int r_x = (int)round(o_x + nn_x);
						r_x = (r_x > 0 ? (r_x < s_width ? r_x : s_width-1) : 0);
						int srcPix = result[srcPixLoc+r_x];
						
						neighborPixLoc += r_x*ratio;
						pixelType neighborPix = ref->GetPicture(t)->GetPixel(r_x*ratio,r_y*ratio);

						//gauss dist to center
						float sdist = sqrt(pow(o_x-r_x, 2) + pow(o_y-r_y, 2));
						float sgauss = simpleGauss(sdist,sigma, mu);
						//gauss radiance diff to center in ref
						float rdist = sqrt(pow(refPix.r-neighborPix.r, 2.0)+
									 pow(refPix.g-neighborPix.g, 2.0)+
									 pow(refPix.b-neighborPix.b, 2.0));
						float rgauss = simpleGauss(rdist,sigma, mu);

						//multiply gausses by value in source and add to total val
						norgauss = srcPix * sgauss;
						norgauss_normalize += sgauss;
						float totalgauss = sgauss * rgauss;
						normalizing_factor += totalgauss;
						total_val += srcPix * totalgauss;						
					}
				}

				if(total_val) {
					total_val /= normalizing_factor;
					up_result[refPixLoc] = total_val;
				}
				else {
					total_val = norgauss/norgauss_normalize;
				}
			
			}
		}
	}

	return up_result;
}

void SaveShiftMap(int *labels, int width, int height, char *name)
{
	Picture *map = new Picture(width,height);
	
	int x, y;
	intensityType map_pix;
	for ( int  i = 0; i < width*height; i++ )
	{
		x = i % width;
		y = i / width;

		map_pix.r = labels[i]*255;
		map_pix.g = labels[i]*255;
		map_pix.b = labels[i]*255;
		map->SetPixelIntensity(x,y,map_pix);
	}

	map->Save(name);
	delete map;
}

void SaveSeamImage(int *labels, Picture *ref, char *name, int flag)
{
	Picture *seam_img;

	/* reduce */	
	if (flag==0)
	{
		seam_img = new Picture(ref->GetWidth()+1,
										ref->GetHeight());
	
		pixelType pix;
		for (int y = 0; y < ref->GetHeight(); y++)
		{
			bool found_seam = false;
			for (int x = 0; x < ref->GetWidth()+1; x++)
			{
				int label;
				if (x<ref->GetWidth())
					label = labels[y*ref->GetWidth()+x];
				else
					label = 1;
				if (label>0 && !found_seam)
				{
					pix.r = 255;
					pix.g = 0;
					pix.b = 0;
					seam_img->SetPixel(x,y,pix);
					found_seam = true;
				}
				else
				{
					seam_img->SetPixel(x,y,ref->GetPixel(x-label,y));
				}
			}
		}
	}

	/* enlarge */
	if (flag==1)
	{
		seam_img = new Picture(ref->GetWidth(),
										ref->GetHeight());
	
		pixelType pix;
		for (int y = 0; y < ref->GetHeight(); y++)
		{
			bool found_seam = false;
			for (int x = 0; x < ref->GetWidth(); x++)
			{
				int label = labels[y*ref->GetWidth()+x];
				if (label!=0 && !found_seam)
				{
					pix.r = 255;
					pix.g = 0;
					pix.b = 0;
					seam_img->SetPixel(x,y,pix);
					found_seam = true;
				}
				else
				{
					seam_img->SetPixel(x,y,ref->GetPixel(x,y));
				}
			}
		}
	}
	

	seam_img->Save(name);
	delete seam_img;
}

/*
 * Function to generate and save retargeted image
 * using shift-map labels
 */
void SaveRetargetPicture(int *labels, PictureList *src,int width, int height, char *name)
{
	int cur_idx = 0;
	for (int t = 0; t < src->GetLength(); t++)
	{
		Picture *result = new Picture(width,height);
		char target_name[512] = {'\0'};
		strcat(target_name,name);
		strcat(target_name,src->GetPicture(t)->GetName());
		result->SetName(target_name);

		Picture *map = new Picture(width,height);
		char map_name[512] = {'\0'};
		strcat(map_name,name);
		strcat(map_name,"shift_");
		strcat(map_name,src->GetPicture(t)->GetName());
		map->SetName(map_name);
	
		int x, y;
		pixelType pixel;
		intensityType map_pix;
		for ( int  i = 0; i < width*height; i++ )
		{
			x = i % width;
			y = i / width;
			//printf("SaveRetargetPicture: GetPixel(%d,%d)\n",x+gc->whatLabel(i),y);
			pixel.r = src->GetPicture(t)->GetPixel(x+labels[cur_idx],y).r;
			pixel.g = src->GetPicture(t)->GetPixel(x+labels[cur_idx],y).g;
			pixel.b = src->GetPicture(t)->GetPixel(x+labels[cur_idx],y).b;
			result->SetPixel(x,y,pixel);

			map_pix.r = labels[cur_idx]*255;
			map_pix.g = labels[cur_idx]*255;
			map_pix.b = labels[cur_idx]*255;
			map->SetPixelIntensity(x,y,map_pix);

			cur_idx++;
		}

		result->Save(result->GetName());
		map->Save(map->GetName());

		delete result;
		delete map;
	}
}

/*
 * Function to generate and save retargeted image
 * using shift-map labels
 */
PictureList *GenerateRetargetResult(int *labels, PictureList *src, int width, 
									int height, char *name, bool save)
{
	PictureList *result = new PictureList(width,height,src->GetLength());
	for (int t = 0; t < src->GetLength(); t++)
	{
		Picture *frame = new Picture(width,height);
		char target_name[512] = {'\0'};
		strcat(target_name,name);
		strcat(target_name,src->GetPicture(t)->GetName());
		frame->SetName(src->GetPicture(t)->GetName());
	
		int x, y;
		pixelType pixel;
		for ( int  i = 0; i < width*height; i++ )
		{
			x = i % width;
			y = i / width;
			//printf("SaveRetargetPicture: GetPixel(%d,%d)\n",x+gc->whatLabel(i),y);
			pixel = SamplingPixel(src->GetPicture(t),x,y,labels[i]);
			frame->SetPixel(x,y,pixel);
		}

		result->SetPicture(t,frame);
		if (save)
		{
			frame->Save(target_name);
		}

		delete frame;
	}

	return result;
}


/*
 * Function to find the optimal shift-map
 */
/*
int *GridGraph_GraphCut(listPyramidType *src, int level, videoSize &target_size, 
						int num_labels, float alpha, float beta, char *target_path)
{
	// set up the needed data to pass to function for the data costs
	gradient2D *gradient = new gradient2D[src->Lists[level].GetLength()];
	for (int t = 0; t < src->Lists[level].GetLength(); t++)
	{
		gradient2D* grad = Gradient(src->Lists[level].GetPicture(t));
		gradient[t].dx = grad->dx;
		gradient[t].dy = grad->dy;
		gradient[t].total_dx = grad->total_dx;
		gradient[t].total_dy = grad->total_dy;
		delete grad;
	}


	int num_pixels = target_size.width*target_size.height*target_size.time;
	int *result = new int[num_pixels];   // stores result of optimization

	try{
		GCoptimization3DGridGraph *gc = new GCoptimization3DGridGraph(target_size.width,
																	  target_size.height,
																	  target_size.time,
																	  num_labels);


		// TODO: replace the hardcode of gradient threoshold
		// for shot boundary
		// data terms comes from function pointer
		ForVideoDataFn toDataFn;
		toDataFn.src = &(src->Lists[level]);
		toDataFn.gradient = gradient;
		toDataFn.target_size = target_size;
		gc->setDataCost(&VideoDataFn,&toDataFn);

		// smoothness comes from function pointer
		ForVideoSmoothFn toSmoothFn;
		toSmoothFn.src = &(src->Lists[level]);		
		toSmoothFn.gradient = gradient;
		toSmoothFn.target_size = target_size;
		toSmoothFn.alpha = alpha;
		toSmoothFn.beta = beta;
		gc->setSmoothCost(&VideoSmoothFn, &toSmoothFn);

		printf("Before optimization energy is %f\n",gc->compute_energy());
		gc->expansion();// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("After optimization energy is %f\n",gc->compute_energy());

		// obtain the labels
		int assign_idx, assignment;
		for ( int  i = 0; i < num_pixels; i++ )
		{
			//printf("The optimal label for pixel %d is %d\n",i,gc->whatLabel(i));
			result[i] = gc->whatLabel(i);
		}		

		// upsampling shift-map from low-resolution
		// to high-resolution
		double ratio = pow(2.0,level);
		videoSize up_size;
		up_size.width = src->Lists[0].GetMaxWidth()-1;
		up_size.height = src->Lists[0].GetMaxHeight();
		up_size.time = src->Lists[0].GetLength();
		int *up_result = UpsamplingShiftMap(result,target_size,ratio,up_size);
		//delete [] result;

		// generate and save retargeted images
		// from source and shift-map
		//SaveRetargetPicture(result,&(src->Lists[level]),target_size.width,
		//					target_size.height,target_path);
		SaveRetargetPicture(up_result,&(src->Lists[0]),up_size.width,
							up_size.height,target_path);

		delete [] up_result;
		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	for (int t = 0; t < src->Lists[level].GetLength(); t++)
	{
		delete gradient[t].dx;
		delete gradient[t].dy;
	}
	return result;

}
*/

/*
 * Function to find the optimal single seam
 */
PictureList *GridGraph_SeamCut(PictureList *src, double ratio, videoSize &target_size, videoSize &up_size,
							   int num_labels, char *target_path, int *&band, int *&lr_band, bool saved)
{
	// set up the needed data to pass to function for the data costs
	gradient3D *gradient = Diff_3D(src,0.0);
	gradient3D *naturality = Naturality_3D(src,0.0);

	int num_pixels = target_size.width*target_size.height*target_size.time;
	int *labels = new int[num_pixels];   // stores result of optimization
	PictureList *result;

	try{
		GCoptimization *gc;
		if (target_size.time>1)
			gc = new GCoptimization3DGridGraph(target_size.width,
											   target_size.height,
											   target_size.time,
											   num_labels);
		else
			gc = new GCoptimizationGridGraph(target_size.width,
											 target_size.height,
											 num_labels);


		// data terms comes from function pointer
		ForVideoDataFn toDataFn;
		toDataFn.src = src;
		toDataFn.target_size = target_size;
		gc->setDataCost(&VideoDataFn,&toDataFn);

		// smoothness comes from function pointer
		ForVideoSmoothFn toSmoothFn;
		toSmoothFn.src = src;
		toSmoothFn.gradient = gradient;
		toSmoothFn.naturality = naturality;
		toSmoothFn.target_size = target_size;
		toSmoothFn.bias = bias;
		gc->setSmoothCost(&VideoSmoothFn, &toSmoothFn);

		//printf("Before optimization energy is %f\n",gc->compute_energy());
		gc->expansion(1);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		//printf("After optimization energy is %f\n",gc->compute_energy());

		// obtain the labels
		for ( int  i = 0; i < num_pixels; i++ )
		{
			//printf("The optimal label for pixel %d is %d\n",i,gc->whatLabel(i));
			if (src->GetMaxWidth()>target_size.width)
				labels[i] = gc->whatLabel(i);
			if (src->GetMaxWidth()<target_size.width)
				labels[i] = -gc->whatLabel(i);
		}		

		// upsampling shift-map from low-resolution to high-resolution
		lr_band = UpsamplingShiftMap(labels,target_size,1.0,target_size);											 
		band = UpsamplingShiftMap(labels,target_size,ratio,up_size);											 
		//delete [] result;		

		// update_
		result = GenerateRetargetResult(labels, src, 
										target_size.width, 
										target_size.height,target_path,saved);
				
		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	delete [] labels;
	delete [] gradient->dx;
	delete [] gradient->dy;
	delete [] gradient->dt;
	delete [] gradient->total_dx;
	delete [] gradient->total_dy;
	delete [] gradient->total_dt;
	delete gradient;
	delete [] naturality->dx;
	delete [] naturality->dy;
	delete [] naturality->dt;
	delete [] naturality->total_dx;
	delete [] naturality->total_dy;
	delete [] naturality->total_dt;
	delete naturality;
	return result;

}

Picture *Subband_Picture(Picture *ref, int *&subband, int idx, double ratio, 
						 int &lbound, int &ubound, int *&labels, bool assign_bound)
{
	if (!assign_bound)
	{
		int cur_lbound = -1;
		int cur_ubound = -1;
		for (int y = 0; y < ref->GetHeight(); y++)
		{
			int lval = max(subband[idx+y]-1*ratio,0);
			int uval = min(subband[idx+y]+2*ratio-1,ref->GetWidth()-1);
			if (lval<cur_lbound || cur_lbound<0)
				cur_lbound = lval;
			if (uval>cur_ubound || cur_ubound<0)
				cur_ubound = uval;
		}

		if (cur_lbound>0)
			cur_lbound--;
		if (cur_ubound<ref->GetWidth()-1)
			cur_ubound++;

		if (lbound<0)
			lbound = cur_lbound;
		else
			lbound = min(cur_lbound,max(lbound,0));
		if (ubound<0)
			ubound = cur_ubound;
		else
			ubound = max(cur_ubound,min(ubound,ref->GetWidth()-1));
	}
	
	//lbound = 0;
	//ubound = ref->GetWidth()-1;
		
	Picture *result = new Picture(ubound-lbound+1,ref->GetHeight());
	int *subband_labels = new int[ref->GetHeight()*(ubound-lbound+1)];
	for (int y = 0; y < ref->GetHeight(); y++)
	{
		pixelType pixel;
		for (int x = lbound; x <= ubound; x++)
		{
			int lidx = y*ref->GetWidth()+x;
			pixel = ref->GetPixel(x,y);
			result->SetPixel(x-lbound,y,pixel);
			subband_labels[y*(ubound-lbound+1)+x-lbound] = labels[lidx];
		}
	}

	delete [] labels;
	labels = subband_labels;
	//result->Save("subband_img.ppm");
	return result;
}

Picture *Combine_SubbandImgs(Picture *src, Picture *band_img, imageSize &target_size,
							 double ratio, int lbound, int ubound, int *&labels)
{
	Picture *result = new Picture(target_size.width, target_size.height);
	result->SetName(src->GetName());
	int *new_labels = new int[result->GetWidth()*result->GetHeight()];

	pixelType pixel;
	for (int y = 0; y < target_size.height; y++)
	{
		int idx;
		for (int x = 0; x < lbound; x++)
		{
			idx = y*result->GetWidth()+x;
			pixel = src->GetPixel(x,y);
			result->SetPixel(x,y,pixel);
			new_labels[idx] = 0;
		}
		for (int x = lbound; x < lbound+band_img->GetWidth(); x++)
		{
			idx = y*result->GetWidth()+x;
			pixel = band_img->GetPixel(x-lbound,y);
			result->SetPixel(x,y,pixel);
			new_labels[idx] = labels[y*band_img->GetWidth()+x-lbound];
		}
		for (int x = lbound+band_img->GetWidth(); x < result->GetWidth(); x++)
		{
			idx = y*result->GetWidth()+x;
			pixel = src->GetPixel(x-target_size.width+src->GetWidth(),y);
			result->SetPixel(x,y,pixel);
			new_labels[idx] = src->GetWidth()-target_size.width;
		}
	}

	delete [] labels;
	labels = new_labels;
	return result;
}

Picture *SingleImg_SeamCut(Picture *img, int *&subband, int idx, int lbound, 
						   imageSize &target_size, int *&labels, bool banded, fstream &fileResult)
{
	int num_pixels = target_size.width*target_size.height;
	Picture *result = NULL;

	try{
		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(target_size.width,
																  target_size.height,
																  2);

		// set up the needed data to pass to function for the data costs
		gradient2D *gradient = Diff_2D(img);
		gradient2D *naturality = Naturality_2D(img);
		//Matrix *bias = CalcNarrownessPrior(gradient,1000.0);

		ForSingleImgDataFn toDataFn;
		toDataFn.src = img;
		toDataFn.subbanded = banded;
		toDataFn.subband = subband;
		toDataFn.subband_idx = idx;
		toDataFn.subband_lbound = lbound;
		toDataFn.target_size = target_size;
		gc->setDataCost(&SingleImgDataFn,&toDataFn);

		// smoothness comes from function pointer
		ForSingleImgSmoothFn toSmoothFn;
		toSmoothFn.src = img;
		toSmoothFn.gradient = gradient;
		toSmoothFn.naturality = naturality;
		toSmoothFn.target_size = target_size;
		gc->setSmoothCost(&SingleImgSmoothFn, &toSmoothFn);

		//printf("Before optimization energy is %f\n",gc->compute_energy());
		gc->expansion(1);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		//printf("After optimization energy is %f\n",gc->compute_energy());

		// obtain the labels
		for ( int  i = 0; i < num_pixels; i++ )
		{
			//printf("The optimal label for pixel %d is %d\n",i,gc->whatLabel(i));
			if (img->GetWidth()>target_size.width)
				labels[i] = gc->whatLabel(i);
			if (img->GetWidth()<target_size.width)
				labels[i] = -gc->whatLabel(i);
		}

		/* update subband */
		int subband_idx = idx;
		for (int y = 0; y < target_size.height; y++)
        {
			for (int x = 0; x < target_size.width; x++)
            {
				int p = y*target_size.width+x;
				if (labels[p] != 0)
				{
					subband[subband_idx] = x+lbound;
					subband_idx++;
					char cY[256]; // or what else, or use outer char array
					sprintf(cY,"%d",y+1);
					char cX[256];
					sprintf(cX,"%d",x+lbound+1);
					fileResult << cY << " " << cX << " ";
					break;
				}
			}
		}
		fileResult << "\n";

		/* generate retargeted subband image */
		result = GenerateRetargetPicture(labels, img, 
										 target_size.width, target_size.height);
		//SaveShiftMap(labels,target_size.width,target_size.height,"shift.ppm");

		delete gradient->dx;
		delete gradient->dy;
		delete gradient;
		delete naturality->dx;
		delete naturality->dy;
		delete naturality;
		//delete bias;
		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	return result;
}

Picture *PairImg_SeamCut(Picture *cur_frame, Picture *pre_frame, int *&subband, 
						 int idx, int lbound, imageSize &target_size, 
						 int *&pre_labels, bool tconsistency, bool banded, fstream &fileResult)
{
	// set up the needed data to pass to function for the data costs
	//gradient2D *gradient = Diff_2D(cur_frame);
	PictureList *pair = new PictureList(cur_frame->GetWidth(),
										cur_frame->GetHeight(),
										2);
	pair->SetPicture(0,pre_frame);
	pair->SetPicture(1,cur_frame);
	gradient3D *gradient = Diff_3D(pair);
	delete pair;
	gradient2D *naturality = Naturality_2D(cur_frame);
	Matrix *grayscale = Rgb2Gray(cur_frame);
	Matrix *pre_grayscale = Rgb2Gray(pre_frame);
	//Matrix *bias = CalcNarrownessPrior(gradient,1000.0);

	int num_pixels = target_size.width*target_size.height;
	int *labels = new int[num_pixels];   // stores result of optimization
	Picture *result = NULL;

	try{
		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(target_size.width,
																  target_size.height,
																  2);


		// TODO: replace the hardcode of gradient threoshold
		// for shot boundary
		// data terms comes from function pointer
		if (tconsistency)
		{
			ForPairImgDataFn toDataFn;
			toDataFn.src = cur_frame;
			toDataFn.gray = grayscale;
			toDataFn.pre = pre_frame;
			toDataFn.pre_gray = pre_grayscale;
			toDataFn.pre_labels = pre_labels;
			toDataFn.subbanded = banded;
			toDataFn.subband = subband;
			toDataFn.subband_idx = idx;
			toDataFn.subband_lbound = lbound;
			toDataFn.target_size = target_size;
			gc->setDataCost(&PairImgDataFn,&toDataFn);
		}
		else
		{
			ForSingleImgDataFn toDataFn;
			toDataFn.src = cur_frame;
			toDataFn.subbanded = banded;
			toDataFn.subband = subband;
			toDataFn.subband_idx = idx;
			toDataFn.subband_lbound = lbound;
			toDataFn.target_size = target_size;
			gc->setDataCost(&SingleImgDataFn,&toDataFn);
		}

		// smoothness comes from function pointer
		ForPairImgSmoothFn toSmoothFn;
		toSmoothFn.src = cur_frame;
		toSmoothFn.gradient = gradient;
		toSmoothFn.naturality = naturality;
		toSmoothFn.target_size = target_size;
		gc->setSmoothCost(&PairImgSmoothFn, &toSmoothFn);

		//printf("Before optimization energy is %f\n",gc->compute_energy());
		gc->expansion(1);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		//printf("After optimization energy is %f\n",gc->compute_energy());

		// obtain the labels
		for ( int  i = 0; i < num_pixels; i++ )
		{
			//printf("The optimal label for pixel %d is %d\n",i,gc->whatLabel(i));
			if (cur_frame->GetWidth()>target_size.width)
				labels[i] = gc->whatLabel(i);
			if (cur_frame->GetWidth()<target_size.width)
				labels[i] = -gc->whatLabel(i);
		}

		/* update subband */
		int subband_idx = idx;
		for (int y = 0; y < target_size.height; y++)
        {
			for (int x = 0; x < target_size.width; x++)
            {
				int p = y*target_size.width+x;
				if (labels[p]!=0)
				{
					subband[subband_idx] = x+lbound;
					subband_idx++;
					char cY[256]; // or what else, or use outer char array
					sprintf(cY,"%d",y+1);
					char cX[256];
					sprintf(cX,"%d",x+lbound+1);
					fileResult << cY << " " << cX << " ";
					break;
				}
			}
		}
		fileResult << "\n";

		/* generate retargeted subband image */
		result = GenerateRetargetPicture(labels, cur_frame, 
										 target_size.width, 
										 target_size.height);
		//SaveShiftMap(labels,target_size.width,target_size.height,"shift.ppm");

		delete [] gradient->dx;
		delete [] gradient->dy;
		delete [] gradient->dt;
		delete [] gradient->total_dx;
		delete [] gradient->total_dy;
		delete [] gradient->total_dt;
		delete gradient;
		delete naturality->dx;
		delete naturality->dy;
		delete naturality;
		delete grayscale;
		delete pre_grayscale;
		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	//delete bias;
	delete [] pre_labels;
	pre_labels = labels;
	return result;

}

/*
 *
 */
PictureList *Refine_Seam(int *&band, PictureList *src, videoSize &origin_size, double ratio, 
						 char *name,  Matrix *&bias, bool tconsistency, bool banded, bool saved, fstream &fileResult)
{
	imageSize target_size;
	Matrix *new_bias = NULL;
	if (origin_size.width>src->GetMaxWidth())
		new_bias = new Matrix[origin_size.time];
	PictureList *result = new PictureList(origin_size.width,
										  origin_size.height,
										  origin_size.time);
	/* process first frame */
	int lbound = -1;
	int ubound = -1;
	int *empty_labels = new int[src->GetMaxHeight()*src->GetMaxWidth()];
	Picture *band_img;
	if (!banded)
	{
		band_img = src->GetPicture(0);
		lbound = 0;
		ubound = src->GetPicture(0)->GetWidth()-1;
	}
	else
		band_img = Subband_Picture(src->GetPicture(0),band,0,ratio,
								   lbound,ubound,empty_labels,false);
	delete [] empty_labels;
	target_size.width = band_img->GetWidth()-src->GetMaxWidth()+origin_size.width;
	target_size.height = band_img->GetHeight();
	int *labels = new int[target_size.width*target_size.height];
	
	Picture *retargeted_band_img = SingleImg_SeamCut(band_img,band,0,lbound,
													 target_size,labels,banded,fileResult);
	if (!banded)
		retargeted_band_img->SetName(src->GetPicture(0)->GetName());
	Picture *combined_img;
	if (!banded)
	{
		combined_img = retargeted_band_img;
		result->SetPicture(0,combined_img);
		delete retargeted_band_img;
	}
	else
	{
		target_size.width = src->GetMaxWidth()-src->GetMaxWidth()+origin_size.width;
		target_size.height = src->GetMaxHeight();
		combined_img = Combine_SubbandImgs(src->GetPicture(0),retargeted_band_img,target_size,ratio,lbound,ubound,labels);
		result->SetPicture(0,combined_img);
		delete band_img;
		delete retargeted_band_img;
		delete combined_img;
	}

	/* update bias */
	if (origin_size.width>src->GetMaxWidth())
	{
		Matrix *first_frame_bias = new Matrix(origin_size.height,
										 origin_size.width);
		for (int y = 0; y < origin_size.height; y++)
		{
			for (int x = 0; x < band[y]-1; x++)
			{
				first_frame_bias->Set(y+1,x+1,bias[0].Get(y+1,x+1));
			}
			for (int x = band[y]; x < src->GetMaxWidth(); x++)
			{
				first_frame_bias->Set(y+1,x+2,bias[0].Get(y+1,x+1));
			}
			first_frame_bias->Set(y+1,band[y],1.0);
			first_frame_bias->Set(y+1,band[y]+1,1.0);
		}
		new_bias[0] = *(first_frame_bias);
		delete first_frame_bias;
	}

	if (saved)
	{
		char target_name[512] = {'\0'};
		strcat(target_name,name);
		strcat(target_name,result->GetPicture(0)->GetName());
		result->GetPicture(0)->Save(target_name);

		/*
		char map_name[512] = {'\0'};
		strcat(map_name,name);
		strcat(map_name,"shift_");
		strcat(map_name,result->GetPicture(0)->GetName());
		SaveShiftMap(labels,result->GetPicture(0)->GetWidth(),
					 result->GetPicture(0)->GetHeight(),map_name);
		*/

		char seam_name[512] = {'\0'};
		strcat(seam_name,name);
		strcat(seam_name,"seam_");
		strcat(seam_name,result->GetPicture(0)->GetName());
		if (origin_size.width<src->GetMaxWidth())
			SaveSeamImage(labels,result->GetPicture(0),seam_name,0);
		if (origin_size.width>src->GetMaxWidth())
			SaveSeamImage(labels,result->GetPicture(0),seam_name,1);
	}

	/* process 2nd frame to t-1 frame */
	Picture *pre_band_img;
	for (int t = 1; t < src->GetLength(); t++)
	{
		//lbound = -1;
		//ubound = -1;
		empty_labels = new int[src->GetMaxHeight()*src->GetMaxWidth()];
		if (!banded)
		{
			band_img = src->GetPicture(t);
			lbound = 0;
			ubound = src->GetPicture(t)->GetWidth()-2;
		}
		else
			band_img = Subband_Picture(src->GetPicture(t),band,t*src->GetMaxHeight(),ratio,
									   lbound,ubound,empty_labels,false);
		delete [] empty_labels; 
		ubound = ubound+origin_size.width-src->GetMaxWidth();
		if (!banded)
		{
			//pre_band_img = result->GetPicture(t-1);
			pre_band_img = src->GetPicture(t-1);			
		}
		else
		{
			pre_band_img = Subband_Picture(result->GetPicture(t-1),band,t*src->GetMaxHeight(),ratio,
										   lbound,ubound,labels,true);
			empty_labels = new int[src->GetMaxHeight()*src->GetMaxWidth()];
			delete pre_band_img;
			ubound = ubound+src->GetMaxWidth()-origin_size.width;
			pre_band_img = Subband_Picture(src->GetPicture(t-1),band,t*src->GetMaxHeight(),ratio,
										   lbound,ubound,empty_labels,true);
			delete [] empty_labels;
		}
		target_size.width = band_img->GetWidth()-src->GetMaxWidth()+origin_size.width;
		target_size.height = band_img->GetHeight();

		//retargeted_band_img = SingleImg_SeamCut(band_img,band,t*src->GetMaxHeight(),lbound,
		//										  target_size,labels,band_bias);
		retargeted_band_img = PairImg_SeamCut(band_img,pre_band_img,band,t*src->GetMaxHeight(),
											  lbound,target_size,labels,tconsistency,banded,fileResult);

		//ubound--;
		retargeted_band_img->SetName(src->GetPicture(t)->GetName());
		if (!banded)
		{
			combined_img = retargeted_band_img;
			result->SetPicture(t,combined_img);
			delete retargeted_band_img;
		}
		else
		{
			target_size.width = src->GetMaxWidth()-src->GetMaxWidth()+origin_size.width;
			target_size.height = src->GetMaxHeight();
			combined_img = Combine_SubbandImgs(src->GetPicture(t),retargeted_band_img,target_size,ratio,lbound,ubound+1,labels);
			result->SetPicture(t,combined_img);
			delete band_img;
			delete pre_band_img;
			delete retargeted_band_img;
			delete combined_img;
		}
		
		/* update bias */
		if (origin_size.width>src->GetMaxWidth())
		{
			Matrix *next_frame_bias = new Matrix(origin_size.height,
												 origin_size.width);
			for (int y = 0; y < origin_size.height; y++)
			{
				int bidx = t*origin_size.height+y;
				for (int x = 0; x < band[bidx]-1; x++)
				{
					next_frame_bias->Set(y+1,x+1,bias[t].Get(y+1,x+1));
				}
				for (int x = band[bidx]; x < src->GetMaxWidth(); x++)
				{
					next_frame_bias->Set(y+1,x+2,bias[t].Get(y+1,x+1));
				}
				next_frame_bias->Set(y+1,band[bidx],1.0);
				next_frame_bias->Set(y+1,band[bidx]+1,1.0);
			}	
			new_bias[t] = *(next_frame_bias);
			delete next_frame_bias;
		}

		if (saved)
		{
			char frame_name[512] = {'\0'};
			strcat(frame_name,name);
			strcat(frame_name,result->GetPicture(t)->GetName());
			result->GetPicture(t)->Save(frame_name);

			/*
			char map_name2[512] = {'\0'};
			strcat(map_name2,name);
			strcat(map_name2,"shift_");
			strcat(map_name2,result->GetPicture(t)->GetName());
			SaveShiftMap(labels,result->GetPicture(t)->GetWidth(),
						 result->GetPicture(t)->GetHeight(),map_name2);
			*/

			char seam_name2[512] = {'\0'};
			strcat(seam_name2,name);
			strcat(seam_name2,"seam_");
			strcat(seam_name2,result->GetPicture(t)->GetName());
			if (origin_size.width<src->GetMaxWidth())
				SaveSeamImage(labels,result->GetPicture(t),seam_name2,0);
			if (origin_size.width>src->GetMaxWidth())
				SaveSeamImage(labels,result->GetPicture(t),seam_name2,1);
		}
	}

	if (origin_size.width>src->GetMaxWidth())
	{
		delete [] bias;
		bias = new_bias;
	}
	delete [] labels;
	return result;
}

/*
 *
 */
PictureList *ResizeVideo_3D(PictureList *shot, int removed_seam, 
							char *output_path, int level, fstream &fileResult)
{
	time_t start, end;
	listPyramidType *spyramid = NULL;
	int num_labels;
	int *band = NULL;	// store shift labels of every pixel
	int *lr_band = NULL;
	Matrix *bias = NULL;
	videoSize target_size, origin_size;

	PictureList *source = NULL;
	PictureList *target = shot;
	while (removed_seam != 0)
	{	
		bool saved = false;
		if (removed_seam==1 || removed_seam==-1)
			saved = true;

		if (level>0)
		{
			spyramid = ListPyramid(target,level+1);
			delete target;
			source = &(spyramid->Lists[level]);	
			target = &(spyramid->Lists[0]);
		}
		else
			source = target;
		
		num_labels = 2;
		if (removed_seam>0)
			target_size.width = source->GetMaxWidth()-1;
		if (removed_seam<0)
			target_size.width = source->GetMaxWidth()+1;
		target_size.height = source->GetMaxHeight();
		target_size.time = source->GetLength();
		if (removed_seam>0)
			origin_size.width = target->GetMaxWidth()-1;
		if (removed_seam<0)
			origin_size.width = target->GetMaxWidth()+1;
		origin_size.height = target->GetMaxHeight();
		origin_size.time = target->GetLength();

		if (bias==NULL)
        {
			bias = new Matrix[origin_size.time];
            for (int t = 0; t < origin_size.time; t++)
            {
				Matrix *init = new Matrix(target->GetMaxHeight(),
										  target->GetMaxWidth(),0.0);
                bias[t] = *(init);
                delete init;
            }
		}

		/* graph cut in low resolution */
		time(&start);
		target = GridGraph_SeamCut(source,pow(2.0,level),target_size,origin_size,
								   num_labels,output_path,band,lr_band,bias,saved);
		time(&end);
		printf("3D shift-map time is %f\n",difftime(end, start));

		// generate output result;
		/*
		target = GenerateRetargetResult(shift_map, target,
										origin_size.width,
										origin_size.height,"..\\..\\..\\results\\video5\\1\\",true);
		*/

		if (removed_seam>0)
			removed_seam--;
		if (removed_seam<0)
			removed_seam++;
		delete [] band;
		delete [] lr_band;
		if (level>0)
		{
			delete [] spyramid->Lists;
			delete spyramid;
			spyramid = NULL;
		}
		else
			delete source;
	}

	if (level>0)
	{
		//delete shot;
		//delete source;
	}

	delete [] bias;
	//delete target;
	return target;
}

/*
 *
 */
PictureList *ResizeVideo_2D_Incremental(PictureList *shot, int removed_seam, 
										char *output_path, int level, fstream &fileResult)
{
	time_t start, end;
	listPyramidType *spyramid = NULL;
	int num_labels;
	int *band = NULL;	// store shift labels of every pixel
	Matrix *bias = NULL;
	videoSize target_size, origin_size;

	//spyramid = ListPyramid(shot,level+1);

	PictureList *source;
	PictureList *target = shot;
	while (removed_seam != 0)
	{	
		bool saved = false;
		if (removed_seam==1 || removed_seam==-1)
			saved = true;

		if (level>0)
		{
			spyramid = ListPyramid(target,level+1);
			delete target;
			source = &(spyramid->Lists[level]);	
			target = &(spyramid->Lists[0]);
		}
		else
			source = target;

		num_labels = 2;
		if (removed_seam>0)
			target_size.width = source->GetMaxWidth()-1;
		if (removed_seam<0)
			target_size.width = source->GetMaxWidth()+1;
		target_size.height = source->GetMaxHeight();
		target_size.time = source->GetLength();
		if (removed_seam>0)
			origin_size.width = target->GetMaxWidth()-1;
		if (removed_seam<0)
			origin_size.width = target->GetMaxWidth()+1;
		origin_size.height = target->GetMaxHeight();
		origin_size.time = target->GetLength();

		/* graph cut in low resolution */
		// TODO
		band = new int[origin_size.time*origin_size.height];
		if (bias==NULL)
        {
			bias = new Matrix[origin_size.time];
            for (int t = 0; t < origin_size.time; t++)
            {
				Matrix *init = new Matrix(target->GetMaxHeight(),
										  target->GetMaxWidth(),0.0);
                bias[t] = *(init);
                delete init;
            }
		}
		

		/* refine seam in higher resolution */
		time(&start);
		target = Refine_Seam(band,target,origin_size,pow(2.0,level),
							 output_path,bias,true,false,saved,fileResult);
		time(&end);
		printf("Incremental 2D shift-map time is %f\n",difftime(end, start));

		if (removed_seam>0)
			removed_seam--;
		if (removed_seam<0)
			removed_seam++;
		delete [] band;
		if (level>0)
		{
			delete [] spyramid->Lists;
			delete spyramid;
			spyramid = NULL;
		}
		else
			delete source;
	}

	if (level>0)
	{
		//delete shot;
		//delete source;
	}

	delete [] bias;
	//delete target;
	return target;
}

/*
 *
 */
PictureList *ResizeImages_2D_Incremental(PictureList *shot, int removed_seam, 
										 char *output_path, int level, fstream &fileResult)
{
	time_t start, end;
	listPyramidType *spyramid = NULL;
	int num_labels;
	int *band = NULL;	// store shift labels of every pixel
	Matrix *bias = NULL;
	videoSize target_size, origin_size;

	//spyramid = ListPyramid(shot,level+1);

	PictureList *source;
	PictureList *target = shot;
	while (removed_seam != 0)
	{	
		bool saved = false;
		if (removed_seam==1 || removed_seam==-1)
			saved = true;

		if (level>0)
		{
			spyramid = ListPyramid(target,level+1);
			delete target;
			source = &(spyramid->Lists[level]);	
			target = &(spyramid->Lists[0]);
		}
		else
			source = target;

		num_labels = 2;
		if (removed_seam>0)
			target_size.width = source->GetMaxWidth()-1;
		if (removed_seam<0)
			target_size.width = source->GetMaxWidth()+1;
		target_size.height = source->GetMaxHeight();
		target_size.time = source->GetLength();
		if (removed_seam>0)
			origin_size.width = target->GetMaxWidth()-1;
		if (removed_seam<0)
			origin_size.width = target->GetMaxWidth()+1;
		origin_size.height = target->GetMaxHeight();
		origin_size.time = target->GetLength();

		/* graph cut in low resolution */
		// TODO
		band = new int[origin_size.time*origin_size.height];
		if (bias==NULL)
        {
			bias = new Matrix[origin_size.time];
            for (int t = 0; t < origin_size.time; t++)
            {
				Matrix *init = new Matrix(target->GetMaxHeight(),
										  target->GetMaxWidth(),0.0);
                bias[t] = *(init);
                delete init;
            }
		}

		/* refine seam in higher resolution */
		time(&start);
		target = Refine_Seam(band,target,origin_size,pow(2.0,level),
							 output_path,bias,false,false,saved,fileResult);
		time(&end);
		printf("Independent shift-map time is %f\n",difftime(end, start));


		if (removed_seam>0)
			removed_seam--;
		if (removed_seam<0)
			removed_seam++;
		delete [] band;
		if (level>0)
		{
			delete [] spyramid->Lists;
			delete spyramid;
			spyramid = NULL;
		}
		else
			delete source;
	}

	if (level>0)
	{
		//delete shot;
		//delete source;
	}

	delete [] bias;
	//delete target;
	return target;
}

/*
 *
 */
PictureList *ResizeVideo_Hybrid(PictureList *shot, int width, 
								char *output_path, int level)
{
	time_t start, end;
	listPyramidType *spyramid = NULL;
	int num_labels;
	int *band = NULL;	// store shift labels of every pixel
	int *lr_band = NULL;
	videoSize target_size, origin_size;

	//spyramid = ListPyramid(shot,level+1);

	PictureList *source;
	PictureList *target = shot;
	if (level>0)
	{
		spyramid = ListPyramid(target,level+1);
		delete target;
		source = &(spyramid->Lists[level]);
		target = &(spyramid->Lists[0]);
	}
	else
		source = target;

	int iVSeams = shot->GetMaxWidth()-width;
	num_labels = SAMPLING_QUANTIZATION*iVSeams+1;
	target_size.width = source->GetMaxWidth()-iVSeams;
	target_size.height = source->GetMaxHeight();
	target_size.time = source->GetLength();
	origin_size.width = target->GetMaxWidth()-iVSeams;
	origin_size.height = target->GetMaxHeight();
	origin_size.time = target->GetLength();

	time(&start);
	source = GridGraph_SeamCut(shot,pow(2.0,level),target_size,origin_size,
							   num_labels,output_path,band,lr_band,true);
	time(&end);
	printf("3D shift-map time is %f\n",difftime(end, start));

	/* refine seam in higher resolution */
	time(&start);
	target = Refine_Seam(band,target,origin_size,pow(2.0,level),
						 output_path,bias,true,true,saved);
	time(&end);
	printf("Incremental 2D shift-map time is %f\n",difftime(end, start));

	delete [] band;
	delete [] lr_band;
	if (level>0)
	{
		delete [] spyramid->Lists;
		delete spyramid;
		spyramid = NULL;
	}
	delete source;

	if (level>0)
	{
		//delete shot;
		//delete source;
	}

	delete [] bias;
	//delete target;
	return target;

}

/*
 * main entry of the grogram
 */ 
int main(int argc, char **argv)
{
	if (argc<7)
	{
		cout << "Usage: mg_shift_map_3d <input_folder> <#height> <#width> <output_folder> method level" << endl;
		return 0;
		//default parameters
	}

	PictureList *shot = new PictureList(argv[1]);	
	PictureList *target = NULL;
	int level = atoi(argv[6]); // gpyramid->Levels-1
	int iHeight = atoi(argv[2]);
	int iWidth = atoi(argv[3]);

	char strSeamResult[512] = {'\0'};
	strcat(strSeamResult,argv[4]);

	switch (atoi(argv[5]))
	{
		case 2: // hybrid 3D & 2D temporal consistent shift-map
		{
			if (iHSeams>0)
			{
				PictureList *tshot = shot->TransposePictureList();
				delete shot;
				shot = ResizeVideo_Hybrid(tshot,iHeight,argv[4],level);
				//delete tshot;
				tshot = shot->TransposePictureList();
				delete shot;
				shot = tshot;
			}
			target = ResizeVideo_Hybrid(shot,iWidth,argv[4],level);
			break;
		}
	}
	
	target->Save(argv[4]);
	//delete shot;
	delete target;
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////
