#pragma once
#include "GCEnergyFunction.h"
#include "ScaleLabelMapping.h"
#include "Mapping2D.h"
#include "PatchUtils.h"
#include "DebugTool.h"

#define SMOOTH_ORIGIN 1



#define DATA_AREA 1
#define DATA_ORIGIN 2

class ScaleEnergyFunction: public GCEnergyFunction
{
public:
	ScaleEnergyFunction(void);
	~ScaleEnergyFunction(void);
// inside algorithm
protected:
	Mapping2D* _mapping2D;
	ScaleLabelMapping* _labelMapping;
// setting
protected:
	CvSize _inputSize;
	CvSize _outputSize;
	IplImage* _saliency;
	IplImage* _image;
	IplImage* _gradient;	
	int _maxSaliency;

	IplImage* _areaCostImage;

	int _dataCostType;
	int _smoothCostType;

	int _smoothPatchSize;
	int _smoothThreshold;

	 

#pragma region area weight
	double _areaWeight;
	double _distortionWeight;
	double _smoothWeight;
#pragma endregion
// configuration public function
public:
	void SetSmoothThreshold(int threshold);
	void SetInput(IplImage* image, IplImage* gradient, IplImage* saliency);
	void SetRetargetSize(CvSize inputSize, CvSize outputSize);
	void SetLabelMapping(ScaleLabelMapping* labelMapping);
	void SetMapping2D(Mapping2D* mapping2D);
	void SetMaxSalLargerImage(int max);
	void SetSmoothCostPatchSize(int patchSize);
	void SetDataCostType(int type);
	void SetSmoothCostType(int type);
	void InitAreaCost();
	void SetupAreaCost(double area, double distrotion, double smooth);	
public:	
	virtual int GetDataCost(int labelId, int nodeId);
	virtual int GetSmoothCost(int labelId1, int labelId2, int nodeId1, int nodeId2);
protected:
	virtual int GetSmoothCostThreshold(int labelId1, int labelId2, int nodeId1, int nodeId2, int penaltyCost, int smoothThreshold);

	// pixel distance: max distance neighbor can have
	virtual int GetSmoothCostWarpOnly(int labelId1, int labelId2, int nodeI1, int nodeId2, int penaltyCost, int pixelDistance);
	
	// smooth cost between patch
	virtual int GetSmoothCostPatch(int labelId1, int labelId2, int nodeId1, int nodeId2, int penaltyCost, int patchsize);
		
	// smooth cost prevent crossing in between important object
	virtual int GetSmoothCostPreventCrossing(int labelId1, int labelId2, int nodeId1, int nodeId2, int penaltyCost, int threshold);

	virtual int GetSmoothCostPreventCrossingPatch(int labelId1, int labelId2, int nodeId1, int nodeId2, int penaltyCost, int threshold, int patchsize);
	
	// normal smooth cost
	virtual int GetSmoothCostOrigin(int labelId1, int labelId2, int nodeId1, int nodeId2, int penaltyCost);

	// normal data cost with boundary constraint
	virtual int GetDataCostOrigin(int labelId, int nodeId, int penaltyCost);

	// prefer larger image
	virtual int GetDataCostPreferLargerImage(int labelId, int nodeId, int penaltyCost);

	//========================================
	// helper getdistortioncost
	int GetDistortionCostPatch(vector<CvScalar>* points, CvPoint point, IplImage* image, int patchsize);

	// helper area cost data function
	int GetDistortionCost(CvPoint point, IplImage* image, int label, int patch_size, double scaleX, double scaleY);

	// combine area cost and distortion cost
	virtual int GetDataCostAreaCost(int labelId, int nodeId, int penaltyCost);
	//========================================

	// reverse saliency in large image and small iamge
	virtual int GetDataCostReverseSaliency(int labelId, int nodeId, int penaltyCost);

	// changing saliency according to threshold
	virtual int GetDataCostReverseThreshold(int labelId, int nodeId, int penaltyCost);
// extra inside helpers
protected:
	bool IsSatisfiedBoundary(int x, double mappedX, int inputSize, int outputSize);
 
};