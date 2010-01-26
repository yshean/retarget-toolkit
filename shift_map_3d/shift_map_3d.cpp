//////////////////////////////////////////////////////////////////////////////
// Main function of shift-map video editing
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

using namespace std;

struct ForDataFn
{
	Video *src;
	Matrix *contrast;
	gradient3D *gradient;
	int *assignments;
	videoSize previous_size;
	videoSize target_size;
};

struct ForSmoothFn
{
	Video *src;
	gradient3D *gradient;
	int *assignments;
	videoSize previous_size;
	videoSize target_size;
	float alpha;
	float beta;
};

double ColorContrast(Video *src, int x, int y, int t, int l)
{
	int width = src->GetFrame(t)->GetWidth();
	int height = src->GetFrame(t)->GetHeight(); 
	int time = src->GetTime();	

	double contrast = 0.0;
	for (int t_offset=-1; t_offset<=1; t_offset++)
		for (int y_offset=-1; y_offset<=1; y_offset++)
			for (int x_offset=-1; x_offset<=1; x_offset++)
				if (t+l+t_offset>=0 && t+l+t_offset<time &&
					y+y_offset>=0 && y+y_offset<height &&
					x+x_offset>=0 && x+x_offset<width)
				{
					contrast += pow((double)(src->GetFrame(t+l)->GetPixel(x,y).r-
											 src->GetFrame(t+l+t_offset)->GetPixel(x+x_offset,y+y_offset).r),2);
					contrast += pow((double)(src->GetFrame(t+l)->GetPixel(x,y).g-
											 src->GetFrame(t+l+t_offset)->GetPixel(x+x_offset,y+y_offset).g),2);
					contrast += pow((double)(src->GetFrame(t+l)->GetPixel(x,y).b-
											 src->GetFrame(t+l+t_offset)->GetPixel(x+x_offset,y+y_offset).b),2);
				}

	return contrast;
}

double GradientContrast(gradient3D *gradient, int time, int x, int y, int t, int l)
{
	int width = gradient->dx[t].NumOfCols();
	int height = gradient->dx[t].NumOfRows();

	double contrast = 0.0;
	for (int t_offset=-1; t_offset<=1; t_offset++)
		for (int y_offset=-1; y_offset<=1; y_offset++)
			for (int x_offset=-1; x_offset<=1; x_offset++)
				if (t+l+t_offset>=0 && t+l+t_offset<time &&
					y+y_offset>0 && y+y_offset<=height &&
					x+x_offset>0 && x+x_offset<=width)
				{
					contrast += pow((double)(gradient->dx[t+l].Get(y,x)-
											 gradient->dx[t+l+t_offset].Get(y+y_offset,x+x_offset)),2);
					contrast += pow((double)(gradient->dy[t+l].Get(y,x)-
											 gradient->dy[t+l+t_offset].Get(y+y_offset,x+x_offset)),2);
					contrast += pow((double)(gradient->dt[t+l].Get(y,x)-
											 gradient->dt[t+l+t_offset].Get(y+y_offset,x+x_offset)),2);
				}

	return contrast;
}

double dataFn(int p, int l, void *data)
{
	ForDataFn *myData = (ForDataFn *) data;

	int s = p % (myData->target_size.width*myData->target_size.height);
	int t = p / (myData->target_size.width*myData->target_size.height);
	int x = s % myData->target_size.width;
	int y = s / myData->target_size.width;

	int assign_idx = Downsampling3DIndex(p,myData->target_size,
											myData->previous_size,2);
	int assignment = ceil((double)myData->assignments[assign_idx]*2);

	double cost = 0.0;
	if (myData->assignments[0]<0)
	{
		// ----------- pixel rearrangement: -----------
		// keep the leftmost/rightmost columns
		/*
		if (t==0 && l!=0)
			cost = MAX_COST_VALUE;
		if (t==myData->target_size.time-1 && 
			l!=myData->src->GetTime()-myData->target_size.time)
			cost = MAX_COST_VALUE;
		*/
		// --------------------------------------------


		/*
		cost += ColorContrast(myData->src,x,y,t,l);
		cost += GradientContrast(myData->gradient,myData->src->GetTime(),
								 x+1,y+1,t,l);
		*/
		cost = myData->contrast[t].Get(y+1,x+1);
	}
	else
	{
		if (t+assignment+l-1<0 || t+assignment+l-1>=myData->src->GetTime())
		//if (abs(l-assignment)<=1)
			cost = 100000000*MAX_COST_VALUE;
	}
	
	/*if (cost>0)
		printf("Calculating data term for pixel (%d,%d,%d) with label %d: %f\n",x,y,t,l,cost);*/
	//return cost;
	return 100000/(0.01+cost);
}

