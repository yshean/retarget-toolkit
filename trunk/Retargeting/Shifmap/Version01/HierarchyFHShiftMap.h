#pragma once
#include "FillHoleShiftMap.h"
#include "HierarchyFHEnergyFunction.h"
#include "MaskShift.h"
#include "highgui.h"

class HierarchyFHShiftMap : public FillHoleShiftMap
{
public:
	HierarchyFHShiftMap(void);
	~HierarchyFHShiftMap(void);

	// better version of ComputeFastShiftMap
	// mask is of the same size with input & maskData will be auto-generated
	virtual void ComputeFastShiftMap2(IplImage* input, IplImage* saliency, IplImage* mask, CvPoint shift, CvSize outputSize);
	virtual void ComputeFastShiftMap(IplImage* input, IplImage* saliency);
	IplImage* GetRetargetedImage(int level);
	int GetLevelCount();
protected:
	vector<IplImage*>* _maskList;
	vector<IplImage*>* _maskDataList;
	vector<IplImage*>* _inputList;
	vector<IplImage*>* _saliencyList;
	vector<CvMat*>* _labelMapList;
	vector<vector<CvPoint*>*>* _pointMappingList;
public:
	virtual void ComputeShiftMapGuess(IplImage* input, IplImage* saliency, CvMat* guess, CvSize output, CvSize shiftSize);	

	vector<CvPoint*>* InterpolatePointMapping(vector<CvPoint*>* pointMapping);

	// interpolate the shift map from down 1 level
	void InterpolateShiftMapping(CvMat* lowerMap, CvMat* higherMap);

	// new version interpolate the shift map from down 1 level 
	void InterpolateShiftMapping(CvMat* lowerMap, CvMat* higherMap, IplImage* lowerMask, IplImage* higherMask);	

	
	// downsampling an image, if size = 0, no blurring is done
	void DownSampling(IplImage* source, IplImage* dst, int size);
	// downsampling until the widht  < limitedSize
	void DownSamplingImage(IplImage* input, vector<IplImage*>* list, int limitedSize, int blurSize);
	void DownSamplingMask(IplImage* higherMask, IplImage* lowerMask);
	void DownSamplingMask(IplImage* mask, vector<IplImage*>* list, int limitedSize);
	void DownSamplingInput(IplImage* input, IplImage* saliency, IplImage* mask, IplImage* maskData);
	// better version of down sampling input
	// mask is the same size as input & maskData will be auto-generated
	void DownSamplingInput(IplImage* input, IplImage* saliency, IplImage* mask, CvSize outputSize, CvPoint shift);
	
	CvMat* CalculateLabelMapGuess(CvMat* intialGuess, vector<CvPoint*>* pointMapping, GCoptimizationGeneralGraph* gc);
	void ClearGC();
};
