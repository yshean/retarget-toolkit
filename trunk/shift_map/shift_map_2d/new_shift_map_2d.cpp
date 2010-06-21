//////////////////////////////////////////////////////////////////////////////
// Testing for new idea about generalized shift-map
//
/////////////////////////////////////////////////////////////////////////////
//
// Yiqun Hu 2010-02-07

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <algorithm>

#include "../Common/picture.h"
#include "../Common/utils.h"
#include "../GCoptimization/GCoptimization.h"

using namespace std;

struct ShiftLabels
{
	double *labels;
	int num_labels;
};


struct ForDataFn
{
	Picture *src;
	gradient2D *gradient;
	ShiftLabels labels;
	imageSize target_size;
	float alpha;
	float beta;
};

struct ForSmoothFn
{
	Picture *src;
	gradient2D *gradient;
	ShiftLabels labels;
	imageSize target_size;
	float alpha;
	float beta;
};

ShiftLabels GenerateLabels(int origin_width, int target_width)
{
	ShiftLabels result;

	//int num_labels = (origin_width-target_width)+1+origin_width-49+1;
	int num_labels = (origin_width-target_width)+1+1;
	result.num_labels = num_labels;
	result.labels = new double[num_labels];
	result.labels[0] = 0;
	for (int i = 1; i <= origin_width-target_width; i++)
	{
		result.labels[i] = i;
	}
	// label for warping
	/*
	for (int i = 0; i <= origin_width-50; i++)
	{
		result.labels[origin_width-target_width+1+i] = i;
	}
	*/

	// label for scaling
	result.labels[origin_width-target_width+1] = ((double)(origin_width-target_width))/((double)(target_width-1));
	//result.labels[2*origin_width-target_width-48] = ((double)(origin_width-target_width))/((double)(target_width-1));

	return result;

	/*
	ShiftLabels result;
	double quant = SHIFT_QUANTIZATION;
	int num_labels = (origin_width-target_width)/quant;
	result.num_labels = num_labels+1;
	result.labels = new double[num_labels+1];
	result.labels[0] = 0;
	for (int i = 1; i <= num_labels; i++)
	{
		result.labels[i] = i*quant;
	}

	return result;
	*/

	/*
	double quant = ((double)(origin_width-target_width))/((double)(target_width-1));
	ShiftLabels result;
	int num_labels = ceil((origin_width-target_width)/quant);
	
	int *flag = new int[origin_width-target_width+1];
	//flag[0] = 1;
	for (int i = 0; i < (origin_width-target_width); i++)
	{
		if ((i/quant)!=floor(i/quant))
		{
			num_labels++;
			flag[i] = 1;
		}
		else
			flag[i] = 0;

	}
	result.num_labels = num_labels+1;
	result.labels = new double[num_labels];
	//result.labels[0] = 0;
	int cur_idx = 0;
	int cur_int = 0;
	for (int i = 0; i <= ceil((origin_width-target_width)/quant); i++)
	{
		if (i*quant>=cur_int)
		{
			if (flag[cur_int]>0)
			{
				result.labels[cur_idx] = cur_int;
				cur_idx++;
			}
			cur_int++;
		}
		result.labels[cur_idx] = i*quant;
		cur_idx++;
	}

	delete [] flag;
	return result;
	*/
}

double dataFn(int p, int l, void *data)
{
	ForDataFn *myData = (ForDataFn *) data;

	int x = p % myData->target_size.width;
	int y = p / myData->target_size.width;

	double cost = 0.0;

	double shift = 0.0;
	if (l<=myData->src->GetWidth()-myData->target_size.width)
		shift = myData->labels.labels[l];
	//if (l>myData->src->GetWidth()-myData->target_size.width && l<myData->labels.num_labels-1)
	//	shift = min((x-myData->labels.labels[l]+1)*(1.0/50.0),1.0);
	if (l==myData->labels.num_labels-1)
		shift = x*myData->labels.labels[l];

	if (l>myData->src->GetWidth()-myData->target_size.width && l<myData->labels.num_labels-1
		&& myData->labels.labels[l]>x)
		return 100000*MAX_COST_VALUE;

	pixelType pix = Interpolate_2D(myData->src,x+shift,y);

	// penalize first column and last column
	/*
	if (x==0 && myData->labels.labels[l]!=0)
		cost = 100000*MAX_COST_VALUE;
	if (x==myData->target_size.width-1)
	{
		if (shift != myData->src->GetWidth()-myData->target_size.width)
		//if (shift != myData->src->GetWidth()-myData->target_size.width &&
		//	myData->labels.labels[l] != myData->src->GetWidth()-myData->target_size.width)
			cost += 100000*MAX_COST_VALUE;
	}
	*/

	if (x==0)
	{
		int ubound = (floor(x+shift)==x+shift) ? 
			floor(x+shift)-1 : floor(x+shift);
		for (int sId = 0; sId <= ubound; sId++)
		{
			cost += 1000000.0*Color_Diff(pix,myData->src->GetPixel(sId,y),0.0);
		}
	}

	if (x==myData->target_size.width-1)
	{
		int lbound = (ceil(x+shift)==x+shift) ? 
			ceil(x+shift)+1 : ceil(x+shift);
		for (int sId = lbound; sId <= myData->src->GetWidth()-1; sId++)
		{
			cost += 1000000.0*Color_Diff(pix,myData->src->GetPixel(sId,y),0.0);
		}
	}

	double ldiff = Color_Diff(pix,myData->src->GetPixel(floor(x+shift),y),0.0);
	double rdiff = Color_Diff(pix,myData->src->GetPixel(ceil(x+shift),y),0.0);
	cost += 1000000.0*min(ldiff,rdiff);//max(ldiff,rdiff);

	// penalize bilinear sampling
	/*
	if (myData->labels.labels[l]-floor(myData->labels.labels[l]) != 0)
	{
		int lx = floor(x+myData->labels.labels[l]);
		int rx = ceil(x+myData->labels.labels[l]);
		double sLDiff = Color_Diff(pix,myData->src->GetPixel(lx,y));
		double sRDiff = Color_Diff(pix,myData->src->GetPixel(rx,y));
		cost += 0.00001*min(sLDiff,sRDi	ff);
	}
	*/
	
	/*
	double ref_shift = ((double)(myData->src->GetWidth()-myData->target_size.width)/
					   (double)(myData->target_size.width-1))*x;
	cost += 1000*pow(myData->labels.labels[l]-ref_shift,2.0);
	*/

	return cost;
}