double ColorDiff(Video *src, int x1, int y1, int t1, 
				 int x2, int y2, int t2, int l1, int l2)
{
	double diff = 0.0;	
	int width = src->GetFrame(t1)->GetWidth();
	int height = src->GetFrame(t1)->GetHeight(); 
	int time = src->GetTime();
	int x_offset = x2-x1;
	int y_offset = y2-y1;
	int t_offset = t2-t1;

	//printf("ColorDiff: pixels: (%d,%d) and (%d,%d) labels: %d and %d\n",x2,y2+l2,x1+x_offset,y1+l1+y_offset,l2,l1);
	if (t2+l2>=0 && t2+l2<time && t1+l1+t_offset>=0 && t1+l1+t_offset<time)
	{
		int nn_t_offset = 0;
		int nn_y_offset = 0;
		int nn_x_offset = 0;
		/*
		for (int nn_t_offset=-1; nn_t_offset<=1; nn_t_offset++)
			for (int nn_y_offset=-1; nn_y_offset<=1; nn_y_offset++)
				for (int nn_x_offset=-1; nn_x_offset<=1; nn_x_offset++)
				{
					if (t2+l2+nn_t_offset>=0 && t2+l2+nn_t_offset<time &&
						y2+nn_y_offset>=0 && y2+nn_y_offset<height &&
						x2+nn_x_offset>=0 && x2+nn_x_offset<width &&
						t1+l1+t_offset+nn_t_offset>=0 && t1+l1+t_offset+nn_t_offset<time)
					{	*/
						//printf("(%d,%d) color components: %d,%d,%d\n",src->GetPixel(y2+l2,x2).r,src->GetPixel(y2+l2,x2).g,src->GetPixel(y2+l2,x2).b);
						diff += pow((double)src->GetFrame(t2+l2+nn_t_offset)->GetPixel(x2+nn_x_offset,y2+nn_y_offset).r-
											src->GetFrame(t1+l1+t_offset+nn_t_offset)->GetPixel(x2+nn_x_offset,y2+nn_y_offset).r,2);
						diff += pow((double)src->GetFrame(t2+l2+nn_t_offset)->GetPixel(x2+nn_x_offset,y2+nn_y_offset).g-
											src->GetFrame(t1+l1+t_offset+nn_t_offset)->GetPixel(x2+nn_x_offset,y2+nn_y_offset).g,2);
						diff += pow((double)src->GetFrame(t2+l2+nn_t_offset)->GetPixel(x2+nn_x_offset,y2+nn_y_offset).b-
											src->GetFrame(t1+l1+t_offset+nn_t_offset)->GetPixel(x2+nn_x_offset,y2+nn_y_offset).b,2);
					//}
				//}
	} else
	{
		diff += 10000*MAX_COST_VALUE;
	}	
	//printf("ColorDiff: finished\n");

	//printf("ColorDiff: cost between (%d,%d) and (%d,%d) with labels %d and %d is %d\n",x1,y1,x2,y2,l1,l2,diff);
	return diff;
}

