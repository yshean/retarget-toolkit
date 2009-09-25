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

struct ForDataFn
{
	Picture *src;
	gradient2D *gradient;
	int *assignments;
	imageSize previous_size;
	imageSize target_size;
};

struct ForSmoothFn
{
	Picture *src;
	gradient2D *gradient;
	int *assignments;
	imageSize target_size;
	imageSize previous_size;
	float alpha;
	float beta;
};

double dataFn(int p, int l, void *data)
{
	ForDataFn *myData = (ForDataFn *) data;

	int x = p % myData->target_size.width;
	int y = p / myData->target_size.width;

	int assign_idx = DownsamplingIndex(p,myData->target_size,
										myData->previous_size,2);
	int assignment = ceil((double)myData->assignments[assign_idx]*2);

	double cost = 0.0;
	if (myData->assignments[0]<0)
	{
		// pixel rearrangement: 
		// keep the leftmost/rightmost columns
		if (x==0 && l!=0)
			cost = 10000*MAX_COST_VALUE;
		if (x==myData->target_size.width-1 && 
			l!=myData->src->GetWidth()-myData->target_size.width)
			cost = 10000*MAX_COST_VALUE;
	}
	else
	{
		//printf("Calculating data term for pixel (%d,%d,%d) with label %d\n",x,y,t,l);
		if (x+assignment+l-1<0 || x+assignment+l-1>=myData->src->GetWidth())
		//if (abs(l-assignment)<=1)
			cost = MAX_COST_VALUE;
	}

	
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
		diff += pow((double)src->GetPixel(x2+l2,y2).r-src->GetPixel(x1+l1+x_offset,y1+y_offset).r,2);
		diff += pow((double)src->GetPixel(x2+l2,y2).g-src->GetPixel(x1+l1+x_offset,y1+y_offset).g,2);
		diff += pow((double)src->GetPixel(x2+l2,y2).b-src->GetPixel(x1+l1+x_offset,y1+y_offset).b,2);
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
	double cost = 0.0;
	//printf("smoothFn: calculating image gradient...\n");
	int width = myData->target_size.width;
	int height = myData->target_size.height;
	//printf("smoothFn: image width=%d and height=%d\n",width,height);
	int x1 = p1 % width;
	int y1 = p1 / width;
	int assign_idx = DownsamplingIndex(p1,myData->target_size,myData->previous_size, 2);
	int assignment1 = ceil((double)myData->assignments[assign_idx]*2);

	int x2 = p2 % width;
	int y2 = p2 / width;
	assign_idx = DownsamplingIndex(p2,myData->target_size,myData->previous_size,2);
	int assignment2 = ceil((double)myData->assignments[assign_idx]*2);
	//printf("smoothFn: (%d,%d) and (%d,%d)\n",x1,y1,x2,y2);

	if (abs(x1-x2)<=1 && abs(y1-y2)<=1)
	{	
		/*
		double weight = 0.0;
		for (int i = -3; i <= 3; i++)
		{
			for (int j = -3; j <= 3; j++)
			{
				if (y1+1+j>=1 && y1+1+j<=myData->gradient->dx->NumOfRows() &&
					x1+1+i>=1 && x1+1+i<=myData->gradient->dx->NumOfCols())
				{
					weight += myData->gradient->dx->Get(y1+1+j,x1+1+i);
					weight += myData->gradient->dy->Get(y1+1+j,x1+1+i);
				}
				if (y2+1+j>=1 && y2+1+j<=myData->gradient->dx->NumOfRows() &&
					x2+1+i>=1 && x2+1+i<=myData->gradient->dx->NumOfCols())
				{
					weight += myData->gradient->dx->Get(y2+1+j,x2+1+i);
					weight += myData->gradient->dy->Get(y2+1+j,x2+1+i);
				}
			}
		}
		*/

		if (myData->assignments[0]>=0)
		{
			cost += myData->alpha*ColorDiff(myData->src, x1, y1, x2, y2, 
													assignment1+l1-1, assignment2+l2-1);
			cost += myData->beta*GradientDiff(myData->gradient,x1+1, y1+1, x2+1, y2+1, 
													assignment1+l1-1, assignment2+l2-1);
		}
		else
		{
			cost += myData->alpha*ColorDiff(myData->src, x1, y1, x2, y2, l1, l2);
			cost += myData->beta*GradientDiff(myData->gradient, x1+1, y1+1, x2+1, y2+1, l1, l2);
		}

	}
	else 
	{
		cost = MAX_COST_VALUE;
	}

	//printf("smoothFn: computing cost between (%d,%d) and (%d,%d) with labels %d and %d : %d\n",y1,x1,y2,x2,l1,l2,cost);
	return cost;
}

