// VideoMontage.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>


#include "VideoCollage.h" 
#include "VideoSequence.h"
#include "TangVideoCollage.h"
#include "CollageLayout.h"

int _tmain(int argc, _TCHAR* argv[])
{
	////TestLoadSaveSequence();
	_chdir("Clips/PinBallJPEG");	
	// TestTangVideoCollage("PinBall.seq", "PinBall.shot");
	TestTangVideoCollageLayout("collageFrame.seq", "PinBall.shot");
	//
	//vector<CvSize*>* sizeSequence = new vector<CvSize*>();	
	//for(int i = 0; i < 20; i++)
	//{		 
	//	CvSize* size = new CvSize();
	//	size->width = rand() % 100 + 10;
	//	size->height = rand() % 100 + 10;
	//	sizeSequence->push_back(size);
	//}
	//CollagePastingAlgo* algo = new CollagePastingAlgo();
	//algo->GetCollageArrangement(sizeSequence);

	//TestCollageLayout();
	//TestTangeCollage();
	
	//TestTangSaveSelectedFrame("PinBall.seq", "PinBall.shot", "SelectedFrame.seq");
	return 0;
}

