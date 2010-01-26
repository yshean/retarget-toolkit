//////////////////////////////////////////////////////////////////////////////
// Main function of shift-map contextual retargeting
//
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "../Common/picturelist.h"
#include "../Common/utils.h"
#include "../GCoptimization/GCoptimization.h"

using namespace std;

struct ForDataFn
{
	PictureList *src;
	gradient2D *gradient;
	int *assignments;
	int num_labels_x;
	int num_labels_y;
	videoSize previous_size;
	videoSize target_size;
};

struct ForSmoothFn
{
	PictureList *src;
	gradient2D *gradient;
	int *assignments;
	int num_labels_x;
	int num_labels_y;
	videoSize previous_size;
	videoSize target_size;
	float alpha;
	float beta;
};

double dataFn(int p, int l, void *data)
{
	return 0.0;
}

double ColorDiff(PictureList *src, int x1, int y1, int t1, 
				 int x2, int y2, int t2, int l1_x, int l1_y, int l2_x, int l2_y)
{
	double diff = 0.0;	
	int width1 = src->GetPicture(t1)->GetWidth();
	int height1 = src->GetPicture(t1)->GetHeight(); 
	int width2 = src->GetPicture(t2)->GetWidth();
	int height2 = src->GetPicture(t2)->GetHeight(); 
	int time = src->GetLength();
	int x_offset = x2-x1;
	int y_offset = y2-y1;
	int t_offset = t2-t1;

	//printf("ColorDiff: pixels: (%d,%d) and (%d,%d) labels: %d and %d\n",x2,y2+l2,x1+x_offset,y1+l1+y_offset,l2,l1);
	if (x2+l2_x>=0 && x2+l2_x<width2 && x1+l1_x+x_offset>=0 && x1+l1_x+x_offset<width1 &&
		y2+l2_y>=0 && y2+l2_y<height2 && y1+l1_y+y_offset>=0 && y1+l1_y+y_offset<height1)
	{
		/*
		for (int nn_t_offset=-1; nn_t_offset<=1; nn_t_offset++)
			for (int nn_y_offset=-1; nn_y_offset<=1; nn_y_offset++)
				for (int nn_x_offset=-1; nn_x_offset<=1; nn_x_offset++)
				{
					if (x2+l2+nn_x_offset>=0 && x2+l2+nn_x_offset<width &&
						y2+nn_y_offset>=0 && y2+nn_y_offset<height &&
						t2+nn_t_offset>=0 && t2+nn_t_offset<time &&
						x1+l1+x_offset+nn_x_offset>=0 && x1+x1+x_offset+nn_x_offset<width)
					{	*/

						int nn_t_offset = 0;
						int nn_y_offset = 0;
						int nn_x_offset = 0;
						//printf("(%d,%d) color components: %d,%d,%d\n",src->GetPixel(y2+l2,x2).r,src->GetPixel(y2+l2,x2).g,src->GetPixel(y2+l2,x2).b);
						pixelType pixel1 = src->GetPicture(t2+nn_t_offset)->GetPixel(x2+l2_x+nn_x_offset,y2+l2_y+nn_y_offset);
						pixelType pixel2 = src->GetPicture(t1+nn_t_offset)->GetPixel(x1+l1_x+x_offset+nn_x_offset,y1+l1_y+y_offset+nn_y_offset);
						diff += pow((double)pixel1.r-pixel2.r,2);
						diff += pow((double)pixel1.g-pixel2.g,2);
						diff += pow((double)pixel1.b-pixel2.b,2);

					//}
				//}
	} else
	{
		diff += MAX_COST_VALUE;
	}	
	//printf("ColorDiff: finished\n");

	//printf("ColorDiff: cost between (%d,%d) and (%d,%d) with labels %d and %d is %d\n",x1,y1,x2,y2,l1,l2,diff);
	return diff;
}

