
/*
 *    Utility functions
 *
 *    Author: Edward Alston Anthony
 *
 */

#include <vector>
#include <windows.h>
#include "utils.h"

using namespace std;

inline double round( double d )
{
	return floor( d + 0.5 );
}

float simpleGauss(float x, float sigma, float mu)
{
	if(sigma == 0) {
		cerr << "simpleGauss: sigma must be non-zero" << endl;
		exit(1);
	}
	float pi = 3.1415926;
	float x_p = x - mu;
	float exponent = exp(x_p * x_p / (-2 * sigma * sigma));
	exponent /= sqrt(2 * pi) * sigma;
	return exponent;
}

/* Converts the RGB values of a pixel to grayscale */
double Intensity(pixelType p)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  return ((p.r * 0.3) + (p.g * 0.59) + (p.b * 0.11));
}

/* Returns the convolution value of the pixel Source(x,y)
 * with the given kernel
 */
int Convolve_Pixel(Picture *src, int y, int x, int kernel[][3])
{
	int conv = 0;
	int width = src->GetWidth();
	int height = src->GetHeight();

	//these loops are a bit complicted since I need to access pixels all around the called pixel
	for(int xx = 0; xx < 3; xx++)
	{
		for(int yy = 0; yy < 3; yy++)
		{
			if (x+xx-1>=0 && x+xx-1<width 
				&& y+yy-1>=0 && y+yy-1<height)
			{
				conv += src->GetPixelIntensity(x + xx - 1, y + yy - 1).r * kernel[xx][yy];
			} else
			{
				int bound_x = min(max(x+xx-1,0),width-1);
				int bound_y = min(max(y+yy-1,0),height-1);
				intensityType tmp = src->GetPixelIntensity(bound_x, bound_y);
				conv += src->GetPixelIntensity(bound_x, bound_y).r * kernel[xx][yy];
			}
		}
	}

	return conv;
}

/* Calculate the gradient magnitude of the source image
 * using Prewitt Method
 */
gradient2D *Gradient(Picture *src)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	Matrix *dx = new Matrix(src->GetHeight(),src->GetWidth());
	Matrix *dy = new Matrix(src->GetHeight(),src->GetWidth());
	double total_dx = 0.0;
	double total_dy = 0.0;

  	//Prewitt's gradient edge detector kernels, first the x direction, then the y
	int Prewitt_Kernel_X[3][3] = 
	{
		{ 1, 0, -1 },
		{ 1, 0, -1 },
		{ 1, 0, -1 }
	};
	int Prewitt_Kernel_Y[3][3] = 
	{
		{ 1, 1, 1 },
		{ 0, 0, 0 },
		{ -1, -1, -1 }
	};
	
	int edge_x = 0;
	int edge_y = 0;
	int edge_value = 0;
	for( int i = 0; i < src->GetHeight(); i++ )
	{
		for( int j = 0; j < src->GetWidth(); j++ )
		{
			//get the edges for the x and y directions
			edge_x = Convolve_Pixel(src,i,j,Prewitt_Kernel_X);
			edge_y = Convolve_Pixel(src,i,j,Prewitt_Kernel_Y);

			//add their weights up
			//edge_value = (abs(edge_x) + abs(edge_y));
			//result->Set(i+1,j+1,edge_value);
			dx->Set(i+1,j+1,abs(edge_x));
			dy->Set(i+1,j+1,abs(edge_y));
			
			total_dx += abs(edge_x);
			total_dy += abs(edge_y);
		}
	}

	gradient2D *result = new gradient2D;
	result->dx = dx;
	result->dy = dy;
	result->total_dx = total_dx;
	result->total_dy = total_dy;
	return result;
}

/* Calculate the spatial temporal gradient magnitude of 
 * the source image using Prewitt Method
 */
gradient3D *Gradient_3D(Video *src, double threshold)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	Matrix *dx = new Matrix[src->GetTime()];
	Matrix *dy = new Matrix[src->GetTime()];
	Matrix *dt = new Matrix[src->GetTime()];
	double *total_dx = new double[src->GetTime()];
	double *total_dy = new double[src->GetTime()];
	double *total_dt = new double[src->GetTime()];

	gradient2D *frame_gradient = NULL;
	for (int t=0; t<src->GetTime(); t++)
	{
		//results[t] = *(new Matrix(0,0));
		// results[t] = *(Gradient(src->GetFrame(t)));
		if (frame_gradient)
			delete frame_gradient;

		frame_gradient = Gradient(src->GetFrame(t));
		dx[t] = *(frame_gradient->dx);
		dy[t] = *(frame_gradient->dy);
		total_dx[t] = frame_gradient->total_dx;
		total_dy[t] = frame_gradient->total_dy;
		
		double tdt = 0.0;
		if (t>0 && t<(src->GetTime()-1))
		{			
			//results[t] += *(FrameDifference(src->GetFrame(t-1),src->GetFrame(t+1)));
			dt[t] = *(FrameDifference(src->GetFrame(t-1),src->GetFrame(t+1),tdt,threshold));
			total_dt[t] = tdt;
		}
		else
		{
			int lower_t = max(t-1,0);
			int upper_t = min(t+1,src->GetTime()-1);
			//results[t] += *(FrameDifference(src->GetFrame(lower_t),src->GetFrame(upper_t)));
			dt[t] = *(FrameDifference(src->GetFrame(lower_t),src->GetFrame(upper_t),tdt,threshold));
			total_dt[t] = tdt;
		}
	}

	gradient3D *results = new gradient3D;
	results->dx = dx;
	results->dy = dy;
	results->dt = dt;
	results->total_dx = total_dx;
	results->total_dy = total_dy;
	results->total_dt = total_dt;

	return results;
}

/* Calculate the spatial temporal gradient magnitude of 
 * the source image using Prewitt Method
 */
gradient2D *Naturality_2D(Picture *src, double threshold)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	Matrix *dx = new Matrix(src->GetHeight(),src->GetWidth());
	Matrix *dy = new Matrix(src->GetHeight(),src->GetWidth());
	double total_dx = 0.0;
	double total_dy = 0.0;

	//results[t] = *(new Matrix(0,0));
	// results[t] = *(Gradient(src->GetFrame(t)));
	Matrix *grayscale = Rgb2Gray(src);
	for (int y=0; y<src->GetHeight(); y++)
	{
		for (int x=0; x<src->GetWidth(); x++)
		{
			if (x>0 && x<src->GetWidth()-1)
			{
				double diff = min(abs(grayscale->Get(y+1,x+1) - grayscale->Get(y+1,x+2)),
									  abs(grayscale->Get(y+1,x) - grayscale->Get(y+1,x+1)));
				if (diff>=threshold)
					dx->Set(y+1,x+1,diff);
				else
					dx->Set(y+1,x+1,0.0);
			}

			if (x<src->GetWidth()-1 && y<src->GetHeight()-1)
			{
				double diff = min(abs(grayscale->Get(y+1,x+1) - grayscale->Get(y+1,x+2)),
								  abs(grayscale->Get(y+2,x+1) - grayscale->Get(y+2,x+2)));
				if (diff>=threshold)
					dy->Set(y+1,x+1,diff);
				else
					dy->Set(y+1,x+1,0.0);
			}
		}
	} // end for

	delete grayscale;

	gradient2D *results = new gradient2D;
	results->dx = dx;
	results->dy = dy;
	results->total_dx = total_dx;
	results->total_dy = total_dy;

	return results;
}

/* Calculate the spatial temporal gradient magnitude of 
 * the source image using Prewitt Method
 */
gradient2D *Diff_2D(Picture *src, double threshold)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	Matrix *dx = new Matrix(src->GetHeight(),src->GetWidth());
	Matrix *dy = new Matrix(src->GetHeight(),src->GetWidth());
	double total_dx = 0.0;
	double total_dy = 0.0;

	//results[t] = *(new Matrix(0,0));
	// results[t] = *(Gradient(src->GetFrame(t)));
	Matrix *grayscale = Rgb2Gray(src);
	for (int y=0; y<src->GetHeight(); y++)
	{
		for (int x=0; x<src->GetWidth(); x++)
		{
			if (x<src->GetWidth()-1)
			{
				double diff = abs(grayscale->Get(y+1,x+1) - grayscale->Get(y+1,x+2));
				if (diff>=threshold)
					dx->Set(y+1,x+1,diff);
				else
					dx->Set(y+1,x+1,0.0);
			}

			if (y<src->GetHeight()-1)
			{
				double diff = abs(grayscale->Get(y+1,x+1) - grayscale->Get(y+2,x+1));
				if (diff>=threshold)
					dy->Set(y+1,x+1,diff);
				else
					dy->Set(y+1,x+1,0.0);
			}
		}
	} // end for

	gradient2D *results = new gradient2D;
	results->dx = dx;
	results->dy = dy;
	results->total_dx = total_dx;
	results->total_dy = total_dy;

	return results;
}

/* Calculate the spatial temporal gradient magnitude of 
 * the source image using Prewitt Method
 */
