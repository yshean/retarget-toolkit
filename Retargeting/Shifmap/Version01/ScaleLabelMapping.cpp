#include "StdAfx.h"
#include "ScaleLabelMapping.h"

ScaleLabelMapping::ScaleLabelMapping(void)
{
}

ScaleLabelMapping::~ScaleLabelMapping(void)
{
}

int ScaleLabelMapping::CountLevelLabel(vector<int>* labelCountLevel)
{
	int levelCount = labelCountLevel->size();
	int labelCount = 0;
	for(int i = 0; i < levelCount; i++)
	{
		labelCount += (*labelCountLevel)[i];
	}
	return labelCount;
}

void ScaleLabelMapping::InitScaleRange(int inputWidth, int outputWidth, int scaleCount, double scaleStep, vector<int>* labelCountLevel)
{ 	 
	for(int i = 0; i < scaleCount; i++)
	{
		int labelCount = abs(outputWidth - (1 - i*scaleStep) * inputWidth);
		labelCountLevel->push_back(labelCount + 1);	 
	}
}
void ScaleLabelMapping::InitWarpScaleRange(int inputWidth, int outputWidth, vector<double>* scaleList, vector<int>* labelCountLevel, vector<int>* labelMinLevel)
{
	double min = 10000;
	double max = 0;
	int len = scaleList->size();
	_scaleCount = len;
	for(int i = 0; i < len; i++)
	{
		if((*scaleList)[i] < min)
		{
			min = (*scaleList)[i];
		}
		if((*scaleList)[i] > max)
		{
			max = (*scaleList)[i];
		}
	}
	
	int length = scaleList->size();
	double largestScale = (*scaleList)[0]; // normally scale 1 - same size
	double smallestScale = (*scaleList)[length - 1];
	
	for(int i = 0; i < length; i++)
	{
		double currentScale = (*scaleList)[i];
		//int minShift = -60;
		//int maxShift = 60;
		int minShift = - outputWidth + (outputWidth - 1) / largestScale * currentScale + 1;
		int maxShift = - outputWidth + (outputWidth - 1)/ smallestScale * currentScale + 1;

		int currentWidth = inputWidth * currentScale;
		if(maxShift > - outputWidth + currentWidth)
			maxShift = - outputWidth + currentWidth;
	 

		int range = abs(maxShift - minShift) + 1;
		labelCountLevel->push_back(range); 
		if(minShift < maxShift)
			labelMinLevel->push_back(minShift);
		else
			labelMinLevel->push_back(maxShift);
 
	}
}
void ScaleLabelMapping::InitScaleRange(CvSize inputSize, CvSize outputSize, int scaleCount, double scaleStepX, double scaleStepY)
{	 
	_inputSize = inputSize;
	_outputSize = outputSize;
	_scaleCount = scaleCount;
	_scaleStepX = scaleStepX;
	_scaleStepY = scaleStepY;

	_labelCountLevelX = new vector<int>();	
	_labelCountLevelY = new vector<int>();
	
	InitScaleRange(_inputSize.width, _outputSize.width, _scaleCount, _scaleStepX, _labelCountLevelX);
	InitScaleRange(_outputSize.height, _outputSize.height, _scaleCount, _scaleStepY, _labelCountLevelY);

	_labelCountLevel = new vector<int>();
	
	for(int i = 0 ; i < scaleCount; i++)
	{
		int labelCount = (*_labelCountLevelX)[i] * (*_labelCountLevelY)[i];		
		_labelCountLevel->push_back(labelCount);
	} 
}
double ScaleLabelMapping::GetScaleX(int scaleId)
{
	return (*_scaleListX)[scaleId];
}
double ScaleLabelMapping::GetScaleY(int scaleId)
{
	return (*_scaleListY)[scaleId];
}

void ScaleLabelMapping::InitWarpScaleRange(CvSize inputSize, CvSize outputSize, vector<double>* scaleListX, vector<double>* scaleListY)
{
	_scaleListX = scaleListX;
	_scaleListY = scaleListY;

	_labelCountLevelX = new vector<int>();	
	_labelCountLevelY = new vector<int>();
	_labelMinLevelX = new vector<int>();
	_labelMinLevelY = new vector<int>();
	

	InitWarpScaleRange(inputSize.width, outputSize.width, scaleListX, _labelCountLevelX, _labelMinLevelX);
	InitWarpScaleRange(inputSize.height, outputSize.height, scaleListY, _labelCountLevelY, _labelMinLevelY);

	_labelCountLevel = new vector<int>();

	int len = scaleListX->size();
	for(int i = 0 ; i < len; i++)
	{
		int labelCount = (*_labelCountLevelX)[i] * (*_labelCountLevelY)[i];		
		_labelCountLevel->push_back(labelCount);
	} 
}

int ScaleLabelMapping::GetScaleId(int label)
{	 
	return GetScaleId(label, _scaleCount, _labelCountLevel);
}

int ScaleLabelMapping::GetLabelCount()
{
	return CountLevelLabel(_labelCountLevel);
}

int ScaleLabelMapping::GetScaleCount()
{
	return _scaleCount;
}

double ScaleLabelMapping::GetScale(int label)
{
	int scaleId = GetScaleId(label);
	if(scaleId > 0)
		printf("test");
	return (1 - scaleId * _scaleStepX) * (1 - scaleId * _scaleStepY);
}

