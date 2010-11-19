#include "stdafx.h"
#include "ScaleEnergyFunction.h"


ScaleEnergyFunction::ScaleEnergyFunction(void)
{
	_dataCostType = DATA_ORIGIN;
	_smoothCostType = SMOOTH_ORIGIN;
}

ScaleEnergyFunction::~ScaleEnergyFunction(void)
{
}


void ScaleEnergyFunction::SetRetargetSize(CvSize inputSize, CvSize outputSize)
{
	_inputSize = inputSize;
	_outputSize = outputSize;
}
void ScaleEnergyFunction::SetSmoothThreshold(int threshold)
{
	_smoothThreshold = threshold;
}
void ScaleEnergyFunction::SetInput(IplImage* image, IplImage* gradient, IplImage* saliency)
{
	_image = image;
	_gradient = gradient;
	_saliency = saliency;
}

void ScaleEnergyFunction::SetLabelMapping(ScaleLabelMapping* labelMapping)
{
	_labelMapping = labelMapping;
}

void ScaleEnergyFunction::SetMaxSalLargerImage(int max)
{
	_maxSaliency = max;
}

void ScaleEnergyFunction::SetSmoothCostPatchSize(int patchSize)
{
	_smoothPatchSize = patchSize;
}

void ScaleEnergyFunction::SetMapping2D(Mapping2D* mapping2D)
{
	_mapping2D = mapping2D;
}

bool ScaleEnergyFunction::IsSatisfiedBoundary(int x, double mappedX, int inputSize, int outputSize)
{
	if(x == 0 && mappedX >= 1)
		return false;

	if(x == outputSize - 1)
	{
		if(mappedX < inputSize - 2 || mappedX > inputSize - 1)
			return false;
	}
	return true;
}

int ScaleEnergyFunction::GetSmoothCostPatch(int labelId1, int labelId2, int nodeId1, int nodeId2, int penaltyCost, int patchsize)
{
	CvPoint point1 = _mapping2D->GetMappedPoint(nodeId1);
	CvPoint point2 = _mapping2D->GetMappedPoint(nodeId2);	

	int energy = 0;

	DoublePoint mappedPoint1 = _labelMapping->GetMappedPoint(labelId1, point1);
	DoublePoint mappedPoint2 = _labelMapping->GetMappedPoint(labelId2, point2);
	
	if(!IsInside(mappedPoint1, _inputSize) || !IsInside(mappedPoint2, _inputSize))
		return penaltyCost;

	DoublePoint mappedNeighbor1 = _labelMapping->GetMappedPoint(labelId1, point2);
	DoublePoint mappedNeighbor2 = _labelMapping->GetMappedPoint(labelId2, point1);
	
	if(!IsInside(mappedNeighbor1, _inputSize) || !IsInside(mappedNeighbor2, _inputSize))
		return penaltyCost;

	energy += GetPatchDifference(point2, point2, labelId1, labelId2, _image, _labelMapping, patchsize);
	energy += GetPatchDifference(point1, point1, labelId1, labelId2, _image, _labelMapping, patchsize);
	
	 
	return energy;
}

