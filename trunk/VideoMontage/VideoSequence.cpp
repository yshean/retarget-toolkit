#include "stdafx.h"
#include "VideoSequence.h"

// Testing
void TestCreateSequence()
{
	VideoSequence sequence;
	vector<char*> frameList(50);
	
	frameList[1] = "Ha ha ha ha";
	frameList[5] = "Ha ha ha ha 2";
	 
	sequence.frameList = frameList;
}

// Load a frame from memory
IplImage* LoadFrame(VideoSequence* videoSequence, int frame_number)
{
	char* filename = videoSequence->frameList[frame_number];
	return cvLoadImage(filename);
}
