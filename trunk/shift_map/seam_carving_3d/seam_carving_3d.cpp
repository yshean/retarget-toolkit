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

int *CalcPixelCost(GCoptimization *gc, int num_labels, int width, int height, int time, int *data)
{
	int prev_width = width/2;
	int prev_height = height/2;
	int prev_tot_pix = prev_width*prev_height*time;
	
	int ctl,ctr,cbl,cbr;
	int pixel_per_fme,fme,col,row;
	pixel_per_fme = prev_width*prev_height;

	for ( int i = 0; i <prev_tot_pix; i++ )
	{		
		fme = i/pixel_per_fme;
		col = (i-pixel_per_fme*fme)%prev_width;
		row = (i-pixel_per_fme*fme)/prev_width;

		if ( i < prev_tot_pix-1 && gc->whatLabel(i)==0 && gc->whatLabel(i+1)==0 )
		{
			ctl = (row*2*width)+(col*2)+(fme*width*height);//ctl = (row*2*width)+(col*2)+(fme*2*width*height);
			ctr = ctl+1;
			cbl = ctl+width;
			cbr = ctr+width;
			
			data[ctl*num_labels+0] = 0;
			data[ctl*num_labels+1] = MAX_COST_VALUE;
			data[ctr*num_labels+0] = 0;
			data[ctr*num_labels+1] = MAX_COST_VALUE;
			data[cbl*num_labels+0] = 0;
			data[cbl*num_labels+1] = MAX_COST_VALUE;
			data[cbr*num_labels+0] = 0;
			data[cbr*num_labels+1] = MAX_COST_VALUE;
		}
		else if ( i < prev_tot_pix-1 && gc->whatLabel(i)==1 && gc->whatLabel(i+1)==1 )
		{
			ctl = (row*2*width)+(col*2)+(fme*width*height);//ctl = (row*2*width)+(col*2)+(fme*2*width*height);
			ctr = ctl+1;
			cbl = ctl+width;
			cbr = ctr+width;
			
			data[ctl*num_labels+0] = MAX_COST_VALUE;
			data[ctl*num_labels+1] = 0;
			data[ctr*num_labels+0] = MAX_COST_VALUE;
			data[ctr*num_labels+1] = 0;
			data[cbl*num_labels+0] = MAX_COST_VALUE;
			data[cbl*num_labels+1] = 0;
			data[cbr*num_labels+0] = MAX_COST_VALUE;
			data[cbr*num_labels+1] = 0;
		}
	}
	return data;
}

int *CalcDataCost(GCoptimization *gc, int width, int height, int time, int num_labels)
{
	int num_pixels = width*height*time;
	// first set up the array for data costs
	int *data = new int[num_pixels*num_labels];
	int cost, pixel_per_pic, frame, col;

	for ( int i = 0; i < num_pixels; i++ )
	{
		for (int l = 0; l < num_labels; l++ )
		{
			cost = MAX_COST_VALUE;
			pixel_per_pic = width*height;
			frame = i / pixel_per_pic;
			col = (i % pixel_per_pic) % width;

			if (col==0 && l==0)								
				data[i*num_labels+0] = 0;							// setting cost=0 preserves that pixel
			else if (col==width-1 && l==1)
				data[i*num_labels+1] = 0;							// setting cost=0 preserves that pixel
			else
				data[i*num_labels+l] = MAX_COST_VALUE;
			
		}
	}
	
	if (gc!=NULL)				// use gc result from previous level for refining
		data=CalcPixelCost(gc,num_labels,width,height,time,data);

	return data;
}

