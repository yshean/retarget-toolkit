#include "StdAfx.h"
#include "EnergyFunction3D.h"

 int dataFunctionShiftmap3D(int pixel, int label, void *extraData)
 {
	 ForDataFunction3D *data = (ForDataFunction3D *) extraData;
	
	// position of output pixel
	//CvPoint origin_label = GetMappedPoint3D(pixel, label, data->outputSize, data->shiftSize);
	
	// commented for now
	//if(IsOutside(origin_label, data->inputSize)) 
	//	return 100000;

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

	//CvScalar value = cvGet2D(data->saliency, origin_label.y, origin_label.x);
	//saliency += (value.val[0] + value.val[1] + value.val[2]); 	
 
	//return saliency ;
	return 0;
 }