
/*
 *    Implementation for the Picture class
 *
 *    Author: Edward Alston Anthony
 *
 */

#include "picture.h"

inline double round( double d )
{
	return floor( d + 0.5 );
}

/*  default constructor - takes a width and a 
 *  height defaultly as 0
 */
Picture::Picture()
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  Image = NULL;
  OriginalImage = NULL;
  Intensity = NULL;

  OriginalWidth = Width = 0;
  OriginalHeight = Height = 0;
  AllowSave = true;
  MaxVal = 255;
  ClearAll();
}

/*  constructor - takes a width and a height 
 *  component for the image
 */
Picture::Picture(int width, int height)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  Image = new pixelType[width * height];
  OriginalImage = new pixelType[width * height];
  Intensity = new intensityType[width * height];

  OriginalWidth = Width = width;
  OriginalHeight = Height = height;
  AllowSave = true;
  MaxVal = 255;
  ClearAll();
}

/*  constructor that loads an image given by filename
 *  into the image space
 */
Picture::Picture(const char *filename)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  Image = NULL;
  OriginalImage = NULL;
  Intensity = NULL;
  AllowSave = false;
  try { LoadPicture(filename); }
  catch (FileNotFoundException ex) { throw FileNotFoundException("Picture", "Picture", filename); }
}

/* copy constructor */
Picture::Picture(const Picture &src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  OriginalWidth = src.OriginalWidth;
  Width = src.Width;
  OriginalHeight = src.OriginalHeight;
  Height = src.Height;
  AllowSave = src.AllowSave;
  MaxVal = src.MaxVal;
  Image = new pixelType[Width * Height];
  OriginalImage = new pixelType[Width * Height];
  Intensity = new intensityType[Width * Height];
  ClearAll();
  SetName(src.Name);

  memcpy((void *) Image, (void *) src.Image, sizeof(pixelType) * Width * Height);
  memcpy((void *) OriginalImage, (void *) src.OriginalImage, sizeof(pixelType) * OriginalWidth * OriginalHeight);
  memcpy((void *) Intensity, (void *) src.Intensity, sizeof(intensityType) * Width * Height);
}

Picture::~Picture()
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  delete OriginalImage;
  delete Image;
  delete Intensity;
}

void Picture::Resize(int width, int height)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if (Image && OriginalImage &&
      (Width > 0) && (Height > 0)) {
    delete OriginalImage;
    delete Image;
    delete Intensity;
  }

  OriginalImage = new pixelType[width * height];
  Image = new pixelType[width * height];
  Intensity = new intensityType[width * height];
  OriginalWidth = Width = width;
  OriginalHeight = Height = height;
  ClearAll();
  MaxVal = 255;
}

/* loads an image from a file */
void Picture::LoadPicture(const char *filename)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  FILE *fl;
  char buf[512] = {'\0'};
  bool done = false;

  AllowSave = false;
  SetName(filename);
  if (Image) {
    delete Image;
    Image = NULL;
  }

  if (OriginalImage) {
    delete OriginalImage;
    OriginalImage = NULL;
  }

  if (Intensity) {
    delete Intensity;
    Intensity = NULL;
  }

  if (!(fl = fopen(filename, "rb")))
    throw FileNotFoundException("Picture", "LoadPicture", filename);

  //fgets(buf, 512, fl);
  while (!done && !feof(fl)) {
    fgets(buf, 512, fl);
    done = true;

    if (strncmp(buf, "#", 1) == 0)
      done = false;
    else {
      sscanf(buf, "%d %d", &OriginalWidth, &OriginalHeight);
      fgets(buf, 512, fl);
      sscanf(buf, "%d", (int *) &MaxVal);
      done = true;
    }
  }

  Width = OriginalWidth;
  Height = OriginalHeight;

  OriginalImage = new pixelType[OriginalWidth * OriginalHeight];
  //int cur_idx = 0;
 // while (!feof(fl))
 // {
	//fscanf(fl, "%c %c %c", &OriginalImage[cur_idx].r, 
	//						&OriginalImage[cur_idx].g,
	//						&OriginalImage[cur_idx].b);
	//cur_idx++;
 // }
  //printf("\n %d",cur_idx);
  fread(OriginalImage, sizeof(pixelType), OriginalWidth * OriginalHeight, fl);
  fclose(fl);

  RestoreOriginal();
}

