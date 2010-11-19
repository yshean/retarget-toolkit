#include "StdAfx.h"
#include "HorizontalScaleEnergyFunction.h"

HorizontalScaleEnergyFunction::HorizontalScaleEnergyFunction(void)
{
}

HorizontalScaleEnergyFunction::~HorizontalScaleEnergyFunction(void)
{
}
//CvScalar HorizontalScaleEnergyFunction::GetInterpolatedValue(double x1, int y, IplImage* image)
//{	
//	int x1Int = (int)floor(x1);
//	double weight = x1 - x1Int;
//
//	CvScalar value1;
//	CvScalar value2;
//
//	if(x1Int >= image->width)
//		return cvScalar(255);
//
//	if(weight != 0)
//	{
//		value1 = cvGet2D(image, y, x1Int);
//	    value2 = cvGet2D(image, y, x1Int + 1);
//	}
//	else
//	{
//		value1 = cvGet2D(image, y, x1Int);
//		value2 = value1;
//	}
//	CvScalar value;
// 
//	for(int i = 0; i < 4; i++)
//	{
//		value.val[i] = value1.val[i] * (1-weight) + value2.val[i] * weight;
//	}
//	
//	return value;
//}
void HorizontalScaleEnergyFunction::SetLabelMapping(HorizontalScaleLabelMapping* labelMapping)
{
	_labelMapping = labelMapping;
}
void HorizontalScaleEnergyFunction::SetMapping2D(Mapping2D* mapping2D)
{
	_mapping2D = mapping2D;
}
bool HorizontalScaleEnergyFunction::IsSatisfiedBoundary(int labelId, int nodeId)
{
	CvPoint point = _mapping2D->GetMappedPoint(nodeId);
	double mappedX = _labelMapping->GetMappedPoint(labelId, point.x);	
 
	if(point.x == 0 && mappedX >= 1)
		return false;

	if(point.x == _outputSize.width - 1)
	{
		if(mappedX < _inputSize.width - 2 || mappedX > _inputSize.width - 1)
			return false;
	}
	return true;
}

void HorizontalScaleEnergyFunction::SetRetargetSize(CvSize inputSize, CvSize outputSize)
{
	_inputSize = inputSize;
	_outputSize = outputSize;
}

void HorizontalScaleEnergyFunction::SetInput(IplImage* image, IplImage* gradient, IplImage* saliency)
{
	_image = image;
	_gradient = gradient;
	_saliency = saliency;
}
int HorizontalScaleEnergyFunction::GetDataCostOrigin(int labelId, int nodeId, int penaltyCost)
{
	CvPoint point = _mapping2D->GetMappedPoint(nodeId);
	double mappedX = _labelMapping->GetMappedPoint(labelId, point.x);
	 
	

	if(mappedX < 0 || mappedX > _inputSize.width - 1)
		return penaltyCost;

	if(!IsSatisfiedBoundary(labelId, nodeId))
		return penaltyCost;
	
 

	CvScalar saliency = GetInterpolatedValue(mappedX, point.y, _saliency);
	return saliency.val[0] + saliency.val[1] + saliency.val[2];
}

int HorizontalScaleEnergyFunction::GetDataCostPreferLargerImage(int labelId, int nodeId, int penaltyCost)
{
	CvPoint point = _mapping2D->GetMappedPoint(nodeId);
	double mappedX = _labelMapping->GetMappedPoint(labelId, point.x);
	
	

	if(mappedX < 0 || mappedX > _inputSize.width - 1)
		return penaltyCost;

	if(!IsSatisfiedBoundary(labelId, nodeId))
		return penaltyCost;
	
	double scale = _labelMapping->GetScale(labelId);
	int scaleCount = _labelMapping->GetScaleCount();
	
	CvScalar saliency = GetInterpolatedValue(mappedX, point.y, _saliency);

	return saliency.val[0] + saliency.val[1] + saliency.val[2] + scaleCount * 100 * (1 - scale);
}