double GradientDiff(gradient3D *gradient, int time, int x1, int y1, int t1, 
					int x2, int y2, int t2, int l1, int l2)
{
	double diff = 0.0;	
	int width = gradient->dx[t1].NumOfCols();
	int height = gradient->dx[t1].NumOfRows();
	int x_offset = x2-x1;
	int y_offset = y2-y1;
	int t_offset = t2-t1;

	if (t2+l2>=0 && t2+l2<time && t1+l1+t_offset>=0 && t1+l1+t_offset<time)
	{
		int nn_t_offset = 0;
		int nn_y_offset = 0;
		int nn_x_offset = 0;

		/*
		for (int nn_t_offset=-1; nn_t_offset<=1; nn_t_offset++)
			for (int nn_y_offset=-1; nn_y_offset<=1; nn_y_offset++)
				for (int nn_x_offset=-1; nn_x_offset<=1; nn_x_offset++)
				{
					if (t2+l2+nn_t_offset>=0 && t2+l2+nn_t_offset<time &&
						y2+nn_y_offset>0 && y2+nn_y_offset<=height &&
						x2+nn_x_offset>0 && x2+nn_x_offset<=width &&
						t1+l1+t_offset+nn_t_offset>=0 && t1+l1+t_offset+nn_t_offset<time)
					{*/
						diff += pow((gradient->dx[t2+l2+nn_t_offset].Get(y2+nn_y_offset,x2+nn_x_offset)-
									gradient->dx[t1+l1+t_offset+nn_t_offset].Get(y2+nn_y_offset,x2+nn_x_offset)),2);
						diff += pow((gradient->dy[t2+l2+nn_t_offset].Get(y2+nn_y_offset,x2+nn_x_offset)-
									gradient->dy[t1+l1+t_offset+nn_t_offset].Get(y2+nn_y_offset,x2+nn_x_offset)),2);
						diff += pow((gradient->dt[t2+l2+nn_t_offset].Get(y2+nn_y_offset,x2+nn_x_offset)-
									gradient->dt[t1+l1+t_offset+nn_t_offset].Get(y2+nn_y_offset,x2+nn_x_offset)),2);
					//}
				//}
	} else
	{	
		diff += 10000*MAX_COST_VALUE;
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
	int time = myData->target_size.time;
	//printf("smoothFn: image width=%d and height=%d\n",width,height);
	int s = p1 % (width*height);
	int t1 = p1 / (width*height);
	int x1 = s % width;
	int y1 = s / width;

	int assign_idx = Downsampling3DIndex(p1,myData->target_size,myData->previous_size, 2);
	int assignment1 = ceil((double)myData->assignments[assign_idx]*2);

	s = p2 % (width*height);
	int t2 = p2 / (width*height);
	int x2 = s % width;
	int y2 = s / width;

	assign_idx = Downsampling3DIndex(p2,myData->target_size,myData->previous_size,2);
	int assignment2 = ceil((double)myData->assignments[assign_idx]*2);
	//printf("smoothFn: (%d,%d,%d) and (%d,%d,%d)\n",x1,y1,t1,x2,y2,t2);

	if (abs(x1-x2)<=1 && abs(y1-y2)<=1 && abs(t1-t2)<=1)
	{	
		if (myData->assignments[0]>=0)
		{
			cost += myData->alpha*ColorDiff(myData->src, x1, y1, t1, x2, y2, t2, 
											assignment1+l1-1, assignment2+l2-1);
			cost += myData->beta*GradientDiff(myData->gradient, myData->src->GetTime(),
											  x1+1, y1+1, t1, x2+1, y2+1, t2, assignment1+l1-1, assignment2+l2-1);
		}
		else
		{
			cost += ((ForSmoothFn*)data)->alpha*ColorDiff(((ForSmoothFn*)data)->src, 
															x1, y1, t1, x2, y2, t2, l1, l2);

			cost += ((ForSmoothFn*)data)->beta*GradientDiff(((ForSmoothFn*)data)->gradient, 
															((ForSmoothFn*)data)->src->GetTime(),
															x1+1, y1+1, t1, x2+1, y2+1, t2, l1, l2);
		}
	}
	else 
	{
		cost = MAX_COST_VALUE;
	}

	/*if (cost>0)
		printf("smoothFn: computing cost between (%d,%d,%d) and (%d,%d,%d) with labels %d and %d : %d\n",x1,y1,t1,x2,y2,t2,l1,l2,cost);*/
	return cost;
}

void SaveRetargetVideo(int *labels, Video *src,int width, int height, int time, char *name)
{
	Video *result = new Video(width,height,time);
	result->SetName(name);
	Picture *frames = new Picture[time];
	for (int tt = 0; tt< time; tt++)
	{
		frames[tt] = *(new Picture(width,height));
	}
	result->SetFrames(frames,time);
	
	int x, y, t;
	pixelType pixel;
	for ( int  i = 0; i < width*height*time; i++ )
	{
		t = i / (width*height);
		x = (i % (width*height)) % width;
		y = (i % (width*height)) / width;

		//printf("SaveRetargetPicture: GetPixel(%d,%d)\n",x+gc->whatLabel(i),y);
		pixel.r = src->GetPixel(x,y,t+labels[i]).r;
		pixel.g = src->GetPixel(x,y,t+labels[i]).g;
		pixel.b = src->GetPixel(x,y,t+labels[i]).b;
		result->SetPixel(x,y,t,pixel);
	}

	result->Save(result->GetName());

	delete result;
}

////////////////////////////////////////////////////////////////////////////////
// in this version, set data and smoothness terms using arrays
// grid neighborhood structure is assumed
//
int *VideoGridGraph_GraphCut(Video *src, int *assignments, videoSize &target_size, videoSize &previous_size,
								int num_labels, float alpha, float beta, char *target_name)
{
	int num_pixels = target_size.width*
					 target_size.height*
					 target_size.time;
	int *result = new int[num_pixels];   // stores result of optimization

	try{
		GCoptimization3DGridGraph *gc = new GCoptimization3DGridGraph(target_size.width,
																	  target_size.height,
																	  target_size.time,num_labels);

		// set up the needed data to pass to function for the data costs
		gradient3D *gradient = Gradient_3D(src);
		Matrix *contrast = Contrast_3D(src,gradient);

		ForDataFn toDataFn;
		toDataFn.src = src;
		toDataFn.gradient = gradient;
		toDataFn.contrast = contrast;
		toDataFn.assignments = assignments;
		toDataFn.previous_size = previous_size;
		toDataFn.target_size = target_size;
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
				result[i] = min(max(assignment+gc->whatLabel(i)-1,0),src->GetTime()-1);
			}
			else
			{
				result[i] = gc->whatLabel(i);
			}			
		}		

		SaveRetargetVideo(result,src,target_size.width,
						  target_size.height,target_size.time,target_name);
		

		//delete [] gradient;
		delete [] gradient->dx;
		delete [] gradient->dy;
		delete [] gradient->dt;
		delete gradient;
		delete [] contrast;
		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	return result;

}

