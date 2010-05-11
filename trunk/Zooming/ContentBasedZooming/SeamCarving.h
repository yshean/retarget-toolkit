#pragma once
#include "MinEnergyPath.h"
#include "MinMax.h"
#include "OpenCVEx.h"

using namespace CVMath;
using namespace OpenCVEx;

namespace Zooming
{	
	public class SeamCarving : public MinEnergyPath
	{
	public:
		SeamCarving(IplImage* energy)
		{
			this->energy = energy;
		}
		~SeamCarving();
	public:
		virtual Path* GetEnergyPath();
	private:
		// this is the input to the algorithm
		// the energy of any image can be passed so the energy calculation can be de-coupled
		IplImage* energy;

		// create a energy map whether energy of each pixel denotes the min energy to reach to that pixel
		// at the same time create a path info in order to retrieve the min path, instead of checking min pixel bottom up
		// path info image only contains -1 0 1 denotes the path should go left, up or right
		void PreprocessEnergyMap(IplImage* energy, IplImage* energyMap, IplImage* pathInfo);
		
		// Generate path given a position to start following the path
		Path* GeneratePath(int position, IplImage* pathInfo);

		// Get the min position of the energy path by searching the last row
		int GetMinPosition(IplImage* energyMap);
	};

}