gradient3D *Naturality_3D(PictureList *src, double threshold)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	Matrix *dx = new Matrix[src->GetLength()];
	Matrix *dy = new Matrix[src->GetLength()];
	Matrix *dt = new Matrix[src->GetLength()];
	double *total_dx = new double[src->GetLength()];
	double *total_dy = new double[src->GetLength()];
	double *total_dt = new double[src->GetLength()];
	double sigma = src->GetMaxWidth()/4;

	double weight;
	for (int t=0; t<src->GetLength(); t++)
	{
		//results[t] = *(new Matrix(0,0));
		// results[t] = *(Gradient(src->GetFrame(t)));
		Matrix *grayscale = Rgb2Gray(src->GetPicture(t));
		Matrix *xdiff = new Matrix(src->GetPicture(t)->GetHeight(),
								   src->GetPicture(t)->GetWidth());
		Matrix *ydiff = new Matrix(src->GetPicture(t)->GetHeight(),
								   src->GetPicture(t)->GetWidth());
		for (int y=0; y<src->GetPicture(t)->GetHeight(); y++)
		{
			for (int x=0; x<src->GetPicture(t)->GetWidth(); x++)
			{
				if (x>0 && x<src->GetPicture(t)->GetWidth()-1)
				{
					weight = simpleGauss(x,sigma,src->GetPicture(t)->GetWidth()/2);
					double diff = min(abs(grayscale->Get(y+1,x+1) - grayscale->Get(y+1,x+2)),
									  abs(grayscale->Get(y+1,x) - grayscale->Get(y+1,x+1)));
					if (diff>=threshold)
						xdiff->Set(y+1,x+1,weight*diff);
					else
						xdiff->Set(y+1,x+1,0.0);
				}

				if (x<src->GetPicture(t)->GetWidth()-1 && y<src->GetPicture(t)->GetHeight()-1)
				{
					weight = simpleGauss(x,sigma,src->GetPicture(t)->GetWidth()/2);
					double diff = min(abs(grayscale->Get(y+1,x+1) - grayscale->Get(y+1,x+2)),
									  abs(grayscale->Get(y+2,x+1) - grayscale->Get(y+2,x+2)));
					if (diff>=threshold)
						ydiff->Set(y+1,x+1,weight*diff);
					else
						ydiff->Set(y+1,x+1,0.0);
				}
			}
		} // end for

		
		dx[t] = *(xdiff);
		dy[t] = *(ydiff);
		total_dx[t] = 0.0;
		total_dy[t] = 0.0;
		delete xdiff;
		delete ydiff;
		
		double tdt = 0.0;
		if (t<(src->GetLength()-1))
		{
			Matrix *next = Rgb2Gray(src->GetPicture(t+1));
			Matrix *tdiff = new Matrix(src->GetPicture(t)->GetHeight(),
									   src->GetPicture(t)->GetWidth());
			for (int y=0; y<src->GetPicture(t)->GetHeight(); y++)
			{
				for (int x=0; x<src->GetPicture(t)->GetWidth(); x++)
				{
					if (x<src->GetPicture(t)->GetWidth()-1)
					{
						weight = simpleGauss(x,sigma,src->GetPicture(t)->GetWidth()/2);
						double diff = min(abs(grayscale->Get(y+1,x+1) - next->Get(y+1,x+2)),
										  abs(next->Get(y+1,x+1) - next->Get(y+1,x+2)));
						if (diff>=threshold)
							tdiff->Set(y+1,x+1,weight*diff);
						else
							tdiff->Set(y+1,x+1,0.0);
					}
				}
			} // end for

			dt[t] = *(tdiff);
			total_dt[t] = 0.0;
			delete tdiff;
			delete next;
		}

		delete grayscale;
	}

	gradient3D *results = new gradient3D;
	results->dx = dx;
	results->dy = dy;
	results->dt = dt;
	results->total_dx = total_dx;
	results->total_dy = total_dy;
	results->total_dt = total_dt;

	return results;
}

/* Calculate the spatial temporal gradient magnitude of 
 * the source image using Prewitt Method
 */
gradient3D *Diff_3D(PictureList *src, double threshold)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	Matrix *dx = new Matrix[src->GetLength()];
	Matrix *dy = new Matrix[src->GetLength()];
	Matrix *dt = new Matrix[src->GetLength()];
	double *total_dx = new double[src->GetLength()];
	double *total_dy = new double[src->GetLength()];
	double *total_dt = new double[src->GetLength()];

	for (int t=0; t<src->GetLength(); t++)
	{
		//results[t] = *(new Matrix(0,0));
		// results[t] = *(Gradient(src->GetFrame(t)));
		Matrix *grayscale = Rgb2Gray(src->GetPicture(t));
		Matrix *xdiff = new Matrix(src->GetPicture(t)->GetHeight(),
								   src->GetPicture(t)->GetWidth());
		Matrix *ydiff = new Matrix(src->GetPicture(t)->GetHeight(),
								   src->GetPicture(t)->GetWidth());
		for (int y=0; y<src->GetPicture(t)->GetHeight(); y++)
		{
			for (int x=0; x<src->GetPicture(t)->GetWidth(); x++)
			{
				if (x<src->GetPicture(t)->GetWidth()-1)
				{
					double diff = abs(grayscale->Get(y+1,x+1) - grayscale->Get(y+1,x+2));
					if (diff>=threshold)
						xdiff->Set(y+1,x+1,diff);
					else
						xdiff->Set(y+1,x+1,0.0);
				}

				if (y<src->GetPicture(t)->GetHeight()-1)
				{
					double diff = abs(grayscale->Get(y+1,x+1) - grayscale->Get(y+2,x+1));
					if (diff>=threshold)
						ydiff->Set(y+1,x+1,diff);
					else
						ydiff->Set(y+1,x+1,0.0);
				}
			}
		} // end for

		
		dx[t] = *(xdiff);
		dy[t] = *(ydiff);
		total_dx[t] = 0.0;
		total_dy[t] = 0.0;
		delete xdiff;
		delete ydiff;
		
		double tdt = 0.0;
		if (t<(src->GetLength()-1))
		{
			Matrix *next = Rgb2Gray(src->GetPicture(t+1));
			Matrix *tdiff = new Matrix(src->GetPicture(t)->GetHeight(),
									   src->GetPicture(t)->GetWidth());
			for (int y=0; y<src->GetPicture(t)->GetHeight(); y++)
			{
				for (int x=0; x<src->GetPicture(t)->GetWidth(); x++)
				{
					double diff = abs(grayscale->Get(y+1,x+1) - next->Get(y+1,x+1));
					if (diff>=threshold)
						tdiff->Set(y+1,x+1,diff);
					else
						tdiff->Set(y+1,x+1,0.0);
				}
			} // end for

			dt[t] = *(tdiff);
			total_dt[t] = 0.0;
			delete tdiff;
			delete next;
		}

		delete grayscale;
	}

	gradient3D *results = new gradient3D;
	results->dx = dx;
	results->dy = dy;
	results->dt = dt;
	results->total_dx = total_dx;
	results->total_dy = total_dy;
	results->total_dt = total_dt;

	return results;
}

double ColorContrast(Video *src, int x, int y, int t)
{
	int width = src->GetFrame(t)->GetWidth();
	int height = src->GetFrame(t)->GetHeight(); 
	int time = src->GetTime();	

	double contrast = 0.0;
	for (int t_offset=-1; t_offset<=1; t_offset++)
		for (int y_offset=-1; y_offset<=1; y_offset++)
			for (int x_offset=-1; x_offset<=1; x_offset++)
				if (t+t_offset>=0 && t+t_offset<time &&
					y+y_offset>=0 && y+y_offset<height &&
					x+x_offset>=0 && x+x_offset<width)
				{
					contrast += pow((double)(src->GetFrame(t)->GetPixel(x,y).r-
											 src->GetFrame(t+t_offset)->GetPixel(x+x_offset,y+y_offset).r),2);
					contrast += pow((double)(src->GetFrame(t)->GetPixel(x,y).g-
											 src->GetFrame(t+t_offset)->GetPixel(x+x_offset,y+y_offset).g),2);
					contrast += pow((double)(src->GetFrame(t)->GetPixel(x,y).b-
											 src->GetFrame(t+t_offset)->GetPixel(x+x_offset,y+y_offset).b),2);
				}

	return contrast;

}

double GradientContrast(gradient3D *gradient, int time,int x, int y, int t)
{
	int width = gradient->dx[t].NumOfCols();
	int height = gradient->dx[t].NumOfRows(); 	

	double contrast = 0.0;
	for (int t_offset=-1; t_offset<=1; t_offset++)
		for (int y_offset=-1; y_offset<=1; y_offset++)
			for (int x_offset=-1; x_offset<=1; x_offset++)
				if (t+t_offset>=0 && t+t_offset<time &&
					y+y_offset>0 && y+y_offset<=height &&
					x+x_offset>0 && x+x_offset<=width)
				{
					contrast += pow((double)(gradient->dx[t].Get(y,x)-
											 gradient->dx[t+t_offset].Get(y+y_offset,x+x_offset)),2);
					contrast += pow((double)(gradient->dy[t].Get(y,x)-
											 gradient->dy[t+t_offset].Get(y+y_offset,x+x_offset)),2);
					contrast += pow((double)(gradient->dt[t].Get(y,x)-
											 gradient->dt[t+t_offset].Get(y+y_offset,x+x_offset)),2);
				}

	return contrast;

}

/* Calculate the spatial temporal contrast of 
 * the source video using Prewitt Method
 */
Matrix *Contrast_3D(Video *src, gradient3D *gradient)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	Matrix *contrast = new Matrix[src->GetTime()];
	for (int t=0; t<src->GetTime(); t++)
		contrast[t] = *(new Matrix(src->GetFrame(t)->GetHeight(),
								   src->GetFrame(t)->GetWidth()));

	for (int t=0; t<src->GetTime(); t++)
		for (int y=0; y<src->GetFrame(t)->GetHeight(); y++)
			for (int x=0; x<src->GetFrame(t)->GetWidth(); x++)
			{
				double local_contrast = ColorContrast(src,x,y,t);
				local_contrast += GradientContrast(gradient,src->GetTime(),x+1,y+1,t);

				contrast[t].Set(y+1,x+1,local_contrast);
			}

	return contrast;
}


Matrix *FrameDifference(Picture *left, Picture *right, double &total_dt, double threshold)
{
	if ((left->GetHeight() != right->GetHeight()) ||
		(right->GetWidth() != right->GetWidth()))
		throw IncompatibleDimensionsException("Utils", "FrameDifference");

	Matrix *result = new Matrix(left->GetHeight(),left->GetWidth());

	total_dt = 0.0;
	for (int x=0; x<left->GetWidth(); x++)
	{
		for (int y=0; y<left->GetHeight(); y++)
		{
			result->Set(y+1,x+1,
					abs(left->GetPixelIntensity(x,y).r-right->GetPixelIntensity(x,y).r)+
					abs(left->GetPixelIntensity(x,y).g-right->GetPixelIntensity(x,y).g)+
					abs(left->GetPixelIntensity(x,y).b-right->GetPixelIntensity(x,y).b));

			if (result->Get(y+1,x+1)>threshold)
				total_dt += result->Get(y+1,x+1);
			else
				result->Set(y+1,x+1,0.0);
		}
	}

	return result;
}

/*  produces the output image based on the pair of images
 *  and a transformation matrix that takes the first image
 *  and transforms it to overlap with the second image
 */
