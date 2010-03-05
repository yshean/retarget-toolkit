#pragma once
#include <cv.h>
#include <vector>
using namespace std;

struct dPoint
{
	double x;
	double y;
};

struct dSize
{
	double width;
	double height;
};

struct LayoutFrame
{
	// in case of inner frames, this will be the relative position ot top-left corner of outside frame
	dPoint position; 
	dSize size;
	vector<LayoutFrame*>* innerFrames;
	LayoutFrame* outerFrame;
};

LayoutFrame* CreateLayoutFrame(int x, int y, int width, int height);

// add a frame to the current frame
// added frame will be resize to the height of frame
void AddToFrameHorizontal(LayoutFrame* frame, LayoutFrame* addedFrame);

// add a frame to the current frame
// added frame will be resize to the width of the frame
void AddToFrameVertical(LayoutFrame* frame, LayoutFrame* addedFrame);

// move layout (inner frames are also moved)
void MoveLayout(LayoutFrame* frame, int x, int y);

// resize the frame (inner frames are also resized)
void ResizeLayout(LayoutFrame* frame, double ratio, bool is_inside);

// resize to a specific width (inner frames are also resized)
void ResizeToWidth(LayoutFrame* frame, int width);

// resize to a specific height (inner frames are also resized)
void ResizeToHeight(LayoutFrame* frame, int height);

// general method to put frame inside outerFrame,
// position & size of frame will determine whether outerFrame 
// should be resized. Overlap inside frames is allowed.
void PutLayoutInside(LayoutFrame* outerFrame, LayoutFrame* frame);

// place frame2 next to frame1 frame1.x + frame1.width = frame2.x && frame2.y = frame1.y
// NOTE no resize is done
void PlaceNextTo(LayoutFrame* frame1, LayoutFrame* frame2);

// place frame2 below frame1, frame1.y + frame1.height = frame.y && frame2.x = frame1.x
// NOTE no resize is done
void PlaceBelow(LayoutFrame* frame1, LayoutFrame* frame2);

// stack 2 frames above each other into one
LayoutFrame* Align2FramesVertical(LayoutFrame* frame1, LayoutFrame* frame2);

// attach 2 frames side by side into one
LayoutFrame* Align2FramesHorizontal(LayoutFrame* frame1, LayoutFrame* frame2);

// get area = width * height
double GetArea(LayoutFrame* frame);

// get real layout position in canvas
CvPoint GetLayoutCanvasPosition(LayoutFrame* frame);

// **** DRAWING functions *************

// draw frame and inner frames into image, x and y are outer frame position
void DrawFrame(LayoutFrame* frame, IplImage* image, int x, int y);

// draw an image to an layout to the canvas
void DrawImage(LayoutFrame* frame, IplImage* image, IplImage* canvas);

// draw an image to canvas
void DrawImage(IplImage* image, IplImage* canvas, CvPoint position);