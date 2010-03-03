#include "StdAfx.h"
#include "AnimatedCollage.h"

AnimatedCollage::AnimatedCollage(ShotInfo* shotInfo, VideoSequence* sequence, 
		VideoSequence* selectedFrames, vector<LayoutFrame*>* layoutList)
{
	_shotInfo = shotInfo;
	_sequence = sequence;
	_selectedFrames = selectedFrames;
	_layoutList = layoutList;
	_isAnimated = false;
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
	return -1;
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
				return &subShotList[j];
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

SubShot* AnimatedCollage::GetClickedSubShot(int layoutIndex)
{	 
	char* filename = (*(_selectedFrames->frameList))[layoutIndex];
	int frameIndex = GetFrameIndex(filename);
	SubShot* chosenSubShot = GetChosenSubShot(frameIndex);
	return chosenSubShot;
}

void AnimatedCollage::DrawNextFrame(IplImage* collage)
{
	// update currentFrame
	if(_currentFrame < _subShot->end)
		_currentFrame++;
	else
		_currentFrame = _subShot->start;
	
	DrawImage(_currentLayoutFrame, LoadFrame(_sequence, _currentFrame), collage);
}

void AnimatedCollage::UpdateCollage(IplImage* collage, int x, int y)
{
	if(x != _current_x || y != _current_y)
	{
		// update mouse position
		_current_x = x;
		_current_y = y;
		// mouse changed
		int layoutIndex = GetClickedLayoutIndex(x, y);
		if(layoutIndex == -1)
		{
			_isAnimated = false;
		}
		else
		{
			if(layoutIndex != _layoutIndex)
			{
				printf("Layout Updating");
				// update layoutIndex
				_layoutIndex = layoutIndex;
				// update new subshot
				_subShot = GetClickedSubShot(layoutIndex);			 
				// reset currentFrame
				_currentFrame = _subShot->start;
				// update currentLayoutFrame
				_currentLayoutFrame = (*_layoutList)[layoutIndex];
				// turn on animation
				_isAnimated = true;
			}
			else
			{
				// same layout is clicked, stop animation
				_isAnimated = !_isAnimated;
			}
		}
	}
	if(_isAnimated)
	{
		// draw next frame to collage
		DrawNextFrame(collage);
	}
}

