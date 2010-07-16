#include "StdAfx.h"
#include "ShiftMapComputer.h"

ShiftMapComputer::ShiftMapComputer(void)
{
}

ShiftMapComputer::~ShiftMapComputer(void)
{
}


IplImage* ShiftMapComputer::GetImage(IplImage* input, IplImage* saliency, int width, int height)
{		
	IplImage* output = cvCreateImage(cvSize(width, height), input->depth, input->nChannels);
	try{
		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(width, height, input->width * input->height);

		// set up the needed data to pass to function for the data costs
		ForDataFunction dataFn;
		dataFn.inHeight = input->height;
		dataFn.inWidth = input->width;
		dataFn.outHeight = height;
		dataFn.outWidth = width;			
		dataFn.saliency = saliency;
		 gc->setDataCost(&dataFunctionShiftmap,&dataFn);
		
		// smoothness comes from function pointer
		ForSmoothFunction smoothFn;
		smoothFn.inHeight = input->height;
		smoothFn.inWidth = input->width;
		smoothFn.outHeight = height;
		smoothFn.outWidth = width;	
		smoothFn.image = input;
		smoothFn.gradient = cvCloneImage(input);
		cvSobel(input, smoothFn.gradient, 1, 1);	 
		 gc->setSmoothCost(&smoothFunctionShiftmap, &smoothFn);

		 long test = gc->compute_energy();
		printf("\nBefore optimization energy is %d \n",gc->compute_energy());
		//gc->swap(20);
		gc->expansion(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("\nAfter optimization energy is %d \n",gc->compute_energy());
 
		GetRetargetImage(gc, input, output);

		delete gc;
	}
	catch (GCException e){
		e.Report();
	}
	return output;
}

void ShiftMapComputer::GetRetargetImage(GCoptimizationGridGraph * gc, IplImage* input, IplImage* output)
{
	int num_pixels = output->width * output->height;

	for(int i = 0; i < num_pixels; i++)
	{
		int label = gc->whatLabel(i);
		CvPoint point = GetPoint(i, output->width, output->height);
		CvPoint pointLabel = GetPoint(label, input->width, input->height);
		CvScalar value = cvGet2D(input, pointLabel.y, pointLabel.x);
		cvSet2D(output, point.y, point.x, value);
		// printf("*%i %i %i %i", point.x, point.y, pointLabel.x, pointLabel.y);
	}
}
