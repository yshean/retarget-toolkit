#include "stdafx.h"
#include "TangVideoCollage.h"

TangVideoCollage::TangVideoCollage(void)
{
	
}

IplImage* TangVideoCollage::GetCollage(void)
{
	IplImage* result;
	return result;
}

int TangVideoCollage::GetMostRelevanceFrame(int start, int end)
{
	double max_assessment;
	int key = start;
	for(int i = start; i <= end; i++)
	{
		char* filename = (*_sequence->frameList)[i];
		IplImage* frame = cvLoadImage(filename);
		
		double quality = _imageQuality->GetImageQuality(frame);
		double importance = _imageImportance->GetImageImportance(frame);
		double assessment = quality + importance;
		if(assessment > max_assessment)
		{
			max_assessment = assessment;
			key = i;
		}

		cvReleaseImage(&frame);
	}
	return key;
}

void TangVideoCollage::KeyFrameSelection1(SubShot *subShot, int *key)
{
	if(subShot->shotType != VCSHOT_STATIC || subShot->shotType != VCSHOT_ZOOM)
	{
		*key = -1;
		printf("TangeVideoCollage::KeyFrameSelection1: Wrong Sub Shot Type, only accept VCSHOT_STATIC and VCSHOT_ZOOM");
		return;
	}

	int start = subShot->start;
	int end = subShot->end;

	*key = GetMostRelevanceFrame(start, end);
}

void TangVideoCollage::KeyFrameSelection2(SubShot *subShot, int *key)
{
	if(subShot->shotType != VCSHOT_PAN || subShot->shotType != VCSHOT_TILT)
	{
		*key = -1;
		printf("TangeVideoCollage::KeyFrameSelection1: Wrong Sub Shot Type, only accept VCSHOT_STATIC and VCSHOT_ZOOM");
		return;
	}
	
	int start = subShot->start;
	int end = subShot->end;
	int middle = (end - start) / 2;
	*key = GetMostRelevanceFrame(start, middle);
	key++;
	*key = GetMostRelevanceFrame(middle + 1, end);
}




VCSolution* TangVideoCollage::GetSolution(ShotInfo* shotInfo)
{
	VCSolution* solution;
	return solution;
}
