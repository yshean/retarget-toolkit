 
#include <cv.h>

/**********************************************
A width x height image will have width x height number of labels
Label index is assigned from left to right, then top to bottom

Note: pixel (0,0) will be assigned label 0
e.g: 4x4 image
0 1  2  3
4 5  6  7
etc
********************************************************/

// Get label index
int GetLabel(CvPoint point, int width, int height);

// Get pixel position from label
CvPoint GetPoint(int label, int width, int height);

 

void SetMapDataTerm(IplImage* map, int u, int v, double value);
