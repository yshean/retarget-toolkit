// some helpers for openCV
#include <cv.h>

namespace OpenCVEx
{
		double GetValue(IplImage* image, int x, int y); 

		// return min possible value instead of max double
		double GetValueEx(IplImage* image, int x, int y);

		void SetValue(IplImage* image, int x, int y, double value);

}
 