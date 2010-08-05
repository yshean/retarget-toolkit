#include "StdAfx.h"
#include "FeedbackRetargeting.h"

FeedbackRetargeting::FeedbackRetargeting(void)
{
}

FeedbackRetargeting::~FeedbackRetargeting(void)
{
}

void FeedbackRetargeting::ComputeShiftMap(IplImage* input, IplImage* saliency, CvSize output, CvSize shiftSize)
{
		try{
		_inputSize.width = input->width;
		_inputSize.height = input->height;
		_outputSize = output;
		_shiftSize = shiftSize;
		_input = input; 

		_gc = new GCoptimizationGridGraph(_outputSize.width, _outputSize.height, _shiftSize.width * _shiftSize.height);

		// set up the needed data to pass to function for the data costs
		ForDataFunction dataFn;
		dataFn.inputSize = _inputSize;
		dataFn.outputSize = _outputSize;
		dataFn.shiftSize = _shiftSize;
		dataFn.saliency = saliency;
 
		_gc->setDataCost(&dataFunctionShiftmap,&dataFn);
		
		// smoothness comes from function pointer
		ForSmoothFunction smoothFn;
		smoothFn.inputSize = _inputSize;
		smoothFn.outputSize = _outputSize;
		smoothFn.shiftSize = _shiftSize;
		smoothFn.image = input;
		smoothFn.gradient = cvCloneImage(input);
		cvSobel(input, smoothFn.gradient, 1, 1);	 
		_gc->setSmoothCost(&smoothFunctionShiftmap, &smoothFn);
		
		printf("\nBefore optimization energy is %d \n", _gc->compute_energy());
		//gc->swap(20);
		_gc->expansion(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("\nAfter optimization energy is %d \n", _gc->compute_energy()); 

		//cvNamedWindow("test");
		//while(1)
		//{
		//	cvShowImage("test", dataFn._visited);
		//	cvWaitKey(100);
		//}
	}
	catch (GCException e){
		e.Report();
	}
}
