#pragma once
//******************************************************************************
//This is the interface for Video Collage.
//Implementation is based on paper: "Video Collage: A novel Presentation of Video Sequence"
//by Tang Wang, Tao Mei, Xian-Sheng Hua, Xue-Liang Liu & He-Qin Zhou
//Implemented to OpenCV style
//******************************************************************************

#include <list>
#include <vector>
#include "VideoSequence.h"
using namespace std;

public class ROIExtraction
{
public:
	ROIExtraction(void){}
	~ROIExtraction(void){}

	// Return the ROI based on saliency map
	virtual CvRect GetROI(IplImage* saliency) = 0;

	// Get part of image which is associated with the ROI
	virtual IplImage* GetROIRegion(IplImage* image, CvRect ROI) = 0;
};

public class VideoCollage
{
public:
	VideoCollage(VideoSequence* sequence)
	{		
		_sequence = sequence;
	}
	~VideoCollage(void){}
protected:
	VideoSequence* _sequence;
public:
	// VideoCollage interface
	// Return an image represents the whole video sequence
	virtual IplImage* GetCollage() = 0;
protected:

	//virtual double VCSaliency(IplImage* frame, VCROI* roi) = 0;

	//virtual double VCQuality(IplImage* frame, VCROI* roi) = 0;

	//virtual double VCDistribution(VCSolution* solution) = 0;


};




