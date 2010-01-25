#include "StdAfx.h"
#include "MontageSaliencyMap.h"

MontageSaliencyMap::MontageSaliencyMap(MontageSaliencySetting* setting)
{
	//_neighbor = 6;
	_setting = setting;
}

MontageSaliencyMap::~MontageSaliencyMap(void)
{
}

void MontageSaliencyMap::CalculateSaliencyMap(Volume3D *volume, Volume3D *output)
{
	// first convert the volume to LUV?

	switch(_neighbor)
	{
	// calculate based on 6 neighbor
	case NEIGHBOR_6:
		Distance6Neighbor(volume, output);
		break;
	default:
		break;
	}
}

void MontageSaliencyMap::Distance6Neighbor(Volume3D* volume, Volume3D* output)
{
	int length = volume->_length;
	int width = volume->_width;
	int height = volume->_height;

	for(int t = 0; t < length; t++)
	{
		// for each frame
		for(int x = 0; x < width; x++)
			for(int y = 0; y < height; y++)
			{
				// for each pixel of the frame
				// get its neighbor
				
			}
	}
}

void MontageSaliencyMap::CalculateSaliencyMap(Matrix3D* matrix, Matrix3D* output)
{	
	int width = matrix->_width;
	int height = matrix->_height;
	int length = matrix->_length;

	for(int z = 0; z < length; z++)		
	{	
		printf("Processing frame %i \n", z);
		for(int x = 0; x < width; x++)
			for(int y = 0; y < height; y++)
			{
				CvScalar center = matrix->Get3DScalar(x, y, z);
				double sum_diff = 0;

				int block_size = 2;
				// calculate difference within the block
				for(int i = x - block_size; i <= x + block_size; i++)
					for(int j = y - block_size; j <= y + block_size; j++)
						for(int k = z - block_size; k <= z + block_size; k++)
						{
							CvScalar value = matrix->Get3DScalar(i, j, k);
							double diff = _setting->norm->CalculateNorm(value, center);
							sum_diff += diff;
						}						
				output->Set3D(x, y, z, sum_diff);				
			}	
	}
}