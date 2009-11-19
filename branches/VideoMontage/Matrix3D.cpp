#include "StdAfx.h"
#include "Matrix3D.h"
#include "Geometry.h"

#pragma region Matrix3D
Matrix3D::Matrix3D(int width, int height, int length)
{
	_width = width;
	_height = height;
	_length = length;
}

Matrix3D::Matrix3D(int width, int height, int length, int channel)
{
	_width = width;
	_height = height;
	_length = length;
	_channel = channel;
}


Matrix3D::~Matrix3D(void)
{
}


bool Matrix3D::IsCoordinateInside(int x, int y, int z)
{
	bool ret = IsInsideBound(0, _width - 1, x) && IsInsideBound(0, _height - 1, y) && IsInsideBound(0, _length - 1, z);

	return ret;
}
 

//char Get3DChar(Matrix3D* matrix, int x, int y, int z)
//{ 
//	if(matrix->IsCoordinateInside(x, y, z))
//	{
//		char* data = matrix->_data;
//		// move to frame z
//		data += z * matrix->_width * matrix->_height;
//
//		// align to row
//		data += y * matrix->_width;
//		
//		// align to col
//		data += x;	
//
//		return *data;
//	}
//	else
//		return 0;
//}
//
//int Get3DInt(Matrix3D* matrix, int x, int y, int z)
//{
//	if(matrix->IsCoordinateInside(x, y, z))
//	{
//		int* data = (int*) matrix->_data;
//		
//		// move to frame z
//		data += z * matrix->_width * matrix->_height;
//
//		// align to row
//		data += y * matrix->_width;
//		
//		// align to col
//		data += x;	
//
//		return *data;
//	}else
//		return 0;
//	
//}
//
//void Set3DChar(Matrix3D* matrix, int x, int y, int z, char value)
//{
//	if(matrix->IsCoordinateInside(x, y, z))
//	{
//		int* data = (int*) matrix->_data;
//		
//		// move to frame z
//		data += z * matrix->_width * matrix->_height;
//
//		// align to row
//		data += y * matrix->_width;
//		
//		// align to col
//		data += x;	
//
//		*data = value;
//	}
//	// else don't set anything
//}
//
//void Set3DInt(Matrix3D* matrix, int x, int y, int z, int value)
//{
//	if(matrix->IsCoordinateInside(x, y, z))
//	{
//		int* data = (int*) matrix->_data;
//		
//		// move to frame z
//		data += z * matrix->_width * matrix->_height;
//
//		// align to row
//		data += y * matrix->_width;
//		
//		// align to col
//		data += x;	
//
//		*data = value;
//	}
//	// else do not set anything
//}
//
//
//


void Matrix3D::GetIplImageZ(int z, IplImage *image)
{
	if( z >= 0 && z < _length)
	{
		for(int x = 0; x < _width; x++)
			for(int y = 0; y < _height; y++)
			{
				CvScalar value = Get3DScalar(x, y, z);
				cvSet2D(image, y, x, value);
			}
			//cvCvtColor(image, image, CV_RGB);
	}
}

void Matrix3D::GetIplImageX(int x, IplImage *image)
{
	if( x >= 0 && x < _width)
	{
		for(int z = 0; z < _length; z++)
			for(int y = 0; y < _height; y++)
			{
				CvScalar value = Get3DScalar(x, y, z);
				cvSet2D(image, z, y, value);
			}
	}
}

void Matrix3D::SetIplImageZ(int z, IplImage *image)
{
	if( z >= 0 && z < _length)
	{
		for(int x = 0; x < _width; x++)
		{
			for(int y = 0; y < _height; y++)
			{				
				CvScalar value = cvGet2D(image, y, x);	  
				Set3DScalar(x, y, z, value);
			}
		}
	}
}

void Matrix3D::SetIplImageX(int x, IplImage *image)
{	
	if( x >= 0 && x < _width)
	{
		for(int z = 0; z < _length; z++)
			for(int y = 0; y < _height; y++)
			{
				CvScalar value = cvGet2D(image, z, y);
				Set3DScalar(x, y, z, value);
			}
	}
}

#pragma endregion 

#pragma region Matrix3DChar

