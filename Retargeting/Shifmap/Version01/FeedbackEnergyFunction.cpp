#include "StdAfx.h"
#include "FeedbackEnergyFunction.h"

FeedbackEnergyFunction::FeedbackEnergyFunction(void)
{
}

FeedbackEnergyFunction::~FeedbackEnergyFunction(void)
{
}

void FeedbackEnergyFunction::SetTargetSample(IplImage* image)
{
	_targetSample = image;
}

int FeedbackEnergyFunction::GetDataCost(int labelId, int nodeId)
{
	int penaltyCost = 100000;
	return GetFeedBackDataCost(labelId, nodeId, penaltyCost);
}

int FeedbackEnergyFunction::GetFeedBackDataCost(int labelId, int nodeId, int penaltyCost)
{
	CvPoint point = _mapping2D->GetMappedPoint(nodeId);
	
	DoublePoint mappedPoint = _labelMapping->GetMappedPoint(labelId, point);

	if(mappedPoint.x < 0 || mappedPoint.x > _inputSize.width - 1)
		return penaltyCost;
	if(mappedPoint.y < 0 || mappedPoint.y > _inputSize.height - 1)
		return penaltyCost;


	if(!IsSatisfiedBoundary(point.x, mappedPoint.x, _inputSize.width, _outputSize.width))
		return penaltyCost;
	if(!IsSatisfiedBoundary(point.y, mappedPoint.y, _inputSize.height, _outputSize.height))
		return penaltyCost;
	
	CvScalar value1 = cvGet2D(_targetSample, point.y, point.x);
	CvScalar value2 = GetInterpolatedValue(mappedPoint, _image);
	return SquareDifference(value1, value2);	
}
