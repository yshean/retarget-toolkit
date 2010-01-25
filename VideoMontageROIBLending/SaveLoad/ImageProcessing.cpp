// This is the main DLL file.

#include "stdafx.h"
#include "ImageProcessing.h"

using namespace ImageProcessing;

CImageProcessing::CImageProcessing()
{
	
}


void CImageProcessing::SetFileList(System::String ^ sFileName, int FileNumber)
{

	char* allFiles = (char*)Marshal::StringToHGlobalAnsi(sFileName).ToPointer();
	long i = 0;
	char c = (char) allFiles[i];
	stringstream ss;
	vector<string> sFilename;
	int iFilename = 0;

	while(c != '\0') {

		
		while( c != ';')
		{
			ss <<c;
			i++;
			c = (char) allFiles[i];
		}
		
		sFilename.push_back(ss.str());
		ss.str("");
		i++;
		c = (char) allFiles[i];
    }
 
	videoFrameSequence = new VideoSequence();
	
	videoFrameSequence->frameList = new vector<char*>(FileNumber+1);

	for(int i = 0; i < FileNumber + 1; i++)
	{
		(*videoFrameSequence->frameList)[i] = (char*)malloc(200 * sizeof(char));
	}

	for(i=0; i<FileNumber; i++)
	{
		strcpy((*videoFrameSequence->frameList)[i], (char*) sFilename.at(i).c_str());
	}

	Marshal::FreeHGlobal(IntPtr(allFiles));
}



void CImageProcessing::SetFolder(System::String ^ sForlderPath)
{
	foldername = (char*)malloc(200 * sizeof(char));
	char* path = (char*)Marshal::StringToHGlobalAnsi(sForlderPath).ToPointer();
	
	strcpy(foldername, path);
	Marshal::FreeHGlobal(IntPtr(path));

}


void CImageProcessing::DisplayFrame(int FrameNo)
{

	//IplImage* image = LoadFrame(videoFrameSequence, FrameNo);
	IplImage* image = cvLoadImage((*videoFrameSequence->frameList)[FrameNo]);

	// Create CV windows & show image
	cvNamedWindow(foldername);
	cvShowImage(foldername, image);
	cvWaitKey(70);

	// Release image
	cvReleaseImage(&image);
}

void CImageProcessing::LoadShotFromFileName(System::String ^ sFilename)
{
	char* cFilename = (char*)Marshal::StringToHGlobalAnsi(sFilename).ToPointer();
	
	// Reinitialize Shot info
	CPlusShotInfoUnmanaged = (ShotInfo*)malloc(sizeof(ShotInfo)); 

	CPlusShotInfoUnmanaged = LoadShotFromFile(cFilename);

	Marshal::FreeHGlobal(IntPtr(cFilename));
}


void CImageProcessing::SaveShotToFileName(System::String ^ sFilename)
{
	char* cFilename = (char*)Marshal::StringToHGlobalAnsi(sFilename).ToPointer();
	
	SaveShotToFile(CPlusShotInfoUnmanaged, cFilename);

	Marshal::FreeHGlobal(IntPtr(cFilename));
	
}


void CImageProcessing::PassShotInfoToCPlusManaged(ShotInfoManaged^ CSharpShotInfoManaged)
{
	int iShotNo = 0;
	int iSubShotNo = 0;

	//CPlusShotInfoUnmanaged = new ShotInfo();
	//CPlusShotInfoUnmanaged->shotList = new Shot();
	//CPlusShotInfoUnmanaged->shotList->subShotList = new SubShot();

	CPlusShotInfoUnmanaged = (ShotInfo*)malloc(sizeof(ShotInfo)); 
	
	ShotManaged ^ TempShotManaged = gcnew ShotManaged();
	SubShotManaged ^ TempSubShotManaged = gcnew SubShotManaged();

	// Start Conversion from CSharpobject to CPlusShotInfoUnmanaged
	// Get subShot count
	CPlusShotInfoUnmanaged->shotCount = CSharpShotInfoManaged->shotCount;

	//Initialiaze Shot based on its quantity created 
	CPlusShotInfoUnmanaged->shotList = (Shot*)malloc(CPlusShotInfoUnmanaged->shotCount * sizeof(Shot));

	// Retrieve Shot parameter
	for(iShotNo = 0; iShotNo < CPlusShotInfoUnmanaged->shotCount; iShotNo++)
	{
		// Convert ArrayList to Shot Managed
		TempShotManaged = (ShotManaged^)CSharpShotInfoManaged->shotList[iShotNo];
			
		// Get subShot count
		CPlusShotInfoUnmanaged->shotList[iShotNo].subShotCount = TempShotManaged->subShotCount;
		
		//Initialiaze subShot based on its quantity created 
		CPlusShotInfoUnmanaged->shotList[iShotNo].subShotList = (SubShot*)malloc(CPlusShotInfoUnmanaged->shotList[iShotNo].subShotCount * sizeof(SubShot));
		
		// Retrieve Sub Shot parameter
		for(iSubShotNo = 0; iSubShotNo < CPlusShotInfoUnmanaged->shotList[iShotNo].subShotCount; iSubShotNo++)
		{
			TempSubShotManaged = (SubShotManaged^)TempShotManaged->subShotList[iSubShotNo];
		
			// Get Shot Type
			CPlusShotInfoUnmanaged->shotList[iShotNo].subShotList[iSubShotNo].shotType = TempSubShotManaged->shotType;

			// Get start value
			CPlusShotInfoUnmanaged->shotList[iShotNo].subShotList[iSubShotNo].start = TempSubShotManaged->start;

			// Get end value
			CPlusShotInfoUnmanaged->shotList[iShotNo].subShotList[iSubShotNo].end = TempSubShotManaged->end;

		}
		
	}

	//CheckResult();
}

