#include "StdAfx.h"
#include "HorizontalScaleSM.h"

HorizontalScaleSM::HorizontalScaleSM(void)
{
}

HorizontalScaleSM::~HorizontalScaleSM(void)
{
}

void HorizontalScaleSM::SetScaleSetting(int scaleCount, double scaleStep)
{
	_scaleCount = scaleCount;
	_scaleStep = scaleStep;
}
void HorizontalScaleSM::SetShiftRange(int shiftRange)
{
	_shiftRange = shiftRange;
}
void HorizontalScaleSM::ComputeOptimalRetargetMapping(IplImage* image, IplImage* saliency, CvSize outputSize)
{
	_input = image;
	_outputSize = outputSize;

	_energyFunction = new HorizontalScaleEnergyFunction();
	IplImage* gradient = cvCloneImage(image);			
	cvSobel(image, gradient, 1, 1);	
	
	HorizontalScaleLabelMapping* labelMapping = new HorizontalScaleLabelMapping();
	labelMapping->InitScaleRange(image->width, outputSize.width, _scaleCount, _scaleStep);
	labelMapping->InitShiftRange(_shiftRange);
	_labelMapping = labelMapping;
	

	Mapping2D* mapping2D = new Mapping2D();
	mapping2D->InitializeMapping(outputSize.width, outputSize.height);
	mapping2D->IsShift(false);
	_mapping2D = mapping2D;

	_energyFunction->SetInput(image, gradient, saliency);
	_energyFunction->SetRetargetSize(cvSize(image->width, image->height), outputSize);
	_energyFunction->SetLabelMapping(labelMapping);
	_energyFunction->SetMapping2D(mapping2D);

	GCScaleImage* gc = new GCScaleImage();
	gc->Initialize(outputSize.width, outputSize.height, labelMapping->GetLabelCount());
	gc->SetEnergyFunction(_energyFunction);
	gc->ComputeGC();
	_gc = gc;
	
}
CvScalar HorizontalScaleSM::GetInterpolatedValue(double x1, int y, IplImage* image)
{
	int x1Int = (int)floor(x1);
	double weight = x1 - x1Int;

	if(weight == 0)
	{
		return cvGet2D(image, y, x1Int);
	}
	else
	{
		CvScalar value1 = cvGet2D(image, y, x1Int);
		CvScalar value2 = cvGet2D(image, y, x1Int + 1);
		CvScalar value;

		for(int i = 0; i < 4; i++)
		{
			value.val[i] = value1.val[i] * (1-weight) + value1.val[i] * weight;
		}
		
		return value;
	}
}
// should call only after compute optimal mapping
IplImage* HorizontalScaleSM::RenderRetargetImage()
{
	IplImage* image = cvCreateImage(_outputSize, IPL_DEPTH_8U, 3);
	
	GCoptimization* gc = _gc->GetGCoptimization();
	int nodeCount = _outputSize.width * _outputSize.height;

	for(int i = 0; i < nodeCount; i++)
	{
		int label = gc->whatLabel(i);
		CvPoint point = _mapping2D->GetMappedPoint(i);			
		double mappedX = _labelMapping->GetMappedPoint(label, point.x);
		 		
		if(mappedX >= _input->width || mappedX < 0)
		{
			printf("Map outside");
			cvSet2D(image, point.y, point.x, cvScalar(255));
		}
		else
		{			 
			CvScalar value = GetInterpolatedValue(mappedX, point.y, _input);
			cvSet2D(image, point.y, point.x, value);			
		}
	}
	//DisplayImage(image, "TEST");
	return image;
}
// render a stack map for visualization of which pixel is mapped to which layer in stack
IplImage* HorizontalScaleSM::RenderStackMapVisualisation()
{
	IplImage* image = cvCreateImage(_outputSize, IPL_DEPTH_8U, 3);
	
	GCoptimization* gc = _gc->GetGCoptimization();
	int nodeCount = _gc->GetSitesCount();
	int scaleCount = _labelMapping->GetScaleCount();

	for(int i = 0; i < nodeCount; i++)
	{
		int label = gc->whatLabel(i);
		CvPoint point = _mapping2D->GetMappedPoint(i);			
		int scaleId = _labelMapping->GetScaleId(label);
		int shiftId = _labelMapping->GetShiftId(label);
			
		double value = scaleId * 255 / scaleCount ;
		double shiftValue = shiftId * 10;
		cvSet2D(image, point.y, point.x, cvScalar(value,0,0));			
		
	}
	//DisplayImage(image, "TEST");
	return image;
}

IplImage* HorizontalScaleSM::RenderSmoothCostMapVisualisation()
{
	IplImage* image = cvCreateImage(_outputSize, IPL_DEPTH_8U, 3);
	
	for(int i = 0; i < _outputSize.width-1; i++)
		for(int j = 0; j < _outputSize.height; j++)
		{
			cvSet2D(image, j, i, cvScalar(0,0,0));
		}

	GCoptimization* gc = _gc->GetGCoptimization();
	int nodeCount = _gc->GetSitesCount();
	int scaleCount = _labelMapping->GetScaleCount();

	// vertical
	for(int i = 0; i < _outputSize.width-1; i++)
		for(int j = 0; j < _outputSize.height; j++)
		{
			int id1 = _mapping2D->GetPointId(cvPoint(i,j));
			int id2 = _mapping2D->GetPointId(cvPoint(i+1,j));
			CvPoint ponit1 = _mapping2D->GetMappedPoint(id1);
			CvPoint ponit2 = _mapping2D->GetMappedPoint(id2);
			int label1 = gc->whatLabel(id1);
			int label2 = gc->whatLabel(id2);
			int smoothCost = _energyFunction->GetSmoothCost(label1, label2, id1, id2);		

			cvSet2D(image, j, i, cvScalar(smoothCost));
		}

	// horizontal
	for(int i = 0; i < _outputSize.width; i++)
		for(int j = 0; j < _outputSize.height - 1; j++)
		{
			int id1 = _mapping2D->GetPointId(cvPoint(i,j));
			int id2 = _mapping2D->GetPointId(cvPoint(i,j + 1));
			int label1 = gc->whatLabel(id1);
			int label2 = gc->whatLabel(id2);
			int smoothCost = _energyFunction->GetSmoothCost(label1, label2, id1, id2);
			
			CvScalar value = cvGet2D(image, j, i);
			CvScalar cost;
			for(int i = 0; i < 4; i++)
				cost.val[i] = value.val[i] + smoothCost;
			cvSet2D(image, j, i, cost);
		}		
	 
	//DisplayImage(image, "TEST");
	return image;
}

string HorizontalScaleSM::GetCost()
{
	string result = "";
	char b[100];
	sprintf(b, "Data-%i", _gc->GetDataEnergy());	
	result += b;	
	sprintf(b, "Smooth-%i", _gc->GetSmoothEnergy());		
	result += b;
	return result;
}