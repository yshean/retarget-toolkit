#include "StdAfx.h"
#include "HierarchyFHShiftMap.h"

HierarchyFHShiftMap::HierarchyFHShiftMap(void)
{
}

HierarchyFHShiftMap::~HierarchyFHShiftMap(void)
{
}

//CvMat* HierarchyFHShiftMap::InterpolateMapping(CvMat* mapping)
//{
//	CvMat* newMapping = cvCreateMat(mapping->rows * 2, mapping->cols * 2, CV_32SC2);
//	
//	for(int i = 0; i < newMapping->width; i++)
//		for(int j = 0; j < newMapping->height; j++)
//		{
//			int low_i = i / 2;
//			int low_j = j / 2;
//			int low_id = cvGet2D(mapping, j, i).val[0];
//			int high_id = low_id * 4;
//			cvSet2D(newMapping, j, i, cvScalar(high_id));
//			cvSet2D(newMapping, j, i + 1, cvScalar(high_id + 1));
//			cvSet2D(newMapping, j + 1, i, cvScalar(high_id + 2));
//			cvSet2D(newMapping, j + 1, i + 1, cvScalar(high_id + 3));
//		}
//
//	return newMapping;
//}

vector<CvPoint*>* HierarchyFHShiftMap::InterpolatePointMapping(vector<CvPoint*>* pointMapping)
{
	vector<CvPoint*>* newPointMapping = new vector<CvPoint*>();
	
	int pointCount = pointMapping->size();
	for(int i = 0; i < pointCount; i++)
	{
		CvPoint* currentPoint = (*pointMapping)[i];
		newPointMapping->push_back(CreateCvPoint(currentPoint->x, currentPoint->y));
		newPointMapping->push_back(CreateCvPoint(currentPoint->x + 1, currentPoint->y));
		newPointMapping->push_back(CreateCvPoint(currentPoint->x, currentPoint->y + 1));
		newPointMapping->push_back(CreateCvPoint(currentPoint->x + 1, currentPoint->y + 1));
	}

	return newPointMapping;
}

void HierarchyFHShiftMap::InterpolateShiftMapping(CvMat* lowerMap, CvMat* higherMap)
{	
	if(higherMap->width / 2 == lowerMap->width && higherMap->height / 2 == lowerMap->height)
	{
		for(int i = 0; i < higherMap->width; i++)
			for(int j = 0; j < higherMap->height; j++)
			{
				CvPoint lowerPoint = cvPoint(i/2, j/2);
				//printf("test: %i %i\n", lowerPoint.x, lowerPoint.y);
				
				// heuristic way to prevent interpolate outside image
				// ex: higher img: 45 -> downsample to 22 size lower img
				if(lowerPoint.x == lowerMap->width)
					lowerPoint.x--;
				if(lowerPoint.y == lowerMap->height)
					lowerPoint.y--;
				
				CvPoint label = GetLabel(lowerPoint, lowerMap);
				 
				SetLabel(cvPoint(i,j), cvPoint(label.x * 2, label.y * 2), higherMap);
			}
	}
	else
	{
		printf("Invalid side");
	}	
}

