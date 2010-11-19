#pragma once
#include <cv.h>

class PyramidLabelMapping
{
public:
	PyramidLabelMapping(void);
	~PyramidLabelMapping(void);
public: 
	 

public:
	CvScalar GetMappedPoint(int labelId, int nodeId);
	CvScalar GetMappedNeighbor(int labelId1, int nodeId1, int nodeId2);
	int GetDataCost(int labelId, int nodeId);
	int GetSmoothCost(int labelId1, int label2, int nodeI1, int nodeId2);
};
