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
 * data structure for motion-guided data term
 */
struct ForMGDataFn
{
	PictureList *src;
	Matrix *gradient;
	videoSize target_size;
};

/*
 * data structure for smoothness term
 */
struct ForSmoothFn
{
	PictureList *src;
	Matrix *gradient;
	videoSize target_size;
	float alpha;
	float beta;
};

/*
 * data structure for smoothness term
 */
struct ForSmoothFn_crossCut
{
	PictureList *src;
	PictureList *origin;
	Matrix *gradient;
	double ratio;
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
 * Motion-guided data term calculation
 */
double MGDataFn(int p, int l, void *data)
{
	ForMGDataFn *myData = (ForMGDataFn *) data;
	int width = myData->target_size.width;
	int height = myData->target_size.height;
	int time = myData->target_size.time;

	int	t = p / (width*height);
	int x = (p % (width*height)) % width;
	int y = (p % (width*height)) / width;

	double cost = 0.0;

	// pixel arrangement
	if (x==0 && l!=0)
		cost = 100000*MAX_COST_VALUE;
	if (x==myData->target_size.width-1 && 
		l!=myData->src->GetPicture(0)->GetWidth()-myData->target_size.width)
		cost = 100000*MAX_COST_VALUE;
	
	return cost;
}

/*
 * Compute color difference between two mapped points
 */
double ColorDiff(PictureList *src, int x1, int y1, int t1, 
				 int x2, int y2, int t2, int l1, int l2)
{
	double diff = 0.0;	
	int width = src->GetPicture(t1)->GetWidth();
	int height = src->GetPicture(t1)->GetHeight(); 
	int time = src->GetLength();
	int x_offset = x2-x1;
	int y_offset = y2-y1;
	int t_offset = t2-t1;

	if (x2+l2>=0 && x2+l2<width && x1+l1+x_offset>=0 && x1+l1+x_offset<width)
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
		diff += 10000*MAX_COST_VALUE;
	}	
	//printf("ColorDiff: finished\n");

	//printf("ColorDiff: cost between (%d,%d) and (%d,%d) with labels %d and %d is %d\n",x1,y1,x2,y2,l1,l2,diff);
	//if (x1==x2 && y1==y2)
	//	return 0.001*diff;
	//else
		return diff;
}

/*
 * Compute color difference between two mapped points
 */
