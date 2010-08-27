#pragma once
#include <cv.h>
#include "Common3D.h"
#include "VideoSequence.h"
// compute shiftmap in 3D for video
class Shiftmap3D
{
public:
	Shiftmap3D(void);
	~Shiftmap3D(void);

	void ComputeShiftMap(VideoSequence* input, VideoSequence* saliency, Point3D shiftSize);
};
