#include "stdafx.h"
#include "MaskShift.h"
void CreateMask(IplImage* input, IplImage* mask, CvPoint shift, CvSize outputSize, IplImage* outputMask, IplImage* outputData)
{
	// check size
	if(outputMask->width != outputSize.width || outputMask->height != outputSize.height)
		return;
	if(outputData->width != outputSize.width || outputData->height != outputSize.height)
		return;
	
	 
	
	for(int i = 0; i < outputMask->width; i++)
		for(int j = 0; j < outputMask->height; j++)
		{
			CvPoint inputPixel;
			inputPixel.x = shift.x + i;
			inputPixel.y = shift.y + j;
			if(!IsOutside(inputPixel, cvSize(input->width, input->height)))
			{
				cvSet2D(outputMask, j, i, cvGet2D(mask, inputPixel.y, inputPixel.x)); 
			}
			else
			{
				cvSet2D(outputMask, j, i, cvScalar(255));				 
			}
		}
	for(int i = 0; i < outputSize.width; i++)
		for(int j = 0; j < outputSize.height; j++)
		{
			cvSet2D(outputData,j,i, cvScalar(231,233,233));	
		}
	for(int i = 0; i < outputSize.width; i++)
		for(int j = 0; j < outputSize.height; j++)
		{
			
			if(IsMaskedPixel(i, j, outputMask))
			{
				// check neighbor
				CvPoint inputPixel;
				inputPixel.x = shift.x + i;
				inputPixel.y = shift.y + j;
				CvScalar value;
				CvSize inputSize = cvSize(input->width, input->height);
				
				if(!IsOutside(cvPoint(inputPixel.x, inputPixel.y), inputSize) && 
					!IsOutside(cvPoint(i,j),  outputSize))
				{
					value = cvGet2D(input, inputPixel.y, inputPixel.x);
					cvSet2D(outputData, j, i, value);
				}

				if(!IsOutside(cvPoint(inputPixel.x+1, inputPixel.y), inputSize) && 
					!IsOutside(cvPoint(i+1,j),  outputSize))
				{
					value = cvGet2D(input, inputPixel.y, inputPixel.x + 1);
					 
					cvSet2D(outputData, j, i+1, value);
				}

				if(!IsOutside(cvPoint(inputPixel.x - 1, inputPixel.y), inputSize) && 
					!IsOutside(cvPoint(i-1,j),  outputSize))
				{
					value = cvGet2D(input, inputPixel.y, inputPixel.x - 1);
					cvSet2D(outputData, j, i-1, value);
				}
 
				if(!IsOutside(cvPoint(inputPixel.x, inputPixel.y+1), inputSize) && 
					!IsOutside(cvPoint(i,j+1),  outputSize))
				{
					value = cvGet2D(input, inputPixel.y + 1, inputPixel.x);
					
					cvSet2D(outputData, j+1, i, value);
				}


				if(!IsOutside(cvPoint(inputPixel.x, inputPixel.y-1), inputSize) && 
					!IsOutside(cvPoint(i,j-1),  outputSize))
				{
					value = cvGet2D(input, inputPixel.y - 1, inputPixel.x);
					cvSet2D(outputData, j-1, i, value);
				}
			}
		}		
	
		for(int i = 0; i < outputSize.width - 1; i++)
		for(int j = 0; j < outputSize.height - 1; j++)
		{
			if(IsMaskedPixel(i, j, outputMask))
			{
				// check neighbor
				CvScalar value;
				value = cvGet2D(outputData, j, i);
				if(value.val[0] == 231 && value.val[1] == 233 && value.val[2] == 233)
					printf("Test");
				value = cvGet2D(outputData, j+1, i);
				if(value.val[0] == 231 && value.val[1] == 233 && value.val[2] == 233)
					printf("Test");
				value = cvGet2D(outputData, j-1, i);
				if(value.val[0] == 231 && value.val[1] == 233 && value.val[2] == 233)
					printf("Test");
				value = cvGet2D(outputData, j, i+1);
				if(value.val[0] == 231 && value.val[1] == 233 && value.val[2] == 233)
					printf("Test");
				value = cvGet2D(outputData, j, i-1);
				if(value.val[0] == 231 && value.val[1] == 233 && value.val[2] == 233)
					printf("Test");
			}
		}

}
// mask is of the same size as input
MaskShift* CreateMaskShift(IplImage* input, IplImage* mask, CvPoint shift, CvSize outputSize)
{
	IplImage* outputMask = cvCreateImage(outputSize, IPL_DEPTH_8U, 3);

	IplImage* outputTest = cvCreateImage(outputSize, IPL_DEPTH_8U, 3);
	IplImage* outputData = cvCreateImage(outputSize, IPL_DEPTH_8U, 3);
	for(int i = 0; i < outputMask->width; i++)
		for(int j = 0; j < outputMask->height; j++)
		{
			CvPoint inputPixel;
			inputPixel.x = shift.x + i;
			inputPixel.y = shift.y + j;
			if(!IsOutside(inputPixel, cvSize(input->width, input->height)))
			{
				cvSet2D(outputMask, j, i, cvGet2D(mask, inputPixel.y, inputPixel.x)); 
			}
			else
			{
				cvSet2D(outputMask, j, i, cvScalar(255));				 
			}
		}
	for(int i = 0; i < outputSize.width; i++)
		for(int j = 0; j < outputSize.height; j++)
		{
			cvSet2D(outputData,j,i, cvScalar(231,233,233));	
		}
	for(int i = 0; i < outputSize.width; i++)
		for(int j = 0; j < outputSize.height; j++)
		{
			
			if(IsMaskedPixel(i, j, outputMask))
			{
				// check neighbor
				CvPoint inputPixel;
				inputPixel.x = shift.x + i;
				inputPixel.y = shift.y + j;
				CvScalar value;
				CvSize inputSize = cvSize(input->width, input->height);
				
				if(!IsOutside(cvPoint(inputPixel.x, inputPixel.y), inputSize) && 
					!IsOutside(cvPoint(i,j),  outputSize))
				{
					value = cvGet2D(input, inputPixel.y, inputPixel.x);
					cvSet2D(outputData, j, i, value);
				}

				if(!IsOutside(cvPoint(inputPixel.x+1, inputPixel.y), inputSize) && 
					!IsOutside(cvPoint(i+1,j),  outputSize))
				{
					value = cvGet2D(input, inputPixel.y, inputPixel.x + 1);
					 
					cvSet2D(outputData, j, i+1, value);
				}

				if(!IsOutside(cvPoint(inputPixel.x - 1, inputPixel.y), inputSize) && 
					!IsOutside(cvPoint(i-1,j),  outputSize))
				{
					value = cvGet2D(input, inputPixel.y, inputPixel.x - 1);
					cvSet2D(outputData, j, i-1, value);
				}
 
				if(!IsOutside(cvPoint(inputPixel.x, inputPixel.y+1), inputSize) && 
					!IsOutside(cvPoint(i,j+1),  outputSize))
				{
					value = cvGet2D(input, inputPixel.y + 1, inputPixel.x);
					
					cvSet2D(outputData, j+1, i, value);
				}


				if(!IsOutside(cvPoint(inputPixel.x, inputPixel.y-1), inputSize) && 
					!IsOutside(cvPoint(i,j-1),  outputSize))
				{
					value = cvGet2D(input, inputPixel.y - 1, inputPixel.x);
					cvSet2D(outputData, j-1, i, value);
				}
			}
		}

		int j2 =21;
		int i2 = 5;	 
		CvScalar value2 = cvGet2D(outputData, j2+1, i2);
 
	for(int i = 0; i < outputSize.width; i++)
		for(int j = 0; j < outputSize.height; j++)
		{
			if(IsMaskedPixel(i, j, outputMask))
			{
				// check neighbor
				CvScalar value;
				value = cvGet2D(outputData, j, i);
				if(value.val[0] == 231 && value.val[1] == 233 && value.val[2] == 233)
					printf("Test");
				value = cvGet2D(outputData, j+1, i);
				if(value.val[0] == 231 && value.val[1] == 233 && value.val[2] == 233)
					printf("Test");
				value = cvGet2D(outputData, j-1, i);
				if(value.val[0] == 231 && value.val[1] == 233 && value.val[2] == 233)
					printf("Test");
				value = cvGet2D(outputData, j, i+1);
				if(value.val[0] == 231 && value.val[1] == 233 && value.val[2] == 233)
					printf("Test");
				value = cvGet2D(outputData, j, i-1);
				if(value.val[0] == 231 && value.val[1] == 233 && value.val[2] == 233)
					printf("Test");
			}
		}
	//cvNamedWindow("Test");
	//while(1)
	//{
	//	cvShowImage("Test", outputData);
	//	cvWaitKey(100);
	//}
	MaskShift* maskShift = new MaskShift(outputMask, input);
	maskShift->SetMaskData(outputData);
	return maskShift;
}
void MaskShift::SetMaskData(IplImage* image)
{
	_maskData = image;
}
bool MaskShift::IsMaskedPixel(int x, int y)
{
	CvScalar value = cvGet2D(_mask, y, x);
	if(value.val[0] > 100)
		return false;
	else return true;
}
bool IsMaskedPixel(int x, int y, IplImage* image)
{
	CvScalar value = cvGet2D(image, y, x);
	if(value.val[0] > 100)
		return false;
	else return true;
}
MaskShift::MaskShift(IplImage* mask, IplImage* imageData)
{
	_mask = mask;
	_imageData = imageData;
}

MaskShift::~MaskShift(void)
{
}

void MaskShift::AddShift(CvPoint shift)
{
	_shift = shift;
}

void MaskShift::ChangeMask(IplImage* mask)
{
	_mask = mask;
}

bool MaskShift::IsMaskedPixel(CvPoint pixel)
{
	return IsMaskedPixel(pixel.x, pixel.y);
}



CvScalar MaskShift::GetPixelValue(CvPoint pixel)
{
	return GetPixelValue(pixel.x, pixel.y);
}
CvScalar MaskShift::GetPixelValue(int x, int y)
{
	return cvGet2D(_imageData, y + _shift.y, x + _shift.x);
}

int MaskShift::GetWidth()
{
	return _mask->width;
}
int MaskShift::GetHeight(){
	return _mask->height;
}

CvScalar MaskShift::GetPixelValue(CvPoint pixel, IplImage* image)
{
	return GetPixelValue(pixel.x, pixel.y, image);
}
CvScalar MaskShift::GetPixelValue(int x, int y, IplImage* image)
{
	return cvGet2D(image, y + _shift.y, x + _shift.x);
}