/* warps the image based on the given matrix */
void Picture::Warp(Matrix *M, bool ignoreTranslation)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  int xp, yp;
  double xMin, xMax, yMin, yMax, D;
  pixelType color;
  Matrix topLeft(3,1);
  Matrix bottomLeft(3, 1);
  Matrix topRight(3, 1);
  Matrix bottomRight(3, 1);
  Matrix *mi = M->Inverse();
  mi->NormalizeBy(mi->Get(3, 3));

  double m[8] = { mi->Get(1, 1), mi->Get(1, 2), mi->Get(1, 3), mi->Get(2, 1),
                  mi->Get(2, 2), mi->Get(2, 3), mi->Get(3, 1), mi->Get(3, 2) };

  topLeft.Set(1, 1, 0);
  topLeft.Set(2, 1, Height);
  topLeft.Set(3, 1, 1);

  bottomLeft.Set(1, 1, 0);
  bottomLeft.Set(2, 1, 0);
  bottomLeft.Set(3, 1, 1);

  topRight.Set(1, 1, Width);
  topRight.Set(2, 1, Height);
  topRight.Set(3, 1, 1);

  bottomRight.Set(1, 1, Width);
  bottomRight.Set(2, 1, 0);
  bottomRight.Set(3, 1, 1);

  topLeft = *M * topLeft;
  bottomLeft = *M * bottomLeft;
  topRight = *M * topRight;
  bottomRight = *M * bottomRight;

  xMin = min(topLeft.Get(1, 1), bottomLeft.Get(1, 1));
  xMin = min(xMin, topRight.Get(1, 1));
  xMin = min(xMin, bottomRight.Get(1, 1));

  xMax = max(topLeft.Get(1, 1), bottomLeft.Get(1, 1));
  xMax = max(xMax, topRight.Get(1, 1));
  xMax = max(xMax, bottomRight.Get(1, 1));

  yMin = min(topLeft.Get(2, 1), bottomLeft.Get(2, 1));
  yMin = min(yMin, topRight.Get(2, 1));
  yMin = min(yMin, bottomRight.Get(2, 1));

  yMax = max(topLeft.Get(2, 1), bottomLeft.Get(2, 1));
  yMax = max(yMax, topRight.Get(2, 1));
  yMax = max(yMax, bottomRight.Get(2, 1));

  Width = (int) (fabs(xMax - xMin));
  Height = (int) (fabs(yMax - yMin));

  delete OriginalImage;
  OriginalImage = Image;
  Image = NULL;
  Image = new pixelType[Width * Height];
  delete Intensity;
  Intensity = NULL;
  Intensity = new intensityType[Width * Height];

  for (int y = (int) floor(yMin); y < (int) ceil(yMax); y++)
    for (int x = (int) floor(xMin); x < (int) ceil(xMax); x++) {
      D = ((m[6] * x) + (m[7] * y) + 1);
      xp = (int) (round(((m[0] * x) + (m[1] * y) + m[2]) / D));
      yp = (int) (round(((m[3] * x) + (m[4] * y) + m[5]) / D));

      if (((xp >= 0) && (xp < OriginalWidth)) && ((yp >=0) && (yp < OriginalHeight))) {
        try {
          color = OriginalImage[(yp * OriginalWidth) + xp];
          if (ignoreTranslation) {
            memcpy((void *) &Image[((y - ((int) floor(yMin))) * Width) + (x - ((int) floor(xMin)))],
                   (void *) &color, sizeof(pixelType));

            Intensity[((y - ((int) floor(yMin))) * Width) + (x - ((int) floor(xMin)))].r = (int) color.r;
            Intensity[((y - ((int) floor(yMin))) * Width) + (x - ((int) floor(xMin)))].g = (int) color.g;
            Intensity[((y - ((int) floor(yMin))) * Width) + (x - ((int) floor(xMin)))].b = (int) color.b;
          }
          else {
            if ((x >= 0) && (x < Width) && (y >= 0) && (y < Height)) {
              memcpy((void *) &Image[(y * Width) + x], (void *) &color, sizeof(pixelType));

              Intensity[(y * Width) + x].r = (int) color.r;
              Intensity[(y * Width) + x].g = (int) color.g;
              Intensity[(y * Width) + x].b = (int) color.b;
            }
          }
        }
        catch (IndexOutOfBoundsException ex) {}
      }
    }
}