//int HorizontalScaleEnergyFunction::GetColorDifference(double x1, int y1, double x2, int y2, IplImage* image)
//{
//	CvScalar value1 = GetInterpolatedValue(x1, y1, image);
//	CvScalar value2 = GetInterpolatedValue(x2, y2, image);
//	return SquareDifference(value1, value2);
//}
bool HorizontalScaleEnergyFunction::IsInside(double x, int width)
{
	if (x < 0 || x > width - 1)
		return false;
	return true;
}
int HorizontalScaleEnergyFunction::GetSmoothCostMin(int labelId1, int labelId2, int nodeId1, int nodeId2)
{
	CvPoint point1 = _mapping2D->GetMappedPoint(nodeId1);
	CvPoint point2 = _mapping2D->GetMappedPoint(nodeId2);

	int energy = 0;
	int penaltyCost = 200000;

	double x1 = _labelMapping->GetMappedPoint(labelId1, point1.x);
	double x2 = _labelMapping->GetMappedPoint(labelId2, point2.x);

	double x1Neighbor;
	double x2Neighbor;

	x1Neighbor = _labelMapping->GetMappedPoint(labelId1, point2.x);
	x2Neighbor = _labelMapping->GetMappedPoint(labelId2, point1.x);
	
	if(!IsInside(x1, _inputSize.width) || !IsInside(x2, _inputSize.width)
		|| !IsInside(x1Neighbor, _inputSize.width) || !IsInside(x2Neighbor, _inputSize.width))
		return penaltyCost;
	
	int minEnergy = GetColorDifference(x1, point1.y, x2Neighbor, point1.y, _image);
	int compareEnergy = GetColorDifference(x2, point2.y, x1Neighbor, point2.y, _image);
	if(minEnergy < compareEnergy)
		energy += minEnergy;
	else
		energy += compareEnergy;

	minEnergy = GetColorDifference(x1, point1.y, x2Neighbor, point1.y, _gradient);
	compareEnergy = GetColorDifference(x2, point2.y, x1Neighbor, point2.y, _gradient);

	if(minEnergy < compareEnergy)
		energy += minEnergy;
	else
		energy += compareEnergy;


	return energy;
}
int HorizontalScaleEnergyFunction::GetSmoothCostOrigin(int labelId1, int labelId2, int nodeId1, int nodeId2)
{
	CvPoint point1 = _mapping2D->GetMappedPoint(nodeId1);
	CvPoint point2 = _mapping2D->GetMappedPoint(nodeId2);

	int energy = 0;
	int penaltyCost = 200000;

	double x1 = _labelMapping->GetMappedPoint(labelId1, point1.x);
	double x2 = _labelMapping->GetMappedPoint(labelId2, point2.x);

	double x1Neighbor;
	double x2Neighbor;

	x1Neighbor = _labelMapping->GetMappedPoint(labelId1, point2.x);
	x2Neighbor = _labelMapping->GetMappedPoint(labelId2, point1.x);
	
	if(!IsInside(x1, _inputSize.width) || !IsInside(x2, _inputSize.width)
		|| !IsInside(x1Neighbor, _inputSize.width) || !IsInside(x2Neighbor, _inputSize.width))
		return penaltyCost;
	
	energy += GetColorDifference(x1, point1.y, x2Neighbor, point1.y, _image);
	energy += GetColorDifference(x2, point2.y, x1Neighbor, point2.y, _image);
	energy += GetColorDifference(x1, point1.y, x2Neighbor, point1.y, _gradient);
	energy += GetColorDifference(x2, point2.y, x1Neighbor, point2.y, _gradient);

	return energy;
}
int HorizontalScaleEnergyFunction::GetSmoothCostPatch(int labelId1, int labelId2, int nodeId1, int nodeId2)
{
	CvPoint point1 = _mapping2D->GetMappedPoint(nodeId1);
	CvPoint point2 = _mapping2D->GetMappedPoint(nodeId2);

	int energy = 0;
	int penaltyCost = 200000;

	double x1 = _labelMapping->GetMappedPoint(labelId1, point1.x);
	double x2 = _labelMapping->GetMappedPoint(labelId2, point2.x);

	double x1Neighbor = _labelMapping->GetMappedPoint(labelId1, point2.x);
	double x2Neighbor = _labelMapping->GetMappedPoint(labelId2, point1.x);
	
	if(!IsInside(x1, _inputSize.width) || !IsInside(x2, _inputSize.width)
		|| !IsInside(x1Neighbor, _inputSize.width) || !IsInside(x2Neighbor, _inputSize.width))
		return penaltyCost;

	energy += GetPatchDifference(point2, point2, labelId1, labelId2, _image, _labelMapping);
	energy += GetPatchDifference(point1, point1, labelId1, labelId2, _image, _labelMapping);
	
	 
	return energy;
}
int HorizontalScaleEnergyFunction::GetSmoothCost(int labelId1, int labelId2, int nodeId1, int nodeId2)
{
	//return GetSmoothCostOrigin(labelId1, labelId2, nodeId1, nodeId2);	
	//return GetSmoothCostPatch(labelId1, labelId2, nodeId1, nodeId2);	
	return GetSmoothCostMin(labelId1, labelId2, nodeId1, nodeId2);	
}

int HorizontalScaleEnergyFunction::GetDataCost(int labelId, int nodeId)
{
	int penaltyCost = 500000;
	int energy;
	//energy = GetDataCostOrigin(labelId, nodeId, penaltyCost);
	energy = GetDataCostOrigin(labelId, nodeId, penaltyCost);
	// energy = GetDataCostPreferLargerImage(labelId, nodeId, penaltyCost);
	return energy;
}