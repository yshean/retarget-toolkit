#include "StdAfx.h"
#include "ClusterMapNeighbor.h"

ClusterMapNeighbor::ClusterMapNeighbor(int nodeCount)
{
	// int relationCount = nodeCount * (nodeCount - 1);
	int relationCount = nodeCount * nodeCount;
	_nodeCount = nodeCount;
	_neighborhoodList = new NeighborhoodList[relationCount];	
	for(int i = 0; i < relationCount; i++)
	{
		_neighborhoodList[i].relationShip = new vector<PointPair*>();
	}
}

ClusterMapNeighbor::~ClusterMapNeighbor(void)
{
}

void ClusterMapNeighbor::AddNeighbor(int nodeId1, int nodeId2, PointPair* pointPair)
{
	NeighborhoodList list;
	//if(nodeId1 > nodeId2)
	//	list = _neighborhoodList[(nodeId1) * (nodeId1 - 1) + nodeId2];
	//else
	//	list = _neighborhoodList[(nodeId2) * (nodeId2 - 1) + nodeId1];
	list = _neighborhoodList[nodeId1 * _nodeCount + nodeId2];
 	list.relationShip->push_back(pointPair);
 
 
}

NeighborhoodList ClusterMapNeighbor::GetNeighbors(int nodeId1, int nodeId2)
{
	//if(nodeId1 > nodeId2)
	//	return _neighborhoodList[(nodeId1 - 1) * (nodeId1 - 2) + nodeId2];
	//else
	//	return _neighborhoodList[(nodeId2 - 1) * (nodeId2 - 2) + nodeId1];
	return _neighborhoodList[nodeId1 * _nodeCount + nodeId2];
}
