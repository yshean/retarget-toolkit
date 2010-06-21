//////////////////////////////////////////////////////////////////////////////
// Main function of 3D selection map for video retargeting
//
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "../Common/picture.h"
#include "../Common/utils.h"
#include "../GCoptimization/GCoptimization.h"

/*
 * data structure for data term
 */
struct ForDataFn
{
	PictureList *src;
	Matrix *gradient;
	float weight_0;
	float weight_1;
	videoSize src_size;
};

/*
 * data structure for smoothness term
 */
struct ForSmoothFn
{
	PictureList *src;
	Matrix *gradient;
	videoSize src_size;
	float alpha;
	float beta;
};

/*
 * function for data term calculation
 */
double DataFn(int p, int l, void *data)
{
	ForDataFn *myData = (ForDataFn *) data;
	int width = myData->src_size.width;
	int height = myData->src_size.height;
	int time = myData->src_size.time;

	int	t = p / (width*height);
	int x = (p % (width*height)) % width;
	int y = (p % (width*height)) / width;

	double cost = 0.0;

	// pixel arrangement
	if (x==0 && l==0)
		cost = 100000*MAX_COST_VALUE;
	if (x==myData->src_size.width-1 && l==0)
		cost = 100000*MAX_COST_VALUE;
	if (x>0 && x<myData->src_size.width-1 && l==1)
		cost += myData->weight_1;
	
	return cost;
}

/*
 * Compute color difference between two mapped points
 */
double ColorDiff(PictureList *src, int x1, int y1, int t1, 
				 int x2, int y2, int t2)
{
	double diff = 0.0;	
	int width = src->GetPicture(t1)->GetWidth();
	int height = src->GetPicture(t1)->GetHeight(); 
	int time = src->GetLength();

	if (x2>=0 && x2<width && x1>=0 && x1<width)
	{
		diff += pow((double)src->GetPicture(t2)->GetPixel(x2,y2).r-
					src->GetPicture(t1)->GetPixel(x1,y1).r,2.0);
		diff += pow((double)src->GetPicture(t2)->GetPixel(x2,y2).g-
					src->GetPicture(t1)->GetPixel(x1,y1).g,2.0);
		diff += pow((double)src->GetPicture(t2)->GetPixel(x2,y2).b-
					src->GetPicture(t1)->GetPixel(x1,y1).b,2.0);
	} else
	{
		diff += 10000*MAX_COST_VALUE;
	}	
	//printf("ColorDiff: finished\n");


	return diff;
}

/*
 * Compute gradient difference between two mapped points
 */
double GradientDiff(Matrix *gradient, int x1, int y1, int t1, 
					int x2, int y2, int t2)
{
	double diff = 0.0;
	int width = gradient[0].NumOfCols();
	int height = gradient[0].NumOfRows();

	if (x2>0 && x2<=width && x1>0 && x1<=width)
	{
		diff += pow((gradient[t2].Get(y2,x2)-gradient[t1].Get(y1,x1)),2.0);
	} else
	{	
		diff += 10000*MAX_COST_VALUE;
	}

	return diff;
}

/*
 * function for smoothness term calculation
 */
