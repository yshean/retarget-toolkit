#pragma once
#include "Matrix3D.h"
#include <cv.h>

class ColorConverter
{
public:
	ColorConverter(void);
	~ColorConverter(void);

	// take out frame by frame and convert from RGB to LUV space
	void ConvertRGB2LUV(Matrix3D* input, Matrix3D* output);

	void ConvertRGB2GRAY(Matrix3D* input, Matrix3D* output);
 
};
