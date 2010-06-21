//////////////////////////////////////////////////////////////////////////////
// Main function of 3D seam carving with Forward Energy
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
	int numLab;
	int *data;
};

struct ForSmoothBEFn
{
	Matrix *gradient;
	int original_width;
	int original_height;
	int original_time;
};

struct ForSmoothFEFn
{
	//gradient3D_FE *gradient;
	Matrix *LR;			
	Matrix *pLU;
	Matrix *nLU;
	Matrix *temp_pLU;
	Matrix *temp_nLU;
	int original_width;
	int original_height;
	int original_time;
};

double dataFn(int p, int l, void *data)
{
	ForDataFn *myData = (ForDataFn *) data;
	int numLab = myData->numLab;
	
//printf("dataFn	p=%d	l=%d	data=%d\n", p, l, myData->data[p*numLab+l]);

	return( myData->data[p*numLab+l] );
}

double smoothBEFn(int p1, int p2, int l1, int l2, void *data)
{
//printf("p1=%i	p2=%i	l1=%i	l2=%i\n", p1,p2,l1,l2);

	int cost = 0;
	int width = ((ForSmoothBEFn*)data)->original_width;
	int height = ((ForSmoothBEFn*)data)->original_height;
	int time = ((ForSmoothBEFn*)data)->original_time;
	int pixel_per_pic = width*height;

	int f1 = p1 / pixel_per_pic;
	int y1 = (p1 - f1 * pixel_per_pic) % width;
	int x1 = ((p1 - f1 * pixel_per_pic)-y1)/width;

	int f2 = p2 / pixel_per_pic;
	int y2 = (p2 - f2 * pixel_per_pic) % width;
	int x2 = ((p2 - f2 * pixel_per_pic)-y2)/width;

//if (l1!=l2)
//printf("current (%d,%d,%d) and (%d,%d,%d) with labels %d and %d \n",f1,y1,x1,f2,y2,x2,l1,l2);
	
	Matrix *gradXY = ((ForSmoothBEFn*)data)->gradient;
	//gradient->dx[t1].NumOfCols();
	//gradXY->
	if (l1==l2)		cost=0;
	// same frame
	// horizontal
	else if (f1==f2 && x1==x2 && y1>y2 && l1==1 && l2==0)	cost=gradXY[f1].Get(x2+1,y2+1);		//gradXY->Get(x2+1,y2+1);
	else if (f1==f2 && x1==x2 && y1>y2 && l1==0 && l2==1)	cost=MAX_COST_VALUE;
	// diagonal */*
	else if (f1==f2 && x1>x2 && y1<y2 && l1==0 && l2==1)	cost=0;
	else if (f1==f2 && x1>x2 && y1<y2 && l1==1 && l2==0)	cost=MAX_COST_VALUE;
	// diagonal *\*
	else if (f1==f2 && x1>x2 && y1>y2 && l1==0 && l2==1)	cost=MAX_COST_VALUE;
	else if (f1==f2 && x1>x2 && y1>y2 && l1==1 && l2==0)	cost=0;
	
	// diff frame
	// diagonal *\*
	else if (f1>f2 && x1==x2 && y1<y2 && l1==0 && l2==1)	cost=0;
	else if (f1>f2 && x1==x2 && y1<y2 && l1==1 && l2==0)	cost=MAX_COST_VALUE;
	// diagonal */*
	else if (f1>f2 && x1==x2 && y1>y2 && l1==1 && l2==0)	cost=0;
	else if (f1>f2 && x1==x2 && y1>y2 && l1==0 && l2==1)	cost=MAX_COST_VALUE;

//printf("smoothFn: computing cost between (%d,%d,%d) and (%d,%d,%d) with labels %d and %d : %d\n",f1,y1,x1,f2,y2,x2,l1,l2,cost);
	return cost;
}

