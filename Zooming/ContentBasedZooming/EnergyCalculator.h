#pragma once
#include <cv.h>


namespace Zooming
{
	public class EnergyCalculator
	{
	public:
		virtual IplImage* GetEnergyImage(IplImage* input) = 0;
	};

	public class SobelEnergy : public EnergyCalculator
	{
	public:
		SobelEnergy(int xOrder, int yOrder)
		{
			_xOrder = xOrder;
			_yOrder = yOrder;
		}
		virtual IplImage* GetEnergyImage(IplImage* input);

	protected:
		int _xOrder;
		int _yOrder;

		
	};
}