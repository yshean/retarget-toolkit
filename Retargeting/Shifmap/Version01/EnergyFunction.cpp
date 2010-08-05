#include "StdAfx.h"
#include "EnergyFunction.h"


// this function is the interface with the paper
int ShifmapDataFunction(CvPoint point, CvPoint shift, IplImage* saliency)
{
	CvScalar value = cvGet2D(saliency, point.y + shift.y, point.x + shift.x);
	return value.val[0] + value.val[1] + value.val[2];
}

int dataFunctionShiftmapH(int pixel, int label, void *extraData)
{
	
	ForDataFunctionH *data = (ForDataFunctionH *) extraData; 	
	CvPoint origin_label = GetMappedPointInitialGuess(pixel, label, data->outputSize, data->shiftSize, data->initialGuess);
	
	if(IsOutside(origin_label, data->inputSize)) 
		return 100000;

	double saliency = 0;

	//// force the 2 left-most & right-most col to be in the output
	//CvPoint pixelPoint = GetPoint(pixel, data->outputSize);
	//if(pixelPoint.x == 0 && origin_label.x == 0 && pixelPoint.y == origin_label.y)
	//	saliency += 100;
	//else 
	//	if(pixelPoint.x == data->outputSize.width - 1 && origin_label.x == data->inputSize.width - 1 && pixelPoint.y == origin_label.y)
	//	saliency += 0;
	//else
	//	saliency += 10000;

	CvScalar value = cvGet2D(data->saliency, origin_label.y, origin_label.x);
	saliency += value.val[0] + value.val[1] + value.val[2]; 	
 
	return saliency ;
 
}

int dataFunctionShiftmap(int pixel, int label, void *extraData)
{	
	ForDataFunction *data = (ForDataFunction *) extraData;
	
	// position of output pixel
	CvPoint origin_label = GetMappedPoint(pixel, label, data->outputSize, data->shiftSize);
	if(IsOutside(origin_label, data->inputSize)) 
		return 100000;

	double saliency = 0;

	//// force the 2 left-most & right-most col to be in the output
	//CvPoint pixelPoint = GetPoint(pixel, data->outputSize);
	//if(pixelPoint.x == 0 && origin_label.x == 0 && pixelPoint.y == origin_label.y)
	//	saliency += 100;
	//else 
	//	if(pixelPoint.x == data->outputSize.width - 1 && origin_label.x == data->inputSize.width - 1 && pixelPoint.y == origin_label.y)
	//	saliency += 0;
	//else
	//	saliency += 10000;

	CvScalar value = cvGet2D(data->saliency, origin_label.y, origin_label.x);
	saliency += (value.val[0] + value.val[1] + value.val[2]); 	
 
	return saliency ;
 
}

int smoothFunctionShiftmapH(int pixel1, int pixel2, int label1, int label2, void* extraData)
{  
	ForSmoothFunctionH *data = (ForSmoothFunctionH *) extraData;

	CvPoint pixelPoint1 = GetPoint(pixel1, data->outputSize);	
	CvPoint labelPoint1 = GetMappedPointInitialGuess(pixel1, label1, data->outputSize, data->shiftSize, data->initialGuess);	 
	
	CvPoint pixelPoint2 = GetPoint(pixel2, data->outputSize);	
	CvPoint labelPoint2 = GetMappedPointInitialGuess(pixel2, label2, data->outputSize, data->shiftSize, data->initialGuess);	 

	if(IsOutside(labelPoint1, data->inputSize) || IsOutside(labelPoint2, data->inputSize))
		return 100000; // prevent mapping outside image
 
	//return 50;
	// pre-compute variables:
	CvPoint neighbor1 = GetNeighbor(pixelPoint1, pixelPoint2, labelPoint1); // neighbor of label1
	CvPoint neighbor2 = GetNeighbor(pixelPoint2, pixelPoint1, labelPoint2); // neighbor of label2

	int energy = 0;

	energy += SquareColorDifference(labelPoint1, neighbor2, data->image);
	energy += SquareColorDifference(labelPoint2, neighbor1, data->image);  

	// gradient different term 
	energy += 2 * SquareColorDifference(labelPoint2, neighbor1, data->gradient);
	energy += 2 * SquareColorDifference(labelPoint1, neighbor2, data->gradient);
	
	return  energy;
}


int smoothFunctionShiftmap(int pixel1, int pixel2, int label1, int label2, void* extraData)
{  
	ForSmoothFunction *data = (ForSmoothFunction *) extraData;
	
	CvPoint pixelPoint1 = GetPoint(pixel1, data->outputSize);	
	CvPoint labelPoint1 = GetMappedPoint(pixel1, label1, data->outputSize, data->shiftSize);	 
	CvPoint pixelPoint2 = GetPoint(pixel2, data->outputSize);	
	CvPoint labelPoint2 = GetMappedPoint(pixel2, label2, data->outputSize, data->shiftSize);	 
	

	if(IsOutside(labelPoint1, data->inputSize) || IsOutside(labelPoint2, data->inputSize))
		return 100000; // prevent mapping outside image
 
	//return 50;
	// pre-compute variables:
	CvPoint neighbor1 = GetNeighbor(pixelPoint1, pixelPoint2, labelPoint1); // neighbor of label1
	CvPoint neighbor2 = GetNeighbor(pixelPoint2, pixelPoint1, labelPoint2); // neighbor of label2

	int energy = 0;

	energy += SquareColorDifference(labelPoint1, neighbor2, data->image);
	energy += SquareColorDifference(labelPoint2, neighbor1, data->image);  

	// gradient different term 
	energy += 2 * SquareColorDifference(labelPoint2, neighbor1, data->gradient);
	energy += 2 * SquareColorDifference(labelPoint1, neighbor2, data->gradient);
	
	return  energy;
}

CvScalar ColorDiffenrece(CvPoint point1, CvPoint point2, IplImage* image)
{
	if(!IsInsideImage(point1, image->width, image->height) || !IsInsideImage(point2, image->width, image->height))
		return cvScalar(0,0,0,0);

	CvScalar value1 = cvGet2D(image, point1.y, point1.x);
	CvScalar value2 = cvGet2D(image, point2.y, point2.x);
	return cvScalar(value1.val[0] - value2.val[0], value1.val[1] - value2.val[1], value1.val[2] - value2.val[2]);
}

int SquareColorDifference(CvPoint point1, CvPoint point2, IplImage* image)
{
	if(!IsInsideImage(point1, image->width, image->height) || !IsInsideImage(point2, image->width, image->height))
		return 1000000;

	CvScalar value1 = cvGet2D(image, point1.y, point1.x);
	CvScalar value2 = cvGet2D(image, point2.y, point2.x);

	return SquareDifference(value1, value2);
}

int SquareDifference(CvScalar value1, CvScalar value2)
{
	return pow(value1.val[0]-value2.val[0], 2) + pow(value1.val[1]-value2.val[1], 2) + pow(value1.val[2]-value2.val[2], 2);
}

bool IsNeighbor(CvPoint point1, CvPoint point2)
{
	if(abs(point1.x - point2.x) <= 1 && abs(point2.y - point1.y) <=1)
		return true;
	else
		return false;
}

CvPoint GetNeighbor(CvPoint pixel1, CvPoint pixel2, CvPoint label)
{
	return cvPoint(label.x - pixel1.x + pixel2.x, label.y - pixel1.y + pixel2.y);
}

bool IsInsideImage(CvPoint point, int width, int height)
{
	if(point.x < 0 || point.y < 0 || point.x + 1 > width || point.y + 1 > height)
		return false;
	return true;
}