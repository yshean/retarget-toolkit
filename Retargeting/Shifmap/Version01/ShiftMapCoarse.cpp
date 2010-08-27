#include "StdAfx.h"
#include "ShiftMapCoarse.h"



ShiftMapCoarse::ShiftMapCoarse(void)
{
}

ShiftMapCoarse::~ShiftMapCoarse(void)
{
}
CvRect GetPatchLocation(CvPoint mappedPoint, CvSize patchSize)
{
	CvRect location;
	location.width = patchSize.width;
	location.height = patchSize.height;
	location.x = mappedPoint.x * patchSize.width;
	location.y = mappedPoint.y * patchSize.height;
	return location;
}
// square difference between horizontal 2 lines, location is at the left most
int SquareDifferenceH(CvPoint location1, CvPoint location2, int width, IplImage* image)
{
	int result = 0;
	for(int i = location1.x; i < location1.x + width; i++)
	{
		result += SquareColorDifference(cvPoint(i, location1.y), cvPoint(i, location2.y), image);
	}
	return result;
}

// square difference between 2 vertical lines, location is at the top
int SquareDifferenceV(CvPoint location1, CvPoint location2, int height, IplImage* image)
{
	int result = 0;
	for(int i = location1.y; i < location1.y + height;i++)
	{
		result += SquareColorDifference(cvPoint(location1.x, i), cvPoint(location2.y, i), image);
	}
	return result;
}

int GetNeighborRectDifference(CvPoint location1, CvPoint location2, CvSize patchSize, IplImage* image)
{
	if(location1.x == location2.x + 1)
	{
		// horizontal
		return SquareDifferenceV(location1, cvPoint(location2.x, location2.y + patchSize.width - 1), patchSize.height, image);
	}
	else
	if(location1.x == location2.x - 1)
	{
		// horizontal
		return SquareDifferenceV(location2, cvPoint(location1.x, location1.y + patchSize.width - 1), patchSize.height, image);
	}
	else
	if(location1.y == location2.y + 1)
	{
		// vertical
		return SquareDifferenceH(location1, cvPoint(location2.x, location2.y + patchSize.height - 1), patchSize.width, image);
	}
	else
	if(location1.y == location2.y - 1)
	{
		// vertical 
		return SquareDifferenceH(location2, cvPoint(location1.x, location1.y + patchSize.height - 1), patchSize.width, image);
	}
	else
	if(location1.x == location2.x && location1.y == location2.y)
		return 0;
	else
		return 100000; // not neighbor
}

// coarse shiftmap
int dataFunctionShiftmapCoarse(int pixel, int label, void *extraData)
{
	ForDataFnCoarse *data = (ForDataFnCoarse *) extraData;
	CvPoint mappedPoint = GetMappedPoint(pixel, label, data->outputSize, data->shiftSize);
	
	if(IsOutside(mappedPoint, data->inputSize))
		return 100000; // prevent mapping outside image

	// convert it back to exact rectangle location in the input
	CvRect location = GetPatchLocation(mappedPoint, data->patchSize);	
	
	int dataCost = 0;
	for(int i = location.x; i  < location.x + location.width; i++)
		for(int j = location.y; j < location.y + location.height; j++)
		{
			CvScalar value = cvGet2D(data->saliency, j, i);
			dataCost += value.val[0] + value.val[1] + value.val[2];
		}

	return dataCost * 0.00005;
}
int smoothFunctionShiftmapCoarse(int pixel1, int pixel2, int label1, int label2, void* extraData)
{
	ForSmoothFnCoarse *data = (ForSmoothFnCoarse *) extraData;
	
	CvPoint pixelPoint1 = GetPoint(pixel1, data->outputSize);	
	CvPoint labelPoint1 = GetMappedPoint(pixel1, label1, data->outputSize, data->shiftSize);	 
	CvPoint pixelPoint2 = GetPoint(pixel2, data->outputSize);	
	CvPoint labelPoint2 = GetMappedPoint(pixel2, label2, data->outputSize, data->shiftSize);	 
	

	if(IsOutside(labelPoint1, data->inputSize) || IsOutside(labelPoint2, data->inputSize))
		return 100000; // prevent mapping outside image
 
	//return 50;
	// pre-compute variables:
	CvPoint neighbor1 = GetNeighbor(pixelPoint1, pixelPoint2, labelPoint1); // neighbor of label1
	CvPoint neighbor2 = GetNeighbor(pixelPoint2, pixelPoint1, labelPoint2); // neighbor of label2

	int energy = 0;

	energy += GetNeighborRectDifference(labelPoint1, neighbor2, data->patchSize, data->image);
	energy += GetNeighborRectDifference(labelPoint2, neighbor1, data->patchSize, data->image);

	// gradient different term 
	energy += 2 * GetNeighborRectDifference(labelPoint2, neighbor1, data->patchSize, data->gradient);
	energy += 2 * GetNeighborRectDifference(labelPoint1, neighbor2, data->patchSize, data->gradient);
	
	return  energy;
	 
}


