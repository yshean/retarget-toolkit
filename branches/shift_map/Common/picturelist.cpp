
/*
 *    Implementation for the PictureList class
 *
 *    Author: Hu Yiqun
 *
 */

#include <assert.h>
#include <string>
#include <vector>
#include "utils.h"
#include "picture.h"
#include "picturelist.h"

using namespace std;

/*  default constructor - takes a width, a 
 *  height and a time defaultly as 0
 */
PictureList::PictureList()
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	List = NULL;
	MinWidth = 0;
	MinHeight = 0;
	MaxWidth = 0;
	MaxHeight = 0;
	Length = 0;
	AllowSave = true;
	MaxVal = 255;
	ClearAll();
}

PictureList::PictureList(int Width, int Height, int length)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	List = new Picture[length];

	this->MinWidth = this->MaxWidth = Width;
	this->MinHeight = this->MaxHeight = Height;
	this->Length = length;
	AllowSave = true;
	MaxVal = 255;
	//ClearAll();
}


/*  constructor that loads a video given 
 *	by foldername into the memory
 */
PictureList::PictureList(const char *foldername)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	List = NULL;
	SetName(foldername);
	AllowSave = false;
  	try { 	  
		LoadPictureList(foldername); 
	}
	catch (FolderNotFoundException ex) 
	{ 
		throw FolderNotFoundException("PictureList", "PictureList", foldername); 
	} //end try
}

PictureList::PictureList(const char *foldername, int t_begin, int t_end)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	List = NULL;
	SetName(foldername);
	AllowSave = false;
	try { 
		LoadPictureList(foldername,t_begin,t_end);
	}
	catch (FolderNotFoundException ex) 
	{ 
		throw FolderNotFoundException("PictureList", "PictureList", foldername);  
	} // end try
}

/* copy constructor */
PictureList::PictureList(const PictureList &src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  MinWidth = src.MinWidth;
  MinHeight = src.MinHeight;
  MaxWidth = src.MaxWidth;
  MaxHeight = src.MaxHeight;
  AllowSave = src.AllowSave;
  MaxVal = src.MaxVal;
  ClearAll();
  SetName(src.Name);

  List = new Picture[src.Length];
  for (int i=0; i<src.Length; i++)
  {
	  List[i] = src.List[i];
	  List[i].SetName(src.List[i].GetName());
  }
}

PictureList::~PictureList()
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	delete [] List;
}

/* loads an image from a file */
void PictureList::LoadPictureList(const char *foldername, int t_begin, int t_end)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	AllowSave = false;
	SetName(foldername);
	if (List) {
		delete [] List;
		List = NULL;
	}

	vector<string> filenames = Get_FrameNames(foldername, 
											  PICTURE_FRAME_EXT);
	if (filenames.empty())
		throw FolderNotFoundException("PictureList", "LoadPictureList", foldername);

	// handle default values for t_begin and t_end
	t_begin = (t_begin>=0) ? t_begin : 0;
	t_end = (t_end>=0) ? t_end : filenames.size()-1;

	// ensure the begin and end frames are valid
	assert((t_begin<=t_end) && (t_end<filenames.size()));

	List = new Picture[t_end-t_begin+1];
	this->MinWidth = -1; this->MinHeight = -1;
	this->MaxWidth = -1; this->MaxHeight = -1;
	for (int i = t_begin; i <= t_end; i++)
	{
		printf("%d %s\n",i,filenames.at(i).c_str());
		string imagefile = foldername+filenames.at(i);
		//printf("%s\n",framefile.c_str());
		List[i-t_begin].LoadPicture(imagefile.c_str());
		List[i-t_begin].SetName(filenames.at(i).c_str());
		List[i-t_begin].AllowedToSave(true);

		this->MinWidth = ((List[i-t_begin].GetWidth()<this->MinWidth) || (this->MinWidth==-1)) ? List[i-t_begin].GetWidth() : this->MinWidth;
		this->MaxWidth = ((List[i-t_begin].GetWidth()>this->MaxWidth) || (this->MaxWidth==-1)) ? List[i-t_begin].GetWidth() : this->MaxWidth;
		this->MinHeight = ((List[i-t_begin].GetHeight()<this->MinHeight) || (this->MinHeight==-1)) ? List[i-t_begin].GetHeight() : this->MinHeight;
		this->MaxHeight = ((List[i-t_begin].GetHeight()>this->MaxHeight) || (this->MaxHeight==-1)) ? List[i-t_begin].GetHeight() : this->MaxHeight;
	}

	Length = t_end-t_begin+1;
	AllowSave = true;
	MaxVal = 255;
}

PictureList *PictureList::TransposePictureList()
{
	int width = this->GetPicture(0)->GetWidth();
	int height = this->GetPicture(0)->GetHeight();
	int time = this->GetLength();
	PictureList *tvid = new PictureList(height,width,time);
	tvid->SetName(this->GetName());

	Picture *frames = new Picture[time];
	for (int tt=0; tt<time; tt++)
	{
		Picture *frame = new Picture(height,width);
		frames[tt] = *(frame);
		delete frame;
		frames[tt].SetName(this->GetPicture(tt)->GetName());
	}
	tvid->SetList(frames,time);

	int x, y, t;
	pixelType pixel;

	for ( int  i = 0; i < width*height*time; i++ )
	{
		t = i / (width*height);
		x = (i % (width*height)) % width;
		y = (i % (width*height)) / width;
		
		pixel.r = this->GetPixel(x,y,t).r;
		pixel.g = this->GetPixel(x,y,t).g;
		pixel.b = this->GetPixel(x,y,t).b;
		
		tvid->SetPixel(y,x,t,pixel);
	}
	return tvid;
}


