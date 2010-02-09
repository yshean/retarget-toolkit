#include "StdAfx.h"
#include "CollageLayout.h"

CollageLayout::CollageLayout(void)
{
}

LayoutFrame* CollageLayout::CreateLayoutCollage(std::vector<LayoutFrame*> *layoutList)
{	 
	vector<LayoutFrame*>* combinedList = GetCombinedLayoutList(layoutList);


	return new LayoutFrame();
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
			if(frame1->size.width < frame1->size.height)
			{
				LayoutFrame* frame = Align2FramesVertical(frame1, frame2);
				combinedList->push_back (frame);
			}
			else
			{
				LayoutFrame* frame = Align2FramesVertical(frame1, frame2);
				combinedList->push_back (frame);
				//LayoutFrame* frame = Align2FramesHorizontal(frame1, frame2);
				//combinedList->push_back (frame);
			}
			i += 2;
		}
		else
		{
			combinedList->push_back(frame1);
			i++;
		}
	}
	return combinedList;
}


void TestCollageLayout()
{
	vector<LayoutFrame*>* frameList = new vector<LayoutFrame*>();
	for(int i = 0; i < 10; i++)
	{
		LayoutFrame* frame = CreateLayoutFrame(0, 0, 200, 200);
		frame->size.height = rand() % 80 + 1;
		frame->size.width = rand() % 80 + 1;
		frameList->push_back(frame);
	}

	CollageLayout* collageLayout = new CollageLayout();
	vector<LayoutFrame*>* combinedList = collageLayout->GetCombinedLayoutList(frameList);

	// put all frames into 1 big frame to see the test result
	int size = combinedList->size();
	LayoutFrame* frame = Align2FramesHorizontal((*combinedList)[0], (*combinedList)[1]);
	for(int i = 2; i < size; i++)
	{
		frame = Align2FramesHorizontal(frame, (*combinedList)[i]);
	}
	 
	IplImage* image = cvCreateImage(cvSize(frame->size.width, frame->size.height), IPL_DEPTH_8U, 3);
	
	
	cvFillImage(image, 0);
	LayoutFrame* test = CreateLayoutFrame(0, 0, 200, 50);
	LayoutFrame* test2 = CreateLayoutFrame(0, 0, 140, 50);
	LayoutFrame* test3 = Align2FramesVertical(test, test2);
	LayoutFrame* test4 = CreateLayoutFrame(0, 0, 220, 50);
	LayoutFrame* test5 = Align2FramesHorizontal(test3, test4);
	DrawFrame(frame, image, 0, 0);
	
	cvNamedWindow("Test");
	while(1)
	{
		cvShowImage("Test", image);
		cvWaitKey(100);
	}
}