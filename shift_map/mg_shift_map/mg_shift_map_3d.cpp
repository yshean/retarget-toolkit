//////////////////////////////////////////////////////////////////////////////
// Motion Guided Shift-Map for Video Retargeting
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
	gradient2D *gradient;
	Matrix *bias;
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
	Matrix *bias;
	imageSize target_size;
	float alpha;
	float beta;
};

/*
 * structure of data term for image pair
 */
struct ForPairImgDataFn
{
	Picture *src;
	Picture *pre;
	int *pre_labels;
	gradient2D *gradient;
	Matrix *bias;
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
	gradient2D *gradient;
	Matrix *bias;
	imageSize target_size;
	float alpha;
	float beta;
};

/*
 * structure of data term for video
 */
struct ForVideoDataFn
{
	PictureList *src;
	gradient2D *gradient;
	Matrix *bias;
	videoSize target_size;
};

/*
 * structure of smooth term for video
 */
struct ForVideoSmoothFn
{
	PictureList *src;
	gradient2D *gradient;
	Matrix *bias;
	videoSize target_size;
	float alpha;
	float beta;
};

Matrix *CalcNarrownessPrior(gradient2D *gradient, double threshold)
{
	Matrix *result = new Matrix(gradient->dx->NumOfRows(),
								gradient->dx->NumOfCols());
	for (int y = 1; y <= result->NumOfRows(); y++)
	{
		for (int x = 1; x <= result->NumOfCols(); x++)
		{
			int offset = 0;
			double energy = gradient->dx->Get(y,x);
			while (energy<threshold)
			{
				offset++;
				if (x-offset>=1 && x-offset<=result->NumOfCols())
					energy += gradient->dx->Get(y,x-offset);
				if (x+offset>=1 && x+offset<=result->NumOfCols())
					energy += gradient->dx->Get(y,x+offset);
			}
			result->Set(y,x,double(1/(0.1+offset)));
			//result->Set(y,x,1.0);
		}
	}

	return result;
}

Matrix *CopySubband_Bias(Matrix *&bias, int lbound, int ubound, int t)
{
	Matrix *result = new Matrix(bias[0].NumOfRows(),ubound-lbound+1);

	for (int y = 0; y < bias[0].NumOfRows(); y++)
	{
		for (int x = lbound; x <= ubound; x++)
		{
			result->Set(y+1,x-lbound+1,bias[t].Get(y+1,x+1));
		}
	}

	return result;
}

