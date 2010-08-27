#include "StdAfx.h"
#include "ShiftMapVisualizer.h"

ShiftMapVisualizer::ShiftMapVisualizer(void)
{
}

ShiftMapVisualizer::~ShiftMapVisualizer(void)
{
}

void  ShiftMapVisualizer::Visualize(CvMat *labelMap, IplImage* source, IplImage* map, CvSize shiftSize)
{
	for(int i = 0; i < source->width; i++)
		for(int j = 0; j < source->height; j++)
		{
			CvScalar value;
			value.val[0] = i * 255 / source->width;
			value.val[1] = j * 255 / source->height;
			value.val[2] = 0;
			cvSet2D(source, j, i, value);
		}

	for(int i = 0; i < map->width; i++)
		for(int j = 0; j < map->height; j++)
		{
			CvPoint shift = GetLabel(cvPoint(i,j), labelMap);			
			CvPoint mappedPoint = cvPoint(shift.x + i, shift.y + j);
			CvScalar value;
			value.val[0] = mappedPoint.x * 255 / source->width;
			value.val[1] = mappedPoint.y * 255 / source->height;
			value.val[2] = 0;
			cvSet2D(map, j, i, value);
		}

}

int ShiftMapVisualizer::ComputeEnergy(CvMat* labelMap)
{
	int energy = 0;
	for(int i = 0; i < labelMap->width; i++)
		for(int j = 0; j < labelMap->height; j++)
		{
			CvPoint point = GetLabel(cvPoint(i,j), labelMap);
			CvPoint neighbor;
			if(i > 0)
			{
				neighbor = GetLabel(cvPoint(i-1,j), labelMap);
				if(point.x - neighbor.x != 1)
					energy += 100;
			}
			if(j > 0)
			{
				neighbor = GetLabel(cvPoint(i,j-1), labelMap);
				if(point.y - neighbor.y != 1)
					energy += 100;
			}
			if(i < labelMap->width-1)
			{
				neighbor = GetLabel(cvPoint(i+1,j), labelMap);
				if(point.x - neighbor.x != -1)
					energy += 100;
			}
			if(j < labelMap->height-1)
			{
				neighbor = GetLabel(cvPoint(i,j+1), labelMap);
				if(point.y - neighbor.y != -1)
					energy += 100;
			}

		}
	return energy;
}