void HierarchyFHShiftMap::InterpolateShiftMapping(CvMat* lowerMap, CvMat* higherMap, IplImage* lowerMask, IplImage* higherMask)
{
	if(higherMap->width / 2 == lowerMap->width && higherMap->height / 2 == lowerMap->height)
	{
		for(int i = 0; i < higherMap->width; i++)
			for(int j = 0; j < higherMap->height; j++)
			{
				
				CvPoint lowerPoint = cvPoint(i/2, j/2);
				//printf("test: %i %i\n", lowerPoint.x, lowerPoint.y);
				
				// heuristic way to prevent interpolate outside image
				// ex: higher img: 45 -> downsample to 22 size lower img
				if(lowerPoint.x == lowerMap->width)
					lowerPoint.x--;
				if(lowerPoint.y == lowerMap->height)
					lowerPoint.y--;
				
				CvPoint label = GetLabel(lowerPoint, lowerMap);
				SetLabel(cvPoint(i,j), cvPoint(label.x * 2, label.y * 2), higherMap);
				//if(!IsMaskedPixel(lowerPoint.x, lowerPoint.y, lowerMask) && !IsMaskedPixel(i,j, higherMask))
				//	SetLabel(cvPoint(i,j), cvPoint(label.x * 2, label.y * 2), higherMap);
				//else
				//if(IsMaskedPixel(lowerPoint.x, lowerPoint.y, lowerMask) && !IsMaskedPixel(i,j, higherMask))
				//{
				//	// search through neighbor hood to find nearest one
				//	for(int ii = 0; ii < 2; ii++)
				//	for(int jj = 0; jj < 2; jj++)
				//	{
				//		if(!IsMaskedPixel(lowerPoint.x + ii, lowerPoint.y + jj, lowerMask))
				//		{
				//			CvPoint tempLabel = GetLabel(cvPoint(lowerPoint.x + ii, lowerPoint.y + jj), lowerMap);							
				//			SetLabel(cvPoint(i,j), cvPoint(tempLabel.x * 2, tempLabel.y * 2), higherMap);
				//		}
				//	}
				//	// search through neighbor hood to find nearest one
				//	for(int ii = 0; ii > -2; ii++)
				//	for(int jj = 0; jj > -2; jj++)
				//	{
				//		if(!IsMaskedPixel(lowerPoint.x + ii, lowerPoint.y + jj, lowerMask))
				//		{
				//			CvPoint tempLabel = GetLabel(cvPoint(lowerPoint.x + ii, lowerPoint.y + jj), lowerMap);							
				//			SetLabel(cvPoint(i,j), cvPoint(tempLabel.x * 2, tempLabel.y * 2), higherMap);
				//		}
				//	}
				//}
				
				
			}
	}
	else
	{
		printf("Invalid size");
	}	
}

void HierarchyFHShiftMap::DownSamplingMask(IplImage* higherMask, IplImage* lowerMask)
{
	for(int i = 0; i < lowerMask->width; i++)
		for(int j = 0; j < lowerMask->height; j++)
		{
			cvSet2D(lowerMask, j, i, cvScalar(0));
		}
	for(int i = 0; i < higherMask->width; i++)
		for(int j = 0; j < higherMask->height; j++)
		{
			CvPoint lowerPoint = cvPoint(i/2, j/2);
			if(lowerPoint.x == lowerMask->width)
				lowerPoint.x--;
			if(lowerPoint.y == lowerMask->height)
				lowerPoint.y--;
			if(!IsMaskedPixel(i, j, higherMask))
				cvSet2D(lowerMask, lowerPoint.y, lowerPoint.x, cvScalar(255));
		}
}

void HierarchyFHShiftMap::DownSampling(IplImage* source, IplImage* dst, int size)
{ 
	IplImage* blur = cvCloneImage(source);
	if(size > 0)
		cvSmooth(source, blur, 2, 3, 3);
	for(int i = 0; i < dst->width; i++)
		for(int j = 0; j < dst->height; j++)
		{
			CvScalar value = cvGet2D(blur, j*2, i*2);
			cvSet2D(dst, j, i, value);
		} 
}

void HierarchyFHShiftMap::DownSamplingImage(IplImage* input, vector<IplImage*>* list, int limitedSize, int blurSize)
{
	IplImage* level = input;	
	list->push_back(level);
	while(level->height > limitedSize)
	{
		IplImage* level_temp = cvCreateImage(cvSize(level->width/2, level->height/2), input->depth, input->nChannels);				
		DownSampling(level, level_temp, blurSize);		
		// 
		//cvPyrDown(level, level_temp); 	
		
		level = level_temp;		
		list->push_back(level);		
	}  
}

