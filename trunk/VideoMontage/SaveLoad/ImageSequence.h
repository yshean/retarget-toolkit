#pragma once

#include <iostream>
#include <stdio.h>
#include <sstream>
#include <string> 
#include <time.h>
#include <windows.h> 
#include <conio.h>
#include <cv.h> 
#include <highgui.h> 
#include "VideoSequence.h"

using std::stringstream;
using std::string;

class CImageSequence
{
	
	string sDirectoryFiles;
	string sfileExtension;

public:
	CImageSequence(void);
	CImageSequence(string fileDirectory);
	CImageSequence(string fileDirectory, string fileExtension);
	~CImageSequence(void);

	//Header declarations
	void listingFile(VideoSequence* videoFrameSequence);
	void setDirectory(string fileDirectory);
	string getDirectory();
	string getfileExtension();
	
	string convertWCharArrayToString(char *wcharArray);

	void launchQuickView(void);

};
