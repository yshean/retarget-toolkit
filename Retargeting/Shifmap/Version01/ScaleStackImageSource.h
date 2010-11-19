#pragma once
#include "ImageSource.h"
#include "Label.h"
#include "EnergyFunction.h"

#include <vector>
using namespace std;



class ScaleStackImageSource : public ImageSource
{
public:
	ScaleStackImageSource(void);
	~ScaleStackImageSource(void);
protected:	
	vector<IplImage*>* _imageStack;
	vector<IplImage*>* _gradientStack;
public:
	// number of level of stack
	int GetLevelCount();
	CvSize GetImageSize(int level);
	void PushImage(IplImage* image);
	void PushGradient(IplImage* image);
	bool IsOutsideSource(Point3D point);
	virtual CvScalar GetImageValue(Point3D point);
	virtual CvScalar GetGradientValue(Point3D point);
	virtual int GetSquareColorDifference(Point3D point1, Point3D point2);
	virtual int GetSquareGradientDifference(Point3D point1, Point3D point2);
protected:
	bool IsOutsideSource(Point3D point, vector<IplImage*>* imageStack);
	CvScalar GetPointValue(Point3D point, vector<IplImage*>* imageStack);
	int SquareColorDifference(Point3D point1, Point3D point2, vector<IplImage*>* imageStack);
};

ScaleStackImageSource* CreateScaleStackImageDSampling(IplImage* input, int stack_height);

// create stack from list of images
ScaleStackImageSource* CreateScaleStackFromList(IplImage* input1, IplImage* input2);
ScaleStackImageSource* CreateScaleStackFromList(vector<IplImage*>* inputList);
void DownSampling(IplImage* image, IplImage* dst, int size);