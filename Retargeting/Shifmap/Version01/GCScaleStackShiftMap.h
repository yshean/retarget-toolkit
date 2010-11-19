#pragma once
#include "GCAlgorithm.h"
#include "GCScaleStackEnergy.h"
#include "GCScaleStackHorizontalEnergy.h"
#include "GCAImage.h"
#include "GCImageRender.h"


// implementation & setup of scale stack shift map
class GCScaleStackShiftMap
{
public:
	GCScaleStackShiftMap(void);
	~GCScaleStackShiftMap(void);
protected:
	GCImageRender* _render;
	GCAImage* _gcAlgorithm;
	ScaleStackImageSource* _imageSource;
	CvSize _outputSize;
	MappingCubic* _mappingShift;
	MappingCubic* _mappingData;
public:
	IplImage* GetRetargetImage(IplImage* input, IplImage* saliency, CvSize outputSize);
	IplImage* GetRetargetImage2(IplImage* input, IplImage* saliency, ScaleStackImageSource* imageSource, ScaleStackImageSource* saliencySource, CvSize outputSize);

	void ComputeOptimalRetargetMapping(ScaleStackImageSource* imageSource, ScaleStackImageSource* saliencySource, CvSize outputSize);
	// should call only after compute optimal mapping
	IplImage* RenderRetargetImage();
	// render a stack map for visualization of which pixel is mapped to which layer in stack
	IplImage* RenderStackMapVisualisation();
};
