#include "StdAfx.h"
#include "ShiftMapComputer.h"

ShiftMapComputer::ShiftMapComputer( )
{
 
}

ShiftMapComputer::~ShiftMapComputer(void)
{
	delete _gc;
}
 

void ShiftMapComputer::ComputeShiftMap(IplImage* input, IplImage* saliency, CvSize output, CvSize shiftSize)
{
	try{
		_inputSize.width = input->width;
		_inputSize.height = input->height;
		_outputSize = output;
		_shiftSize = shiftSize;
		_input = input;
		
		_gc = new GCoptimizationGridGraph(_inputSize.width, _inputSize.height, _shiftSize.width * _shiftSize.height);

		// set up the needed data to pass to function for the data costs
		ForDataFunction dataFn;
		dataFn.inHeight = _inputSize.height;
		dataFn.inWidth = _inputSize.width;
		dataFn.outHeight = _outputSize.width;
		dataFn.outWidth = _outputSize.height;
		dataFn.shiftWidth = _shiftSize.width;
		dataFn.shiftHeight = _shiftSize.height;
		dataFn.saliency = saliency;
		_gc->setDataCost(&dataFunctionShiftmap,&dataFn);
		
		// smoothness comes from function pointer
		ForSmoothFunction smoothFn;
		smoothFn.inHeight = _inputSize.height;
		smoothFn.inWidth = _inputSize.width;
		smoothFn.outHeight = _outputSize.height;
		smoothFn.outWidth = _outputSize.width;
		smoothFn.shiftHeight = _shiftSize.height;
		smoothFn.shiftWidth = _shiftSize.width;
		smoothFn.image = input;
		smoothFn.gradient = cvCloneImage(input);
		cvSobel(input, smoothFn.gradient, 1, 1);	 
		_gc->setSmoothCost(&smoothFunctionShiftmap, &smoothFn);
		
		printf("\nBefore optimization energy is %d \n", _gc->compute_energy());
		//gc->swap(20);
		_gc->expansion(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("\nAfter optimization energy is %d \n", _gc->compute_energy()); 
	}
	catch (GCException e){
		e.Report();
	}
}

IplImage* ShiftMapComputer::GetRetargetImage()
{
	IplImage* output = cvCreateImage(_outputSize, _input->depth, _input->nChannels);
	int num_pixels = _outputSize.width * _outputSize.height;

	printf("Rendering graph-cut result to image... \n");
	for(int i = 0; i < num_pixels; i++)
	{
		int label = _gc->whatLabel(i);
		CvPoint point = GetPoint(i, _outputSize.width, _outputSize.height);
		CvPoint pointLabel = GetMappedPoint(point, label, _shiftSize.width, _shiftSize.height);
		//printf("*%i %i %i %i", point.x, point.y, pointLabel.x, pointLabel.y);
		if(!IsOutside(pointLabel, _inputSize.width, _inputSize.height))
		{
			CvScalar value = cvGet2D(_input, pointLabel.y, pointLabel.x);
			cvSet2D(output, point.y, point.x, value);
			//printf("*%i %i %i %i", point.x, point.y, pointLabel.x, pointLabel.y);
		}
		else
			cvSet2D(output, point.y, point.x, cvScalar(255, 0, 0));
	}
	return output;
}



//void GetRetargetImageInitialGuess(IplImage* input, IplImage* saliency, int width, int height, IplImage* initialGuess)
//{
//IplImage* output = cvCreateImage(cvSize(width, height), input->depth, input->nChannels);
//	try{
//		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(width, height, 3 * 3);
//
//		// set up the needed data to pass to function for the data costs
//		ForDataFunctionInitialGuess dataFn;
//		dataFn.inHeight = input->height;
//		dataFn.inWidth = input->width;
//		dataFn.outHeight = height;
//		dataFn.outWidth = width;			
//		dataFn.saliency = saliency;
//		dataFn.initialGuess = initialGuess;
//		gc->setDataCost(&dataFunctionInitialGuess,&dataFn);
//		
//		// smoothness comes from function pointer
//		ForSmoothFunctionInitialGuess smoothFn;
//		smoothFn.inHeight = input->height;
//		smoothFn.inWidth = input->width;
//		smoothFn.outHeight = height;
//		smoothFn.outWidth = width;	
//		smoothFn.image = input;
//		smoothFn.gradient = cvCloneImage(input);
//		smoothFn.initialGuess = initialGuess;
//		cvSobel(input, smoothFn.gradient, 1, 1);	 
//		gc->setSmoothCost(&smoothFunctionInitialGuess, &smoothFn);
//
//		long test = gc->compute_energy();
//		printf("\nBefore optimization energy is %d \n",gc->compute_energy());
//		//gc->swap(20);
//		gc->expansion(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
//		printf("\nAfter optimization energy is %d \n",gc->compute_energy());
// 
//		Get(gc, input, output);
//
//		delete gc;
//	}
//	catch (GCException e){
//		e.Report();
//	}
//	return output;
//}