/*  if an image was loaded from a file, we store
 *  the original copy of the image in a separate
 *  buffer. This function restores that original
 *  image buffer to the current buffer
 */
void Picture::RestoreOriginal()
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if (Image)
    delete Image;
  if (Intensity)
    delete Intensity;

  Width = OriginalWidth;
  Height = OriginalHeight;
  Image = new pixelType[Width * Height];
  Intensity = new intensityType[Width * Height];
  for (int i = 0; i < Height; i++)
    for (int j = 0; j < Width; j++)
      //try { SetPixelFromScreenClick(j, i, OriginalImage[(i * Width) + j]); }
	  try { SetPixel(j,i,OriginalImage[(i * Width) + j]); }
      catch (IndexOutOfBoundsException ex) {}
}

/* clear the image buffer */
void Picture::Clear()
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  memset((pixelType *) Image, 0, sizeof(pixelType) * Width * Height);
  memset((pixelType *) Intensity, 0, sizeof(intensityType) * Width * Height);
}

/* clear all image buffers */
void Picture::ClearAll()
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  memset((pixelType *) Image, 0, sizeof(pixelType) * Width * Height);
  memset((pixelType *) OriginalImage, 0, sizeof(pixelType) * OriginalWidth * OriginalHeight);
  memset((pixelType *) Intensity, 0, sizeof(intensityType) * Width * Height);
}

/*  Sets the color component of the image at the given point */
void Picture::SetPixel(int x, int y, pixelType c)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if ((x >= Width) || (x < 0) ||
      (y >= Height) || (y < 0))
    throw IndexOutOfBoundsException("Picture", "SetPixel");
  else {
    memcpy((void *) &Image[(y * Width) + x], (void *) &c, sizeof(pixelType));
    Intensity[(y * Width) + x].r = (int) c.r;
    Intensity[(y * Width) + x].g = (int) c.g;
    Intensity[(y * Width) + x].b = (int) c.b;
  }
}

/*  Sets the color component (integer representation - 
 *  See GetPicture() for more details) of the image at
 *  the given point 
 */
void Picture::SetPixelIntensity(int x, int y, intensityType c)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if ((x >= Width) || (x < 0) ||
      (y >= Height) || (y < 0))
    throw IndexOutOfBoundsException("Picture", "SetPixelIntensity");
  else {
    memcpy((void *) &Intensity[(y * Width) + x], (void *) &c, sizeof(intensityType));
    Image[(y * Width) + x].r = min(max(c.r, 0), 255);
    Image[(y * Width) + x].g = min(max(c.g, 0), 255);
    Image[(y * Width) + x].b = min(max(c.b, 0), 255);
  }
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
void Picture::SetPixelFromScreenClick(int x, int y, pixelType c)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  int screeny = Height - (y + 1);

  if ((x >= Width) || (x < 0) ||
      (y >= Height) || (y < 0))
    throw IndexOutOfBoundsException("Picture", "SetPixelFromScreenClick");
  else {
    memcpy((void *) &Image[(screeny * Width) + x], (void *) &c, sizeof(pixelType));
    Intensity[(screeny * Width) + x].r = (int) c.r;
    Intensity[(screeny * Width) + x].g = (int) c.g;
    Intensity[(screeny * Width) + x].b = (int) c.b;
  }
}

