#pragma once
#include "GCAlgorithm.h"


class GCScaleImage : public GCAlgorithm
{
public:
	GCScaleImage(void);
	~GCScaleImage(void);

public:
	void Initialize(int width, int height, int labelCount);
};
