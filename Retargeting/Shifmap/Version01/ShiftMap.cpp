#include "StdAfx.h"
#include "ShiftMap.h"

ShiftMap::ShiftMap( )
{
 
}

ShiftMap::~ShiftMap(void)
{	
}


IplImage* ShiftMap::GetRetargetImage(IplImage* input, IplImage* saliency, CvSize outputSize)
{
	// ComputeShiftMap(input, saliency, outputSize, outputSize);
	ComputeShiftMap(input, saliency, outputSize, cvSize(input->width, input->height));
	return CalculateRetargetImage();
}

void ShiftMap::ClearGC()
{
	delete _gc;
}

void ShiftMap::ComputeShiftMap(IplImage* input, IplImage* saliency, CvSize output, CvSize shiftSize)
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

//IplImage* ShiftMap::GetLabelMap()
//{
//	IplImage* output = cvCreateImage(_outputSize, _input->depth, _input->nChannels);
//	int num_pixels = _outputSize.width * _outputSize.height;
//	for(int i = 0; i < num_pixels; i++)
//	{	
//		int label = _gc->whatLabel(i);
//		CvPoint point = GetPoint(i, _outputSize.width, _outputSize.height);
//		CvPoint pointLabel = GetMappedPoint(point, label, _shiftSize.width, _shiftSize.height);
//		SetLabel(point, pointLabel, output);
//	}
//	return output;
//}
	



IplImage* ShiftMap::CalculateRetargetImage()
{
	IplImage* output = cvCreateImage(_outputSize, _input->depth, _input->nChannels);
	int num_pixels = _outputSize.width * _outputSize.height;

	printf("Rendering graph-cut result to image... \n");
	for(int i = 0; i < num_pixels; i++)
	{
		int label = _gc->whatLabel(i);
		CvPoint point = GetPoint(i, _outputSize);		 
		CvPoint pointLabel = GetMappedPoint(i, label, _outputSize, _shiftSize);

		if(!IsOutside(pointLabel, _inputSize))
		{
			CvScalar value = cvGet2D(_input, pointLabel.y, pointLabel.x);
			cvSet2D(output, point.y, point.x, value);
			//printf("*%i %i %i %i", point.x, point.y, pointLabel.x, pointLabel.y);
		}
		else
		{
			printf("warning mapped outside");
			cvSet2D(output, point.y, point.x, cvScalar(255, 0, 0));
		}
	}
	return output;
}

CvMat* ShiftMap::CalculateLabelMap()
{
	CvMat* output = cvCreateMat(_outputSize.height, _outputSize.width, CV_32SC2);
	int num_pixels = _outputSize.width * _outputSize.height;

	printf("Getting label map... \n");
	for(int i = 0; i < num_pixels; i++)
	{
		int label = _gc->whatLabel(i);				
		CvPoint point = GetPoint(i, _outputSize);		
		CvPoint pointLabel = GetShift(label, _shiftSize);
		
		//// test
		//if(point.x == 11 && point.y == 0)
		//	printf("test");
		//CvPoint mapped = cvPoint(point.x + pointLabel.x, point.y + pointLabel.y);
		//if(IsOutside(mapped, _inputSize))
		//	printf("test");
		 
		SetLabel(point, pointLabel, output);
	}
	return output;
}
 


IplImage* ShiftMap::GetImageFromLabelMap(CvMat* map, IplImage* image)
{
	IplImage* output = cvCreateImage(cvSize(map->width, map->height), IPL_DEPTH_8U, 3);
	for(int i = 0; i < map->width; i++)
		for(int j = 0; j < map->height; j++)
		{
			CvPoint label = GetLabel(cvPoint(i,j), map);
			CvPoint mappedPoint = cvPoint(i + label.x, j + label.y);
			
			CvScalar value;
			if(IsOutside(mappedPoint, cvSize(image->width, image->height)))
				value = cvScalar(0, 0, 255);
			else
				value = cvGet2D(image, j + label.y, i + label.x);
			cvSet2D(output, j, i, value);
		}
	return output;
}
