#include "StdAfx.h"
#include "GCImageRender.h"

GCImageRender::GCImageRender(void)
{
}

GCImageRender::~GCImageRender(void)
{
}

void GCImageRender::Initialize(LabelMapper* mapper, GCoptimization* gc, ScaleStackImageSource* imageSource)
{
	_mapper = mapper;
	_gc = gc;
	_imageSource = imageSource;
	
}

void GCImageRender::SetOutputSize(int width, int height)
{
	_width = width;
	_height = height;
	_numSites = width * height;
}
IplImage* GCImageRender::GetVisualizedImage()
{
	int levelCount = _imageSource->GetLevelCount();

	IplImage* image = cvCreateImage(cvSize(_width, _height), IPL_DEPTH_8U, 3);
	for(int i = 0; i < _numSites; i++)
	{
		int label = _gc->whatLabel(i);
		Point3D mappedPoint = _mapper->GetMappedPoint(label, i);
		//CvPoint point = GetPoint(i, cvSize(_width, _height));
		Point3D point = _mapper->GetPoint(i);
		
		if(_imageSource->IsOutsideSource(mappedPoint))
		{
			printf("Map outside");
			cvSet2D(image, point.y, point.x, cvScalar(255));
		}
		else
		{
			double value = mappedPoint.z * 255 / levelCount;
			cvSet2D(image, point.y, point.x, cvScalar(value));			
		}
		
	}
	//DisplayImage(image, "TEST");
	return image;
}

IplImage* GCImageRender::GetRenderedImage()
{	 
	IplImage* image = cvCreateImage(cvSize(_width, _height), IPL_DEPTH_8U, 3);
	for(int i = 0; i < _numSites; i++)
	{
		int label = _gc->whatLabel(i);
		Point3D mappedPoint = _mapper->GetMappedPoint(label, i);
		//CvPoint point = GetPoint(i, cvSize(_width, _height));
		Point3D point = _mapper->GetPoint(i);
		
		if(_imageSource->IsOutsideSource(mappedPoint))
		{
			printf("Map outside");
			cvSet2D(image, point.y, point.x, cvScalar(255));
		}
		else
		{
			CvScalar value = _imageSource->GetImageValue(mappedPoint);
			cvSet2D(image, point.y, point.x, value);
			if(mappedPoint.z == 1)
			{
				printf("smaller scale");
			}
		}
		
	}
	//DisplayImage(image, "TEST");
	return image;
}