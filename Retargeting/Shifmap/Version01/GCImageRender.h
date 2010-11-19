#pragma once
#include <cv.h>
#include "GCOptimization.h"
#include "LabelMapper.h"
#include "ScaleStackImageSource.h"
#include "MappingCubic.h"
#include "DebugTool.h"
// rendering the graph-cut result
class GCImageRender
{
public:
	GCImageRender(void);
	~GCImageRender(void);
protected:
	int _numSites;
	GCoptimization* _gc;
	LabelMapper* _mapper;
	ScaleStackImageSource* _imageSource;
	
	int _width;
	int _height;
public:
	void SetOutputSize(int width, int height);
	void Initialize(LabelMapper* mapper, GCoptimization* gc, ScaleStackImageSource* imageSource);
	IplImage* GetRenderedImage();
	// visualize which pxiel is mapped to which layer
	IplImage* GetVisualizedImage();
	
};
