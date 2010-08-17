#include "StdAfx.h"
#include "ClusterShiftMap.h"

ClusterShiftMap::ClusterShiftMap(void)
{
}

ClusterShiftMap::~ClusterShiftMap(void)
{
}

void ClusterShiftMap::ComputeShiftMap(IplImage* input, IplImage* saliency, IplImage* mask, CvSize output, CvSize shiftSize)
{	
	//int *data = new int[num_pixels*num_labels];
	//int *smooth = new int[num_labels*num_labels];
	// IplImage* mask = CreateMask(saliency, output);

	ClusterMap* clusterMap = new ClusterMap();
	clusterMap->CreateClusterOutputMap(mask);
	clusterMap->AssignNeighborHood(clusterMap->_nodeMapping, cvSize(mask->width, mask->height), clusterMap->NodeCount(), _gcGeneral);
	int nodeCount = clusterMap->NodeCount();
	_gcGeneral = new GCoptimizationGeneralGraph(nodeCount, output.width * output.height);
	
	// now set up a neighborhood system according to clustermap
	for(int i = 0; i < nodeCount; i++)
		for(int j = 0; j < nodeCount; j++)
		{
			NeighborhoodList list = clusterMap->_neighborMap->GetNeighbors(i, j);
			if(list.relationShip->size() != 0)
			{
				_gcGeneral->setNeighbors(i,j);
			}
		}

	// now set up data cost and energy cost
}

IplImage* ClusterShiftMap::CreateMask(IplImage* saliency, CvSize output)
{
	IplImage* mask = cvCreateImage(output, saliency->depth, saliency->nChannels);
	// from saliency create mask
	return mask;
}