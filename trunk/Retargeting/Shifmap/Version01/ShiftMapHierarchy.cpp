#include "StdAfx.h"
#include "ShiftMapHierarchy.h"

ShiftMapHierarchy::ShiftMapHierarchy(void)
{
}

ShiftMapHierarchy::~ShiftMapHierarchy(void)
{
}

IplImage* ShiftMapHierarchy::GetRetargetImage(IplImage* input, IplImage* saliency, CvSize outputSize)
{
	IplImage* dummy;
	return dummy;
}
 

void ShiftMapHierarchy::ComputeFastShiftMap(IplImage* input, IplImage* saliency, CvSize output)
{
	printf("Downsampling...");
	_imageList = new vector<IplImage*>(0);
	_labelMapList = new vector<CvMat*>(0);
	vector<IplImage*>* imageSList = new vector<IplImage*>(0);
	vector<CvSize>* outputSizeList = new vector<CvSize>(0);
	_imageList->push_back(input);
	imageSList->push_back(saliency);
	outputSizeList->push_back(output);

	IplImage* level = input;
	IplImage* levelS = saliency;	
	CvSize levelSize = output;

	int levelCount = 0;

	while(level->width >20 && level->height > 20)
	{
		IplImage* level_temp = cvCreateImage(cvSize(level->width/2, level->height/2), input->depth, input->nChannels);
		IplImage* levelS_temp = cvCreateImage(cvSize(level->width/2, level->height/2), input->depth, input->nChannels);
		
		DownSampling(level, level_temp);
		DownSampling(levelS, levelS_temp);
		// 
		//cvPyrDown(level, level_temp);
		//cvPyrDown(levelS, levelS_temp);		
		
		level = level_temp;
		levelS = levelS_temp; 
		levelCount++;

		CvSize output_temp;
		output_temp.width = levelSize.width / 2;
		output_temp.height = levelSize.height / 2;
		levelSize = output_temp;
		
		_imageList->push_back(level);
		imageSList->push_back(levelS);
		outputSizeList->push_back(levelSize);
	}  

	// dummy initialGuess - just to be released later
	 _initialGuess = cvCreateMat(10,10, CV_32SC2);
	
	 _level = levelCount;
	//int index = levelCount;
	for(int i = levelCount; i >= 0; i--)
	{ 
		if(i == levelCount)		
			// first level does not require an initialGuess
			ComputeShiftMap((*_imageList)[i], (*imageSList)[i], (*outputSizeList)[i], cvSize((*_imageList)[i]->width, (*_imageList)[i]->height));
		else
			ComputeShiftMapGuess((*_imageList)[i], (*imageSList)[i], _initialGuess, (*outputSizeList)[i], cvSize(3,3));
		//
		//if(i == levelCount - 3)
		//{
		//	IplImage* image = GetRetargetImage();
		//	//IplImage* image = (*imageSList)[i];
		//	cvNamedWindow("test");
		//	while(1)
		//	{
		//		cvShowImage("test", image);
		//		cvWaitKey(100);
		//	}
		//}
		// save result	

		

		CvMat* labelMap;
		if(i == levelCount)
			labelMap = CalculateLabelMap();
		else
			labelMap = CalculateLabelMapGuess();
	 


		_labelMapList->insert(_labelMapList->begin(), labelMap);
		if(i > 0)
		{	
			cvReleaseMat(&_initialGuess);	
			_initialGuess = cvCreateMat((*outputSizeList)[i-1].height, (*outputSizeList)[i-1].width , CV_32SC2);
			GetInterpolationMap(labelMap, _initialGuess);			
			//if(i == index)
			//{
			//	IplImage* image = GetImageFromLabelMap(_initialGuess, (*_imageList)[i-1]);
			//	//IplImage* image = (*imageList)[0];
			//	cvNamedWindow("test");
			//	while(1)
			//	{
			//		cvShowImage("test", image);
			//		int key = cvWaitKey(100);
			//		if(key == 32)
			//		{
			//			index--;
			//			break;
			//		}
			//	}
			//}			 

			ClearGC();
		}		
	}

}
void ShiftMapHierarchy::DownSampling(IplImage* source, IplImage* dst)
{ 
	IplImage* blur = cvCloneImage(source);
	cvSmooth(source, blur,2, 5, 5);
	for(int i = 0; i < dst->width; i++)
		for(int j = 0; j < dst->height; j++)
		{
			CvScalar value = cvGet2D(blur, j*2, i*2);
			cvSet2D(dst, j, i, value);
		} 
}


