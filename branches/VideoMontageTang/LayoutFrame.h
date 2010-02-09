#pragma once
#include <cv.h>
#include <vector>
using namespace std;

struct LayoutFrame
{
	// in case of inner frames, this will be the relative position ot top-left corner of outside frame
	CvPoint position; 
	CvSize size;
	vector<LayoutFrame*>* innerFrames;
};

LayoutFrame* CreateLayoutFrame(int x, int y, int width, int height);

// move layout (inner frames are also moved)
void MoveLayout(LayoutFrame* frame, int x, int y);

// resize the frame (inner frames are also resized)
void ResizeLayout(LayoutFrame* frame, double ratio);

// resize to a specific width (inner frames are also resized)
void ResizeToWidth(LayoutFrame* frame, int width);

// resize to a specific height (inner frames are also resized)
void ResizeToHeight(LayoutFrame* frame, int height);

// stack 2 frames above each other into one
LayoutFrame* Align2FramesVertical(LayoutFrame* frame1, LayoutFrame* frame2);

// attach 2 frames side by side into one
LayoutFrame* Align2FramesHorizontal(LayoutFrame* frame1, LayoutFrame* frame2);

// get area = width * height
double GetArea(LayoutFrame* frame);

// **** DRAWING functions *************

// draw frame and inner frames into image, x and y are outer frame position
void DrawFrame(LayoutFrame* frame, IplImage* image, int x, int y);