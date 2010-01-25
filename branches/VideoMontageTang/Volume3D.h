#pragma once
#include <cv.h>
#include <highgui.h>


struct Block3D
{
	int* data;
	int width;
	int height;
	int length;
};
 

// Volume3D of a video Interface
// Note that different frame size is allowed for the sake of flexibility
class Volume3D
{
public:
	// create an empty 3D volume, recommended for create a Volume
	Volume3D(int width, int height, int length)
	{
		// setting volume properties
		_width = width;
		_height = height;
		_length = length;
		
		// allocate pointers memory
		_firstFrame = (long*)malloc(length * sizeof(long));
	}

	// create an empty 3D volume, no width and height is required
	Volume3D(int length)
	{
		_length = length;
		// allocate pointers memory
		_firstFrame = (long*)malloc(length * sizeof(long));
	}
	
	// extract a frame in a specific time
	// return NULL if time is not in side the volume
	virtual IplImage* GetFrame(int time);

	// assign a frame to a specfic time
	// do not release the image ref as the volume only holds the reference to the image
	virtual void AssignFrame(IplImage* frame, int time); 

	// get pixel from a specific 3D location
	// note that do not grab a pixel which is out of volume bound
	// for efficiency purpose
	virtual CvScalar Get3DPixel(int x, int y, int z);

public:
	// access to the width of the volume
	int _width;
	// access to the height of the volume
	int _height;
	// access to the length of the volume
	int _length;
	
	long* _firstFrame; // pointer to first frame pointer
 	
	~Volume3D(void);
 
	// add a frame
	// void AppendFrame(IplImage* frame);

	// add a frame with offset
	// void AppendFrame(IplImage* frame, int x_offset, int y_offset);

	// assign a frame to a specific time with offset
	// void AssignFrame(IplImage* frame, int x_offset, int y_offset);
};
