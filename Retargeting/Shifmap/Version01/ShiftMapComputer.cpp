#include "StdAfx.h"
#include "ShiftMapComputer.h"

ShiftMapComputer::ShiftMapComputer( )
{
 
}

ShiftMapComputer::~ShiftMapComputer(void)
{	
}

void ShiftMapComputer::ClearGC()
{
	delete _gc;
}
IplImage* ShiftMapComputer::GetOriginalImage(int level)
{
	return (*_imageList)[level];
}

void ShiftMapComputer::DownSampling(IplImage* source, IplImage* dst)
{ 
	IplImage* blur = cvCloneImage(source);
	cvSmooth(source, blur);
	for(int i = 0; i < dst->width; i++)
		for(int j = 0; j < dst->height; j++)
		{
			CvScalar value = cvGet2D(blur, j*2, i*2);
			cvSet2D(dst, j, i, value);
		} 
}

int  ShiftMapComputer::GetLevelCount()
{
	return _level;
}
void ShiftMapComputer::ComputeFastShiftMap(IplImage* input, IplImage* saliency, CvSize output)
{
	printf("Downsampling...");
	_imageList = new vector<IplImage*>(0);
	_labelMapList = new vector<IplImage*>(0);
	vector<IplImage*>* imageSList = new vector<IplImage*>(0);
	vector<CvSize>* outputSizeList = new vector<CvSize>(0);
	_imageList->push_back(input);
	imageSList->push_back(saliency);
	outputSizeList->push_back(output);

	IplImage* level = input;
	IplImage* levelS = saliency;	
	CvSize levelSize = output;

	int levelCount = 0;

	while(level->width >40 && level->height > 40)
	{
		IplImage* level_temp = cvCreateImage(cvSize(level->width/2, level->height/2), input->depth, input->nChannels);
		IplImage* levelS_temp = cvCreateImage(cvSize(level->width/2, level->height/2), input->depth, input->nChannels);
		
		DownSampling(level, level_temp);
		DownSampling(levelS, levelS_temp);		
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
	 _initialGuess = cvCreateImage(cvSize(10,10), IPL_DEPTH_8U, 3);
	
	 _level = levelCount;
	//int index = levelCount;
	for(int i = levelCount; i >= 0; i--)
	{
		if(i == levelCount)		
			// first level does not require an initialGuess
			ComputeShiftMap((*_imageList)[i], (*imageSList)[i], (*outputSizeList)[i], cvSize((*_imageList)[i]->width, (*_imageList)[i]->height));
		else
			ComputeShiftMap((*_imageList)[i], (*imageSList)[i], _initialGuess, (*outputSizeList)[i], cvSize(3,3));
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
		IplImage* labelMap;
		if(i == levelCount)
			labelMap = CalculateLabelMap();
		else
			labelMap = CalculateLabelMap2();

		_labelMapList->insert(_labelMapList->begin(), labelMap);
		if(i > 0)
		{	
			cvReleaseImage(&_initialGuess);	
			_initialGuess = cvCreateImage((*outputSizeList)[i-1], IPL_DEPTH_8U, 3);
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
void ShiftMapComputer::ComputeShiftMap(IplImage* input, IplImage* saliency, IplImage* initialGuess, CvSize output, CvSize shiftSize)
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
void ShiftMapComputer::ComputeShiftMap(IplImage* input, IplImage* saliency, CvSize output, CvSize shiftSize)
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

//IplImage* ShiftMapComputer::GetLabelMap()
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
	
IplImage* ShiftMapComputer::GetRetargetImage(int level)
{
 	IplImage* labelMap = (*_labelMapList)[level];
	// IplImage* image = cvCreateImage(cvSize(labelMap->width,labelMap->height), IPL_DEPTH_8U, 3);
	
	return GetImageFromLabelMap(labelMap, (*_imageList)[level]);	
}

IplImage* ShiftMapComputer::GetRetargetImageH()
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

IplImage* ShiftMapComputer::CalculateRetargetImage()
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

IplImage* ShiftMapComputer::CalculateLabelMap()
{
	IplImage* output = cvCreateImage(_outputSize, _input->depth, _input->nChannels);
	int num_pixels = _outputSize.width * _outputSize.height;

	printf("Getting label map... \n");
	for(int i = 0; i < num_pixels; i++)
	{
		int label = _gc->whatLabel(i);				
		CvPoint point = GetPoint(i, _outputSize);		
		CvPoint pointLabel = GetShift(label, _shiftSize);
		
		// test
		CvPoint mapped = cvPoint(point.x + pointLabel.x, point.y + pointLabel.y);
		if(IsOutside(mapped, _inputSize))
			printf("test");

		SetLabel(point, pointLabel, output);
	}
	return output;
}
IplImage* ShiftMapComputer::CalculateLabelMap2()
{
	IplImage* output = cvCreateImage(_outputSize, _input->depth, _input->nChannels);
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
		CvPoint mapped = cvPoint(point.x + pointLabel.x, point.y + pointLabel.y);
		if(IsOutside(mapped, _inputSize))
			printf("test");

		SetLabel(point, pointLabel, output);
	}
	return output;
}


IplImage* ShiftMapComputer::GetLabelMap(int level)
{
	return (*_labelMapList)[level];
}

void ShiftMapComputer::GetInterpolationMap(IplImage* lowerMap, IplImage* higherMap)
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

IplImage* ShiftMapComputer::GetImageFromLabelMap(IplImage* map, IplImage* image)
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
