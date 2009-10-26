#include "StdAfx.h"
#include "MinEnergyPath.h"

void Zooming::ReleasePath(Zooming::Path *path)
{
	delete[] path->pixelList;
	path->pixelList = NULL;
	path = NULL;
}