double smoothFEFn(int p1, int p2, int l1, int l2, void *data)
{
//printf("p1=%i	p2=%i	l1=%i	l2=%i\n", p1,p2,l1,l2);

	int cost = 0;
	int width = ((ForSmoothFEFn*)data)->original_width;
	int height = ((ForSmoothFEFn*)data)->original_height;
	int time = ((ForSmoothFEFn*)data)->original_time;
	int pixel_per_pic = width*height;

	int f1 = p1 / pixel_per_pic;					// frame
	int y1 = (p1 - f1 * pixel_per_pic) % width;		// col
	int x1 = ((p1 - f1 * pixel_per_pic)-y1)/width;	// row

	int f2 = p2 / pixel_per_pic;						
	int y2 = (p2 - f2 * pixel_per_pic) % width;
	int x2 = ((p2 - f2 * pixel_per_pic)-y2)/width;

	Matrix *LR = ((ForSmoothFEFn*)data)->LR;
	Matrix *pLU = ((ForSmoothFEFn*)data)->pLU;
	Matrix *nLU = ((ForSmoothFEFn*)data)->nLU;
	Matrix *temp_pLU = ((ForSmoothFEFn*)data)->temp_pLU;
	Matrix *temp_nLU = ((ForSmoothFEFn*)data)->temp_nLU;

	if (l1==l2)			cost=0;
	else if (f1==f2 && x1==x2 && y1>y2 && l1==1 && l2==0)	cost=LR[f1].Get(x2+1,y2+1);
	else if (f1==f2 && x1==x2 && y1>y2 && l1==0 && l2==1)	cost=MAX_COST_VALUE;

	else if (f1==f2 && x1>x2 && y1==y2 && l1==0 && l2==1)	cost=pLU[f1].Get(x1+1,y1+1);
	else if (f1==f2 && x1>x2 && y1==y2 && l1==1 && l2==0)	cost=nLU[f1].Get(x2+1,y2+1);

	else if (f1==f2 && x1>x2 && y1>y2 && l1==1 && l2==0)	cost=0;
	else if (f1==f2 && x1>x2 && y1>y2 && l1==0 && l2==1)	cost=MAX_COST_VALUE;

	else if (f1==f2 && x1>x2 && y1<y2 && l1==1 && l2==0)	cost=MAX_COST_VALUE;
	else if (f1==f2 && x1>x2 && y1<y2 && l1==0 && l2==1)	cost=0;

	else if (f1>f2 && x1==x2 && y1==y2 && l1==1 && l2==0)	cost=temp_pLU[x1].Get(f2+1,y1+1);//cost=0;
	else if (f1>f2 && x1==x2 && y1==y2 && l1==0 && l2==1)	cost=temp_nLU[x1].Get(f1+1,y1+1);//cost=0;

	else if (f1>f2 && x1==x2 && y1>y2 && l1==1 && l2==0)	cost=0;
	else if (f1>f2 && x1==x2 && y1>y2 && l1==0 && l2==1)	cost=MAX_COST_VALUE;

	else if (f1>f2 && x1==x2 && y1<y2 && l1==0 && l2==1)	cost=0;
	else if (f1>f2 && x1==x2 && y1<y2 && l1==1 && l2==0)	cost=MAX_COST_VALUE;

	//if (l1==l2)		cost=0;
	//// same frame
	//// horizontal
	//else if (f1==f2 && x1==x2 && y1>y2 && l1==1 && l2==0)	cost=gradXY[f1].LR

	//.Get(x2+1,y2+1);		//gradXY->Get(x2+1,y2+1);
	//else if (f1==f2 && x1==x2 && y1>y2 && l1==0 && l2==1)	cost=MAX_COST_VALUE;
	//// diagonal */*
	//else if (f1==f2 && x1>x2 && y1<y2 && l1==0 && l2==1)	cost=0;
	//else if (f1==f2 && x1>x2 && y1<y2 && l1==1 && l2==0)	cost=MAX_COST_VALUE;
	//// diagonal *\*
	//else if (f1==f2 && x1>x2 && y1>y2 && l1==0 && l2==1)	cost=MAX_COST_VALUE;
	//else if (f1==f2 && x1>x2 && y1>y2 && l1==1 && l2==0)	cost=0;
	//
	//// diff frame
	//// diagonal *\*
	//else if (f1>f2 && x1==x2 && y1<y2 && l1==0 && l2==1)	cost=0;
	//else if (f1>f2 && x1==x2 && y1<y2 && l1==1 && l2==0)	cost=MAX_COST_VALUE;
	//// diagonal */*
	//else if (f1>f2 && x1==x2 && y1>y2 && l1==1 && l2==0)	cost=0;
	//else if (f1>f2 && x1==x2 && y1>y2 && l1==0 && l2==1)	cost=MAX_COST_VALUE;

	//delete [] LR;
	//delete [] pLU;
	//delete [] nLU;
	//delete [] temp_pLU;
	//delete [] temp_nLU;

//printf("smoothFn: computing cost between (%d,%d,%d) and (%d,%d,%d) with labels %d and %d : %d\n",f1,y1,x1,f2,y2,x2,l1,l2,cost);
	return cost;
}

