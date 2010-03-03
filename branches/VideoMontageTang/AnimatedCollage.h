#pragma once
#include <cv.h>
#include "VideoSequence.h"
#include "LayoutFrame.h"

class AnimatedCollage
{
public:
	// refactor the animation part to new class
	AnimatedCollage(ShotInfo* ShotInfo, VideoSequence* sequence, 
		VideoSequence* selectedFrame, vector<LayoutFrame*>* layoutList);
	~AnimatedCollage(void);

public:
// private member set public, advanced user can change it, otherwise don't touch it
	ShotInfo* _shotInfo; // shot/ subshot info
	VideoSequence* _sequence; // original sequence
	VideoSequence* _selectedFrame; // list of selected frame
	vector<LayoutFrame*>* _layoutList; // layoutFrame for each selected frame, means frame & layout must match

// mouse event related variables
protected:
	// current mouse position
	// if updated, subshot will be find again
	int _current_x;
	int _current_y;

	// current subShot, if changed, the sequence will be reset
	SubShot* _subShot;

	// current frame index to update, should be increased everytime updated
	int _currentFrame;

	// current LayoutFrame, for the shake of easy updating
	LayoutFrame* _currentLayoutFrame;
	
	//void UpdateCollage(VideoSequence* sequence,
	//	ShotInfo* shotInfo, 
	//	vector<LayoutFrame*>* layoutList, 
	//	SubShot* subShot, int selectedLayout,
	//	IplImage* collage,
	//	int* currentFrameIndex);
protected:
// operating function
	//void UpdateCollage(VideoSequence* sequence,
	//	ShotInfo* shotInfo, 
	//	vector<LayoutFrame*>* layoutList, 
	//	SubShot* subShot, int selectedLayout,
	//	IplImage* collage,
	//	int* currentFrameIndex);
 
	// get clicked subshot given mouse position and list of layout list
	SubShot* GetClickedSubShot(int x, int y);

	// detect layout selected in a list of layout
	// this index is match with selected frames, too
	int GetClickedLayoutIndex(int x, int y);

	// get chosen subshot given the index of selected frame
	SubShot* GetChosenSubShot(int selectedFrame);

	// return -1 if name of file not found in sequence
	int GetFrameIndex(char* fileName);
};
