#pragma once
#include <cv.h>

 struct Point3D
{
	int x; 
	int y;
	int z;
};

 Point3D point3D(int x, int y, int z);

 struct Volume3D
 {
	 int width;
	 int height;
	 int depth;
 };

 Volume3D volume3D(int width, int height, int depth);