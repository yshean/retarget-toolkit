
/*
 *    Header file for the PictureList class
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

#ifndef _PICTURELIST_H_
#define _PICTURELIST_H_


class PictureList {
	private:
		int MinWidth, MinHeight;
		int MaxWidth, MaxHeight;
		int Length;
		char Name[512];
		byte MaxVal;
		bool AllowSave;
		Picture *List;

	public:
		PictureList();
		PictureList(int Width, int Height, int length);
		PictureList(const char *foldername);
		PictureList(const char *foldername, int t_begin, int t_end);

    /* throw FileNotFoundException */
    PictureList(const PictureList &src);
    ~PictureList();

    void ClearAll();

    /* throws FileNotFoundException */
    void LoadPictureList(const char *foldername, int t_begin=-1, int t_end=-1);
	PictureList *TransposePictureList();

    /* throws IndexOutOfBoundsException */
    void SetPixel(int x, int y, int l, pixelType c);

    /* throws IndexOutOfBoundsException */
    void SetPixelIntensity(int x, int y, int l, intensityType c);

    /* throws IndexOutOfBoundsException */
    pixelType GetPixel(int x, int y, int l);

    /* throws IndexOutOfBoundsException */
    intensityType GetPixelIntensity(int x, int y, int l);

    char *GetName() { return Name; }
    void SetName(const char *newName) { strncpy(Name, newName, 512); }

    void AllowedToSave(bool ok);

    bool Inside(int x, int y, int l);
    int GetMinWidth() { return MinWidth; }
	int GetMaxWidth() { return MaxWidth; }
    int GetMinHeight() { return MinHeight; }
	int GetMaxHeight() { return MaxHeight; }
	void SetMinWidth(int minWidth) { MinWidth = minWidth; }
	void SetMaxWidth(int maxWidth) { MaxWidth = maxWidth; }
    void SetMinHeight(int minHeight) { MinHeight = minHeight; }
	void SetMaxHeight(int maxHeight) { MaxHeight = maxHeight; }
	int GetLength() { return Length; }
    byte GetMaxVal() { return MaxVal; }
    
	void SetPicture(int l, Picture *src);
	Picture *GetPicture(int t);

	void SetList(Picture *list, int length);

    void Save(char *foldername);

    PictureList &operator =(PictureList &src);
};

#endif