Picture *DrawImage(Picture *I1, Picture *I2, Matrix *M, bool UseMultiresolutionSpline)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  pixelType c, c1, c2;
  Picture *result = NULL;
  Picture *overlapI1 = NULL;
  Picture *overlapI2 = NULL;
  
  Matrix *mi = M->Inverse();
  mi->NormalizeBy(mi->Get(3, 3));

  int xMin = min((int) round(M->Get(1, 3)), 0);
  int xMax = max((int) (round(M->Get(1, 3)) + I1->GetWidth()), I2->GetWidth());
  int yMin = min((int) round(M->Get(2, 3)), 0);
  int yMax = max((int) (round(M->Get(2, 3)) + I1->GetHeight()), I2->GetHeight());

  int width = (int) (floor((double)(xMax - xMin)) - (xMax - xMin) % 8) + 8;
  int height = (int) (floor((double)(yMax - yMin)) - (yMax - yMin) % 8) + 8;

  /*  for use in the multiresolution splining
   *  to discover a suitable weighted average
   *  at the overlapping region
   */
  pointType Max1 = { 0, 0 }, Max2 = { 0, 0 }, overlapMax = { 0, 0 };
  pointType Min1 = { width, height }, Min2 = { width, height }, overlapMin = { width, height };

  result = new Picture(width, height);
  overlapI1 = new Picture(width, height);
  overlapI2 = new Picture(width, height);

  for (int x = xMin; x < xMax; x++) {
    for (int y = yMin; y < yMax; y++) {
      double D, xp, yp;
  
      memset((void *) &c, 0, sizeof(pixelType));

      D = (mi->Get(3, 1) * x) + (mi->Get(3, 2) * y) + 1;
      xp = ((mi->Get(1, 1) * x) + (mi->Get(1, 2) * y) + mi->Get(1, 3)) / D;
      yp = ((mi->Get(2, 1) * x) + (mi->Get(2, 2) * y) + mi->Get(2, 3)) / D; 

      if (I1->Inside((int) round(xp), (int) round(yp)) &&
          I2->Inside(x, y)) {
        try { c1 = I1->GetPixel((int) round(xp), (int) round(yp)); }
        catch (IndexOutOfBoundsException ex) {}

        try { c2 = I2->GetPixel(x, y); }
        catch (IndexOutOfBoundsException ex) {}

        if (Max1.x < x + (0 - xMin))
          Max1.x = x + (0 - xMin);
        if (Min1.x > x + (0 - xMin))
          Min1.x = x + (0 - xMin);
        if (Max1.y < y + (0 - yMin))
          Max1.y = y + (0 - yMin);
        if (Min1.y > y + (0 - yMin))
          Min1.y = y + (0 - yMin);

        if (overlapMax.x < x + (0 - xMin))
          overlapMax.x = x + (0 - xMin);
        if (overlapMin.x > x + (0 - xMin))
          overlapMin.x = x + (0 - xMin);
        if (overlapMax.y < y + (0 - yMin))
          overlapMax.y = y + (0 - yMin);
        if (overlapMin.y > y + (0 - yMin))
          overlapMin.y = y + (0 - yMin);

        if (Max2.x < x + (0 - xMin))
          Max2.x = x + (0 - xMin);
        if (Min2.x > x + (0 - xMin))
          Min2.x = x + (0 - xMin);
        if (Max2.y < y + (0 - yMin))
          Max2.y = y + (0 - yMin);
        if (Min2.y > y + (0 - yMin))
          Min2.y = y + (0 - yMin);

        /*  collect overlapping points on two separate image space
         *  to perform the multiresolution splining on the overlapping region
         */
        try { overlapI1->SetPixel(x + (0 - xMin), y + (0 - yMin), c1); }
        catch (IndexOutOfBoundsException ex) {}

        try { overlapI2->SetPixel(x + (0 - xMin), y + (0 - yMin), c2); }
        catch (IndexOutOfBoundsException ex) {}
      }
      else if (I1->Inside((int) round(xp), (int) round(yp))) {
        if (Max1.x < x + (0 - xMin))
          Max1.x = x + (0 - xMin);
        if (Min1.x > x + (0 - xMin))
          Min1.x = x + (0 - xMin);
        if (Max1.y < y + (0 - yMin))
          Max1.y = y + (0 - yMin);
        if (Min1.y > y + (0 - yMin))
          Min1.y = y + (0 - yMin);

        try { c = I1->GetPixel((int) round(xp), (int) round(yp)); }
        catch (IndexOutOfBoundsException ex) {}

        try { overlapI1->SetPixel(x + (0 - xMin), y + (0 - yMin), c); }
        catch (IndexOutOfBoundsException ex) {}
      }
      else if (I2->Inside(x, y)) {
        if (Max2.x < x + (0 - xMin))
          Max2.x = x + (0 - xMin);
        if (Min2.x > x + (0 - xMin))
          Min2.x = x + (0 - xMin);
        if (Max2.y < y + (0 - yMin))
          Max2.y = y + (0 - yMin);
        if (Min2.y > y + (0 - yMin))
          Min2.y = y + (0 - yMin);

        try { c = I2->GetPixel(x, y); }
        catch (IndexOutOfBoundsException ex) {}

        try { overlapI2->SetPixel(x + (0 - xMin), y + (0 - yMin), c); }
        catch (IndexOutOfBoundsException ex) {}
      }
      else
        memset((void *) &c, 0, sizeof(pixelType));

      if (!UseMultiresolutionSpline) {
        try { result->SetPixel(x + (0 - xMin), y + (0 - yMin), c); }
        catch (IndexOutOfBoundsException ex) {}
      }
    }  
  }

  if (UseMultiresolutionSpline) {
    Picture *temp = Combine(overlapI1, overlapI2, Max1, Min1, Max2, Min2, overlapMax, overlapMin);
    *result = *temp;
    delete temp;
  }
  else {
    /*  find the weights for the contribution of pixel
     *  intensity of the overlapping images and draw the
     *  the pixel of that overlapping region
     */

    pixelType c;
    int x1Max = 0, y1Max = 0;
    int x1Min = overlapI1->GetWidth();
    int y1Min = overlapI1->GetHeight();

    int x2Max = 0, y2Max = 0;
    int x2Min = overlapI2->GetWidth();
    int y2Min = overlapI2->GetHeight();

    for (int x = 0; x < overlapI1->GetWidth(); x++)
      for (int y = 0; y < overlapI1->GetHeight(); y++) {
        c = overlapI1->GetPixel(x, y);
        if ((c.r != 0) || (c.g != 0) || (c.b != 0)) {
          x1Min = min(x1Min, x);
          x1Max = max(x1Max, x);
          y1Min = min(y1Min, y);
          y1Max = max(y1Max, y);
        }

        c = overlapI2->GetPixel(x, y);
        if ((c.r != 0) || (c.g != 0) || (c.b != 0)) {
          x2Min = min(x2Min, x);
          x2Max = max(x2Max, x);
          y2Min = min(y2Min, y);
          y2Max = max(y2Max, y);
        }
      }

    for (int x = 0; x < overlapI1->GetWidth(); x++)
      for (int y = 0; y < overlapI1->GetHeight(); y++) {
        pixelType c1 = overlapI1->GetPixel(x, y);
        pixelType c2 = overlapI2->GetPixel(x, y);

        if (((c1.r != 0) || (c1.g != 0) || (c1.b != 0)) &&
            ((c2.r != 0) || (c2.g != 0) || (c2.b != 0))) {
          double w1, w2;

          if ((abs(y2Max - y1Max)) < abs((x2Max - x1Max)))
            w1 = 1.0 * (x - x1Min) / (x1Max - x1Min);
          else
            w1 = 1.0 * (y - y1Min) / (y1Max - y1Min);
          w2 = 1 - w1;

          c.r = (byte) (round((w1 * c1.r)) + round((w2 * c2.r)));
          c.g = (byte) (round((w1 * c1.g)) + round((w2 * c2.g)));
          c.b = (byte) (round((w1 * c1.b)) + round((w2 * c2.b)));

          try { result->SetPixel(x, y, c); }
          catch (IndexOutOfBoundsException ex) {}
        }
      }
  }

  delete mi;
  delete overlapI1;
  delete overlapI2;

  result->SetName("output.ppm");
  return result;
}

/*  The registration algorithm
 *  using the Levenberg-Marquadt minimization algorithm 
 */
Matrix *Register(Picture *I1, Picture *I2, pointType InitialPoints[2][4])
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif

  Matrix u1(8, 8);
  Matrix u2(8, 1);
  Matrix m(8, 1);

  double x[4][2];
  double y[4][2];

  for (int i = 0; i < 4; i++) {
    x[i][0] = (double) InitialPoints[0][i].x;
    y[i][0] = (double) InitialPoints[0][i].y;
    x[i][1] = (double) InitialPoints[1][i].x;
    y[i][1] = (double) InitialPoints[1][i].y;
  }

  for (int i = 0; i < 4; i++) {
    u1.Set((2*i)+1, 1, x[i][0]);
    u1.Set((2*i)+1, 2, y[i][0]);
    u1.Set((2*i)+1, 3, 1);
    u1.Set((2*i)+1, 4, 0);
    u1.Set((2*i)+1, 5, 0);
    u1.Set((2*i)+1, 6, 0);
    u1.Set((2*i)+1, 7, -(x[i][0] * x[i][1]));
    u1.Set((2*i)+1, 8, -(y[i][0] * x[i][1]));

    u1.Set((2*i)+2, 1, 0);
    u1.Set((2*i)+2, 2, 0);
    u1.Set((2*i)+2, 3, 0);
    u1.Set((2*i)+2, 4, x[i][0]);
    u1.Set((2*i)+2, 5, y[i][0]);
    u1.Set((2*i)+2, 6, 1);
    u1.Set((2*i)+2, 7, -(x[i][0] * y[i][1]));
    u1.Set((2*i)+2, 8, -(y[i][0] * x[i][1]));

    u2.Set((2*i)+1, 1, x[i][1]);
    u2.Set((2*i)+2, 1, y[i][1]);
  }
  m = *u1.Inverse() * u2;
  cout << "Initial m = \n" << m << endl;

  Matrix pt(3, 1);
  Matrix m2(3, 3);
  m2.Set(1, 1, m.Get(1, 1));
  m2.Set(1, 2, m.Get(2, 1));
  m2.Set(1, 3, m.Get(3, 1));
  m2.Set(2, 1, m.Get(4, 1));
  m2.Set(2, 2, m.Get(5, 1));
  m2.Set(2, 3, m.Get(6, 1));
  m2.Set(3, 1, m.Get(7, 1));
  m2.Set(3, 2, m.Get(8, 1));
  m2.Set(3, 3, 1);

  for (int i = 0; i < 4; i++) {
    pt.Set(1, 1, x[i][0]);
    pt.Set(2, 1, y[i][0]);
    pt.Set(3, 1, 1);
    pt = m2 * pt;
    pt.Set(1, 1, pt.Get(1, 1) / pt.Get(3, 1));
    pt.Set(2, 1, pt.Get(2, 1) / pt.Get(3, 1));
    pt.Set(3, 1, 1);
  }

