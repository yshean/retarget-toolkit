#pragma once
#include "ShiftMap.h"
#include "ClusterMap.h"
#include "Label.h"
#include <vector>

class ClusterShiftMap : public ShiftMap
{
public:
	ClusterShiftMap(void);
	~ClusterShiftMap(void);

	virtual void ComputeShiftMap(IplImage* input, IplImage* saliency, IplImage* mask, CvSize output, CvSize shiftSize);	
	
public:
	// scan saliency map to produce a cluster output map
	// which is used as the graph for the GCOptimization
protected:
	IplImage* CreateMask(IplImage* saliency, CvSize output);
	GCoptimizationGeneralGraph* _gcGeneral;
};
