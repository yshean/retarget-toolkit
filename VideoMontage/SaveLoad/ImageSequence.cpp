#include "stdafx.h"
#include "ImageSequence.h"
#include "VideoSequence.h"

//constructor
CImageSequence::CImageSequence(void)
{
	sDirectoryFiles = "C:\\Java\\File\\Basketball\\RetargetBMP\\";
}


//constructor with directory folder initialization
CImageSequence::CImageSequence(string fileDirectory)
{
	sDirectoryFiles = fileDirectory;
}

//constructor with directory folder initialization and extention
CImageSequence::CImageSequence(string fileDirectory, string fileExtension)
{
	sDirectoryFiles = fileDirectory;
	sfileExtension = fileExtension;
}

//Destructor
CImageSequence::~CImageSequence(void)
{
}

//Search , list, and load files in target folder into cvloadimage
void CImageSequence::listingFile(VideoSequence* videoFrameSequence)
{
    WIN32_FIND_DATAA findFileData;
	char path[255];  
	char* filename = NULL;
	IplImage* src;
	//VideoSequence*  videoFrameSequence;
	vector<string> frameList;
	int i = 0;

	strcpy(path,(char*)(getDirectory()+ getfileExtension()).c_str());
	    
	HANDLE hFind = FindFirstFileA((LPCSTR)path, &findFileData);
	
    if(hFind  == INVALID_HANDLE_VALUE) {
        std::cout <<"No files found." <<std::endl;
		std::cout << GetLastError() <<std::endl;
    } else {
        std::cout <<"Files found." <<std::endl;
    }
    
    int fileNumber = 0;
    std::cout <<fileNumber <<":" <<convertWCharArrayToString(findFileData.cFileName) <<std::endl;

	//load to first frame list vector
	frameList.push_back(convertWCharArrayToString(findFileData.cFileName).c_str());

	while(FindNextFileA(hFind, &findFileData)) {

		
        fileNumber++;
        std::cout <<fileNumber <<":" <<convertWCharArrayToString(findFileData.cFileName) <<std::endl;
		
		//load to frame list vector
		frameList.push_back(convertWCharArrayToString(findFileData.cFileName).c_str());
	}
 
	// Read files
	FindClose(hFind);	

	videoFrameSequence->frameList = new vector<char*>(fileNumber + 1);
	 
	for(int i = 0; i < fileNumber + 1; i++)
	{
		char* temp;
		(*videoFrameSequence->frameList)[i] = new char(200);
	}

	// Store list to video sequence
	
	for(i=0; i<=fileNumber; i++)
	{
		strcpy((*videoFrameSequence->frameList)[i], (((char*) (frameList.at(i)).c_str()  )));
		//(*videoFrameSequence->frameList)[i] = (char*) (frameList.at(i)).c_str();
	}
		//videoFrameSequence->frameList[i] = (char*) (frameList.at(i)).c_str();
	videoFrameSequence->frameList->resize(fileNumber);
	
}


//internal function to convert char to string
string CImageSequence::convertWCharArrayToString(CHAR * wcharArray) 
{
    
	stringstream ss;
 
    int i = 0;
    char c = (char) wcharArray[i];

	while(c != '\0') {
        ss <<c;
        i++;
        c = (char) wcharArray[i];
    }
 
    string convert = ss.str();
    
	return convert;

}


// Set target directory 
void CImageSequence::setDirectory(string fileDirectory)
{
	sDirectoryFiles = fileDirectory;
}

// Get target directory
string CImageSequence::getDirectory()
{
	return sDirectoryFiles;
}

// Get file extension
string CImageSequence::getfileExtension()
{
	return sfileExtension;
}

// Launch Quick View
void CImageSequence::launchQuickView()
{
	string sConvFile = "C:\\Java\\App\\QuickView.exe ";
	string sFullExecCmd = "";
	
	sFullExecCmd += sConvFile;
	sFullExecCmd += getDirectory();
	int length = sFullExecCmd.length();
	char *pFullExecCmd = new char[ length + 1 ]; // including null

	sFullExecCmd.copy( pFullExecCmd, length, 0 ); // copy string1 to ptr2 char*
	pFullExecCmd[ length ] = '\0'; // add null terminator
	system(pFullExecCmd);

}