ShotInfoManaged ^ CImageProcessing::GetShotInfoToCSharp()
{
	int iShotNo = 0;
	int iSubShotNo = 0;

	ShotInfoManaged ^ TempShotInfoManaged = gcnew ShotInfoManaged();;	
	
	// Start Conversion from CPlusShotInfoUnmanaged to CSharpStructure  
	// Get Shot count and assign its value to TempShotInfoManaged
	 TempShotInfoManaged->shotCount = CPlusShotInfoUnmanaged->shotCount ;
	
	 // Init ShotList
	 TempShotInfoManaged->shotList = gcnew ArrayList();

	// Retrieve Shot parameter
	for(iShotNo = 0; iShotNo < TempShotInfoManaged->shotCount; iShotNo++)
	{	
		ShotManaged ^ TempShotManaged = gcnew ShotManaged();

		// Get subShot count
		TempShotManaged->subShotCount = CPlusShotInfoUnmanaged->shotList[iShotNo].subShotCount;
		
		// Init subShotList
		TempShotManaged->subShotList = gcnew ArrayList();
	
		// Retrieve Sub Shot parameter
		for(iSubShotNo = 0; iSubShotNo < TempShotManaged->subShotCount; iSubShotNo++)
		{
			SubShotManaged ^ TempSubShotManaged = gcnew SubShotManaged();

			// Get Shot Type
			TempSubShotManaged->shotType = CPlusShotInfoUnmanaged->shotList[iShotNo].subShotList[iSubShotNo].shotType;

			// Get start value
			TempSubShotManaged->start = CPlusShotInfoUnmanaged->shotList[iShotNo].subShotList[iSubShotNo].start;

			// Get end value
			TempSubShotManaged->end = CPlusShotInfoUnmanaged->shotList[iShotNo].subShotList[iSubShotNo].end;
				

			// Add its value to TempShot Managed
			TempShotManaged->subShotList->Add(TempSubShotManaged);
	  
			
		}
	
		// Add its value to TempShotInfoManaged Managed
		TempShotInfoManaged->shotList->Add(TempShotManaged);
		
		
	}

	return TempShotInfoManaged;

}

void CImageProcessing::DeallocateMemory()
{
	//free(CPlusShotInfoUnmanaged->shotList->subShotList);
	
	free(CPlusShotInfoUnmanaged);
	free(CPlusShotInfoUnmanaged->shotList);

}
void CImageProcessing::CheckResult()
{

	int h=0;
	h = CPlusShotInfoUnmanaged->shotList[0].subShotList[0].end;
	h = CPlusShotInfoUnmanaged->shotList[0].subShotList[0].start;
	h = CPlusShotInfoUnmanaged->shotList[0].subShotList[0].shotType;

	h = CPlusShotInfoUnmanaged->shotList[1].subShotList[0].end;
	h = CPlusShotInfoUnmanaged->shotList[1].subShotList[0].start;
	h = CPlusShotInfoUnmanaged->shotList[1].subShotList[0].shotType;

	h = CPlusShotInfoUnmanaged->shotList[2].subShotList[0].end;
	h = CPlusShotInfoUnmanaged->shotList[2].subShotList[0].start;
	h = CPlusShotInfoUnmanaged->shotList[2].subShotList[0].shotType;

	h = CPlusShotInfoUnmanaged->shotList[3].subShotList[0].end;
	h = CPlusShotInfoUnmanaged->shotList[3].subShotList[0].start;
	h = CPlusShotInfoUnmanaged->shotList[3].subShotList[0].shotType;

	h = CPlusShotInfoUnmanaged->shotList[0].subShotList[1].end;
	h = CPlusShotInfoUnmanaged->shotList[0].subShotList[1].start;
	h = CPlusShotInfoUnmanaged->shotList[0].subShotList[1].shotType;

	h = CPlusShotInfoUnmanaged->shotList[0].subShotList[2].end;
	h = CPlusShotInfoUnmanaged->shotList[0].subShotList[2].start;
	h = CPlusShotInfoUnmanaged->shotList[0].subShotList[2].shotType;
}


