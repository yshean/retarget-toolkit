//////////////////////////////////////////////////////////////////////////////
// Main function of shift-map image editing
//
/////////////////////////////////////////////////////////////////////////////
//
//  Optimization problem:
//  is a set of sites (pixels) of width 10 and hight 5. Thus number of pixels is 50
//  grid neighborhood: each pixel has its left, right, up, and bottom pixels as neighbors
//  7 labels
//  Data costs: D(pixel,label) = 0 if pixel < 25 and label = 0
//            : D(pixel,label) = 10 if pixel < 25 and label is not  0
//            : D(pixel,label) = 0 if pixel >= 25 and label = 5
//            : D(pixel,label) = 10 if pixel >= 25 and label is not  5
// Smoothness costs: V(p1,p2,l1,l2) = min( (l1-l2)*(l1-l2) , 4 )
// Below in the main program, we illustrate different ways of setting data and smoothness costs
// that our interface allow and solve this optimizaiton problem

// For most of the examples, we use no spatially varying pixel dependent terms. 
// For some examples, to demonstrate spatially varying terms we use
// V(p1,p2,l1,l2) = w_{p1,p2}*[min((l1-l2)*(l1-l2),4)], with 
// w_{p1,p2} = p1+p2 if |p1-p2| == 1 and w_{p1,p2} = p1*p2 if |p1-p2| is not 1

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

#define MAX_COST_VALUE 100000

struct ForDataFn
{
	int numLab;
	int *data;
};

struct ForSmoothFn
{
	Picture *src;
	Matrix *gradient;
	int target_width;
	int target_height;
	float alpha;
	float beta;
};

int dataFn(int p, int l, void *data)
{
	ForDataFn *myData = (ForDataFn *) data;
	int numLab = myData->numLab;
	
	return( myData->data[p*numLab+l] );
}

int ColorDiff(Picture *src, int x1, int y1, 
				 int x2, int y2, int l1, int l2)
{
	int diff = 0;
	int width = src->GetWidth();
	int height = src->GetHeight(); 
	int x_offset = x2-x1;
	int y_offset = y2-y1;

	//printf("ColorDiff: pixels: (%d,%d) and (%d,%d) labels: %d and %d\n",x2,y2+l2,x1+x_offset,y1+l1+y_offset,l2,l1);
	if (y2+l2>=0 && y2+l2<width && y1+l1+y_offset>=0 && y1+l1+y_offset<width &&
		x1+x_offset>=0 && x1+x_offset<height)
	{
		//printf("(%d,%d) color components: %d,%d,%d\n",src->GetPixel(y2+l2,x2).r,src->GetPixel(y2+l2,x2).g,src->GetPixel(y2+l2,x2).b);
		diff += pow((double)src->GetPixel(y2+l2,x2).r-src->GetPixel(y1+l1+y_offset,x1+x_offset).r,2);
		diff += pow((double)src->GetPixel(y2+l2,x2).g-src->GetPixel(y1+l1+y_offset,x1+x_offset).g,2);
		diff += pow((double)src->GetPixel(y2+l2,x2).b-src->GetPixel(y1+l1+y_offset,x1+x_offset).b,2);
	} else
	{
		diff += MAX_COST_VALUE;
	}
	//printf("ColorDiff: finished\n");

	//printf("ColorDiff: cost between (%d,%d) and (%d,%d) with labels %d and %d is %d\n",x1,y1,x2,y2,l1,l2,diff);
	return diff;
}

int GradientDiff(Matrix *gradient, int x1, int y1, 
					int x2, int y2, int l1, int l2)
{
	int diff = 0;
	int width = gradient->NumOfCols();
	int height = gradient->NumOfRows();
	int x_offset = x2-x1;
	int y_offset = y2-y1;

	if (y2+l2>0 && y2+l2<=width && y1+l1+y_offset>0 && y1+l1+y_offset<=width &&
		x1+x_offset>0 && x1+x_offset<=height)
	{
		diff += pow((gradient->Get(x2,y2+l2)-gradient->Get(x1+x_offset,y1+l1+y_offset)),2);
	} else
	{	
		diff += MAX_COST_VALUE;
	}

	return diff;
}

int smoothFn(int p1, int p2, int l1, int l2, void *data)
{
	int cost = 0;
	//printf("smoothFn: calculating image gradient...\n");
	int width = ((ForSmoothFn*)data)->target_width;
	int height = ((ForSmoothFn*)data)->target_height;
	//printf("smoothFn: image width=%d and height=%d\n",width,height);
	int y1 = p1 % width;
	int x1 = (p1-y1)/width;
	int y2 = p2 % width;
	int x2 = (p2-y2)/width;
	//printf("smoothFn: (%d,%d) and (%d,%d)\n",x1,y1,x2,y2);

	if (abs(x1-x2)<=1 && abs(y1-y2)<=1)
	{
		cost += ((ForSmoothFn*)data)->alpha*ColorDiff(((ForSmoothFn*)data)->src, x1, y1, x2, y2, l1, l2);
		cost += ((ForSmoothFn*)data)->beta*GradientDiff(((ForSmoothFn*)data)->gradient, x1+1, y1+1, x2+1, y2+1, l1, l2);
	}
	else 
	{
		cost = MAX_COST_VALUE;
	}

	//printf("smoothFn: computing cost between (%d,%d) and (%d,%d) with labels %d and %d : %d\n",y1,x1,y2,x2,l1,l2,cost);
	if (cost<0)
		printf("error\n");
	return cost;
}