double ColorDiff(Picture *src, int x1, int y1, 
				 int x2, int y2, int l1, int l2)
{
	double diff = 0.0;
	int width = src->GetWidth();
	int height = src->GetHeight(); 
	int x_offset = x2-x1;
	int y_offset = y2-y1;

	//printf("ColorDiff: pixels: (%d,%d) and (%d,%d) labels: %d and %d\n",x2,y2+l2,x1+x_offset,y1+l1+y_offset,l2,l1);
	if (x2+l2>=0 && x2+l2<width && x1+l1+x_offset>=0 && x1+l1+x_offset<width)
	{
		//printf("(%d,%d) color components: %d,%d,%d\n",src->GetPixel(y2+l2,x2).r,src->GetPixel(y2+l2,x2).g,src->GetPixel(y2+l2,x2).b);
		diff += pow((double)(src->GetPixel(x2+l2,y2).r-src->GetPixel(x1+l1+x_offset,y1+y_offset).r),2.0);
		diff += pow((double)(src->GetPixel(x2+l2,y2).g-src->GetPixel(x1+l1+x_offset,y1+y_offset).g),2.0);
		diff += pow((double)(src->GetPixel(x2+l2,y2).b-src->GetPixel(x1+l1+x_offset,y1+y_offset).b),2.0);
	} else
	{
		diff += MAX_COST_VALUE;
	}
	//printf("ColorDiff: finished\n");

	//printf("ColorDiff: cost between (%d,%d) and (%d,%d) with labels %d and %d is %d\n",x1,y1,x2,y2,l1,l2,diff);
	return diff;
}

double GradientDiff(gradient2D *gradient, int x1, int y1, 
					int x2, int y2, int l1, int l2)
{
	double diff = 0.0;
	int width = gradient->dx->NumOfCols();
	int height = gradient->dx->NumOfRows();
	int x_offset = x2-x1;
	int y_offset = y2-y1;

	if (x2+l2>0 && x2+l2<=width && x1+l1+x_offset>0 && x1+l1+x_offset<=width)
	{
		diff += pow((gradient->dx->Get(y2,x2+l2)-gradient->dx->Get(y1+y_offset,x1+l1+x_offset)),2)+
				pow((gradient->dy->Get(y2,x2+l2)-gradient->dy->Get(y1+y_offset,x1+l1+x_offset)),2);
	} else
	{	
		diff += MAX_COST_VALUE;
	}

	return diff;
}

