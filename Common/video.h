
/*
 *    Header file for the Video class
 *    Author: Hu Yiqun
 */

#include <string>
#include <iostream>
#include <stdio.h>
#include <math.h>

#include "exceptions.h"
#include "picture.h"

#ifdef USE_TRACEBACK
  #include "traceback.h"
  extern Traceback *Trace;
#endif

#ifndef _VIDEO_H_
#define _VIDEO_H_


class Video {
	private:
		int Width, Height, Time;
		char Name[512];
		byte MaxVal;
		bool AllowSave;
		Picture *Frames;

	public:
		Video();
		Video(int width, int height, int time);
		Video(const char *foldername);
		Video(const char *foldername, int t_begin, int t_end);

    /* throw FileNotFoundException */
    Video(const Video &src);
    ~Video();

    void Resize(int width, int height, int time);
    void ClearAll();

    /* throws FileNotFoundException */
    void LoadVideo(const char *foldername, int t_begin=-1, int t_end=-1);

    /* throws IndexOutOfBoundsException */
    void SetPixel(int x, int y, int t, pixelType c);

    /* throws IndexOutOfBoundsException */
    void SetPixelIntensity(int x, int y, int t, intensityType c);

	/* throws IndexOutOfBoundsException */
	void SetPixelFromScreenClick(int x, int y, int t, pixelType c);

    /* throws IndexOutOfBoundsException */
    pixelType GetPixel(int x, int y, int t);

    /* throws IndexOutOfBoundsException */
    intensityType GetPixelIntensity(int x, int y, int t);

    char *GetName() { return Name; }
    void SetName(const char *newName) { strncpy(Name, newName, 512); }

    void AllowedToSave(bool ok);

    bool Inside(int x, int y, int t);
    int GetWidth() { return Width; }
    int GetHeight() { return Height; }
	int GetTime() { return Time; }
    byte GetMaxVal() { return MaxVal; }
    
	void SetFrame(int t, Picture *src);
	Picture *GetFrame(int t);

	void SetFrames(Picture *frames, int time);

    void Save(char *foldername);

    Video &operator =(Video &src);
};

#endif
