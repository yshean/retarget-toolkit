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





VideoSequence* TangVideoCollage::GetSolution2(ShotInfo* shotInfo)
{
	VideoSequence* solution = new VideoSequence();
	solution->frameList = new vector<char*>();
	
	int count = GetSolutionFrameCount(shotInfo);
	 
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
				
				solution->frameList->push_back((*(_sequence->frameList))[*selectedShot]);
				solution->frameList->push_back((*(_sequence->frameList))[*(selectedShot + 1)]);
				/*solution->frameList[index] = image1;
				solution->frameList[index + 1] = image2;*/
				index += 2;
				}
				break;
			case VCSHOT_STATIC:
			case VCSHOT_ZOOM:
				{
				printf("\nProcessing STATIC/ZOOM Shot %i SubShot %i ...", i, j);
				KeyFrameSelection1(&subShot, selectedShot);
				IplImage* image = LoadFrame(_sequence, *selectedShot);
				solution->frameList->push_back((*(_sequence->frameList))[*selectedShot]);
				index++;
				}
				break;
			default:
				printf("unknown subshot type");
				break;
			}
		}
	}
	 
 	return solution;
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

vector<LayoutFrame*>* TangVideoCollage::GetRectCollageLayout(LayoutArrangement* arranger, ShotInfo* shotInfo, VideoSequence* selectedFrames, LayoutFrame* rectLayout)
{	
	int size = selectedFrames->frameList->size();
	ImageImportance* importance = new SobelImageImportance();
	vector<double>* saliencyList = new vector<double>();
	vector<LayoutFrame*>* layoutList = new vector<LayoutFrame*>();
	double total_saliency = 0;
	
	// get sequence image information
	//IplImage* first_image = LoadFrame(selectedFrames, 0);
	//int depth = first_image->depth;
	//int nChannels = first_image->nChannels;

	// process each frame
	for(int i = 0; i < size; i++)
	{
		IplImage* frame = LoadFrame(selectedFrames, i);
		double saliency = importance->GetImageImportance(frame);
		layoutList->push_back(CreateLayoutFrame(0, 0, frame->width, frame->height));
		saliencyList->push_back(saliency);
		total_saliency += saliency;
	}
	// resize each frame layout
	for(int i = 0; i < size; i++)
	{
		ResizeLayout((*layoutList)[i], 10 * (*saliencyList)[i] / total_saliency, false);
	}

	// get collage
	//CollageLayout* collageLayout = new CollageLayout(arranger);	
	//collageLayout->CreateRectLayout(layoutList, rectLayout);
	arranger->CreateRectLayout(layoutList, rectLayout);
	 
	return layoutList; 
}

IplImage* TangVideoCollage::GetFinalCollage(vector<LayoutFrame*>* layoutList, LayoutFrame* rectLayout, VideoSequence* selectedFrames)
{ 
	int size = selectedFrames->frameList->size();
	//ImageImportance* importance = new SobelImageImportance();
	//vector<double>* saliencyList = new vector<double>();
	//vector<LayoutFrame*>* layoutList = new vector<LayoutFrame*>();
	//double total_saliency = 0;
	//
	// get sequence image information
	IplImage* first_image = LoadFrame(selectedFrames, 0);
	int depth = first_image->depth;
	int nChannels = first_image->nChannels;

	//// process each frame
	//for(int i = 0; i < size; i++)
	//{
	//	IplImage* frame = LoadFrame(selectedFrames, i);
	//	double saliency = importance->GetImageImportance(frame);
	//	layoutList->push_back(CreateLayoutFrame(0, 0, frame->width, frame->height));
	//	saliencyList->push_back(saliency);
	//	total_saliency += saliency;
	//}
	//// resize each frame layout
	//for(int i = 0; i < size; i++)
	//{
	//	ResizeLayout((*layoutList)[i], 10 * (*saliencyList)[i] / total_saliency, false);
	//}
	//// get collage
	//CollageLayout* collageLayout = new CollageLayout();
	//LayoutFrame* rectLayout = collageLayout->CreateRectLayout(layoutList, cvRect(0, 0, 1000, 200));
	////
	//LayoutFrame* rectLayout = CreateLayoutFrame(0, 0, 1000, 0);
	//vector<LayoutFrame*>* layoutList = GetRectCollageLayout(shotInfo, selectedFrames, rectLayout);
	// 
	IplImage* canvas = cvCreateImage(cvSize(rectLayout->size.width, rectLayout->size.height), depth, nChannels);
	// draw image
	for(int i = 0; i < size; i++)
	{
		DrawImage((*layoutList)[i], LoadFrame(selectedFrames, i), canvas);
	}	
	return canvas;
}

