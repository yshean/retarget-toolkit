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
	imageSize target_size;
};

/*
 * structure of smooth term for single image
 */
struct ForSingleImgSmoothFn
{
	Picture *src;
	gradient2D *gradient;
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
	imageSize target_size;	
};

/*
 * structure of smooth term for image pair
 */
struct ForPairImgSmoothFn
{
	Picture *src;
	gradient2D *gradient;
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
	Matrix *gradient;
	videoSize target_size;
};

/*
 * structure of smooth term for video
 */
struct ForVideoSmoothFn
{
	PictureList *src;
	Matrix *gradient;
	videoSize target_size;
	float alpha;
	float beta;
};

double PairwiseDiff(intensityType p1, intensityType p2, double threshold)
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
	
	return cost;
}

double SingleImgColorDiff(Picture *src, int x1, int y1, 
						  int x2, int y2, int l1, int l2)
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
		//printf("(%d,%d) color components: %d,%d,%d\n",src->GetPixel(y2+l2,x2).r,src->GetPixel(y2+l2,x2).g,src->GetPixel(y2+l2,x2).b);
		diff += pow((double)src->GetPixel(x2+l2,y2).r-src->GetPixel(x1+l1+x_offset,y1+y_offset).r,2);
		diff += pow((double)src->GetPixel(x2+l2-x_offset,y2-y_offset).r-src->GetPixel(x1+l1,y1).r,2);
		diff += pow((double)src->GetPixel(x2+l2,y2).g-src->GetPixel(x1+l1+x_offset,y1+y_offset).g,2);
		diff += pow((double)src->GetPixel(x2+l2-x_offset,y2-y_offset).g-src->GetPixel(x1+l1,y1).g,2);
		diff += pow((double)src->GetPixel(x2+l2,y2).b-src->GetPixel(x1+l1+x_offset,y1+y_offset).b,2);
		diff += pow((double)src->GetPixel(x2+l2-x_offset,y2-y_offset).b-src->GetPixel(x1+l1,y1).b,2);
	} else
	{
		diff += 100000*MAX_COST_VALUE;
	}
	//printf("ColorDiff: finished\n");

	//printf("ColorDiff: cost between (%d,%d) and (%d,%d) with labels %d and %d is %d\n",x1,y1,x2,y2,l1,l2,diff);
	return diff;
}

