#include "StdAfx.h"
#include "FillHoleEnergyFunction.h"

int dataFunctionFH(int pixel, int label, void *extraData)
{
	ForDataFH* forData = (ForDataFH*) extraData;
	CvPoint* point = (*(forData->pointMapping))[pixel];
	
	CvPoint origin;
	CvPoint shift = GetShift(label, forData->shiftSize);
	origin.x = point->x + shift.x;
	origin.y = point->y + shift.y;
	
	// penalty if outside
	if(IsOutside(origin, forData->inputSize)) 
		return 1000000;

	// saliency cost
	double saliency = 0;
	CvScalar value = cvGet2D(forData->saliency, origin.y, origin.x);
	saliency += (value.val[0] + value.val[1] + value.val[2]); 	
	
	// smooth cost with predefined region
	int energy = 0;
	value = cvGet2D(forData->maskNeighbor, point->y, point->x);
	if(value.val[0] == 1)
	{
		// compare with up position
		energy += smoothFunctionFHMask(origin, *point, cvPoint(point->x, point->y - 1), 
			forData->input, forData->inputGradient, forData->maskData, forData->maskDataGradient);
	}
	if(value.val[1] == 1)
	{
		// compare with right
		energy += smoothFunctionFHMask(origin, *point, cvPoint(point->x + 1, point->y), 
			forData->input, forData->inputGradient, forData->maskData, forData->maskDataGradient);
	}
	if(value.val[2] == 1)
	{
		// compare with down
		energy += smoothFunctionFHMask(origin, *point, cvPoint(point->x, point->y + 1), 
			forData->input, forData->inputGradient, forData->maskData, forData->maskDataGradient);
	}
	if(value.val[3] == 1)
	{
		// compare with left
		energy += smoothFunctionFHMask(origin, *point, cvPoint(point->x - 1, point->y), 
			forData->input, forData->inputGradient, forData->maskData, forData->maskDataGradient);
	}

	return energy + saliency;
}

int dataFunctionFH2(int pixel, int label, void *extraData)
{
	ForDataFH2* forData = (ForDataFH2*) extraData;
	CvPoint* point = (*(forData->pointMapping))[pixel];
	
	CvPoint origin;
	CvPoint shift = GetShift(label, forData->shiftSize);
	origin.x = point->x + shift.x;
	origin.y = point->y + shift.y;
	
	// penalty if outside
	if(IsOutside(origin, forData->inputSize)) 
		return 100000;

	// saliency cost
	double saliency = 0;
	CvScalar value = cvGet2D(forData->saliency, origin.y, origin.x);
	saliency += (value.val[0] + value.val[1] + value.val[2]); 	
	
	// smooth cost with predefined region
	int energy = 0;
	value = cvGet2D(forData->maskNeighbor, point->y, point->x);
	if(value.val[0] == 1)
	{
		// compare with up position
		energy += smoothFunctionFHMask2(origin, *point, cvPoint(point->x, point->y - 1), forData);
	}
	if(value.val[1] == 1)
	{
		// compare with right
		energy += smoothFunctionFHMask2(origin, *point, cvPoint(point->x + 1, point->y), 
			forData );
	}
	if(value.val[2] == 1)
	{
		// compare with down
		energy += smoothFunctionFHMask2(origin, *point, cvPoint(point->x, point->y + 1), 
			forData);
	}
	if(value.val[3] == 1)
	{
		// compare with left
		energy += smoothFunctionFHMask2(origin, *point, cvPoint(point->x - 1, point->y), 
			forData);
	}

	return energy + saliency;
}
int smoothFunctionFH(int pixel1, int pixel2, int label1, int label2, void* extraData)
{
	ForSmoothFH* data = (ForSmoothFH*)extraData;
	CvPoint* point1 = (*(data->pointMapping))[pixel1];
	CvPoint* point2 = (*(data->pointMapping))[pixel2];
	CvPoint shift1 = GetShift(label1, data->shiftSize);
	CvPoint shift2 = GetShift(label2, data->shiftSize);
	CvPoint origin1 = cvPoint(shift1.x + point1->x, shift1.y + point1->y);
	CvPoint origin2 = cvPoint(shift2.x + point2->x, shift2.y + point2->y);
	CvPoint neighbor1 = GetNeighbor(*point1, *point2, origin1);
	CvPoint neighbor2 = GetNeighbor(*point2, *point1, origin2);

	CvSize inputSize = cvSize(data->input->width, data->input->height);
	if(IsOutside(origin1, inputSize) || IsOutside(origin2, inputSize)
		|| IsOutside(neighbor1, inputSize) || IsOutside(neighbor2, inputSize))
		return 10000;

	int energy = 0;
	energy += SquareColorDifference(origin1, neighbor2, data->input);
	energy += SquareColorDifference(origin2, neighbor1, data->input);
	energy += SquareColorDifference(origin1, neighbor2, data->inputGradient);
	energy += SquareColorDifference(origin2, neighbor1, data->inputGradient);	
}
int smoothFunctionFHMask(CvPoint inputPoint, CvPoint outputPoint, CvPoint knownPoint, IplImage* input, IplImage* inputGradient,
				 IplImage* maskOutput, IplImage* maskOutputGradient)
{
	
	CvPoint inputNeighbor = GetNeighbor(knownPoint, outputPoint, inputPoint); // neighbor of inputPoint
	// neighbor of the knownPoint is actually the outputPoint
	
	int energy = 0;
	if(IsOutside(inputNeighbor, cvSize(input->width, input->height)))
	{
		return 100000;
	}
	
	// data term
	energy += SquareColorDifference(inputNeighbor, input, knownPoint, maskOutput);
	energy += SquareColorDifference(outputPoint, maskOutput, inputPoint, input);

	// gradient different term 
	energy += 2 * SquareColorDifference(inputNeighbor, inputGradient, knownPoint, maskOutputGradient);
	energy += 2 * SquareColorDifference(outputPoint, maskOutputGradient, inputPoint, inputGradient);
	
	return energy;
}
int smoothFunctionFHMask2(CvPoint inputPoint, CvPoint outputPoint, CvPoint knownPoint, ForDataFH2* forData)
{
	
	CvPoint inputNeighbor = GetNeighbor(knownPoint, outputPoint, inputPoint); // neighbor of inputPoint
	// neighbor of the knownPoint is actually the outputPoint
	
	int energy = 0;
	if(IsOutside(inputNeighbor, cvSize(forData->input->width, forData->input->height)))
	{
		return 100000;
	}
	
	// data term
	energy += SquareColorDifference(inputNeighbor, forData->input, knownPoint, forData->mask, forData->input);
	energy += SquareColorDifference(inputPoint, forData->input, outputPoint, forData->mask, forData->input);

	// gradient different term 
	energy += 2 * SquareColorDifference(inputNeighbor, forData->inputGradient, knownPoint, forData->mask, forData->inputGradient);
	energy += 2 * SquareColorDifference(inputPoint, forData->inputGradient, outputPoint, forData->mask, forData->inputGradient);
	
	return energy;
}
int SquareColorDifference(CvPoint point, IplImage* image, CvPoint pointMask, MaskShift* mask, IplImage* maskSource)
{
	CvScalar value1 = cvGet2D(image, point.y, point.x);
	CvScalar value2 = mask->GetPixelValue(pointMask, maskSource);
	return SquareDifference(value1, value2);
}