int main(int argc, char **argv)
{
	Video *input = NULL;
	int width, height, time;
	videoPyramidType *vpyramid = NULL;
	int num_pixels, num_labels;
	int *assignments, *new_assignments;
	videoSize target_size, previous_size;

	if (argc<6)
	{
		cout << "Usage: shift_map_3d <input_folder> <alpha> <beta> <ratio> <output_folder>" << endl;
		return 0;
		//default parameters
	}

	// load input video
	cout << "Creating gaussian pyramid for input video" << endl;
	input = new Video(argv[1]);	
	vpyramid = VideoPyramid(input);
	
	int start_level = 2; // gpyramid->Levels-1
	for (int i = start_level; i >= 0; i--)
	{
		if (i==start_level)
		{
			width = vpyramid->Videos[i].GetFrame(0)->GetWidth();
			height = vpyramid->Videos[i].GetFrame(0)->GetHeight();
			time = vpyramid->Videos[i].GetTime();
			num_pixels = ceil(time*atof(argv[4]))*width*height;
			
			assignments = new int[num_pixels];
			for (int j = 0; j < num_pixels; j++)
				assignments[j] = -1;

			num_labels = time-ceil(time*atof(argv[4]))+1;
			time = ceil(time*atof(argv[4]));

			target_size.width = width;
			target_size.height = height;
			target_size.time = time;
			previous_size.width = 0;
			previous_size.height = 0;
			previous_size.time = 0;

			// smoothness and data costs are set up using functions
			new_assignments = VideoGridGraph_GraphCut(&(vpyramid->Videos[i]),assignments,target_size,previous_size,
													num_labels,atof(argv[2]),atof(argv[3]),argv[5]);
			delete [] assignments;
			assignments = new_assignments;
		} else
		{
			width = vpyramid->Videos[i].GetFrame(0)->GetWidth();
			height = vpyramid->Videos[i].GetFrame(0)->GetHeight();
			time = vpyramid->Videos[i].GetTime();
			num_pixels = ceil(time*atof(argv[4]))*width*height;

			num_labels = 3;//time-ceil(time*atof(argv[4]))+1;
			time = ceil(time*atof(argv[4]));
			
			previous_size = target_size;
			target_size.width = width;
			target_size.height = height;
			target_size.time = time;

			new_assignments = VideoGridGraph_GraphCut(&(vpyramid->Videos[i]),assignments,target_size,previous_size,
														num_labels,atof(argv[2]),atof(argv[3]),argv[5]);
			delete [] assignments;
			assignments = new_assignments;
		}
	}


	delete input;
	delete [] assignments;
	delete vpyramid;

	return 1;

}

/////////////////////////////////////////////////////////////////////////////////