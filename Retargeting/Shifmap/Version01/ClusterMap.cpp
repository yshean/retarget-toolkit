#include "stdafx.h"
#include "ClusterMap.h"
#include "Label.h"

// this file implement cluster map algorithm
ClusterMap::ClusterMap()
{
}

ClusterMap::~ClusterMap()
{
}


int ClusterMap::AssignToCluster(int* mapping, CvPoint point, CvSize imageSize)
{
	int currentLabel = GetLabel(point, imageSize);
	int index;
	index = CheckClusterNeighbor(mapping, currentLabel, cvPoint(point.x - 1, point.y), imageSize);
	if(index!=-1) return index;
	index = CheckClusterNeighbor(mapping, currentLabel, cvPoint(point.x, point.y - 1), imageSize);
	if(index!=-1) return index;
	index = CheckClusterNeighbor(mapping, currentLabel, cvPoint(point.x - 1, point.y - 1), imageSize);
	if(index!=-1) return index;
	index = CheckClusterNeighbor(mapping, currentLabel, cvPoint(point.x + 1, point.y - 1), imageSize);
	return index;
}
int ClusterMap::CheckClusterNeighbor(int* mapping, int currentLabel, CvPoint neighbor, CvSize imageSize)
{
	if(IsOutside(neighbor, imageSize))
		return -1;
	int label = GetLabel(neighbor, imageSize);
	if(mapping[label]!=-1)
	{
		mapping[currentLabel] = mapping[label];
		return mapping[label];
	}
	else
		return -1;
}
void ClusterMap::CreateClusterOutputMap(IplImage* mask)
{
	int index = 0;
	_labelCount = mask->width * mask->height;
	_nodeMapping = new int[_labelCount];
	_pixelMapping = new vector<PixelList*>();
	for(int i = 0; i < mask->width*mask->height;i++)
	{
		_nodeMapping[i] = -1;
	}

	for(int i = 0; i < mask->width; i++)
		for(int j = 0; j < mask->height; j++)
		{
			int label = GetLabel(cvPoint(i,j), cvSize(mask->width, mask->height));
			CvScalar value = cvGet2D(mask, j, i);
			CvPoint* point = new CvPoint();
			point->x = i;
			point->y = j;
			if(value.val[0] < 100 )
			{
				int clusterId = AssignToCluster(_nodeMapping, cvPoint(i,j), cvSize(mask->width, mask->height));

				if(clusterId != -1)
				{				
					(*_pixelMapping)[clusterId]->points->push_back(point);
					_nodeMapping[label] = clusterId;
				}
				else
				{					
					// new cluster
					_nodeMapping[label] = index;
					PixelList* list = new PixelList();
					_pixelMapping->push_back(list);
					list->points = new vector<CvPoint*>();
					//CvPoint* point = new CvPoint();
					//point->x = i;
					//point->y = j;
					list->points->push_back(point);
					index++;
				}
			}
			else
			{				
				_nodeMapping[label] = index;
				PixelList* list = new PixelList();
				_pixelMapping->push_back(list);
				list->points = new vector<CvPoint*>();
				//CvPoint* point = new CvPoint();
				//point->x = i;
				//point->y = j;
				list->points->push_back(point);
				index++;
			}
		}		
		
}

void ClusterMap::AssignNeighborHood(int* mapping, CvSize imageSize, int numLabel, GCoptimizationGeneralGraph * gc)
{
	int totalPixel = imageSize.width * imageSize.height;
	vector<PointPair*>* neighborhood = new vector<PointPair*>(totalPixel);
	_neighborMap = new ClusterMapNeighbor(NodeCount());
	for(int i = 0; i < totalPixel; i++)
	{
		int nodeId1 = mapping[i];
		// getting neighbor
		CvPoint currentPoint = GetPoint(i, imageSize);
		 
		SetNeighborPointPair(mapping, imageSize, nodeId1, currentPoint, cvPoint(currentPoint.x, currentPoint.y + 1), _neighborMap);
		SetNeighborPointPair(mapping, imageSize, nodeId1, currentPoint, cvPoint(currentPoint.x, currentPoint.y - 1), _neighborMap);
		SetNeighborPointPair(mapping, imageSize, nodeId1, currentPoint, cvPoint(currentPoint.x + 1, currentPoint.y), _neighborMap);
		SetNeighborPointPair(mapping, imageSize, nodeId1, currentPoint, cvPoint(currentPoint.x - 1, currentPoint.y), _neighborMap);
	}
}

void ClusterMap::SetNeighborPointPair(int* mapping, CvSize imageSize, int nodeId1, CvPoint currentPoint, CvPoint neighbor, ClusterMapNeighbor* neighborMap)
{ 	
	int pixelId2 = GetLabel(neighbor, imageSize);
	if(pixelId2 != -1)
	{
		int nodeId2 = mapping[pixelId2];
		if(nodeId2 != nodeId1)
		{
			PointPair* pair = new PointPair();
			pair->point1 = currentPoint;
			pair->point2 = neighbor; 
			neighborMap->AddNeighbor(nodeId1, nodeId2, pair);
		}
	}
}

int ClusterMap::GetMapping(int pixelId)
{
	return _nodeMapping[pixelId];
}

int ClusterMap::NodeCount()
{
	return _pixelMapping->size();
}