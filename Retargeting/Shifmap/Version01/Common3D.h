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
 
 // Get the neighbor of label which have the some neighborhood properties 
// with pixel1 and pixel2
// or label -> result ~ pixel1 -> pixel2
Point3D GetNeighbor(Point3D pixel1, Point3D pixel2, Point3D label);