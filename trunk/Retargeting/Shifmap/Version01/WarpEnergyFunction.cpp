#include "StdAfx.h"
#include "WarpEnergyFunction.h"

WarpEnergyFunction::WarpEnergyFunction(void)
{
}

WarpEnergyFunction::~WarpEnergyFunction(void)
{
}

int WarpEnergyFunction::GetDataCost(int labelId, int nodeId)
{
	int penaltyCost = 7000000;
	//return GetDataCostOrigin(labelId, nodeId, penaltyCost);
	return GetDataWarpDistort(labelId, nodeId, penaltyCost);
}

int WarpEnergyFunction::GetSmoothCostPreserveObject(int labelId1, int labelId2, int nodeId1, int nodeId2, int penaltyCost)
{
	int energy = GetSmoothCostOrigin(labelId1, labelId2, nodeId1, nodeId2, penaltyCost);
	if(energy == penaltyCost)
		return energy;

	int scaleId1 = _labelMapping->GetScaleId(labelId1);
	int scaleId2 = _labelMapping->GetScaleId(labelId2);
	CvMat* distortMat1,* distortMat2;
	//if(scaleId1 == 0)
	//	distortMat1 = _importanceMap;
	//else
	//	distortMat1 = (*_distortionList)[scaleId1 - 1];
	//if(scaleId2 == 0)
	//	distortMat2 = _importanceMap;
	//else
	//	distortMat2 = (*_distortionList)[scaleId2 - 1];
	
	//distortMat1 = (*_importanceList)[scaleId1];
	//distortMat2 = (*_importanceList)[scaleId2];
	distortMat1 = (*_distortionList)[scaleId1];
	distortMat2 = (*_distortionList)[scaleId2];

	CvPoint point1 = _mapping2D->GetMappedPoint(nodeId1);
	CvPoint point2 = _mapping2D->GetMappedPoint(nodeId2);
	CvPoint mappedPoint1 = _labelMapping->GetMappedPointInt(labelId1, point1);
	CvPoint mappedPoint2 = _labelMapping->GetMappedPointInt(labelId2, point2);
	
	CvScalar distort1 = cvGet2D(distortMat1,  mappedPoint1.y , mappedPoint1.x);
	CvScalar distort2 = cvGet2D(distortMat2, mappedPoint2.y, mappedPoint2.x);

	//printf("Smooth at %i %i \n", point1.x, point1.y);
	//return energy;
	int threshold = 60;
	if(distort1.val[0] > threshold || distort2.val[0] > threshold)
		if(labelId1 != labelId2)
			// return 10 * energy + 20;
			return penaltyCost;
	//(*_distortionList)[scaleId|
	return energy;
}

int WarpEnergyFunction::GetSmoothCost(int labelId1, int labelId2, int nodeId1, int nodeId2)
{	
	int penaltyCost = 7000000;
	// return GetSmoothCostOrigin(labelId1, labelId2, nodeId1, nodeId2, penaltyCost) / 10;
	// return GetSmoothCostGradient(labelId1, labelId2, nodeId1, nodeId2, penaltyCost);
	return GetSmoothCostPreserveObject(labelId1, labelId2, nodeId1, nodeId2, penaltyCost);
}

double WarpEnergyFunction::GetScaleDistortion(IplImage* image, IplImage* image2, CvPoint point, 
											  double scaleX, double scaleY, int patchSize)
{
	int scaledX = point.x / scaleX;
	int scaledY = point.y / scaleY;
	// int scaledY = point.y;
	int scaledPatchX = patchSize / scaleX;
	int scaledPatchY = patchSize / scaleY;
 
	double maxDiff = 0;
	for(int x = scaledX; x < scaledX + scaledPatchX - patchSize + 1; x++)
	{
		for(int y = scaledY; y < scaledY + scaledPatchY - patchSize + 1; y++)
		{
			double diff = 0;		
			for(int i = 0; i < patchSize; i++)
				for(int j = 0; j < patchSize; j++)
				{	 
					//printf("Test: %i %i Point: %i %i\n", i + point.x, point.y + j, point.x, point.y);
					CvScalar value1 = cvGet2D(image2, j + point.y, i + point.x);
					CvScalar value2 = cvGet2D(image, j + y, i + x);				
					diff += SquareDifference(value1, value2);
					
				}
			
			if(maxDiff < diff)
				maxDiff = diff;
		}
	
	}
	return maxDiff / (patchSize * patchSize);
}
int WarpEnergyFunction::OverlapNumber(CvPoint point, int patchSize, int width, int height)
{
	int minX = point.x - patchSize + 1;
	if(minX < 0)
		minX = 0;
	int minY = point.y - patchSize + 1;
	if(minY < 0)
		minY = 0;
	if(point.x > width - patchSize + 1)
		point.x = width - patchSize + 1;
	if(point.y > height - patchSize + 1)
		point.y = height - patchSize + 1;

	return (point.x - minX + 1) * (point.y - minY + 1);
}

