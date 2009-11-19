#include "StdAfx.h"
#include "Volume3D.h"
 
Volume3D::~Volume3D(void)
{
}
 
void Volume3D::AssignFrame(IplImage* frame, int time)
{	
	*(_firstFrame + time) = (long)frame;
}

IplImage* Volume3D::GetFrame(int time)
{
	if(time < 0 || time >= _length)
	{
		printf("Input time must not exceed the length of the volume");
		exit(-1);
	}	
	
	IplImage* frame = (IplImage*)*(_firstFrame + time);
}

 
CvScalar Volume3D::Get3DPixel(int x, int y, int z)
{
	IplImage* frame = GetFrame(z);
	return cvGet2D(frame, x, y);
}