#ifndef BYPASS_MINIMIZATION
  Matrix A(8, 8);
  Matrix b(8, 1);
  Matrix dm(8, 1);
  double lambda = 0.0001;
  double eThreshold = 10;
  double E = 1000;
  double lastE = 0.0;
  double totalError = 0.0;
  int totalOverlap = 0;
  int iteration = 0;

  /* Continue iterating as long as the sum of the error in intensity
   * between corresponding pixels is greater than the threshold
   */
  while ((E > eThreshold) &&
         (iteration < MAXIMUM_ITERATIONS)) {
    double xp, yp, D;

    /* Initialize the 8x8 Hessian matrix, A,
     * and the weighted gradient vector b
     * to 0 in all cells
     */
    A.LoadZero();
    b.LoadZero();
    dm.LoadZero();
    totalError = 0.0;
    totalOverlap = 0;

    /* for each pixel i at location (x_i, y_i) */
    for (int y = 0; y < I1->GetHeight(); y++) {
      for (int x = 0; x < I1->GetWidth(); x++) {
        /* a) compute its corresponding position in the other image (x', y')
         *    using x' = (m_0x + m_1y + m_2) / D, y' = (m_3x + m_3y + m_5) / D
         *    where D = m_6x + m_7y +1
         */
        D = (m.Get(7, 1) * x) + (m.Get(8, 1) * y) + 1;
        xp = ((m.Get(1, 1) * x) + (m.Get(2, 1) * y) + m.Get(3, 1)) / D;
        yp = ((m.Get(4, 1) * x) + (m.Get(5, 1) * y) + m.Get(6, 1)) / D;

        /* we're only interested in those pixels that are within the other image */
        if (I2->Inside((int) floor(xp), (int) floor(yp)) &&
            I2->Inside((int) ceil(xp), (int) ceil(yp))) {
          totalOverlap++;

          /* b) compute the error in intensity between the corresponding pixels
           *    e = I'(x', y') - I(x, y)
           *    and intensity gradient (dI' / dx', dI' / dy')
           *    using bilinear interpolation on I
           */
          double I, Ip, err;
          double x_l = floor(xp);
          double x_r = x_l + 1;
          double y_d = floor(yp);
          double y_u = y_d + 1;

          double dx = xp - x_l;
          double dy = yp - y_d;
          double Iy_u, Iy_d, Ix_l, Ix_r;

          try {
            Iy_u = ((Intensity(I2->GetPixel((int) x_r, (int) y_u)) -
                     Intensity(I2->GetPixel((int) x_l, (int) y_u))) * dx) +
                   Intensity(I2->GetPixel((int) x_l, (int) y_u));

            Iy_d = ((Intensity(I2->GetPixel((int) x_r, (int) y_d)) -
                     Intensity(I2->GetPixel((int) x_l, (int) y_d))) * dx) +
                   Intensity(I2->GetPixel((int) x_l, (int) y_d));

            Ix_l = ((Intensity(I2->GetPixel((int) x_l, (int) y_u)) -
                     Intensity(I2->GetPixel((int) x_l, (int) y_d))) * dy) +
                   Intensity(I2->GetPixel((int) x_l, (int) y_d));

            Ix_r = ((Intensity(I2->GetPixel((int) x_r, (int) y_u)) -
                     Intensity(I2->GetPixel((int) x_r, (int) y_d))) * dy) +
                   Intensity(I2->GetPixel((int) x_r, (int) y_d));

            try {
              I = Intensity(I1->GetPixel(x, y));
              Ip = ((Iy_u - Iy_d) * dy) + Iy_d;
              err = Ip - I;
              totalError += (err * err);
            }
            catch (IndexOutOfBoundsException ex) {}
          }
          catch (IndexOutOfBoundsException ex) {}

          double dI_dxp = Ix_r - Ix_l;
          double dI_dyp = Iy_u - Iy_d;

          /* c) Compute the partial derivatives of e_i with respect to m_k using
           *    de / dm_k = ((dI' / dx') * (dx' / dm_k)) + ((dI' / dy') * (dy' / dm_k))
           */
          double de_dm[8];
          de_dm[0] = dI_dxp * x / D;
          de_dm[1] = dI_dxp * y / D;
          de_dm[2] = dI_dxp / D;
          de_dm[3] = dI_dyp * x / D;
          de_dm[4] = dI_dyp * y / D;
          de_dm[5] = dI_dyp / D;
          de_dm[6] = -(x / D) * ((dI_dxp * xp) + (dI_dyp * yp));
          de_dm[7] = -(y / D) * ((dI_dxp * xp) + (dI_dyp * yp));

          /* d) Add pixel's contribution to A and b using
           *    a_kl = summation((de / dm_k) * (de / dm_l))
           *    b_k = - summation(e * (de / dm_k))
           */
          for (int k = 0; k < 8; k++) {
            for (int l = 0; l < 8; l++)
              A.Set(k+1, l+1, A.Get(k+1, l+1) + (de_dm[k] * de_dm[l]));
            b.Set(k+1, 1, b.Get(k+1, 1) + (err * de_dm[k]));
          }
        }
      }
    }

    /* Divide the sum of the squared intensity error by the number of overlapping pixels */
    E = totalError / totalOverlap;
    cout << "Total error: " << E << ", overlapping pixels: " << totalOverlap << endl;

    /* 2) Solve the system of equations (A + lambda*I)dm = b,
     *    and update the motion estimate m^t+1 = m^t + dm
     *    therefore, dm = inv(A + lambda*I) * b
     */

    /* Normalize the matrices A, and b
     * by the number of pixels in the overlap region
     */
    for (int i = 1; i < 9; i++) {
      for (int j = 1; j < 9; j++)
        A.Set(i, j, A.Get(i, j) / totalOverlap);
      b.Set(i, 1, -1 *  b.Get(i, 1) / totalOverlap);
    }

    Matrix I(8, 8);
    I.LoadIdentity();
    dm = *(A + (I * lambda)).Inverse() * b;
    m += dm;

    /* 3) Check the total error in intensity between corresponding pixels
     *    had decreased, if not, increment lambda and compute a new dm
     */
    if (iteration > 0) {       /* if not first iteration */
      double diff = E - lastE;
      lastE = E;

      /* if error increased, increment lambda */
      if (diff > 0) {
        /* do not update m with the motion estimate */
        m -= dm;
        lambda = lambda * 10;
      }

      /* if error doesn't change much, stop iterating */
      else if (fabs(diff) < SMALLEST_ERROR_CHANGE) {
        cout << "No change in error, terminating loop" << endl;
        iteration = MAXIMUM_ITERATIONS;
      }

      /* if error decreased, decrement lambda */
      else {
        if (fabs(diff) < 5) {
          if (lambda < MAXIMUM_LAMBDA_VALUE)
            lambda = lambda * 10;
        }
        else
          if (lambda > MINIMUM_LAMBDA_VALUE)
            lambda = lambda / 10;
      }
    }
    else
      lastE = E;

    cout << "Iteration: " << iteration << ", lambda: " << lambda << endl;
    iteration++;
    cout << "m = \n" << m << endl;
  }
#endif

  Matrix *M = new Matrix(3, 3);
  M->Set(1, 1, m.Get(1, 1));
  M->Set(1, 2, m.Get(2, 1));
  M->Set(1, 3, m.Get(3, 1));
  M->Set(2, 1, m.Get(4, 1));
  M->Set(2, 2, m.Get(5, 1));
  M->Set(2, 3, m.Get(6, 1));
  M->Set(3, 1, m.Get(7, 1));
  M->Set(3, 2, m.Get(8, 1));
  M->Set(3, 3, 1);

  return M;
}

/* reduces an image, as described in the Burt-Adelson paper */
Picture *Reduce(Picture *src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  int Width = (src->GetWidth() / 2);
  int Height = (src->GetHeight() / 2);

  //cout << "Reducing image to " << Width << "x" << Height << endl;
  Picture *result = new Picture(Width, Height);

  /* weights as suggested in the Burt-Adelson's paper */
  double weight[5] = { 0.05, 0.25, 0.4, 0.25, 0.05 };

  intensityType g0, g1;

  for (int j = 0; j < Height; j++)
	for (int i = 0; i < Width; i++) {
	  memset((void *) &g1, 0, sizeof(intensityType));

	  for (int m = -2; m < 3; m++)
		for (int n = -2; n < 3; n++) {
		  int x = (2 * i) + m;
		  int y = (2 * j) + n;

		  /* for boundary conditions, use a reflection across the edge node */
		  if (x >= src->GetWidth())
			x = x - abs(Width - (x + 1));
		  if (y >= src->GetHeight())
			y = y - abs(src->GetHeight() - (y + 1));
		  if (x < 0)
			x = abs(x);
		  if (y < 0)
			y = abs(y);

		  int mp = m + 2;
		  int np = n + 2;
		  if (src->Inside(x, y)) {
			try {
			  g0 = src->GetPixelIntensity(x, y);
			  g1.r = (int) round((weight[mp] * weight[np] * g0.r) + g1.r);
			  g1.g = (int) round((weight[mp] * weight[np] * g0.g) + g1.g);
			  g1.b = (int) round((weight[mp] * weight[np] * g0.b) + g1.b);
			}
			catch (IndexOutOfBoundsException ex) {}
		  }
		}
	  result->SetPixelIntensity(i, j, g1);
	}

  return result;
}

/* expands an image, as described in the Burt-Adelson paper */
Picture *Expand(Picture *src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  int Width = (src->GetWidth()) * 2;
  int Height = (src->GetHeight()) * 2;

  /* weights as suggested in the Burt-Adelson's paper */
  double weight[5] = { 0.05, 0.25, 0.4, 0.25, 0.05 };

  cout << "Expanding image to " << Width << "x" << Height << endl;
  Picture *result = new Picture(Width, Height);

  intensityType g0, g1;

  for (int j = 0; j < Height; j++)
    for (int i = 0; i < Width; i++) {
      memset((void *) &g1, 0, sizeof(intensityType));

      for (int m = -1; m < 2; m++)
        for (int n = -1; n < 2; n++) {
          int x = (i + m) / 2;
          int y = (j + n) / 2;

          /* for boundary conditions, use a reflection across the edge node */
          if (x >= src->GetWidth())
            x = x - abs(Width - (x + 1));
          if (y >= src->GetHeight())
            y = y - abs(src->GetHeight() - (y + 1));
          if (x < 0)
            x = abs(x);
          if (y < 0)
            y = abs(y);

          int mp = (m + 2);
          int np = (n + 2);
          if (src->Inside(x, y)) {
            try {
              g0 = src->GetPixelIntensity(x, y);
              g1.r = (int) round(weight[mp] * weight[np] * g0.r) + g1.r;
              g1.g = (int) round(weight[mp] * weight[np] * g0.g) + g1.g;
              g1.b = (int) round(weight[mp] * weight[np] * g0.b) + g1.b;
            }
            catch (IndexOutOfBoundsException ex) {}
          }
        }
      result->SetPixelIntensity(i, j, g1);
    }

  return result;
}

