#pragma once
#include "GCEnergyFunction.h"
#include "HorizontalScaleLabelMapping.h"
#include "Mapping2D.h"
#include "EnergyFunction.h"
#include "PatchUtils.h"
#include <cv.h>

class HorizontalScaleEnergyFunction: public GCEnergyFunction
{
public:
	HorizontalScaleEnergyFunction(void);
	~HorizontalScaleEnergyFunction(void);
protected:
	HorizontalScaleLabelMapping* _labelMapping;
	Mapping2D* _mapping2D;
	IplImage* _image;
	IplImage* _gradient;
	IplImage* _saliency;
	CvSize _outputSize;
	CvSize _inputSize;
protected:
	bool IsSatisfiedBoundary(int labelId, int nodeId);	
	//CvScalar GetInterpolatedValue(double x, int y, IplImage* image);	
	//int GetColorDifference(double x1, int y1, double x2, int y2, IplImage* image);
	bool IsInside(double x, int width);
public:
	
	void SetInput(IplImage* image, IplImage* gradient, IplImage* saliency);
	void SetRetargetSize(CvSize inputSize, CvSize outputSize);
	void SetLabelMapping(HorizontalScaleLabelMapping* labelMapping);
	void SetMapping2D(Mapping2D* mapping2D);
 
protected:
	// normal data cost with boundary constraint
	virtual int GetDataCostOrigin(int labelId, int nodeId, int penaltyCost);
	// give more weight to the larger image
	virtual int GetDataCostPreferLargerImage(int labelId, int nodeId, int penaltyCost);
public:	
	virtual int GetDataCost(int labelId, int nodeId);
	virtual int GetSmoothCost(int labelId1, int labelId2, int nodeId1, int nodeId2);
protected:
	virtual int GetSmoothCostOrigin(int labelId1, int labelId2, int nodeId1, int nodeId2);
	virtual int GetSmoothCostMin(int labelId1, int labelId2, int nodeId1, int nodeId2);
	virtual int GetSmoothCostPatch(int labelId1, int labelId2, int nodeId1, int nodeId2);
};
