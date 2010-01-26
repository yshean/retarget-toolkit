
/*
 *    Implementation for the Video class
 *
 *    Author: Hu Yiqun
 *
 */

#include <assert.h>
#include <string>
#include <vector>
#include "utils.h"
#include "picture.h"
#include "video.h"

using namespace std;

/*  default constructor - takes a width, a 
 *  height and a time defaultly as 0
 */
Video::Video()
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
	Frames = NULL;
	Width = 0;
	Height = 0;
	Time = 0;
	AllowSave = true;
	MaxVal = 255;
	ClearAll();
}

/*  constructor - takes a width, a height 
 *  and a time component for the video
 */
Video::Video(int width, int height, int time)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  Frames = new Picture[time];

  Width = width;
  Height = height;
  Time = time;
  AllowSave = true;
  MaxVal = 255;
  ClearAll();
}

/*  constructor that loads a video given 
 *	by foldername into the memory
 */
Video::Video(const char *foldername)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	Frames = NULL;
	SetName(foldername);
	AllowSave = false;
	try { 	  
		LoadVideo(foldername); 
	}
	catch (FolderNotFoundException ex) 
	{ 
		throw FolderNotFoundException("Video", "Video", foldername); 
	} //end try
}

Video::Video(const char *foldername, int t_begin, int t_end)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	Frames = NULL;
	SetName(foldername);
	AllowSave = false;
	try { 
		LoadVideo(foldername,t_begin,t_end); 
	}
	catch (FolderNotFoundException ex) 
	{ 
		throw FolderNotFoundException("Video", "Video", foldername); 
	} // end try
}

/* copy constructor */
Video::Video(const Video &src)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	Width = src.Width;
	Height = src.Height;
	AllowSave = src.AllowSave;
	MaxVal = src.MaxVal;
	ClearAll();
	SetName(src.Name);

	Frames = new Picture[src.Time];
	for (int i=0; i<src.Time; i++)
	{
		Frames[i] = src.Frames[i];
	}
}

Video::~Video()
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	delete [] Frames;
}

void Video::Resize(int width, int height, int time)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif

}

/* loads an image from a file */
void Video::LoadVideo(const char *foldername, int t_begin, int t_end)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	if (Frames) {
		delete [] Frames;
		Frames = NULL;
	}


	vector<string> framenames = Get_FrameNames(foldername, 
											   VIDEO_FRAME_EXT);
		
	if (framenames.empty())
		throw FolderNotFoundException("Video", "LoadVideo", foldername);

	// handle default values for t_begin and t_end
	t_begin = (t_begin>=0) ? t_begin : 0;
	t_end = (t_end>=0) ? t_end : framenames.size()-1;

	// ensure the begin and end frames are valid
	assert((t_begin<=t_end) && (t_end<framenames.size()));

	Frames = new Picture[t_end-t_begin+1];
	for (int i = t_begin; i <= t_end; i++)
	{
		printf("%d %s\n",i,framenames.at(i).c_str());
		string framefile = foldername+framenames.at(i);
		//printf("%s\n",framefile.c_str());
		Frames[i-t_begin].LoadPicture(framefile.c_str());
		Frames[i-t_begin].SetName(framenames.at(i).c_str());
		Frames[i-t_begin].AllowedToSave(true);
	}

	Width = Frames[0].GetWidth();
	Height = Frames[0].GetHeight();
	Time = t_end-t_begin+1;
	AllowSave = true;
	MaxVal = 255;

}


/* clear all image buffers */
void Video::ClearAll()
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  for (int i=0; i<Time; i++)
  {
	  Frames[i].ClearAll();
  }
}

/*  Sets the color component of the image at the given point */
void Video::SetPixel(int x, int y, int t, pixelType c)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if ((x >= Width) || (x < 0) ||
      (y >= Height) || (y < 0) ||
	  (t >= Time) || (t < 0))
    throw IndexOutOfBoundsException("Video", "SetPixel");
  else
	  Frames[t].SetPixel(x,y,c);
}

/*  Sets the color component (integer representation - 
 *  See GetPicture() for more details) of the image at
 *  the given point 
 */