/* clear all image buffers */
void PictureList::ClearAll()
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  for (int i=0; i<Length; i++)
  {
	  List[i].ClearAll();
  }
}

/*  Sets the color component of the image at the given point */
void PictureList::SetPixel(int x, int y, int l, pixelType c)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	if ((l >= Length) || (l < 0))
		throw IndexOutOfBoundsException("PictureList", "SetPixel");
	else
		if ((x >= List[l].GetWidth()) || (x < 0) ||
			(y >= List[l].GetHeight()) || (y < 0))
			throw IndexOutOfBoundsException("PictureList", "SetPixel");
		else
			List[l].SetPixel(x,y,c);
}

/*  Sets the color component (integer representation - 
 *  See GetPicture() for more details) of the image at
 *  the given point 
 */
void PictureList::SetPixelIntensity(int x, int y, int l, intensityType c)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	if ((l >= Length) || (l < 0))
		throw IndexOutOfBoundsException("PictureList", "SetPixelIntensity");
	else
		if ((x >= List[l].GetWidth()) || (x < 0) ||
			(y >= List[l].GetHeight()) || (y < 0))
			throw IndexOutOfBoundsException("PictureList", "SetPixelIntensity");
		else
			List[l].SetPixelIntensity(x,y,c);

}

/* returns the color component at the given point */
pixelType PictureList::GetPixel(int x, int y, int l)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	if ((l >= Length) || (l < 0))
		throw IndexOutOfBoundsException("PictureList", "GetPixel");
	else
		if ((x >= List[l].GetWidth()) || (x < 0) ||
			(y >= List[l].GetHeight()) || (y < 0))
			throw IndexOutOfBoundsException("PictureList", "GetPixel");
		else
			return List[l].GetPixel(x,y);
}

/*  returns the integer representation of the pixel's
 *  color component at the given point (see the function
 *  GetPicture() for more details)
 */
intensityType PictureList::GetPixelIntensity(int x, int y, int l)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	if ((l >= Length) || (l < 0))
		throw IndexOutOfBoundsException("PictureList", "GetPixel");
	else
		if ((x >= List[l].GetWidth()) || (x < 0) ||
			(y >= List[l].GetHeight()) || (y < 0))
			throw IndexOutOfBoundsException("PictureList", "GetPixelIntensity");
		else
			return List[l].GetPixelIntensity(x,y);
}

bool PictureList::Inside(int x, int y, int l)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	if ((l >= Length) || (l < 0))
		return false;
	else
		if ((x >= List[l].GetWidth()) || (x < 0) ||
			(y >= List[l].GetHeight()) || (y < 0))
			return false;
		else
			return true;
}

/* save the image to filename */
void PictureList::Save(char *foldername)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	if (!AllowSave) {
		cerr << "Save has been disabled for this image\n";
		return;
	}

	for (int i = 0; i < Length; i++)
	{
		char framename[512] = {'\0'};
		char buf[512] = {'\0'};

		strcat(framename,foldername);
		strcat(framename,List[i].GetName());
		List[i].Save(framename);
	}
}

/* overloading of the equals operator */
PictureList &PictureList::operator =(PictureList &src)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	ClearAll();
	MinWidth = src.GetMinWidth();
	MinHeight = src.GetMinHeight();
	MaxWidth = src.GetMaxWidth();
	MaxHeight = src.GetMaxHeight();
	Length = src.GetLength();
	MaxVal = src.GetMaxVal();
	strncpy(Name, src.Name, 512);
	AllowSave = src.AllowSave;  

	if (this->List)
    delete [] this->List;

	List = new Picture[src.Length];
	for (int i=0; i<src.Length; i++)
	{
		List[i] = src.List[i];
		List[i].SetName(src.List[i].GetName());
	}

  return *this;
}

/* sets if the video is read only, read/write */
void PictureList::AllowedToSave(bool ok)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	AllowSave = ok;
}

void PictureList::SetList(Picture *list, int length)
{
	ClearAll();
	if (List)		
		delete [] List;
	
	List = list;
	this->Length = length;
}

void PictureList::SetPicture(int l, Picture *src)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	try
	{
		List[l] = *src;
		MinWidth = ((src->GetWidth()<MinWidth) || (MinWidth==-1)) ? src->GetWidth() : MinWidth;
		MaxWidth = ((src->GetWidth()>MaxWidth) || (MaxWidth==-1)) ? src->GetWidth() : MaxWidth;
		MinHeight = ((src->GetHeight()<MinHeight) || (MinHeight==-1)) ? src->GetHeight() : MinHeight;
		MaxHeight = ((src->GetHeight()>MaxHeight) || (MaxHeight==-1)) ? src->GetHeight() : MaxHeight;		
	}
	catch (IndexOutOfBoundsException ex) {}
}

Picture *PictureList::GetPicture(int l)
{
#ifdef USE_TRACEBACK
	Trace->Add(__FILE__, __LINE__);
#endif
	try
	{
		return &(List[l]);
	}
	catch (IndexOutOfBoundsException ex) {}
}