double SingleImgGradientDiff(gradient2D *gradient, int x1, int y1, 
							 int x2, int y2, int l1, int l2)
{
	double diff = 0.0;
	int width = gradient->dx->NumOfCols();
	int height = gradient->dx->NumOfRows();
	int x_offset = x2-x1;
	int y_offset = y2-y1;

	if (x2+l2>0 && x2+l2<=width && x1+l1+x_offset>0 && x1+l1+x_offset<=width &&
		x2+l2-x_offset>0 && x2+l2-x_offset<=width && x1+l1>0 && x1+l1<=width)
	{
		diff += pow((gradient->dx->Get(y2,x2+l2)-gradient->dx->Get(y1+y_offset,x1+l1+x_offset)),2)+
				pow((gradient->dy->Get(y2,x2+l2)-gradient->dy->Get(y1+y_offset,x1+l1+x_offset)),2);
		diff += pow((gradient->dx->Get(y2-y_offset,x2+l2-x_offset)-gradient->dx->Get(y1,x1+l1)),2)+
				pow((gradient->dy->Get(y2-y_offset,x2+l2-x_offset)-gradient->dy->Get(y1,x1+l1)),2);
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
		cost += myData->alpha*SingleImgColorDiff(myData->src, x1, y1, x2, y2, l1, l2);
		cost += myData->beta*SingleImgGradientDiff(myData->gradient, x1+1, y1+1, x2+1, y2+1, l1, l2);
	}
	else 
	{
		cost = 100000*MAX_COST_VALUE;
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
		l!=myData->src->GetWidth()-myData->target_size.width)
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
		min_diff = 0.1*PairwiseDiff(myData->src->GetPixelIntensity(cur_match,y),
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

	cost += min_diff;
	
	return cost;
}

/*
 * Compute color difference between two mapped points
 */
double PairImgColorDiff(Picture *src, int x1, int y1, 
						int x2, int y2, int l1, int l2)
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
		//printf("(%d,%d) color components: %d,%d,%d\n",src->GetPixel(y2+l2,x2).r,src->GetPixel(y2+l2,x2).g,src->GetPixel(y2+l2,x2).b);
		diff += pow((double)src->GetPixel(x2+l2,y2).r-src->GetPixel(x1+l1+x_offset,y1+y_offset).r,2);
		diff += pow((double)src->GetPixel(x2+l2-x_offset,y2-y_offset).r-src->GetPixel(x1+l1,y1).r,2);
		diff += pow((double)src->GetPixel(x2+l2,y2).g-src->GetPixel(x1+l1+x_offset,y1+y_offset).g,2);
		diff += pow((double)src->GetPixel(x2+l2-x_offset,y2-y_offset).g-src->GetPixel(x1+l1,y1).g,2);
		diff += pow((double)src->GetPixel(x2+l2,y2).b-src->GetPixel(x1+l1+x_offset,y1+y_offset).b,2);
		diff += pow((double)src->GetPixel(x2+l2-x_offset,y2-y_offset).b-src->GetPixel(x1+l1,y1).b,2);
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
double PairImgGradientDiff(gradient2D *gradient, int x1, int y1, 
						   int x2, int y2, int l1, int l2)
{
	double diff = 0.0;
	int width = gradient->dx->NumOfCols();
	int height = gradient->dx->NumOfRows();
	int x_offset = x2-x1;
	int y_offset = y2-y1;

	if (x2+l2>0 && x2+l2<=width && x1+l1+x_offset>0 && x1+l1+x_offset<=width &&
		x2+l2-x_offset>0 && x2+l2-x_offset<=width && x1+l1>0 && x1+l1<=width)
	{
		diff += pow((gradient->dx->Get(y2,x2+l2)-gradient->dx->Get(y1+y_offset,x1+l1+x_offset)),2)+
				pow((gradient->dy->Get(y2,x2+l2)-gradient->dy->Get(y1+y_offset,x1+l1+x_offset)),2);
		diff += pow((gradient->dx->Get(y2-y_offset,x2+l2-x_offset)-gradient->dx->Get(y1,x1+l1)),2)+
				pow((gradient->dy->Get(y2-y_offset,x2+l2-x_offset)-gradient->dy->Get(y1,x1+l1)),2);
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
		cost += myData->alpha*PairImgColorDiff(myData->src, x1, y1, x2, y2, l1, l2);
		cost += myData->beta*PairImgGradientDiff(myData->gradient, x1+1, y1+1, x2+1, y2+1, l1, l2);
	}
	else 
	{
		cost = 100000*MAX_COST_VALUE;
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
	if (x==myData->target_size.width-1 && 
		l!=myData->src->GetPicture(0)->GetWidth()-myData->target_size.width)
		return 100000*MAX_COST_VALUE;
	
	return cost;
}

/*
 * Compute color difference between two mapped points
 */
double VideoColorDiff(PictureList *src, int x1, int y1, int t1, 
					  int x2, int y2, int t2, int l1, int l2)
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
		diff += pow((double)src->GetPicture(t2)->GetPixel(x2+l2,y2).r-
					src->GetPicture(t2)->GetPixel(x1+l1+x_offset,y1+y_offset).r,2.0);
		diff += pow((double)src->GetPicture(t1)->GetPixel(x2+l2-x_offset,y2-y_offset).r-
					src->GetPicture(t1)->GetPixel(x1+l1,y1).r,2.0);
		diff += pow((double)src->GetPicture(t2)->GetPixel(x2+l2,y2).g-
					src->GetPicture(t2)->GetPixel(x1+l1+x_offset,y1+y_offset).g,2.0);
		diff += pow((double)src->GetPicture(t1)->GetPixel(x2+l2-x_offset,y2-y_offset).g-
					src->GetPicture(t1)->GetPixel(x1+l1,y1).g,2.0);
		diff += pow((double)src->GetPicture(t2)->GetPixel(x2+l2,y2).b-
					src->GetPicture(t2)->GetPixel(x1+l1+x_offset,y1+y_offset).b,2.0);
		diff += pow((double)src->GetPicture(t1)->GetPixel(x2+l2-x_offset,y2-y_offset).b-
					src->GetPicture(t1)->GetPixel(x1+l1,y1).b,2.0);
	} else
	{
		diff += 100000*MAX_COST_VALUE;
	}	
	//printf("ColorDiff: finished\n");

	//printf("ColorDiff: cost between (%d,%d) and (%d,%d) with labels %d and %d is %d\n",x1,y1,x2,y2,l1,l2,diff);
	//if (x1==x2 && y1==y2)
	//	return 0.001*diff;
	//else
		return diff;
}

