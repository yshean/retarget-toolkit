#pragma once
#include "ROIBlend.h"

class Collage
{
public:
	// this class create a collage of list of picture
	Collage(void);
	~Collage(void);

public:
	ROIBlend* _roiBlend;
	virtual IplImage* CreateCollage(IplImage** pictureList, int length);
};