void Video::SetPixelIntensity(int x, int y, int t, intensityType c)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if ((x >= Width) || (x < 0) ||
      (y >= Height) || (y < 0) ||
	  (t >= Time) || (t < 0))
    throw IndexOutOfBoundsException("Video", "SetPixelIntensity");
  else
	Frames[t].SetPixelIntensity(x,y,c);

}

/*  OpenGL's coordinate system has the y-axis at the top.
 *  We want to simulate an image space that has the regular
 *  Cartesian coordinate system, with the origin at the bottom
 *  left corner of the screen
 *
 *  This function takes an OpenGL coordinate and transforms it
 *  to our image space, setting the color component of that location
 *  with the given color
 */
void Video::SetPixelFromScreenClick(int x, int y, int t, pixelType c)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  int screeny = Height - (y + 1);

  if ((x >= Width) || (x < 0) ||
      (y >= Height) || (y < 0) ||
	  (t >= Time) || (t < 0))
    throw IndexOutOfBoundsException("Video", "SetPixelFromScreenClick");
  else
	Frames[t].SetPixelFromScreenClick(x,y,c);

}

/* returns the color component at the given point */
pixelType Video::GetPixel(int x, int y, int t)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if ((x >= Width) || (x < 0) ||
      (y >= Height) || (y < 0) ||
	  (t >= Time) || (t < 0))
    throw IndexOutOfBoundsException("Video", "GetPixel");
  else
	  return Frames[t].GetPixel(x,y);
}

/*  returns the integer representation of the pixel's
 *  color component at the given point (see the function
 *  GetPicture() for more details)
 */
intensityType Video::GetPixelIntensity(int x, int y, int t)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if ((x >= Width) || (x < 0) ||
      (y >= Height) || (y < 0) ||
	  (t >= Time) || (t < 0))
    throw IndexOutOfBoundsException("Video", "GetPixelIntensity");
  else
	  return Frames[t].GetPixelIntensity(x,y);
}

bool Video::Inside(int x, int y, int t)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  return (((x >= 0) && (x < Width)) && 
		  ((y >= 0) && (y < Height)) &&
		  ((t >= 0) && (t < Time)));
}

/* save the image to filename */
void Video::Save(char *foldername)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if (!AllowSave) {
    cerr << "Save has been disabled for this image\n";
    return;
  }

  for (int i = 0; i < Time; i++)
  {
	  char framename[512] = {'\0'};
	  char buf[512] = {'\0'};

	  strcat(framename,foldername);
	  strcat(framename,itoa(i,buf,10));
	  strcat(framename,".ppm");
	  Frames[i].Save(framename);
  }
  //pixelType *temp = new pixelType[Width * Height];
  //for (int i = 0; i < Height; i++)
  //  for (int j = 0; j < Width; j++)
  //    //try { temp[((Height - i - 1) * Width) + j] = GetPixel(j, i); }
	 // try { temp[(i * Width) + j] = GetPixel(j, i); }
  //    catch (IndexOutOfBoundsException ex) {}

  //FILE *fl = fopen(filename, "wb");
  //fprintf(fl, "P6\n%d %d\n%d\n", Width, Height, (int) MaxVal);
  //fwrite(temp, sizeof(pixelType), Width * Height, fl);  
  //fclose(fl);
  //delete temp;
}

/* overloading of the equals operator */
Video &Video::operator =(Video &src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  ClearAll();
  Width = src.Width;
  Height = src.Height;
  Time = src.Time;
  MaxVal = src.MaxVal;
  strncpy(Name, src.Name, 512);
  AllowSave = src.AllowSave;  

  if (this->Frames)
    delete [] this->Frames;

  Frames = new Picture[src.Time];
  for (int i=0; i<src.Time; i++)
  {
	  Frames[i] = src.Frames[i];
  }

  return *this;
}

/* sets if the video is read only, read/write */
void Video::AllowedToSave(bool ok)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  AllowSave = ok;
}

void Video::SetFrames(Picture *frames, int time)
{
	ClearAll();
	if (Frames)		
		delete [] Frames;
	
	Frames = frames;
	this->Time = time;
}

void Video::SetFrame(int t, Picture *src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
	try
	{
		Frames[t] = *src;
	}
	catch (IndexOutOfBoundsException ex) {}
}

Picture *Video::GetFrame(int t)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  try
  {
	return &(Frames[t]);
  }
  catch (IndexOutOfBoundsException ex) {}
}