int CalcPixelCost(Picture *src, int pixel, int label, int width, int height)
{
	int cost = MAX_COST_VALUE;
	int col = pixel % width;
	int row = (pixel-col)/width;

	// pixel rearrangement: 
	// keep the leftmost/rightmost columns
	if (col==0 && label==0)
		cost = 0;
	if (col==width-1 && label==src->GetWidth()-width)
		cost = 0;

	//printf("CalcPixelCost: (%d,%d,%d) cost=%d\n",row,col,label,cost);
	return cost;
}

int *CalcDataCost(Picture *src, int width, int height, int num_labels)
{
	int num_pixels = width*height;
	// first set up the array for data costs
	int *data = new int[num_pixels*num_labels];
	for ( int i = 0; i < num_pixels; i++ )
	{
		for (int l = 0; l < num_labels; l++ )
		{
			data[i*num_labels+l] = CalcPixelCost(src,i,l,width,height);
		}
	}

	return data;
}

void SaveRetargetPicture(GCoptimizationGridGraph *gc, Picture *src,int width, int height, char *name)
{
	Picture *result = new Picture(width,height);
	result->SetName(name);
	
	int x, y;
	pixelType pixel;
	for ( int  i = 0; i < width*height; i++ )
	{
		x = i % width;
		y = i/width;
		pixel.r = src->GetPixel(x+gc->whatLabel(i),y).r;
		pixel.g = src->GetPixel(x+gc->whatLabel(i),y).g;
		pixel.b = src->GetPixel(x+gc->whatLabel(i),y).b;
		result->SetPixel(x,y,pixel);
	}

	result->Save(result->GetName());
}

////////////////////////////////////////////////////////////////////////////////
// in this version, set data and smoothness terms using arrays
// grid neighborhood structure is assumed
//
void GridGraph_GraphCut(Picture *src, int *data, int width,int height,
						int num_labels, float alpha, float beta, char *target_name)
{
	int num_pixels = width*height;
	int *result = new int[num_pixels];   // stores result of optimization

	try{
		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(width,height,num_labels);

		// set up the needed data to pass to function for the data costs
		ForDataFn toDataFn;
		toDataFn.data = data;
		toDataFn.numLab = num_labels;

		gc->setDataCost(&dataFn,&toDataFn);

		// smoothness comes from function pointer
		ForSmoothFn toSmoothFn;
		toSmoothFn.src = src;
		Matrix *gradient = Gradient(src);
		toSmoothFn.gradient = gradient;
		toSmoothFn.target_width = width;
		toSmoothFn.target_height = height;
		toSmoothFn.alpha = alpha;
		toSmoothFn.beta = beta;
		gc->setSmoothCost(&smoothFn, &toSmoothFn);

		printf("Before optimization energy is %d\n",gc->compute_energy());
		gc->expansion(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("After optimization energy is %d\n",gc->compute_energy());

		for ( int  i = 0; i < num_pixels; i++ )
		{
			printf("The optimal label for pixel %d is %d\n",i,gc->whatLabel(i));
			result[i] = gc->whatLabel(i);
		}

		SaveRetargetPicture(gc,src,width,height,target_name);

		delete gradient;
		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	delete [] result;

}

int main(int argc, char **argv)
{
	Picture *input = NULL;
	pyramidType *gpyramid = NULL;
	int width, height;
	int num_pixels, num_labels;
	int *data;

	if (argc<6)
	{
		cout << "Usage: shift_map_2d <input_img> <alpha> <beta> <ratio> <output_filename>" << endl;
		return 0;
		//default parameters
	}

	// load input image
	cout << "Creating gaussian pyramid for input image" << endl;
	input = new Picture(argv[1]);
	gpyramid = GaussianPyramid(input);

	for (int i = gpyramid->Levels-1; i >= 0; i--)
	{
		if (i+1==gpyramid->Levels)
		{
			width = gpyramid->Images[1].GetWidth();
			height = gpyramid->Images[1].GetHeight();
			num_pixels = ceil(width*atof(argv[4]))*height;
			num_labels = width-ceil(width*atof(argv[4]))+1;
			width = ceil(width*atof(argv[4]));
			data = CalcDataCost(&(gpyramid->Images[1]),width,height,num_labels);	

			// smoothness and data costs are set up using functions
			GridGraph_GraphCut(&(gpyramid->Images[1]),data,width,height,
								num_labels,atof(argv[2]),atof(argv[3]),argv[5]);
		} else
		{

		}
	}

	delete data;
	delete input;
	delete gpyramid;

	return 1;

}

/////////////////////////////////////////////////////////////////////////////////