int ScaleEnergyFunction::GetSmoothCostPreventCrossingPatch(int labelId1, int labelId2, int nodeId1, int nodeId2, int penaltyCost, int threshold, int patchsize)
{	
	CvPoint point1 = _mapping2D->GetMappedPoint(nodeId1);
	CvPoint point2 = _mapping2D->GetMappedPoint(nodeId2);

	int energy = 0; 



	DoublePoint mappedPoint1 = _labelMapping->GetMappedPoint(labelId1, point1);
	DoublePoint mappedPoint2 = _labelMapping->GetMappedPoint(labelId2, point2);
	
	if(!IsInside(mappedPoint1, _inputSize) || !IsInside(mappedPoint2, _inputSize))
		return penaltyCost;

	int scaleId1 = _labelMapping->GetScaleId(labelId1);
	int scaleId2 = _labelMapping->GetScaleId(labelId2);
	if(scaleId1 != scaleId2)
	{
		CvScalar saliency1 = GetInterpolatedValue(mappedPoint1, _saliency);		
		CvScalar saliency2 = GetInterpolatedValue(mappedPoint2, _saliency);

		if(saliency1.val[0] + saliency1.val[1] + saliency1.val[2] > threshold
			&& saliency2.val[0] + saliency2.val[1] + saliency2.val[2] > threshold)
			return penaltyCost;
	}


	DoublePoint mappedNeighbor1 = _labelMapping->GetMappedPoint(labelId1, point2);
	DoublePoint mappedNeighbor2 = _labelMapping->GetMappedPoint(labelId2, point1);

	if(!IsInside(mappedNeighbor1, _inputSize) || !IsInside(mappedNeighbor2, _inputSize))
		return penaltyCost;

	energy += GetPatchDifference(point2, point2, labelId1, labelId2, _image, _labelMapping, patchsize);
	energy += GetPatchDifference(point1, point1, labelId1, labelId2, _image, _labelMapping, patchsize);
	energy += GetPatchDifference(point2, point2, labelId1, labelId2, _gradient, _labelMapping, patchsize);
	energy += GetPatchDifference(point1, point1, labelId1, labelId2, _gradient, _labelMapping, patchsize);

	return energy;
}


int ScaleEnergyFunction::GetSmoothCostPreventCrossing(int labelId1, int labelId2, int nodeId1, int nodeId2, int penaltyCost, int threshold)
{	
	CvPoint point1 = _mapping2D->GetMappedPoint(nodeId1);
	CvPoint point2 = _mapping2D->GetMappedPoint(nodeId2);

	int energy = 0; 



	DoublePoint mappedPoint1 = _labelMapping->GetMappedPoint(labelId1, point1);
	DoublePoint mappedPoint2 = _labelMapping->GetMappedPoint(labelId2, point2);
	
	if(!IsInside(mappedPoint1, _inputSize) || !IsInside(mappedPoint2, _inputSize))
		return penaltyCost;

	int scaleId1 = _labelMapping->GetScaleId(labelId1);
	int scaleId2 = _labelMapping->GetScaleId(labelId2);
	if(scaleId1 != scaleId2)
	{
		CvScalar saliency1 = GetInterpolatedValue(mappedPoint1, _saliency);		
		CvScalar saliency2 = GetInterpolatedValue(mappedPoint2, _saliency);

		if(saliency1.val[0] + saliency1.val[1] + saliency1.val[2] > threshold
			&& saliency2.val[0] + saliency2.val[1] + saliency2.val[2] > threshold)
			return penaltyCost;
	}


	DoublePoint mappedNeighbor1 = _labelMapping->GetMappedPoint(labelId1, point2);
	DoublePoint mappedNeighbor2 = _labelMapping->GetMappedPoint(labelId2, point1);

	if(!IsInside(mappedNeighbor1, _inputSize) || !IsInside(mappedNeighbor2, _inputSize))
		return penaltyCost;

	energy += GetColorDifference(mappedPoint1, mappedNeighbor2, _image);
	energy += GetColorDifference(mappedPoint2, mappedNeighbor1, _image);
	energy += GetColorDifference(mappedPoint1, mappedNeighbor2, _gradient);
	energy += GetColorDifference(mappedPoint2, mappedNeighbor1, _gradient);

	return energy;
}


