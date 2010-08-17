#pragma once
#include <cv.h>
#include "GCoptimization.h"
#include <vector>
using namespace std;
#include "ClusterMapNeighbor.h"

struct PixelList
{
	vector<CvPoint*>* points;
};

class ClusterMap
{ 
public:
	ClusterMap(void);
	~ClusterMap(void);

	int GetMapping(int pixelId);
	void CreateClusterOutputMap(IplImage* mask);
	void AssignNeighborHood(int* mapping, CvSize imageSize, int numLabel, GCoptimizationGeneralGraph * gc);
	int NodeCount();
	// neighborhood:
	// vectors of pairs of vectors?
public:
	ClusterMapNeighbor* _neighborMap;
	int* _nodeMapping;
protected:
	
	vector<PixelList*>* _pixelMapping;
	int _labelCount;
protected:
	// flexible neighborhood system
	// building the cluster map
	// which is the mapping of grid map into cluster map in the output	

	// return cluster index
	int AssignToCluster(int* mapping, CvPoint point, CvSize imageSize);
	int CheckClusterNeighbor(int* mapping, int currentLabel, CvPoint neighbor, CvSize imageSize);
	
	void SetNeighborPointPair(int* mapping, CvSize imageSize, int nodeId1, CvPoint currentPoint, CvPoint neighbor, ClusterMapNeighbor* neighborMap);
	
};

 

 
