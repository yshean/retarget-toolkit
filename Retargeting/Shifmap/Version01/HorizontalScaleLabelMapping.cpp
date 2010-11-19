#include "StdAfx.h"
#include "HorizontalScaleLabelMapping.h"

HorizontalScaleLabelMapping::HorizontalScaleLabelMapping(void)
{
}

HorizontalScaleLabelMapping::~HorizontalScaleLabelMapping(void)
{
}

void HorizontalScaleLabelMapping::InitShiftRange(int shiftRange)
{
	_shiftRange = shiftRange;
}
int HorizontalScaleLabelMapping::GetScaleCount()
{
	return _scaleCount;
}
int HorizontalScaleLabelMapping::GetLabelCount()
{
	return _labelCount;
}
double HorizontalScaleLabelMapping::GetMappedPoint(int label, int x)
{	
	int scaleId;
	int shiftId;
 
	for(int i = 0; i < _scaleCount; i++)
	{
		int labelCount = (*_labelCountLevel)[i];
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

	double scale = 1 - scaleId * _scaleStep;
 
	int levelWidth = scale * _inputWidth;
	double mappedX;
	if(levelWidth < _outputWidth)
		mappedX = x - shiftId;
	else
		mappedX = x + shiftId;
	
	mappedX /= scale;	
	return mappedX;
}

int HorizontalScaleLabelMapping::GetShiftId(int label)
{
	int scaleId;
	int shiftId;
	for(int i = 0; i < _scaleCount; i++)
	{
		int labelCount = (*_labelCountLevel)[i];
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
	return shiftId;
}

double HorizontalScaleLabelMapping::GetScale(int label)
{
	int scaleId = GetScaleId(label);
	return 1 - scaleId * _scaleStep;
}

int HorizontalScaleLabelMapping::GetScaleId(int label)
{ 
	int scaleId;
	int shiftId;
	for(int i = 0; i < _scaleCount; i++)
	{
		int labelCount = (*_labelCountLevel)[i];
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

void HorizontalScaleLabelMapping::InitScaleRange(int inputWidth, int outputWidth, int scaleCount, double scaleStep)
{	
	_scaleStep = scaleStep;
	_inputWidth = inputWidth;
	_outputWidth = outputWidth;

	_labelCountLevel = new vector<int>();	
	_labelCount = 0;
	_scaleCount = 0;	
	for(int i = 0; i < scaleCount; i++)
	{
		int labelCount = abs(outputWidth - (1 - i*scaleStep) * inputWidth);
		_labelCountLevel->push_back(labelCount + 1);
		_labelCount += labelCount + 1;
		_scaleCount++;
	}
}