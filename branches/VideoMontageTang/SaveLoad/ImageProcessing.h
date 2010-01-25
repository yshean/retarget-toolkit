// ImageProcessing.h

#pragma once
#include <string>
#include <sstream>
#include "VideoSequence.h"

using namespace System::Runtime::InteropServices;
using namespace System;
using namespace System::Collections;

using std::stringstream;


namespace ImageProcessing {
			// Structure created based on C# object
			public ref struct SubShotManaged
			{
				// type of sub shot, see:
				//#define VCSHOT_STATIC 0
				//#define VCSHOT_TILT 1
				//#define VCSHOT_PAN 2
				//#define VCSHOT_ZOOM 3
				int shotType;

				// beginning frame index
				int start;
				// last frame index
				int end;
			};

			public ref struct ShotManaged
			{
				// in case of undefined subshot, the list may contain only 1 element
				// which is the subshot with type "VCSHOT_UNDEFINED"
				ArrayList^ subShotList;

				int subShotCount;
			};

			public ref struct ShotInfoManaged
			{
				// list of shot
				ArrayList^ shotList;

				// number of shot count
				int shotCount;

			};

		public ref class CImageProcessing
		{



			// Function
	
	public:

			CImageProcessing();
			void SetFileList(System::String ^ sFileList, int FileNumber);
			void SetFolder(System::String ^ sForlderPath);
			void DisplayFrame(int FrameNo);
			void LoadShotFromFileName(System::String ^ sFilename);
			void SaveShotToFileName(System::String ^ sFilename);
			void PassShotInfoToCPlusManaged(ShotInfoManaged^ ShotInfoManaged);
			ShotInfoManaged ^ CImageProcessing::GetShotInfoToCSharp();
			void DeallocateMemory();

			// Variable
			VideoSequence* videoFrameSequence;
			
			//ShotInfoManaged^ CPlusShotInfoManaged;
			ShotInfo * CPlusShotInfoUnmanaged;

			void CheckResult();

//			ArrayList^ ShotManagedArray;
//			ArrayList^ SubShotManagedArray;

			char* foldername;
	};


	
}
