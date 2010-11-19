#pragma once
#include <vector>
using namespace std;

// this special class mapping labels into shift or shift + scale
class HorizontalScaleLabelMapping
{
public:
	HorizontalScaleLabelMapping(void);
	~HorizontalScaleLabelMapping(void);
protected:
	int _shiftRange;
	int _scaleCount;
	double _scaleStep;
	//int _originSize;
	//int _targetSize;
	int _labelCount;
	vector<int>* _labelCountLevel;
	int _inputWidth;
	int _outputWidth;
public:

	double GetMappedPoint(int label, int x);
	int GetScaleId(int label);
	int GetShiftId(int label);
	double GetScale(int label);
	int GetLabelCount();
	int GetScaleCount();
	
public:
	
	// a pixel can shift from -shiftRange/2 to shiftRange/2
	void InitShiftRange(int shiftRange);
	// a pixel can scale its shift too
	void InitScaleRange(int inputWidth, int outputWidth, int scaleCount, double scaleStep);
};