double smoothFn(int p1, int p2, int l1, int l2, void *data)
{
	ForSmoothFn *myData = (ForSmoothFn *) data;
	double cost = 0.0;
	//printf("smoothFn: calculating image gradient...\n");
	int width = myData->src_size.width;
	int height = myData->src_size.height;
	int time = myData->src_size.time;
	//printf("smoothFn: image width=%d and height=%d\n",width,height);
	int	t1 = p1 / (width*height);
	int x1 = (p1 % (width*height)) % width;
	int y1 = (p1 % (width*height)) / width;

	int	t2 = p2 / (width*height);
	int x2 = (p2 % (width*height)) % width;
	int y2 = (p2 % (width*height)) / width;

	int x_offset = x2-x1;
	int y_offset = y2-y1;
	int t_offset = t2-t1;

	if (y_offset == 0 && t_offset == 0)
	{
		if (l1==0 && l2==0)
			cost += 100000*MAX_COST_VALUE;
		if (l1==1 && l2==1)
			cost += 0;
		if (l1==0 && l2==1)
		{
			if (x_offset<0)
			{
				cost += myData->alpha*ColorDiff(myData->src, x1, y1, t1, x1+1, y1, t1);
				cost += myData->beta*GradientDiff(myData->gradient, x1+1, y1+1, t1, x1+2, y1+1, t1);
			}
			else
			{
				cost += myData->alpha*ColorDiff(myData->src, x1, y1, t1, x1-1, y1, t1);
				cost += myData->beta*GradientDiff(myData->gradient, x1+1, y1+1, t1, x1, y1+1, t1);
			}
		}
		if (l1==1 && l2==0)
		{
			if (x_offset<0)
			{
				cost += myData->alpha*ColorDiff(myData->src, x2, y2, t2, x2-1, y2, t2);
				cost += myData->beta*GradientDiff(myData->gradient, x2+1, y2+1, t2, x2, y2+1, t2); 
			}
			else
			{
				cost += myData->alpha*ColorDiff(myData->src, x2, y2, t2, x2+1, y2, t2);
				cost += myData->beta*GradientDiff(myData->gradient, x2+1, y2+1, t2, x2+2, y2+1, t2);
			}
		}
	}
	if (x_offset == 0 && t_offset == 0)
	{
		if (l1==0 && l2==0)
			cost += 0;
		if (l1==1 && l2==1)
			cost += 0;
	}

	if (x_offset != 0 && y_offset != 0 && t_offset == 0)
	{
		if (l1 == 0 && l2 == 0)
		{
			if ((x_offset<0 && y_offset<0) || (x_offset<0 && y_offset>0))
			{
				cost += myData->alpha*ColorDiff(myData->src, x1-1, y1, t1, x1, y1, t1);
				cost += myData->beta*GradientDiff(myData->gradient, x1, y1+1, t1, x1+1, y1+1, t1);
				cost += myData->alpha*ColorDiff(myData->src, x2, y2, t2, x2+1, y2, t2);
				cost += myData->beta*GradientDiff(myData->gradient, x2+1, y2+1, t2, x2+2, y2+1, 21);

			}
			if ((x_offset>0 && y_offset>0) || (x_offset>0 && y_offset<0))
			{
				cost += myData->alpha*ColorDiff(myData->src, x2-1, y2, t2, x2, y2, t2);
				cost += myData->beta*GradientDiff(myData->gradient, x2, y2+1, t2, x2+1, y2+1, t2);
				cost += myData->alpha*ColorDiff(myData->src, x1, y1, t1, x1+1, y1, t1);
				cost += myData->beta*GradientDiff(myData->gradient, x1+1, y1+1, t1, x1+2, y1+1, t1);
			}
		}
	}

	if (x_offset == 0 && y_offset == 0)
	{
		if (l1==0 && l2==0)
			cost += 0;
		if (l1==1 && l2==1)
			cost += 0;
	}

	if (x_offset != 0 && y_offset == 0 && t_offset != 0)
	{
		if (l1 == 0 && l2 == 0)
		{
			if ((x_offset<0 && t_offset<0) || (x_offset<0 && t_offset>0))
			{
				cost += myData->alpha*ColorDiff(myData->src, x1-1, y1, t1, x1, y1, t1);
				cost += myData->beta*GradientDiff(myData->gradient, x1, y1+1, t1, x1+1, y1+1, t1);
				cost += myData->alpha*ColorDiff(myData->src, x2, y2, t2, x2+1, y2, t2);
				cost += myData->beta*GradientDiff(myData->gradient, x2+1, y2+1, t2, x2+2, y2+1, t2);
			}
			if ((x_offset>0 && t_offset>0) || (x_offset>0 && t_offset<0))
			{
				cost += myData->alpha*ColorDiff(myData->src, x1+1, y1, t1, x1, y1, t1);
				cost += myData->beta*GradientDiff(myData->gradient, x1+2, y1+1, t1, x1+1, y1+1, t1);
				cost += myData->alpha*ColorDiff(myData->src, x2-1, y2, t2, x2, y2, t2);
				cost += myData->beta*GradientDiff(myData->gradient, x2, y2+1, t2, x2+1, y2+1, t2);
			}
		}
	}

	//printf("smoothFn: computing cost between (%d,%d) and (%d,%d) with labels %d and %d : %d\n",y1,x1,y2,x2,l1,l2,cost);
	return cost;
}

/*
 * Function to find the optimal selection map
 */