int ScaleLabelMapping::GetScaleId(int label, int scaleCount, vector<int>* labelCountLevel)
{
	int scaleId;
	int shiftId;
	for(int i = 0; i < scaleCount; i++)
	{
		int labelCount = (*labelCountLevel)[i];
		if(label > labelCount - 1)
		{
			label = label - labelCount;
		}
		else
		{
			scaleId = i;
			shiftId = label;
			break;
		}
	}
	return scaleId;
}
DoublePoint ScaleLabelMapping::GetMappedScalePoint(int labelId, CvPoint point)
{
	int scaleId;
	int shiftId;

	int labelIdTemp = labelId;
	for(int i = 0; i < _scaleCount; i++)
	{
		int labelCount = (*_labelCountLevel)[i];
		if(labelId > labelCount - 1)
		{
			labelId = labelId - labelCount;
		}
		else
		{
			scaleId = i;
			shiftId = labelId;
			break;
		}
	}


	int labelCountX = (*_labelCountLevelX)[scaleId];

	int shiftIdX = shiftId % labelCountX;
	int shiftIdY = shiftId / labelCountX;

	double scaleX = 1 - scaleId * _scaleStepX;
	double scaleY = 1 - scaleId * _scaleStepY;
	int levelWidth = scaleX * _inputSize.width;
	int levelHeight = scaleY * _inputSize.height;

	DoublePoint mappedPoint;

	if(levelWidth < _outputSize.width)
		mappedPoint.x = point.x - shiftIdX;
	else
		mappedPoint.x = point.x + shiftIdX;

	if(levelHeight < _outputSize.height)
		mappedPoint.y = point.y - shiftIdY;
	else
		mappedPoint.y = point.y + shiftIdY;

	mappedPoint.x /= scaleX;
	mappedPoint.y /= scaleY;
	return mappedPoint;
}
DoublePoint ScaleLabelMapping::GetMappedWarpPoint(int labelId, CvPoint point, vector<double>* scaleListX, vector<double>* scaleListY)
{
	int scaleId;
	int shiftId;

	int labelIdTemp = labelId;
	int scaleCount = scaleListX->size();
	
	for(int i = 0; i < scaleCount; i++)
	{
		int labelCount = (*_labelCountLevel)[i];
		if(labelId > labelCount - 1)
		{
			labelId = labelId - labelCount;
		}
		else
		{
			scaleId = i;
			shiftId = labelId;
			break;
		}
	}
	
	int labelCountX = (*_labelCountLevelX)[scaleId];

	int shiftIdX = shiftId % labelCountX;
	int shiftIdY = shiftId / labelCountX;
 
	shiftIdX += (*_labelMinLevelX)[scaleId];
	shiftIdY += (*_labelMinLevelY)[scaleId];

	double scaleX = (*scaleListX)[scaleId];
	double scaleY = (*scaleListY)[scaleId];
	
	DoublePoint result;
	result.x = (point.x + shiftIdX) / scaleX;
	result.y = (point.y + shiftIdY) / scaleY;
	return result;
}

DoublePoint ScaleLabelMapping::GetMappedPoint(int labelId, CvPoint point)
{
	// return GetMappedScalePoint(labelId, point);
	return GetMappedWarpPoint(labelId, point, _scaleListX, _scaleListY);
}

CvPoint ScaleLabelMapping::GetMappedPointInt(int labelId, CvPoint point)
{
	int scaleId;
	int shiftId;

	int labelIdTemp = labelId;
	int scaleCount = _scaleListX->size();
	
	for(int i = 0; i < _scaleCount; i++)
	{
		int labelCount = (*_labelCountLevel)[i];
		if(labelId > labelCount - 1)
		{
			labelId = labelId - labelCount;
		}
		else
		{
			scaleId = i;
			shiftId = labelId;
			break;
		}
	}
	
	int labelCountX = (*_labelCountLevelX)[scaleId];

	int shiftIdX = shiftId % labelCountX;
	int shiftIdY = shiftId / labelCountX;
 
	shiftIdX += (*_labelMinLevelX)[scaleId];
	shiftIdY += (*_labelMinLevelY)[scaleId];
	
	CvPoint result;
	result.x = point.x + shiftIdX;
	result.y = point.y + shiftIdY;
	return result;
}

double ScaleLabelMapping::GetMappedPoint(int label, int x, int scaleStep, int scaleCount, int inputSize, int outputSize, vector<int>* labelCountLevel)
{	
	int scaleId;
	int shiftId;
 
	for(int i = 0; i < scaleCount; i++)
	{
		int labelCount = (*labelCountLevel)[i];
		if(label > labelCount - 1)
		{
			label = label - labelCount;
		}
		else
		{
			scaleId = i;
			shiftId = label;
			break;
		}
	}

	double scale = 1 - scaleId * scaleStep;
 
	int levelWidth = scale * inputSize;
	double mappedX;
	if(levelWidth < outputSize)
		mappedX = x - shiftId;
	else
		mappedX = x + shiftId;
	
	mappedX /= scale;	
	return mappedX;
}

int GetPatchDifference(CvPoint point1, CvPoint point2, int labelId1, int labelId2, IplImage* image, ScaleLabelMapping* labelMapping, int patchSize)
{
	int diff = 0;
	
	for(int i = -patchSize; i < patchSize+1; i++)
		for(int j = -patchSize; j < patchSize+1; j++)
		{
			DoublePoint mappedPoint1 = labelMapping->GetMappedPoint(labelId1, cvPoint(point1.x + i, point1.y + j));
			DoublePoint mappedPoint2 = labelMapping->GetMappedPoint(labelId2, cvPoint(point2.x + i, point2.y + j));
			
			

			diff += GetColorDifference(mappedPoint1, mappedPoint2, image);
		}
	return diff / ((patchSize * 2 + 1)*(patchSize * 2 + 1));
}
