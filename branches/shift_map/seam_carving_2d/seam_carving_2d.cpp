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
	int numLab;
	int *data;
};

struct ForSmoothFn
{
	Picture *src;
	// backward energy
	Matrix *gradient_x;
	Matrix *gradient_y;
	// forward energy
	Matrix *fwdeng_LR;			
	Matrix *fwdeng_pLU;
	Matrix *fwdeng_nLU;
	int method;			//0 backward		1 forward energy
	int target_width;
	int target_height;
};

double dataFn(int p, int l, void *data)
{
	ForDataFn *myData = (ForDataFn *) data;
	int numLab = myData->numLab;
	
//printf("dataFn	p=%d	l=%d	data=%d\n", p, l, myData->data[p*numLab+l]);

	return( myData->data[p*numLab+l] );
}

double smoothFn(int p1, int p2, int l1, int l2, void *data)
{
//printf("p1=%i	p2=%i	l1=%i	l2=%i\n", p1,p2,l1,l2);

	int cost = 0;
	int width = ((ForSmoothFn*)data)->target_width;
	int height = ((ForSmoothFn*)data)->target_height;
	int y1 = p1 % width;			// col  1
	int x1 = (p1-y1)/width;			// row  1
	int y2 = p2 % width;			// col 2
	int x2 = (p2-y2)/width;			// row 2
	int method = ((ForSmoothFn*)data)->method;

	if (method==0)
	{
		Matrix *gradX = ((ForSmoothFn*)data)->gradient_x;
		Matrix *gradY = ((ForSmoothFn*)data)->gradient_y;

		if  ( l1==l2 )
			cost = 0;
		else if ( x1==x2 && l1==1 && l2==0 )
			cost = gradY->Get(x2+1,y2+1) + gradX->Get(x2+1,y2+1);
		else if ( x1==x2 && l1==0 && l2==1 )
			cost = MAX_COST_VALUE;

		else if ( y1>y2 && x1>x2 && l1==1 && l2==0 )
			cost = 0;
		else if ( y1>y2 && x1>x2 && l1==0 && l2==1 )
			cost = MAX_COST_VALUE;

		else if ( y1<y2 && x1>x2 && l1==0 && l2==1 )
			cost = 0;
		else if ( y1<y2 && x1>x2 && l1==1 && l2==0 )
			cost = MAX_COST_VALUE;
	}
	else if (method==1)
	{
		Matrix *fwdeng_LR = ((ForSmoothFn*)data)->fwdeng_LR;
		Matrix *fwdeng_pLU = ((ForSmoothFn*)data)->fwdeng_pLU;
		Matrix *fwdeng_nLU = ((ForSmoothFn*)data)->fwdeng_nLU;

		if  ( l1==l2 )
			cost = 0;
		else if ( x1==x2 && l1==1 && l2==0 )
			cost = fwdeng_LR->Get(x2+1,y2+1);
		else if ( x1==x2 && l1==0 && l2==1 )
			cost = MAX_COST_VALUE;

		else if ( y1>y2 && x1>x2 && l1==1 && l2==0 )
			cost = 0;
		else if ( y1>y2 && x1>x2 && l1==0 && l2==1 )
			cost = MAX_COST_VALUE;
		
		else if ( y1<y2 && x1>x2 && l1==0 && l2==1 )
			cost = 0;
		else if ( y1<y2 && x1>x2 && l1==1 && l2==0 )
			cost = MAX_COST_VALUE;

		else if ( y1==y2 && l1==0 && l2==1 )
			cost = fwdeng_pLU->Get(x2+1,y2+1);
		else if ( y1==y2 && l1==1 && l2==0 )
			cost = fwdeng_nLU->Get(x2+1,y2+1);
	}
	
//printf("smoothFn: computing cost between (%d,%d) and (%d,%d) with labels %d and %d : %d\n",y1,x1,y2,x2,l1,l2,cost);

	return cost;
}