Matrix *CalcSeamBias(Matrix *&mask, int *&band, int width, int height, int time, double sigma)
{
	Matrix *result = new Matrix[time];
	for (int t = 0; t < time; t++)
	{
		Matrix *init = new Matrix(height,width);
		result[t] = *(init);
		delete init;
	}

	int band_idx = 0;
	for (int t = 0; t < time; t++)
	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				double weight = mask[t].Get(y+1,x+1);
				weight += simpleGauss(x,sigma,band[band_idx]);
				result[t].Set(y+1,x+1,weight);
			}
			band_idx++;
		}
	}

	delete [] mask;
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
		l!=myData->src->GetWidth()-width)
		return 100000*MAX_COST_VALUE;
	
	int refine_offset = 3;
	if (x+myData->subband_lbound<myData->subband[myData->subband_idx+y]-refine_offset && l>0)
		return 100000*MAX_COST_VALUE;
	if (x+myData->subband_lbound>myData->subband[myData->subband_idx+y]+refine_offset && l==0)
		return 100000*MAX_COST_VALUE;
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
		double color_diff;
		//printf("(%d,%d) color components: %d,%d,%d\n",src->GetPixel(y2+l2,x2).r,src->GetPixel(y2+l2,x2).g,src->GetPixel(y2+l2,x2).b);
		if (direction==0)
		{
			color_diff = 0.0;
			//weight = bias->Get(y2+1,x2+l2+1)*bias->Get(y1+y_offset+1,x1+l1+x_offset+1);
			color_diff += pow((double)src->GetPixel(x2+l2,y2).r-src->GetPixel(x1+l1+x_offset,y1+y_offset).r,2);
			color_diff += pow((double)src->GetPixel(x2+l2,y2).g-src->GetPixel(x1+l1+x_offset,y1+y_offset).g,2);
			color_diff += pow((double)src->GetPixel(x2+l2,y2).b-src->GetPixel(x1+l1+x_offset,y1+y_offset).b,2);
			//diff += 1000*weight;
			diff += (color_diff<threshold) ? 0 : color_diff;
		}
		if (direction==1)
		{
			color_diff = 0.0;
			//weight = bias->Get(y2-y_offset+1,x2+l2-x_offset+1)*bias->Get(y1+1,x1+l1+1);
			color_diff += pow((double)src->GetPixel(x2+l2-x_offset,y2-y_offset).r-src->GetPixel(x1+l1,y1).r,2);		
			color_diff += pow((double)src->GetPixel(x2+l2-x_offset,y2-y_offset).g-src->GetPixel(x1+l1,y1).g,2);	
			color_diff += pow((double)src->GetPixel(x2+l2-x_offset,y2-y_offset).b-src->GetPixel(x1+l1,y1).b,2);
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
			grad_diff += pow((gradient->dx->Get(y2,x2+l2)-gradient->dx->Get(y1+y_offset,x1+l1+x_offset)),2)+
						 pow((gradient->dy->Get(y2,x2+l2)-gradient->dy->Get(y1+y_offset,x1+l1+x_offset)),2);
			//diff += 1000*weight;
			diff += (grad_diff<threshold) ? 0 : grad_diff;
		}
		if (direction==1)
		{
			//weight = bias->Get(y2-y_offset,x2+l2-x_offset)*bias->Get(y1,x1+l1);
			grad_diff = 0.0;
			grad_diff += pow((gradient->dx->Get(y2-y_offset,x2+l2-x_offset)-gradient->dx->Get(y1,x1+l1)),2)+
						 pow((gradient->dy->Get(y2-y_offset,x2+l2-x_offset)-gradient->dy->Get(y1,x1+l1)),2);
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

	if (abs(x1-x2)<=1 && abs(y1-y2)<=1)
	{	
		double cost1 = 0.0;
		cost1 += myData->alpha*SingleImgColorDiff(myData->src, myData->bias, x1, y1, x2, y2, l1, l2, 0, 0.0);
		cost1 += myData->beta*SingleImgGradientDiff(myData->gradient, myData->bias, x1+1, y1+1, x2+1, y2+1, l1, l2, 0, 0.0);

		double cost2 = 0.0;
		cost2 += myData->alpha*SingleImgColorDiff(myData->src, myData->bias, x1, y1, x2, y2, l1, l2, 1, 0.0);
		cost2 += myData->beta*SingleImgGradientDiff(myData->gradient, myData->bias, x1+1, y1+1, x2+1, y2+1, l1, l2, 1, 0.0);
		
		double weight;
		if (l1>0)
			weight = myData->bias->Get(y1+1,x1+1);
		if (l2>0)
			weight = myData->bias->Get(y2+1,x2+1);
		cost += weight*(cost1+cost2);
		//cost += min(cost1,cost2);
	}
	else 
	{
		cost = 100000*MAX_COST_VALUE;
	}

	/*
	if ((y1==y2) && (x1<x2) && l2>0)
	{
		intensityType p1 = myData->src->GetPixelIntensity(x2,y2);
		intensityType p2;
		int offset = 2;
		double edge_cost = 0.0;
		for (int nn_y = y2-offset; nn_y <= y2+offset; nn_y++)
		{
			for (int nn_x = x2-offset; nn_x <= x2+offset; nn_x++)
			{
				if (nn_y>=0 && nn_y<myData->src->GetHeight() &&
					nn_x>=0 && nn_x<myData->src->GetWidth())
				{
					double edgeness = 0.0;
					p2 = myData->src->GetPixelIntensity(nn_x,nn_y);
					edgeness += myData->alpha*PairwiseColorDiff(p1,p2,0.0);
					edgeness += myData->beta*myData->gradient->dx->Get(nn_y+1,nn_x+1);
					edge_cost = (edge_cost<edgeness) ? edgeness : edge_cost;
				}
			}
		}
		cost += 1*edge_cost;
	}
	if ((y1==y2) && (x2<x1) && l1>0)
	{
		intensityType p1 = myData->src->GetPixelIntensity(x1,y1);
		intensityType p2;
		int offset = 2;
		double edge_cost = 0.0;
		for (int nn_y = y1-offset; nn_y <= y1+offset; nn_y++)
		{
			for (int nn_x = x1-offset; nn_x <= x1+offset; nn_x++)
			{
				if (nn_y>=0 && nn_y<myData->src->GetHeight() &&
					nn_x>=0 && nn_x<myData->src->GetWidth())
				{
					double edgeness = 0.0;
					p2 = myData->src->GetPixelIntensity(nn_x,nn_y);
					edgeness += myData->alpha*PairwiseColorDiff(p1,p2,0.0);
					edgeness += myData->beta*myData->gradient->dx->Get(nn_y+1,nn_x+1);
					edge_cost = (edge_cost<edgeness) ? edgeness : edge_cost;
				}
			}
		}
		cost += 1*edge_cost;
	}
	*/

	// consider edge
	/*
	if ((y1==y2) && (x1<x2) && l2>0)
	{
		intensityType p1 = myData->src->GetPixelIntensity(x2,y2);
		gradientType p1_grad;
		p1_grad.dx = myData->gradient->dx->Get(y2+1,x2+1);
		p1_grad.dy = myData->gradient->dy->Get(y2+1,x2+1);
		intensityType p2;
		gradientType p2_grad;
		int nn_num = 0;
		double edge_cost = 0.0;
		if (x2>0)
		{	
			p2 = myData->src->GetPixelIntensity(x2-1,y2);
			p2_grad.dx = myData->gradient->dx->Get(y2+1,x2);
			p2_grad.dy = myData->gradient->dy->Get(y2+1,x2);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		if (x2<myData->src->GetWidth()-1)
		{
			p2 = myData->src->GetPixelIntensity(x2+1,y2);
			p2_grad.dx = myData->gradient->dx->Get(y2+1,x2+2);
			p2_grad.dy = myData->gradient->dy->Get(y2+1,x2+2);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		if (y2>0)
		{
			p2 = myData->src->GetPixelIntensity(x2,y2-1);
			p2_grad.dx = myData->gradient->dx->Get(y2,x2+1);
			p2_grad.dy = myData->gradient->dy->Get(y2,x2+1);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		if (y2<myData->src->GetHeight()-1)
		{
			p2 = myData->src->GetPixelIntensity(x2,y2+1);
			p2_grad.dx = myData->gradient->dx->Get(y2+2,x2+1);
			p2_grad.dy = myData->gradient->dy->Get(y2+2,x2+1);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		if (x2>0 && y2>0)
		{	
			p2 = myData->src->GetPixelIntensity(x2-1,y2-1);
			p2_grad.dx = myData->gradient->dx->Get(y2,x2);
			p2_grad.dy = myData->gradient->dy->Get(y2,x2);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		if (x2<myData->src->GetWidth()-1 && y2>0)
		{	
			p2 = myData->src->GetPixelIntensity(x2+1,y2-1);
			p2_grad.dx = myData->gradient->dx->Get(y2,x2+2);
			p2_grad.dy = myData->gradient->dy->Get(y2,x2+2);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		if (x2>0 && y2<myData->src->GetHeight()-1)
		{	
			p2 = myData->src->GetPixelIntensity(x2-1,y2+1);
			p2_grad.dx = myData->gradient->dx->Get(y2+2,x2);
			p2_grad.dy = myData->gradient->dy->Get(y2+2,x2);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		if (x2<myData->src->GetWidth()-1 && y2<myData->src->GetHeight()-1)
		{	
			p2 = myData->src->GetPixelIntensity(x2+1,y2+1);
			p2_grad.dx = myData->gradient->dx->Get(y2+2,x2+2);
			p2_grad.dy = myData->gradient->dy->Get(y2+2,x2+2);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		cost += 0.1*edge_cost/nn_num;
	}
	if ((y1==y2) && (x2<x1) && l1>0)
	{
		intensityType p1 = myData->src->GetPixelIntensity(x1,y1);
		gradientType p1_grad;
		p1_grad.dx = myData->gradient->dx->Get(y1+1,x1+1);
		p1_grad.dy = myData->gradient->dy->Get(y1+1,x1+1);
		intensityType p2;
		gradientType p2_grad;
		int nn_num = 0;
		double edge_cost = 0.0;
		if (x1>0)
		{	
			p2 = myData->src->GetPixelIntensity(x1-1,y1);
			p2_grad.dx = myData->gradient->dx->Get(y1+1,x1);
			p2_grad.dy = myData->gradient->dy->Get(y1+1,x1);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		if (x1<myData->src->GetWidth()-1)
		{
			p2 = myData->src->GetPixelIntensity(x1+1,y1);
			p2_grad.dx = myData->gradient->dx->Get(y1+1,x1+2);
			p2_grad.dy = myData->gradient->dy->Get(y1+1,x1+2);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		if (y1>0)
		{
			p2 = myData->src->GetPixelIntensity(x1,y1-1);
			p2_grad.dx = myData->gradient->dx->Get(y1,x1+1);
			p2_grad.dy = myData->gradient->dy->Get(y1,x1+1);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		if (y1<myData->src->GetHeight()-1)
		{
			p2 = myData->src->GetPixelIntensity(x1,y1+1);
			p2_grad.dx = myData->gradient->dx->Get(y1+2,x1+1);
			p2_grad.dy = myData->gradient->dy->Get(y1+2,x1+1);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		if (x1>0 && y1>0)
		{	
			p2 = myData->src->GetPixelIntensity(x1-1,y1-1);
			p2_grad.dx = myData->gradient->dx->Get(y1,x1);
			p2_grad.dy = myData->gradient->dy->Get(y1,x1);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		if (x1<myData->src->GetWidth()-1 && y1>0)
		{	
			p2 = myData->src->GetPixelIntensity(x1+1,y1-1);
			p2_grad.dx = myData->gradient->dx->Get(y1,x1+2);
			p2_grad.dy = myData->gradient->dy->Get(y1,x1+2);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		if (x1>0 && y1<myData->src->GetHeight()-1)
		{	
			p2 = myData->src->GetPixelIntensity(x1-1,y1+1);
			p2_grad.dx = myData->gradient->dx->Get(y1+2,x1);
			p2_grad.dy = myData->gradient->dy->Get(y1+2,x1);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		if (x1<myData->src->GetWidth()-1 && y1<myData->src->GetHeight()-1)
		{	
			p2 = myData->src->GetPixelIntensity(x1+1,y1+1);
			p2_grad.dx = myData->gradient->dx->Get(y1+2,x1+2);
			p2_grad.dy = myData->gradient->dy->Get(y1+2,x1+2);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		cost += 0.1*edge_cost/nn_num;
	}
	*/

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
		l!=myData->src->GetWidth()-myData->target_size.width)
		return 100000*MAX_COST_VALUE;

	int refine_offset = 3;
	if (x+myData->subband_lbound<myData->subband[myData->subband_idx+y]-refine_offset && l>0)
		return 100000*MAX_COST_VALUE;
	if (x+myData->subband_lbound>myData->subband[myData->subband_idx+y]+refine_offset && l==0)
		return 100000*MAX_COST_VALUE;

	// Assign motion guided data energy 
	// preserve temporal consistency
	int cur_match = x+l;
	int pre_match = x+myData->pre_labels[p];

	if (cur_match==pre_match)
		return 0.0;

	double min_diff;
	if (cur_match>=0 && cur_match<myData->src->GetWidth())
	{
		intensityType p1 = myData->src->GetPixelIntensity(cur_match,y);
		intensityType p2 = myData->src->GetPixelIntensity(pre_match,y);
		min_diff = PairwiseColorDiff(myData->src->GetPixelIntensity(cur_match,y),
									 myData->src->GetPixelIntensity(pre_match,y),0.0);
		/*
		min_diff += pow(myData->gradient->dx->Get(y+1,cur_match+1)-
						myData->gradient->dx->Get(y+1,pre_match+1),2.0);
		min_diff += pow(myData->gradient->dy->Get(y+1,cur_match+1)-
						myData->gradient->dy->Get(y+1,pre_match+1),2.0);
		*/
	}
	else
		min_diff = 100000*MAX_COST_VALUE;

	cost += 0.1*min_diff;
	//cost += 1000*myData->bias->Get(y+1,x+l+1);
	
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
		double color_diff;
		//printf("(%d,%d) color components: %d,%d,%d\n",src->GetPixel(y2+l2,x2).r,src->GetPixel(y2+l2,x2).g,src->GetPixel(y2+l2,x2).b);
		if (direction==0)
		{
			//weight = bias->Get(y2+1,x2+l2+1)*bias->Get(y1+y_offset+1,x1+l1+x_offset+1);
			color_diff = 0.0;
			color_diff += pow((double)(src->GetPixel(x2+l2,y2).r-src->GetPixel(x1+l1+x_offset,y1+y_offset).r),2.0);
			color_diff += pow((double)(src->GetPixel(x2+l2,y2).g-src->GetPixel(x1+l1+x_offset,y1+y_offset).g),2.0);
			color_diff += pow((double)(src->GetPixel(x2+l2,y2).b-src->GetPixel(x1+l1+x_offset,y1+y_offset).b),2.0);
			//diff += 1000*weight;
			diff += (color_diff<threshold) ? 0 : color_diff;
		}
		
		if (direction==1)
		{
			//weight = bias->Get(y2-y_offset+1,x2+l2-x_offset+1)*bias->Get(y1+1,x1+l1+1);
			color_diff = 0.0;
			color_diff += pow((double)(src->GetPixel(x2+l2-x_offset,y2-y_offset).r-src->GetPixel(x1+l1,y1).r),2.0);	
			color_diff += pow((double)(src->GetPixel(x2+l2-x_offset,y2-y_offset).g-src->GetPixel(x1+l1,y1).g),2.0);
			color_diff += pow((double)(src->GetPixel(x2+l2-x_offset,y2-y_offset).b-src->GetPixel(x1+l1,y1).b),2.0);
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

	if (abs(x1-x2)<=1 && abs(y1-y2)<=1)
	{
		double cost1 = 0.0;
		cost1 += myData->alpha*PairImgColorDiff(myData->src, myData->bias, x1, y1, x2, y2, l1, l2, 0, 0.0);
		cost1 += myData->beta*PairImgGradientDiff(myData->gradient, myData->bias, x1+1, y1+1, x2+1, y2+1, l1, l2, 0, 0.0);

		double cost2 = 0.0;
		cost2 += myData->alpha*PairImgColorDiff(myData->src, myData->bias, x1, y1, x2, y2, l1, l2, 1, 0.0);
		cost2 += myData->beta*PairImgGradientDiff(myData->gradient, myData->bias, x1+1, y1+1, x2+1, y2+1, l1, l2, 1, 0.0);

		double weight;
		if (l1>0)
			weight = myData->bias->Get(y1+1,x1+1);
		if (l2>0)
			weight = myData->bias->Get(y2+1,x2+1);
		cost += weight*(cost1+cost2);
		//cost += min(cost1,cost2);
	}
	else 
	{
		cost = 100000*MAX_COST_VALUE;
	}

	/*
	if ((y1==y2) && (x1<x2) && l2>0)
	{
		intensityType p1 = myData->src->GetPixelIntensity(x2,y2);
		intensityType p2;
		int offset = 2;
		double edge_cost = 0.0;
		for (int nn_y = y2-offset; nn_y <= y2+offset; nn_y++)
		{
			for (int nn_x = x2-offset; nn_x <= x2+offset; nn_x++)
			{
				if (nn_y>=0 && nn_y<myData->src->GetHeight() &&
					nn_x>=0 && nn_x<myData->src->GetWidth())
				{
					double edgeness = 0.0;
					p2 = myData->src->GetPixelIntensity(nn_x,nn_y);
					edgeness += myData->alpha*PairwiseColorDiff(p1,p2,0.0);
					edgeness += myData->beta*myData->gradient->dx->Get(nn_y+1,nn_x+1);
					edge_cost = (edge_cost<edgeness) ? edgeness : edge_cost;
				}
			}
		}
		cost += 1*edge_cost;
	}
	if ((y1==y2) && (x2<x1) && l1>0)
	{
		intensityType p1 = myData->src->GetPixelIntensity(x1,y1);
		intensityType p2;
		int offset = 2;
		double edge_cost = 0.0;
		for (int nn_y = y1-offset; nn_y <= y1+offset; nn_y++)
		{
			for (int nn_x = x1-offset; nn_x <= x1+offset; nn_x++)
			{
				if (nn_y>=0 && nn_y<myData->src->GetHeight() &&
					nn_x>=0 && nn_x<myData->src->GetWidth())
				{
					double edgeness = 0.0;
					p2 = myData->src->GetPixelIntensity(nn_x,nn_y);
					edgeness += myData->alpha*PairwiseColorDiff(p1,p2,0.0);
					edgeness += myData->beta*myData->gradient->dx->Get(nn_y+1,nn_x+1);
					edge_cost = (edge_cost<edgeness) ? edgeness : edge_cost;
				}
			}
		}
		cost += 1*edge_cost;
	}
	*/

	// consider edge
	/*
	if ((y1==y2) && (x1<x2) && l2>0)
	{
		intensityType p1 = myData->src->GetPixelIntensity(x2,y2);
		intensityType p2;
		int nn_num = 0;
		double edge_cost = 0.0;
		if (x2>0)
		{	
			p2 = myData->src->GetPixelIntensity(x2-1,y2);
			edge_cost += PairwiseDiff(p1, p2, 0.0);
			nn_num++;
		}
		if (x2<myData->src->GetWidth()-1)
		{
			p2 = myData->src->GetPixelIntensity(x2+1,y2);
			edge_cost += PairwiseDiff(p1, p2, 0.0);
			nn_num++;
		}
		if (y2>0)
		{
			p2 = myData->src->GetPixelIntensity(x2,y2-1);
			edge_cost += PairwiseDiff(p1, p2, 0.0);
			nn_num++;
		}
		if (y2<myData->src->GetHeight()-1)
		{
			p2 = myData->src->GetPixelIntensity(x2,y2+1);
			edge_cost += PairwiseDiff(p1, p2, 0.0);
			nn_num++;
		}
		cost += 100*edge_cost/nn_num;
	}
	if ((y1==y2) && (x2<x1) && l1>0)
	{
		intensityType p1 = myData->src->GetPixelIntensity(x1,y1);
		intensityType p2;
		int nn_num = 0;
		double edge_cost = 0.0;
		if (x1>0)
		{	
			p2 = myData->src->GetPixelIntensity(x1-1,y1);
			edge_cost += PairwiseDiff(p1, p2, 0.0);
			nn_num++;
		}
		if (x1<myData->src->GetWidth()-1)
		{
			p2 = myData->src->GetPixelIntensity(x1+1,y1);
			edge_cost += PairwiseDiff(p1, p2, 0.0);
			nn_num++;
		}
		if (y1>0)
		{
			p2 = myData->src->GetPixelIntensity(x1,y1-1);
			edge_cost += PairwiseDiff(p1, p2, 0.0);
			nn_num++;
		}
		if (y1<myData->src->GetHeight()-1)
		{
			p2 = myData->src->GetPixelIntensity(x1,y1+1);
			edge_cost += PairwiseDiff(p1, p2, 0.0);
			nn_num++;
		}
		cost += 100*edge_cost/nn_num;
	}
	*/

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
	if (x==myData->target_size.width-1 && 
		l!=myData->src->GetPicture(0)->GetWidth()-myData->target_size.width)
		return 100000*MAX_COST_VALUE;
	
	return cost;
}

