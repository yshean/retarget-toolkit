#pragma once
#include "VideoSequence.h"
#include "VideoCollage.h"
#include "ImageImportance.h"
#include "ImageQuality.h"
 
#include "SobelImageImportance.h"
#include "SobelImageQuality.h"
#include "Collage.h"

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

	// get list of frame selected as candidates for the collage
	VideoSequence* GetSelectedFrames(ShotInfo* shotInfo, VideoSequence* sequence );

	// get list of frame selected as candiates and then save to disk
	void SaveSelectedFrames(ShotInfo* shotInfo, VideoSequence* sequence, char* filename);

protected:
	ImageQuality* _imageQuality;
	ImageImportance* _imageImportance;
	ShotInfo* _shotInfo;

	
protected:
	virtual VCSolution* GetSolution(ShotInfo* shotInfo);
	
	// this solution is only to get a list of frame
	virtual VideoSequence* GetSolution2(ShotInfo* shotInfo);
	// couting the number of frame in solution 
	// by counting subshot, some subshot may return 2 frames
	int GetSolutionFrameCount(ShotInfo* shotInfo);

	// Get frame with max A(Ik) + Q(Ik) from frame #start to frame #end
	int GetMostRelevanceFrame(int start, int end);

	// select the keyframe of the static / zoom SubShot
	virtual void KeyFrameSelection1(SubShot* subShot, int* key);

	// select the keyframe of the pan / tilt SubShot
	virtual void KeyFrameSelection2(SubShot* subShot, int* key);


public:
	 
 
};
	// test tang video collage
	// @param: filename of shotinfo & filename of sequence
	void TestTangVideoCollage(char* sequencename, char* shotname);
 
	// test tang video collage - process the list and then save selected frame to disk
	// @param: file name of shotinf & filename of sequence
	// void TestTangSaveSelectedFrame(char* sequencename, char* shotname, char* filename);