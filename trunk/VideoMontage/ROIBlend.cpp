#include "stdafx.h"

// Include header files
#include "cv.h"
#include "highgui.h"
/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>*/
#include "math.h"
IplImage *Igray=0, *It = 0, *Iat;

//Commented for later use in 2D
 
/*void cvShowManyImages(char* title, int nArgs, ...) {

    // img - Used for getting the arguments 
    IplImage *img;
	CvPoint p1, p2;

    // DispImage - the image in which input images are to be copied
    IplImage *DispImage;

    int size;
    int i;
    int m, n;
    int x, y;

    // w - Maximum number of images in a row 
    // h - Maximum number of images in a column 
    int w, h;

    // scale - How much we have to resize the image
    float scale;
    int max;

    // If the number of arguments is lesser than 0 or greater than 12
    // return without displaying 
    if(nArgs <= 0) {
        printf("Number of arguments too small....\n");
        return;
    }
    else if(nArgs > 12) {
        printf("Number of arguments too large....\n");
        return;
    }
    // Determine the size of the image, 
    // and the number of rows/cols 
    // from number of arguments 
    else if (nArgs == 1) {
        w = h = 1;
        size = 300;
    }
    else if (nArgs == 2) {
        w = 2; h = 1;
        size = 300;
    }
    else if (nArgs == 3 || nArgs == 4) {
        w = 2; h = 2;
        size = 300;
    }
    else if (nArgs == 5 || nArgs == 6) {
        w = 3; h = 2;
        size = 200;
    }
    else if (nArgs == 7 || nArgs == 8) {
        w = 4; h = 2;
        size = 200;
    }
    else {
        w = 4; h = 3;
        size = 150;
    }
	
    // Create a new 3 channel image
    DispImage = cvCreateImage( cvSize(size*w, size*h), 8, 3 );

    // Used to get the arguments passed
    va_list args;
    va_start(args, nArgs);

    // Loop for nArgs number of arguments
    for (i = 0, m = 20, n = 20; i < nArgs; i++, m += (size)-50) {

        // Get the Pointer to the IplImage
        img = va_arg(args, IplImage*);

        // Check whether it is NULL or not
        // If it is NULL, release the image, and return
        if(img == 0) {
            printf("Invalid arguments");
            cvReleaseImage(&DispImage);
            return;
        }

        // Find the width and height of the image
        x = img->width;
        y = img->height;

        // Find whether height or width is greater in order to resize the image
        max = (x > y)? x: y;

        // Find the scaling factor to resize the image
        scale = (float) ( (float) max / size );

        // Used to Align the images
        if( i % w == 0 && m!= 20) {
            //m = 20;
            //n+= 20 + size;
			m = 20;
            n+= size-150;
        }
		
        // Set the image ROI to display the current image
        cvSetImageROI(DispImage, cvRect(m, n, (int)( x/scale ), (int)( y/scale )));
		
        // Resize the input image and copy it to the Single Big Image
        cvResize(img, DispImage);

        // Reset the ROI in order to display the next image
        cvResetImageROI(DispImage);
		p1.x = (int)( x/scale );
		p1.y = (int)( y/scale );
		p2.x = m;
		p2.y = n;
		cvRectangle(DispImage, p1, p2, CV_RGB(255,0,0), 3, 8, 0 );

		printf("p1x=%d\np1y=%d\np2x=%d\np2y=%d\n", p1.x, p1.y, p2.x, p2.y);	
    }		 

    // Create a new window, and show the Single Big Image
    cvNamedWindow( title, 1 );
    cvShowImage( title, DispImage);

    cvWaitKey();
    cvDestroyWindow(title);

    // End the number of arguments
    va_end(args);

    // Release the Image Memory
    cvReleaseImage(&DispImage);	
}
*/
void cvShowManyImages(char* title, int nArgs, ...) {

    // img - Used for getting the arguments 
    IplImage *img;
	CvPoint p1, p2, p3;

    // DispImage - the image in which input images are to be copied
    IplImage *DispImage;

    int size;
    int i;
    int m, n;
    int x, y;

    // w - Maximum number of images in a row 
    // h - Maximum number of images in a column 
    int w, h;

    // scale - How much we have to resize the image
    float scale;
    int max;

    // If the number of arguments is lesser than 0 or greater than 12
    // return without displaying 
    if(nArgs <= 0) {
        printf("Number of arguments too small....\n");
        return;
    }
    else if(nArgs > 12) {
        printf("Number of arguments too large....\n");
        return;
    }
    // Determine the size of the image, 
    // and the number of rows/cols 
    // from number of arguments 
    else if (nArgs == 1) {
        w = h = 1;
        size = 300;
    }
    else if (nArgs == 2) {
        w = 2; h = 1;
        size = 300;
    }
    else if (nArgs == 3 || nArgs == 4) {
        w = 3; h = 1;
        size = 300;
    }
    else if (nArgs == 5 || nArgs == 6) {
        w = 3; h = 2;
        size = 200;
    }
    else if (nArgs == 7 || nArgs == 8) {
        w = 4; h = 2;
        size = 200;
    }
    else {
        w = 4; h = 3;
        size = 150;
    }  
	
    // Create a new 3 channel image
    DispImage = cvCreateImage( cvSize(size*w, size*h), 8, 3 );

    // Used to get the arguments passed
    va_list args;
    va_start(args, nArgs);

    // Loop for nArgs number of arguments
    for (i = 0, m = 20, n = 20; i < nArgs; i++, m += (size)) {

        // Get the Pointer to the IplImage
        img = va_arg(args, IplImage*);

        // Check whether it is NULL or not
        // If it is NULL, release the image, and return
        if(img == 0) {
            printf("Invalid arguments");
            cvReleaseImage(&DispImage);
            return;
        }

        // Find the width and height of the image
        x = img->width;
        y = img->height;

        // Find whether height or width is greater in order to resize the image
        max = (x > y)? x: y;

        // Find the scaling factor to resize the image
        scale = (float) ( (float) max / size );

        // Used to Align the images
        if( i % w == 0 && m!= 20) {
            //m = 20;
            //n+= 20 + size;
			m = 20;
			n += y;
        }		
		
        // Set the image ROI to display the current image
        cvSetImageROI(DispImage, cvRect(m, n, (int)( x/scale ), (int)( y/scale )));
		
        // Resize the input image and copy it to the Single Big Image
        cvResize(img, DispImage);

        // Reset the ROI in order to display the next image
        cvResetImageROI(DispImage);						

		/*p1.x = m;
		p1.y = n;
		p2.x = m;
		p2.y = (int)( y/scale )+20;
		

		cvLine(DispImage, p1, p2, CV_RGB(0,0,255), 2, 8, 0);*/

		p1.x = m-20;
		p1.y = n;
		p2.x = m;
		p2.y = (int)( y/scale )+20;
		p3.x = m+20;
		p3.y = n;
		

		cvRectangle(DispImage, p1, p2, CV_RGB(0,0,255), 2, 8, 0);
		cvRectangle(DispImage, p3, p2, CV_RGB(255,0,0), 2, 8, 0);

		/*printf("p1x=%d\np1y=%d\np2x=%d\np2y=%d\n", p1.x, p1.y, p2.x, p2.y);*/	
    }		 		

    // Create a new window, and show the Single Big Image
    cvNamedWindow( title, 1 );
    cvShowImage( title, DispImage);

    cvWaitKey();
    cvDestroyWindow(title);

    // End the number of arguments
    va_end(args);

    // Release the Image Memory
    cvReleaseImage(&DispImage);	
}