double smoothFn(int p1, int p2, int l1, int l2, void *data)
{
	ForSmoothFn *myData = (ForSmoothFn *) data;

	//if (l1==l2 && l1<=myData->src->GetWidth()-myData->target_size.width)
	//if (l1==l2 && (l1<=myData->src->GetWidth()-myData->target_size.width || l1==myData->labels.num_labels-1))
	if (l1==l2)
		return 0.0;

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

	double shift1 = 0.0;
	if (l1<=myData->src->GetWidth()-myData->target_size.width)
		shift1 = myData->labels.labels[l1];
	//if (l1>myData->src->GetWidth()-myData->target_size.width && l1<myData->labels.num_labels-1)
	//	shift1 = min((x1-myData->labels.labels[l1]+1)*(1.0/50.0),1.0);
	if (l1==myData->labels.num_labels-1)
		shift1 = x1*myData->labels.labels[l1];
	double shift2 = 0.0;
	if (l2<=myData->src->GetWidth()-myData->target_size.width)
		shift2 = myData->labels.labels[l2];
	//if (l2>myData->src->GetWidth()-myData->target_size.width && l2<myData->labels.num_labels-1)
	//	shift2 = min((x2-myData->labels.labels[l2]+1)*(1.0/50.0),1.0);
	if (l2==myData->labels.num_labels-1)
		shift2 = x2*myData->labels.labels[l2];

	/*
	if (x1!=0 && x2!=0 && shift1==0 && shift2!=0)
		return 100000*MAX_COST_VALUE;
	if (x1!=0 && x2!=0 && shift2==0 && shift1!=0)
		return 100000*MAX_COST_VALUE;

	if (shift1==x1*myData->labels.labels[l1] && shift2!=x2*myData->labels.labels[l2])
		return 100000*MAX_COST_VALUE;
	if (shift1!=x1*myData->labels.labels[l1] && shift2==x2*myData->labels.labels[l2])
		return 100000*MAX_COST_VALUE;
	*/

	if (l1>myData->src->GetWidth()-myData->target_size.width && l1<myData->labels.num_labels-1 && 
		myData->labels.labels[l1]>x1)
		return 100000*MAX_COST_VALUE;
	if (l2>myData->src->GetWidth()-myData->target_size.width && l2<myData->labels.num_labels-1 && 
		myData->labels.labels[l2]>x2)
		return 100000*MAX_COST_VALUE;

	if (y1==y2 && x1!=0 && l1==0 && l2==myData->labels.num_labels-1)
		return 100000*MAX_COST_VALUE;
	if (y1==y2 && x2!=0 && l2==0 && l1==myData->labels.num_labels-1)
		return 100000*MAX_COST_VALUE;
	if (y1==y2 && l1==myData->labels.num_labels-1 && l2!=0 && l2!=myData->labels.num_labels-1)
		return 100000*MAX_COST_VALUE;
	if (y1==y2 && l2==myData->labels.num_labels-1 && l1!=0 && l1!=myData->labels.num_labels-1)
		return 100000*MAX_COST_VALUE;
	if (y1==y2 && l1>myData->src->GetWidth()-myData->target_size.width && 
		l1<myData->labels.num_labels-1 && l2!=0 && l2!=l1)
		return 100000*MAX_COST_VALUE;
	if (y1==y2 && l2>myData->src->GetWidth()-myData->target_size.width && 
		l2<myData->labels.num_labels-1 && l1!=0 && l1!=l2)
		return 100000*MAX_COST_VALUE;
	if (y1==y2 && l1==0 && l2>myData->src->GetWidth()-myData->target_size.width && l2<myData->labels.num_labels-1 &&
		myData->labels.labels[l2]!=x2)
		return 100000*MAX_COST_VALUE;
	if (y1==y2 && l2==0 && l1>myData->src->GetWidth()-myData->target_size.width && l1<myData->labels.num_labels-1 &&
		myData->labels.labels[l1]!=x1)
		return 100000*MAX_COST_VALUE;
	if (y1==y2 && ((x1<x2 && shift1>shift2) || (x1>x2 && shift1<shift2)))
		return 100000*MAX_COST_VALUE;

	pixelType pix1 = Interpolate_2D(myData->src,x1+shift1,y1);
	pixelType pix2 = Interpolate_2D(myData->src,x2+shift2,y2);

	double ref_shift = ((double)(myData->src->GetWidth()-myData->target_size.width))/
						((double)(myData->target_size.width-1));
	double shift_grad = abs(shift1-shift2);

	// horizontal neighbors
	if (y1==y2)
	{
		double sLColDiff = 0.0;
		double sRColDiff = 0.0;
		double sLGradDiff = 0.0;
		double sRGradDiff = 0.0;

		if (x1+shift1<x2+shift2)
		{			
			int lbound = (ceil(x1+shift1)==x1+shift1) ? 
				ceil(x1+shift1)+1 : ceil(x1+shift1);
			int ubound = (floor(x2+shift2)==x2+shift2) ?
				floor(x2+shift2)-1 : floor(x2+shift2);
			for (int sId = lbound; sId <= ubound; sId++)
			{
				sLColDiff += 1.0*Color_Diff(pix1,myData->src->GetPixel(sId,y1),0.0);
			}
			//sLDiff += Color_Diff(pix2,myData->src->GetPixel(floor(x2+myData->labels.labels[l2]),y2));

			for (int sId = ubound; sId >= lbound; sId--)
			{
				sRColDiff += 1.0*Color_Diff(pix2,myData->src->GetPixel(sId,y2),0.0);
			}
			//sRDiff += Color_Diff(pix1,myData->src->GetPixel(ceil(x1+myData->labels.labels[l1]),y1));
		
			cost += 0.0*min(sLColDiff,sRColDiff);

			/*
			double lnaturality = 1.0*Color_Diff(myData->src->GetPixel(floor(x1+shift1)+x2-x1,y1),
											 myData->src->GetPixel(floor(x2+shift2),y2));
			double rnaturality = 1.0*Color_Diff(myData->src->GetPixel(ceil(x2+shift2)+x1-x2,y2),
											myData->src->GetPixel(ceil(x1+shift1),y1));
			if (shift2==1.0/3.0 || shift2==2*(1.0/3.0) || shift2==1.0)
			//if ((shift1==0.0 && shift2==1.0/3.0) || (shift1==2.0/3.0 && shift2==1.0))
				cost += 1.0/3.0*(lnaturality+rnaturality);//max(lnaturality,rnaturality);
			if (l1!=myData->labels.num_labels-1 && l2==myData->labels.num_labels-1)
				cost += 1.0/((double)(myData->target_size.width-1))*(lnaturality+rnaturality);//min(lnaturality,rnaturality);
			if (l1==0 && l2<=myData->src->GetWidth()-myData->target_size.width && l1!=l2)
				cost += 1.0*(lnaturality+rnaturality);//min(lnaturality,rnaturality);
			*/

			/*
			if (shift1==0.0 && shift2==1.0/50.0)
			{
				cost += 1.0*Color_Diff(myData->src->GetPixel(x2,y2),pix2,50.0);
			}
			
			if (shift1==1.0/3.0 && shift2==2.0/3.0)
			{
				sLDiff = 1.0*(Color_Diff(myData->src->GetPixel(x1,y1),pix1)+Color_Diff(myData->src->GetPixel(x2,y2),pix2));
				sRDiff = 1.0*(Color_Diff(myData->src->GetPixel(x1+1,y1),pix1)+Color_Diff(myData->src->GetPixel(x2+1,y2),pix2));
				cost += 1.0*min(sLDiff,sRDiff);
			}
			
			if (shift1==1.0-1.0/50.0 && shift2==1.0)
			{
				cost += 1.0*Color_Diff(myData->src->GetPixel(x2,y1),pix1,50.0);
			}
			*/
			if (shift1==0.0 && shift2==1.0)
			{
				sLColDiff = Color_Diff(myData->src->GetPixel(x1+shift1,y1),myData->src->GetPixel(x2+shift2+x1-x2,y1),1000.0);
				sRColDiff = Color_Diff(myData->src->GetPixel(x2+shift2,y2),myData->src->GetPixel(x1+shift1+x2-x1,y2),1000.0);
				sLGradDiff = Gradient_Diff_2D(myData->gradient,x1+shift1,y1,x2+shift2+x1-x2,y1);
				sRGradDiff = Gradient_Diff_2D(myData->gradient,x2+shift2,y2,x1+shift1+x2-x1,y2);

				cost += 1.0*min(sLColDiff,sRColDiff);//(sLColDiff+sRColDiff)

				sLColDiff = 0.0;
				sRColDiff = 0.0;
				sLGradDiff = 0.0;
				sRGradDiff = 0.0;

				// forward energy
				cost += 0.1*Color_Diff(pix1,pix2,1000.0);

				if (y2>0)
				{
					sLColDiff = Color_Diff(myData->src->GetPixel(x2,y2),myData->src->GetPixel(x2,y2-1),0.0);
					if (x2>0)
						sLColDiff += Color_Diff(myData->src->GetPixel(x2,y2),myData->src->GetPixel(x2-1,y2-1),0.0);
					if (x2<myData->src->GetWidth())
						sLColDiff += Color_Diff(myData->src->GetPixel(x2,y2),myData->src->GetPixel(x2+1,y2-1),0.0);
					sLGradDiff = Gradient_Diff_2D(myData->gradient,x2,y2,x2,y2-1);
				}
				if (y2<myData->src->GetHeight()-1)
				{
					sRColDiff = Color_Diff(myData->src->GetPixel(x2,y2),myData->src->GetPixel(x2,y2+1),0.0);
					if (x2>0)
						sRColDiff += Color_Diff(myData->src->GetPixel(x2,y2),myData->src->GetPixel(x2-1,y2+1),0.0);
					if (x2<myData->src->GetWidth())
						sRColDiff += Color_Diff(myData->src->GetPixel(x2,y2),myData->src->GetPixel(x2+1,y2+1),0.0);
					sRGradDiff = Gradient_Diff_2D(myData->gradient,x2,y2,x2,y2+1);
				}

				cost += 0.0*(sLColDiff+sRColDiff);
			}
		}
		else
		{			
			int lbound = (ceil(x2+shift2)==x2+shift2) ? 
				ceil(x2+shift2)+1 : ceil(x2+shift2);
			int ubound = (floor(x1+shift1)==x1+shift1) ?
				floor(x1+shift1)-1 : floor(x1+shift1);
			for (int sId = lbound; sId <= ubound; sId++)
			{
				sLColDiff += 1.0*Color_Diff(pix2,myData->src->GetPixel(sId,y2),0.0);
			}
			//sLDiff += Color_Diff(pix1,myData->src->GetPixel(floor(x1+myData->labels.labels[l1]),y1));

			for (int sId = ubound; sId >= lbound; sId--)
			{
				sRColDiff += 1.0*Color_Diff(pix1,myData->src->GetPixel(sId,y1),0.0);
			}
			//sRDiff += Color_Diff(pix2,myData->src->GetPixel(ceil(x2+myData->labels.labels[l2]),y2));
		
			cost += 0.0*min(sLColDiff,sRColDiff);

			/*
			double lnaturality = 1.0*Color_Diff(myData->src->GetPixel(floor(x2+shift2)+x1-x2,y2),
				myData->src->GetPixel(ceil(x1+shift1),y1));
			double rnaturality = 1.0*Color_Diff(myData->src->GetPixel(ceil(x1+shift1)+x2-x1,y1),
				myData->src->GetPixel(floor(x2+shift2),y2));

			if (shift1==1.0/3.0 || shift1==2*(1.0/3.0))
			//if ((shift2==0.0 && shift1==1.0/3.0) || (shift2==2.0/3.0 && shift1==1.0))
				cost += 1.0/3.0*(lnaturality+rnaturality);//max(lnaturality,rnaturality);
			if (l2!=myData->labels.num_labels-1 && l1==myData->labels.num_labels-1)
				cost += 1.0/((double)(myData->target_size.width-1))*(lnaturality+rnaturality);//min(lnaturality,rnaturality);
			if (l2==0 && l1<=myData->src->GetWidth()-myData->target_size.width && l2!=l1)
				cost += 1.0*(lnaturality+rnaturality);//min(lnaturality,rnaturality);
			*/

			/*
			if (shift2==0.0 && shift1==1.0/50.0)
			{
				cost += 1.0*Color_Diff(myData->src->GetPixel(x1,y1),pix1,50.0);
			}
			
			if (shift2==1.0/3.0 && shift1==2.0/3.0)
			{
				sLDiff = 1.0*(Color_Diff(myData->src->GetPixel(x2,y2),pix2)+Color_Diff(myData->src->GetPixel(x1,y1),pix1));
				sRDiff = 1.0*(Color_Diff(myData->src->GetPixel(x2+1,y2),pix2)+Color_Diff(myData->src->GetPixel(x1+1,y1),pix1));
				cost += 1.0*min(sLDiff,sRDiff);
			}
			
			if (shift2==1.0-1.0/50.0 && shift1==1.0)
			{
				cost += 1.0*Color_Diff(myData->src->GetPixel(x1,y1),pix2,50.0);
			}
			*/
			if (shift2==0.0 && shift1==1.0)
			{
				sLColDiff = Color_Diff(myData->src->GetPixel(x1+shift1,y1),myData->src->GetPixel(x2+shift2+x1-x2,y1),1000.0);
				sRColDiff = Color_Diff(myData->src->GetPixel(x2+shift2,y2),myData->src->GetPixel(x1+shift1+x2-x1,y2),1000.0);
				sLGradDiff = Gradient_Diff_2D(myData->gradient,x1+shift1,y1,x2+shift2+x1-x2,y1);
				sRGradDiff = Gradient_Diff_2D(myData->gradient,x2+shift2,y2,x1+shift1+x2-x1,y2);
				cost += 1.0*min(sLColDiff,sRColDiff);//(sLColDiff+sRColDiff);

				sLColDiff = 0.0;
				sRColDiff = 0.0;
				sLGradDiff = 0.0;
				sRGradDiff = 0.0;

				// forward energy
				cost += 0.1*Color_Diff(pix1,pix2,1000.0);

				if (y1>0)
				{
					sLColDiff = Color_Diff(myData->src->GetPixel(x1,y1),myData->src->GetPixel(x1,y1-1),0.0);
					if (x1>0)
						sLColDiff += Color_Diff(myData->src->GetPixel(x1,y1),myData->src->GetPixel(x1-1,y1-1),0.0);
					if (x1<myData->src->GetWidth())
						sLColDiff += Color_Diff(myData->src->GetPixel(x1,y1),myData->src->GetPixel(x1+1,y1-1),0.0);
					sLGradDiff = Gradient_Diff_2D(myData->gradient,x1,y1,x1,y1-1);
				}
				if (y1<myData->src->GetHeight()-1)
				{
					sRColDiff = Color_Diff(myData->src->GetPixel(x1,y1),myData->src->GetPixel(x1,y1+1),0.0);
					if (x1>0)
						sRColDiff += Color_Diff(myData->src->GetPixel(x1,y1),myData->src->GetPixel(x1-1,y1+1),0.0);
					if (x1<myData->src->GetWidth())
						sRColDiff += Color_Diff(myData->src->GetPixel(x1,y1),myData->src->GetPixel(x1+1,y1+1),0.0);
					sRGradDiff = Gradient_Diff_2D(myData->gradient,x1,y1,x1,y1+1);
				}
				cost += 0.0*(sLColDiff+sRColDiff);
			}
		}

		/*
		double diff = 1*Color_Diff(myData->src->GetPixel(floor(x1+shift1)+x2-x1,y1,
			myData->src->GetPixel(x2+shift2,y2));
		double diff1 = 1*Color_Diff(pix1,myData->src->GetPixel(floor(x1+shift1),y1));
		diff1 += 1*Color_Diff(pix2,myData->src->GetPixel(floor(x1+shift1)+x2-x1,y1));

		double diff2 = 1*Color_Diff(pix1,myData->src->GetPixel(ceil(x1+shift1),y1));
		diff2 += 1*Color_Diff(pix2,myData->src->GetPixel(ceil(x1+shift1)+x2-x1,y1));

		double diff3 = 1*Color_Diff(pix2,myData->src->GetPixel(floor(x2+shift2),y2));
		diff3 += 1*Color_Diff(pix1,myData->src->GetPixel(floor(x2+shift2)+x1-x2,y2));

		double diff4 = 1*Color_Diff(pix2,myData->src->GetPixel(ceil(x2+shift2),y2));
		diff4 += 1*Color_Diff(pix1,myData->src->GetPixel(ceil(x2+shift2)+x1-x2,y2));

		cost += 1*min(min(min(diff1,diff2),diff3),diff4);
		*/
	}
	else
	{
		double sLDiff = 0.0;
		double sRDiff = 0.0;

		if (x1==x2)
		{
			pixelType pp2 = Interpolate_2D(myData->src,x1+shift1,y2);
			double sBDiff = Color_Diff(pix2,pp2,1000.0);
			pixelType pp1 = Interpolate_2D(myData->src,x2+shift2,y1);
			double sTDiff = Color_Diff(pix1,pp1,1000.0);

			//if (!((l1==myData->labels.num_labels-1) || (l2==myData->labels.num_labels-1)))
			//	cost += 1.0*min(sBDiff,sTDiff);//max(sBDiff,sTDiff);
			//else
				cost += 1.0*min(sBDiff,sTDiff);//(sBDiff+sTDiff);//max(sBDiff,sTDiff);

			// forward energy
			cost += 0.1*Color_Diff(pix1,pix2,1000.0);
		}
		else
		{
			if (shift1==0.0 && shift2==1.0/3.0)
			{
				cost += 1.0*Color_Diff(myData->src->GetPixel(x2,y2),pix2);
			}
			if (shift2==0.0 && shift1==1.0/3.0)
			{
				cost += 1.0*Color_Diff(myData->src->GetPixel(x1,y1),pix1);
			}
			if (shift1==1.0/3.0 && shift2==2.0/3.0)
			{
				sLDiff = 1.0*(Color_Diff(myData->src->GetPixel(x1,y1),pix1)+Color_Diff(myData->src->GetPixel(x2,y2),pix2));
				sRDiff = 1.0*(Color_Diff(myData->src->GetPixel(x1+1,y1),pix1)+Color_Diff(myData->src->GetPixel(x2+1,y2),pix2));
				cost += 1.0*min(sLDiff,sRDiff);
			}
			if (shift2==1.0/3.0 && shift1==2.0/3.0)
			{
				sLDiff = 1.0*(Color_Diff(myData->src->GetPixel(x2,y2),pix2)+Color_Diff(myData->src->GetPixel(x1,y1),pix1));
				sRDiff = 1.0*(Color_Diff(myData->src->GetPixel(x2+1,y2),pix2)+Color_Diff(myData->src->GetPixel(x1+1,y1),pix1));
				cost += 1.0*min(sLDiff,sRDiff);
			}
			if (shift1==2.0/3.0 && shift2==1.0)
			{
				cost += 1.0*Color_Diff(myData->src->GetPixel(x1+1,y1),pix1);
			}
			if (shift2==2.0/3.0 && shift1==1.0)
			{
				cost += 1.0*Color_Diff(myData->src->GetPixel(x2+1,y2),pix2);
			}
			if (shift1==0.0 && shift2==1.0)
			{
				sLDiff = Color_Diff(myData->src->GetPixel(x1,y1),myData->src->GetPixel(x2,y1));
				sRDiff = Color_Diff(myData->src->GetPixel(x2,y2),myData->src->GetPixel(x1,y2));
				cost += 1.0*(sLDiff+sRDiff);
			}
			if (shift2==0.0 && shift1==1.0)
			{
				sLDiff = Color_Diff(myData->src->GetPixel(x2,y2),myData->src->GetPixel(x1,y2));
				sRDiff = Color_Diff(myData->src->GetPixel(x1,y1),myData->src->GetPixel(x2,y1));
				cost += 1.0*(sLDiff+sRDiff);
			}
		}
	}

	//
	/*
	if (y1==y2)
	{
		double ref_shift = ((int)(((double)(myData->src->GetWidth()-myData->target_size.width)/
							(double)(myData->target_size.width-1))/SHIFT_QUANTIZATION)+1)*SHIFT_QUANTIZATION;
		double shift = abs(myData->labels.labels[l1]-myData->labels.labels[l2]);
		if (shift>ref_shift)
			cost += 1000*max(abs(shift-ref_shift)-SHIFT_QUANTIZATION,0);
	}
	*/
	
	//printf("smoothFn: computing cost between (%d,%d) and (%d,%d) with labels %d and %d : %d\n",y1,x1,y2,x2,l1,l2,cost);
	return cost;
}