/* returns the color component at the given point */
pixelType Picture::GetPixel(int x, int y)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if ((x >= Width) || (x < 0) ||
      (y >= Height) || (y < 0))
    throw IndexOutOfBoundsException("Picture", "GetPixel");
  else
    return Image[(y * Width) + x];
}

/*  returns the integer representation of the pixel's
 *  color component at the given point (see the function
 *  GetPicture() for more details)
 */
intensityType Picture::GetPixelIntensity(int x, int y)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if ((x >= Width) || (x < 0) ||
      (y >= Height) || (y < 0))
    throw IndexOutOfBoundsException("Picture", "GetPixelIntensity");
  else
    return Intensity[(y * Width) + x];
}

bool Picture::Inside(int x, int y)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  return (((x >= 0) && (x < Width)) && ((y >=0) && (y < Height)));
}

/* draws a line using the DDA algorithm */
void Picture::Line(int xa, int ya, int xb, int yb, pixelType color)
{
  /*  DDA Algorithm taken directly from page 88 of
   *  Computer Graphics - C Version (Donald Hearn, M. Pauline Baker), 2nd Edition
   */
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  int dx = xb - xa, dy = yb - ya, steps, k;
  float xIncrement, yIncrement, x = xa, y = ya;

  if (abs(dx) > abs(dy)) steps = abs(dx);
  else steps = abs(dy);
  xIncrement = dx / (float) steps;
  yIncrement = dy / (float) steps;

  try {
    SetPixel(xa, ya, color);
    SetPixel(xb, yb, color);
  }
  catch (IndexOutOfBoundsException ex) {}
  for (k = 0; k < steps; k++) {
    x += xIncrement;
    y += yIncrement;
    try { SetPixel((int) round(x), (int) round(y), color); }
    catch (IndexOutOfBoundsException ex) {}
  }
}

/* save the image to filename */
void Picture::Save(char *filename)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if (!AllowSave) {
    cerr << "Save has been disabled for this image\n";
    return;
  }

  pixelType *temp = new pixelType[Width * Height];
  for (int i = 0; i < Height; i++)
    for (int j = 0; j < Width; j++)
      //try { temp[((Height - i - 1) * Width) + j] = GetPixel(j, i); }
	  try { temp[(i * Width) + j] = GetPixel(j, i); }
      catch (IndexOutOfBoundsException ex) {}

  FILE *fl = fopen(filename, "wb");
  fprintf(fl, "P6\n%d %d\n%d\n", Width, Height, (int) MaxVal);
  fwrite(temp, sizeof(pixelType), Width * Height, fl);  
  fclose(fl);
  delete temp;
}

/* overloading of the equals operator */
Picture &Picture::operator =(Picture &src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  ClearAll();
  OriginalWidth = src.OriginalWidth;
  OriginalHeight = src.OriginalHeight;
  Width = src.Width;
  Height = src.Height;
  MaxVal = src.MaxVal;
  strncpy(Name, src.Name, 512);
  AllowSave = src.AllowSave;

  if (this->OriginalImage)
    delete this->OriginalImage;
  if (this->Image)
    delete this->Image;
  if (this->Intensity)
    delete this->Intensity;

  OriginalImage = new pixelType[OriginalWidth * OriginalHeight];
  Image = new pixelType[Width * Height];
  Intensity = new intensityType[Width * Height];

  memcpy((void *) OriginalImage, (void *) src.OriginalImage,
         sizeof(pixelType) * OriginalWidth * OriginalHeight);
  memcpy((void *) Image, (void *) src.Image,
         sizeof(pixelType) * Width * Height);
  memcpy((void *) Intensity, (void *) src.Intensity,
         sizeof(intensityType) * Width * Height);

  return *this;
}