// normal smooth cost
int ScaleEnergyFunction::GetSmoothCostOrigin(int labelId1, int labelId2, int nodeId1, int nodeId2, int penaltyCost)
{	
	CvPoint point1 = _mapping2D->GetMappedPoint(nodeId1);
	CvPoint point2 = _mapping2D->GetMappedPoint(nodeId2);

	int energy = 0; 

	DoublePoint mappedPoint1 = _labelMapping->GetMappedPoint(labelId1, point1);
	DoublePoint mappedPoint2 = _labelMapping->GetMappedPoint(labelId2, point2);
	
	if(!IsInside(mappedPoint1, _inputSize) || !IsInside(mappedPoint2, _inputSize))
		return penaltyCost;

	DoublePoint mappedNeighbor1 = _labelMapping->GetMappedPoint(labelId1, point2);
	DoublePoint mappedNeighbor2 = _labelMapping->GetMappedPoint(labelId2, point1);

	if(!IsInside(mappedNeighbor1, _inputSize) || !IsInside(mappedNeighbor2, _inputSize))
		return penaltyCost;

	energy += GetColorDifference(mappedPoint1, mappedNeighbor2, _image);
	energy += GetColorDifference(mappedPoint2, mappedNeighbor1, _image);
	energy += GetColorDifference(mappedPoint1, mappedNeighbor2, _gradient);
	energy += GetColorDifference(mappedPoint2, mappedNeighbor1, _gradient);

	return energy;
}

// normal data cost with boundary constraint
int ScaleEnergyFunction::GetDataCostOrigin(int labelId, int nodeId, int penaltyCost)
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

	CvScalar saliency = GetInterpolatedValue(mappedPoint, _saliency);
	return saliency.val[0] + saliency.val[1] + saliency.val[2];
}
int ScaleEnergyFunction::GetDataCostPreferLargerImage(int labelId, int nodeId, int penaltyCost)
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

	CvScalar saliency = GetInterpolatedValue(mappedPoint, _saliency);
	
	double scale = _labelMapping->GetScale(labelId);
	
	return (saliency.val[0] + saliency.val[1] + saliency.val[2]) * scale;
}

int ScaleEnergyFunction::GetDistortionCostPatch(vector<CvScalar*>* points, CvPoint point, IplImage* image, int patchsize)
{
	int distortion = 0;
	for(int i = 0; i < patchsize; i++)
		for(int j = 0; j < patchsize; j++)
		{
			CvScalar* value = (*points)[i * patchsize + j];
			CvScalar value2;
			if(point.x + i < 0 || point.x + i > _inputSize.width - 1
				|| point.y + j < 0 || point.y + j > _inputSize.height - 1)
				value2 = cvScalar(0,0,0);
			else
				value2 = cvGet2D(image, point.y + j, point.x + i);
			distortion += SquareDifference(*value, value2);
		}
	return distortion;
}

int ScaleEnergyFunction::GetDistortionCost(CvPoint point, IplImage* image, int label, int patch_size, double scale)
{
	vector<CvScalar*>* scaled_points = new vector<CvScalar*>(patch_size * patch_size);
	for(int i = 0; i < patch_size; i++)
		for(int j = 0; j < patch_size; j++)
		{
			DoublePoint currPoint = _labelMapping->GetMappedPoint(label, cvPoint(point.x + i, point.y + j));	
			CvScalar* value = new CvScalar();
			*value = GetInterpolatedValue(currPoint, image);
			(*scaled_points)[i * patch_size + j] = value;
		}
	
		
	int scaled_patch_size = patch_size / scale;
	
	DoublePoint mappedPoint = _labelMapping->GetMappedPoint(label, point);
	int x = (int)mappedPoint.x;
	int y = (int)mappedPoint.y;

	int minDistortion = 10000000;
	for(int i = x; i < x + scaled_patch_size - patch_size + 1; i++)
		for(int j = y; j < y + scaled_patch_size - patch_size + 1; j++)
		{
			 
			int distortion = GetDistortionCostPatch(scaled_points, cvPoint(i,j), image, patch_size);
			if(distortion < minDistortion)
				minDistortion = distortion;
		}

	return minDistortion;
}

int ScaleEnergyFunction::GetDataCostAreaCost(int labelId, int nodeId, int penaltyCost)
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

	CvScalar saliency = GetInterpolatedValue(mappedPoint, _saliency);
	
	
	double scale = _labelMapping->GetScale(labelId);
		
	int distortionCost = GetDistortionCost(point, _image, labelId, 3, scale);
	int areaCost = 255 * scale;

	int energy = (_distortionWeight * distortionCost + _areaWeight *  areaCost) / _smoothWeight;
	return energy;
	// return (saliency.val[0] + saliency.val[1] + saliency.val[2]) * scale;
}

