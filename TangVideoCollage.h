#pragma once
#include "VideoSequence.h"
#include "VideoCollage.h"
#include "ImageImportance.h"
#include "ImageQuality.h"
<<<<<<< .mine
#include "SobelImageImportance.h"
#include "SobelImageQuality.h"
=======
#include "SobelImageQuality.h"
#include "SobelImageImportance.h"
#include "Collage.h"
>>>>>>> .r136
/************************ Tang Wang Video Collage Paper ***************************/

/******************************** VCSolution **************************************/

// video collage ROI
struct VCROI
{ 
	// indicates whether the associated frame is selected
	bool label;

	// denote the position of associated frame
	CvPoint position;	

	// size of the associated frame after resized according to salieny
	CvSize size;	
};

CV_INLINE VCROI vcROI(bool label, int x, int y, int width, int height)
{
	VCROI roi;
	roi.label = true;
	roi.position.x = x;
	roi.position.y = y;
	roi.size.width = width;
	roi.size.height = height;
	return roi;
}

// a solution to the video collage
// contain list of frame and correspond ROI	
struct VCSolution
{	
	IplImage** frameList;
	VCROI* roiList;
	int length;
};

public class TangVideoCollage : public VideoCollage
{
public:
	// remember to set:
	// - _sequence : input video
	// - _imageQuality : Quality Assessment algorithm
	// - _imageImportance: Saliency Assessment algorithm
	TangVideoCollage(ImageQuality* imageQuality, ImageImportance* imageImportance, VideoSequence* sequence, ShotInfo* shotInfo);
	~TangVideoCollage(void);

public:
	// interface to VideoCollage
	// Return an image represents the whole video sequence
	virtual IplImage* GetCollage();

protected:
	ImageQuality* _imageQuality;
	ImageImportance* _imageImportance;
	ShotInfo* _shotInfo;

	
protected:
	virtual VCSolution* GetSolution(ShotInfo* shotInfo);
	// couting the number of frame in solution 
	// by counting subshot, some subshot may return 2 frames
	int GetSolutionFrameCount(ShotInfo* shotInfo);

	// Get frame with max A(Ik) + Q(Ik) from frame #start to frame #end
	int GetMostRelevanceFrame(int start, int end);

	// select the keyframe of the static / zoom SubShot
	virtual void KeyFrameSelection1(SubShot* subShot, int* key);

	// select the keyframe of the pan / tilt SubShot
	virtual void KeyFrameSelection2(SubShot* subShot, int* key);
<<<<<<< .mine
};

// test tang video collage
// @param: filename of shotinfo & filename of sequence
void TestTangVideoCollage(char* sequencename, char* shotname);=======
};


// test tang video collage
// @param: filename of shotinfo & filename of sequence
void TestTangVideoCollage(char* sequencename, char* shotname);
>>>>>>> .r136
