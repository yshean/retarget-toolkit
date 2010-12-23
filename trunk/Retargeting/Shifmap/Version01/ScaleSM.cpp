#include "StdAfx.h"
#include "ScaleSM.h"

ScaleSM::ScaleSM(void)
{
	_mapping2D = new Mapping2D();
	_energyFunction = new ScaleEnergyFunction();
}

ScaleSM::~ScaleSM(void)
{
}

void ScaleSM::SetSmoothThreshold(int threshold)
{
	_smoothThreshold = threshold;
}

void ScaleSM::SetScaleSetting(int scaleCount, double scaleStepX, double scaleStepY)
{
	_scaleCount = scaleCount;
	_scaleStepX = scaleStepX;
	_scaleStepY = scaleStepY;
}

void ScaleSM::SetMaxSaliency(int max)
{
	_energyFunction->SetMaxSalLargerImage(max);	
}
void ScaleSM::SetEnergyFunction(ScaleEnergyFunction* energyFunction)
{
	_energyFunction = energyFunction;
}
void ScaleSM::SetMapping2D(Mapping2D* mapping2D)
{
	_mapping2D = mapping2D;
}

void ScaleSM::InitGraphCut(IplImage* image, IplImage* saliency, CvSize outputSize)
{
	_input = image;
	_outputSize = outputSize;

	

	IplImage* gradient = cvCloneImage(image);			
	cvSobel(image, gradient, 1, 1);	
	
	//ScaleLabelMapping* labelMapping = new ScaleLabelMapping();
	//labelMapping->InitScaleRange(cvSize(_input->width, _input->height), 
	//	_outputSize, _scaleCount, _scaleStepX, _scaleStepY);	 
	//_labelMapping = labelMapping;
	//
	//
	//_mapping2D->InitializeMapping(outputSize.width, outputSize.height);
	//_mapping2D->IsShift(false);
	//

	//_energyFunction->SetSmoothThreshold(_smoothThreshold);
	//_energyFunction->SetInput(image, gradient, saliency);
	//_energyFunction->SetRetargetSize(cvSize(image->width, image->height), outputSize);
	//_energyFunction->SetLabelMapping(_labelMapping);
	//_energyFunction->SetMapping2D(_mapping2D);
	//_energyFunction->SetSmoothCostPatchSize(3);

	GCScaleImage* gc = new GCScaleImage();
	gc->Initialize(outputSize.width, outputSize.height, _labelMapping->GetLabelCount());
	gc->SetEnergyFunction(_energyFunction);
	_gc = gc;
}
void ScaleSM::SetLabelMapping(ScaleLabelMapping* labelMapping)
{
	_labelMapping = labelMapping;
}
void ScaleSM::ComputeGraphCut()
{
	_gc->ComputeGC();
}

void ScaleSM::ComputeOptimalRetargetMapping(IplImage* image, IplImage* saliency, CvSize outputSize)
{
	_input = image;
	_outputSize = outputSize;

	

	IplImage* gradient = cvCloneImage(image);			
	cvSobel(image, gradient, 1, 1);	
	
	ScaleLabelMapping* labelMapping = new ScaleLabelMapping();
	labelMapping->InitScaleRange(cvSize(_input->width, _input->height), 
		_outputSize, _scaleCount, _scaleStepX, _scaleStepY);	 
	_labelMapping = labelMapping;
	
	
	_mapping2D->InitializeMapping(outputSize.width, outputSize.height);
	_mapping2D->IsShift(false);
	

	_energyFunction->SetSmoothThreshold(_smoothThreshold);
	_energyFunction->SetInput(image, gradient, saliency);
	_energyFunction->SetRetargetSize(cvSize(image->width, image->height), outputSize);
	_energyFunction->SetLabelMapping(_labelMapping);
	_energyFunction->SetMapping2D(_mapping2D);
	_energyFunction->SetSmoothCostPatchSize(3);

	GCScaleImage* gc = new GCScaleImage();
	gc->Initialize(outputSize.width, outputSize.height, labelMapping->GetLabelCount());
	gc->SetEnergyFunction(_energyFunction);
	gc->ComputeGC();
	_gc = gc;
	
}
 
// should call only after compute optimal mapping
IplImage* ScaleSM::RenderRetargetImage()
{
	IplImage* image = cvCreateImage(_outputSize, IPL_DEPTH_8U, 3);
	
	GCoptimization* gc = _gc->GetGCoptimization();
	int nodeCount = _outputSize.width * _outputSize.height;

	for(int i = 0; i < nodeCount; i++)
	{
		int label = gc->whatLabel(i);
		CvPoint point = _mapping2D->GetMappedPoint(i);			
		DoublePoint mappedPoint = _labelMapping->GetMappedPoint(label, point);
		 		
		if(!IsInside(mappedPoint, cvSize(_input->width, _input->height)))
		{
			printf("Map outside");
			printf("test % i %i ", point.x, point.y);
			cvSet2D(image, point.y, point.x, cvScalar(255));
		}
		else
		{
			// printf("test % f %f ", mappedPoint.x, mappedPoint.y);
			CvScalar value = GetInterpolatedValue(mappedPoint, _input);
			cvSet2D(image, point.y, point.x, value);			
		}		
	}
	//DisplayImage(image, "TEST");
	return image;
}
// render a stack map for visualization of which pixel is mapped to which layer in stack
IplImage* ScaleSM::RenderStackMapVisualisation()
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
		// int shiftId = _labelMapping->GetShiftId(label);
			
		double value = scaleId * 255 / scaleCount ;
		//double shiftValue = shiftId * 10;
		cvSet2D(image, point.y, point.x, cvScalar(value,0,0));			
		
	}
	//DisplayImage(image, "TEST");
	return image;
}
