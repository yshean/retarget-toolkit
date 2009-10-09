#pragma once
#include <cv.h>
 

#define MAT3D_8S 0
#define MAT3D_16S 1


class Matrix3D
{
public:
	Matrix3D(int width, int height, int length);
	~Matrix3D(void);

	int _width;
	int _height;
	int _length;
	int _type;

	// check whether the coordinate is inside the volume
	bool IsCoordinateInside(int x, int y, int z);
	virtual double Get3D(int x, int y, int z) = 0;
	virtual void Set3D(int x, int y, int z, double value) = 0;

	char* _data;	
};

class Matrix3DChar : public Matrix3D
{
public:
	Matrix3DChar(int width, int height, int length) : Matrix3D(width, height, length)
	{
		_data = (char*)malloc(length * sizeof(char));
	}

	~Matrix3DChar(void);

	virtual double Get3D(int x, int y, int z);
	virtual void Set3D(int x, int y, int z, double value);
};

