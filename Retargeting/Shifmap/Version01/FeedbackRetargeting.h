#pragma once
#include "GCoptimization.h"
#include "EnergyFunctionFeedback.h"
#include "ShiftmapComputer.h"

class FeedbackRetargeting : public ShiftMap
{
public:
	FeedbackRetargeting(void);
	~FeedbackRetargeting(void);
	
	virtual void ComputeShiftMap(IplImage* input, IplImage* saliency, CvSize output, CvSize shiftSize);

};