/*
 * Compute color difference between two mapped points
 */
double VideoColorDiff(PictureList *src, int x1, int y1, int t1, 
					  int x2, int y2, int t2, int l1, int l2, int direction)
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
		if (direction == 0)
		{
			diff += pow((double)(src->GetPicture(t2)->GetPixel(x2+l2,y2).r-
						src->GetPicture(t2)->GetPixel(x1+l1+x_offset,y1+y_offset).r),2.0);
			diff += pow((double)(src->GetPicture(t2)->GetPixel(x2+l2,y2).g-
						src->GetPicture(t2)->GetPixel(x1+l1+x_offset,y1+y_offset).g),2.0);
			diff += pow((double)(src->GetPicture(t2)->GetPixel(x2+l2,y2).b-
						src->GetPicture(t2)->GetPixel(x1+l1+x_offset,y1+y_offset).b),2.0);
		}

		if (direction == 1)
		{
			diff += pow((double)(src->GetPicture(t1)->GetPixel(x2+l2-x_offset,y2-y_offset).r-
						src->GetPicture(t1)->GetPixel(x1+l1,y1).r),2.0);
			diff += pow((double)(src->GetPicture(t1)->GetPixel(x2+l2-x_offset,y2-y_offset).g-
						src->GetPicture(t1)->GetPixel(x1+l1,y1).g),2.0);
			diff += pow((double)(src->GetPicture(t1)->GetPixel(x2+l2-x_offset,y2-y_offset).b-
						src->GetPicture(t1)->GetPixel(x1+l1,y1).b),2.0);
		}

	} else
	{
		diff += 100000*MAX_COST_VALUE;
	}	
	//printf("ColorDiff: finished\n");

	//printf("ColorDiff: cost between (%d,%d) and (%d,%d) with labels %d and %d is %d\n",x1,y1,x2,y2,l1,l2,diff);
	if (x1==x2 && y1==y2)
		return 0.1*diff;
	else
		return diff;
}