CvMat* WarpEnergyFunction::GetDistortionMat(IplImage* image, double scaleX, double scaleY)
{	
	int width = image->width * scaleX;
	int height = image->height * scaleY;

	IplImage* scaledImage = cvCreateImage(cvSize(width, height), image->depth, image->nChannels);
	cvResize(image, scaledImage);
	CvMat* res = cvCreateMat(scaledImage->height, scaledImage->width, CV_32FC1);
	
	for(int i = 0; i < res->cols; i++)
	for(int j = 0; j < res->rows; j++)
		cvSet2D(res, j, i, cvScalar(0,0,0));
	 	
	int patchSize = 3;		
	for(int x = 0; x < scaledImage->width - patchSize; x++)	 
	for(int y = 0; y < scaledImage->height - patchSize; y++)
		{
			double diff = GetScaleDistortion(image, scaledImage, cvPoint(x, y), scaleX, scaleY, patchSize);

			for(int i = 0; i < patchSize; i++)
				for(int j = 0; j < patchSize; j++)
				{
					CvPoint curr = cvPoint(x + i, y + j);						
					CvScalar value = cvGet2D(res, curr.y, curr.x);
					int overlapCount = OverlapNumber(curr, patchSize, scaledImage->width, scaledImage->height);
					diff = diff / overlapCount;
					value.val[0] += diff;						
					cvSet2D(res, curr.y, curr.x, value);		
				}				
		}			
 
	return res;
}
void WarpEnergyFunction::InitializeImportanceMap()
{
	CvMat* result_mat = cvCreateMat(_inputSize.height, _inputSize.width,CV_32FC1);
	int scale_count = 9;

	// reset result_mat
	for(int i = 0; i < result_mat->cols; i++)
		for(int j = 0; j < result_mat->rows; j++)
		{
			cvSet2D(result_mat, j, i, cvScalar(0));
		}

	for(int a = 1; a <= scale_count; a++)
	{
		CvMat* mat = GetDistortionMat(_image, 1 - a * 0.1, 1);
		CvMat* temp_mat = cvCreateMat(_inputSize.height, _inputSize.width,CV_32FC1);
		cvResize(mat, temp_mat);		
		
		for(int i = 0; i < temp_mat->cols; i++)
			for(int j = 0; j < temp_mat->rows; j++)
			{
				CvScalar value = cvGet2D(temp_mat, j, i);
				CvScalar temp = cvGet2D(result_mat, j, i);
				 
				for(int k = 0; k < 4; k++)
				{
					value.val[k] /= scale_count;
				}
				 
				for(int k = 0; k < 4; k++)
				{
					temp.val[k] += value.val[k];
				}
				 
				cvSet2D(result_mat, j, i, temp);
			}
		
	}

	int max = 0;
	int min = 10000000;
	for(int i = 0; i < result_mat->cols; i++)
		for(int j = 0; j < result_mat->rows; j++)
		{
			CvScalar temp = cvGet2D(result_mat, j, i);
			if(temp.val[0] < min)
				min = temp.val[0];
			if(temp.val[0] > max)
				max = temp.val[0];
		}
	for(int i = 0; i < result_mat->cols; i++)
		for(int j = 0; j < result_mat->rows; j++)
		{
			CvScalar temp = cvGet2D(result_mat, j, i);
			temp.val[0] = (temp.val[0] - min) * 255 / max;
			cvSet2D(result_mat, j, i, temp);
		}
	// _importanceMap = result_mat;
	(*_distortionList)[0] = result_mat;

	//IplImage* avg_img = cvCloneImage(setting.input);
	//for(int i = 0; i < avg_img->width; i++)
	//	for(int j = 0; j < avg_img->height; j++)
	//	{
	//		CvScalar temp = cvGet2D(result_mat, j ,i);
	//		cvSet2D(avg_img, j , i, temp);
	//	}

	//cvSaveImage("test.jpg", avg_img);
}
void WarpEnergyFunction::IntializeDistortionMeasure()
{	
	int scaleCount = _labelMapping->GetScaleCount();
	_distortionList = new vector<CvMat*>();
	
	for(int i = 0; i < scaleCount; i++)
	{
		double ratioX = _labelMapping->GetScaleX(i);
		double ratioY = _labelMapping->GetScaleY(i);
		int width = _image->width * ratioX;
		int height = _image->width * ratioY; 
		CvMat* res = GetDistortionMat(_image, ratioX, ratioY);
		_distortionList->push_back(res);
	}

	//int max = 0;
	//CvMat* result = (*distortionList)[0];
	//for(int i = 0; i < result->cols; i++)
	//for(int j = 0; j < result->rows; j++)
	//{
	//	CvScalar value = cvGet2D(result, j, i);
	//	if(max < value.val[0])
	//		max = value.val[0];
	//}

	//IplImage* img = cvCreateImage(cvSize(result->cols, result->rows), IPL_DEPTH_8U, 1);
	//for(int i = 0; i < result->cols; i++)
	//for(int j = 0; j < result->rows; j++)
	//{
	//	CvScalar value = cvGet2D(result, j, i);
	//	value.val[0] *= 255;
	//	value.val[0] /= max;
	//	cvSet2D(img, j, i, value);
	//}

	//DisplayImage(img, "Test");
	
}
void WarpEnergyFunction::InitializeDistortionMeasure(IplImage* saliency)
{
	
	// keep values in saliency for 1 scale
	//CvMat* mat = cvCreateMat(saliency->height, saliency->width, CV_32FC1);
	//for(int i = 0; i < saliency->width; i++)
	//	for(int j = 0; j < saliency->height; j++)
	//	{
	//		CvScalar value = cvGet2D(saliency, j, i);
	//		cvSet2D(mat, j, i, value);
	//	}
	//
	//_importanceList->push_back(mat);

	
	// importance
	_importanceList = new vector<CvMat*>();
	for(int k = 0; k < _labelMapping->GetScaleCount(); k++)
	{
		double scaleX = _labelMapping->GetScaleX(k);
		double scaleY = _labelMapping->GetScaleY(k);
		IplImage* temp_img = cvCreateImage(cvSize(saliency->width * scaleX, saliency->height * scaleY), 
			saliency->depth, saliency->nChannels);
		cvResize(saliency, temp_img);
		CvMat* mat = cvCreateMat(temp_img->height, temp_img->width, CV_32FC1);
		for(int i = 0; i < temp_img->width; i++)
			for(int j = 0; j < temp_img->height; j++)
			{
				CvScalar value = cvGet2D(saliency, j, i);
				value.val[0] = 255 - value.val[0];
				cvSet2D(mat, j, i, value);
			}	
		_importanceList->push_back(mat);
	}

	// distortion
	_distortionList = new vector<CvMat*>();
	// first level no distortion
	CvMat* mat = cvCreateMat(saliency->height, saliency->width, CV_32FC1);
	for(int i = 0; i < mat->cols; i++)
		for(int j = 0; j < mat->rows; j++)
		{
			cvSet2D(mat, j, i, cvScalar(0,0,0));
		}
	_distortionList->push_back(mat);
	// next level
	for(int k = 1; k < _labelMapping->GetScaleCount(); k++)
	{
		double scaleX = _labelMapping->GetScaleX(k);
		double scaleY = _labelMapping->GetScaleY(k);
		IplImage* temp_img = cvCreateImage(cvSize(saliency->width * scaleX, saliency->height * scaleY), 
			saliency->depth, saliency->nChannels);
		cvResize(saliency, temp_img);
		mat = cvCreateMat(temp_img->height, temp_img->width, CV_32FC1);
		for(int i = 0; i < temp_img->width; i++)
			for(int j = 0; j < temp_img->height; j++)
			{
				CvScalar value = cvGet2D(temp_img, j, i);
				value.val[0] = (255 - value.val[0] + 1) * 2 / scaleX;
				if(value.val[0] > 255)
					value.val[0] = 255;
				cvSet2D(mat, j, i, value);
			}		
		_distortionList->push_back(mat);
	}
}


