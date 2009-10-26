#include "Stdafx.h"
#include "EnergyCalculator.h"

IplImage* Zooming::SobelEnergy::GetEnergyImage(IplImage* input)
{ 
	IplImage* dstX = cvCloneImage(input);	
	IplImage* dstY = cvCloneImage(input);
	cvSobel(input, dstY, 0, _yOrder);
	cvSobel(input, dstX, 1, _xOrder);

	int width = input->width;
	int height = input->height;

	// average 3 channels to 1
	IplImage* energy = cvCreateImage(cvSize(width, height), IPL_DEPTH_16U, 1);
	//
	//char* dstData = dstX->imageData;
	//short* energyData = (short*)energy->imageData;

	for(int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j++)
		{
			CvScalar inputPixelX = cvGet2D(dstX, i, j); 
			CvScalar inputPixelY = cvGet2D(dstY, i, j);
 
			CvScalar outputX = cvScalar( abs(inputPixelX.val[0] + inputPixelX.val[1] + inputPixelX.val[2]) / 3);
			CvScalar outputY = cvScalar( abs(inputPixelY.val[0] + inputPixelY.val[1] + inputPixelY.val[2]) / 3);
			
			CvScalar output;
			output.val[0] = outputX.val[0] + outputY.val[0];

			cvSet2D(energy, i, j, output);
		}
		//dstData += dst->widthStep;
		//energyData += energy->width;
	}
	return energy;
}