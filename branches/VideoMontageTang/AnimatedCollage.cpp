#include "StdAfx.h"
#include "AnimatedCollage.h"

AnimatedCollage::AnimatedCollage(ShotInfo* shotInfo, VideoSequence* sequence, 
		VideoSequence* selectedFrame, vector<LayoutFrame*>* layoutList)
{
	_shotInfo = shotInfo;
	_sequence = sequence;
	_selectedFrame = selectedFrame;
	_layoutList = layoutList;
}

AnimatedCollage::~AnimatedCollage(void)
{
}

int AnimatedCollage::GetClickedLayoutIndex(int x, int y)
{
	int size = _layoutList->size();
	for(int i = 0 ; i < size; i++)
	{
		LayoutFrame* current_frame = (*_layoutList)[i];
		CvPoint point = GetLayoutCanvasPosition(current_frame);
		if(x >= point.x && x <= point.x + current_frame->size.width
			&& y > point.y && y <= point.y + current_frame->size.height)
		{
			return i;
		}
	}
}

SubShot* AnimatedCollage::GetChosenSubShot(int selectedFrame)
{
	for(int i = 0; i < _shotInfo->shotCount; i++)
	{
		Shot* shot = _shotInfo->shotList;
		Shot current_shot = shot[i];
		for(int j = 0; j < current_shot.subShotCount; j++)
		{
			SubShot* subShotList = current_shot.subShotList;
			SubShot current_subshot = subShotList[j];
			if(current_subshot.start <= selectedFrame && current_subshot.end >= selectedFrame)
				return &current_subshot;
		}		
	}
}

int AnimatedCollage::GetFrameIndex(char* filename)
{
	int size = _sequence->frameList->size();
	for(int i = 0; i < size; i++)
	{
		char* index_file = (*(_sequence->frameList))[i];
		if(strcmp(filename, index_file) == 0)
		{
			return i;
		}
	}
	return -1;
}

SubShot* AnimatedCollage::GetClickedSubShot(int x, int y)
{
	int layoutIndex = GetClickedLayoutIndex(x, y);
	char* filename = (*(_selectedFrame->frameList))[layoutIndex];
	int frameIndex = GetFrameIndex(filename);
	SubShot* chosenSubShot = GetChosenSubShot(frameIndex);
	return chosenSubShot;
}