void SaveRetargetPicture(int *labels, Picture *src,int width, int height, char *name)
{
	Picture *result = new Picture(width,height);
	result->SetName(name);

	Picture *map = new Picture(width,height);
	char map_name[512] = {'\0'};
	strcat(map_name,name);
	strcat(map_name,"shift.ppm");	
	map->SetName(map_name);
	
	int x, y;
	pixelType pixel;
	intensityType map_pix;
	for ( int  i = 0; i < width*height; i++ )
	{
		x = i % width;
		y = i / width;
		//printf("SaveRetargetPicture: GetPixel(%d,%d)\n",x+gc->whatLabel(i),y);
		pixel.r = src->GetPixel(x+labels[i],y).r;
		pixel.g = src->GetPixel(x+labels[i],y).g;
		pixel.b = src->GetPixel(x+labels[i],y).b;
		result->SetPixel(x,y,pixel);

		map_pix.r = labels[i];
		map_pix.g = labels[i];
		map_pix.b = labels[i];
		map->SetPixelIntensity(x,y,map_pix);
	}

	result->Save(result->GetName());
	map->Save(map->GetName());

	delete result;
	delete map;
}

////////////////////////////////////////////////////////////////////////////////
// in this version, set data and smoothness terms using arrays
// grid neighborhood structure is assumed
//
int *GridGraph_GraphCut(Picture *src, int *assignments, imageSize &target_size, imageSize &previous_size,
						int num_labels, float alpha, float beta, char *target_name)
{
	int num_pixels = target_size.width*target_size.height;
	int *result = new int[num_pixels];   // stores result of optimization

	try{
		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(target_size.width,
																  target_size.height,
																  num_labels);

		// set up the needed data to pass to function for the data costs
		gradient2D *gradient = Gradient(src);

		ForDataFn toDataFn;
		toDataFn.src = src;
		toDataFn.assignments = assignments;
		toDataFn.target_size = target_size;
		toDataFn.previous_size = previous_size;
		gc->setDataCost(&dataFn,&toDataFn);

		// smoothness comes from function pointer
		ForSmoothFn toSmoothFn;
		toSmoothFn.src = src;
		
		toSmoothFn.gradient = gradient;
		toSmoothFn.assignments = assignments;
		toSmoothFn.previous_size = previous_size;
		toSmoothFn.target_size = target_size;
		toSmoothFn.alpha = alpha;
		toSmoothFn.beta = beta;
		gc->setSmoothCost(&smoothFn, &toSmoothFn);

		printf("Before optimization energy is %f\n",gc->compute_energy());
		gc->expansion(1);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("After optimization energy is %f\n",gc->compute_energy());

		int assign_idx, assignment;
		for ( int  i = 0; i < num_pixels; i++ )
		{
			//printf("The optimal label for pixel %d is %d\n",i,gc->whatLabel(i));
			assign_idx = DownsamplingIndex(i,target_size,previous_size,2);
			if (assign_idx>=0)
				assignment = ceil((double)assignments[assign_idx]*2);
			else
				assignment = -1;

			if (assignment<0)
				result[i] = gc->whatLabel(i);
			else
				result[i] = max(assignment+gc->whatLabel(i)-1,0);			
		}

		SaveRetargetPicture(result,src,target_size.width,
							target_size.height,target_name);

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
	Picture *input = NULL;
	pyramidType *gpyramid = NULL;
	int width, height;
	int num_pixels, num_labels;
	int *assignments, *new_assignments;
	imageSize target_size, previous_size;

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

	int start_level = 1; // gpyramid->Levels-1
	for (int i = start_level; i >= 0; i--)
	{
		if (i==start_level)
		{
			width = gpyramid->Images[i].GetWidth();
			height = gpyramid->Images[i].GetHeight();
			num_pixels = ceil(width*atof(argv[4]))*height;
			
			assignments = new int[num_pixels];
			for (int j = 0; j < num_pixels; j++)
				assignments[j] = -1;

			num_labels = width-ceil(width*atof(argv[4]))+1;
			width = ceil(width*atof(argv[4]));

			target_size.width = width;
			target_size.height = height;
			previous_size.width = 0;
			previous_size.height = 0;	

			// smoothness and data costs are set up using functions			
			new_assignments = GridGraph_GraphCut(&(gpyramid->Images[i]),assignments,target_size,previous_size,
													num_labels,atof(argv[2]),atof(argv[3]),argv[5]);
			delete [] assignments;
			assignments = new_assignments;
		} else
		{
			width = gpyramid->Images[i].GetWidth();
			height = gpyramid->Images[i].GetHeight();
			num_pixels = ceil(width*atof(argv[4]))*height;

			num_labels = 3;//width-ceil(width*atof(argv[4]))+1;
			width = ceil(width*atof(argv[4]));
			
			previous_size = target_size;
			target_size.width = width;
			target_size.height = height;

			// refinement of initial assignment
			new_assignments = GridGraph_GraphCut(&(gpyramid->Images[i]),assignments,target_size,previous_size,
													num_labels,atof(argv[2]),atof(argv[3]),argv[5]);
			delete [] assignments;
			assignments = new_assignments;
		}
	}

	delete [] assignments;
	delete input;
	delete gpyramid;

	return 1;

}

/////////////////////////////////////////////////////////////////////////////////