void cvBlendImages(IplImage *blend1)
{
	int width, height, rmean, gmean, bmean, rt=0, bt=0, gt=0, clw=0;
	width = blend1->width;
	height = blend1->height;	

	CvScalar s;
	for(int i=0;i<height;i++){
		for(int j=width-20;j<width;j++){
			s = cvGet2D(blend1, i, j);
			rt += s.val[2];
			gt += s.val[1];
			bt += s.val[0];
			clw++;
			//printf("B=%f, G=%f, R=%f\n",s.val[0],s.val[1],s.val[2]);			
		}		
	}
	rmean = rt/clw;
	bmean = bt/clw;
	gmean = gt/clw;

	printf("R mean=%d\nG mean=%d\nB mean=%d\n",rmean, gmean, bmean);
}
// Main function, defines the entry point for the program.
int main( int argc, char** argv )
{
	IplImage *src, *dst, *src1;	
	
	src=cvLoadImage(argv[1],1);
	dst=cvLoadImage(argv[2],1);
	src1=cvLoadImage(argv[3],1);

	cvBlendImages(src);
	//cvShowManyImages("Images", 3, src, dst, src1);	
	
	cvWaitKey();

	cvReleaseImage(&src);
	cvReleaseImage(&dst);
	cvReleaseImage(&src1);

	return 0;
}