IplImage* ShiftMapCoarse::GetRetargetImage(IplImage* input, IplImage* saliency, CvSize outputSize)
{
	ComputeShiftMap(input, saliency, outputSize, outputSize);
	return CalculateRetargetImage();
	///return dummy;
}
void ShiftMapCoarse::ComputeShiftMap(IplImage* input, IplImage* saliency, CvSize output, CvSize shiftSize)
{
	try{
		int patchWidth = 10;
		int patchHeight = 10;
		CvSize patchSize = cvSize(patchWidth,patchHeight);
		_patchSize = patchSize;
		_inputSize.width = input->width / patchWidth;
		_inputSize.height = input->height / patchHeight;
		_outputSize.width = output.width / patchWidth;
		_outputSize.height = output.height / patchHeight;
		_shiftSize = _inputSize;
		_input = input; 
		
		_gc = new GCoptimizationGridGraph(_outputSize.width, _outputSize.height, _shiftSize.width * _shiftSize.height);

		// set up the needed data to pass to function for the data costs
		ForDataFnCoarse dataFn;
		dataFn.inputSize = _inputSize;
		dataFn.outputSize = _outputSize;
		dataFn.shiftSize = _shiftSize;
		dataFn.saliency = saliency;
		dataFn.patchSize = _patchSize;
		_gc->setDataCost(&dataFunctionShiftmapCoarse,&dataFn);
		
		// smoothness comes from function pointer
		ForSmoothFnCoarse smoothFn;
		smoothFn.inputSize = _inputSize;
		smoothFn.outputSize = _outputSize;
		smoothFn.shiftSize = _shiftSize;
		smoothFn.image = input;
		smoothFn.gradient = cvCloneImage(input);
		smoothFn.patchSize = _patchSize;
		cvSobel(input, smoothFn.gradient, 1, 1);	 
		_gc->setSmoothCost(&smoothFunctionShiftmapCoarse, &smoothFn);
		
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

IplImage* ShiftMapCoarse::CalculateRetargetImage()
{
	CvSize outputSize;
	outputSize.width = _outputSize.width * _patchSize.width;
	outputSize.height = _outputSize.height * _patchSize.height;
	IplImage* output = cvCreateImage(outputSize, _input->depth, _input->nChannels);
	
	int num_pixels = _outputSize.width * _outputSize.height;

	printf("Rendering graph-cut result to image... \n");
	for(int i = 0; i < num_pixels; i++)
	{
		int label = _gc->whatLabel(i);
		CvPoint point = GetPoint(i, _outputSize);		 
		CvPoint pointLabel = GetMappedPoint(i, label, _outputSize, _shiftSize);
		CvRect patchLocation = GetPatchLocation(pointLabel, _patchSize);
		if(!IsOutside(pointLabel, _inputSize))
		{
			
			for(int i = patchLocation.x; i < patchLocation.x + patchLocation.width; i++)
				for(int j = patchLocation.y; j < patchLocation.y + patchLocation.height; j++)
				{
					CvScalar value = cvGet2D(_input, j, i);
					cvSet2D(output, point.y * _patchSize.height + j - patchLocation.y, point.x * _patchSize.width + i - patchLocation.x, value);
				}
			
			//printf("*%i %i %i %i", point.x, point.y, pointLabel.x, pointLabel.y);
		}
		else
		{
			printf("warning mapped outside");
			for(int i = patchLocation.x; i < patchLocation.x + patchLocation.width; i++)
				for(int j = patchLocation.y; j < patchLocation.y + patchLocation.height; j++)
				{
					cvSet2D(output, j, i, cvScalar(255, 0, 0));
				}
		}
	}
	return output;
}