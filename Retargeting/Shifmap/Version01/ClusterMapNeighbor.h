#pragma once
#include <cv.h>
#include <vector>
using namespace std;

struct PointPair
{
	CvPoint point1;
	CvPoint point2;
};

struct NeighborhoodList
{
	vector<PointPair*>* relationShip;
};

// this class handle neighborhood between nodes in clustermap
class ClusterMapNeighbor
{
public:
	ClusterMapNeighbor(int nodeCount);
	~ClusterMapNeighbor(void);

	void AddNeighbor(int nodeId1, int nodeId2, PointPair* pointPair);
	NeighborhoodList GetNeighbors(int nodeId1, int nodeId2);
protected:
	NeighborhoodList* _neighborhoodList;	
	int _nodeCount;
};