/*  returns the Laplacian representation,
 *  given two input images, as described in
 *  the Burt-Adelson paper
 */
Picture *Laplacian(Picture *g1, Picture *g0)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  cout << "Deriving Laplacian image\n";
  int width = min(g1->GetWidth(), g0->GetWidth());
  int height = min(g1->GetHeight(), g0->GetHeight());

  Picture *result = new Picture(width, height);
  intensityType c0, c1, color;

  for (int j = 0; j < height; j++)
    for (int i = 0; i < width; i++) {
      try {
        c0 = g0->GetPixelIntensity(i, j);
        c1 = g1->GetPixelIntensity(i, j);
      }
      catch (IndexOutOfBoundsException ex) {}

      color.r = c0.r - c1.r;
      color.g = c0.g - c1.g;
      color.b = c0.b - c1.b;

      result->SetPixelIntensity(i, j, color);
    }
  return result;
}

/*  constructs a Gaussian pyramid, given an input image,
 *  as described in the Burt-Adelson paper
 */
pyramidType *GaussianPyramid(Picture *src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  pyramidType *result = new pyramidType;

  cout << "Computing Gaussian Pyramid\n";
  result->Images = new Picture[(int) ceil(log((double)(min(src->GetWidth(), src->GetHeight())))) + 2];
  result->Images[0] = *src;
  result->Levels = 1;

  result->Images[result->Levels] = *Reduce(src);
  while ((result->Images[result->Levels].GetWidth() > 8) &&
         (result->Images[result->Levels].GetHeight() > 8)) {
    result->Levels++;
    result->Images[result->Levels] = *Reduce(&result->Images[result->Levels - 1]);
  }
  result->Levels++;

  return result;
}

/* construct a scale map corresponding to some level in the 
 * pyramid, whose element indicates the local scale. This  
 * local scale is decided by the local maximum.
 */
Matrix *LocalScaleMap(pyramidType *pyramid, int level)
{
	Matrix *result = new Matrix(pyramid->Images[level].GetHeight(),
								pyramid->Images[level].GetWidth());	
	return result;
}

/*  constructs a Laplacian pyramid, given a Gaussian pyramid
 *  as described in the Burt-Adelson paper
 */
pyramidType *LaplacianPyramid(pyramidType *gaussianPyramid)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  cout << "Computing Laplacian Pyramid\n";
  pyramidType *result = new pyramidType;
  Matrix M(3, 3);
  M.Set(1, 1, 1);
  M.Set(1, 2, 0);
  M.Set(1, 3, 0);

  M.Set(2, 1, 0);
  M.Set(2, 2, 1);
  M.Set(2, 3, 0);

  M.Set(3, 1, 0);
  M.Set(3, 2, 0);
  M.Set(3, 3, 1);

  result->Images = new Picture[(int) ceil(log((double)(min(gaussianPyramid->Images[0].GetWidth(),
                                                  gaussianPyramid->Images[0].GetHeight())))) + 2];
  result->Images[0] = *Laplacian(Expand(&gaussianPyramid->Images[1]),
                                 &gaussianPyramid->Images[0]);
  M.Set(1, 1, 1.0 * gaussianPyramid->Images[0].GetWidth() / result->Images[0].GetWidth());
  M.Set(2, 2, 1.0 * gaussianPyramid->Images[0].GetHeight() / result->Images[0].GetHeight());

  /* warp the image to the right dimensions */
  if ((M.Get(1, 1) != 1) ||
      (M.Get(2, 2) != 1)) {
    result->Images[0].Warp(&M, true);
  }

  result->Levels = 1;
  for (int i = 2; i < gaussianPyramid->Levels; i++) {
    result->Images[i-1] = *Laplacian(Expand(&gaussianPyramid->Images[i]), &gaussianPyramid->Images[i-1]);
    M.Set(1, 1, 1.0 * gaussianPyramid->Images[i-1].GetWidth() / result->Images[i-1].GetWidth());
    M.Set(2, 2, 1.0 * gaussianPyramid->Images[i-1].GetHeight() / result->Images[i-1].GetHeight());

    /* warp the image to the right dimensions */
    if ((M.Get(1, 1) != 1) ||
        (M.Get(2, 2) != 1)) {
      result->Images[i-1].Warp(&M, true);
    }
    result->Levels++;
  }
  result->Images[gaussianPyramid->Levels - 1] = gaussianPyramid->Images[gaussianPyramid->Levels - 1];
  result->Levels++;

  return result;
}

/*  Collapses a Laplacian pyramid to reconstruct the
 *  original input image
 */
Picture *Collapse(pyramidType *LaplacianPyramid)
{
  cout << "Collapsing Laplacian Pyramid\n";
  Picture *result = NULL;
  Picture *temp = NULL;
  Matrix M(3, 3);
  M.Set(1, 1, 1);
  M.Set(1, 2, 0);
  M.Set(1, 3, 0);

  M.Set(2, 1, 0);
  M.Set(2, 2, 1);
  M.Set(2, 3, 0);

  M.Set(3, 1, 0);
  M.Set(3, 2, 0);
  M.Set(3, 3, 1);

  result = Expand(&LaplacianPyramid->Images[LaplacianPyramid->Levels - 1]);
  M.Set(1, 1, 1.0 * LaplacianPyramid->Images[LaplacianPyramid->Levels - 2].GetWidth() / result->GetWidth());
  M.Set(2, 2, 1.0 * LaplacianPyramid->Images[LaplacianPyramid->Levels - 2].GetHeight() / result->GetHeight());

  /* warp the image to the right dimensions */
  if ((M.Get(1, 1) != 1) ||
      (M.Get(2, 2) != 1))
    result->Warp(&M, true);
  *result = *result + LaplacianPyramid->Images[LaplacianPyramid->Levels - 2];

  for (int i = LaplacianPyramid->Levels - 3; i >= 0; i--) {
    try {
      temp = Expand(result);
      M.Set(1, 1, 1.0 * LaplacianPyramid->Images[i].GetWidth() / temp->GetWidth());
      M.Set(2, 2, 1.0 * LaplacianPyramid->Images[i].GetHeight() / temp->GetHeight());

      /* warp the image to the right dimensions */
      if ((M.Get(1, 1) != 1) ||
          (M.Get(2, 2) != 1))
        temp->Warp(&M, true);
      *temp = *temp + LaplacianPyramid->Images[i];
      *result = *temp;
    }
    catch (IncompatibleDimensionsException ex) {}
  }
  return result;
}

/*  Perform the Multiresolution splining method
 *  as described in the Burt-Adelson paper
 */
Picture *Combine(Picture *I1, Picture *I2,
                 pointType Max1, pointType Min1,
                 pointType Max2, pointType Min2,
                 pointType overlapMax, pointType overlapMin)
{
  pyramidType *LA = LaplacianPyramid(GaussianPyramid(I1));
  pyramidType *LB = LaplacianPyramid(GaussianPyramid(I2));
  pyramidType *LS = new pyramidType;

  pixelType c1, c2, c;
  bool I1left = Max2.x > Max1.x;
  bool I2left = Max1.x > Max2.x;
  bool I1top = Max1.y > Max2.y;
  bool I2top = Max2.y > Max1.y;
  int dx = abs(min((int) round(Max1.x), (int) round(Max2.x)) - max((int) round(Min1.x), (int) round(Min2.x)));
  int dy = abs(min((int) round(Max1.y), (int) round(Max2.y)) - max((int) round(Min1.y), (int) round(Min2.y)));

  if (I1left) cout << "I1 is at the left" << endl;
  if (I2left) cout << "I2 is at the left" << endl;
  if (I1top) cout << "I1 is at the top" << endl;
  if (I2top) cout << "I2 is at the top" << endl;

  cout << "Combining Laplacian Pyramids\n";

  LS->Images = new Picture[LA->Levels];
  LS->Levels = LA->Levels;
  for (int i = 0; i < LS->Levels; i++) {
    int width = LA->Images[i].GetWidth();
    int height = LA->Images[i].GetHeight();

    LS->Images[i] = LA->Images[i];
    LS->Images[i].Clear();

    for (int y = 0; y < height; y++)
      for (int x = 0; x < width; x++) {
        try { c1 = LA->Images[i].GetPixel(x, y); }
        catch (IndexOutOfBoundsException ex) {}

        try { c2 = LB->Images[i].GetPixel(x, y); }
        catch (IndexOutOfBoundsException ex) {}

        if ((c1.r == 0) && (c1.g == 0) && (c1.b == 0)) {	/* the first image doesn't overlap here, use the second */
          try { LS->Images[i].SetPixel(x, y, c2); }
          catch (IndexOutOfBoundsException ex) { cerr << "1:"; }
        }
        else if ((c2.r == 0) && (c2.g == 0) && (c2.b == 0)) {	/* the second image doesn't overlap here, use the first */
          try { LS->Images[i].SetPixel(x, y, c1); }
          catch (IndexOutOfBoundsException ex) { cerr << "2:";}
        }
        else {	/* we are at the overlapping region */
          double wx, wy;

          /*  blending from left to right, top to bottom,
           *  depending on which image is the left/top image
           */
          wx = -1;
          wy = -1;
          if (I1left)
            wx = (1.0 * abs((int) round(overlapMax.x) - x) / dx);
          if (I2left)
            wx = 1 - (1.0 * abs((int) round(overlapMax.x) - x) / dx);
          if (I1top)
            wy = 1.0 * abs((int) round(overlapMax.y) - y) / dy;
          if (I2top)
            wy = 1 - (1.0 * abs((int) round(overlapMax.y) - y) / dy);

          /* normalize the weights, if it exceeds the range -1.0 to 1.0 */
          if (wx > 1.0) wx = 1.0;
          if (wx < -1.0) wx = -1.0;
          if (wy > 1.0) wy = 1.0;
          if (wy < -1.0) wy = -1.0;

          if (wx == -1) {	/* if neither image is to the left of the other */
            c.r = (byte) ((wy * c1.r) + ((1-wy) * c2.r));
            c.g = (byte) ((wy * c1.g) + ((1-wy) * c2.g));
            c.b = (byte) ((wy * c1.b) + ((1-wy) * c2.b));
          }
          else if (wy == -1) {	/* if neither image is at the top of the other */
            c.r = (byte) ((wx * c1.r) + ((1-wx) * c2.r));
            c.g = (byte) ((wx * c1.g) + ((1-wx) * c2.g));
            c.b = (byte) ((wx * c1.b) + ((1-wx) * c2.b));
          }
          else {
            c.r = (byte) ((wx * wy * c1.r) + ((1-wx) * (1-wy) * c2.r));
            c.g = (byte) ((wx * wy * c1.g) + ((1-wx) * (1-wy) * c2.g));
            c.b = (byte) ((wx * wy * c1.b) + ((1-wx) * (1-wy) * c2.b));
          }

          try { LS->Images[i].SetPixel(x, y, c); }
          catch (IndexOutOfBoundsException ex) {cerr << "3:";}
        }
      }

    cout << "Summing up laplacian layer " << i << endl;
    overlapMax.x /= 2;
    overlapMax.y /= 2;
    overlapMin.x /= 2;
    overlapMin.y /= 2;
    dx /= 2;
    dy /= 2;
  }

  return Collapse(LS);
}