/*
 * Compute gradient difference between two mapped points
 */
double VideoGradientDiff(gradient2D *gradient, int x1, int y1, int t1, 
						 int x2, int y2, int t2, int l1, int l2, int direction)
{
	double diff = 0.0;
	int width = gradient[0].dx->NumOfCols();
	int height = gradient[0].dx->NumOfRows();
	int x_offset = x2-x1;
	int y_offset = y2-y1;
	int t_offset = t2-t1;

	if (x2+l2>0 && x2+l2<=width && x1+l1+x_offset>0 && x1+l1+x_offset<=width &&
		x2+l2-x_offset>0 && x2+l2-x_offset<=width)
	{
		if (direction==0)
		{
			diff += pow((double)(gradient[t2].dx->Get(y2,x2+l2)-gradient[t2].dx->Get(y1+y_offset,x1+l1+x_offset)),2.0);
			diff += pow((double)(gradient[t2].dy->Get(y2,x2+l2)-gradient[t2].dy->Get(y1+y_offset,x1+l1+x_offset)),2.0);
		}

		if (direction==1)
		{
			diff += pow((double)(gradient[t1].dx->Get(y2-y_offset,x2+l2-x_offset)-gradient[t1].dx->Get(y1,x1+l1)),2.0);
			diff += pow((double)(gradient[t1].dy->Get(y2-y_offset,x2+l2-x_offset)-gradient[t1].dy->Get(y1,x1+l1)),2.0);
		}
	} else
	{	
		diff += 100000*MAX_COST_VALUE;
	}

	if (x1==x2 && y1==y2)
		return 0.1*diff;
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

	if (abs(x1-x2)<=1 && abs(y1-y2)<=1 && abs(t1-t2)<=1)
	{		
		double cost1 = 0.0;
		cost1 += myData->alpha*VideoColorDiff(myData->src, x1, y1, t1, x2, y2, t2, l1, l2, 0);
		cost1 += myData->beta*VideoGradientDiff(myData->gradient, x1+1, y1+1, t1, x2+1, y2+1, t2, l1, l2, 0);

		double cost2 = 0.0;
		cost2 += myData->alpha*VideoColorDiff(myData->src, x1, y1, t1, x2, y2, t2, l1, l2, 1);
		cost2 += myData->beta*VideoGradientDiff(myData->gradient, x1+1, y1+1, t1, x2+1, y2+1, t2, l1, l2, 1);

		double weight;
		if (l1>0)
			weight = myData->bias[t1].Get(y1+1,x1+1);
		if (l2>0)
			weight = myData->bias[t2].Get(y2+1,x2+1);
		cost += weight*(cost1+cost2);
		//cost += min(cost1,cost2);
	}
	else 
	{
		cost = 100000*MAX_COST_VALUE;
	}

	/*
	if ((t1==t2) && (y1==y2) && (x1<x2) && l2>0)
	{
		intensityType p1 = myData->src->GetPicture(t2)->GetPixelIntensity(x2,y2);
		intensityType p2;
		int offset = 2;
		double edge_cost = 0.0;
		for (int nn_y = y2-offset; nn_y <= y2+offset; nn_y++)
		{
			for (int nn_x = x2-offset; nn_x <= x2+offset; nn_x++)
			{
				if (nn_y>=0 && nn_y<myData->src->GetPicture(t2)->GetHeight() &&
					nn_x>=0 && nn_x<myData->src->GetPicture(t2)->GetWidth())
				{
					double edgeness = 0.0;
					p2 = myData->src->GetPicture(t2)->GetPixelIntensity(nn_x,nn_y);
					edgeness += myData->alpha*PairwiseColorDiff(p1,p2,0.0);
					edge_cost = (edge_cost<edgeness) ? edgeness : edge_cost;
				}
			}
		}
		cost += 1000*edge_cost;
	}
	if ((t1==t2) && (y1==y2) && (x2<x1) && l1>0)
	{
		intensityType p1 = myData->src->GetPicture(t1)->GetPixelIntensity(x1,y1);
		intensityType p2;
		int offset = 2;
		double edge_cost = 0.0;
		for (int nn_y = y1-offset; nn_y <= y1+offset; nn_y++)
		{
			for (int nn_x = x1-offset; nn_x <= x1+offset; nn_x++)
			{
				if (nn_y>=0 && nn_y<myData->src->GetPicture(t1)->GetHeight() &&
					nn_x>=0 && nn_x<myData->src->GetPicture(t1)->GetWidth())
				{
					double edgeness = 0.0;
					p2 = myData->src->GetPicture(t1)->GetPixelIntensity(nn_x,nn_y);
					edgeness += myData->alpha*PairwiseColorDiff(p1,p2,0.0);
					edge_cost = (edge_cost<edgeness) ? edgeness : edge_cost;
				}
			}
		}
		cost += 1000*edge_cost;
	}
	*/

	// consider edge
	/*
	if ((y1==y2) && (t1==t2) && (x1<x2) && l2>0)
	{
		intensityType p1 = myData->src->GetPicture(t2)->GetPixelIntensity(x2,y2);
		gradientType p1_grad;
		p1_grad.dx = myData->gradient[t2].dx->Get(y2+1,x2+1);
		p1_grad.dy = myData->gradient[t2].dy->Get(y2+1,x2+1);
		intensityType p2;
		gradientType p2_grad;
		int nn_num = 0;
		double edge_cost = 0.0;
		if (x2>0)
		{	
			p2 = myData->src->GetPicture(t2)->GetPixelIntensity(x2-1,y2);
			p2_grad.dx = myData->gradient[t2].dx->Get(y2+1,x2);
			p2_grad.dy = myData->gradient[t2].dy->Get(y2+1,x2);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);			
			nn_num++;
		}
		if (x2<myData->src->GetPicture(t2)->GetWidth()-1)
		{
			p2 = myData->src->GetPicture(t2)->GetPixelIntensity(x2+1,y2);
			p2_grad.dx = myData->gradient[t2].dx->Get(y2+1,x2+2);
			p2_grad.dy = myData->gradient[t2].dy->Get(y2+1,x2+2);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		if (y2>0)
		{
			p2 = myData->src->GetPicture(t2)->GetPixelIntensity(x2,y2-1);
			p2_grad.dx = myData->gradient[t2].dx->Get(y2,x2+1);
			p2_grad.dy = myData->gradient[t2].dy->Get(y2,x2+1);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		if (y2<myData->src->GetPicture(t2)->GetHeight()-1)
		{
			p2 = myData->src->GetPicture(t2)->GetPixelIntensity(x2,y2+1);
			p2_grad.dx = myData->gradient[t2].dx->Get(y2+2,x2+1);
			p2_grad.dy = myData->gradient[t2].dy->Get(y2+2,x2+1);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		cost += 10*edge_cost/nn_num;
	}
	if ((y1==y2) && (t1==t2) && (x2<x1) && l1>0)
	{
		intensityType p1 = myData->src->GetPicture(t1)->GetPixelIntensity(x1,y1);
		gradientType p1_grad;
		p1_grad.dx = myData->gradient[t1].dx->Get(y1+1,x1+1);
		p1_grad.dy = myData->gradient[t1].dy->Get(y1+1,x1+1);
		intensityType p2;
		gradientType p2_grad;
		int nn_num = 0;
		double edge_cost = 0.0;
		if (x1>0)
		{	
			p2 = myData->src->GetPicture(t1)->GetPixelIntensity(x1-1,y1);
			p2_grad.dx = myData->gradient[t1].dx->Get(y1+1,x1);
			p2_grad.dy = myData->gradient[t1].dy->Get(y1+1,x1);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		if (x1<myData->src->GetPicture(t1)->GetWidth()-1)
		{
			p2 = myData->src->GetPicture(t1)->GetPixelIntensity(x1+1,y1);
			p2_grad.dx = myData->gradient[t1].dx->Get(y1+1,x1+2);
			p2_grad.dy = myData->gradient[t1].dy->Get(y1+1,x1+2);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		if (y1>0)
		{
			p2 = myData->src->GetPicture(t1)->GetPixelIntensity(x1,y1-1);
			p2_grad.dx = myData->gradient[t1].dx->Get(y1,x1+1);
			p2_grad.dy = myData->gradient[t1].dy->Get(y1,x1+1);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		if (y1<myData->src->GetPicture(t1)->GetHeight()-1)
		{
			p2 = myData->src->GetPicture(t1)->GetPixelIntensity(x1,y1+1);
			p2_grad.dx = myData->gradient[t1].dx->Get(y1+2,x1+1);
			p2_grad.dy = myData->gradient[t1].dy->Get(y1+2,x1+1);
			edge_cost += myData->alpha*PairwiseColorDiff(p1, p2, 0.0);
			edge_cost += myData->beta*PairwiseGradientDiff(p1_grad, p2_grad, 0.0);
			nn_num++;
		}
		cost += 10*edge_cost/nn_num;
	}
	*/

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
				if (result[idx]>0)
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

void SaveSeamImage(int *labels, Picture *ref, char *name)
{
	Picture *seam_img = new Picture(ref->GetWidth()+1,
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
	int cur_idx = 0;
	PictureList *result = new PictureList(width,height,src->GetLength());
	for (int t = 0; t < src->GetLength(); t++)
	{
		Picture *frame = new Picture(width,height);
		char target_name[512] = {'\0'};
		strcat(target_name,name);
		strcat(target_name,src->GetPicture(t)->GetName());
		frame->SetName(src->GetPicture(t)->GetName());

		Picture *map = new Picture(width,height);
		char map_name[512] = {'\0'};
		strcat(map_name,name);
		strcat(map_name,"shift_");
		strcat(map_name,src->GetPicture(t)->GetName());
		map->SetName(src->GetPicture(t)->GetName());
	
		int x, y;
		pixelType pixel;
		intensityType map_pix;
		int *frame_labels = new int[width*height];
		int frame_idx = 0;
		for ( int  i = 0; i < width*height; i++ )
		{
			x = i % width;
			y = i / width;
			//printf("SaveRetargetPicture: GetPixel(%d,%d)\n",x+gc->whatLabel(i),y);
			pixel.r = src->GetPicture(t)->GetPixel(x+labels[cur_idx],y).r;
			pixel.g = src->GetPicture(t)->GetPixel(x+labels[cur_idx],y).g;
			pixel.b = src->GetPicture(t)->GetPixel(x+labels[cur_idx],y).b;
			frame->SetPixel(x,y,pixel);

			map_pix.r = labels[cur_idx]*255;
			map_pix.g = labels[cur_idx]*255;
			map_pix.b = labels[cur_idx]*255;
			map->SetPixelIntensity(x,y,map_pix);

			frame_labels[frame_idx] = labels[cur_idx];
			frame_idx++;
			cur_idx++;
		}

		result->SetPicture(t,frame);
		if (save)
		{
			frame->Save(target_name);
			map->Save(map_name);
		}

		char seam_name[512] = {'\0'};
		strcat(seam_name,name);
		strcat(seam_name,"seam_");
		strcat(seam_name,src->GetPicture(t)->GetName());
		SaveSeamImage(frame_labels,frame,seam_name);

		delete frame;
		delete map;
		delete [] frame_labels;
	}

	return result;
}


/*
 * Function to find the optimal shift-map
 */
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

/*
 * Function to find the optimal single seam
 */
PictureList *GridGraph_SeamCut(PictureList *src, double ratio, videoSize &target_size, videoSize &up_size,
							   int num_labels, float alpha, float beta, char *target_path, int *&band,
							   int *&lr_band, Matrix *bias)
{
	// set up the needed data to pass to function for the data costs
	gradient2D *gradient = new gradient2D[src->GetLength()];
	for (int t = 0; t < src->GetLength(); t++)
	{
		gradient2D* grad = Gradient(src->GetPicture(t));
		gradient[t].dx = grad->dx;
		gradient[t].dy = grad->dy;
		gradient[t].total_dx = grad->total_dx;
		gradient[t].total_dy = grad->total_dy;
		delete grad;
	}


	int num_pixels = target_size.width*target_size.height*target_size.time;
	int *labels = new int[num_pixels];   // stores result of optimization
	PictureList *result;

	try{
		GCoptimization3DGridGraph *gc = new GCoptimization3DGridGraph(target_size.width,
																	  target_size.height,
																	  target_size.time,
																	  num_labels);


		// TODO: replace the hardcode of gradient threoshold
		// for shot boundary
		// data terms comes from function pointer
		ForVideoDataFn toDataFn;
		toDataFn.src = src;
		toDataFn.gradient = gradient;
		toDataFn.bias = bias;
		toDataFn.target_size = target_size;
		gc->setDataCost(&VideoDataFn,&toDataFn);

		// smoothness comes from function pointer
		ForVideoSmoothFn toSmoothFn;
		toSmoothFn.src = src;		
		toSmoothFn.gradient = gradient;
		toSmoothFn.bias = bias;
		toSmoothFn.target_size = target_size;
		toSmoothFn.alpha = alpha;
		toSmoothFn.beta = beta;
		gc->setSmoothCost(&VideoSmoothFn, &toSmoothFn);

		//printf("Before optimization energy is %f\n",gc->compute_energy());
		gc->expansion();// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		//printf("After optimization energy is %f\n",gc->compute_energy());

		// obtain the labels
		for ( int  i = 0; i < num_pixels; i++ )
		{
			//printf("The optimal label for pixel %d is %d\n",i,gc->whatLabel(i));
			labels[i] = gc->whatLabel(i);
		}		

		// upsampling shift-map from low-resolution to high-resolution
		lr_band = UpsamplingShiftMap(labels,target_size,1.0,target_size);											 
		band = UpsamplingShiftMap(labels,target_size,ratio,up_size);											 
		//delete [] result;		

		// update_
		result = GenerateRetargetResult(labels, src, 
										target_size.width, 
										target_size.height,target_path,true);
		
		
		delete labels;
		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	for (int t = 0; t < src->GetLength(); t++)
	{
		delete gradient[t].dx;
		delete gradient[t].dy;
	}
	delete [] gradient;
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
			int lval = max(subband[idx+y]-0*ratio,0);
			int uval = min(subband[idx+y]+1*ratio-1,ref->GetWidth()-1);
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
			int idx = y*ref->GetWidth()+x;
			pixel = ref->GetPixel(x,y);
			result->SetPixel(x-lbound,y,pixel);
			subband_labels[y*(ubound-lbound+1)+x-lbound] = labels[idx];
		}
	}

	delete [] labels;
	labels = subband_labels;
	result->Save("subband_img.ppm");
	return result;
}

/*
 * Function to generate and save retargeted image
 * using shift-map labels
 */
Picture *GenerateRetargetPicture(int *labels, Picture *src,int width, int height)
{
	Picture *result = new Picture(width,height);
	
	int x, y;
	pixelType pixel;
	for ( int  i = 0; i < width*height; i++ )
	{
		x = i % width;
		y = i / width;
		//printf("SaveRetargetPicture: GetPixel(%d,%d)\n",x+gc->whatLabel(i),y);
		pixel = src->GetPixel(x+labels[i],y);
		result->SetPixel(x,y,pixel);
	}

	result->Save("0_cut.ppm");
	return result;
}

Picture *Combine_SubbandImgs(Picture *src, Picture *band_img, 
							 double ratio, int lbound, int ubound, int *&labels)
{
	Picture *result = new Picture(src->GetWidth()-1, src->GetHeight());
	result->SetName(src->GetName());
	int *new_labels = new int[result->GetWidth()*result->GetHeight()];

	pixelType pixel;
	for (int y = 0; y < src->GetHeight(); y++)
	{
		int idx;
		for (int x = 0; x < lbound; x++)
		{
			idx = y*result->GetWidth()+x;
			pixel = src->GetPixel(x,y);
			result->SetPixel(x,y,pixel);
			new_labels[idx] = 0;
		}
		for (int x = lbound; x < ubound; x++)
		{
			idx = y*result->GetWidth()+x;
			pixel = band_img->GetPixel(x-lbound,y);
			result->SetPixel(x,y,pixel);
			new_labels[idx] = labels[y*(ubound-lbound)+x-lbound];
		}
		for (int x = ubound; x < result->GetWidth(); x++)
		{
			idx = y*result->GetWidth()+x;
			pixel = src->GetPixel(x+1,y);
			result->SetPixel(x,y,pixel);
			new_labels[idx] = 1;
		}
	}

	delete [] labels;
	labels = new_labels;
	return result;
}

Picture *SingleImg_SeamCut(Picture *img, int *&subband, int idx, int lbound, imageSize &target_size,
						   float alpha, float beta, int *&labels, Matrix *&bias)
{
	int num_pixels = target_size.width*target_size.height;
	Picture *result = NULL;

	try{
		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(target_size.width,
																  target_size.height,
																  2);

		// set up the needed data to pass to function for the data costs
		gradient2D *gradient = Gradient(img);
		//Matrix *bias = CalcNarrownessPrior(gradient,1000.0);

		ForSingleImgDataFn toDataFn;
		toDataFn.src = img;
		toDataFn.gradient = gradient;
		toDataFn.bias = bias;
		toDataFn.subband = subband;
		toDataFn.subband_idx = idx;
		toDataFn.subband_lbound = lbound;
		toDataFn.target_size = target_size;
		gc->setDataCost(&SingleImgDataFn,&toDataFn);

		// smoothness comes from function pointer
		ForSingleImgSmoothFn toSmoothFn;
		toSmoothFn.src = img;		
		toSmoothFn.gradient = gradient;
		toSmoothFn.bias = bias;
		toSmoothFn.target_size = target_size;
		toSmoothFn.alpha = alpha;
		toSmoothFn.beta = beta;
		gc->setSmoothCost(&SingleImgSmoothFn, &toSmoothFn);

		//printf("Before optimization energy is %f\n",gc->compute_energy());
		gc->expansion();// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		//printf("After optimization energy is %f\n",gc->compute_energy());

		// obtain the labels
		for ( int  i = 0; i < num_pixels; i++ )
		{
			//printf("The optimal label for pixel %d is %d\n",i,gc->whatLabel(i));
			labels[i] = gc->whatLabel(i);
		}

		/* update subband */
		int subband_idx = idx;
		for (int y = 0; y < target_size.height; y++)
        {
			for (int x = 0; x < target_size.width; x++)
            {
				int p = y*target_size.width+x;
				if (labels[p]>0)
				{
					subband[subband_idx] = x+lbound;
					subband_idx++;
					break;
				}
			}
		}

		/* generate retargeted subband image */
		result = GenerateRetargetPicture(labels, img, 
										 target_size.width, target_size.height);
		SaveShiftMap(labels,target_size.width,target_size.height,"shift.ppm");

		delete gradient->dx;
		delete gradient->dy;
		delete gradient;
		//delete bias;
		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	return result;
}

Picture *PairImg_SeamCut(Picture *cur_frame, Picture *pre_frame, int *&subband, int idx, int lbound, Matrix *&bias,
						 imageSize &target_size, float alpha, float beta, int *&pre_labels, bool tconsistency)
{
	// set up the needed data to pass to function for the data costs
	gradient2D *gradient = Gradient(cur_frame);
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
			toDataFn.pre = pre_frame;
			toDataFn.pre_labels = pre_labels;
			toDataFn.gradient = gradient;
			toDataFn.bias = bias;
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
			toDataFn.gradient = gradient;
			toDataFn.bias = bias;
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
		toSmoothFn.bias = bias;
		toSmoothFn.target_size = target_size;
		toSmoothFn.alpha = alpha;
		toSmoothFn.beta = beta;
		gc->setSmoothCost(&PairImgSmoothFn, &toSmoothFn);

		//printf("Before optimization energy is %f\n",gc->compute_energy());
		gc->expansion();// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		//printf("After optimization energy is %f\n",gc->compute_energy());

		// obtain the labels
		for ( int  i = 0; i < num_pixels; i++ )
		{
			//printf("The optimal label for pixel %d is %d\n",i,gc->whatLabel(i));
			labels[i] = gc->whatLabel(i);
		}

		/* update subband */
		int subband_idx = idx;
		for (int y = 0; y < target_size.height; y++)
        {
			for (int x = 0; x < target_size.width; x++)
            {
				int p = y*target_size.width+x;
				if (labels[p]>0)
				{
					subband[subband_idx] = x+lbound;
					subband_idx++;
					break;
				}
			}
		}

		/* generate retargeted subband image */
		result = GenerateRetargetPicture(labels, cur_frame, 
										 target_size.width, 
										 target_size.height);
		SaveShiftMap(labels,target_size.width,target_size.height,"shift.ppm");


		delete gradient->dx;
		delete gradient->dy;
		delete gradient;
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
PictureList *Refine_Seam(int *&band, PictureList *src, double ratio, float alpha, float beta, 
						 char *name, Matrix *&bias, bool tconsistency, bool banded)
{
	imageSize target_size;
	bool leftmost_preserve, rightmost_preserve;
	PictureList *result = new PictureList(src->GetMaxWidth()-1,
										  src->GetMaxHeight(),
										  src->GetLength());
	/* process first frame */
	int lbound = -1;
	int ubound = -1;
	int *empty_labels = new int[src->GetMaxHeight()*src->GetMaxWidth()];
	Picture *band_img;
	if (!banded)
		band_img = src->GetPicture(0);
	else
		band_img = Subband_Picture(src->GetPicture(0),band,0,ratio,
								   lbound,ubound,empty_labels,false);
	delete [] empty_labels;
	target_size.width = band_img->GetWidth()-1;
	target_size.height = band_img->GetHeight();
	int *labels = new int[target_size.width*target_size.height];
	
	Matrix *band_bias = CopySubband_Bias(bias,lbound,ubound,0);
	Picture *retargeted_band_img = SingleImg_SeamCut(band_img,band,0,lbound,target_size,
													 alpha,beta,labels,band_bias);
	delete band_bias;
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
		combined_img = Combine_SubbandImgs(src->GetPicture(0),retargeted_band_img,ratio,lbound,ubound,labels);
		result->SetPicture(0,combined_img);
		delete band_img;
		delete retargeted_band_img;
		delete combined_img;
	}

	char target_name[512] = {'\0'};
	strcat(target_name,name);
	strcat(target_name,result->GetPicture(0)->GetName());
	result->GetPicture(0)->Save(target_name);

	char map_name[512] = {'\0'};
	strcat(map_name,name);
	strcat(map_name,"shift_");
	strcat(map_name,result->GetPicture(0)->GetName());
	SaveShiftMap(labels,result->GetPicture(0)->GetWidth(),
				 result->GetPicture(0)->GetHeight(),map_name);

	char seam_name[512] = {'\0'};
	strcat(seam_name,name);
	strcat(seam_name,"seam_");
	strcat(seam_name,result->GetPicture(0)->GetName());
	SaveSeamImage(labels,result->GetPicture(0),seam_name);

	/* process 2nd frame to t-1 frame */
	Picture *pre_band_img;
	for (int t = 1; t < src->GetLength(); t++)
	{
		//lbound = -1;
		//ubound = -1;
		empty_labels = new int[src->GetMaxHeight()*src->GetMaxWidth()];
		if (!banded)
			band_img = src->GetPicture(t);
		else
			band_img = Subband_Picture(src->GetPicture(t),band,t*src->GetMaxHeight(),ratio,
									   lbound,ubound,empty_labels,false);
		delete [] empty_labels; 
		ubound--;
		if (!banded)
			pre_band_img = result->GetPicture(t-1);
		else
			pre_band_img = Subband_Picture(result->GetPicture(t-1),band,t*src->GetMaxHeight(),ratio,
										   lbound,ubound,labels,true);
		target_size.width = band_img->GetWidth()-1;
		target_size.height = band_img->GetHeight();

		band_bias = CopySubband_Bias(bias,lbound,ubound,t);	
		//retargeted_band_img = SingleImg_SeamCut(band_img,band,t*src->GetMaxHeight(),lbound,target_size,
		//										  alpha,beta,labels,band_bias);
		retargeted_band_img = PairImg_SeamCut(band_img,pre_band_img,band,t*src->GetMaxHeight(),
											  lbound,bias,target_size,alpha,beta,labels,tconsistency);
		delete band_bias;

		retargeted_band_img->SetName(src->GetPicture(t)->GetName());
		if (!banded)
		{
			combined_img = retargeted_band_img;
			result->SetPicture(t,combined_img);
			delete retargeted_band_img;
		}
		else
		{
			combined_img = Combine_SubbandImgs(src->GetPicture(t),retargeted_band_img,ratio,lbound,ubound+1,labels);
			result->SetPicture(t,combined_img);
			delete band_img;
			delete pre_band_img;
			delete retargeted_band_img;
			delete combined_img;
		}
		
		char frame_name[512] = {'\0'};
		strcat(frame_name,name);
		strcat(frame_name,result->GetPicture(t)->GetName());
		result->GetPicture(t)->Save(frame_name);

		char map_name2[512] = {'\0'};
		strcat(map_name2,name);
		strcat(map_name2,"shift_");
		strcat(map_name2,result->GetPicture(t)->GetName());
		SaveShiftMap(labels,result->GetPicture(t)->GetWidth(),
					 result->GetPicture(t)->GetHeight(),map_name2);

		char seam_name2[512] = {'\0'};
		strcat(seam_name2,name);
		strcat(seam_name2,"seam_");
		strcat(seam_name2,result->GetPicture(t)->GetName());
		SaveSeamImage(labels,result->GetPicture(t),seam_name2);
	}

	delete [] labels;
	return result;
}

/*
 *
 */
void ResizeVideo_3D(char *input_path, double alpha, double beta,
					int removed_seam, char *output_path, int level)
{
	listPyramidType *spyramid = NULL;
	int num_labels;
	int *band = NULL;	// store shift labels of every pixel
	int *lr_band = NULL;
	Matrix *bias = NULL;
	videoSize target_size, origin_size;

	PictureList *shot = new PictureList(input_path);

	PictureList *source = NULL;
	PictureList *target = shot;
	while (removed_seam>0)
	{	
		if (level>0)
		{
			spyramid = ListPyramid(target,level+1);
			source = &(spyramid->Lists[level]);	
			target = &(spyramid->Lists[0]);
		}
		else
			source = target;
		
		num_labels = 2;
		target_size.width = source->GetMaxWidth()-1;
		target_size.height = source->GetMaxHeight();
		target_size.time = source->GetLength();
		origin_size.width = target->GetMaxWidth()-1;
		origin_size.height = target->GetMaxHeight();
		origin_size.time = target->GetLength();

		if (bias==NULL)
		{
			bias = new Matrix[target_size.time];
			for (int t = 0; t < target_size.time; t++)
			{
				Matrix *init = new Matrix(target_size.height,
										  target_size.width,1.0);
				bias[t] = *(init);
				delete init;
			}
		}

		/* graph cut in low resolution */
		target = GridGraph_SeamCut(source,pow(2.0,level),target_size,origin_size,
								   num_labels,alpha,beta,output_path,band,lr_band,bias);

		// generate output result;
		/*
		target = GenerateRetargetResult(shift_map, target,
										origin_size.width,
										origin_size.height,"..\\..\\..\\results\\video5\\1\\",true);
		*/

		removed_seam--;
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
		delete shot;
		delete source;
	}
	delete target;
}

/*
 *
 */
void ResizeVideo_2D_Incremental(char *input_path, double alpha, double beta,
								int removed_seam, char *output_path, int level)
{
	PictureList *shot = NULL;
	listPyramidType *spyramid = NULL;
	int num_labels;
	int *band = NULL;	// store shift labels of every pixel
	Matrix *bias = NULL;
	videoSize target_size, origin_size;

	shot = new PictureList(input_path);	
	//spyramid = ListPyramid(shot,level+1);

	PictureList *source;
	PictureList *target = shot;
	while (removed_seam>0)
	{	
		if (level>0)
		{
			spyramid = ListPyramid(target,level+1);
			source = &(spyramid->Lists[level]);	
			target = &(spyramid->Lists[0]);
		}
		else
			source = target;

		num_labels = 2;
		target_size.width = source->GetMaxWidth()-1;
		target_size.height = source->GetMaxHeight();
		target_size.time = source->GetLength();
		origin_size.width = target->GetMaxWidth()-1;
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
				Matrix *init = new Matrix(origin_size.height,
										  origin_size.width,1.0);
				bias[t] = *(init);
				delete init;
			}
		}

		/* refine seam in higher resolution */
		target = Refine_Seam(band,target,pow(2.0,level),
							 alpha,beta,output_path,bias,true,false);

		removed_seam--;
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
		delete shot;
		delete source;
	}
	delete target;
}

