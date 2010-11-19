#include "StdAfx.h"
#include "GCScaleStackShiftMap.h"
 
GCScaleStackShiftMap::GCScaleStackShiftMap(void)
{
}

GCScaleStackShiftMap::~GCScaleStackShiftMap(void)
{
}
IplImage* GCScaleStackShiftMap::GetRetargetImage2(IplImage* input, IplImage* saliency, ScaleStackImageSource* imageSource, ScaleStackImageSource* saliencySource, CvSize outputSize)
{
// setting
	int stack_height = 2; 

	MappingCubic* mappingData = new MappingCubic();
	//mappingData->InitializeMapping(input->width, input->height, 1);
	mappingData->InitializeMapping(outputSize.width, outputSize.height, 1);
	mappingData->IsShift(false);
	MappingCubic* mappingShift = new MappingCubic();
	mappingShift->InitializeMapping(input->width, input->height, stack_height);
	mappingShift->IsShift(true);
	
	//ScaleStackImageSource* imageSource = CreateScaleStackImageDSampling(input, 2);
	//ScaleStackImageSource* saliencySource = CreateScaleStackImageDSampling(saliency, 2);

 	GCScaleStackEnergy* energy = new GCScaleStackHorizontalEnergy();
	energy->SetMapping(mappingData, mappingShift);
	energy->SetSource(imageSource, saliencySource);
	

	GCAImage* gcAlgorithm = new GCAImage(energy);
	 
	gcAlgorithm->Initialize(outputSize.width, outputSize.height, mappingShift);	
	gcAlgorithm->ComputeGC();
	
	GCoptimization* gc = gcAlgorithm->GetGCoptimization();

	// label mapper, mapping from graph to origin stack of images
	// can be changed to mapping using a guess map for e.g
	LabelMapper* mapper = new LabelMapper();
	mapper->SetMappingCubic(mappingData, mappingShift);

	
	
	GCImageRender* render = new GCImageRender();	
	render->Initialize(mapper, gc, imageSource);
	render->SetOutputSize(outputSize.width, outputSize.height);
	return render->GetRenderedImage();	
}

void GCScaleStackShiftMap::ComputeOptimalRetargetMapping(ScaleStackImageSource* imageSource, ScaleStackImageSource* saliencySource, CvSize outputSize)
{ 	
	_imageSource = imageSource;
	_outputSize = outputSize;
	int stack_height = imageSource->GetLevelCount();
	// first level
	CvSize firstLevelSize = imageSource->GetImageSize(0);

	_mappingData = new MappingCubic();
	//mappingData->InitializeMapping(input->width, input->height, 1);
	_mappingData->InitializeMapping(outputSize.width, outputSize.height, 1);
	_mappingData->IsShift(false);
	
	_mappingShift = new MappingCubic();
	_mappingShift->InitializeMapping(firstLevelSize.width / 2, firstLevelSize.height / 2, stack_height);
	_mappingShift->IsShift(true);
	
	//ScaleStackImageSource* imageSource = CreateScaleStackImageDSampling(input, 2);
	//ScaleStackImageSource* saliencySource = CreateScaleStackImageDSampling(saliency, 2);

 	GCScaleStackEnergy* energy = new GCScaleStackHorizontalEnergy();
	energy->SetMapping(_mappingData, _mappingShift);
	energy->SetSource(imageSource, saliencySource);
	

	
	_gcAlgorithm = new GCAImage(energy);
	 
	_gcAlgorithm->Initialize(_outputSize.width, _outputSize.height, _mappingShift);	
	_gcAlgorithm->ComputeGC();
}

IplImage* GCScaleStackShiftMap::RenderRetargetImage()
{
	GCoptimization* gc = _gcAlgorithm->GetGCoptimization();

	// label mapper, mapping from graph to origin stack of images
	// can be changed to mapping using a guess map for e.g
	LabelMapper* mapper = new LabelMapper();
	mapper->SetMappingCubic(_mappingData, _mappingShift);
	
	GCImageRender* render = new GCImageRender();	
	render->Initialize(mapper, gc, _imageSource);
	render->SetOutputSize(_outputSize.width, _outputSize.height);
	return render->GetRenderedImage();	
}

IplImage* GCScaleStackShiftMap::RenderStackMapVisualisation()
{	
	GCoptimization* gc = _gcAlgorithm->GetGCoptimization();

	// label mapper, mapping from graph to origin stack of images
	// can be changed to mapping using a guess map for e.g
	LabelMapper* mapper = new LabelMapper();
	mapper->SetMappingCubic(_mappingData, _mappingShift);
	
	GCImageRender* render = new GCImageRender();	
	render->Initialize(mapper, gc, _imageSource);
	render->SetOutputSize(_outputSize.width, _outputSize.height);
	return render->GetVisualizedImage();	
}

IplImage* GCScaleStackShiftMap::GetRetargetImage(IplImage* input, IplImage* saliency, CvSize outputSize)
{
	// setting
	int stack_height = 2; 

	MappingCubic* mappingData = new MappingCubic();
	//mappingData->InitializeMapping(input->width, input->height, 1);
	mappingData->InitializeMapping(outputSize.width, outputSize.height, 1);
	mappingData->IsShift(false);
	MappingCubic* mappingShift = new MappingCubic();
	mappingShift->InitializeMapping(input->width, input->height, stack_height);
	mappingShift->IsShift(true);
	
	ScaleStackImageSource* imageSource = CreateScaleStackImageDSampling(input, 2);
	ScaleStackImageSource* saliencySource = CreateScaleStackImageDSampling(saliency, 2);

 	GCScaleStackEnergy* energy = new GCScaleStackEnergy();
	energy->SetMapping(mappingData, mappingShift);
	energy->SetSource(imageSource, saliencySource);
	

	GCAImage* gcAlgorithm = new GCAImage(energy);
	 
	gcAlgorithm->Initialize(outputSize.width, outputSize.height, mappingShift);	
	gcAlgorithm->ComputeGC();
	
	GCoptimization* gc = gcAlgorithm->GetGCoptimization();

	// label mapper, mapping from graph to origin stack of images
	// can be changed to mapping using a guess map for e.g
	LabelMapper* mapper = new LabelMapper();
	mapper->SetMappingCubic(mappingData, mappingShift);

	
	
	GCImageRender* render = new GCImageRender();	
	render->Initialize(mapper, gc, imageSource);
	render->SetOutputSize(outputSize.width, outputSize.height);
	return render->GetRenderedImage();	
}