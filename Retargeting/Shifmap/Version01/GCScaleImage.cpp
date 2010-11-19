#include "StdAfx.h"
#include "GCScaleImage.h"

GCScaleImage::GCScaleImage(void)
{
}

GCScaleImage::~GCScaleImage(void)
{
}

void GCScaleImage::Initialize(int width, int height, int labelCount)
{
	_gc = new GCoptimizationGridGraph(width, height, labelCount);	
}