void HierarchyFHShiftMap::DownSamplingMask(IplImage* mask, vector<IplImage*>* list, int limitedSize)
{
	IplImage* level = mask;	
	list->push_back(level);
	while(level->height > limitedSize)
	{
		IplImage* level_temp = cvCreateImage(cvSize(level->width/2, level->height/2), mask->depth, mask->nChannels);				
		DownSamplingMask(level, level_temp);		
		// 
		//cvPyrDown(level, level_temp); 	
		
		level = level_temp;		
		list->push_back(level);		
	}  
}
void HierarchyFHShiftMap::DownSamplingInput(IplImage* input, IplImage* saliency, IplImage* mask, IplImage* maskData)
{
	_inputList = new vector<IplImage*>();
	_saliencyList = new vector<IplImage*>();
	_maskList = new vector<IplImage*>();
	_maskDataList = new vector<IplImage*>();
	DownSamplingImage(input, _inputList, 30, 3);
	DownSamplingImage(saliency, _saliencyList, 30, 3);
	DownSamplingImage(mask, _maskList, 30, 3);
	DownSamplingImage(maskData, _maskDataList, 30, 3);
}
void HierarchyFHShiftMap::ComputeShiftMapGuess(IplImage* input, IplImage* saliency, CvMat* guess, CvSize output, CvSize shiftSize)
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
		ForDataFHHierarchy dataCost;
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
		dataCost.guess = guess;
		_gcGeneral->setDataCost(&dataFunctionFHHierarchy, &dataCost);

		// setup smooth cost
		ForSmoothFHHierarchy smoothCost;
		smoothCost.input = input;
		smoothCost.inputGradient = inputGradient;
		smoothCost.inputSize = cvSize(input->width, input->height);
		smoothCost.pointMapping = _pointMapping;
		smoothCost.shiftSize = shiftSize;
		smoothCost.guess = guess;
		_gcGeneral->setSmoothCost(&smoothFunctionFHHierarchy, &smoothCost);

		printf("\nBefore optimization energy is %d \n", _gcGeneral->compute_energy());
		//gc->swap(20);
		_gcGeneral->expansion(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("\nAfter optimization energy is %d \n", _gcGeneral->compute_energy()); 
	}
	catch (GCException e){
		e.Report();
	}

}
CvMat* HierarchyFHShiftMap::CalculateLabelMapGuess(CvMat* intialGuess, vector<CvPoint*>* pointMapping, GCoptimizationGeneralGraph* gc)
{
	CvMat* output = cvCreateMat(intialGuess->height, intialGuess->width, CV_32SC2);
	int num_pixels = pointMapping->size();

	printf("Getting label map... \n");
	for(int i = 0; i < num_pixels; i++)
	{
		int label = gc->whatLabel(i);		
		CvPoint* point = (*(_pointMapping))[i];
		CvPoint shift = GetShift(label, _shiftSize);
		CvPoint guess = GetLabel(*point, intialGuess);		
		CvPoint pointLabel = cvPoint(shift.x + guess.x, shift.y + guess.y);	

		// test
		CvPoint mapped = cvPoint(point->x + pointLabel.x, point->y + pointLabel.y);
		if(IsOutside(mapped, cvSize(_input->width, _input->height)))
			printf("mapped outside");
		 
		SetLabel(*point, pointLabel, output);
	}
	return output;
}
IplImage* HierarchyFHShiftMap::GetRetargetedImage(int level)
{
	IplImage* output = cvCloneImage((*_maskDataList)[level]);
	//cvNamedWindow("Test");
	//while(1)
	//{
	//	cvShowImage("Test", output);
	//	cvWaitKey(100);
	//}
	vector<CvPoint*>* pointMapping = (*_pointMappingList)[level];
	IplImage* image = (*_inputList)[level];
	CvMat* labelMap = (*_labelMapList)[level];

	int num_pixels = pointMapping->size();

	printf("Rendering graph-cut result to image... \n");
	
	for(int i = 0; i < num_pixels; i++)
	{
		CvPoint point = *(*pointMapping)[i];
		CvPoint label = GetLabel(point, labelMap);
		CvPoint mappedPoint = cvPoint(point.x + label.x, point.y + label.y);
		
		CvScalar value;
		if(IsOutside(mappedPoint, cvSize(image->width, image->height)))
			value = cvScalar(0, 0, 255);
		else
			value = cvGet2D(image, mappedPoint.y, mappedPoint.x);
		cvSet2D(output, point.y, point.x, value);
	}

	return output;
}
int HierarchyFHShiftMap::GetLevelCount()
{
	return _maskList->size();
}

