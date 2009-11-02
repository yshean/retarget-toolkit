// TestCallCSharp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <string> 
#include <time.h>
#include <windows.h> 



using std::stringstream;
using std::string;

//Header declarations
long WINAPI DisplayDialog(long lParam);
long WINAPI OtherTask(long lParam);
void doEntry(string );
string convertWCharArrayToString(const WCHAR * const wcharArray);

int _tmain(int argc, _TCHAR* argv[])
{
	//if (argc!=3)
	//{
		//cerr << "bad arguments";
	//	exit(-1);
	//}

	//cout << "prog name is: " << argv[0];
	//cout << "first parameter is: " << argv[1];
	//cout << "second parameter is: " << argv[2];
	string sProjectDirectory = "C:\\Java\\File\\Basketball\\RetargetBMP\\";
	
	doEntry(sProjectDirectory);

	string sConvFile = "C:\\Java\\App\\QuickView.exe";
	string sDirectoryFiles = " C:\\Java\\File\\Basketball\\RetargetBMP\\0000.bmp";
	string sFullExecCmd = "";
	
	sFullExecCmd += sConvFile;
	sFullExecCmd += sDirectoryFiles;
	int length = sFullExecCmd.length();
	char *pFullExecCmd = new char[ length + 1 ]; // including null

	sFullExecCmd.copy( pFullExecCmd, length, 0 ); // copy string1 to ptr2 char*
	pFullExecCmd[ length ] = '\0'; // add null terminator
	system(pFullExecCmd);

	HANDLE hThread[3];
	DWORD dwID[3];
	DWORD dwRetVal = 0;
	
	//release the threads. Remember, ThreadOne is our main thread
	hThread[0] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)DisplayDialog,NULL,0,&dwID[0]);
 
	//wait for all threads to complete before continuing
	  dwRetVal = WaitForMultipleObjects(1, hThread, TRUE, INFINITE);

	//Display a messagebox to show that the Wait state has finished
	MessageBox(NULL,_T("This Messagebox is to show that all threads have completed."),_T("Messagebox"), NULL);  
  
	//close handles
	CloseHandle(hThread[0]);

	return 0;
}


long WINAPI DisplayDialog(long lParam)
{
	
    return 0;
}

long WINAPI OtherTask(long lParam)
{
    return 0;
}

void doEntry(string dir)
{
    WIN32_FIND_DATA findFileData;
	//HANDLE hFind = FindFirstFile((LPCWSTR)(dir.c_str()), &findFileData);
    HANDLE hFind = FindFirstFile((LPCWSTR)dir.c_str(), &findFileData);

    if(hFind  == INVALID_HANDLE_VALUE) {
        std::cout <<"No files found." <<std::endl;
		std::cout << GetLastError() <<std::endl;
    } else {
        std::cout <<"Files found." <<std::endl;
    }
    
	//std::cout << GetLastError() <<std::endl;

    int fileNumber = 0;
    std::cout <<fileNumber <<":" <<convertWCharArrayToString(findFileData.cFileName) <<std::endl;

	while(FindNextFile(hFind, &findFileData)) {
        fileNumber++;
        std::cout <<fileNumber <<":" <<convertWCharArrayToString(findFileData.cFileName) <<std::endl;
    }
 
   FindClose(hFind);

}


string convertWCharArrayToString(const WCHAR * const wcharArray) 
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