Picture *SaveRetargetPicture(int *labeling, int *&pre_cords, int *&scaling_num, vector<double> *&removed_cords, 
							 Picture *origin, Picture *src, ShiftLabels &labels_ref, int width, int height, char *name)
{
	Picture *result = new Picture(width,height);
	result->SetName(name);
	int *new_cords = new int[width*height];
	int *new_scaling_num = new int[height];
	for (int i = 0; i<height; i++)
		new_scaling_num[i] = 0;

	Picture *map = new Picture(width,height);
	char map_name[512] = {'\0'};
	strcat(map_name,name);
	strcat(map_name,"shift.ppm");	
	map->SetName(map_name);

	//Picture *tmp = new Picture(origin->GetWidth()-removed_cords[0].size(),height);
	//char map_name2[512] = {'\0'};
	//strcat(map_name2,name);
	//strcat(map_name2,"tmp.ppm");	
	//tmp->SetName(map_name2);

	pixelType pixel;
	for (int y = 0; y < height; y++)
	{
		double shift;
		if (labeling[y*width]==labels_ref.num_labels-1)
		{
			new_scaling_num[y] = scaling_num[y]+1;

			// removed the removed pixels
			int *new_pidx = new int[origin->GetWidth()-
									removed_cords[y].size()];
			int cur_removed_idx = 0;
			int cur_id = 0;
			for (int x = 0; x < origin->GetWidth(); x++)
			{
				if (removed_cords[y].size()<cur_removed_idx+1)
				{
					new_pidx[cur_id] = x;
					//tmp->SetPixel(cur_id,y,origin->GetPixel(x,y));
					cur_id++;					
				}
				else
				{
					if (x<removed_cords[y][cur_removed_idx])
					{
						new_pidx[cur_id] = x;
						//tmp->SetPixel(cur_id,y,origin->GetPixel(x,y));
						cur_id++;
					}
					if (x==removed_cords[y][cur_removed_idx])
						cur_removed_idx++;
				}
			}

			// interpolation
			for (int x = 0; x < width; x++)
			{
				shift = x*((double)(scaling_num[y]+1))/((double)(width-1));
				pixel = Interpolate_2D(origin->GetPixel(new_pidx[(int)(floor(x+shift))],y),
									   origin->GetPixel(new_pidx[(int)(ceil(x+shift))],y), 1-shift+floor(shift));
				result->SetPixel(x,y,pixel);
				new_cords[y*width+x] = ((shift-floor(shift))<0.5) ? new_pidx[(int)(floor(x+shift))] : new_pidx[(int)(ceil(x+shift))];
			}

			delete [] new_pidx;


			/*
			new_scaling_num[y] = scaling_num[y]+1;

			int new_width = width+origin->GetWidth()-width-scaling_num[y]-1;
			int cur_id = 0;
			double min_dist = -1.0;
			for (int x = 0; x < new_width; x++)
			{
				shift = x*((double)(scaling_num[y]+1))/((double)(new_width-1));
				if (removed_cords[y].size()<cur_id+1)
				{
					pixel = Interpolate_2D(origin,x+shift,y);
					result->SetPixel(x-cur_id,y,pixel);
					new_cords[y*width+x-cur_id] = x+shift;					
				}
				else
				{
					if ((x+shift)<removed_cords[y][cur_id])
					{
						if (x-cur_id<width)
						{
							pixel = Interpolate_2D(origin,x+shift,y);
							result->SetPixel(x-cur_id,y,pixel);
							min_dist = abs(x+shift-removed_cords[y][cur_id]);
							new_cords[y*width+x-cur_id] = x+shift;
						}
						else
						{
							cur_id++;
							min_dist = 0.0;
						}
					}
					if ((x+shift)>=removed_cords[y][cur_id])
					{
						if (min_dist<0)
						{	
							cur_id++;
							min_dist = -1.0;
						}
						else
						{
							if (abs(x+shift-removed_cords[y][cur_id])>min_dist)
							{
								// replace previous pixel which is carved						
								cur_id++;
								if (removed_cords[y].size()<cur_id+1)
								{
									pixel = Interpolate_2D(origin,x+shift,y);
									result->SetPixel(x-cur_id,y,pixel);
									min_dist = 0.0;
									new_cords[y*width+x-cur_id] = x+shift;								
								}
								else
								{
									if ((x+shift)<removed_cords[y][cur_id])
									{
										pixel = Interpolate_2D(origin,x+shift,y);
										result->SetPixel(x-cur_id,y,pixel);
										min_dist = 0.0;
									}
									if ((x+shift)>=removed_cords[y][cur_id])
									{
										cur_id++;
										min_dist = -1.0;
									}
								}
							}
							else
							{
								cur_id++;
								min_dist = -1.0;
							}
						}
					}
				}
			}
			*/
		}
			
		if (labeling[y*width]<=1)
		{
			new_scaling_num[y] = scaling_num[y];

			if (labeling[y*width+width-1]==0)
			{
				removed_cords[y].push_back(floor(pre_cords[y*(width+1)+width-1]+0.5));
				sort(removed_cords[y].begin(),removed_cords[y].end());
			}
			for (int x = 0; x < width; x++)
			{
				int i = y*width+x;
				shift = labeling[i];

				if (labeling[i]==1)
				{
					removed_cords[y].push_back(floor(pre_cords[y*(width+1)+x]+0.5));
					sort(removed_cords[y].begin(),removed_cords[y].end());
					break;
				}
			}

			for (int x = 0; x < width; x++)
			{
				int i = y*width+x;
				pixel = Interpolate_2D(src,x+labeling[i],y);
				result->SetPixel(x,y,pixel);
				new_cords[y*width+x] = pre_cords[(int)(y*(width+1)+x+labeling[i])];
			}
		}
	}

	
	//
	int xx, yy;	
	intensityType map_pix;
	for ( int  i = 0; i < width*height; i++ )
	{
		xx = i % width;
		yy = i / width;

		double shift = 0.0;
		if (labeling[i]<=1)
			shift = labels_ref.labels[labeling[i]];
		//if (l>myData->src->GetWidth()-myData->target_size.width && l<myData->labels.num_labels-1)
		//	shift = min((x-myData->labels.labels[l]+1)*(1.0/50.0),1.0);
		if (labeling[i]==labels_ref.num_labels-1)
			shift = xx*labels_ref.labels[labeling[i]];

		map_pix.r = shift*((double)(255.0))/((double)(1.0));
		map_pix.g = shift*((double)(255.0))/((double)(1.0));
		map_pix.b = shift*((double)(255.0))/((double)(1.0));
		map->SetPixelIntensity(xx,yy,map_pix);
	}

	/*	
	int x, y;
	pixelType pixel;
	intensityType map_pix;
	for ( int  i = 0; i < width*height; i++ )
	{
		x = i % width;
		y = i / width;
		//printf("SaveRetargetPicture: GetPixel(%d,%d)\n",x+gc->whatLabel(i),y);

		double shift = 0.0;
		if (labeling[i]<=1)
		{
			shift = labeling[i];
			if (update_scaling_num[y]<0)
				update_scaling_num[y] = scaling_num[y];
		}
		if (labeling[i]==labels_ref.num_labels-1)
		{
			shift = x*((double)(scaling_num[y]+1))/((double)(width+(origin->GetWidth()-width-scaling_num[y])-1));
			if (update_scaling_num[y]<0)
				update_scaling_num[y] = scaling_num[y]+1;
		}

		//pixel = Interpolate_2D(src,x+shift[i]+pre_shift[(int)(y*(width+1)+x+floor(shift[i]))],y);
		if (lneighbors==NULL)
		{
			pixel = Interpolate_2D(origin,x+shift,y);
			update_lneighbors[i] = floor(x+shift);
		}
		else
		{
			if (labeling[i]>1)
			{
				double shift2;
				if (labeling[i]<=1)
					shift2 = labeling[i];
				else
					shift2 = x*1.0/((double)(width-1));

				if (shift2<1.0)
				{
					pixel = Interpolate_2D(origin,lneighbors[(int)(floor(y*(width+1)+x+shift2))]+shift,y);
					if (lneighbors[(int)(floor(y*(width+1)+x+shift2))]+shift!=x+x*((double)(origin->GetWidth()-width))/((double)(width-1)))
						printf("error!");
				}
				if (shift2==1.0)
				{
					pixel = Interpolate_2D(origin,lneighbors[(int)(floor(y*(width+1)+x+shift2))],y);
					if (lneighbors[(int)(floor(y*(width+1)+x+shift2))]!=x+x*((double)(origin->GetWidth()-width))/((double)(width-1)))
						printf("error!");
				}
				update_lneighbors[i] = lneighbors[(int)(floor(y*(width+1)+x+shift2))];
			}
			if (labeling[i]<=1)
			{
				pixel = Interpolate_2D(src,x+labeling[i],y);
				update_lneighbors[i] = lneighbors[(int)(y*(width+1)+x+labeling[i])];
			}
		}

		result->SetPixel(x,y,pixel);

		//map_pix.r = shift[i]*((double)(255.0))/((double)(src->GetWidth()-width));
		//map_pix.g = shift[i]*((double)(255.0))/((double)(src->GetWidth()-width));
		//map_pix.b = shift[i]*((double)(255.0))/((double)(src->GetWidth()-width));

		map_pix.r = shift*((double)(255.0))/((double)(1.0));
		map_pix.g = shift*((double)(255.0))/((double)(1.0));
		map_pix.b = shift*((double)(255.0))/((double)(1.0));
		map->SetPixelIntensity(x,y,map_pix);

		//shift[i] = shift[i]+pre_shift[(int)(y*(width+1)+x+floor(shift[i]))];
	}
	*/

	result->Save(result->GetName());
	map->Save(map->GetName());
	//tmp->Save(tmp->GetName());

	delete [] pre_cords;
	pre_cords = new_cords;
	delete [] scaling_num;
	scaling_num = new_scaling_num;
	delete map;
	return result;
}

