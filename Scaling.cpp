#include "StdAfx.h"
#include "Scaling.h"

Scaling::Scaling(void)
{
}

Scaling::~Scaling(void)
{
}

void Scaling::PyrDownZ(Matrix3D *input, Matrix3D *output)
{
	int width = input->_width;
	int height = input->_height;
	int length = input->_length;
	
	IplImage* image = cvCreateImage(cvSize(width, height), input->_type, 3);
	IplImage* dst = cvCreateImage(cvSize(width/2, height/2), input->_type, 3);
	
	 
	for(int z = 0; z < length; z++)
	{
		// fill image with a slice in 3D matrix
		input->GetIplImageZ(z, image);
		cvPyrDown(image, dst);
		output->SetIplImageZ(z, dst);		
	}
	cvReleaseImage(&image);
	cvReleaseImage(&dst);
}

void Scaling::PyrDownX(Matrix3D *input, Matrix3D *output)
{
	int width = input->_width;
	int height = input->_height;
	int length = input->_length;
	
	IplImage* image = cvCreateImage(cvSize(height, length), input->_type, 3);
	IplImage* dst = cvCreateImage(cvSize(height/2, length/2), input->_type, 3);
	
	 
	for(int x = 0; x < width; x++)
	{
		// fill image with a slice in 3D matrix
		input->GetIplImageX(x, image);
		cvPyrDown(image, dst);
		output->SetIplImageX(x, dst);		
	}
	cvReleaseImage(&image);
	cvReleaseImage(&dst);
}