void HierarchyFHShiftMap::DownSamplingInput(IplImage* input, IplImage* saliency, IplImage* mask, CvSize outputSize, CvPoint shift)
{
	_inputList = new vector<IplImage*>();
	_saliencyList = new vector<IplImage*>();
	_maskDataList = new vector<IplImage*>();
	_maskList = new vector<IplImage*>();

	DownSamplingImage(input, _inputList, 30, 3);
	DownSamplingImage(saliency, _saliencyList, 30, 3);	
	
	IplImage* maskData = cvCreateImage(outputSize, IPL_DEPTH_8U, 3);
	DownSamplingImage(maskData, _maskDataList, 30, 0);
	IplImage* maskOutput = cvCreateImage(outputSize, IPL_DEPTH_8U, 3);
	DownSamplingImage(maskOutput, _maskList, 30, 0);

	vector<IplImage*>* maskInputList = new vector<IplImage*>();
	DownSamplingMask(mask, maskInputList, 30);

	for(int i = 0; i < maskInputList->size(); i++)
	{
		CvSize outputSizeLevel;
		outputSizeLevel.width = (*_maskDataList)[i]->width;
		outputSizeLevel.height = (*_maskDataList)[i]->height;
		CvPoint shiftLevel;
		if(i == 0)
		{			
			shiftLevel = shift;
		}
		else
		{
			shiftLevel.x = shift.x / (2 * i);
			shiftLevel.y = shift.y / (2 * i);
		}
		CreateMask((*_inputList)[i], (*maskInputList)[i], shiftLevel, outputSizeLevel, (*_maskList)[i], (*_maskDataList)[i]);
	}
}

void HierarchyFHShiftMap::ComputeFastShiftMap2(IplImage* input, IplImage* saliency, IplImage* mask, CvPoint shift, CvSize outputSize)
{
	// preprocess mask
	for(int i = 0; i < mask->width; i++)
		for(int j = 0; j < mask->height; j++)
		{
			CvScalar value = cvGet2D(mask, j, i);
			if(IsMaskedPixel(i, j, mask))
				cvSet2D(mask, j, i, cvScalar(0));
			else
				cvSet2D(mask, j, i, cvScalar(255));

		}

	// downsampling inputs and masks
	DownSamplingInput(input, saliency, mask, outputSize, shift);

	int levelCount = _maskList->size();
	_labelMapList = new vector<CvMat*>();
	_pointMappingList = new vector<vector<CvPoint*>*>();
	CvMat* guess;
	CvMat* labelMap;

	//_mask = (*_maskList)[3];
	//_maskData = (*_maskDataList)[3];
	//ComputeShiftMap((*_inputList)[3], (*_saliencyList)[3], 
	//	cvSize((*_maskList)[3]->width, (*_maskList)[3]->height), 
	//	cvSize((*_inputList)[3]->width, (*_inputList)[3]->height));
	//IplImage* image = CalculatedRetargetImage();
	//		cvNamedWindow("Test"); 
	//		while(1)
	//		{
	//			cvShowImage("Test", (*_inputList)[3]);
	//			// cvShowImage("Test", image);
	//			cvWaitKey(100);
	//		}
	for(int i = levelCount - 1; i>=0; i--)
	{
		// setup mask and maskdata
		_mask = (*_maskList)[i];
		_maskData = (*_maskDataList)[i];
		cvSaveImage("testMask.jpg", (*_maskList)[i]);
		cvSaveImage("testMaskData.jpg", (*_maskDataList)[i]);
		cvSaveImage("test.jpg", (*_inputList)[i]);
		cvSaveImage("testS.jpg", (*_saliencyList)[i]);
		if(i == levelCount - 1)
		{		
			// first level does not require a guess
			ComputeShiftMap((*_inputList)[i], (*_saliencyList)[i], 
				cvSize((*_maskList)[i]->width, (*_maskList)[i]->height), 
				cvSize((*_inputList)[i]->width, (*_inputList)[i]->height));
			labelMap = CalculateLabelMap();
			//IplImage* image = CalculatedRetargetImage();
			//cvNamedWindow("Test");
			//while(1)
			//{
			//	cvShowImage("Test", (*_maskList)[i]);
			//	//cvShowImage("Test", image);
			//	cvWaitKey(100);
			//}

		}
		else
		{			
			ComputeShiftMapGuess((*_inputList)[i], (*_saliencyList)[i], guess,
				cvSize((*_maskList)[i]->width, (*_maskList)[i]->height), 
				cvSize(3,3));	
			labelMap = CalculateLabelMapGuess(guess, _pointMapping, _gcGeneral);
		}		

		_labelMapList->insert(_labelMapList->begin(), labelMap);
		_pointMappingList->insert(_pointMappingList->begin(), _pointMapping);
		if(i > 0)
		{
			guess = cvCreateMat((*_maskList)[i-1]->height, (*_maskList)[i-1]->width , CV_32SC2);
			InterpolateShiftMapping(labelMap, guess, (*_maskList)[i], (*_maskList)[i-1]);
		}
	}
}

