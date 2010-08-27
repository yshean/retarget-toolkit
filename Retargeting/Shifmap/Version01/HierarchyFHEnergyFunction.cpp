#include "StdAfx.h"
#include "HierarchyFHEnergyFunction.h"

 int dataFunctionFHHierarchy(int pixel, int label, void *extraData)
 {
	ForDataFHHierarchy* forData = (ForDataFHHierarchy*) extraData;
	CvPoint* point = (*(forData->pointMapping))[pixel];
	
	CvPoint origin;
	CvPoint shift = GetShift(label, forData->shiftSize);
	CvPoint guess = GetLabel(*point, forData->guess);
	origin.x = point->x + shift.x + guess.x;
	origin.y = point->y + shift.y + guess.y;
	
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

	return 2 * energy + saliency;
 }

 int smoothFunctionFHHierarchy(int pixel1, int pixel2, int label1, int label2, void* extraData)
 {
	ForSmoothFHHierarchy* data = (ForSmoothFHHierarchy*)extraData;
	CvPoint* point1 = (*(data->pointMapping))[pixel1];
	CvPoint* point2 = (*(data->pointMapping))[pixel2];
	CvPoint shift1 = GetShift(label1, data->shiftSize);
	CvPoint shift2 = GetShift(label2, data->shiftSize);
	CvPoint guess1 = GetLabel(*point1, data->guess);
	CvPoint guess2 = GetLabel(*point2, data->guess);
	CvPoint origin1 = cvPoint(shift1.x + point1->x + guess1.x, shift1.y + point1->y + guess1.y);
	CvPoint origin2 = cvPoint(shift2.x + point2->x + guess2.x, shift2.y + point2->y + guess2.y);
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
	return 2 * energy;
 }