void ShiftMapHierarchy::GetInterpolationMap(CvMat* lowerMap, CvMat* higherMap)
{
	// check if size is double
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
		printf("not going inside");
	}
}

void ShiftMapHierarchy::ComputeShiftMapGuess(IplImage* input, IplImage* saliency, CvMat* initialGuess, CvSize output, CvSize shiftSize)
{	
		try{
		_inputSize.width = input->width;
		_inputSize.height = input->height;
		_outputSize = output;
		_shiftSize = shiftSize;
		_input = input; 
		_initialGuess = initialGuess;
			
		_gc = new GCoptimizationGridGraph(_outputSize.width, _outputSize.height, _shiftSize.width * _shiftSize.height);

		// set up the needed data to pass to function for the data costs
		ForDataFunctionH dataFn;
		dataFn.inputSize = _inputSize;
		dataFn.outputSize = _outputSize;
		dataFn.shiftSize = _shiftSize;
		dataFn.saliency = saliency;
		dataFn.initialGuess = initialGuess;

		_gc->setDataCost(&dataFunctionShiftmapH,&dataFn);
		
		// smoothness comes from function pointer
		ForSmoothFunctionH smoothFn;
		smoothFn.inputSize = _inputSize;
		smoothFn.outputSize = _outputSize;
		smoothFn.shiftSize = _shiftSize;
		smoothFn.image = input;
		smoothFn.gradient = cvCloneImage(input);
		smoothFn.initialGuess = initialGuess;
		cvSobel(input, smoothFn.gradient, 1, 1);	 
		_gc->setSmoothCost(&smoothFunctionShiftmapH, &smoothFn);
		
		printf("\nBefore optimization energy is %d \n", _gc->compute_energy());
		//gc->swap(20);
		_gc->expansion(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("\nAfter optimization energy is %d \n", _gc->compute_energy()); 	
 
	}
	catch (GCException e){
		e.Report();
	}

}

IplImage* ShiftMapHierarchy::GetRetargetImage(int level)
{
 	CvMat* labelMap = (*_labelMapList)[level];
	// IplImage* image = cvCreateImage(cvSize(labelMap->width,labelMap->height), IPL_DEPTH_8U, 3);
	
	return GetImageFromLabelMap(labelMap, (*_imageList)[level]);	
}

int  ShiftMapHierarchy::GetLevelCount()
{
	return _level;
}

IplImage* ShiftMapHierarchy::GetOriginalImage(int level)
{
	return (*_imageList)[level];
}

IplImage* ShiftMapHierarchy::GetRetargetImageH()
{
	IplImage* output = cvCreateImage(_outputSize, _input->depth, _input->nChannels);
	int num_pixels = _outputSize.width * _outputSize.height;

	printf("Rendering graph-cut result to image... \n");
	for(int i = 0; i < num_pixels; i++)
	{
		int label = _gc->whatLabel(i);
		CvPoint point = GetPoint(i, _outputSize);		
		CvPoint pointLabel = GetMappedPointInitialGuess(i, label, _outputSize, _shiftSize, _initialGuess);
		
		if(!IsOutside(pointLabel, _inputSize))
		{
			CvScalar value = cvGet2D(_input, pointLabel.y, pointLabel.x);
			cvSet2D(output, point.y, point.x, value);		
		}
		else
		{
			printf("warning mapped outside");
			cvSet2D(output, point.y, point.x, cvScalar(255, 0, 0));
		}
	}
	return output;
}

CvMat* ShiftMapHierarchy::GetLabelMap(int level)
{
	return (*_labelMapList)[level];
}

CvMat* ShiftMapHierarchy::CalculateLabelMapGuess()
{
	CvMat* output = cvCreateMat(_outputSize.height, _outputSize.width, CV_32SC2);
	int num_pixels = _outputSize.width * _outputSize.height;

	printf("Getting label map... \n");
	for(int i = 0; i < num_pixels; i++)
	{
		int label = _gc->whatLabel(i);		
		CvPoint point = GetPoint(i, _outputSize);
		CvPoint shift = GetShift(label, _shiftSize);
		CvPoint guess = GetLabel(point, _initialGuess);		
		CvPoint pointLabel = cvPoint(shift.x + guess.x, shift.y + guess.y);	

		// test
		//CvPoint mapped = cvPoint(point.x + pointLabel.x, point.y + pointLabel.y);
		//if(IsOutside(mapped, _inputSize))
		//	printf("test");
		 
		SetLabel(point, pointLabel, output);
	}
	return output;
}
