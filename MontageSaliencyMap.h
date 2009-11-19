#pragma once
#include "CommonDefinition.h"
#include "SaliencyMap.h"
#include "Norm.h"
#include "Scaling.h"

struct MontageSaliencySetting
{
	Norm* norm;
	Scaling* scaling;
};

// this class compute the saliency map based on Space-Time Video Montage paper
// see 3.1 section of the paper
class MontageSaliencyMap : public SaliencyMap
{
public:	 
	MontageSaliencyMap(MontageSaliencySetting* setting);
	~MontageSaliencyMap(void);

#pragma region Algorithm Setting
	MontageSaliencySetting* _setting;
	
	// neighbor setting
	// NEIGHBOR_6
	// others: default to NEIGHBOR_6 (will be extended later)
	int _neighbor;
	
#pragma endregion

	// This method calculate the saliency map by first take the sum
	// of the distance between each pixel and its neighbors
	// Distance function is based on the _norm used	
	virtual void CalculateSaliencyMap(Volume3D* volume, Volume3D* output);
	
	virtual void CalculateSaliencyMap(Matrix3D* matrix, Matrix3D* output);
	
protected:
	// calculate distance between a pixel and its 6 neighbors in 3D space
	virtual void Distance6Neighbor(Volume3D* volume, Volume3D* output);
 
};
