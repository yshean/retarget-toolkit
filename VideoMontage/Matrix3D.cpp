#include "StdAfx.h"
#include "Matrix3D.h"
#include "Geometry.h"

Matrix3D::Matrix3D(int width, int height, int length)
{
	_width = width;
	_height = height;
}

Matrix3D::~Matrix3D(void)
{
}


bool Matrix3D::IsCoordinateInside(int x, int y, int z)
{
	bool ret = IsInsideBound(0, _width, x) && IsInsideBound(0, _height, y) && IsInsideBound(0, _length, z);

	return ret;
}

char Get3DChar(Matrix3D* matrix, int x, int y, int z)
{ 
	if(matrix->IsCoordinateInside(x, y, z))
	{
		char* data = matrix->_data;
		// move to frame z
		data += z * matrix->_width * matrix->_height;

		// align to row
		data += y * matrix->_width;
		
		// align to col
		data += x;	

		return *data;
	}
	else
		return 0;
}

int Get3DInt(Matrix3D* matrix, int x, int y, int z)
{
	if(matrix->IsCoordinateInside(x, y, z))
	{
		int* data = (int*) matrix->_data;
		
		// move to frame z
		data += z * matrix->_width * matrix->_height;

		// align to row
		data += y * matrix->_width;
		
		// align to col
		data += x;	

		return *data;
	}else
		return 0;
	
}

void Set3DChar(Matrix3D* matrix, int x, int y, int z, char value)
{
	if(matrix->IsCoordinateInside(x, y, z))
	{
		int* data = (int*) matrix->_data;
		
		// move to frame z
		data += z * matrix->_width * matrix->_height;

		// align to row
		data += y * matrix->_width;
		
		// align to col
		data += x;	

		*data = value;
	}
	// else don't set anything
}

void Set3DInt(Matrix3D* matrix, int x, int y, int z, int value)
{
	if(matrix->IsCoordinateInside(x, y, z))
	{
		int* data = (int*) matrix->_data;
		
		// move to frame z
		data += z * matrix->_width * matrix->_height;

		// align to row
		data += y * matrix->_width;
		
		// align to col
		data += x;	

		*data = value;
	}
	// else do not set anything
}




double Matrix3DChar::Get3D(int x, int y, int z)
{
	if(this->IsCoordinateInside(x, y, z))
	{
		char* data = _data;
		// move to frame z
		data += z * _width * _height;

		// align to row
		data += y * _width;
		
		// align to col
		data += x;	

		return *data;
	}
	else
		return 0;
}

void Matrix3DChar::Set3D(int x, int y, int z, double value)
{
	if(this->IsCoordinateInside(x, y, z))
	{
		char* data = _data;
		// move to frame z
		data += z * _width * _height;

		// align to row
		data += y * _width;
		
		// align to col
		data += x;	

		*data = value;
	} 
}