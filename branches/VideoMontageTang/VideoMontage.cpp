// VideoMontage.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <direct.h>
#include <stdio.h>
#include <stdlib.h>


#include "VideoCollage.h" 
#include "VideoSequence.h"
#include "TangVideoCollage.h"
int _tmain(int argc, _TCHAR* argv[])
{
	//TestLoadSaveSequence();
	_chdir("Clips/PinBallJPEG");	
	TestTangVideoCollage("PinBall.seq", "PinBall.shot");
	return 0;
}

