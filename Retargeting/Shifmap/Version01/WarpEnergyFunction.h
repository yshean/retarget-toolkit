#pragma once
#include "DebugTool.h"
#include "ScaleEnergyFunction.h"

class WarpEnergyFunction : public ScaleEnergyFunction
{
public:
	WarpEnergyFunction(void);
	~WarpEnergyFunction(void);
protected:
	vector<CvMat*>* _distortionList;
	vector<CvMat*>* _importanceList;
	// CvMat* _importanceMap;
public:
	virtual int GetDataCost(int labelId, int nodeId);
	virtual int GetSmoothCost(int labelId1, int labelId2, int nodeId1, int nodeId2);

	void IntializeDistortionMeasure();	
	void InitializeImportanceMap();
	void InitializeDistortionMeasure(IplImage* saliency);
public:
	double GetScaleDistortion(IplImage* image, IplImage* image2, CvPoint point, double scaleX, double scaleY, int patchSize);
	int OverlapNumber(CvPoint point, int patchSize, int width, int height);
	CvMat* GetDistortionMat(IplImage* image, double scaleX, double scaleY);
	virtual int GetSmoothCostWarp(int labelId1, int labelId2, int nodeId1, int nodeId2, int penaltyCost, int pixelDistance);

protected:
	int GetSmoothCostPreserveObject(int labelId1, int labelId2, int nodeId1, int nodeId2, int penaltyCost);
	int GetSmoothCostGradient(int labelId1, int labelId2, int nodeId1, int nodeId2, int penaltyCost);
	int GetDataWarpDistort(int labelId, int nodeId, int penaltyCost);
};