int WarpEnergyFunction::GetSmoothCostWarp(int labelId1, int labelId2, int nodeId1, int nodeId2, int penaltyCost, int pixelDistance)
{
	CvPoint point1 = _mapping2D->GetMappedPoint(nodeId1);
	CvPoint point2 = _mapping2D->GetMappedPoint(nodeId2);

	int energy = 0; 

	DoublePoint mappedPoint1 = _labelMapping->GetMappedPoint(labelId1, point1);
	DoublePoint mappedPoint2 = _labelMapping->GetMappedPoint(labelId2, point2);
	
	if(!IsInside(mappedPoint1, _inputSize) || !IsInside(mappedPoint2, _inputSize))
		return penaltyCost;


	DoublePoint mappedNeighbor1 = _labelMapping->GetMappedPoint(labelId1, point2);
	DoublePoint mappedNeighbor2 = _labelMapping->GetMappedPoint(labelId2, point1);

	if(!IsInside(mappedNeighbor1, _inputSize) || !IsInside(mappedNeighbor2, _inputSize))
		return penaltyCost;

	if(_labelMapping->GetScaleId(labelId1) == _labelMapping->GetScaleId(labelId2))
	{
		if(labelId1 != labelId2)
		{
			return penaltyCost;
		}
	}
	

	energy += GetColorDifference(mappedPoint1, mappedNeighbor2, _image);
	energy += GetColorDifference(mappedPoint2, mappedNeighbor1, _image);
	energy += GetColorDifference(mappedPoint1, mappedNeighbor2, _gradient);
	energy += GetColorDifference(mappedPoint2, mappedNeighbor1, _gradient);



	return energy;
}

