#pragma once
#include <vector>
#include "LayoutFrame.h"

#include <cv.h>
#include <highgui.h>
#include "LayoutArrangement.h"

// create collage layout from list of frame
class CollageLayout
{
public:
	CollageLayout(LayoutArrangement* arranger);

	// create a collage of layout from list of layout
	LayoutFrame* CreateLayoutCollage(vector<LayoutFrame*>* layoutList);

	// create a rectangle layout - assuming that all frames are listed horizontally
	// set rectLayout.width for initial width
	void CreateRectLayout(vector<LayoutFrame*>* layoutList, LayoutFrame* rectLayout);
 

public:
	// get average size of the layout in list
	double GetAverageSize(vector<LayoutFrame*>* layoutList);
	// group layout with small size together
	vector<LayoutFrame*>* GetCombinedLayoutList(vector<LayoutFrame*>* layoutList);
protected:
	LayoutArrangement* _arranger;
};

void TestCollageLayout();