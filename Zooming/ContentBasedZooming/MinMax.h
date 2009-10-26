#pragma once

namespace CVMath
{
	// minPosition denotes which value is the min : 1 or 2 or 3
	// favor middle item in case of equality
	template <class T>
	T GetMinOfThree(T v1, T v2, T v3, int* minPosition)
	{
		T min = v3;
		*minPosition = 3;

		if (v1 <= min)
		{
			min = v1;
			*minPosition = 1;
		}			
		if (v2 <= min)
		{
			min = v2;
			*minPosition = 2;
		} 
		return min;
	}

	template <class T>
	T GetMaxOfThree(T v1, T v2, T v3, int* maxPosition)
	{
		T max = v3;
		*maxPosition = 3;

		if (v1 >= max)
		{
			max = v1;
			*maxPosition = 1;
		}			
		if (v2 >= max)
		{
			max = v2;
			*maxPosition = 2;
		} 
		return max;
	}
			
	template <class T>
	T GetMinOfTwo(T v1, T v2, int* minPosition)
	{
		T min = v1;
		*minPosition = 1;

		if (v2 <= min)
		{
			min = v2;
			*minPosition = 2;
		}
		return min;
	}
}