//SubShot* TangVideoCollage::GetClickedSubShot(int x, int y, vector<LayoutFrame*>* layoutList)
//{
//	int clickedLayoutIndex = GetClickedLayout(x, y, layoutList);
//	
//}




 
//void TangVideoCollage::UpdateCollage(
//		VideoSequence* sequence,
//		ShotInfo* shotInfo, 
//		vector<LayoutFrame*>* layoutList, 
//		SubShot* subShot, int selectedLayout,
//		IplImage* collage,
//		int* currentFrameIndex)
//{
//	
//	if(*currentFrameIndex < subShot->end)
//	{
//		(*currentFrameIndex)++;
//		DrawImage((*layoutList)[selectedLayout], LoadFrame(sequence, *currentFrameIndex), collage);
//	}
//	
//	
//}


void TestPlayableCollageLayout(char* sequencename, char* shotname, char* selectedFrameName)
{
	// loading data
	VideoSequence* sequence = LoadSequenceFromFile(sequencename);
	VideoSequence* selectedFrames = LoadSequenceFromFile(selectedFrameName);
	ShotInfo* shotInfo = LoadShotFromFile(shotname);	

	// get collage part
	ImageImportance* imageImportance = new SobelImageImportance();
	ImageQuality* imageQuality = new SobelImageQuality();
	TangVideoCollage* collage = new TangVideoCollage(imageQuality, imageImportance, sequence, shotInfo);	
	
	LayoutFrame* rectLayout = CreateLayoutFrame(0, 0, 1000, 0);
	LayoutArrangement* arranger = new LayoutResizeArrangement();
	vector<LayoutFrame*>* layoutList = collage->GetRectCollageLayout(arranger, shotInfo, selectedFrames, rectLayout);
	IplImage* collage_image = collage->GetFinalCollage(layoutList, rectLayout, selectedFrames);

	// setup window and mouse event
	int* mouseParam = new int();
	*mouseParam = -1;
	*(mouseParam + 1) = -1;

	cvNamedWindow("Collage");
	cvSetMouseCallback("Collage", collageClicked, mouseParam);	
	
	AnimatedCollage* animation = new AnimatedCollage(shotInfo, sequence, 
		selectedFrames, layoutList);
	while(1)
	{
		animation->UpdateCollage(collage_image, *mouseParam, *(mouseParam + 1) );		
		cvShowImage("Collage", collage_image);
		cvWaitKey(100);
	}

}

void collageClicked(int event_type, int x, int y, int flags, void* param)
  {
	  printf("mouse");
    switch(event_type){ 
      case CV_EVENT_LBUTTONUP:   
		  printf("mouse");
	    // update mouse position
		int* position = (int*)param;
		*position = x;
		*(position + 1) = y;
        break;
	} 
}


void TestTangVideoCollageLayout(char* sequencename, char* shotname)
{
	VideoSequence* sequence = LoadSequenceFromFile(sequencename);
	ShotInfo* shotInfo = LoadShotFromFile(shotname);
	ImageImportance* imageImportance = new SobelImageImportance();
	ImageQuality* imageQuality = new SobelImageQuality();
	TangVideoCollage* collage = new TangVideoCollage(imageQuality, imageImportance, sequence, shotInfo);
	
	
	LayoutFrame* rectLayout = CreateLayoutFrame(0, 0, 1000, 0);
	LayoutArrangement* arranger = new LayoutResizeArrangement();
	vector<LayoutFrame*>* layoutList = collage->GetRectCollageLayout(arranger, shotInfo, sequence, rectLayout);
	IplImage* collage_image = collage->GetFinalCollage(layoutList, rectLayout, sequence);

 
	cvNamedWindow("test");
	while(1)
	{
		cvShowImage("test", collage_image);
		cvWaitKey(100);
	}
}

void TestTangVideoCollage(char* sequencename, char* shotname)
{
	VideoSequence* sequence = LoadSequenceFromFile(sequencename);
	ShotInfo* shotInfo = LoadShotFromFile(shotname);
	ImageImportance* imageImportance = new SobelImageImportance();
	ImageQuality* imageQuality = new SobelImageQuality();

	TangVideoCollage* collage = new TangVideoCollage(imageQuality, imageImportance, sequence, shotInfo);
	VideoSequence* result = collage->GetSolution2(shotInfo);
	SaveVideoSequenceToFile(result, "collageFrame.seq");
	
} 

 









