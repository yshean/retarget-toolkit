#include "StdAfx.h"
#include "GCScaleStackHorizontalEnergy.h"

GCScaleStackHorizontalEnergy::GCScaleStackHorizontalEnergy(void)
{
}

GCScaleStackHorizontalEnergy::~GCScaleStackHorizontalEnergy(void)
{
}
int GCScaleStackHorizontalEnergy::GetDataCost2(int labelId, int nodeId)
{
Point3D nodePoint = _mappingCubicData->GetMappedPoint(nodeId);
	Point3D shift = _mappingCubicShift->GetMappedPoint(labelId);
	Point3D mappedPoint = point3D(nodePoint.x + shift.x, nodePoint.y + shift.y, nodePoint.z + shift.z);

	int penaltyCost = 100000;

	if(!_imageSaliency->IsOutsideSource(mappedPoint))
	{	 
		// ensure boundary
		if(nodePoint.x == 0 && (mappedPoint.x != 0 || nodePoint.y != mappedPoint.y || mappedPoint.z != 0))
			return penaltyCost;
 
		if(nodePoint.x == _mappingCubicData->GetWidth() - 1
			&& (mappedPoint.x != _imageSource->GetImageSize(0).width - 1 || mappedPoint.y != nodePoint.y) )
			return penaltyCost;
	 
		CvScalar saliency = _imageSaliency->GetImageValue(mappedPoint);

	 
		return saliency.val[0] + saliency.val[1] + saliency.val[2];
	}
	else
		return penaltyCost;
}
int GCScaleStackHorizontalEnergy::GetDataCost3(int labelId, int nodeId)
{
Point3D nodePoint = _mappingCubicData->GetMappedPoint(nodeId);
	Point3D shift = _mappingCubicShift->GetMappedPoint(labelId);
	Point3D mappedPoint = point3D(nodePoint.x + shift.x, nodePoint.y + shift.y, nodePoint.z + shift.z);

	int penaltyCost = 200000;

	if(!_imageSaliency->IsOutsideSource(mappedPoint))
	{	 
		// ensure boundary
		if(nodePoint.x == 0 && (mappedPoint.x != 0 || nodePoint.y != mappedPoint.y || mappedPoint.z != 0))
			return penaltyCost;
 
		if(nodePoint.x == _mappingCubicData->GetWidth() - 1
			&& (mappedPoint.x != _imageSource->GetImageSize(0).width - 1 || mappedPoint.y != nodePoint.y) )
			return penaltyCost;
	 
		CvScalar saliency = _imageSaliency->GetImageValue(mappedPoint);

			//// higher level receive less saliency
			if(mappedPoint.z != 0)
				for(int i = 0; i < 4; i++)
				{
					saliency.val[i] *= (mappedPoint.z + 0.1);
				}
		return saliency.val[0] + saliency.val[1] + saliency.val[2];
	}
	else
		return penaltyCost;
}

int GCScaleStackHorizontalEnergy::GetDataCost1(int labelId, int nodeId)
{
Point3D nodePoint = _mappingCubicData->GetMappedPoint(nodeId);
	Point3D shift = _mappingCubicShift->GetMappedPoint(labelId);
	Point3D mappedPoint = point3D(nodePoint.x + shift.x, nodePoint.y + shift.y, nodePoint.z + shift.z);

	int penaltyCost = 10000;
	if(!_imageSaliency->IsOutsideSource(mappedPoint))
	{	 
	// ensure boundary
	if(nodePoint.x == 0 && (mappedPoint.x != 0 || nodePoint.y != mappedPoint.y))
		return penaltyCost;
	
	// compare with all images in different scales
	int lastColMapped = 0;
	if(nodePoint.x == _mappingCubicData->GetWidth() - 1)
	{
		for(int i = 0; i < _imageSource->GetLevelCount(); i++)
		// for(int i = 0; i < _imageSource->GetLevelCount(); i++)
		{
			if(mappedPoint.x == _imageSource->GetImageSize(i).width - 1 && mappedPoint.y == nodePoint.y && 
				mappedPoint.z == i)
				lastColMapped = 1;
		}
		if(lastColMapped == 0) // no mapped last column
			return penaltyCost;
	}

 
	

		CvScalar saliency = _imageSaliency->GetImageValue(mappedPoint);

		// higher level receive less saliency
		//if(mappedPoint.z != 0)
		//	for(int i = 0; i < 4; i++)
		//	{
		//		saliency.val[i] /= (mappedPoint.z + 2);
		//	}
		return saliency.val[0] + saliency.val[1] + saliency.val[2];
	}
	else
		return penaltyCost;
}
int GCScaleStackHorizontalEnergy::GetDataCost(int labelId, int nodeId)
{
	return GetDataCost1(labelId, nodeId);
}

