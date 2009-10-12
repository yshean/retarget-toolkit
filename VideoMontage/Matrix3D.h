#pragma once
#include <cv.h>


#define MAT3D_8S 0
#define MAT3D_16S 1


class Matrix3D
{
public:
	// default 1 channel
	Matrix3D(int width, int height, int length);
	Matrix3D(int width, int height, int length, int channel);
	~Matrix3D(void);

	int _width;
	int _height;
	int _length;
	int _channel;

	// check whether the coordinate is inside the volume
	bool IsCoordinateInside(int x, int y, int z);
	virtual double Get3D(int x, int y, int z) = 0;
	virtual CvScalar Get3DScalar(int x, int y, int z) = 0;
	virtual void Set3D(int x, int y, int z, double value) = 0;
	virtual void Set3DScalar(int x, int y, int z, CvScalar value) = 0;

	virtual void GetIplImageZ(int z, IplImage* image);
	virtual void GetIplImageX(int x, IplImage* image);

	// fill a image to the matrix
	virtual void SetIplImageZ(int z, IplImage* image);
	virtual void SetIplImageX(int x, IplImage* image);

	char* _data;	
};

class Matrix3DChar : public Matrix3D
{
public:
	// default 1 channel
	Matrix3DChar(int width, int height, int length) : Matrix3D(width, height, length)
	{	
		_data = (char*)malloc(length * width * height * sizeof(char));
	}

	Matrix3DChar(int width, int height, int length, int channel) : Matrix3D(width, height, length, channel)
	{
		_data = (char*)malloc(length * width * height * channel * sizeof(char));
	}

	~Matrix3DChar(void);

	virtual double Get3D(int x, int y, int z);
	virtual CvScalar Get3DScalar(int x, int y, int z);
	virtual void Set3D(int x, int y, int z, double value);
	virtual void Set3DScalar(int x, int y, int z, CvScalar value);
};

