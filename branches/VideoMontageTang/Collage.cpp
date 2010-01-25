#include "StdAfx.h" 
#include "Collage.h"
#include "ROIBlend.h"

Collage::Collage(void)
{
}

Collage::~Collage(void)
{
}

IplImage* Collage::CreateCollage(IplImage** pictureList, int length)
{
	_roiBlend = new ROIBlend();
	
	if(length > 1)
	{	
		IplImage* result;

		// calculate minheight
		int minHeight = int::MaxValue;		
		for(int i = 0; i < length; i++)
		{
			IplImage* image = pictureList[i];			 
			int height = image->height;
			if(height < minHeight) minHeight = height;		
		}		
		
		// ********resize then blend
		// first image
		double ratio = (double)minHeight/ (double)pictureList[0]->height;
		result = cvCreateImage(cvSize(pictureList[0]->width * ratio, minHeight), 
			pictureList[0]->depth, pictureList[0]->nChannels);
		cvResize(pictureList[0], result);
		int size = 80;
		int std = 20;
		int blendType = 3;
		// from second image
		for(int i = 1; i < length; i++)
		{
			IplImage* temp = cvCreateImage(cvSize(pictureList[i]->width * ratio, minHeight), 
			pictureList[i]->depth, pictureList[i]->nChannels);
			cvResize(pictureList[i], temp);

			switch(blendType)
			{
			case 1: result = _roiBlend->BlendImages1(result, temp, size, std);
				break;
			case 2: result = _roiBlend->BlendImages2(result, temp, size, std);
				break;
			case 3: result = _roiBlend->BlendImages3(result, temp, size, std);
				break;
			default:
				result = _roiBlend->BlendImages3(result, temp, size,  std);
			}
		}
		
		return result;
	}	
}