int *Selection_GraphCut(PictureList *src, videoSize src_size, int num_labels, 
						float weight_0, float weight_1, float alpha, float beta)
{
	// set up the needed data to pass to function for the data costs
	Matrix *gradient = new Matrix[src->GetLength()];
	for (int t = 0; t < src->GetLength(); t++)
	{
		gradient2D* grad = Gradient(src->GetPicture(t));
		gradient[t] = *(grad->dx) + *(grad->dy);
	}

	int num_pixels = src_size.width*src_size.height*src_size.time;
	int *result = new int[num_pixels];   // stores result of optimization

	try{
		GCoptimizationFwdEn3DSeamGraph *gc = new GCoptimizationFwdEn3DSeamGraph(src_size.width,
																				src_size.height,
																				src_size.time,
																				num_labels);


		// TODO: replace the hardcode of gradient threoshold
		// for shot boundary
		// data terms comes from function pointer
		ForDataFn toDataFn;
		toDataFn.src = src;
		toDataFn.gradient = gradient;
		toDataFn.src_size = src_size;
		toDataFn.weight_0 = weight_0;
		toDataFn.weight_1 = weight_1;
		gc->setDataCost(&DataFn,&toDataFn);

		// smoothness comes from function pointer
		ForSmoothFn toSmoothFn;
		toSmoothFn.src = src;		
		toSmoothFn.gradient = gradient;
		toSmoothFn.src_size = src_size;
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

		// upsampling shift-map from low-resolution to high-resolution
		//up_result = UpsamplingShiftMap(result,ratio);	

		delete [] gradient;
		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	return result;

}

/*
 * function to generate and save retargeted output using selection labels
 */
void SaveRetargetOutput(int *labels, PictureList *src, char *name)
{
	int cur_idx = 0;
	for (int t = 0; t < src->GetLength(); t++)
	{
		Picture *map = new Picture(src->GetPicture(t)->GetWidth(),
									  src->GetPicture(t)->GetHeight());
		char map_name[512] = {'\0'};
		strcat(map_name,name);
		strcat(map_name,src->GetPicture(t)->GetName());
		map->SetName(map_name);
	
		int x, y;
		intensityType map_pix;
		int bound = src->GetPicture(t)->GetWidth()*
					src->GetPicture(t)->GetHeight();
		for ( int  i = 0; i < bound; i++ )
		{
			x = i % src->GetPicture(t)->GetWidth();
			y = i / src->GetPicture(t)->GetWidth();
			//printf("SaveRetargetPicture: GetPixel(%d,%d)\n",x+gc->whatLabel(i),y);
			map_pix.r = labels[cur_idx]*255;
			map_pix.g = labels[cur_idx]*255;
			map_pix.b = labels[cur_idx]*255;
			map->SetPixelIntensity(x,y,map_pix);

			cur_idx++;
		}

		map->Save(map->GetName());

		delete map;
	}
}

int main(int argc, char **argv)
{
	PictureList *src = NULL;
	listPyramidType *vpyramid = NULL;
	int num_labels;
	int *map;
	videoSize src_size;

	if (argc<7)
	{
		cout << "Usage: selection_map_3d <src_folder> <0-weight> <1-weight> <alpha> <beta> <output_folder>" << endl;
		return 0;
		//default parameters
		/*
		argv[1] = "..\\VideoInPPM\\";
		argv[2] = "2";					// num of vertical manifold to remove
		argv[3] = "0";					// num of horizontal manifold to remove
		argv[4] = "..\\VideoOutPPM\\";
		argv[5] = "1";			//0 backward energy		1 forward energy
		*/
	}

	// load input video
	cout << "Creating gaussian pyramid for input video" << endl;
	src = new PictureList(argv[1]);
	int level = 3; // gpyramid->Levels-1
	vpyramid = ListPyramid(src,level+1);
	src = &(vpyramid->Lists[level]);

	src_size.width  = src->GetPicture(0)->GetWidth();
	src_size.height = src->GetPicture(0)->GetHeight();
	src_size.time = src->GetLength();	
	num_labels = 2;

	map = Selection_GraphCut(src,src_size,num_labels,
										atof(argv[2]),atof(argv[3]),
										atof(argv[4]),atof(argv[5]));
	
	SaveRetargetOutput(map, src, argv[6]);
	
	delete [] vpyramid->Lists;
	delete vpyramid;
	delete map;
	
	system("Pause");
	return 1;

}

/////////////////////////////////////////////////////////////////////////////////