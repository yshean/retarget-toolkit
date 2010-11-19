#pragma once
#include "GCScaleImage.h"
#include "HorizontalScaleEnergyFunction.h"
#include "HorizontalScaleLabelMapping.h"
#include "GCImageRender.h"

class HorizontalScaleSM
{
public:
	HorizontalScaleSM(void);
	~HorizontalScaleSM(void);

protected:
	int _scaleCount;
	double _scaleStep;
	int _shiftRange;
	GCAlgorithm* _gc;
	HorizontalScaleLabelMapping* _labelMapping;
	IplImage* _input;
	CvSize _outputSize;
	Mapping2D* _mapping2D;
	HorizontalScaleEnergyFunction* _energyFunction;
protected:
	CvScalar GetInterpolatedValue(double x1, int y, IplImage* image);
public:
	// how many scale is taken into consideration
	void SetScaleSetting(int scaleCount, double scaleStep);
	void SetShiftRange(int shiftRange);
public:
	void ComputeOptimalRetargetMapping(IplImage* image, IplImage* saliency, CvSize outputSize);
	// should call only after compute optimal mapping
	IplImage* RenderRetargetImage();
	// render a stack map for visualization of which pixel is mapped to which layer in stack
	IplImage* RenderStackMapVisualisation();
	// render a smooth cost visualization
	string GetCost();
	IplImage* RenderSmoothCostMapVisualisation();
};