int GCScaleStackHorizontalEnergy::GetSmoothCost2(int labelId1, int labelId2, int nodeId1, int nodeId2)
{
	Point3D pixelPoint1 = _mappingCubicData->GetMappedPoint(nodeId1);		
	Point3D labelPoint1 = _mappingCubicShift->GetMappedPoint(labelId1);
	Point3D pixelPoint2 = _mappingCubicData->GetMappedPoint(nodeId2);	
	Point3D labelPoint2 = _mappingCubicShift->GetMappedPoint(labelId2); 

	Point3D mappedPoint1 = point3D(pixelPoint1.x + labelPoint1.x, pixelPoint1.y + labelPoint1.y, pixelPoint1.z + labelPoint1.z);
	Point3D mappedPoint2 = point3D(pixelPoint2.x + labelPoint2.x, pixelPoint2.y + labelPoint2.y, pixelPoint2.z + labelPoint2.z);

	if(_imageSource->IsOutsideSource(mappedPoint1) || _imageSource->IsOutsideSource(mappedPoint2))
		return 10000;
	//return 50;
	
	// pre-compute variables:
	Point3D neighbor1 = GetNeighbor(pixelPoint1, pixelPoint2, mappedPoint1); // neighbor of label1
	Point3D neighbor2 = GetNeighbor(pixelPoint2, pixelPoint1, mappedPoint2); // neighbor of label2

	if(pixelPoint1.z != pixelPoint2.z)
		printf("Test");

	int energy = 0;

	if(_imageSource->IsOutsideSource(neighbor2) ||
		_imageSource->IsOutsideSource(neighbor1))
		return 10000;

	energy += _imageSource->GetSquareColorDifference(mappedPoint1, neighbor2);
	energy += _imageSource->GetSquareColorDifference(mappedPoint2, neighbor1);  

	// gradient different term 
	energy += 2 * _imageSource->GetSquareGradientDifference(mappedPoint2, neighbor1);
	energy += 2 * _imageSource->GetSquareGradientDifference(mappedPoint1, neighbor2);
	
	//***** plus a term for real distance
	CvSize firstLevel = _imageSource->GetImageSize(0);
	CvSize secondLevel = _imageSource->GetImageSize(1);
	// first put 2 mappedPoint to same scale
	if(mappedPoint1.z == 1)
	{
		mappedPoint1.x = mappedPoint1.x * firstLevel.width / secondLevel.width;
		mappedPoint1.y = mappedPoint1.y * firstLevel.height / secondLevel.height;			
	}
	if(mappedPoint2.z == 1)
	{
		mappedPoint2.x = mappedPoint2.x * firstLevel.width / secondLevel.width;
		mappedPoint2.y = mappedPoint2.y * firstLevel.height / secondLevel.height;			
	}

	// take distance
	int distance = pow((mappedPoint1.x - mappedPoint2.x),2.0) + pow((mappedPoint2.y - mappedPoint1.y),2.0);
	energy += distance * 10;

	return energy;
}