/*
 *
 */
void ResizeImages_2D_Incremental(char *input_path, double alpha, double beta,
								 int removed_seam, char *output_path, int level)
{
	PictureList *shot = NULL;
	listPyramidType *spyramid = NULL;
	int num_labels;
	int *band = NULL;	// store shift labels of every pixel
	Matrix *bias = NULL;
	videoSize target_size, origin_size;

	shot = new PictureList(input_path);	
	//spyramid = ListPyramid(shot,level+1);

	PictureList *source;
	PictureList *target = shot;
	while (removed_seam>0)
	{	
		if (level>0)
		{
			spyramid = ListPyramid(target,level+1);
			source = &(spyramid->Lists[level]);	
			target = &(spyramid->Lists[0]);
		}
		else
			source = target;

		num_labels = 2;
		target_size.width = source->GetMaxWidth()-1;
		target_size.height = source->GetMaxHeight();
		target_size.time = source->GetLength();
		origin_size.width = target->GetMaxWidth()-1;
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
				Matrix *init = new Matrix(origin_size.height,
										  origin_size.width,1.0);
				bias[t] = *(init);
				delete init;
			}
		}

		/* refine seam in higher resolution */
		target = Refine_Seam(band,target,pow(2.0,level),
							 alpha,beta,output_path,bias,false,false);

		removed_seam--;
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
		delete shot;
		delete source;
	}
	delete target;
}