vector<string> Get_FrameNames(const char *foldername, const char *frame_ext)
{
	vector<string> frameNames;
	WIN32_FIND_DATAA fileData;

	string folderpath = (string)foldername + "*";
	HANDLE hFind = FindFirstFile(folderpath.c_str(), &fileData);
	bool bFinished = (hFind == INVALID_HANDLE_VALUE);
	int bRepeat = 1;	

	while (bRepeat && !bFinished) {
		bool bProcess = true;
		// If the current file is a directory, ignore
		if( (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			bProcess = false;
		}
		// If the current file is "." or "..", ignore
		if(strcmp(fileData.cFileName, ".") == 0 || 
			strcmp(fileData.cFileName, "..") == 0 ) {
			bProcess = false;	
		}
		// Obtain the file name
		// string filename fileData.cFileName;
		string cfilename = (string)fileData.cFileName;
		size_t dotidx = cfilename.find_last_of(".");
		string ext = cfilename.substr(dotidx+1,cfilename.length());
		if (ext != (string)frame_ext)
			bProcess = false;
		
		if (bProcess)
		{
			frameNames.push_back(cfilename);
		}

		bRepeat = FindNextFile(hFind, &fileData);
	} // end while

	return frameNames;
}

videoPyramidType *VideoPyramid(Video *src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  videoPyramidType *result = new videoPyramidType;

  cout << "Computing Gaussian Pyramid\n";
  double level = (double)(min(src->GetWidth(), src->GetHeight(), src->GetTime()));
  result->Videos = new Video[(int) ceil(log(level)) + 2];
  result->Videos[0] = *src;
  result->Levels = 1;

  result->Videos[result->Levels] = *ReduceVideo(src);
  while ((result->Videos[result->Levels].GetWidth() > 8) &&
         (result->Videos[result->Levels].GetHeight() > 8) &&
		 (result->Videos[result->Levels].GetTime() > 8)) {
    result->Levels++;
    result->Videos[result->Levels] = *ReduceVideo(&result->Videos[result->Levels - 1]);
  }
  result->Levels++;

  return result;
}

Video *ReduceVideo(Video *src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  Picture *frames = new Picture[src->GetTime()];

  for (int t = 0; t < src->GetTime(); t++)
  {
	  frames[t] = *(Reduce(src->GetFrame(t)));
  }

  Video *sresult = new Video(frames[0].GetWidth(),
							frames[0].GetHeight(),
							src->GetTime());
  sresult->SetFrames(frames,src->GetTime());

  return TemporalReduce(sresult);
}

Video *TemporalReduce(Video *src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
	int Time = (src->GetTime() / 2);

	cout << "Reducing video to " << src->GetWidth() << "x" 
			<< src->GetHeight() << endl;
	Video *result = new Video(src->GetWidth(), src->GetHeight(), Time);
	Picture *frames = new Picture[Time];
	for (int t = 0; t < Time; t++)
	{
		frames[t] = *(new Picture(src->GetWidth(),src->GetHeight()));
	}
	result->SetFrames(frames, Time);

	/* weights as suggested in the Burt-Adelson's paper */
	double weight[5] = { 0.05, 0.25, 0.4, 0.25, 0.05 };

	intensityType g0, g1;
	//pixelType p0, p1;

	for (int t = 0; t < Time; t++)
		for (int x = 0; x < src->GetWidth(); x++)
			for (int y = 0; y < src->GetHeight(); y++)
			{
				memset((void *) &g1, 0, sizeof(intensityType));
				//memset((void *) &p1, 0, sizeof(pixelType));

				for (int m = -2; m < 3; m++)
				{
					int tt = (2 * t) + m;

					/* for boundary conditions, use a reflection across 
					 the edge node */
					if (tt >= src->GetTime())
						tt = tt - abs(Time - (tt + 1));
					if (tt < 0)
						tt = abs(tt);

					int mp = m + 2;
					if (t<src->GetTime()) {
						try 
						{
							// intensity type
							g0 = src->GetPixelIntensity(x, y, tt);
							g1.r = (int) round((weight[mp] * g0.r) + g1.r);
							g1.g = (int) round((weight[mp] * g0.g) + g1.g);
							g1.b = (int) round((weight[mp] * g0.b) + g1.b);

							// pixel type
							//p0 = src->GetPixel(x, y, tt);
							//p1.r = (int) round((weight[mp] * p0.r) + p1.r);
							//p1.g = (int) round((weight[mp] * p0.g) + p1.g);
							//p1.b = (int) round((weight[mp] * p0.b) + p1.b);
						}
						catch (IndexOutOfBoundsException ex) {}
					}// end if
				}// end for m

				result->SetPixelIntensity(x,y,t,g1);
				//result->SetPixel(x,y,t,p1);
			}

	return result;
}

int *CalcMotionComponent(gradient3D *gradient, int source_time, 
						 int target_time, double &aveMotion)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
	int *keyframes = new int[source_time];
	for (int i = 0; i < source_time; i++)
		keyframes[i] = 0;

	int kf_exist = 1;
	int left_time = target_time;
	while (kf_exist>0)
	{
		aveMotion = 0.0;
		for (int t = 0; t < source_time; t++)
		{
			if (keyframes[t]==0)
				aveMotion += gradient->total_dt[t];
		}
		aveMotion = aveMotion/left_time;

		kf_exist = 0;
		for (int t = 0; t < source_time; t++)
		{
			if (gradient->total_dt[t]>aveMotion &&
				keyframes[t]==0)
			{
				keyframes[t] = 1;			
				kf_exist++;
				left_time--;
			}
		}
	}

	return keyframes;
}

Picture *InterpolateFrame(Picture *left, double lweight, Picture *right, double rweight)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	Picture *result = new Picture(left->GetWidth(),left->GetHeight());

	if ((left->GetHeight()!=right->GetHeight()) || 
		(left->GetWidth()!=right->GetWidth()))
		throw IncompatibleDimensionsException("Video", "InterpolateFrame");
	else {		
		for( int i = 0; i < left->GetHeight(); i++ )
		{
			for( int j = 0; j < left->GetWidth(); j++ )
			{
				pixelType value;
				pixelType lvalue = left->GetPixel(j,i);
				pixelType rvalue = right->GetPixel(j,i);
				value.r = lweight*lvalue.r+rweight*rvalue.r;
				value.r = min(floor(value.r+0.5),255);
				value.g = lweight*lvalue.g+rweight*rvalue.g;
				value.g = min(floor(value.g+0.5),255);
				value.b = lweight*lvalue.b+rweight*rvalue.b;
				value.b = min(floor(value.b+0.5),255);
				result->SetPixel(j,i,value);
			}
		}
	}

	return result;
}


PictureList *ReduceList(PictureList *src)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	Picture *list = new Picture[src->GetLength()];

	int minWidth = -1; int minHeight = -1;
	int maxWidth = -1; int maxHeight = -1;
	for (int l = 0; l < src->GetLength(); l++)
	{
		if (src->GetPicture(l)->GetWidth()>8 &&
			src->GetPicture(l)->GetHeight()>8)
		{
			Picture *downscaled = Reduce(src->GetPicture(l));
			list[l] = *(downscaled);
			delete downscaled;
			list[l].SetName(src->GetPicture(l)->GetName());
			minWidth = ((list[l].GetWidth()<minWidth) || 
						(minWidth==-1)) ? 
						list[l].GetWidth() : minWidth;
			maxWidth = ((list[l].GetWidth()>maxWidth) || 
						(maxWidth==-1)) ? 
						list[l].GetWidth() : maxWidth;
			minHeight = ((list[l].GetHeight()<minHeight) ||
						 (minHeight==-1)) ? 
						list[l].GetHeight() : minHeight;
			maxHeight = ((list[l].GetHeight()>maxHeight) ||
						 (maxHeight==-1)) ? 
						list[l].GetHeight() : maxHeight;
		}		
		else
		{
			// throw exception
		}
	}

	PictureList *result = new PictureList(minWidth,minHeight,src->GetLength());
	result->SetName(src->GetName());
	result->SetList(list,src->GetLength());
	result->SetMinWidth(minWidth);
	result->SetMaxWidth(maxWidth);
	result->SetMinHeight(minHeight);
	result->SetMaxHeight(maxHeight);

	return result;
}

listPyramidType *ListPyramid(PictureList *src, int levels)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	listPyramidType *result = new listPyramidType;

	cout << "Computing Gaussian Pyramid for picture list \n";
	//result->Lists = new PictureList[(int) ceil(log((double)levels)) + 2];
	result->Lists = new PictureList[levels];
	result->Lists[0] = *src;
	result->Levels = 1;

	for (int l = 1; l < levels; l++)
	{
		PictureList *list = ReduceList(&result->Lists[result->Levels - 1]);
		result->Lists[result->Levels] = *list;
		result->Levels++;
		delete list;
	}

  return result;
}

/*
 *
 */
