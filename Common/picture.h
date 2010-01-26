
/*
 *    Header file for the Picture class
 *
 *    A wrapper class for the ppm library
 *
 *    Author: Edward Alston Anthony
 *
 */

#include <string>
#include <iostream>
#include <stdio.h>
#include <math.h>

#include "exceptions.h"
#include "matrix.h"

#ifdef USE_TRACEBACK
  #include "traceback.h"
  extern Traceback *Trace;
#endif

#ifndef min
  #define min(a, b) (a < b ? a : b)
#endif

#ifndef max
  #define max(a, b) (a > b ? a : b)
#endif


#ifndef _PICTURE_H_
#define _PICTURE_H_

#define USE_INTENSITY
typedef unsigned char byte;

typedef struct {  
  float x, y;   
} pointType;

typedef struct {
  byte r, g, b;
} pixelType;

typedef struct {
  int r, g, b;
} intensityType;

class Picture {
  private:
    pixelType *OriginalImage;
    pixelType *Image;
    byte MaxVal;
    int Width, Height;
    int OriginalWidth, OriginalHeight;
    char Name[512];
    bool AllowSave;
    intensityType *Intensity;

  public:
	Picture();
    Picture(int width, int height);
    Picture(const char *filename);

    /* throw FileNotFoundException */
    Picture(const Picture &src);
    ~Picture();

    void Warp(Matrix *M, bool ignoreTranslation);
    void Resize(int width, int height);
    void RestoreOriginal();
    void Clear();
    void ClearAll();

    /* throws FileNotFoundException */
    void LoadPicture(const char *filename);

    /* throws IndexOutOfBoundsException */
    void SetPixel(int x, int y, pixelType c);

    /* throws IndexOutOfBoundsException */
    void SetPixelIntensity(int x, int y, intensityType c);

    /* throws IndexOutOfBoundsException */
    void SetPixelFromScreenClick(int x, int y, pixelType c);

    /* throws IndexOutOfBoundsException */
    pixelType GetPixel(int x, int y);

    /* throws IndexOutOfBoundsException */
    intensityType GetPixelIntensity(int x, int y);

    char *GetName() { return Name; }
    void SetName(const char *newName) { strncpy(Name, newName, 512); }

    void AllowedToSave(bool ok);

    void Line(int xa, int ya, int xb, int yb, pixelType color);

    bool Inside(int x, int y);
    int GetWidth() { return Width; }
    int GetHeight() { return Height; }
    byte GetMaxVal() { return MaxVal; }
    #ifndef USE_INTENSITY
      pixelType *GetPicture() { return Image; }
    #else
      pixelType *GetPicture();
    #endif

    void Save(char *filename);

    Picture &operator =(Picture &src);
    Picture &operator /=(int factor);

    /* throws IncompatibleDimensionsException */
    Picture &operator +(Picture &src);

	/* throws IncompatibleDimensionsException */
	Picture &operator -(Picture &src);
};

#endif
