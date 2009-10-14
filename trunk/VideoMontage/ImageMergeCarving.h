#pragma once
#include <cv.h>
#include "EnergyPath.h"
#include "RegionExtractor.h"

using namespace Zooming;

// unit testing
void TestImageMergeCarving();


public class ImageMergeCarving
{
public:
	ImageMergeCarving(void);
	~ImageMergeCarving(void);

	RegionExtractor* extractor;

	// assuming that 2 images having the same height
	// slide 2 images over to see with which seam the 2 can be combined
	virtual IplImage* GetMergedImage(IplImage* image1, IplImage* image2, int overlap_size);
};


