#pragma once
#include <cv.h>
#include <highgui.h>

// unit testing
void TestRegionExtractor();


// extract a region of the image
class RegionExtractor
{
public:
	RegionExtractor(void);
	~RegionExtractor(void);

	// extract a region out of the image
	// if the overbound, the region will be cropped
	virtual IplImage* GetRegion(IplImage* image, CvPoint position, CvSize size);
};
