#include "StdAfx.h"
#include "SobelImageImportance.h"

SobelImageImportance::SobelImageImportance(void)
{
}

SobelImageImportance::~SobelImageImportance(void)
{
}

double SobelImageImportance::GetImageImportance(IplImage* image)
{
	IplImage* clone = cvCloneImage(image);
	cvSobel(image, clone, 2, 2);
	CvScalar value = cvSum(clone);
	return value.val[0] + value.val[1] + value.val[2];
}
