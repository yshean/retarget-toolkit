#pragma once
#include <cv.h>
#include <math.h>
#include "MinEnergyPath.h"

namespace Zooming
{
	#define PI 3.14159265

	struct Point2D
	{
		int x;
		int y;
	};

	struct PointDouble2D
	{
		double x;
		double y;
	};
 
	PointDouble2D pointDouble2D(double x, double y);
 
	Point2D point2D(int x, int y);

	public class PolarTransform
	{
	protected:
		int width;
		int height;
		double unitAngle;		
	public:
		// unitAngle in degree
		// resolution: number of pixel taken in 1 radius
		PolarTransform(double unitAngle, int resolution);

	public:
		// create a polar transformation 
		// the folding with start from the right hand size, then anti-clockwise
		IplImage* CreatePolarImage(IplImage* image, Point2D center, int  outerRadius, int innerRadius);

		// transform a list of point back to the image
		Path* GetOriginalPath(Point2D center, int outerRadius, int innerRadius, Path* path);
	private:
		// angle is in degree
		Point2D GetCartersianCoordination(double angle, double radius)
		{
			int x = (int) radius * sin(angle * PI / 180);
			int y = (int) radius * cos(angle * PI / 180);
			return point2D(x, y);
		}

		// angle is in degrees
		PointDouble2D GetPolarCoordination(int x, int y)
		{
			double radius = sqrt((double)(x * x + y * y));
			double theta = atan2((double)x, (double)y) * PI / 180;
			return pointDouble2D(radius, theta);
		}
	};
}
