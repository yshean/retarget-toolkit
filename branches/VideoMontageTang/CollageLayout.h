#pragma once
#include <vector>
#include "LayoutFrame.h"

#include <cv.h>
#include <highgui.h>

// create collage layout from list of frame
class CollageLayout
{
public:
	CollageLayout(void);

	// create a collage of layout from list of layout
	LayoutFrame* CreateLayoutCollage(vector<LayoutFrame*>* layoutList);

	// create a rectangle layout - assuming that all frames are listed horizontally
	LayoutFrame* CreateRectLayout(vector<LayoutFrame*>* layoutList, CvRect size);
 

public:
	// get average size of the layout in list
	double GetAverageSize(vector<LayoutFrame*>* layoutList);
	// group layout with small size together
	vector<LayoutFrame*>* GetCombinedLayoutList(vector<LayoutFrame*>* layoutList);

};

void TestCollageLayout();