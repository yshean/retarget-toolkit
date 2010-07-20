 
#include <cv.h>
 

// Get label index
int GetLabel(CvPoint point, int width, int height);

// Get pixel position from label
CvPoint GetPoint(int label, int width, int height);

bool IsOutside(CvPoint point, int width, int height); 

CvPoint GetMappedPoint(CvPoint pixel, int label, int width, int height);

void SetMapDataTerm(IplImage* map, int u, int v, double value);

// width & height are default to be 3 (so 9 labels in total)
// however just leave them here for flexibility
CvPoint GetMappedPointInitialGuess(CvPoint pixel, int label, int width, int height, IplImage* initialGuess);

// set a label to a label map
CvPoint SetLabel(CvPoint point, CvPoint shiftLabel, IplImage* labelMap);

// get a label from a label map
CvPoint GetLabel(CvPoint point, IplImage* labelMap);