int DownsamplingIndex(int p, imageSize &target_size, imageSize &previous_size, double ratio)
{
	int x = floor((double)(p % target_size.width)/ratio);
	if (x==previous_size.width)
		x--;
	int y = floor((double)(p / target_size.width)/ratio);
	if (y==previous_size.height)
		y--;

	int idx = y*previous_size.width+x;
	if (idx<0)
		idx++;
	if (idx==previous_size.width*
			 previous_size.height)
		idx--;

	return idx;
}

int Downsampling3DIndex(int p, videoSize &target_size, videoSize &previous_size, double ratio)
{
	int s = floor((double)(p % (target_size.width*target_size.height)));
	int t = floor((double)(p / (target_size.width*target_size.height)));
	if (t>=previous_size.time)
		t = previous_size.time-1;
	int x = floor((double)(s % target_size.width)/ratio);
	if (x>=previous_size.width)
		x = previous_size.width-1;
	int y = floor((double)(s / target_size.width)/ratio);
	if (y>=previous_size.height)
		y = previous_size.height-1;

	int idx = t*(previous_size.width*previous_size.height)
				+y*previous_size.width+x;
	if (idx<0)
		idx++;
	if (idx>=previous_size.width*
			 previous_size.height*
			 previous_size.time)
		idx = previous_size.width*
			  previous_size.height*
			  previous_size.time-1;

	return idx;
}

/*
 *
 */
int *SimpleUpsamplingMap(int *result, imageSize size, double ratio)
{
	imageSize up_size;
	up_size.width = round(size.width*ratio);
	up_size.height = round(size.height*ratio);
	int *up_result = new int[up_size.width*up_size.height];

	for (int y = 0; y < up_size.height; y++)
	{
		for (int x = 0; x < up_size.width; x++)
		{
			int p = y*up_size.width+x;
			int idx = DownsamplingIndex(p,up_size,size,ratio);
			int label = ceil((double)result[idx]*ratio);
			up_result[p] = label;
		} // end for x
	} // end for y

	return up_result;
}

/*
 *
 */
int *SimpleUpsamplingMapList(int *result, videoSize size, double ratio)
{
	videoSize up_size;
	up_size.width = round(size.width*ratio);
	up_size.height = round(size.height*ratio);
	up_size.time = size.time;
	int *up_result = new int[up_size.width*up_size.height*up_size.time];

	for (int t = 0; t < up_size.time; t++)
	{
		for (int y = 0; y < up_size.height; y++)
		{
			for (int x = 0; x < up_size.width; x++)
			{
				int p = t*up_size.width*up_size.height+y*up_size.width+x;
				int idx = Downsampling3DIndex(p,up_size,size,ratio);
				int label = ceil((double)result[idx]*ratio);
				up_result[p] = label;
			} // end for x
		} // end for y
	}

	return up_result;
}

/*
 *
 */
int *JointBilateralUpsampling(int *result, int s_width, int s_height, 
							  int s_time, PictureList *ref, double ratio)
{
	int mu = 0;
	float sigma = 0.5;
	ratio = 1;
	
	int width = ref->GetMaxWidth();
	int height = ref->GetMaxHeight();
	int time = ref->GetLength();
	int *up_result = new int[time*width*height];

	for (int t = 0; t < time; t++)
	{
		for (int y = 0; y < height; y++)
		{
			int refPixLoc = t*(width*height)+y*width;
			for (int x = 0; x < width; x++)
			{	
				refPixLoc += x;
				pixelType refPix = ref->GetPicture(t)->GetPixel(x,y);
				
				float total_val = 0;
				float normalizing_factor = 0;
			
				//prevent black areas fro all rgausses being 0
				float norgauss = 0; 
				float norgauss_normalize = 0;

				/* coordinates in the source */
				float o_x = x/ratio;
				float o_y = y/ratio;

				for (int nn_y = -ratio-1; nn_y < ratio; nn_y++)
				{
					int r_y = (int)round(o_y + nn_y);
					r_y = (r_y > 0 ? (r_y < s_height ? r_y : s_height-1) : 0);
					int srcPixLoc = t*(s_width*s_height)+r_y*s_width;
					int neighborPixLoc = t*(width*height)+r_y*ratio*width;

					for (int nn_x = -ratio-1; nn_x < ratio; nn_x++)
					{
						int r_x = (int)round(o_x + nn_x);
						r_x = (r_x > 0 ? (r_x < s_width ? r_x : s_width-1) : 0);
						int srcPix = result[srcPixLoc+r_x];
						
						neighborPixLoc += r_x*ratio;
						pixelType neighborPix = ref->GetPicture(t)->GetPixel(r_x*ratio,r_y*ratio);

						//gauss dist to center
						float sdist = sqrt(pow(o_x-r_x, 2) + pow(o_y-r_y, 2));
						float sgauss = simpleGauss(sdist,sigma, mu);
						//gauss radiance diff to center in ref
						float rdist = sqrt(pow(refPix.r-neighborPix.r, 2.0)+
									 pow(refPix.g-neighborPix.g, 2.0)+
									 pow(refPix.b-neighborPix.b, 2.0));
						float rgauss = simpleGauss(rdist,sigma, mu);

						//multiply gausses by value in source and add to total val
						norgauss = srcPix * sgauss;
						norgauss_normalize += sgauss;
						float totalgauss = sgauss * rgauss;
						normalizing_factor += totalgauss;
						total_val += srcPix * totalgauss;						
					}
				}

				if(total_val) {
					total_val /= normalizing_factor;
					up_result[refPixLoc] = total_val;
				}
				else {
					total_val = norgauss/norgauss_normalize;
				}
			
			}
		}
	}

	return up_result;
}

/* reduces an image, as described in the Burt-Adelson paper */
Matrix *ReduceMatrix(Matrix *src)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	int Cols = (src->NumOfCols() / 2);
	int Rows = (src->NumOfRows() / 2);

	cout << "Reducing matrix to " << Rows << "x" << Cols << endl;
	Matrix *result = new Matrix(Rows,Cols);

	/* weights as suggested in the Burt-Adelson's paper */
	double weight[5] = { 0.05, 0.25, 0.4, 0.25, 0.05 };


	for (int i = 1; i <= Rows; i++)
		for (int j = 1; j <= Cols; j++)
		{
			double val = 0.0;
			for (int m = -2; m < 3; m++)
				for (int n = -2; n < 3; n++)
				{
					int y = (2 * i) + m;
					int x = (2 * j) + n;

					/* for boundary conditions, use a reflection across the edge node */
					if (y > src->NumOfRows())
						y = y-abs(src->NumOfRows()-y)-1;
					if (x > src->NumOfCols())
						x = x-abs(src->NumOfCols()-x)-1;
					if (x <= 1)
						x = abs(x-1)+1;
					if (y <= 1)
						y = abs(y-1)+1;

					int mp = m + 2;
					int np = n + 2;
					try {
						val += weight[mp]*weight[np]*src->Get(y,x);
					}
					catch (IndexOutOfBoundsException ex) {}
				}		
		
			result->Set(i,j,val);
		}

	return result;
}

/*
 * spatial gradient for every frame for seam carving
 */
Matrix *Gradient_xy(PictureList *src)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	Matrix *dx = new Matrix[src->GetLength()];	// 0 rows and 0 cols of matrix dx. dx has an array size of src->GetTime()
	Matrix *dy = new Matrix[src->GetLength()];
	Matrix *dxy = new Matrix[src->GetLength()];

	gradient2D *frame_gradient = NULL;
	for (int t=0; t<src->GetLength(); t++)
	{
		if (frame_gradient)
			delete frame_gradient;
		frame_gradient = Gradient(src->GetPicture(t));			//wj find 2D gradient for frame t
		dx[t] = *(frame_gradient->dx);
		dy[t] = *(frame_gradient->dy);
		dxy[t] = dx[t] + dy[t];			// combine dx[t] and dy[t]
	}
	
	return dxy;
}

/*
 * convert the image from RGB to gray level
 */
Matrix *Rgb2Gray(Picture *src)
{
	Matrix *result = new Matrix(src->GetHeight(),src->GetWidth());
	double red; double green; double blue; int value;

	for( int i = 0; i < src->GetHeight(); i++ )
	{
		for( int j = 0; j < src->GetWidth(); j++ )
		{
			red = src->GetPixelIntensity(j, i).r;
			green = src->GetPixelIntensity(j, i).g;
			blue = src->GetPixelIntensity(j, i).b;

			//0.2125R+0.7154G+0.0721B
			value = (0.2125*red) + (0.7154*green) + (0.0721*blue);
			result->Set(i+1,j+1,value);

//			printf("(%d,%d) r=%f g=%f b=%f	v=%d\n", j, i, red, green, blue, value);
		}
	}
	return result;
}

/*
 * 
 */
gradient2D_L1 *Gradient2D_L1(Picture *src)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif

	Matrix *grayscale = new Matrix(src->GetHeight(),src->GetWidth());
	
	grayscale = Rgb2Gray(src);

	Matrix *dx = new Matrix(src->GetHeight(),src->GetWidth());
	Matrix *dy = new Matrix(src->GetHeight(),src->GetWidth());
	int diff;

	for( int i = 1; i <= grayscale->NumOfRows(); i++ )
	{
		for( int j = 1; j <= grayscale->NumOfCols(); j++ )
		{
			if ( j==grayscale->NumOfCols() )
				diff = abs(grayscale->Get(i,j) - grayscale->Get(i,1));
			else
				diff = abs(grayscale->Get(i,j) - grayscale->Get(i,j+1));
			dx->Set(i,j,diff);

			if ( i==grayscale->NumOfRows() )
				diff = abs(grayscale->Get(i,j) - grayscale->Get(1,j));
			else
				diff = abs(grayscale->Get(i,j) - grayscale->Get(i+1,j));
	
			dy->Set(i,j,diff);
		}
	}
	
	gradient2D_L1 *result = new gradient2D_L1;
	result->dx = dx;
	result->dy = dy;
	
	delete grayscale;

	return result;
}

/*
 * Calculate 2D forward energy for seam carving
 */
