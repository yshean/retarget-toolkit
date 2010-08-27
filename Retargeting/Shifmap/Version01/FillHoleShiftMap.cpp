#include "StdAfx.h"
#include "FillHoleShiftMap.h"

FillHoleShiftMap::FillHoleShiftMap(void)
{
}

FillHoleShiftMap::~FillHoleShiftMap(void)
{
}

void FillHoleShiftMap::ComputeShiftMap(IplImage* input, IplImage* saliency, CvSize output, CvSize shiftSize)
{
	try{
		_input = input;
		_shiftSize = shiftSize;

		IplImage* inputGradient = cvCloneImage(input);
		cvSobel(input, inputGradient, 1, 1);

		// first processing the mask (including neighborhood)	
		// ProcessMask();
		ProcessMask(_mask);

		// setup data cost & smooth cost for masked data
		ForDataFH dataCost;
		dataCost.maskData = _maskData;
		IplImage* maskDataGradient = cvCloneImage(_maskData);
		cvSobel(_maskData, maskDataGradient, 1, 1);
		dataCost.maskDataGradient = maskDataGradient;
		dataCost.pointMapping = _pointMapping;
		dataCost.shiftSize = shiftSize;
		dataCost.saliency = saliency;
		dataCost.inputSize = cvSize(input->width, input->height);
		dataCost.maskNeighbor = _maskNeighbor;
		dataCost.inputGradient = inputGradient;
		dataCost.input = input;
		_gcGeneral->setDataCost(&dataFunctionFH, &dataCost);

		// setup smooth cost
		ForSmoothFH smoothCost;
		smoothCost.input = input;
		smoothCost.inputGradient = inputGradient;
		smoothCost.inputSize = cvSize(input->width, input->height);
		smoothCost.pointMapping = _pointMapping;
		smoothCost.shiftSize = shiftSize;
		_gcGeneral->setSmoothCost(&smoothFunctionFH, &smoothCost);

		printf("\nBefore optimization energy is %d \n", _gcGeneral->compute_energy());
		//gc->swap(20);
		_gcGeneral->expansion(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("\nAfter optimization energy is %d \n", _gcGeneral->compute_energy()); 
	}
	catch (GCException e){
		e.Report();
	}
}
void FillHoleShiftMap::ComputeShiftMap2(IplImage* input, IplImage* saliency, CvSize output, CvSize shiftSize, MaskShift* maskShift)
{
	try{
		_input = input;
		_shiftSize = shiftSize;

		IplImage* inputGradient = cvCloneImage(input);
		cvSobel(input, inputGradient, 1, 1);

		// first processing the mask (including neighborhood)	
		// ProcessMask();
		ProcessMask(maskShift);

		// setup data cost & smooth cost for masked data
		ForDataFH2 dataCost;
		dataCost.mask = maskShift;
		 
		IplImage* maskDataGradient = cvCloneImage(_maskData);
		cvSobel(_maskData, maskDataGradient, 1, 1);
		 
		dataCost.pointMapping = _pointMapping;
		dataCost.shiftSize = shiftSize;
		dataCost.saliency = saliency;
		dataCost.inputSize = cvSize(input->width, input->height);
		dataCost.maskNeighbor = _maskNeighbor;
		dataCost.inputGradient = inputGradient;
		dataCost.input = input;
		_gcGeneral->setDataCost(&dataFunctionFH2, &dataCost);

		// setup smooth cost
		ForSmoothFH smoothCost;
		smoothCost.input = input;
		smoothCost.inputGradient = inputGradient;
		smoothCost.inputSize = cvSize(input->width, input->height);
		smoothCost.pointMapping = _pointMapping;
		smoothCost.shiftSize = shiftSize;
		_gcGeneral->setSmoothCost(&smoothFunctionFH, &smoothCost);

		printf("\nBefore optimization energy is %d \n", _gcGeneral->compute_energy());
		//gc->swap(20);
		_gcGeneral->expansion(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("\nAfter optimization energy is %d \n", _gcGeneral->compute_energy()); 
	}
	catch (GCException e){
		e.Report();
	}
}
IplImage* FillHoleShiftMap::GetRetargetImage(IplImage* input, IplImage* saliency, CvSize outputSize)
{	
	ComputeShiftMap(input, saliency, outputSize, cvSize(input->width, input->height));
	
	return CalculatedRetargetImage();
}
IplImage* FillHoleShiftMap::GetRetargetImage2(IplImage* input, IplImage* saliency, CvSize outputSize, MaskShift* maskShift)
{
	ComputeShiftMap2(input, saliency, outputSize, cvSize(input->width, input->height), maskShift);
	return CalculatedRetargetImage();
}

void FillHoleShiftMap::SetMask(IplImage *mask, IplImage *maskData)
{
	_mask = mask;
	_maskData = maskData;
}
void FillHoleShiftMap::ProcessMapping(IplImage* mask, CvMat* mapping, vector<CvPoint*>* pointMapping)
{
	// counting number of nodes and set up mapping
	//CvMat* mapping = cvCreateMat(_mask->height, _mask->width, CV_32SC2);
	//_pointMapping = new vector<CvPoint*>();	
	int count = 0;
	for(int i = 0; i < _mask->width; i++)
		for(int j = 0; j < _mask->height; j++)
		{
			CvScalar value = cvGet2D(_mask, j, i);
			if(value.val[0] > 100) // white
			{
				CvPoint* point = new CvPoint();
				point->x = i;
				point->y = j;
				pointMapping->push_back(point);					
				cvSet2D(mapping, j, i, cvScalar(count, count, count));								
				count++;
			}
		}	

}
void FillHoleShiftMap::ProcessMapping(MaskShift* mask, CvMat* mapping, vector<CvPoint*>* pointMapping)
{
	// counting number of nodes and set up mapping
	//CvMat* mapping = cvCreateMat(_mask->height, _mask->width, CV_32SC2);
	//_pointMapping = new vector<CvPoint*>();	
	int count = 0;
	int width = mask->GetWidth();
	int height = mask->GetHeight();
	for(int i = 0; i < width; i++)
		for(int j = 0; j < height; j++)
		{
			if(!mask->IsMaskedPixel(i, j))
			{
				CvPoint* point = new CvPoint();
				point->x = i;
				point->y = j;
				pointMapping->push_back(point);					
				cvSet2D(mapping, j, i, cvScalar(count, count, count));								
				count++;
			}
		}	

}

void FillHoleShiftMap::ProcessMask(IplImage* mask)
{
	// counting number of nodes and set up mapping
	CvMat* mapping = cvCreateMat(mask->height, mask->width, CV_32SC2);
	for(int i = 0; i < mapping->width; i++)
		for(int j = 0; j < mapping->height; j++)
		{
			cvSet2D(mapping, j, i, cvScalar(-1));
		}

	vector<CvPoint*>* pointMapping = new vector<CvPoint*>();
	ProcessMapping(mask, mapping, pointMapping);
	
	_pointMapping = pointMapping;
	_gcGeneral = new GCoptimizationGeneralGraph(_pointMapping->size(), _shiftSize.width * _shiftSize.height);
	
	SetupGCOptimizationNeighbor(_gcGeneral, mapping);
	_maskNeighbor = cvCreateImage(cvSize(_mask->width, _mask->height), IPL_DEPTH_8U, 4);
	SetupMaskNeighbor(_maskNeighbor, mask);
	 
}
void FillHoleShiftMap::ProcessMask(MaskShift* mask)
{
	// compute mask and mask data
	int width = mask->GetWidth();
	int height = mask->GetHeight();
	for(int i = 0; i < width; i++)
		for(int j = 0; j < width; j++)
		{
		}
	// counting number of nodes and set up mapping
	CvMat* mapping = cvCreateMat(mask->GetHeight(), mask->GetWidth(), CV_32SC2);
	for(int i = 0; i < mapping->width; i++)
		for(int j = 0; j < mapping->height; j++)
		{
			cvSet2D(mapping, j, i, cvScalar(-1));
		}

	vector<CvPoint*>* pointMapping = new vector<CvPoint*>();
	ProcessMapping(mask, mapping, pointMapping);
	
	_pointMapping = pointMapping;
	_gcGeneral = new GCoptimizationGeneralGraph(_pointMapping->size(), _shiftSize.width * _shiftSize.height);
	
	SetupGCOptimizationNeighbor(_gcGeneral, mapping);

	
	_maskNeighbor = cvCreateImage(cvSize(_mask->width, _mask->height), IPL_DEPTH_8U, 4);
	SetupMaskNeighbor(_maskNeighbor, mask);
	 
}
void FillHoleShiftMap::SetupGCOptimizationNeighbor(GCoptimizationGeneralGraph* gcGeneral, CvMat* mapping)
{	
	for(int i = 0; i < mapping->width-1; i++)
		for(int j = 0; j < mapping->height-1; j++)
		{
			int nodeId1 = cvGet2D(mapping, j, i).val[0];
			int nodeId2;
			nodeId2 = cvGet2D(mapping, j, i + 1).val[0];
 
			// setup neighbors to the right and down only
			if(nodeId2 >= 0 && nodeId1 >= 0)
				gcGeneral->setNeighbors(nodeId1, nodeId2);
			nodeId2 = cvGet2D(mapping, j + 1, i).val[0];
			if(nodeId2 >= 0 && nodeId1 >= 0)
				gcGeneral->setNeighbors(nodeId1, nodeId2); 
		}
}

void FillHoleShiftMap::SetupMaskNeighbor(IplImage* maskNeighbor, IplImage* mask)
{
	for(int i = 0; i < mask->width - 1; i++)
		for(int j = 0; j < mask->height - 1; j++)
		{
			ProcessNeighbor(cvPoint(i,j), cvPoint(i+1,j), maskNeighbor, mask);
			ProcessNeighbor(cvPoint(i,j), cvPoint(i,j+1), maskNeighbor, mask);
		}
}
void FillHoleShiftMap::SetupMaskNeighbor(IplImage* maskNeighbor, MaskShift* mask)
{
	int width = mask->GetWidth();
	int height = mask->GetHeight();
	for(int i = 0; i < width - 1; i++)
		for(int j = 0; j < height - 1; j++)
		{
			ProcessNeighbor(cvPoint(i,j), cvPoint(i+1,j), maskNeighbor, mask);
			ProcessNeighbor(cvPoint(i,j), cvPoint(i,j+1), maskNeighbor, mask);
		}
}
void FillHoleShiftMap::ProcessNeighbor(CvPoint currentPoint, CvPoint neighborPoint, IplImage* maskNeighbor, IplImage* mask)
{
	if(!IsMaskedPixel(currentPoint.x, currentPoint.y, mask) && IsMaskedPixel(neighborPoint.x, neighborPoint.y, mask))
	{
		CvScalar value = cvGet2D(maskNeighbor, currentPoint.y, currentPoint.x);
		SetMaskNeighbor(currentPoint, neighborPoint, &value);					
		cvSet2D(maskNeighbor, currentPoint.y, currentPoint.x, value);
	}
	else
	if(IsMaskedPixel(currentPoint.x, currentPoint.y, mask) && !IsMaskedPixel(neighborPoint.x, neighborPoint.y, mask))
	{
		CvScalar value = cvGet2D(maskNeighbor, currentPoint.y, currentPoint.x);
		SetMaskNeighbor(neighborPoint, currentPoint, &value);					
		cvSet2D(maskNeighbor, neighborPoint.y, neighborPoint.x, value);
	}
}
void FillHoleShiftMap::ProcessNeighbor(CvPoint currentPoint, CvPoint neighborPoint, IplImage* maskNeighbor, MaskShift* mask)
{
	if(!mask->IsMaskedPixel(currentPoint.x, currentPoint.y) && mask->IsMaskedPixel(neighborPoint.x, neighborPoint.y))
	{
		CvScalar value = cvGet2D(maskNeighbor, currentPoint.y, currentPoint.x);
		SetMaskNeighbor(currentPoint, neighborPoint, &value);					
		cvSet2D(maskNeighbor, currentPoint.y, currentPoint.x, value);
	}
	else
	if(mask->IsMaskedPixel(currentPoint.x, currentPoint.y) && !mask->IsMaskedPixel(neighborPoint.x, neighborPoint.y))
	{
		CvScalar value = cvGet2D(maskNeighbor, currentPoint.y, currentPoint.x);
		SetMaskNeighbor(neighborPoint, currentPoint, &value);					
		cvSet2D(maskNeighbor, neighborPoint.y, neighborPoint.x, value);
	}
}


void FillHoleShiftMap::ProcessMask()
{
	// counting number of nodes and set up mapping
	CvMat* mapping = cvCreateMat(_mask->height, _mask->width, CV_32SC2);
	_pointMapping = new vector<CvPoint*>();
	int count = 0;
	for(int i = 0; i < _mask->width; i++)
		for(int j = 0; j < _mask->height; j++)
		{
			CvScalar value = cvGet2D(_mask, j, i);
			if(value.val[0] > 100) // white
			{
				CvPoint* point = new CvPoint();
				point->x = i;
				point->y = j;
				_pointMapping->push_back(point);					
				cvSet2D(mapping, j, i, cvScalar(count, count, count));				
				count++;
			}
		}	
			_gcGeneral = new GCoptimizationGeneralGraph(count, _input->width * _input->height);
	
	// now setup neighborhood system
	_maskNeighbor = cvCreateImage(cvSize(_mask->width, _mask->height), IPL_DEPTH_8U, 4);
	// NOTE: up 0, right 1, down 2, left 3
	for(int i = 0; i < _mask->width; i++)
		for(int j = 0; j < _mask->height; j++)
		{
			int nodeId2;
			int nodeId1 = cvGet2D(mapping, j, i).val[0];

			ProcessNeighbor(cvPoint(i,j), cvPoint(i-1,j), _maskNeighbor, mapping, _gcGeneral);
			ProcessNeighbor(cvPoint(i,j), cvPoint(i+1,j), _maskNeighbor, mapping, _gcGeneral);
			ProcessNeighbor(cvPoint(i,j), cvPoint(i,j-1), _maskNeighbor, mapping, _gcGeneral);
			ProcessNeighbor(cvPoint(i,j), cvPoint(i,j+1), _maskNeighbor, mapping, _gcGeneral);			
		}
}

bool FillHoleShiftMap::IsMaskedPixel(int x, int y, IplImage* mask)
{
	if(!IsOutside(cvPoint(x,y), cvSize(mask->width, mask->height)))
	{
		CvScalar value = cvGet2D(mask, y, x);
		if(value.val[0] < 100)
			return true;
		else return false;
	}
	else
	{
		return true;
	}
}

void FillHoleShiftMap::ProcessNeighbor(CvPoint currentPoint, CvPoint neighborPoint,
									   IplImage* maskNeighbor, CvMat* mapping, GCoptimizationGeneralGraph* gcGeneral)
{
	CvScalar value =  cvGet2D(mapping, currentPoint.y, currentPoint.x);
	int nodeId1 = value.val[0];
	if(nodeId1 >= 0)
	{
		if(!IsOutside(currentPoint, cvSize(maskNeighbor->width, maskNeighbor->height)) &&
			!IsOutside(neighborPoint, cvSize(maskNeighbor->width, maskNeighbor->height)))
		{
			// printf("i: %i, j: %i\n", neighborPoint.x, neighborPoint.y);
			CvScalar value = cvGet2D(mapping, neighborPoint.y, neighborPoint.x);
			int nodeId2 = value.val[0];
			if(nodeId2 >= 0)
			{	
				gcGeneral->setNeighbors(nodeId1, nodeId2);				
			}
			if(!IsMaskedPixel(currentPoint.x, currentPoint.y,_mask) && IsMaskedPixel(neighborPoint.x,neighborPoint.y,_mask))
			{
				CvScalar value = cvGet2D(maskNeighbor, currentPoint.y, currentPoint.x);
				SetMaskNeighbor(currentPoint, neighborPoint, &value);					
				cvSet2D(maskNeighbor, currentPoint.y, currentPoint.x, value);
			}
		}
	}
}

IplImage* FillHoleShiftMap::CalculatedRetargetImage()
{	 
	IplImage* output = cvCloneImage(_maskData);

	int num_pixels = _pointMapping->size();

	printf("Rendering graph-cut result to image... \n");
	for(int i = 0; i < num_pixels; i++)
	{
		int label = _gcGeneral->whatLabel(i);
		CvPoint* point = (*(_pointMapping))[i];
		CvPoint shift = GetShift(label, _shiftSize);		 
		CvPoint pointLabel = cvPoint(point->x + shift.x, point->y + shift.y);

		if(!IsOutside(pointLabel, cvSize(_input->width, _input->height)))
		{
			CvScalar value = cvGet2D(_input, pointLabel.y, pointLabel.x);
			cvSet2D(output, point->y, point->x, value);
			// printf("*%i %i %i %i", point->x, point->y, pointLabel.x, pointLabel.y);
		}
		else
		{
			printf("warning mapped outside");
			cvSet2D(output, point->y, point->x, cvScalar(255, 0, 0));
		}
	}
	return output;
}
void FillHoleShiftMap::ClearGC()
{
	delete _gcGeneral;
}

CvMat* FillHoleShiftMap::CalculateLabelMap()
{
	CvMat* output = cvCreateMat(_mask->height, _mask->width, CV_32SC2);
	
	for(int i = 0; i < output->width; i++)
		for(int j = 0; j < output->height; j++)
		{
			SetLabel(cvPoint(i,j), cvPoint(-100, -100), output);
		}

	int num_pixels = _pointMapping->size();

	printf("Getting label map... \n");
	for(int i = 0; i < num_pixels; i++)
	{
		int label = _gcGeneral->whatLabel(i);		
		CvPoint* point = (*(_pointMapping))[i];
		CvPoint pointLabel = GetShift(label, _shiftSize);		
		
		SetLabel(*point, pointLabel, output);
	}


	return output;
}