#include "StdAfx.h"
#include "GCScaleStackEnergy.h"

GCScaleStackEnergy::GCScaleStackEnergy(void)
{
}

GCScaleStackEnergy::~GCScaleStackEnergy(void)
{
}
void GCScaleStackEnergy::SetMapping(MappingCubic* data, MappingCubic* shift)
{
	_mappingCubicData = data;
	_mappingCubicShift = shift;
}
void GCScaleStackEnergy::SetSource(ScaleStackImageSource* source, ScaleStackImageSource* saliency)
{
	_imageSource = source;
	_imageSaliency = saliency;
}
 int GCScaleStackEnergy::GetDataCost(int labelId, int nodeId)
 {
	 Point3D nodePoint = _mappingCubicData->GetMappedPoint(nodeId);
	 Point3D shift = _mappingCubicShift->GetMappedPoint(labelId);
     Point3D mappedPoint = point3D(nodePoint.x + shift.x, nodePoint.y + shift.y, nodePoint.z + shift.z);
	 
	 if(!_imageSaliency->IsOutsideSource(mappedPoint))
	 {
		CvScalar saliency = _imageSaliency->GetImageValue(mappedPoint);

		// higher level receive more saliency
		//if(mappedPoint.z != 0)
		//	for(int i = 0; i < 4; i++)
		//	{
		//		saliency.val[i] /= (mappedPoint.z + 1.5);
		//	}
		return saliency.val[0] + saliency.val[1] + saliency.val[2];
	 }
	 else
		 return 100000;
 }
 int GCScaleStackEnergy::GetSmoothCost(int labelId1, int labelId2, int nodeId1, int nodeId2)
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
	
	// scale down the energy
	if(energy < 50) energy = 0;

	return energy;
 }