/*
 * Compute gradient difference between two mapped points
 */
double VideoGradientDiff(Matrix *gradient, int x1, int y1, int t1, 
						 int x2, int y2, int t2, int l1, int l2)
{
	double diff = 0.0;
	int width = gradient[0].NumOfCols();
	int height = gradient[0].NumOfRows();
	int x_offset = x2-x1;
	int y_offset = y2-y1;
	int t_offset = t2-t1;

	if (x2+l2>0 && x2+l2<=width && x1+l1+x_offset>0 && x1+l1+x_offset<=width &&
		x2+l2-x_offset>0 && x2+l2-x_offset<=width)
	{
		diff += pow((gradient[t2].Get(y2,x2+l2)-gradient[t2].Get(y1+y_offset,x1+l1+x_offset)),2.0);
		diff += pow((gradient[t1].Get(y2-y_offset,x2+l2-x_offset)-gradient[t1].Get(y1,x1+l1)),2.0);
	} else
	{	
		diff += 100000*MAX_COST_VALUE;
	}

	//if (x1==x2 && y1==y2)
	//	return 0.001*diff;
	//else
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
		cost += myData->alpha*VideoColorDiff(myData->src, x1, y1, t1, x2, y2, t2, l1, l2);
		cost += myData->beta*VideoGradientDiff(myData->gradient, x1+1, y1+1, t1, x2+1, y2+1, t2, l1, l2);
	}
	else 
	{
		cost = 100000*MAX_COST_VALUE;
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
	Picture *seam_img = new Picture(ref->GetWidth(),
									ref->GetHeight());
	
	pixelType pix;
	for (int y = 0; y < ref->GetHeight(); y++)
	{
		bool found_seam = false;
		for (int x = 0; x < ref->GetWidth(); x++)
		{
			int idx = y*ref->GetWidth()+x;
			if (labels[idx]>0 && !found_seam)
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

			cur_idx++;
		}

		result->SetPicture(t,frame);
		if (save)
		{
			frame->Save(target_name);
			//map->Save(map_name);
		}

		delete frame;
		delete map;
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
	Matrix *gradient = new Matrix[src->Lists[level].GetLength()];
	for (int t = 0; t < src->Lists[level].GetLength(); t++)
	{
		gradient2D* grad = Gradient(src->Lists[level].GetPicture(t));
		gradient[t] = *(grad->dx) + *(grad->dy);
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

		delete [] gradient;
		delete [] up_result;
		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	return result;

}

/*
 * Function to find the optimal single seam
 */
PictureList *GridGraph_SeamCut(PictureList *src, double ratio, videoSize &target_size, videoSize &up_size,
								int num_labels, float alpha, float beta, char *target_path, int * &band)
{
	// set up the needed data to pass to function for the data costs
	Matrix *gradient = new Matrix[src->GetLength()];
	for (int t = 0; t < src->GetLength(); t++)
	{
		gradient2D* grad = Gradient(src->GetPicture(t));
		gradient[t] = *(grad->dx) + *(grad->dy);
		delete grad->dx;
		delete grad->dy;
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
		toDataFn.target_size = target_size;
		gc->setDataCost(&VideoDataFn,&toDataFn);

		// smoothness comes from function pointer
		ForVideoSmoothFn toSmoothFn;
		toSmoothFn.src = src;		
		toSmoothFn.gradient = gradient;
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
		band = UpsamplingShiftMap(labels,target_size,ratio,up_size);											 
		//delete [] result;

		// DEBUG: observe interpolated map
		/*
		Picture *upsampled_map = NULL;
		for (int t = 0; t < up_size.time; t++)
		{
			upsampled_map = new Picture(up_size.width,up_size.height);
			for (int y = 0; y < up_size.height; y++)
			{
				for (int x = 0; x < up_size.width; x++)
				{
					pixelType pix;
					if (x==band[t*up_size.height+y])
					{
						pix.r = 255;
						pix.g = 255;
						pix.b = 255;
					}
					else
					{
						pix.r = 0;
						pix.g = 0;
						pix.b = 0;
					}
					upsampled_map->SetPixel(x,y,pix);
				}
			}
			upsampled_map->Save("upsampled_map.ppm");
			delete upsampled_map;
		}
		*/

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

	delete [] gradient;
	return result;

}

Picture *Subband_Picture(Picture *ref, int *subband, int idx, double ratio, 
						 int &lbound, int &ubound, int *&labels)
{
	bool modify_labels = true;
	if (lbound<0 && ubound <0)
	{
		modify_labels = false;
		for (int y = 0; y < ref->GetHeight(); y++)
		{
			int lval = max(subband[idx+y]-1*ratio,0);
			int uval = min(subband[idx+y]+2*ratio-1,ref->GetWidth()-1);
			if (lval<lbound || lbound<0)
				lbound = lval;
			if (uval>ubound || ubound<0)
				ubound = uval;
		}
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
			if (modify_labels)
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
		for (int x = 0; x < src->GetWidth()-1; x++)
		{
			int idx = y*result->GetWidth()+x;
			if (x >= lbound && x <= ubound-1)
			{
				pixel = band_img->GetPixel(x-lbound,y);
				result->SetPixel(x,y,pixel);
				new_labels[idx] = labels[y*(ubound-lbound)+x-lbound];
			}
			else
			{
				pixel = src->GetPixel(x,y);
				result->SetPixel(x,y,pixel);
				if (x < lbound)
					new_labels[idx] = 0;
				else
					new_labels[idx] = 1;
			}
		}
	}

	delete [] labels;
	labels = new_labels;
	return result;
}

Picture *SingleImg_SeamCut(Picture *img, imageSize &target_size,
						   float alpha, float beta, int *&labels)
{
	int num_pixels = target_size.width*target_size.height;
	Picture *result = NULL;

	try{
		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(target_size.width,
																  target_size.height,
																  2);

		// set up the needed data to pass to function for the data costs
		gradient2D *gradient = Gradient(img);

		ForSingleImgDataFn toDataFn;
		toDataFn.src = img;
		toDataFn.gradient = gradient;
		toDataFn.target_size = target_size;
		gc->setDataCost(&SingleImgDataFn,&toDataFn);

		// smoothness comes from function pointer
		ForSingleImgSmoothFn toSmoothFn;
		toSmoothFn.src = img;		
		toSmoothFn.gradient = gradient;
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

		/* generate retargeted subband image */
		result = GenerateRetargetPicture(labels, img, 
										 target_size.width, target_size.height);
		SaveShiftMap(labels,target_size.width,target_size.height,"shift.ppm");

		delete gradient->dx;
		delete gradient->dy;
		delete gradient;
		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	return result;
}

Picture *PairImg_SeamCut(Picture *cur_frame, Picture *pre_frame, imageSize &target_size, 
						 float alpha, float beta, int *&pre_labels, bool tconsistency)
{
	// set up the needed data to pass to function for the data costs
	gradient2D *gradient = Gradient(cur_frame);

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
			toDataFn.target_size = target_size;
			gc->setDataCost(&PairImgDataFn,&toDataFn);
		}
		else
		{
			ForSingleImgDataFn toDataFn;
			toDataFn.src = cur_frame;
			toDataFn.gradient = gradient;
			toDataFn.target_size = target_size;
			gc->setDataCost(&SingleImgDataFn,&toDataFn);
		}

		// smoothness comes from function pointer
		ForPairImgSmoothFn toSmoothFn;
		toSmoothFn.src = cur_frame;		
		toSmoothFn.gradient = gradient;
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

	delete [] pre_labels;
	pre_labels = labels;
	return result;

}

/*
 *
 */
PictureList *Refine_Seam(int *band, PictureList *src, double ratio,
						 float alpha, float beta, char *name, bool tconsistency,bool banded)
{
	imageSize target_size;
	PictureList *result = new PictureList(src->GetMaxWidth()-1,
										  src->GetMaxHeight(),
										  src->GetLength());
	/* process first frame */
	int lbound = -1;
	int ubound = -1;
	int *empty_labels = new int[1];
	Picture *band_img;
	if (!banded)
		band_img = src->GetPicture(0);
	else
		band_img = Subband_Picture(src->GetPicture(0),band,0,ratio,lbound,ubound,empty_labels);
	delete [] empty_labels;
	target_size.width = band_img->GetWidth()-1;
	target_size.height = band_img->GetHeight();
	int *labels = new int[target_size.width*target_size.height];
	Picture *retargeted_band_img = SingleImg_SeamCut(band_img,target_size,alpha,beta,labels);
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

	/*
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
	*/

	/* process 2nd frame to t-1 frame */
	Picture *pre_band_img;
	for (int t = 1; t < src->GetLength(); t++)
	{
		lbound = -1;
		ubound = -1;
		empty_labels = new int[1];
		if (!banded)
			band_img = src->GetPicture(t);
		else
			band_img = Subband_Picture(src->GetPicture(t),band,t*src->GetMaxHeight(),ratio,lbound,ubound,empty_labels);
		delete [] empty_labels; 
		ubound--;
		if (!banded)
			pre_band_img = result->GetPicture(t-1);
		else
			pre_band_img = Subband_Picture(result->GetPicture(t-1),band,t*src->GetMaxHeight(),ratio,lbound,ubound,labels);
		target_size.width = band_img->GetWidth()-1;
		target_size.height = band_img->GetHeight();
		retargeted_band_img = PairImg_SeamCut(band_img,pre_band_img,target_size,alpha,beta,labels,tconsistency);
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

		/*
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
		*/
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
	videoSize target_size, origin_size;

	PictureList *shot = new PictureList(input_path);

	PictureList *source = NULL;
	PictureList *target = shot;
	while (removed_seam>0)
	{	
		if (level>1)
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
		target = GridGraph_SeamCut(source,pow(2.0,level),target_size,origin_size,
								   num_labels,alpha,beta,output_path,band);

		// generate output result;
		/*
		target = GenerateRetargetResult(shift_map, target,
										origin_size.width,
										origin_size.height,"..\\..\\..\\results\\video5\\1\\",true);
		*/

		removed_seam--;
		delete [] band;
		if (level>1)
		{
			delete [] spyramid->Lists;
			delete spyramid;
			spyramid = NULL;
		}
		else
			delete source;
	}

	if (level>1)
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
	videoSize target_size, origin_size;

	shot = new PictureList(input_path);	
	//spyramid = ListPyramid(shot,level+1);

	PictureList *source;
	PictureList *target = shot;
	while (removed_seam>0)
	{	
		if (level>1)
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
		/* refine seam in higher resolution */
		target = Refine_Seam(band,target,pow(2.0,level),
							 alpha,beta,output_path,true,false);

		removed_seam--;
		delete [] band;
		if (level>1)
		{
			delete [] spyramid->Lists;
			delete spyramid;
			spyramid = NULL;
		}
		else
			delete source;
	}

	if (level>1)
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
	videoSize target_size, origin_size;

	shot = new PictureList(input_path);	
	//spyramid = ListPyramid(shot,level+1);

	PictureList *source;
	PictureList *target = shot;
	while (removed_seam>0)
	{	
		if (level>1)
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
		/* refine seam in higher resolution */
		target = Refine_Seam(band,target,pow(2.0,level),
							 alpha,beta,output_path,false,false);

		removed_seam--;
		delete [] band;
		if (level>1)
		{
			delete [] spyramid->Lists;
			delete spyramid;
			spyramid = NULL;
		}
		else
			delete source;
	}

	if (level>1)
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
	videoSize target_size, origin_size;

	shot = new PictureList(input_path);	
	//spyramid = ListPyramid(shot,level+1);

	PictureList *source;
	PictureList *target = shot;
	while (removed_seam>0)
	{	
		spyramid = ListPyramid(target,level+1);
		source = &(spyramid->Lists[level]);	
		target = &(spyramid->Lists[0]);
		num_labels = 2;
		target_size.width = source->GetMaxWidth()-1;
		target_size.height = source->GetMaxHeight();
		target_size.time = source->GetLength();
		origin_size.width = spyramid->Lists[0].GetMaxWidth()-1;
		origin_size.height = spyramid->Lists[0].GetMaxHeight();
		origin_size.time = spyramid->Lists[0].GetLength();

		/* graph cut in low resolution */
		source = GridGraph_SeamCut(source,pow(2.0,level),target_size,
								   origin_size,num_labels,alpha,
								   beta,output_path,band);

		/* refine seam in higher resolution */
		target = Refine_Seam(band,target,pow(2.0,level),
							 alpha,beta,output_path,true,true);


		removed_seam--;
		delete [] band;		
		delete [] spyramid->Lists;
		delete spyramid;
		spyramid = NULL;		
	}

	delete shot;
	delete source;
	delete target;
}

/*
 * main entry of the grogram
 */ 
int main(int argc, char **argv)
{
	PictureList *shot = NULL;
	listPyramidType *spyramid = NULL;
	int width, height, time;
	int num_pixels, num_labels;
	int *band = NULL;	// store shift labels of every pixel
	videoSize target_size, origin_size;

	if (argc<7)
	{
		cout << "Usage: mg_shift_map_3d <input_folder> <alpha> <beta> <number of removed pixel> <output_folder> method" << endl;
		return 0;
		//default parameters
	}

	int level = 0; // gpyramid->Levels-1

	switch (atoi(argv[6]))
	{
		case 0:	// 3D temporal consistent shift-map
		{
			ResizeVideo_3D(argv[1],atof(argv[2]),atof(argv[3]),atoi(argv[4]),argv[5],level);
			break;
		}
		case 1:	// 2D incremental temporal consistent shift-map
		{
			ResizeVideo_2D_Incremental(argv[1],atof(argv[2]),atof(argv[3]),atoi(argv[4]),argv[5],level);
			break;
		}
		case 2: // hybrid 3D & 2D temporal consistent shift-map
		{
			ResizeVideo_Hybrid(argv[1],atof(argv[2]),atof(argv[3]),atoi(argv[4]),argv[5],level);
			break;
		}
		case 3: // 2D incremental shift-map without temporal consistency
		{
			ResizeImages_2D_Incremental(argv[1],atof(argv[2]),atof(argv[3]),atoi(argv[4]),argv[5],level);
		}
	}
	
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////