void ScaleEnergyFunction::SetupAreaCost(double area, double distortion, double smooth)
{
	_dataCostType = DATA_AREA;
	_smoothCostType = SMOOTH_ORIGIN;

	_areaWeight = area;
	_distortionWeight = distortion;
	_smoothWeight = smooth;
	
}
int ScaleEnergyFunction::GetDataCostReverseSaliency(int labelId, int nodeId, int penaltyCost)
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

	CvScalar saliency = GetInterpolatedValue(mappedPoint, _saliency);
	
	double scale = _labelMapping->GetScale(labelId);
	
	int scaleId = _labelMapping->GetScaleId(labelId);

	//DisplayImage(_saliency, "saliency");

	if(scaleId == 0)
		return saliency.val[0] + saliency.val[1] + saliency.val[2];
	else
		return _maxSaliency - (saliency.val[0] + saliency.val[1] + saliency.val[2]);
}
void ScaleEnergyFunction::SetDataCostType(int type)
{
	_dataCostType = type;
}
void ScaleEnergyFunction::SetSmoothCostType(int type)
{
	_smoothCostType = type;
}

int ScaleEnergyFunction::GetDataCostReverseThreshold(int labelId, int nodeId, int penaltyCost)
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

	CvScalar saliency = GetInterpolatedValue(mappedPoint, _saliency);
	
	double scale = _labelMapping->GetScale(labelId);
	
	int scaleId = _labelMapping->GetScaleId(labelId);

	//DisplayImage(_saliency, "saliency");

	int value = saliency.val[0] + saliency.val[1] + saliency.val[2];
	int threshold = 150 * 3;
	 
	if(value > threshold)
		value = threshold + (value - threshold) * scale;
	else
		value = threshold - (threshold - value) * scale;

	return value;
}

int ScaleEnergyFunction::GetDataCost(int labelId, int nodeId)
{
	int penaltyCost = 500000;

	switch(_dataCostType)
	{
		case DATA_AREA: return GetDataCostAreaCost(labelId, nodeId, penaltyCost);
			break;
		case DATA_ORIGIN: return GetDataCostOrigin(labelId, nodeId, penaltyCost);
			break;
	}
	//return GetDataCostOrigin(labelId, nodeId, penaltyCost);
	// return GetDataCostPreferLargerImage(labelId, nodeId, penaltyCost);
	// return GetDataCostReverseSaliency(labelId, nodeId, penaltyCost);
	//return GetDataCostReverseThreshold(labelId, nodeId, penaltyCost);
}

int ScaleEnergyFunction::GetSmoothCostThreshold(int labelId1, int labelId2, int nodeId1, int nodeId2, int penaltyCost, int smoothThreshold)
{
	int energy = GetSmoothCostOrigin(labelId1, labelId2, nodeId1, nodeId2, penaltyCost);
	
	if(energy < smoothThreshold)
		energy = 0;
	return energy;
}
int ScaleEnergyFunction::GetSmoothCost(int labelId1, int labelId2, int nodeId1, int nodeId2)
{
	int penaltyCost = 500000;
	//return GetSmoothCostThreshold(labelId1, labelId2, nodeId1, nodeId2, penaltyCost, _smoothThreshold);
	return GetSmoothCostOrigin(labelId1, labelId2, nodeId1, nodeId2, penaltyCost);
	// return GetSmoothCostPatch(labelId1, labelId2, nodeId1, nodeId2, penaltyCost, _smoothPatchSize);
	// return GetSmoothCostPreventCrossing(labelId1, labelId2, nodeId1, nodeId2, penaltyCost, 100);
	//return GetSmoothCostPreventCrossingPatch(labelId1, labelId2, nodeId1, nodeId2, penaltyCost, 100, _smoothPatchSize);
	
}