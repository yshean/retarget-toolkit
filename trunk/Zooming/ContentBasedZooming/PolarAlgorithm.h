#pragma once
#include "MinEnergyPath.h"

namespace Zooming
{
	class PolarAlgorithm :
		public MinEnergyPath
	{
	public:
		PolarAlgorithm(IplImage* image) : MinEnergyPath()
		{}
	public:
		~PolarAlgorithm(void);
	public:	
		virtual Path* GetEnergyPath();
		 
	};
}	
