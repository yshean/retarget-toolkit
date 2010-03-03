#pragma once
#include <cv.h>
#include "VideoSequence.h"
#include "LayoutFrame.h"

class AnimatedCollage
{
public:
	// refactor the animation part to new class
	AnimatedCollage(ShotInfo* shotInfo, VideoSequence* sequence, 
		VideoSequence* selectedFrames, vector<LayoutFrame*>* layoutList);
	~AnimatedCollage(void);

public:
// private member set public, advanced user can change it, otherwise don't touch it
	ShotInfo* _shotInfo; // shot/ subshot info
	VideoSequence* _sequence; // original sequence
	VideoSequence* _selectedFrames; // list of selected frame
	vector<LayoutFrame*>* _layoutList; // layoutFrame for each selected frame, means frame & layout must match

// mouse event related variables
protected:
	// current mouse position
	// if updated, subshot will be find again
	int _current_x;
	int _current_y;

	// current LayoutIndex, if changed, sequence will be reset
	int _layoutIndex;

	// current subShot, for the sake of easy updating
	SubShot* _subShot;

	// current frame index to update, should be increased everytime updated
	int _currentFrame;

	// this flag is to stop/start animation
	bool _isAnimated;

	// current LayoutFrame, for the sake of easy updating
	LayoutFrame* _currentLayoutFrame;
public:
	// update collage
	// this method will change the collage if necessary (i.e x and y change
	void UpdateCollage(IplImage* collage, int x, int y);

protected:
	// *************** operating function  *************
 
	// get clicked subshot given clicked layoutindex
	// allow faster process for updating collage
	SubShot* GetClickedSubShot(int layoutIndex);

	// detect layout selected in a list of layout
	// this index is match with selected frames, too
	int GetClickedLayoutIndex(int x, int y);

	// get chosen subshot given the index of selected frame
	SubShot* GetChosenSubShot(int selectedFrame);

	// return -1 if name of file not found in sequence
	int GetFrameIndex(char* fileName);

	// draw next frame to the collage, using internal information
	// very private function, only use after subshot, current frame etc are confirmed
	void DrawNextFrame(IplImage* collage);
};