double CrossColorDiff(PictureList *src, int x1, int y1, int t1, 
					  int x2, int y2, int t2, int l1, int l2, double ratio)
{
	double diff = 0.0;	
	int width = src->GetPicture(t1)->GetWidth();
	int height = src->GetPicture(t1)->GetHeight(); 
	int time = src->GetLength();
	int x_offset = x2-x1;
	int y_offset = y2-y1;
	int t_offset = t2-t1;

	if (x2+l2>=0 && x2+l2<width && x1+l1+x_offset>=0 && x1+l1+x_offset<width)
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
		diff += 10000*MAX_COST_VALUE;
	}

	for (int nn_x_offset = -ratio+1; nn_x_offset <= 0; nn_x_offset++)
	{
		for (int nn_y_offset = -ratio+1; nn_y_offset <= 0; nn_y_offset++)
		{
			if (x2+l2+nn_x_offset>=0 && x2+l2+nn_x_offset<width && 
				x1+l1+x_offset+nn_x_offset>=0 && x1+l1+x_offset+nn_x_offset<width &&
				y2+nn_y_offset>=0 && y2+nn_y_offset<height)
			{
				double nn_cost = 0.0;
				nn_cost += pow((double)src->GetPicture(t2)->GetPixel(x2+l2+nn_x_offset,y2+nn_y_offset).r-
								src->GetPicture(t2)->GetPixel(x1+l1+x_offset+nn_x_offset,y1+y_offset+nn_y_offset).r,2.0);
				nn_cost += pow((double)src->GetPicture(t1)->GetPixel(x2+l2-x_offset+nn_x_offset,y2-y_offset+nn_y_offset).r-
								src->GetPicture(t1)->GetPixel(x1+l1+nn_x_offset,y1+nn_y_offset).r,2.0);
				nn_cost += pow((double)src->GetPicture(t2)->GetPixel(x2+l2+nn_x_offset,y2+nn_y_offset).g-
								src->GetPicture(t2)->GetPixel(x1+l1+x_offset+nn_x_offset,y1+y_offset+nn_y_offset).g,2.0);
				nn_cost += pow((double)src->GetPicture(t1)->GetPixel(x2+l2-x_offset+nn_x_offset,y2-y_offset+nn_y_offset).g-
								src->GetPicture(t1)->GetPixel(x1+l1+nn_x_offset,y1+nn_y_offset).g,2.0);
				nn_cost += pow((double)src->GetPicture(t2)->GetPixel(x2+l2+nn_x_offset,y2+nn_y_offset).b-
								src->GetPicture(t2)->GetPixel(x1+l1+x_offset+nn_x_offset,y1+y_offset+nn_y_offset).b,2.0);
				nn_cost += pow((double)src->GetPicture(t1)->GetPixel(x2+l2-x_offset+nn_x_offset,y2-y_offset+nn_y_offset).b-
								src->GetPicture(t1)->GetPixel(x1+l1+nn_x_offset,y1+nn_y_offset).b,2.0);

				if (nn_cost<diff)
					diff = nn_cost;
			}
		}
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
double GradientDiff(Matrix *gradient, int x1, int y1, int t1, 
					int x2, int y2, int t2, int l1, int l2)
{
	double diff = 0.0;
	int width = gradient[0].NumOfCols();
	int height = gradient[0].NumOfRows();
	int x_offset = x2-x1;
	int y_offset = y2-y1;
	int t_offset = t2-t1;

	if (x2+l2>0 && x2+l2<=width && x1+l1+x_offset>0 && x1+l1+x_offset<=width)
	{
		diff += pow((gradient[t2].Get(y2,x2+l2)-gradient[t2].Get(y1+y_offset,x1+l1+x_offset)),2.0);
		diff += pow((gradient[t1].Get(y2-y_offset,x2+l2-x_offset)-gradient[t1].Get(y1,x1+l1)),2.0);
	} else
	{	
		diff += 10000*MAX_COST_VALUE;
	}

	//if (x1==x2 && y1==y2)
	//	return 0.001*diff;
	//else
		return diff;
}

/*
 * Compute gradient difference between two mapped points
 */
double CrossGradientDiff(Matrix *gradient, int x1, int y1, int t1, 
						 int x2, int y2, int t2, int l1, int l2, double ratio)
{
	double diff = 0.0;
	int width = gradient[0].NumOfCols();
	int height = gradient[0].NumOfRows();
	int x_offset = x2-x1;
	int y_offset = y2-y1;
	int t_offset = t2-t1;

	if (x2+l2>0 && x2+l2<=width && x1+l1+x_offset>0 && x1+l1+x_offset<=width)
	{
		diff += pow((gradient[t2].Get(y2,x2+l2)-gradient[t2].Get(y1+y_offset,x1+l1+x_offset)),2.0);
		diff += pow((gradient[t1].Get(y2-y_offset,x2+l2-x_offset)-gradient[t1].Get(y1,x1+l1)),2.0);
	} else
	{	
		diff += 10000*MAX_COST_VALUE;
	}

	for (int nn_x_offset = -ratio+1; nn_x_offset <= 0; nn_x_offset++)
	{
		for (int nn_y_offset = -ratio+1; nn_y_offset <= 0; nn_y_offset++)
		{
			if (x2+l2+nn_x_offset>0 && x2+l2+nn_x_offset<=width && 
				x1+l1+x_offset+nn_x_offset>0 && x1+l1+x_offset+nn_x_offset<=width &&
				y2+nn_y_offset>0 && y2+nn_y_offset<=height)
			{
				double nn_cost = 0.0;
				nn_cost += pow((gradient[t2].Get(y2+nn_y_offset,x2+l2+nn_x_offset)-
								gradient[t2].Get(y1+y_offset+nn_y_offset,x1+l1+x_offset+nn_x_offset)),2.0);
				nn_cost += pow((gradient[t1].Get(y2-y_offset+nn_y_offset,x2+l2-x_offset+nn_x_offset)-
								gradient[t1].Get(y1+nn_y_offset,x1+l1+nn_x_offset)),2.0);
				if (nn_cost<diff)
					diff = nn_cost;
			}
		}
	}

	//if (x1==x2 && y1==y2)
	//	return 0.001*diff;
	//else
		return diff;
}

/*
 * Smoothness term calculation
 */
double smoothFn(int p1, int p2, int l1, int l2, void *data)
{
	ForSmoothFn *myData = (ForSmoothFn *) data;
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

	if (abs(x1-x2)<=1 && abs(y1-y2)<=1 && abs(t1-t2)<=1)
	{		
		cost += myData->alpha*ColorDiff(myData->src, x1, y1, t1, x2, y2, t2, l1, l2);
		cost += myData->beta*GradientDiff(myData->gradient, x1+1, y1+1, t1, x2+1, y2+1, t2, l1, l2);
	}
	else 
	{
		cost = 10000*MAX_COST_VALUE;
	}

	//printf("smoothFn: computing cost between (%d,%d) and (%d,%d) with labels %d and %d : %d\n",y1,x1,y2,x2,l1,l2,cost);
	return cost;
}

/*
 * Smoothness term calculation
 */
double smoothFn_crossCut(int p1, int p2, int l1, int l2, void *data)
{
	ForSmoothFn_crossCut *myData = (ForSmoothFn_crossCut *) data;
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

	if (abs(x1-x2)<=1 && abs(y1-y2)<=1 && abs(t1-t2)<=1)
	{	
		int x_offset = x2-x1;
		int y_offset = y2-y1;
		cost += myData->alpha*CrossColorDiff(myData->origin, x1*myData->ratio, y1*myData->ratio, t1, 
											 x2*myData->ratio, y2*myData->ratio, t2, 
											 l1*myData->ratio, l2*myData->ratio, myData->ratio);
		cost += myData->beta*CrossGradientDiff(myData->gradient, x1*myData->ratio+1, y1*myData->ratio+1, t1, 
											   x2*myData->ratio+1, y2*myData->ratio+1, t2, 
											   l1*myData->ratio, l2*myData->ratio, myData->ratio);
	}
	else 
	{
		cost = 10000*MAX_COST_VALUE;
	}

	//printf("smoothFn: computing cost between (%d,%d) and (%d,%d) with labels %d and %d : %d\n",y1,x1,y2,x2,l1,l2,cost);
	return cost;
}

/*
 * Function to interpolate binary shift map of low resolution to higher resolution
 */
int *UpsamplingShiftMap(int *result, videoSize size, double ratio, videoSize up_size)
{
	int *up_result = new int[up_size.width*up_size.height*up_size.time];

	for (int t = 0; t < up_size.time; t++)
	{
		for (int y = 0; y < up_size.height; y++)
		{
			for (int x = 0; x < up_size.width; x++)
			{
				int p = t*up_size.width*up_size.height+y*up_size.width+x;
				int idx = Downsampling3DIndex(p,up_size,size,ratio);
				int label = result[idx];//ceil((double)result[idx]*ratio);
				up_result[p] = label;
			} // end for x
		} // end for y
	}

	return up_result;
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

			map_pix.r = labels[cur_idx];
			map_pix.g = labels[cur_idx];
			map_pix.b = labels[cur_idx];
			map->SetPixelIntensity(x,y,map_pix);

			cur_idx++;
		}

		result->Save(result->GetName());
		//map->Save(map->GetName());

		delete result;
		delete map;
	}
}