/*
 *
 */
void ResizeVideo_Hybrid(char *input_path, double alpha, double beta,
						int removed_seam, char *output_path, int level)
{
	PictureList *shot = NULL;
	listPyramidType *spyramid = NULL;
	int num_labels;
	int *band = NULL;	// store shift labels of every pixel
	int *lr_band = NULL;
	Matrix *lr_bias = NULL;
	Matrix *bias = NULL;
	videoSize target_size, origin_size;

	shot = new PictureList(input_path);	
	//spyramid = ListPyramid(shot,level+1);

	PictureList *source;
	PictureList *target = shot;
	while (removed_seam>0)
	{	
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
		target_size.width = source->GetMaxWidth()-1;
		target_size.height = source->GetMaxHeight();
		target_size.time = source->GetLength();
		origin_size.width = target->GetMaxWidth()-1;
		origin_size.height = target->GetMaxHeight();
		origin_size.time = target->GetLength();

		if (lr_bias!=NULL)
			delete [] lr_bias;
		//if (lr_bias==NULL)
		//{
			lr_bias = new Matrix[target_size.time];
			for (int t = 0; t < target_size.time; t++)
			{
				Matrix *init = new Matrix(target_size.height,
										  target_size.width,1.0);
				lr_bias[t] = *(init);
				delete init;
			}
		//}

		/* graph cut in low resolution */
		source = GridGraph_SeamCut(source,pow(2.0,level),target_size,
								   origin_size,num_labels,alpha,
								   beta,output_path,band,lr_band,lr_bias);

		// DEBUG: observe interpolated map
		Picture *upsampled_map = NULL;
		for (int t = 0; t < origin_size.time; t++)
		{
			upsampled_map = new Picture(origin_size.width,origin_size.height);
			for (int y = 0; y < origin_size.height; y++)
			{
				for (int x = 0; x < origin_size.width; x++)
				{
					pixelType pix;
					if (x<band[t*origin_size.height+y])
					{
						pix = target->GetPicture(t)->GetPixel(x,y);
					}
					if (x==band[t*origin_size.height+y])
					{
						pix.r = 255;
						pix.g = 255;
						pix.b = 255;
					}
					if (x>band[t*origin_size.height+y])
					{
						pix = target->GetPicture(t)->GetPixel(x+1,y);
					}
					upsampled_map->SetPixel(x,y,pix);
				}
			}
			char map_name[512] = {'\0'};
			strcat(map_name,output_path);
			strcat(map_name,"upsampled_");
			strcat(map_name,target->GetPicture(t)->GetName());
			upsampled_map->Save(map_name);
			delete upsampled_map;
		}

		if (bias!=NULL)
			delete [] bias;
		//if (bias==NULL)
		//{
			bias = new Matrix[origin_size.time];
			for (int t = 0; t < origin_size.time; t++)
			{
				Matrix *init = new Matrix(origin_size.height,
										  origin_size.width,1.0);
				bias[t] = *(init);
				delete init;
			}
		//}
		/* refine seam in higher resolution */
		target = Refine_Seam(band,target,pow(2.0,level),alpha,
							 beta,output_path,bias,true,true);

		/*
		lr_bias = CalcSeamBias(lr_bias, lr_band, target_size.width,
							   target_size.height, target_size.time,
							   target_size.width/32);

		bias = CalcSeamBias(bias, band, origin_size.width, origin_size.height, 
							origin_size.time, origin_size.width/32);
		*/

		removed_seam--;
		delete [] band;
		delete [] lr_band;
		if (level>0)
		{
			delete [] spyramid->Lists;
			delete spyramid;
			spyramid = NULL;
		}
		delete source;
	}

	if (level>0)
	{
		//delete shot;
		//delete source;
	}

	delete [] lr_bias;
	delete [] bias;
	delete target;
}

