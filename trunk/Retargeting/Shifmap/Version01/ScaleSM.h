#pragma once

#include "ScaleLabelMapping.h"
#include "ScaleEnergyFunction.h"
#include "GCScaleImage.h"

class ScaleSM
{
public:
	ScaleSM(void);
	~ScaleSM(void);
// inside algorithm
protected:
	GCAlgorithm* _gc;
	ScaleEnergyFunction* _energyFunction;
	Mapping2D* _mapping2D;
	ScaleLabelMapping* _labelMapping;
// setting
protected:
	int _scaleCount;
	double _scaleStepX;
	double _scaleStepY;
	IplImage* _input;
	CvSize _outputSize;
	int _smoothThreshold;

// public setting
public:
	void SetSmoothThreshold(int threshold);
	// how many scale is taken into consideration
	void SetScaleSetting(int scaleCount, double scaleStepX, double scaleStepY);
public:

	void SetEnergyFunction(ScaleEnergyFunction* energyFunction);
	void SetLabelMapping(ScaleLabelMapping* labelMapping);
	void SetMapping2D(Mapping2D* mapping2D);
	void SetMaxSaliency(int max);
	void InitGraphCut(IplImage* image, IplImage* saliency, CvSize outputSize);
	void ComputeGraphCut();
	void ComputeOptimalRetargetMapping(IplImage* image, IplImage* saliency, CvSize outputSize);
	// should call only after compute optimal mapping
	IplImage* RenderRetargetImage();
	// render a stack map for visualization of which pixel is mapped to which layer in stack
	IplImage* RenderStackMapVisualisation();
};