/* overloading of the addition operator */
Picture &Picture::operator +(Picture &src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if ((this->Height != src.Height) ||
      (this->Width != src.Width))
    throw IncompatibleDimensionsException("Picture", "operator +");

  Picture *result = new Picture(Width, Height);

  for (int i = 0; i < src.Width * src.Height; i++) {
    result->OriginalImage[i].r = min(OriginalImage[i].r + src.OriginalImage[i].r, 255);
    result->OriginalImage[i].g = min(OriginalImage[i].g + src.OriginalImage[i].g, 255);
    result->OriginalImage[i].b = min(OriginalImage[i].b + src.OriginalImage[i].b, 255);

    result->Image[i].r = min(Image[i].r + src.Image[i].r, 255);
    result->Image[i].g = min(Image[i].g + src.Image[i].g, 255);
    result->Image[i].b = min(Image[i].b + src.Image[i].b, 255);

    result->Intensity[i].r = Intensity[i].r + src.Intensity[i].r;
    result->Intensity[i].g = Intensity[i].g + src.Intensity[i].g;
    result->Intensity[i].b = Intensity[i].b + src.Intensity[i].b;
  }

  return *result;
}

/* overloading of the addition operator */
Picture &Picture::operator -(Picture &src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if ((this->Height != src.Height) ||
      (this->Width != src.Width))
    throw IncompatibleDimensionsException("Picture", "operator -");

  Picture *result = new Picture(Width, Height);

  for (int i = 0; i < src.Width * src.Height; i++) {
    result->OriginalImage[i].r = min(OriginalImage[i].r - src.OriginalImage[i].r, 255);
    result->OriginalImage[i].g = min(OriginalImage[i].g - src.OriginalImage[i].g, 255);
    result->OriginalImage[i].b = min(OriginalImage[i].b - src.OriginalImage[i].b, 255);

    result->Image[i].r = min(Image[i].r - src.Image[i].r, 255);
    result->Image[i].g = min(Image[i].g - src.Image[i].g, 255);
    result->Image[i].b = min(Image[i].b - src.Image[i].b, 255);

    result->Intensity[i].r = Intensity[i].r - src.Intensity[i].r;
    result->Intensity[i].g = Intensity[i].g - src.Intensity[i].g;
    result->Intensity[i].b = Intensity[i].b - src.Intensity[i].b;
  }

  return *result;
}


/* overloading of the division operator */
Picture &Picture::operator /=(int factor)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if (factor == 0) return *this;

  for (int i = 0; i < Width * Height; i++) {
    this->Image[i].r = Image[i].r / factor;
    this->Image[i].g = Image[i].g / factor;
    this->Image[i].b = Image[i].b / factor;

    this->Intensity[i].r = Intensity[i].r / factor;
    this->Intensity[i].g = Intensity[i].g / factor;
    this->Intensity[i].b = Intensity[i].b / factor;
  }

  return *this;
}

/* sets if the image is read only, read/write */
void Picture::AllowedToSave(bool ok)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  AllowSave = ok;
}

/*  When computing the Gaussian and Laplacian pyramids,
 *  a lot of additions and subtractions takes place on
 *  the different layers of the pyramids. Since a each
 *  of the RGB components can hold an integer value of
 *  at most 255, this limits the "exact" computation
 *  because if the intensity of any of the RGB components
 *  exceeds 255 or drops below 0 in the process of the
 *  pyramid construction, this could introduce a LOT
 *  of error in the final output. Therefore, we use
 *  a bunch of integer values to represent the intensity
 *  of each individual components, and scale it down
 *  to a range of 0 to 255 AFTER all computations have
 *  been completed.
 */
#ifdef USE_INTENSITY
pixelType *Picture::GetPicture()
{
  for (int i = 0; i < Width * Height; i++) {
    Image[i].r = min(max(Intensity[i].r, 0), 255);
    Image[i].g = min(max(Intensity[i].g, 0), 255);
    Image[i].b = min(max(Intensity[i].b, 0), 255);
  }
  return Image;
}
#endif