int WarpEnergyFunction::GetSmoothCostGradient(int labelId1, int labelId2, int nodeId1, int nodeId2, int penaltyCost)
{
	CvPoint point1 = _mapping2D->GetMappedPoint(nodeId1);
	CvPoint point2 = _mapping2D->GetMappedPoint(nodeId2);

	int energy = 0; 

	DoublePoint mappedPoint1 = _labelMapping->GetMappedPoint(labelId1, point1);
	DoublePoint mappedPoint2 = _labelMapping->GetMappedPoint(labelId2, point2);
	
	if(!IsInside(mappedPoint1, _inputSize) || !IsInside(mappedPoint2, _inputSize))
		return penaltyCost;

	DoublePoint mappedNeighbor1 = _labelMapping->GetMappedPoint(labelId1, point2);
	DoublePoint mappedNeighbor2 = _labelMapping->GetMappedPoint(labelId2, point1);

	if(!IsInside(mappedNeighbor1, _inputSize) || !IsInside(mappedNeighbor2, _inputSize))
		return penaltyCost;

	int scaleId1 = _labelMapping->GetScaleId(labelId1);
	int scaleId2 = _labelMapping->GetScaleId(labelId2);
	if(scaleId1 == scaleId2)
	{
		energy += GetColorDifference(mappedPoint1, mappedNeighbor2, _image);
		energy += GetColorDifference(mappedPoint2, mappedNeighbor1, _image);
	}
	energy += GetColorDifference(mappedPoint1, mappedNeighbor2, _gradient);
	energy += GetColorDifference(mappedPoint2, mappedNeighbor1, _gradient);

	return energy;
}
int WarpEnergyFunction::GetDataWarpDistort(int labelId, int nodeId, int penaltyCost)
{
	CvPoint point = _mapping2D->GetMappedPoint(nodeId);
	
	DoublePoint mappedPoint = _labelMapping->GetMappedPoint(labelId, point);

	if(mappedPoint.x < 0 || mappedPoint.x > _inputSize.width - 1)
		return penaltyCost;
	if(mappedPoint.y < 0 || mappedPoint.y > _inputSize.height - 1)
		return penaltyCost;


	if(!IsSatisfiedBoundary(point.x, mappedPoint.x, _inputSize.width, _outputSize.width))
		return penaltyCost; 
	if(!IsSatisfiedBoundary(point.y, mappedPoint.y, _inputSize.height, _outputSize.height))
		return penaltyCost;

	int scaleId = _labelMapping->GetScaleId(labelId);
	double scaleX = _labelMapping->GetScaleX(scaleId);
	double scaleY = _labelMapping->GetScaleY(scaleId);
	int mappedX = mappedPoint.x * scaleX;
	int mappedY = mappedPoint.y * scaleY;

	CvMat* distortMat = (*_distortionList)[scaleId];
	CvScalar value = cvGet2D(distortMat, mappedY, mappedX);

	//if(scaleId > 2 )
	//	DisplayImage(distortMat, "TEST");
	//if(value.val[0] > 1000)
	//	printf("Test");
	return value.val[0];
	//return 0;

	// no saliency for now Chau 30Nov2010
	// CvScalar saliency = GetInterpolatedValue(mappedPoint, _saliency);
	 
	// return saliency.val[0] + saliency.val[1] + saliency.val[2];
}