int CalcPixelCost(int pixel, int label, int width, int height, int time)
{
	int cost = MAX_COST_VALUE;
	int pixel_per_pic = width*height;
	int frame = pixel / pixel_per_pic;
	int col = (pixel - frame * pixel_per_pic) % width;
	int row = ((pixel - frame * pixel_per_pic)-col)/width;

	// leftmost column to be the source by labelling it to 0
	// rightmost column to be the sink by labelling it to 1
	// preserves the leftmost/rightmost columns by setting cost to 0
	if (col==0 && label==0)								
		cost = 0;							// setting cost=0 preserves that pixel
	else if (col==width-1 && label==1)
		cost = 0;							// setting cost=0 preserves that pixel

//if (cost==0)
//	printf("CalcPixelCost: (%d,%d,%d,%d) cost=%d\n",row,col,frame,label,cost);

	return cost;
}

int *CalcDataCost(int width, int height, int time, int num_labels)
{
	int num_pixels = width*height*time;
	// first set up the array for data costs
	int *data = new int[num_pixels*num_labels];

	for ( int i = 0; i < num_pixels; i++ )
	{
		for (int l = 0; l < num_labels; l++ )
		{
			data[i*num_labels+l] = CalcPixelCost(i,l,width,height,time);
		}
	}

	return data;
}

PictureList *removeManifold(GCoptimization *gc, PictureList *src)
{
	int width = src->GetPicture(0)->GetWidth();
	int height = src->GetPicture(0)->GetHeight();
	int time = src->GetLength();
	PictureList *result = new PictureList(width-1,height,time);
	result->SetName(src->GetName());
	
	Picture *frames = new Picture[time];
	for (int tt = 0; tt< time; tt++)
	{
		frames[tt] = *(new Picture(width-1,height));
		frames[tt].SetName(src->GetPicture(tt)->GetName());
	}
	result->SetList(frames,time);
	
	int x, y, t;
	pixelType pixel;
	for ( int  i = 0; i < width*height*time; i++ )
	{
		t = i / (width*height);
		x = (i % (width*height)) % width;
		y = (i % (width*height)) / width;
		
		pixel.r = src->GetPixel(x,y,t).r;
		pixel.g = src->GetPixel(x,y,t).g;
		pixel.b = src->GetPixel(x,y,t).b;

		if ( i < width*height*time-1 && 
			((gc->whatLabel(i)!=0 && gc->whatLabel(i+1)!=1) ||
			(gc->whatLabel(i) == gc->whatLabel(i+1))) )
		{
			if (gc->whatLabel(i)==0)
				result->SetPixel(x,y,t,pixel);
			else
				result->SetPixel(x-1,y,t,pixel);		//sink pixels shift left by 1
		}
		else if ( i == width*height*time-1 )
			result->SetPixel(x-1,y,t,pixel);
	}
	//delete src;
	return result;
}

