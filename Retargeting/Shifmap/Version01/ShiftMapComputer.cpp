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
void ShiftMapComputer::ComputeFastShiftMap(IplImage* input, IplImage* saliency, CvSize output)
{
	printf("Downsampling...");
	vector<IplImage*>* imageList = new vector<IplImage*>(0);
	vector<IplImage*>* imageSList = new vector<IplImage*>(0);
	vector<CvSize>* outputSizeList = new vector<CvSize>(0);
	imageList->push_back(input);
	imageSList->push_back(saliency);
	outputSizeList->push_back(output);

	IplImage* level = input;
	IplImage* levelS = saliency;	
	CvSize levelSize = output;

	int levelCount = 0;

	while(level->width >70 && level->height > 70)
	{
		IplImage* level_temp = cvCreateImage(cvSize(level->width/2, level->height/2), input->depth, input->nChannels);
		IplImage* levelS_temp = cvCreateImage(cvSize(level->width/2, level->height/2), input->depth, input->nChannels);
		
		cvPyrDown(level, level_temp);
		cvPyrDown(levelS, levelS_temp);		
		level = level_temp;
		levelS = levelS_temp; 
		levelCount++;

		CvSize output_temp;
		output_temp.width = levelSize.width / 2;
		output_temp.height = levelSize.height / 2;
		levelSize = output_temp;
		
		imageList->push_back(level);
		imageSList->push_back(levelS);
		outputSizeList->push_back(levelSize);
	}
 
	
	// build initial guess
	IplImage* initialGuess = cvCreateImage(cvSize((*outputSizeList)[levelCount].width, (*outputSizeList)[levelCount].height), IPL_DEPTH_8U, 3);
	for(int i = 0; i < level->width; i++)
		for(int j = 0; j < level->height; j++)
		{
			SetLabel(cvPoint(i,j), cvPoint(i,j), initialGuess);
		}

		int index = levelCount;
	for(int i = levelCount; i >= 0; i--)
	{
		if(i == levelCount)		
			ComputeShiftMap((*imageList)[i], (*imageSList)[i], initialGuess, (*outputSizeList)[i], cvSize((*imageList)[i]->width, (*imageList)[i]->height));
		else
			ComputeShiftMap((*imageList)[i], (*imageSList)[i], initialGuess, (*outputSizeList)[i], cvSize(3,3));
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
		if(i > 0)
		{
			IplImage* labelMap = GetLabelMap();
			cvReleaseImage(&initialGuess);
			initialGuess = cvCreateImage((*outputSizeList)[i-1], IPL_DEPTH_8U, 3);
			GetInterpolationMap(labelMap, initialGuess);
			
			if(i == index)
			{
				IplImage* image = GetImageFromLabelMap(labelMap, (*imageList)[i]);
				//IplImage* image = (*imageList)[0];
				cvNamedWindow("test");
				while(1)
				{
					cvShowImage("test", image);
					int key = cvWaitKey(100);
					if(key == 32)
					{
						index--;
						break;
					}
				}
			}

			cvReleaseImage(&labelMap);

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
		ForDataFunction dataFn;
		dataFn.inHeight = _inputSize.height;
		dataFn.inWidth = _inputSize.width;
		dataFn.outWidth = _outputSize.width;
		dataFn.outHeight = _outputSize.height;
		dataFn.shiftWidth = _shiftSize.width;
		dataFn.shiftHeight = _shiftSize.height;
		dataFn.saliency = saliency;
		dataFn.initialGuess = initialGuess;
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
		smoothFn.initialGuess = initialGuess;
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
		CvPoint guess = GetLabel(point, _initialGuess);
		CvPoint pointLabel = GetMappedPoint(guess, label, _shiftSize.width, _shiftSize.height);
		//printf("*%i %i %i %i", point.x, point.y, pointLabel.x, pointLabel.y);
		if(!IsOutside(pointLabel, _inputSize.width, _inputSize.height))
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

IplImage* ShiftMapComputer::GetLabelMap()
{
	IplImage* output = cvCreateImage(_outputSize, _input->depth, _input->nChannels);
	int num_pixels = _outputSize.width * _outputSize.height;

	printf("Getting label map... \n");
	for(int i = 0; i < num_pixels; i++)
	{
		int label = _gc->whatLabel(i);		
		CvPoint point = GetPoint(i, _outputSize.width, _outputSize.height);
		CvPoint guess = GetLabel(point, _initialGuess);
		CvPoint pointLabel = GetMappedPoint(guess, label, _shiftSize.width, _shiftSize.height);
		SetLabel(point, pointLabel, output);
	}
	return output;
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
			CvScalar value = cvGet2D(image, label.y, label.x);
			cvSet2D(output, j, i, value);
		}
	return output;
}