void HierarchyFHShiftMap::ComputeFastShiftMap(IplImage *input, IplImage *saliency)
{
	// downsampling inputs and masks
	DownSamplingInput(input, saliency, _mask, _maskData);

	int levelCount = _maskList->size();
	_labelMapList = new vector<CvMat*>();
	_pointMappingList = new vector<vector<CvPoint*>*>();
	CvMat* guess;
	CvMat* labelMap;

	for(int i = levelCount - 1; i>=0; i--)
	{
		// setup mask and maskdata
		_mask = (*_maskList)[i];
		_maskData = (*_maskDataList)[i];
		cvSaveImage("testMask.jpg", (*_maskList)[i]);
		cvSaveImage("testMaskData.jpg", (*_maskDataList)[i]);
		cvSaveImage("test.jpg", (*_inputList)[i]);
		cvSaveImage("testS.jpg", (*_saliencyList)[i]);
		if(i == levelCount - 1)
		{		
			// first level does not require a guess
			ComputeShiftMap((*_inputList)[i], (*_saliencyList)[i], 
				cvSize((*_maskList)[i]->width, (*_maskList)[i]->height), 
				cvSize((*_inputList)[i]->width, (*_inputList)[i]->height));
			//labelMap = CalculateLabelMap();
			IplImage* image = CalculatedRetargetImage();
			cvNamedWindow("Test");
			while(1)
			{
				cvShowImage("Test", (*_maskDataList)[i]);
				cvWaitKey(100);
			}

		}
		else
		{			
			ComputeShiftMapGuess((*_inputList)[i], (*_saliencyList)[i], guess,
				cvSize((*_maskList)[i]->width, (*_maskList)[i]->height), 
				cvSize(3,3));	
			labelMap = CalculateLabelMapGuess(guess, _pointMapping, _gcGeneral);
		}		

		_labelMapList->insert(_labelMapList->begin(), labelMap);
		_pointMappingList->insert(_pointMappingList->begin(), _pointMapping);
		if(i > 0)
		{
			guess = cvCreateMat((*_maskList)[i-1]->height, (*_maskList)[i-1]->width , CV_32SC2);
			InterpolateShiftMapping(labelMap, guess);
		}
	}
	//for(int i = levelCount; i >= 0; i--)
	//{ 
	//	if(i == levelCount)		
	//		// first level does not require an initialGuess
	//		ComputeShiftMap((*_imageList)[i], (*imageSList)[i], (*outputSizeList)[i], cvSize((*_imageList)[i]->width, (*_imageList)[i]->height));
	//	else
	//		ComputeShiftMapGuess((*_imageList)[i], (*imageSList)[i], _initialGuess, (*outputSizeList)[i], cvSize(3,3));
	//	//
	//	//if(i == levelCount - 3)
	//	//{
	//	//	IplImage* image = GetRetargetImage();
	//	//	//IplImage* image = (*imageSList)[i];
	//	//	cvNamedWindow("test");
	//	//	while(1)
	//	//	{
	//	//		cvShowImage("test", image);
	//	//		cvWaitKey(100);
	//	//	}
	//	//}
	//	// save result	

	//	

	//	CvMat* labelMap;
	//	if(i == levelCount)
	//		labelMap = CalculateLabelMap();
	//	else
	//		labelMap = CalculateLabelMapGuess();
	// 


	//	_labelMapList->insert(_labelMapList->begin(), labelMap);
	//	if(i > 0)
	//	{	
	//		cvReleaseMat(&_initialGuess);	
	//		_initialGuess = cvCreateMat((*outputSizeList)[i-1].height, (*outputSizeList)[i-1].width , CV_32SC2);
	//		GetInterpolationMap(labelMap, _initialGuess);			
	//		//if(i == index)
	//		//{
	//		//	IplImage* image = GetImageFromLabelMap(_initialGuess, (*_imageList)[i-1]);
	//		//	//IplImage* image = (*imageList)[0];
	//		//	cvNamedWindow("test");
	//		//	while(1)
	//		//	{
	//		//		cvShowImage("test", image);
	//		//		int key = cvWaitKey(100);
	//		//		if(key == 32)
	//		//		{
	//		//			index--;
	//		//			break;
	//		//		}
	//		//	}
	//		//}			 

	//		ClearGC();
	//	}		
	//}
}