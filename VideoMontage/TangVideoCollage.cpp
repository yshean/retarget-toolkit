#include "stdafx.h"
#include "TangVideoCollage.h"
 
TangVideoCollage::TangVideoCollage(ImageQuality* imageQuality, ImageImportance* imageImportance, VideoSequence* sequence, ShotInfo* shotInfo) : VideoCollage(sequence)
{
	_imageQuality = imageQuality;
	_imageImportance = imageImportance;
	_shotInfo = shotInfo;
}

IplImage* TangVideoCollage::GetCollage(void)
{
	IplImage* result;
	VCSolution* solution = GetSolution(_shotInfo);
	
	Collage* collage = new Collage();
	result = collage->CreateCollage(solution->frameList, solution->length);
	cvSaveImage("blend.jpg", result);
		printf("\n Image Saved");
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
	if(subShot->shotType != VCSHOT_STATIC && subShot->shotType != VCSHOT_ZOOM)
	{
		*key = -1;
		printf("\nTangeVideoCollage::KeyFrameSelection1: Wrong Sub Shot Type, only accept VCSHOT_STATIC and VCSHOT_ZOOM");
		return;
	}

	int start = subShot->start;
	int end = subShot->end;

	*key = GetMostRelevanceFrame(start, end);
}

void TangVideoCollage::KeyFrameSelection2(SubShot *subShot, int *key)
{
	if(subShot->shotType != VCSHOT_PAN && subShot->shotType != VCSHOT_TILT)
	{
		*key = -1;
		printf("\nTangeVideoCollage::KeyFrameSelection1: Wrong Sub Shot Type, only accept VCSHOT_STATIC and VCSHOT_ZOOM");
		return;
	}
	
	int start = subShot->start;
	int end = subShot->end;
	int middle = start + (end - start) / 2;
	*key = GetMostRelevanceFrame(start, middle);
	key++;
	*key = GetMostRelevanceFrame(middle + 1, end);
}


int TangVideoCollage::GetSolutionFrameCount(ShotInfo* shotInfo)
{
	int count = 0;
	for(int i = 0; i < shotInfo->shotCount; i++)
	{
		for(int j = 0; j < shotInfo->shotList[i].subShotCount; j++)
		{
			SubShot subShot = shotInfo->shotList[i].subShotList[j]; 
			switch(subShot.shotType)
			{
				case VCSHOT_PAN:
				case VCSHOT_TILT:
					count += 2;
					break;
				case VCSHOT_STATIC:
				case VCSHOT_ZOOM:
					count++;
					break;
				default:
					// do nothing for now
					printf("unknown subShot retrieved");
					break;
			}
		}
	}
	return count;
}

VCSolution* TangVideoCollage::GetSolution(ShotInfo* shotInfo)
{
	VCSolution* solution = new VCSolution();	
	int count = GetSolutionFrameCount(shotInfo);
	solution->frameList = (IplImage**) malloc(count * sizeof(long));	
	int index = 0;
	for(int i = 0; i < shotInfo->shotCount; i++)
	{
		Shot shot = shotInfo->shotList[i];
		for(int j = 0; j < shot.subShotCount; j++)
		{
			SubShot subShot = shot.subShotList[j];
			int* selectedShot = (int*)malloc(2 * sizeof(int)); 
			switch(subShot.shotType)
			{
			case VCSHOT_PAN:
			case VCSHOT_TILT:
				{
				printf("\nProcessing PAN/TILT Shot %i SubShot %i ...", i, j);
				KeyFrameSelection2(&subShot, selectedShot);	
				IplImage* image1 = LoadFrame(_sequence, *selectedShot);
				IplImage* image2 = LoadFrame(_sequence, *(selectedShot + 1));
				solution->frameList[index] = image1;
				solution->frameList[index + 1] = image2;
				index += 2;
				}
				break;
			case VCSHOT_STATIC:
			case VCSHOT_ZOOM:
				{
				printf("\nProcessing STATIC/ZOOM Shot %i SubShot %i ...", i, j);
				KeyFrameSelection1(&subShot, selectedShot);
				IplImage* image = LoadFrame(_sequence, *selectedShot);
				solution->frameList[index] = image;
				index++;
				}
				break;
			default:
				printf("unknown subshot type");
				break;
			}
		}
	}
	solution->length = index;
 	return solution;
}


void TestTangVideoCollage(char* sequencename, char* shotname)
{
	VideoSequence* sequence = LoadSequenceFromFile(sequencename);
	ShotInfo* shotInfo = LoadShotFromFile(shotname);
	ImageImportance* imageImportance = new SobelImageImportance();
	ImageQuality* imageQuality = new SobelImageQuality();

	VideoCollage* collage = new TangVideoCollage(imageQuality, imageImportance, sequence, shotInfo);
	collage->GetCollage();	
}