PictureList *drawManifold(GCoptimization *gc, PictureList *src)
{
	int width = src->GetPicture(0)->GetWidth();
	int height = src->GetPicture(0)->GetHeight();
	int time = src->GetLength();
	PictureList *result = new PictureList(width,height,time);
	result->SetName(src->GetName());
	
	Picture *frames = new Picture[time];
	for (int tt = 0; tt< time; tt++)
	{
		frames[tt] = *(new Picture(width,height));
		frames[tt].SetName(src->GetPicture(tt)->GetName());
	}
	result->SetList(frames,time);
	
	int x, y, t;
	pixelType pixel;
	for ( int  i = 0; i < width*height*time; i++ )
	{
		t = i / (width*height);
		x = (i % (width*height)) % width;
		y = (i % (width*height)) / width;
		
		if ( i < width*height*time-1 && gc->whatLabel(i)==0 && gc->whatLabel(i+1)==1 )
		{
			//printf("Found Seam\n");
			pixel.r = 255;
			pixel.g = 0;
			pixel.b = 0;
		}
		else{
			//printf("SaveRetargetPicture: GetPixel(%d,%d)\n",x+gc->whatLabel(i),y);
			pixel.r = src->GetPixel(x,y,t).r;
			pixel.g = src->GetPixel(x,y,t).g;
			pixel.b = src->GetPixel(x,y,t).b;
		}
		result->SetPixel(x,y,t,pixel);
	}

	//delete src;
	return result;
}

void SaveRetargetVideo(PictureList *src, char *name)
{
	src->Save(name);
}

void SaveRetargetVideo(int *labels, PictureList *src,int width, int height, int time, char *name)
{
	PictureList *result = new PictureList(width,height,time);
	result->SetName(name);
	Picture *frames = new Picture[time];
	for (int tt = 0; tt< time; tt++)
	{
		frames[tt] = *(new Picture(width,height));
		frames[tt].SetName(src->GetPicture(tt)->GetName());
	}
	result->SetList(frames,time);
	
	int x, y, t;
	pixelType pixel;
	for ( int  i = 0; i < width*height*time; i++ )
	{
		t = i / (width*height);
		x = (i % (width*height)) % width;
		y = (i % (width*height)) / width;
		
		if ( i < width*height*time-1 && labels[i]==0 && labels[i+1]==1 )
		{
			//printf("Found Seam\n");
			pixel.r = 255;
			pixel.g = 0;
			pixel.b = 0;
		}
		else{
			//printf("SaveRetargetPicture: GetPixel(%d,%d)\n",x+gc->whatLabel(i),y);
			pixel.r = src->GetPixel(x,y,t).r;
			pixel.g = src->GetPixel(x,y,t).g;
			pixel.b = src->GetPixel(x,y,t).b;
		}
		result->SetPixel(x,y,t,pixel);
	}

	result->Save(result->GetName());

	delete result;
}

