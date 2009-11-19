#pragma once
#include <cv.h>
#include "EnergyPath.h"
#include "RegionExtractor.h"
#include "DrawingFunctions.h"

using namespace Zooming;

// unit testing
void TestImageMergeCarving();


public class ImageMergeCarving
{
public:
	ImageMergeCarving(void);
	~ImageMergeCarving(void);

	RegionExtractor* extractor;
	// MinEnergyPath* minEnergyPath;

	// assuming that 2 images having the same height
	// slide 2 images over to see with which seam the 2 can be combined
	virtual IplImage* GetMergedImage(IplImage* image1, IplImage* image2, int overlap_size);

public:
	// PRIVATE: get overlap area between 2 images
	// the overlap area of right image is shifted 1 pixel
	// so they are align one pixel shifted with each other
	// so 2 pixel which is neighbor (horizontally) will have the 
	// same coordinate in each overlap images.
	// ** NOTE ** 
	// - overlap1 and overlap2 should be null
	virtual void GetOverlapImages(IplImage* image1, IplImage* image2, 
		IplImage** overlap1, IplImage** overlap2, int overlap_size);

	// create the overlap cost image by comparing every pixel in same location of overlap1 and overlap2
	virtual IplImage* CreateOverlapImageCost(IplImage* overlap1, IplImage* overlap2);
};


