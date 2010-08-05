#pragma once
#include "GCoptimization.h"
#include "EnergyFunctionFeedback.h"
#include "Shiftmap.h"

class FeedbackRetargeting : public ShiftMap
{
public:
	FeedbackRetargeting(void);
	~FeedbackRetargeting(void);
	
	virtual void ComputeShiftMap(IplImage* input, IplImage* saliency, CvSize output, CvSize shiftSize);

};
