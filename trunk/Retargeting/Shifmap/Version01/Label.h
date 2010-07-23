 
#include <cv.h>
 

// Get label index
int GetLabel(CvPoint point, CvSize imageSize);

// Get pixel position from label
CvPoint GetPoint(int label, CvSize imageSize);

bool IsOutside(CvPoint point, CvSize imageSize); 

CvPoint GetMappedPoint(int pixel, int label, CvSize output, CvSize shiftSize);

CvPoint GetShift(int label, CvSize shiftSize);

void SetMapDataTerm(IplImage* map, int u, int v, double value);

// width & height are default to be 3 (so 9 labels in total)
// however just leave them here for flexibility
CvPoint GetMappedPointInitialGuess(int pixel, int label, CvSize output, CvSize shiftSize, IplImage* initialGuess);



// set a label to a label map
void SetLabel(CvPoint point, CvPoint shiftLabel, IplImage* labelMap);

// get a label from a label map
CvPoint GetLabel(CvPoint point, IplImage* labelMap);