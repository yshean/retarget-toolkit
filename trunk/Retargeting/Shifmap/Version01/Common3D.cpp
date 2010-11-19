#include "StdAfx.h"
#include "Common3D.h"
 
Point3D point3D(int x, int y, int z)
{
	Point3D point;
	point.x = x;
	point.y = y;
	point.z = z;
	return point;
}

Volume3D volume3D(int width, int height, int depth)
{
	Volume3D volume;
	volume.width = width;
	volume.height = height;
	volume.depth = depth;
	return volume;
}

Point3D GetNeighbor(Point3D pixel1, Point3D pixel2, Point3D label)
{
	return point3D(label.x - pixel1.x + pixel2.x, label.y - pixel1.y + pixel2.y, label.z);
}