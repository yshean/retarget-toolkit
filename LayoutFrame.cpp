#include "StdAfx.h"
#include "LayoutFrame.h"


LayoutFrame* CreateLayoutFrame(int x, int y, int width, int height)
{
	LayoutFrame* frame = new LayoutFrame();
	frame->position.x = x;
	frame->position.y = y;
	frame->size.width = width;
	frame->size.height = height;
	frame->innerFrames = new vector<LayoutFrame*>(0); // empty inner frames 
	return frame;
}

 // resize the frame (inner frames are also resized)
void ResizeLayout(LayoutFrame* frame, double ratio)
{
	frame->size.width *= ratio;
	frame->size.height *= ratio;
	int size = frame->innerFrames->size();

	for(int i = 0; i < size; i++)
	{	
		ResizeLayout( (*(frame->innerFrames))[i], ratio);
	}
}

// resize to a specific width (inner frames are also resized)
void ResizeToWidth(LayoutFrame* frame, int width)
{
	double ratio = (double) width / (double)frame->size.width;
	ResizeLayout(frame, ratio);
}

// resize to a specific height (inner frames are also resized)
void ResizeToHeight(LayoutFrame* frame, int height)
{
	double ratio = (double) height / (double)frame->size.height;
	ResizeLayout(frame, ratio);
}


// stack 2 frames above each other into one
// frame1 will be above frame2
// position will be reset to 0
LayoutFrame* Align2FramesVertical(LayoutFrame* frame1, LayoutFrame* frame2)
{
	LayoutFrame* frame = CreateLayoutFrame(0, 0, 0, 0);
	// resize frame2 so that the width is the same with frame1
	ResizeToWidth(frame2, frame1->size.width);
	// set position of frame1 to (0,0)
	frame1->position.x = 0;
	frame1->position.y = 0;
	// set position of frame2 to (0, frame1->height)
	frame2->position.x = 0;
	frame2->position.y = frame1->size.height;
	// add 2 frames to outer frame
	frame->innerFrames->push_back(frame1);
	frame->innerFrames->push_back(frame2);
	// set width & height
	frame->size.width = frame1->size.width;
	frame->size.height = frame1->size.height + frame2->size.height;
	return frame;
}

// attach 2 frames side by side into one
LayoutFrame* Align2FramesHorizontal(LayoutFrame* frame1, LayoutFrame* frame2)
{
	LayoutFrame* frame = CreateLayoutFrame(0, 0, 0, 0);
	// resize frame2 so that the width is the same with frame1
	ResizeToHeight(frame2, frame1->size.height);
	// set position of frame1 to (0,0)
	frame1->position.x = 0;
	frame1->position.y = 0;
	// set position of frame2 to (frame1->width, 0)
	frame2->position.x = frame1->size.width;
	frame2->position.y = 0;
	// add 2 frames to outer frame
	frame->innerFrames->push_back(frame1);
	frame->innerFrames->push_back(frame2);
	// set width & height
	frame->size.width = frame1->size.width + frame2->size.width;
	frame->size.height = frame1->size.height;
	return frame;
}

// move layout (inner frames are also moved)
void MoveLayout(LayoutFrame* frame, int x, int y)
{
	int size = frame->innerFrames->size();
	for(int i = 0; i < size; i++)
	{
		(*(frame->innerFrames))[i]->position.x += x - frame->position.x;
		(*(frame->innerFrames))[i]->position.y += y - frame->position.y;
	}
	frame->position.x = x;
	frame->position.y = y;
}

// get area
double GetArea(LayoutFrame* frame)
{
	return frame->size.width * frame->size.height;
}

void DrawFrame(LayoutFrame* frame, IplImage* image, int x, int y)
{
	cvDrawRect(image, 
		cvPoint(x + frame->position.x, y + frame->position.y), 
		cvPoint(x + frame->position.x + frame->size.width, y + frame->position.y + frame->size.height), 
		cvScalar(255,0,0));

	int size = frame->innerFrames->size();
	for(int i = 0; i < size; i++)
	{
		DrawFrame((*(frame->innerFrames))[i], image, frame->position.x, frame->position.y);
	}
}