gradient2D_FE *Gradient2D_FE(Picture *src)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif

	Matrix *grayscale = new Matrix(src->GetHeight(),src->GetWidth());
	
	grayscale = Rgb2Gray(src);

	Matrix *LR = new Matrix(src->GetHeight(),src->GetWidth());
	Matrix *pLU = new Matrix(src->GetHeight(),src->GetWidth());
	Matrix *nLU = new Matrix(src->GetHeight(),src->GetWidth());
	int value;

	for( int i = 1; i <= src->GetHeight(); i++ )
	{
		for( int j = 1; j <= src->GetWidth(); j++ )
		{
			if ( j==1 )
				value = abs(grayscale->Get(i,grayscale->NumOfCols()) - grayscale->Get(i,j+1));
			else if ( j==grayscale->NumOfCols() )
				value = abs(grayscale->Get(i,j-1) - grayscale->Get(i,1));
			else
				value = abs(grayscale->Get(i,j-1) - grayscale->Get(i,j+1));
	
			LR->Set(i,j,value);

			if ( i==1 && j==1 )
				value = abs(grayscale->Get(i,grayscale->NumOfCols()) - grayscale->Get(grayscale->NumOfRows(),j));
			else if ( i>1 && j==1 )
				value = abs(grayscale->Get(i,grayscale->NumOfCols()) - grayscale->Get(i-1,j));
			else if ( i==1 && j>1 )
				value = abs(grayscale->Get(i,j-1) - grayscale->Get(grayscale->NumOfRows(),j));
			else
				value = abs(grayscale->Get(i,j-1) - grayscale->Get(i-1,j));
	
			pLU->Set(i,j,value);

			if ( i==grayscale->NumOfRows() && j==1 )
				value = abs(grayscale->Get(i,grayscale->NumOfCols()) - grayscale->Get(1,j));
			else if ( i<grayscale->NumOfRows() && j==1 )
				value = abs(grayscale->Get(i,grayscale->NumOfCols()) - grayscale->Get(i+1,j));
			else if ( i==grayscale->NumOfRows() && j>1 )
				value = abs(grayscale->Get(i,j-1) - grayscale->Get(1,j));
			else
				value = abs(grayscale->Get(i,j-1) - grayscale->Get(i+1,j));
			
			nLU->Set(i,j,value);
		}
	}
	
	gradient2D_FE *result = new gradient2D_FE;
	result->LR = LR;
	result->pLU = pLU;
	result->nLU = nLU;

	delete grayscale;
	
	return result;
}

/*
 * Calculate 3D Forward Energy for seam carving
 */
gradient3D_FE *Gradient3D_FE(PictureList *src)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif

	Matrix *LR = new Matrix[src->GetLength()];
	Matrix *pLU = new Matrix[src->GetLength()];
	Matrix *nLU = new Matrix[src->GetLength()];
	Matrix *temp_pLU = new Matrix[src->GetPicture(0)->GetHeight()];
	Matrix *temp_nLU = new Matrix[src->GetPicture(0)->GetHeight()];

	Matrix *grayscale = new Matrix[src->GetLength()];

	int value;

	for (int t=0; t<src->GetLength(); t++)
		grayscale[t] = *(Rgb2Gray(src->GetPicture(t)));
	
	for (int t=0; t<src->GetLength(); t++)
	{
		LR[t] = *(new Matrix(src->GetPicture(0)->GetHeight(),src->GetPicture(0)->GetWidth()));
		pLU[t] = *(new Matrix(src->GetPicture(0)->GetHeight(),src->GetPicture(0)->GetWidth()));
		nLU[t] = *(new Matrix(src->GetPicture(0)->GetHeight(),src->GetPicture(0)->GetWidth()));
	}

	for (int h=0; h<src->GetPicture(0)->GetHeight(); h++)
	{
		temp_pLU[h] = *(new Matrix(src->GetLength(),src->GetPicture(0)->GetWidth()));
		temp_nLU[h] = *(new Matrix(src->GetLength(),src->GetPicture(0)->GetWidth()));
	}

	/* 
	For pixels on the image boundaries, we use backward energy to calculate the cost for removing that pixel.
	Because pixels on the boundary are missing the necessary neighbours to calculate the forward energy.
	Moreover, removing pixels from the image boundaries do not create visual artifacts in the image, thus
	we do not need to calculate the forward energy.
	The backward energy is the L1 norm gradient.
	*/
	for (int t=0; t<src->GetLength(); t++)
	{
		for( int i = 1; i <= src->GetPicture(0)->GetHeight(); i++ )
		{
			for( int j = 1; j <= src->GetPicture(0)->GetWidth(); j++ )
			{
				if ( j==1 )								// Boundary pixel
					value = abs(grayscale[t].Get(i,j) - grayscale[t].Get(i,j+1));	// Backward energy
					//Using Forward Energy
					//value = abs(grayscale[t].Get(i,grayscale[t].NumOfCols()) - grayscale[t].Get(i,j+1));
				else if ( j==grayscale[t].NumOfCols() )	// Boundary pixel
					value = abs(grayscale[t].Get(i,j-1) - grayscale[t].Get(i,j));	// Backward energy
					//Using Forward Energy
					//value = abs(grayscale[t].Get(i,j-1) - grayscale[t].Get(i,1));
				else
					value = abs(grayscale[t].Get(i,j-1) - grayscale[t].Get(i,j+1));
				//printf("LR[%d].Set(%d,%d,%d)\n",t,i,j,value);
				LR[t].Set(i,j,value);

				if ( i==1 && j==1 )			// Boundary pixel
					value = abs(grayscale[t].Get(i,j) - grayscale[t].Get(grayscale[t].NumOfRows(),j));	// Backward energy
					//Using Forward Energy
					//value = abs(grayscale[t].Get(i,grayscale[t].NumOfCols()) - grayscale[t].Get(grayscale[t].NumOfRows(),j)); 
				else if ( i>1 && j==1 )		// Boundary pixel
					value = abs(grayscale[t].Get(i,j) - grayscale[t].Get(i-1,j));	// Backward energy
					//Using Forward Energy
					//value = abs(grayscale[t].Get(i,grayscale[t].NumOfCols()) - grayscale[t].Get(i-1,j));
				else if ( i==1 && j>1 )		// Boundary pixel
					value = abs(grayscale[t].Get(i,j) - grayscale[t].Get(grayscale[t].NumOfRows(),j));	// Backward energy
					//Using Forward Energy
					//value = abs(grayscale[t].Get(i,j-1) - grayscale[t].Get(grayscale[t].NumOfRows(),j));
				else
					value = abs(grayscale[t].Get(i,j-1) - grayscale[t].Get(i-1,j));
				
				//printf("pLU[%d].Set(%d,%d,%d)\n",t,i,j,value);
				pLU[t].Set(i,j,value);

				if ( i==grayscale[t].NumOfRows() && j==1 )			// Boundary pixel
					value = abs(grayscale[t].Get(i,j) - grayscale[t].Get(1,j));			// Backward energy
					//Using Forward Energy
					//value = abs(grayscale[t].Get(i,grayscale[t].NumOfCols()) - grayscale[t].Get(1,j));
				else if ( i<grayscale[t].NumOfRows() && j==1 )		// Boundary pixel
					value = abs(grayscale[t].Get(i,j) - grayscale[t].Get(i+1,j));		// Backward energy
					//Using Forward Energy
					//value = abs(grayscale[t].Get(i,grayscale[t].NumOfCols()) - grayscale[t].Get(i+1,j));
				else if ( i==grayscale[t].NumOfRows() && j>1 )		// Boundary pixel
					value = abs(grayscale[t].Get(i,j) - grayscale[t].Get(1,j));			// Backward energy
					//Using Forward Energy
					//value = abs(grayscale[t].Get(i,j-1) - grayscale[t].Get(1,j));
				else
					value = abs(grayscale[t].Get(i,j-1) - grayscale[t].Get(i+1,j));
				
				//printf("nLU[%d].Set(%d,%d,%d)\n",t,i,j,value);
				nLU[t].Set(i,j,value);

				if ( t==0 && j==1 )				// Boundary pixel
					value = abs(grayscale[t].Get(i,j) - grayscale[src->GetLength()-1].Get(i,j));		// Backward energy
					//Using Forward Energy
					//value = abs(grayscale[t].Get(i,grayscale[t].NumOfCols()) - grayscale[src->GetTime()-1].Get(i,j));
				else if ( t>0 && j==1 )			// Boundary pixel
					value = abs(grayscale[t].Get(i,j) - grayscale[t-1].Get(i,j));			// Backward energy
					//Using Forward Energy
					//value = abs(grayscale[t].Get(i,grayscale[t].NumOfCols()) - grayscale[t-1].Get(i,j));
				else if ( t==0 && j>1 )			// Boundary pixel
					value = abs(grayscale[t].Get(i,j) - grayscale[src->GetLength()-1].Get(i,j));		// Backward energy
					//Using Forward Energy
					//value = abs(grayscale[t].Get(i,j-1) - grayscale[src->GetTime()-1].Get(i,j));
				else
					value = abs(grayscale[t].Get(i,j-1) - grayscale[t-1].Get(i,j));
				
				//printf("temp_pLU[%d].Set(%d,%d,%d)\n",i-1,i,j,value);
				temp_pLU[i-1].Set(t+1,j,value);

				if ( t==src->GetLength()-1 && j==1 )			// Boundary pixel
					value = abs(grayscale[t].Get(i,j) - grayscale[0].Get(i,j));			// Backward energy
					//Using Forward Energy
					//value = abs(grayscale[t].Get(i,grayscale[t].NumOfCols()) - grayscale[0].Get(i,j));
				else if ( t<src->GetLength()-1 && j==1 )		// Boundary pixel
					value = abs(grayscale[t].Get(i,j) - grayscale[t+1].Get(i,j));		// Backward energy
					//Using Forward Energy
					//value = abs(grayscale[t].Get(i,grayscale[t].NumOfCols()) - grayscale[t+1].Get(i,j));
				else if ( t==src->GetLength()-1 && j>1 )		// Boundary pixel
					value = abs(grayscale[t].Get(i,j) - grayscale[0].Get(i,j));			// Backward energy
					//Using Forward Energy
					//value = abs(grayscale[t].Get(i,j-1) - grayscale[0].Get(i,j));
				else
					value = abs(grayscale[t].Get(i,j-1) - grayscale[t+1].Get(i,j));
				
				//printf("temp_nLU[%d].Set(%d,%d,%d)\n",i-1,i,j,value);
				temp_nLU[i-1].Set(t+1,j,value);
			}
		}
	}

	gradient3D_FE *result = new gradient3D_FE;
	result->LR = LR;
	result->pLU = pLU;
	result->nLU = nLU;
	result->temp_pLU = temp_pLU;
	result->temp_nLU = temp_nLU;

	delete [] grayscale;

	return result;
}