int CalcPixelCost(Picture *src, int pixel, int label, 
				  int width, int height)
{
	int cost = MAX_COST_VALUE;
	int col = pixel % width;
	int row = (pixel-col)/width;

	// leftmost column to be the source by labelling it to 0
	// rightmost column to be the sink by labelling it to 1
	// preserves the leftmost/rightmost columns by setting cost to 0
	if (col==0 && label==0)								
		cost = 0;							// setting cost=0 preserves that pixel
	else if (col==width-1 && label==1)
		cost = 0;							// setting cost=0 preserves that pixel

	//printf("CalcPixelCost: (%d,%d,%d) cost=%d\n",row,col,label,cost);
	return cost;
}

int *CalcDataCost(Picture *src, int width, int height, int num_labels)
{
	int num_pixels = width*height;
	// first set up the array for data costs
//printf("num pixels = %i\n", num_pixels);

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

Picture *removeSeam(GCoptimization *gc, Picture *src)
{
	int width = src->GetWidth();
	int height = src->GetHeight();

	Picture *result = new Picture(width-1, height);
	
	int x, y;
	pixelType pixel;
	for ( int  i = 0; i < width*height; i++ )
	{
		x = i % width;
		y = i/width;
		pixel.r = src->GetPixel(x,y).r;
		pixel.g = src->GetPixel(x,y).g;
		pixel.b = src->GetPixel(x,y).b;
		if ( i < width*height-1 && 
			((gc->whatLabel(i)!=0 && gc->whatLabel(i+1)!=1) ||
			(gc->whatLabel(i) == gc->whatLabel(i+1))) )
		{
			if (gc->whatLabel(i)==0)
				result->SetPixel(x,y,pixel);
			else
				result->SetPixel(x-1,y,pixel);		//sink pixels shift left by 1
		}
		else if ( i == width*height-1 )
			result->SetPixel(x-1,y,pixel);
	}
	delete src;					// delete becos old picture no longer in use, new picture is 1 seam smaller
	return result;
}

void SaveRetargetPicture(Picture *src, char *name)
{
	src->Save(name);
}

Picture *drawSeam(GCoptimization *gc, Picture *src)
{
	int width = src->GetWidth();
	int height = src->GetHeight();
	Picture *result = new Picture(width,height);
	
	int x, y;
	pixelType pixel;
	for ( int  i = 0; i < width*height; i++ )
	{
		x = i % width;
		y = i/width;
		if ( i < width*height-1 && (gc->whatLabel(i))==0 && gc->whatLabel(i+1)==1 )
		{
			//printf("Found Seam\n");
			pixel.r = 255;
			pixel.g = 0;
			pixel.b = 0;
		}
		else
		{
			pixel.r = src->GetPixel(x,y).r;
			pixel.g = src->GetPixel(x,y).g;
			pixel.b = src->GetPixel(x,y).b;
		
		}
		result->SetPixel(x,y,pixel);
	}
	delete src;
	return result;
}

Picture *Transpose_Picture( Picture *src )
{
	int width = src->GetWidth();
	int height = src->GetHeight();
	Picture *tpe = new Picture(height,width);
	int x, y;
	pixelType pixel;

	for ( int  i = 0; i < width*height; i++ )
	{
		x = i % width;
		y = i/width;
		
		pixel.r = src->GetPixel(x,y).r;
		pixel.g = src->GetPixel(x,y).g;
		pixel.b = src->GetPixel(x,y).b;
		
		tpe->SetPixel(y,x,pixel);
	}
	delete src;
	return tpe;
}

////////////////////////////////////////////////////////////////////////////////
// in this version, set data and smoothness terms using arrays
// seam graph neighborhood structure is assumed
//
Picture *Seam2DGraph_GraphCut(Picture *src, int *data, int num_labels, int method)
{
	int width = src->GetWidth();
	int height = src->GetHeight();
	int num_pixels = width*height;

	GCoptimization *gc;

time_t start;
time_t end;

	try{
		if (method == 1)
			gc = new GCoptimizationFwdEn2DSeamGraph(width, height, num_labels);
		else if (method == 0)
			gc = new GCoptimization2DSeamGraph(width, height, num_labels);
		
		// set up the needed data to pass to function for the data costs
		ForDataFn toDataFn;			
		toDataFn.data = data;		//wj from the data cost calculated earlier from CalcDataCost, assign the data cost to toDataFn.data
		toDataFn.numLab = num_labels;

		gc->setDataCost(&dataFn,&toDataFn);

		// smoothness comes from function pointer
		ForSmoothFn toSmoothFn;
		toSmoothFn.src = src;							// set picture source to SmoothFn.src
		toSmoothFn.method = method;

		if (method==0){

time( &start );
			gradient2D_L1 *gradient_L1 = Gradient2D_L1(src);
time( &end );
printf("Gradient2D_L1 Took: %.4f seconds\n",difftime( end, start ));

			toSmoothFn.gradient_x = gradient_L1->dx;
			toSmoothFn.gradient_y = gradient_L1->dy;

			toSmoothFn.target_width = width;
			toSmoothFn.target_height = height;

			delete gradient_L1;
		}
		else if (method==1){

time( &start );
			gradient2D_FE *gradient_FE = Gradient2D_FE(src);
time( &end );
printf("			Gradient2D_FE Took: %.4f seconds\n",difftime( end, start ));

			toSmoothFn.fwdeng_LR = gradient_FE->LR;
			toSmoothFn.fwdeng_pLU = gradient_FE->pLU;
			toSmoothFn.fwdeng_nLU = gradient_FE->nLU;
			toSmoothFn.target_width = width;
			toSmoothFn.target_height = height;

			delete gradient_FE;
		}

		gc->setSmoothCost(&smoothFn, &toSmoothFn);
time( &start );
	//	printf("Before optimization energy is %d\n",gc->compute_energy());
		gc->expansion(1);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
	//	printf("After optimization energy is %d\n",gc->compute_energy());
time( &end );
printf("						Expansion Took: %.4f seconds\n",difftime(end,start));

		src = removeSeam(gc,src);
		//src = drawSeam(gc,src);
		delete gc;	

	}
	catch (GCException e){
		e.Report();
	}

	return src;
}

int main(int argc, char **argv)
{
	Picture *pic = NULL;
	int width, height;
	int num_pixels, num_labels;
	int *data;

	if (argc<4)
	{
		cout << "Usage: seam_carving_2d <input_img> <num of v seams to remove> <num of h seams to remove> <output_filename> <method>" << endl;
		return 0;
		//default parameters
		/*
		argv[1] = "..\\Images\\sea.ppm";
		argv[2] = "1";			// num of v seams to remove
		argv[3] = "0";			// num of h seams to remove
		argv[4] = "..\\Images\\sea_Out_fe_1seam_new.ppm";
		argv[5] = "1";			//0 backward energy		1 forward energy
		*/
	}

	// load input image
	pic = new Picture(argv[1]);
					
	num_labels = 2;

	for( int s=1; s<=atoi(argv[2]); s++ )
	{
		printf("seam #%d\n",s);
		width = pic->GetWidth();
		height = pic->GetHeight();
		num_pixels = width*height;
		data = CalcDataCost(pic,width,height,num_labels);
		pic = Seam2DGraph_GraphCut(pic,data,num_labels,atoi(argv[5]));
		delete data;
	}

	pic = Transpose_Picture(pic);

	for( int s=1; s<=atoi(argv[3]); s++ )
	{
		printf("seam #%d\n",s);
		width = pic->GetWidth();
		height = pic->GetHeight();
		num_pixels = width*height;
		data = CalcDataCost(pic,width,height,num_labels);
		pic = Seam2DGraph_GraphCut(pic,data,num_labels,atoi(argv[5]));
		delete data;
	}

	pic = Transpose_Picture(pic);
	SaveRetargetPicture(pic,argv[4]);

	delete pic;	

	system("Pause");
	return 1;
	
}

/////////////////////////////////////////////////////////////////////////////////