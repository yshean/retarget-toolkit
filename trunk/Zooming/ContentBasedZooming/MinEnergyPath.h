#pragma once
#include <vector>
#include <cv.h>

namespace Zooming
{
	struct Pixel
	{
		int x;
		int y;
	};	

	struct Path
	{
		Pixel* pixelList;
		int size;
	};

	
	void ReleasePath(Path* path);

	public class MinEnergyPath
	{	
	public:
		virtual Path* GetEnergyPath() = 0;
	};


}