/*
 * main entry of the grogram
 */ 
int main(int argc, char **argv)
{
	if (argc<7)
	{
		cout << "Usage: mg_shift_map_3d <input_folder> <alpha> <beta> <number of removed pixel> <output_folder> method" << endl;
		return 0;
		//default parameters
	}

	int level = 2; // gpyramid->Levels-1

	switch (atoi(argv[6]))
	{
		case 0:	// 3D temporal consistent shift-map
		{
			ResizeVideo_3D(argv[1],atof(argv[2]),atof(argv[3]),atoi(argv[4]),argv[5],0);
			break;
		}
		case 1:	// 2D incremental temporal consistent shift-map
		{
			ResizeVideo_2D_Incremental(argv[1],atof(argv[2]),atof(argv[3]),atoi(argv[4]),argv[5],0);
			break;
		}
		case 2: // hybrid 3D & 2D temporal consistent shift-map
		{
			ResizeVideo_Hybrid(argv[1],atof(argv[2]),atof(argv[3]),atoi(argv[4]),argv[5],level);
			break;
		}
		case 3: // 2D incremental shift-map without temporal consistency
		{
			ResizeImages_2D_Incremental(argv[1],atof(argv[2]),atof(argv[3]),atoi(argv[4]),argv[5],0);
		}
	}
	
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////