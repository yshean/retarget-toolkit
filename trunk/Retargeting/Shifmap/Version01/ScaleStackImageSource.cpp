#include "StdAfx.h"
#include "ScaleStackImageSource.h"

ScaleStackImageSource::ScaleStackImageSource(void)
{
	_imageStack = new vector<IplImage*>();
	_gradientStack = new vector<IplImage*>();
}

ScaleStackImageSource::~ScaleStackImageSource(void)
{
}
bool ScaleStackImageSource::IsOutsideSource(Point3D point)
{
	return IsOutsideSource(point, _imageStack);
}
CvScalar ScaleStackImageSource::GetImageValue(Point3D point)
{	
	return GetPointValue(point, _imageStack);
}
CvScalar ScaleStackImageSource::GetGradientValue(Point3D point)
{
	return GetPointValue(point, _gradientStack);
}
bool ScaleStackImageSource::IsOutsideSource(Point3D point, vector<IplImage*>* imageStack)
{
	if(point.z > imageStack->size())
		return true;
	CvPoint point2D = cvPoint(point.x, point.y);
	CvSize imageSize = cvSize((*imageStack)[point.z]->width, (*imageStack)[point.z]->height);
	if(IsOutside(point2D, imageSize))
		return true;
	return false;
}

CvScalar ScaleStackImageSource::GetPointValue(Point3D point, vector<IplImage*>* imageStack)
{
	if(IsOutsideSource(point, imageStack))
		return cvScalar(-1,-1,-1);
	
	return cvGet2D((*imageStack)[point.z], point.y, point.x);
}
int ScaleStackImageSource::SquareColorDifference(Point3D point1, Point3D point2, vector<IplImage*>* imageStack)
{
	CvScalar value1 = GetPointValue(point1, imageStack);
	CvScalar value2 = GetPointValue(point2, imageStack);
	return SquareDifference(value1, value2);
}

int ScaleStackImageSource::GetSquareColorDifference(Point3D point1, Point3D point2)
{
	return SquareColorDifference(point1, point2, _imageStack);
}
int ScaleStackImageSource::GetSquareGradientDifference(Point3D point1, Point3D point2)
{
	return SquareColorDifference(point1, point2, _gradientStack);
}

int ScaleStackImageSource::GetLevelCount()
{
	return _imageStack->size();
}

CvSize ScaleStackImageSource::GetImageSize(int level)
{
	return cvSize((*_imageStack)[level]->width, (*_imageStack)[level]->height);
}

void  ScaleStackImageSource::PushImage(IplImage* image)
{
	_imageStack->push_back(image);
}
void  ScaleStackImageSource::PushGradient(IplImage* image)
{
	_gradientStack->push_back(image);
}
void DownSampling(IplImage* image, IplImage* dst, int size)
{	
	IplImage* blur = cvCloneImage(image);
	if(size > 0)
		cvSmooth(image, blur, 2, 3, 3);
	for(int i = 0; i < dst->width; i++)
		for(int j = 0; j < dst->height; j++)
		{
			CvScalar value = cvGet2D(blur, j*2, i*2);
			cvSet2D(dst, j, i, value);
		}
}

ScaleStackImageSource* CreateScaleStackFromList(IplImage* input1, IplImage* input2)
{
	ScaleStackImageSource* imageSource = new ScaleStackImageSource();
	imageSource->PushImage(input1);
	imageSource->PushImage(input2);

	IplImage* gradient1 = cvCloneImage(input1);
	IplImage* gradient2 = cvCloneImage(input2);
	cvSobel(input1, gradient1, 1, 1);
	cvSobel(input2, gradient2, 1, 1);
	imageSource->PushGradient(gradient1);
	imageSource->PushGradient(gradient2);

	return imageSource;
}

ScaleStackImageSource* CreateScaleStackFromList(vector<IplImage*>* inputList)
{
	ScaleStackImageSource* imageSource = new ScaleStackImageSource();
	for(int i = 0; i < inputList->size(); i++)
	{
		IplImage* input = (*inputList)[i];
		imageSource->PushImage(input);		
		IplImage* gradient = cvCloneImage(input);
		cvSobel(input, gradient, 1, 1);
		imageSource->PushGradient(gradient);
	}
	return imageSource;
}

ScaleStackImageSource* CreateScaleStackImageDSampling(IplImage* input, int stack_height)
{
	IplImage* level = input;	
	 

	ScaleStackImageSource* imageSource = new ScaleStackImageSource();
	imageSource->PushImage(level);
	
	IplImage* gradient = cvCloneImage(level);
	cvSobel(level, gradient, 1, 1);
	imageSource->PushGradient(gradient);

	for(int i = 0; i < stack_height; i++)
	{
		IplImage* level_temp = cvCreateImage(cvSize(level->width/2, level->height/2), input->depth, input->nChannels);				
		DownSampling(level, level_temp, 3);		
		 
		level = level_temp;		
		imageSource->PushImage(level);		
		
		gradient = cvCloneImage(level);
		cvSobel(level, gradient, 1, 1);
		imageSource->PushGradient(gradient);
	}  
	return imageSource;
}