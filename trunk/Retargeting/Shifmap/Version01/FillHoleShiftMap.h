#pragma once
#include "ImageRetargeter.h"
#include "FillHoleEnergyFunction.h"
#include "GCoptimization.h"
#include "MaskShift.h"
#include <vector>
using namespace std;

class FillHoleShiftMap : public ImageRetargeter
{
public:
	FillHoleShiftMap(void);
	~FillHoleShiftMap(void);
	virtual IplImage* GetRetargetImage(IplImage* input, IplImage* saliency, CvSize outputSize);
	virtual IplImage* GetRetargetImage2(IplImage* input, IplImage* saliency, CvSize outputSize, MaskShift* maskShift);
	virtual void ComputeShiftMap(IplImage* input, IplImage* saliency, CvSize output, CvSize shiftSize);	
	virtual void ComputeShiftMap2(IplImage* input, IplImage* saliency, CvSize output, CvSize shiftSize, MaskShift* maskShift);	
	void SetMask(IplImage* mask, IplImage* maskData);

protected:
	IplImage* _mask;
	IplImage* _maskData;
	GCoptimizationGeneralGraph* _gcGeneral;
	vector<CvPoint*>* _pointMapping;
	IplImage* _input;
	IplImage* _maskNeighbor; // carry neighbor info with mask
	CvSize _shiftSize;
protected:
	void ClearGC();
	void ProcessMask();
	bool IsMaskedPixel(int x, int y, IplImage* mask);
	
	// process the mapping
	// pointMapping is an empty vector
	// mapping is a matrix which has the same size as the mask	
	// mask is the input
	// return total number of nodes
	void ProcessMapping(IplImage* mask, CvMat* mapping, vector<CvPoint*>* pointMapping);
	void ProcessMapping(MaskShift* mask, CvMat* mapping, vector<CvPoint*>* pointMapping);
	// 
	void ProcessMask(IplImage* mask);
	void ProcessMask(MaskShift* mask);
	
	// setup neighborhood system for GC
	void SetupGCOptimizationNeighbor(GCoptimizationGeneralGraph* gcGeneral, CvMat* mapping);

	// setup neighborhood system for nodes & masked pixels
	// maskNeighbor & mask should have the same size
	// action: scan through the mask and set info for pixel which is a neighbor of a masked pixel
	// 
	void SetupMaskNeighbor(IplImage* maskNeighbor, IplImage* mask);
	void SetupMaskNeighbor(IplImage* maskNeighbor, MaskShift* mask);
	// helpful function for SetupMaskNeighbor
	// doesn't check whether currentPoint & neighborPoint is neighbor
	// and inside the image 
	// tradeoff safety for performance
	void ProcessNeighbor(CvPoint currentPoint, CvPoint neighborPoint, IplImage* maskNeighbor, IplImage* mask);
	void ProcessNeighbor(CvPoint currentPoint, CvPoint neighborPoint, IplImage* maskNeighbor, MaskShift* mask);
	
	void ProcessNeighbor(CvPoint currentPoint, CvPoint neighborPoint, IplImage* maskNeighbor, CvMat* mapping, GCoptimizationGeneralGraph* gcGeneral);
	IplImage* CalculatedRetargetImage();
	CvMat* CalculateLabelMap();
};
