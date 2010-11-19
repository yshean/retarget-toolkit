#pragma once
#include <vector>
using namespace std;
#include <cv.h>
#include "PatchUtils.h"



class ScaleLabelMapping
{
public:
	ScaleLabelMapping(void);
	~ScaleLabelMapping(void);

protected:
	int _scaleCount;
	double _scaleStepX;
	double _scaleStepY;	
	int _labelCountX;
	int _labelCountY;
	vector<int>* _labelCountLevelX;
	vector<int>* _labelCountLevelY;
	vector<int>* _labelCountLevel;
	vector<int>* _labelMinLevelX;
	vector<int>* _labelMinLevelY;
	vector<double>* _scaleListX;
	vector<double>* _scaleListY;
	CvSize _inputSize;
	CvSize _outputSize;
protected:
	// counting number of labels each level
	int CountLevelLabel(vector<int>* labelCountLevel);
	// init scale range for one direction (vertical or horizontal)
	
	// wider range of shift
	void InitWarpScaleRange(int inputWidth, int outputWidth, vector<double>* scaleListX, vector<int>* labelCountLevel, vector<int>* labelMinLevel);
	void InitScaleRange(int inputWidth, int outputWidth, int scaleCount, double scaleStep, vector<int>* levelCount);
	double GetMappedPoint(int label, int x, int scaleStep, int scaleCount, int inputSize, int outputSize, vector<int>* labelCountLevel);
	DoublePoint GetMappedWarpPoint(int labelId, CvPoint point, vector<double>* scaleListX, vector<double>* scaleListY);
	DoublePoint GetMappedScalePoint(int label, CvPoint point);
	int GetScaleId(int label, int scaleCount, vector<int>* labelCountLevel);
public:
	void InitScaleRange(CvSize inputSize, CvSize outputSize, int scaleCount, double scaleStepX, double scaleStepY);
	void InitWarpScaleRange(CvSize inputSize, CvSize outputSize, vector<double>* scaleListX, vector<double>* scaleListY);
public:
	DoublePoint GetMappedPoint(int labelId, CvPoint point);
	int GetScaleId(int label);
	// total number of labels
	int GetLabelCount();
	// get scale count
	int GetScaleCount();
	// get scale
	double GetScale(int label);
};
int GetPatchDifference(CvPoint point1, CvPoint point2, int labelId1, int labelId2, IplImage* image, ScaleLabelMapping* labelMapping, int patchSize);