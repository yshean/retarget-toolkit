#pragma once
#include <cv.h>
#include <highgui.h>
#include <vector>
#include <list>


using namespace std;

/******************************** Video Sequence definition ********************************/

// video sequence is just a list of file name
// encapsulate it with a struct just to make it more meaningful
struct VideoSequence
{
	// list of char to the image file name
	vector<char*>* frameList;
};

// Load a frame from memory
// Throw Out of Bound Exception
IplImage* LoadFrame(VideoSequence* videoSequence, int frame_number);

// 

void TestCreateSequence();
/******************************** SubShot/Shot definition **************************************/

#include <iostream> //iostream 
#include <fstream> //fstream 


#define VCSHOT_STATIC 0
#define VCSHOT_TILT 1
#define VCSHOT_PAN 2
#define VCSHOT_ZOOM 3
#define VCSHOT_UNDEFINED 4

struct SubShot
{
	// type of sub shot, see:
	//#define VCSHOT_STATIC 0
	//#define VCSHOT_TILT 1
	//#define VCSHOT_PAN 2
	//#define VCSHOT_ZOOM 3
	int shotType;
	
	// beginning frame index
	int start;
	// last frame index
	int end;
};

struct Shot
{
	// in case of undefined subshot, the list may contain only 1 element
	// which is the subshot with type "VCSHOT_UNDEFINED"
	SubShot* subShotList;

	int subShotCount;
};

// the structure which is associated with a video sequence
struct ShotInfo
{
	// list of shot
	Shot* shotList;

	// number of shot count
	int shotCount;
};


// ************* IO *****************
// LOAD SEQUENCE FROM FILE
void TestLoadSaveSequence();
// load list of frame filename from file
VideoSequence* LoadSequenceFromFile(char* filename);

// save list of frame filename to file
void SaveVideoSequenceToFile(VideoSequence* sequence, char* fileName);


// LOAD SHOT INFO FROM FILE

// testing unit
// Create a ShotInfo then save/load it to file
void TestSaveLoadShot();

ShotInfo* LoadShotFromFile(char* fileName);

void SaveShotToFile(ShotInfo* shotInfo, char* fileName);




