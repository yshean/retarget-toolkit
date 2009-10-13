#include "StdAfx.h"
#include "ColorConverter.h"

ColorConverter::ColorConverter(void)
{
}

ColorConverter::~ColorConverter(void)
{
}

void ColorConverter::ConvertRGB2LUV(Matrix3D* input, Matrix3D* output)
{
	int length = input->_length;
	IplImage* image = cvCreateImage(cvSize(input->_width, input->_height), input->_type, input->_channel);
	
	for(int z = 0; z < length; z++)
	{
		input->GetIplImageZ(z, image);
		cvCvtColor(image, image, CV_RGB2Luv);
		output->SetIplImageZ(z, image);
	}
}