PictureList *removeManifold(GCoptimization *gc, PictureList *src)
{
	int width = src->GetPicture(0)->GetWidth();
	int height = src->GetPicture(0)->GetHeight();
	int time = src->GetLength();
	string str;
	int pos;
	const char *c;

	PictureList *result = new PictureList(width-1,height,time);
	result->SetName(src->GetName());
	
	Picture *frames = new Picture[time];
	for (int tt = 0; tt< time; tt++)
	{
		//frames[tt] = *(new Picture(width-1,height));
		//frames[tt].SetName(src->GetPicture(tt)->GetName());
		frames[tt] = *(new Picture(width-1,height));
		str = src->GetPicture(tt)->GetName();
		pos = str.find("seam_");
		if (pos!=-1)
			str = str.erase (0,5);
		
		c=str.c_str();
		frames[tt].SetName(c);
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
	delete [] src;
	return result;

}

//PictureList *drawManifold(GCoptimization *gc, PictureList *src)
//{
//	int width = src->GetPicture(0)->GetWidth();
//	int height = src->GetPicture(0)->GetHeight();
//	int time = src->GetLength();
//	PictureList *result = new PictureList(width,height,time);
//	result->SetName(src->GetName());
//	
//	Picture *frames = new Picture[time];
//	for (int tt = 0; tt< time; tt++)
//	{
//		//frames[tt] = *(new Picture(width,height));
//		//frames[tt].SetName(src->GetPicture(tt)->GetName());
//		frames[tt] = *(new Picture(width,height));
//		char target_name[512] = "seam_";
//		strcat(target_name,src->GetPicture(tt)->GetName());
//		frames[tt].SetName(target_name);
//	}
//	result->SetList(frames,time);
//	
//	int x, y, t;
//	pixelType pixel;
//	for ( int  i = 0; i < width*height*time; i++ )
//	{
//		t = i / (width*height);
//		x = (i % (width*height)) % width;
//		y = (i % (width*height)) / width;
//		
//		if ( i < width*height*time-1 && gc->whatLabel(i)==0 && gc->whatLabel(i+1)==1 )
//		{
//			//printf("Found Seam\n");
//			pixel.r = 255;
//			pixel.g = 0;
//			pixel.b = 0;
//		}
//		else{
//			//printf("SaveRetargetPicture: GetPixel(%d,%d)\n",x+gc->whatLabel(i),y);
//			pixel.r = src->GetPixel(x,y,t).r;
//			pixel.g = src->GetPixel(x,y,t).g;
//			pixel.b = src->GetPixel(x,y,t).b;
//		}
//		result->SetPixel(x,y,t,pixel);
//	}
//
//	//delete src;
//	return result;
//}


void drawManifold(GCoptimization *gc, PictureList *src)
{
	int width = src->GetMaxWidth();
	int height = src->GetMaxHeight();
	int time = src->GetLength();
	
	for (int tt = 0; tt< time; tt++)
	{
		char target_name[512] = "seam_";
		strcat(target_name,src->GetPicture(tt)->GetName());
		src->GetPicture(tt)->SetName(target_name);
	}
	
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
			src->SetPixel(x,y,t,pixel);
		}
	}
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
		//frames[tt] = *(new Picture(width,height));
		//frames[tt].SetName(src->GetPicture(tt)->GetName());
		frames[tt] = *(new Picture(width,height));
		char target_name[512] = "seam_";
		strcat(target_name,src->GetPicture(tt)->GetName());
		frames[tt].SetName(target_name);
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
GCoptimization/*PictureList*/ *VideoSeamGraph_GraphCut(PictureList *src, int num_labels, int *data, int method)
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

			printf("FwdEn3DSeamGraph setDataCost(&dataFn,&toDataFn);\n");

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

			printf("3DSeamGraph setDataCost(&dataFn,&toDataFn);\n");

			toSmoothBEFn.gradient = gradient;
			toSmoothBEFn.original_width = width;
			toSmoothBEFn.original_height = height;
			toSmoothBEFn.original_time = time;

			gc->setSmoothCost(&smoothBEFn, &toSmoothBEFn);		// set the weights of the seam graph		
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
		
		//src = drawManifold(gc, src);
		//src = removeManifold(gc, src);
		
		//delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	//return src;
	return gc;
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
	int src_width, src_height, src_time;
	int prev_width=0, prev_height=0, prev_time=0;

	listPyramidType *lpyramid = NULL;
	int num_labels;
	int *data;
	GCoptimization *gc = NULL;
	int pym_level = atoi(argv[6]);					// specify no. of pyramid levels
	int list_level;						// 0 <= list_level < pym_level

	time_t start, end;

	if (argc<6)
	{
		cout << "Usage: seam_carving_3d <src_folder> <num of v seams to remove> <num of h seams to remove> <output_folder> <method> <pyramid_level>" << endl;
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

	//// load input video
	//cout << "Creating gaussian pyramid for input video" << endl;
	//src = new PictureList(argv[1]);
	//vpyramid = ListPyramid(src,3);
	//src = &(vpyramid->Lists[0]);

	//num_labels = 2;

	//for( int s=1; s<=atoi(argv[2]); s++ )
	//{
	//	printf("v seam #%d\n",s);
	//	width = src->GetPicture(0)->GetWidth();
	//	height = src->GetPicture(0)->GetHeight();
	//	time = src->GetLength();		
	//	data = CalcDataCost(width,height,time,num_labels);
	//	src = VideoSeamGraph_GraphCut(src,num_labels,data,atoi(argv[5]));
	//	delete data;
	//}
	//
	//src = Transpose_Video(src);

	//for( int s=1; s<=atoi(argv[3]); s++ )
	//{
	//	printf("h seam #%d\n",s);
	//	width = src->GetPicture(0)->GetWidth();
	//	height = src->GetPicture(0)->GetHeight();
	//	time = src->GetLength();		
	//	data = CalcDataCost(width,height,time,num_labels);
	//	src = VideoSeamGraph_GraphCut(src,num_labels,data,atoi(argv[5]));
	//	delete data;
	//}
	//
	//src = Transpose_Video(src);

	//SaveRetargetVideo(src, argv[4]);
	//
	//delete [] vpyramid->Lists;
	//delete vpyramid;
	////delete src;
	//
	//system("Pause");
	//return 1;
		// load input video
	cout << "Creating gaussian pyramid for input video" << endl;
	src = new PictureList(argv[1]);

	num_labels = 2;

	for( int s=1; s<=atoi(argv[2]); s++ )
	{
		printf("\t\t>>> v seam #%d\n",s);
		list_level = pym_level - 1;
		time( &start );
		lpyramid = ListPyramid(src, pym_level);
		time( &end );
		cout << "compute L Pyramid: " << difftime( end, start ) << " seconds" << endl;

		delete src;
		
		// Refines the seam from the previous level. Continue to refine until the original video size.
		while ( list_level>=0 )
		{
			printf("\t\tcurrent list_level is %d\n", list_level);
			src = &(lpyramid->Lists[list_level]);
			src_width = src->GetMaxWidth();
			src_height = src->GetMaxHeight();
			src_time = src->GetLength();	
			time( &start );
			data = CalcDataCost(gc,src_width,src_height,src_time,num_labels);
			time( &end );
			cout << "CalcDataCost list_level " << list_level << ": " << difftime( end, start ) << " seconds" << endl;
			delete gc;
			time( &start );
			gc = VideoSeamGraph_GraphCut(src,num_labels,data,atoi(argv[5]));
			time( &end );
			cout << "VideoSeamGraph_GraphCut list_level " << list_level << ": " << difftime( end, start ) << " seconds" << endl;
			delete data;

			list_level--;
		}
		delete lpyramid;
		
		//src = removeManifold(gc, src);
		
		drawManifold(gc, src);
		
		SaveRetargetVideo(src, argv[4]);

		src = removeManifold(gc, src);
		
		SaveRetargetVideo(src, argv[4]);

		delete gc;
		gc=NULL;
	}
	
	if ( atoi(argv[3]) > 0 )			// if the number of hseams to be removed<=0, then skip hseam removal step.
	{
		//21OCT: COMMENT OUT FIRST AS FUNCTION TO SAVE BOTH THE SEAM AND OUTPUT INTO FOLDER IS NOT IMPLEMENTED YET
		//src = Transpose_Video(src);

		//for( int s=1; s<=atoi(argv[3]); s++ )
		//{
		//	printf("\t\t>>> h seam #%d\n",s);
		//	list_level = pym_level - 1;
		//	lpyramid = ListPyramid(src, pym_level);
		//	delete src;
		//	// Refines the seam from the previous level. Continue to refine until the original video size.
		//	while ( list_level>=0 )				
		//	{
		//		printf("\t\tcurrent list_level is %d\n", list_level);
		//		src = &(lpyramid->Lists[list_level]);
		//		width = src->GetMaxWidth();
		//		height = src->GetMaxHeight();
		//		time = src->GetLength();
		//		data = CalcDataCost(gc,width,height,time,num_labels);
		//		delete gc;
		//		gc = VideoSeamGraph_GraphCut(src,num_labels,data,atoi(argv[5]));
		//		delete data;

		//		list_level--;
		//	}
		//	delete lpyramid;
		//	
		//	//if (s<atoi(argv[3]))
		//		src = removeManifold(gc, src);
		//	//else
		//	//	drawManifold(gc, src);
		//	
		//	delete gc;
		//	gc = NULL;
		//}
		//delete gc;

		//src = Transpose_Video(src);
	}

	SaveRetargetVideo(src, argv[4]);

	delete src;
	
	//system("Pause");
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////