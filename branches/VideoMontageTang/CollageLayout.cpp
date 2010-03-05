#include "StdAfx.h"
#include "CollageLayout.h"

CollageLayout::CollageLayout(LayoutArrangement* arranger)
{
	_arranger = arranger;
}

LayoutFrame* CollageLayout::CreateLayoutCollage(std::vector<LayoutFrame*> *layoutList)
{	 
	vector<LayoutFrame*>* combinedList = GetCombinedLayoutList(layoutList);
	
	return new LayoutFrame();
}


void CollageLayout::CreateRectLayout(vector<LayoutFrame*>* layoutList, LayoutFrame* rectLayout)
{
	int length = layoutList->size();	
	int current_width = 0;

	// LayoutFrame* rectLayout = CreateLayoutFrame(0, 0, size.width, 0);
	LayoutFrame* row = CreateLayoutFrame(0, 0, 0, 200);
	
	for(int i = 0; i < length; i++)
	{
		LayoutFrame* frame = (*layoutList)[i];
		current_width += frame->size.width;
		
		if(current_width > rectLayout->size.width)
		{	
			// exceed, create new row
			current_width = frame->size.width;
			ResizeToWidth(row, rectLayout->size.width);
			AddToFrameVertical(rectLayout, row);
			row = CreateLayoutFrame(0, 0, 0, 200);
		}		
		AddToFrameHorizontal(row, frame);
	}
	AddToFrameVertical(rectLayout, row); 
}


double CollageLayout::GetAverageSize(std::vector<LayoutFrame*> *layoutList)
{
	int size = layoutList->size();
	double avg_size;
	for(int i = 0; i < size; i++)
	{
		avg_size += (*layoutList)[i]->size.width * (*layoutList)[i]->size.height;
	}
	avg_size = avg_size / size;
	return avg_size;
}

vector<LayoutFrame*>* CollageLayout::GetCombinedLayoutList(vector<LayoutFrame*>* layoutList)
{
	int avg_size = GetAverageSize(layoutList);
	vector<LayoutFrame*>* combinedList = new vector<LayoutFrame*>(0);

	int length = layoutList->size();
	int i = 0;
	while( i < length - 1)
	{
		LayoutFrame* frame1 = (*layoutList)[i];
		LayoutFrame* frame2 = (*layoutList)[i + 1];
		double size1 = GetArea(frame1);
		double size2 = GetArea(frame2);

		if(size1 < avg_size && size2 < avg_size)
		{
			LayoutFrame* frame = Align2FramesVertical(frame1, frame2);
			combinedList->push_back (frame);			 
			i += 2;
		}
		else
		{
			combinedList->push_back(frame1);
			i++;
		}
	}

	if(i < length)
	{
		// push last frame
		LayoutFrame* frame = (*layoutList)[i];
		combinedList->push_back(frame);
	}
	return combinedList;
}


void TestCollageLayout()
{
	vector<LayoutFrame*>* frameList = new vector<LayoutFrame*>();
	for(int i = 0; i < 20; i++)
	{
		LayoutFrame* frame = CreateLayoutFrame(0, 0, 200, 200);
		frame->size.height = rand() % 80 + 1;
		frame->size.width = rand() % 80 + 1;
		//frame->size.height = 30;
		//frame->size.width = 30;
		frameList->push_back(frame);
	}
 
	LayoutArrangement* arranger = new LayoutResizeArrangement();
	CollageLayout* collageLayout = new CollageLayout(arranger);
	vector<LayoutFrame*>* combinedList = collageLayout->GetCombinedLayoutList(frameList);

	// put all frames into 1 big frame to see the test result
	//int size = combinedList->size();
	//LayoutFrame* frame = Align2FramesHorizontal((*combinedList)[0], (*combinedList)[1]);
	//for(int i = 2; i < size; i++)
	//{
	//	frame = Align2FramesHorizontal(frame, (*combinedList)[i]);
	//}
	 LayoutFrame* frame = CreateLayoutFrame(0, 0, 300, 0);
	collageLayout->CreateRectLayout(combinedList, frame);

	IplImage* image = cvCreateImage(cvSize(frame->size.width + 2, frame->size.height + 2), IPL_DEPTH_8U, 3);	
	cvFillImage(image, 0); 
	LayoutFrame* innerFrame = (*(frame->innerFrames))[1];
 
	DrawFrame(frame, image, 0, 0);	 
	cvNamedWindow("Test");
	while(1)
	{
		cvShowImage("Test", image);
		cvWaitKey(100);
	}
}