int SquareColorDifference(CvPoint point1, IplImage* image1, CvPoint point2, IplImage* image2)
{
	CvScalar value1 = cvGet2D(image1, point1.y, point1.x);
	CvScalar value2 = cvGet2D(image2, point2.y, point2.x);
	if(value1.val[0] == 231 && value1.val[1] == 233 && value1.val[2] == 233)
		printf("Test");
	if(value2.val[0] == 231 && value2.val[1] == 233 && value1.val[2] == 233)
		printf("Test");
	return SquareDifference(value1, value2);
}
void SetMaskNeighbor(CvPoint current, CvPoint neighbor, CvScalar* value)
{
	if(neighbor.x == current.x + 1 && neighbor.y == current.y)
	{
		// right
		value->val[1] = 1;
	}
	else
	if(neighbor.x == current.x - 1 && neighbor.y == current.y)
	{
		// left
		value->val[3] = 1;
	}
	else
	if(neighbor.x == current.x && neighbor.y == current.y + 1)
	{
		// down
		value->val[2] = 1;
	}
	else
	if(neighbor.x == current.x && neighbor.y == current.y - 1)
	{
		// up
		value->val[0] = 1;
	}
}
vector<CvPoint*>* GetMaskNeighbor(CvScalar value, CvPoint current)
{
	vector<CvPoint*>* result = new vector<CvPoint*>();
	if(value.val[0] == 1)
	{
		CvPoint* point = new CvPoint();
		point->x = current.x;
		point->y = current.y - 1;
		result->push_back(point);
	}
	else
	if(value.val[1] == 1)
	{
		CvPoint* point = new CvPoint();
		point->x = current.x + 1;
		point->y = current.y;
		result->push_back(point);
	}
	else
	if(value.val[2] == 1)
	{
		CvPoint* point = new CvPoint();
		point->x = current.x;
		point->y = current.y + 1;
		result->push_back(point);
	}
	else
	if(value.val[3] == 1)
	{
		CvPoint* point = new CvPoint();
		point->x = current.x - 1;
		point->y = current.y;
		result->push_back(point);
	}
	return result;
}