double GradientDiff(gradient2D *gradient, int time, int x1, int y1, int t1, 
					int x2, int y2, int t2, int l1_x, int l1_y, int l2_x, int l2_y)
{
	double diff = 0.0;	
	/*
	int width1 = gradient->dx[t1].NumOfCols();
	int height1 = gradient->dx[t1].NumOfRows();
	int width2 = gradient->dx[t2].NumOfCols();
	int height2 = gradient->dx[t2].NumOfRows();
	*/

	int width1 = gradient[t1].dx->NumOfCols();
	int height1 = gradient[t1].dx->NumOfRows();
	int width2 = gradient[t2].dx->NumOfCols();
	int height2 = gradient[t2].dx->NumOfRows();
	
	int x_offset = x2-x1;
	int y_offset = y2-y1;
	int t_offset = t2-t1;

	if (x2+l2_x>0 && x2+l2_x<=width2 && x1+l1_x+x_offset>0 && x1+l1_x+x_offset<=width1 &&
		y2+l2_y>0 && y2+l2_y<=height2 && y1+l1_y+y_offset>0 && y1+l1_y+y_offset<=height1)
	{
		/*
		for (int nn_t_offset=-1; nn_t_offset<=1; nn_t_offset++)
			for (int nn_y_offset=-1; nn_y_offset<=1; nn_y_offset++)
				for (int nn_x_offset=-1; nn_x_offset<=1; nn_x_offset++)
				{
					if (x2+l2+nn_x_offset>0 && x2+l2+nn_x_offset<=width &&
						y2+nn_y_offset>0 && y2+nn_y_offset<=height &&
						t2+nn_t_offset>=0 && t2+nn_t_offset<time &&
						x1+l1+x_offset+nn_x_offset>0 && x1+x1+x_offset+nn_x_offset<=width)
					{*/
						int nn_t_offset = 0;
						int nn_y_offset = 0;
						int nn_x_offset = 0;
						diff += pow((gradient[t2+nn_t_offset].dx->Get(y2+l2_y+nn_y_offset,x2+l2_x+nn_x_offset)+
									 gradient[t2+nn_t_offset].dy->Get(y2+l2_y+nn_y_offset,x2+l2_x+nn_x_offset)-
									 gradient[t1+nn_t_offset].dx->Get(y1+l1_y+y_offset+nn_y_offset,x1+l1_x+x_offset+nn_x_offset)-
									 gradient[t1+nn_t_offset].dy->Get(y1+l1_y+y_offset+nn_y_offset,x1+l1_x+x_offset+nn_x_offset)),2);
					//}
				//}
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
	int s = p1 % (width*height);
	int t1 = p1 / (width*height);
	int x1 = s % width;
	int y1 = s / width;

	int assign_idx = Downsampling3DIndex(p1,myData->target_size,myData->previous_size, 2);
	int assignment1 = ceil((double)myData->assignments[assign_idx]*2);

	int l1_x = l1 % myData->num_labels_x;
	int l1_y = l1 / myData->num_labels_x;

	s = p2 % (width*height);
	int t2 = p2 / (width*height);
	int x2 = s % width;
	int y2 = s / width;

	assign_idx = Downsampling3DIndex(p2,myData->target_size,myData->previous_size,2);
	int assignment2 = ceil((double)myData->assignments[assign_idx]*2);
	//printf("smoothFn: (%d,%d,%d) and (%d,%d,%d)\n",x1,y1,t1,x2,y2,t2);

	int l2_x = l2 % myData->num_labels_x;
	int l2_y = l2 / myData->num_labels_x;

	if (abs(x1-x2)<=1 && abs(y1-y2)<=1)
	{	
		if (myData->assignments[0]>=0)
		{
			/*
			cost += myData->alpha*ColorDiff(myData->src, x1, y1, t1, x2, y2, t2, 
											assignment1+l1-1, assignment2+l2-1);
			cost += myData->beta*GradientDiff(myData->gradient, myData->src->GetLength(),
											  x1+1, y1+1, t1, x2+1, y2+1, t2, assignment1+l1-1, assignment2+l2-1);
			*/
		}
		else
		{

			cost += ((ForSmoothFn*)data)->alpha*ColorDiff(((ForSmoothFn*)data)->src, 
															x1, y1, t1, x2, y2, t2, l1_x, l1_y, l2_x, l2_y);				

			cost += ((ForSmoothFn*)data)->beta*GradientDiff(((ForSmoothFn*)data)->gradient, 
															((ForSmoothFn*)data)->src->GetLength(),
															x1+1, y1+1, t1, x2+1, y2+1, t2, l1_x, l1_y, l2_x, l2_y);
		}
	}
	else 
	{
		cost = 10000*MAX_COST_VALUE;
	}

	//if (cost>0)
	//	printf("smoothFn: computing cost between (%d,%d,%d) and (%d,%d,%d) with labels %d and %d : %d\n",x1,y1,t1,x2,y2,t2,l1,l2,cost);
	if (t1==t2)
		return 1000*cost;
	else
		return cost;
}

PictureList *SaveRetargetPicture(int *labels, PictureList *src, int num_labels_x, int width, int height, char *name)
{
	PictureList *result = new PictureList(-1,-1,src->GetLength());
	Picture *img;
	
	int x, y;
	pixelType pixel;
	for (int l = 0; l < src->GetLength(); l++)
	{
		img = new Picture(width,height);
		img->SetName(name);
		for ( int  i = l*width*height; i < (l+1)*width*height; i++ )
		{
			int l_x = labels[i] % num_labels_x;
			int l_y = labels[i] / num_labels_x;
			x = (i-l*width*height) % width;
			y = (i-l*width*height) / width;
			//printf("SaveRetargetPicture: GetPixel(%d,%d)\n",x+gc->whatLabel(i),y);
			pixel.r = src->GetPicture(l)->GetPixel(x+l_x,y+l_y).r;
			pixel.g = src->GetPicture(l)->GetPixel(x+l_x,y+l_y).g;
			pixel.b = src->GetPicture(l)->GetPixel(x+l_x,y+l_y).b;
			img->SetPixel(x,y,pixel);
		}

		char output_name[512] = {'\0'};
		char buf[512] = {'\0'};

		strcat(output_name,img->GetName());
		strcat(output_name,itoa(l,buf,10));
		strcat(output_name,"_retargeted.ppm");
		img->SetName(output_name);
		img->Save(output_name);
		result->SetPicture(l,img);
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////
// in this version, set data and smoothness terms using arrays
// grid neighborhood structure is assumed
//
int *ContextGridGraph_GraphCut(PictureList *src, PictureList *&output, int *assignments, videoSize &target_size, videoSize &previous_size,
							   int num_labels_x, int num_labels_y, float alpha, float beta, char *target_name)
{
	int num_pixels = target_size.width*
					 target_size.height*
					 target_size.time;
	int *result = new int[num_pixels];   // stores result of optimization

	try{
		GCoptimizationMultiGridGraph *gc = new GCoptimizationMultiGridGraph(target_size.width,
																			target_size.height,
																			src->GetLength(),
																			num_labels_x*num_labels_y);

		// set up the needed data to pass to function for the data costs
		gradient2D *gradient = new gradient2D[src->GetLength()];
		for (int i = 0; i < src->GetLength(); i++)
			gradient[i] = *(Gradient(src->GetPicture(i)));

		ForDataFn toDataFn;
		toDataFn.src = src;
		toDataFn.gradient = gradient;
		toDataFn.assignments = assignments;
		toDataFn.num_labels_x = num_labels_x;
		toDataFn.num_labels_y = num_labels_y;
		toDataFn.previous_size = previous_size;
		toDataFn.target_size = target_size;
		gc->setDataCost(&dataFn,&toDataFn);

		// smoothness comes from function pointer
		ForSmoothFn toSmoothFn;
		toSmoothFn.src = src;		
		toSmoothFn.gradient = gradient;
		toSmoothFn.assignments = assignments;
		toSmoothFn.num_labels_x = num_labels_x;
		toSmoothFn.num_labels_y = num_labels_y;
		toSmoothFn.previous_size = previous_size;
		toSmoothFn.target_size = target_size;
		toSmoothFn.alpha = alpha;
		toSmoothFn.beta = beta;
		gc->setSmoothCost(&smoothFn, &toSmoothFn);

		printf("Before optimization energy is %f\n",gc->compute_energy());
		gc->expansion();// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("After optimization energy is %f\n",gc->compute_energy());
		
		int s, x, y, t;
		int assign_idx, assignment;
		for ( int  i = 0; i < num_pixels; i++ )
		{
			//printf("The optimal label for pixel %d is %d\n",i,gc->whatLabel(i));
			assign_idx = Downsampling3DIndex(i,target_size,previous_size,2);
			if (assign_idx>=0)
				assignment = ceil((double)assignments[assign_idx]*2);
			else
				assignment = -1;
			
			if (assignment>=0)
			{
				result[i] = max(assignment+gc->whatLabel(i)-1,0);
			}
			else
			{
				result[i] = gc->whatLabel(i);
			}			
		}		

		output = SaveRetargetPicture(result,src,num_labels_x,target_size.width,
									 target_size.height,target_name);
		

		//delete [] gradient;
		for (int i = 0; i < src->GetLength(); i++)
		{
			delete gradient[i].dx;
			delete gradient[i].dy;
		}
		delete [] gradient;
		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	return result;

}

int main(int argc, char **argv)
{
	PictureList *input = NULL;
	PictureList *retarget = NULL;
	int width, height, time;
	listPyramidType *lpyramid = NULL;
	int num_pixels, num_labels_x, num_labels_y;
	int *assignments, *new_assignments;
	videoSize target_size, previous_size;

	if (argc<7)
	{
		cout << "Usage: contextual_shift_map_2d <input_folder> <alpha> <beta> <y_ratio> <x_ratio> <output_folder>" << endl;
		return 0;
		//default parameters
	}

	// load input video
	cout << "Load Image List ... \n" << endl;
	input = new PictureList(argv[1]);	
	lpyramid = ListPyramid(input,3);
	
	int start_level = 2; // gpyramid->Levels-1
	for (int i = start_level; i >= 2; i--)
	{
		if (i==start_level)
		{
			width = lpyramid->Lists[i].GetMinWidth();
			height = lpyramid->Lists[i].GetMinHeight();
			time = lpyramid->Lists[i].GetLength();
			num_labels_x = (lpyramid->Lists[i].GetMinWidth()-ceil(width*atof(argv[5]))+1);
			num_labels_y = (lpyramid->Lists[i].GetMinHeight()-ceil(height*atof(argv[4]))+1);

			/*
			num_pixels = ceil(width*atof(argv[5]))*ceil(height*atof(argv[4]));			
			assignments = new int[num_pixels];
			for (int j = 0; j < num_pixels; j++)
				assignments[j] = -1;	

			width = ceil(width*atof(argv[5]));
			height = ceil(height*atof(argv[4]));

			target_size.width = width;
			target_size.height = height;
			target_size.time = time;
			previous_size.width = 0;
			previous_size.height = 0;
			previous_size.time = 0;

			// smoothness and data costs are set up using functions
			new_assignments = ContextGridGraph_GraphCut(&(lpyramid->Lists[i]),retarget,assignments,target_size,previous_size,
														num_labels_x,num_labels_y,atof(argv[2]),atof(argv[3]),argv[6]);
			delete [] assignments;
			assignments = new_assignments;
			input = retarget;
			*/

			if (num_labels_x>1)
			{
				num_pixels = ceil(width*atof(argv[5]))*height;//*ceil(height*atof(argv[4]));			
				assignments = new int[num_pixels];
				for (int j = 0; j < num_pixels; j++)
					assignments[j] = -1;	

				width = ceil(width*atof(argv[5]));
				//height = ceil(height*atof(argv[4]));

				target_size.width = width;
				target_size.height = height;
				target_size.time = time;
				previous_size.width = 0;
				previous_size.height = 0;
				previous_size.time = 0;

				// smoothness and data costs are set up using functions
				new_assignments = ContextGridGraph_GraphCut(&(lpyramid->Lists[i]),retarget,assignments,target_size,previous_size,
															num_labels_x,1,atof(argv[2]),atof(argv[3]),argv[6]);
				delete [] assignments;
				assignments = new_assignments;
				input = retarget;
			}
			else
			{
				input = &(lpyramid->Lists[i]);
			}
			if (num_labels_y>1)
			{
				width = input->GetMinWidth();
				height = input->GetMinHeight();
				time = input->GetLength();
				num_pixels = width*ceil(height*atof(argv[4]));			
				assignments = new int[num_pixels];
				for (int j = 0; j < num_pixels; j++)
					assignments[j] = -1;	
				
				height = ceil(height*atof(argv[4]));

				target_size.width = width;
				target_size.height = height;
				target_size.time = time;
				previous_size.width = 0;
				previous_size.height = 0;
				previous_size.time = 0;
				// smoothness and data costs are set up using functions
				new_assignments = ContextGridGraph_GraphCut(input,retarget,assignments,target_size,previous_size,
															1,num_labels_y,atof(argv[2]),atof(argv[3]),argv[6]);
				delete [] assignments;
				assignments = new_assignments;
			}

		} else
		{
			/*
			width = lpyramid->Lists[i].GetMinWidth();
			height = lpyramid->Lists[i].GetMinHeight();
			time = lpyramid->Lists[i].GetLength();
			num_pixels = ceil(width*atof(argv[4]))*ceil(height*atof(argv[4]));

			num_labels_x = 3;
			num_labels_y = 3;
			width = ceil(width*atof(argv[5]));
			height = ceil(height*atof(argv[4]));
			
			previous_size = target_size;
			target_size.width = width;
			target_size.height = height;
			target_size.time = time;

			new_assignments = ContextGridGraph_GraphCut(&(lpyramid->Lists[i]),assignments,target_size,previous_size,
														num_labels_x,num_labels_y,atof(argv[2]),atof(argv[3]),argv[6]);
			delete [] assignments;
			assignments = new_assignments;
			*/
		}
	}


	delete input;
	delete [] assignments;
	delete lpyramid;

	return 1;

}

/////////////////////////////////////////////////////////////////////////////////