#pragma once
#include "ShiftMap.h"

class ShiftMapHierarchy : public ShiftMap
{
public:
	ShiftMapHierarchy(void);
	~ShiftMapHierarchy(void);
	virtual IplImage* GetRetargetImage(IplImage* input, IplImage* saliency, CvSize outputSize);
 
public:
	// fast shift by downsampling
	void ComputeFastShiftMap(IplImage* input, IplImage* saliency, CvSize output);
protected:
	// compute shift-map using an initial guess
	void ComputeShiftMapGuess(IplImage* input, IplImage* saliency, CvMat* initialGuess, CvSize output, CvSize shiftSize);
	CvMat* _initialGuess; // for hierarchical shift-map
	int _level;
	// with initialGuess
	CvMat* CalculateLabelMapGuess();
	
	// get a interpolation of lower map by x2 times value of shift in the lowermap	
	void GetInterpolationMap(CvMat* lowerMap, CvMat* higherMap);

	void DownSampling(IplImage* source, IplImage* dst);
public:	
	// data to analyze performance of algorithm
	vector<IplImage*>* _imageList;
	vector<CvMat*>* _labelMapList;

	// get retarget image in a specifica level
	IplImage* GetRetargetImage(int level);
	int GetLevelCount();
	IplImage* GetOriginalImage(int level);
	
	IplImage* GetRetargetImageH();

	// get label map of a specific level
	CvMat* GetLabelMap(int level);
};