/*
 * Function to generate and save retargeted image
 * using shift-map labels
 */
PictureList *GenerateRetargetResult(int *labels, PictureList *src,int width, int height,char *name)
{
	int cur_idx = 0;
	PictureList *result = new PictureList(width,height,src->GetLength());
	for (int t = 0; t < src->GetLength(); t++)
	{
		Picture *frame = new Picture(width,height);
		char target_name[512] = {'\0'};
		//strcat(target_name,name);
		strcat(target_name,src->GetPicture(t)->GetName());
		frame->SetName(target_name);
	
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

			cur_idx++;
		}

		result->SetPicture(t,frame);
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
		ForMGDataFn toDataFn;
		toDataFn.src = &(src->Lists[level]);
		toDataFn.gradient = gradient;
		toDataFn.target_size = target_size;
		gc->setDataCost(&MGDataFn,&toDataFn);

		// smoothness comes from function pointer
		ForSmoothFn toSmoothFn;
		toSmoothFn.src = &(src->Lists[level]);		
		toSmoothFn.gradient = gradient;
		toSmoothFn.target_size = target_size;
		toSmoothFn.alpha = alpha;
		toSmoothFn.beta = beta;
		gc->setSmoothCost(&smoothFn, &toSmoothFn);

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
								int num_labels, float alpha, float beta, char *target_path,int * &up_result)
{
	// set up the needed data to pass to function for the data costs
	Matrix *gradient = new Matrix[src->GetLength()];
	for (int t = 0; t < src->GetLength(); t++)
	{
		gradient2D* grad = Gradient(src->GetPicture(t));
		gradient[t] = *(grad->dx) + *(grad->dy);
	}


	int num_pixels = target_size.width*target_size.height*target_size.time;
	int *label = new int[num_pixels];   // stores result of optimization
	PictureList *result;

	try{
		GCoptimization3DGridGraph *gc = new GCoptimization3DGridGraph(target_size.width,
																	  target_size.height,
																	  target_size.time,
																	  num_labels);


		// TODO: replace the hardcode of gradient threoshold
		// for shot boundary
		// data terms comes from function pointer
		ForMGDataFn toDataFn;
		toDataFn.src = src;
		toDataFn.gradient = gradient;
		toDataFn.target_size = target_size;
		gc->setDataCost(&MGDataFn,&toDataFn);

		// smoothness comes from function pointer
		ForSmoothFn toSmoothFn;
		toSmoothFn.src = src;		
		toSmoothFn.gradient = gradient;
		toSmoothFn.target_size = target_size;
		toSmoothFn.alpha = alpha;
		toSmoothFn.beta = beta;
		gc->setSmoothCost(&smoothFn, &toSmoothFn);

		printf("Before optimization energy is %f\n",gc->compute_energy());
		gc->expansion();// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("After optimization energy is %f\n",gc->compute_energy());

		// obtain the labels
		int assign_idx, assignment;
		for ( int  i = 0; i < num_pixels; i++ )
		{
			//printf("The optimal label for pixel %d is %d\n",i,gc->whatLabel(i));
			label[i] = gc->whatLabel(i);
		}		

		// upsampling shift-map from low-resolution
		// to high-resolution
		up_result = UpsamplingShiftMap(label,target_size,ratio,up_size);
		//delete [] result;

		// update_
		result = GenerateRetargetResult(label, src, 
										target_size.width, 
										target_size.height,target_path);
		

		delete [] gradient;
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
PictureList *GridGraph_CrossSeamCut(listPyramidType *src, int level, videoSize &target_size, videoSize &up_size,
									int num_labels, float alpha, float beta, char *target_path,int * &up_result)
{
	// set up the needed data to pass to function for the data costs
	Matrix *gradient = new Matrix[src->Lists[0].GetLength()];
	for (int t = 0; t < src->Lists[0].GetLength(); t++)
	{
		gradient2D* grad = Gradient(src->Lists[0].GetPicture(t));
		gradient[t] = *(grad->dx) + *(grad->dy);
	}


	int num_pixels = target_size.width*target_size.height*target_size.time;
	int *label = new int[num_pixels];   // stores result of optimization
	PictureList *result;

	try{
		GCoptimization3DGridGraph *gc = new GCoptimization3DGridGraph(target_size.width,
																	  target_size.height,
																	  target_size.time,
																	  num_labels);


		// TODO: replace the hardcode of gradient threoshold
		// for shot boundary
		// data terms comes from function pointer
		ForMGDataFn toDataFn;
		toDataFn.src = &(src->Lists[level]);
		toDataFn.gradient = gradient;
		toDataFn.target_size = target_size;
		gc->setDataCost(&MGDataFn,&toDataFn);

		// smoothness comes from function pointer
		double ratio = pow(2.0,level);
		ForSmoothFn_crossCut toSmoothFn;
		toSmoothFn.src = &(src->Lists[level]);		
		toSmoothFn.gradient = gradient;
		toSmoothFn.origin = &(src->Lists[0]);
		toSmoothFn.ratio = ratio;
		toSmoothFn.target_size = target_size;
		toSmoothFn.alpha = alpha;
		toSmoothFn.beta = beta;
		gc->setSmoothCost(&smoothFn_crossCut, &toSmoothFn);

		printf("Before optimization energy is %f\n",gc->compute_energy());
		gc->expansion();// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("After optimization energy is %f\n",gc->compute_energy());

		// obtain the labels
		int assign_idx, assignment;
		for ( int  i = 0; i < num_pixels; i++ )
		{
			//printf("The optimal label for pixel %d is %d\n",i,gc->whatLabel(i));
			label[i] = gc->whatLabel(i);
		}		

		// upsampling shift-map from low-resolution
		// to high-resolution
		up_result = UpsamplingShiftMap(label,target_size,ratio,up_size);
		//delete [] result;

		// update_
		result = GenerateRetargetResult(label, &(src->Lists[level]), 
										target_size.width, 
										target_size.height,target_path);
		

		delete [] gradient;
		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	return result;

}


/*
 *
 */
Matrix *Simple_SaliencyMap(PictureList *frames, int time, int level)
{
	double tdt = 0.0;
	Matrix *smap = new Matrix(frames->GetPicture(0)->GetHeight(),frames->GetPicture(0)->GetWidth());
	smap->LoadZero();
	for (int t = 0; t < time-1; t++)
	{
		Matrix *gradient = FrameDifference(frames->GetPicture(t),frames->GetPicture(t+1),tdt,30.0);
		for (int i = 1; i <= gradient->NumOfRows(); i++)
			for (int j = 1; j <= gradient->NumOfCols(); j++)
				smap->Set(i,j,smap->Get(i,j)+gradient->Get(i,j));
		delete gradient;
	}

	Matrix *result = NULL;
	for (int i = 1; i <= level; i++)
	{
		result = ReduceMatrix(smap);
		delete smap;
		smap = result;
	}

	return result;
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
	int *shift_map = NULL;	// store shift labels of every pixel
	videoSize target_size, origin_size;

	if (argc<6)
	{
		cout << "Usage: mg_shift_map_3d <input_folder> <alpha> <beta> <number of removed pixel> <output_folder>" << endl;
		return 0;
		//default parameters
	}

	// load input video
	int level = 3; // gpyramid->Levels-1
	shot = new PictureList(argv[1]);	
	//spyramid = ListPyramid(shot,level+1);

	int left_seam = atof(argv[4]);
	PictureList *source;
	PictureList *target = shot;
	while (left_seam>0)
	{	
		spyramid = ListPyramid(target,level+1);
		int max_remove = min(left_seam,floor((double)(spyramid->Lists[level].GetMaxWidth()/2)));
		for (int seam = 1; seam <= max_remove; seam++)
		{
			if (spyramid==NULL)
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
									   origin_size,num_labels,atof(argv[2]),
									   atof(argv[3]),argv[5],shift_map);

			/* cross graph cut */
			/*
			source = GridGraph_CrossSeamCut(spyramid,level,target_size,
											origin_size,num_labels,atof(argv[2]),
											atof(argv[3]),argv[5],shift_map);
			*/

			// generate output result;
			target = GenerateRetargetResult(shift_map, target,
											origin_size.width,
											origin_size.height,argv[5]);

			left_seam--;
			delete [] shift_map;
			
			delete [] spyramid->Lists;
			delete spyramid;
			spyramid = NULL;
			//level--;			
		}

				
	}

	target->Save(argv[5]);

	/*
	width = spyramid->Lists[level].GetPicture(1)->GetWidth();
	height = spyramid->Lists[level].GetPicture(1)->GetHeight();
	time = spyramid->Lists[level].GetLength();
	num_pixels = ceil(width*atof(argv[4]))*height*time;
			
	num_labels = atof(argv[4])+1;
	width = width-atof(argv[4]);

	target_size.width = width;
	target_size.height = height;
	target_size.time = time;
		
	shift_map = GridGraph_GraphCut(spyramid,level,target_size,num_labels,
									 atof(argv[2]),atof(argv[3]),argv[5]);
    */

	delete shot;
	//delete [] spyramid->Lists;
	//delete spyramid;	
	delete source;
	delete target;
	//delete [] shift_map;

	return 1;

}

/////////////////////////////////////////////////////////////////////////////////