////////////////////////////////////////////////////////////////////////////////
// in this version, set data and smoothness terms using arrays
// grid neighborhood structure is assumed
//
PictureList *VideoSeamGraph_GraphCut(PictureList *src, int num_labels, int *data, int method)
{
	int width = src->GetPicture(0)->GetWidth();
	int height = src->GetPicture(0)->GetHeight();
	int time = src->GetLength();
	int num_pixels = width*height*time;
	GCoptimization *gc;
	ForSmoothFEFn toSmoothFEFn;
	ForSmoothBEFn toSmoothBEFn;

	int *result = new int[num_pixels];   // stores result of optimization

	try{
		if (method == 1)
		{
			gc = new GCoptimizationFwdEn3DSeamGraph(width, height, time, num_labels);
			gradient3D_FE *gradient = Gradient3D_FE(src);
		
			ForDataFn toDataFn;			
			toDataFn.data = data;		//wj from the data cost calculated earlier from CalcDataCost, assign the data cost to toDataFn.data
			toDataFn.numLab = num_labels;

			gc->setDataCost(&dataFn,&toDataFn);

			printf("gc->setDataCost(&dataFn,&toDataFn);\n");

			toSmoothFEFn.LR = gradient->LR;			
			toSmoothFEFn.pLU = gradient->pLU;
			toSmoothFEFn.nLU = gradient->nLU;
			toSmoothFEFn.temp_pLU = gradient->temp_pLU;
			toSmoothFEFn.temp_nLU = gradient->temp_nLU;

			toSmoothFEFn.original_width = width;
			toSmoothFEFn.original_height = height;
			toSmoothFEFn.original_time = time;

			gc->setSmoothCost(&smoothFEFn, &toSmoothFEFn);		// set the weights of the seam graph
		}
		else if (method == 0)
		{
			gc = new GCoptimization3DSeamGraph(width, height, time, num_labels);
			Matrix *gradient = Gradient_xy(src);
		
			ForDataFn toDataFn;			
			toDataFn.data = data;		//wj from the data cost calculated earlier from CalcDataCost, assign the data cost to toDataFn.data
			toDataFn.numLab = num_labels;

			gc->setDataCost(&dataFn,&toDataFn);

			printf("gc->setDataCost(&dataFn,&toDataFn);\n");

			toSmoothBEFn.gradient = gradient;
			toSmoothBEFn.original_width = width;
			toSmoothBEFn.original_height = height;
			toSmoothBEFn.original_time = time;

			gc->setSmoothCost(&smoothBEFn, &toSmoothBEFn);		// set the weights of the seam graph
			printf("gc->setSmoothCost(&smoothFn, &toSmoothFn)\n");		
		}

		
		printf("gc->setSmoothCost(&smoothFn, &toSmoothFn)\n");		
 		gc->expansion(1);
		printf("gc->expansion\n");
		
		if (method == 1)
		{
			delete [] toSmoothFEFn.LR;		
			delete [] toSmoothFEFn.pLU;
			delete [] toSmoothFEFn.nLU;
			delete [] toSmoothFEFn.temp_pLU;
			delete [] toSmoothFEFn.temp_nLU;
		}
		if (method == 0)
		{
			delete [] toSmoothBEFn.gradient;
		}
		
		src = drawManifold(gc, src);
		//src = removeManifold(gc, src);
		
		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	return src;

}

PictureList *Transpose_Video( PictureList *src )
{
	int width = src->GetPicture(0)->GetWidth();
	int height = src->GetPicture(0)->GetHeight();
	int time = src->GetLength();
	PictureList *tvid = new PictureList(height,width,time);
	tvid->SetName(src->GetName());

	Picture *frames = new Picture[time];
	for (int tt=0; tt<time; tt++)
	{
		frames[tt] = *(new Picture(height,width));
		frames[tt].SetName(src->GetPicture(tt)->GetName());
	}
	tvid->SetList(frames,time);

	int x, y, t;
	pixelType pixel;

	for ( int  i = 0; i < width*height*time; i++ )
	{
		t = i / (width*height);
		x = (i % (width*height)) % width;
		y = (i % (width*height)) / width;
		
		pixel.r = src->GetPixel(x,y,t).r;
		pixel.g = src->GetPixel(x,y,t).g;
		pixel.b = src->GetPixel(x,y,t).b;
		
		tvid->SetPixel(y,x,t,pixel);
	}
	return tvid;
}

int main(int argc, char **argv)
{
	PictureList *src = NULL;
	int width, height, time;
	listPyramidType *vpyramid = NULL;
	int num_pixels, num_labels;
	int *new_assignments;
	videoSize original_size;
	int *data;

	if (argc<6)
	{
		cout << "Usage: seam_carving_3d <src_folder> <num of v seams to remove> <num of h seams to remove> <output_folder> <method>" << endl;
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
	vpyramid = ListPyramid(src,3);
	src = &(vpyramid->Lists[0]);

	num_labels = 2;

	for( int s=1; s<=atoi(argv[2]); s++ )
	{
		printf("v seam #%d\n",s);
		width = src->GetPicture(0)->GetWidth();
		height = src->GetPicture(0)->GetHeight();
		time = src->GetLength();		
		data = CalcDataCost(width,height,time,num_labels);
		src = VideoSeamGraph_GraphCut(src,num_labels,data,atoi(argv[5]));
		delete data;
	}
	
	src = Transpose_Video(src);

	for( int s=1; s<=atoi(argv[3]); s++ )
	{
		printf("h seam #%d\n",s);
		width = src->GetPicture(0)->GetWidth();
		height = src->GetPicture(0)->GetHeight();
		time = src->GetLength();		
		data = CalcDataCost(width,height,time,num_labels);
		src = VideoSeamGraph_GraphCut(src,num_labels,data,atoi(argv[5]));
		delete data;
	}
	
	src = Transpose_Video(src);

	SaveRetargetVideo(src, argv[4]);
	
	delete [] vpyramid->Lists;
	delete vpyramid;
	//delete src;
	
	system("Pause");
	return 1;

}

/////////////////////////////////////////////////////////////////////////////////