////////////////////////////////////////////////////////////////////////////////
// in this version, set data and smoothness terms using arrays
// grid neighborhood structure is assumed
//
int *GridGraph_GraphCut(Picture *src, imageSize &target_size, ShiftLabels &labels, 
						   float alpha, float beta, char *target_name)
{
	int num_pixels = target_size.width*target_size.height;
	int *result = new int[num_pixels];   // stores result of optimization

	try{
		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(target_size.width,
																  target_size.height,
																  labels.num_labels);

		// set up the needed data to pass to function for the data costs
		gradient2D *gradient = Gradient(src);

		ForDataFn toDataFn;
		toDataFn.src = src;
		toDataFn.labels = labels;
		toDataFn.target_size = target_size;
		toDataFn.gradient = gradient;
		toDataFn.alpha = alpha;
		toDataFn.beta = beta;
		gc->setDataCost(&dataFn,&toDataFn);

		// smoothness comes from function pointer
		ForSmoothFn toSmoothFn;
		toSmoothFn.src = src;		
		toSmoothFn.gradient = NULL;
		toSmoothFn.labels = labels;
		toSmoothFn.target_size = target_size;
		toSmoothFn.gradient = gradient;
		toSmoothFn.alpha = alpha;
		toSmoothFn.beta = beta;
		gc->setSmoothCost(&smoothFn, &toSmoothFn);

		printf("Before optimization energy is %f\n",gc->compute_energy());
		gc->expansion();// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("After optimization energy is %f\n",gc->compute_energy());

		for ( int  i = 0; i < num_pixels; i++ )
		{
			int x = i % target_size.width;
			int y = i / target_size.width;

			result[i] = gc->whatLabel(i);
		}
		
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

int main(int argc, char **argv)
{
	Picture *origin = NULL;
	Picture *input = NULL;
	Picture *target = NULL;
	int *labeling = NULL;
	int *pre_cords = NULL;
	int *scaling_num = NULL;
	vector<double> *removed_cords = NULL;
	int width, height, target_width;
	int num_pixels, num_labels;
	imageSize target_size;
	ShiftLabels labels;

	if (argc<6)
	{
		cout << "Usage: shift_map_2d <input_img> <alpha> <beta> <new-width> <output_filename>" << endl;
		return 0;
		//default parameters
	}

	// load input image
	origin = new Picture(argv[1]);
	input = origin;
	target_width = input->GetWidth()-1;

	// initialize matrix of previous coordinates
	int x, y;
	pre_cords = new int[origin->GetHeight()*origin->GetWidth()];
	for (int i = 0; i < origin->GetHeight()*origin->GetWidth(); i++)
	{
		x = i % origin->GetWidth();
		y = i / origin->GetWidth();
		pre_cords[i] = (double)x;
	}

	// initialize scaling number for every row
	scaling_num = new int[origin->GetHeight()];
	for (int i = 0; i < origin->GetHeight(); i++)
		scaling_num[i] = 0;

	removed_cords = new vector<double>[origin->GetHeight()];
			
	while (target_width>=atoi(argv[4]))
	{
		//
		width = input->GetWidth();		
		height = input->GetHeight();
		labels = GenerateLabels(width,target_width);

		target_size.width = target_width;
		target_size.height = height;	

		// smoothness and data costs are set up using functions			
		labeling = GridGraph_GraphCut(input,target_size,labels,
									  atof(argv[2]),atof(argv[3]),argv[5]);

		target = SaveRetargetPicture(labeling,pre_cords,scaling_num,removed_cords,origin,input,
									 labels,target_size.width,target_size.height,argv[5]);

		if (target_width+1<origin->GetWidth())
			delete input;
		input = target;
		
		delete [] labeling;


		target_width = input->GetWidth()-1;
	}

	return 1;

}

/////////////////////////////////////////////////////////////////////////////////