void Matrix3DChar::Set3D(int x, int y, int z, double value)
{
	//printf("X: %i, Y: %i, Z: %i \n", x, y, z);

	if(this->IsCoordinateInside(x, y, z) || _channel == 1)
	{
		unsigned char* data = (unsigned char*)_data;	 
		// move to frame z
		data += z * _width * _height;

		// align to row
		data += y * _width;
		
		// align to col
		data += x;	

		*data = value; 
		 
	} 
}
double Matrix3DChar::Get3D(int x, int y, int z)
{
	if(this->IsCoordinateInside(x, y, z) && _channel == 1)
	{
		unsigned char* data = (unsigned char*)_data;
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


CvScalar Matrix3DChar::Get3DScalar(int x, int y, int z)
{
	CvScalar value = cvScalar(0);
	
	if(this->IsCoordinateInside(x, y, z))
	{
		unsigned char* data = (unsigned char*)_data;	 
		//char* data = _data;
		// move to frame z
		data += z * _width * _height * _channel;

		// align to row
		data += y * _width * _channel;
		
		// align to col
		data += x * _channel;	
		
		for(int i = 0; i < _channel; i++)
		{
			value.val[i] = (double)*data;		
			data++;
		}
	}

	return value;
}

void Matrix3DChar::Set3DScalar(int x, int y, int z, CvScalar value)
{
	if(this->IsCoordinateInside(x, y, z))
	{
		unsigned char* data = (unsigned char*)_data;	 
		// move to frame z
		data += z * _width * _height * _channel;

		// align to row
		data += y * _width * _channel;
		
		// align to col
		data += x * _channel;	
		
		for(int i = 0; i < _channel; i++)
		{
			*data = (unsigned char)value.val[i];		
			data++;
		}
	}
}

#pragma endregion

#pragma region Matrix3DInt

void Matrix3DInt::Set3D(int x, int y, int z, double value)
{
	//printf("X: %i, Y: %i, Z: %i \n", x, y, z);

	if(this->IsCoordinateInside(x, y, z) || _channel == 1)
	{
		unsigned int* data = (unsigned int*)_data;	 
		// move to frame z
		data += z * _width * _height;

		// align to row
		data += y * _width;
		
		// align to col
		data += x;	

		*data = value; 
		 
	} 
}
double Matrix3DInt::Get3D(int x, int y, int z)
{
	if(this->IsCoordinateInside(x, y, z) && _channel == 1)
	{
		unsigned int* data = (unsigned int*)_data;
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


CvScalar Matrix3DInt::Get3DScalar(int x, int y, int z)
{
	CvScalar value = cvScalar(0);
	
	if(this->IsCoordinateInside(x, y, z))
	{
		unsigned int* data = (unsigned int*)_data;	 
		//char* data = _data;
		// move to frame z
		data += z * _width * _height * _channel;

		// align to row
		data += y * _width * _channel;
		
		// align to col
		data += x * _channel;	
		
		for(int i = 0; i < _channel; i++)
		{
			value.val[i] = (double)*data;		
			data++;
		}
	}

	return value;
}

void Matrix3DInt::Set3DScalar(int x, int y, int z, CvScalar value)
{
	if(this->IsCoordinateInside(x, y, z))
	{
		unsigned int* data = (unsigned int*)_data;	 
		// move to frame z
		data += z * _width * _height * _channel;

		// align to row
		data += y * _width * _channel;
		
		// align to col
		data += x * _channel;	
		
		for(int i = 0; i < _channel; i++)
		{
			*data = (unsigned int)value.val[i];		
			data++;
		}
	}
}


#pragma endregion

#pragma region Matrix3DFloat

void Matrix3DFloat::Set3D(int x, int y, int z, double value)
{
	//printf("X: %i, Y: %i, Z: %i \n", x, y, z);

	if(this->IsCoordinateInside(x, y, z) || _channel == 1)
	{
		float* data = (float*)_data;	 
		// move to frame z
		data += z * _width * _height;

		// align to row
		data += y * _width;
		
		// align to col
		data += x;	

		*data = value; 
		 
	} 
}
double Matrix3DFloat::Get3D(int x, int y, int z)
{
	if(this->IsCoordinateInside(x, y, z) && _channel == 1)
	{
		float* data = (float*)_data;
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


CvScalar Matrix3DFloat::Get3DScalar(int x, int y, int z)
{
	CvScalar value = cvScalar(0);
	
	if(this->IsCoordinateInside(x, y, z))
	{
		float* data = (float*)_data;	 
		//char* data = _data;
		// move to frame z
		data += z * _width * _height * _channel;

		// align to row
		data += y * _width * _channel;
		
		// align to col
		data += x * _channel;	
		
		for(int i = 0; i < _channel; i++)
		{
			value.val[i] = (double)*data;		
			data++;
		}
	}

	return value;
}

void Matrix3DFloat::Set3DScalar(int x, int y, int z, CvScalar value)
{
	if(this->IsCoordinateInside(x, y, z))
	{
		float* data = (float*)_data;	 
		// move to frame z
		data += z * _width * _height * _channel;

		// align to row
		data += y * _width * _channel;
		
		// align to col
		data += x * _channel;	
		
		for(int i = 0; i < _channel; i++)
		{
			*data = (float)value.val[i];		
			data++;
		}
	}
}

#pragma endregion 