
/*
 *    Implementation for the Traceback class
 *
 *    Author: Edward Alston Anthony
 *
 */

#include <iostream>
#include <stdlib.h>

#include "traceback.h"

#ifdef USE_TRACEBACK

/* constructor */
Traceback::Traceback(int Levels)
{
  Trace = new tracebackNodeType[Levels];
  MaxTracebackLevels = Levels;
}

/* destructor */
Traceback::~Traceback()
{
  delete Trace;
}

/*  private function that returns n-number
 *  of blank space in a character array
 */
char *Traceback::Space(const int n)
{
  char *space = new char[n+1] = {'\0'};
  for (int i = 0; i < n; i++)
    space[i] = ' ';
  space[n] = '\0';
  return space;
}

/* add an entry to the traceback history */
void Traceback::Add(const char *file, int line)
{
  for (int i = 0; i < MaxTracebackLevels - 1; i++) {
    strcpy(Trace[i].Filename, Trace[i+1].Filename);
    Trace[i].Line = Trace[i+1].Line;
  }
  strcpy(Trace[MaxTracebackLevels - 1].Filename, file);
  Trace[MaxTracebackLevels - 1].Line = line;
}

/* print the last n traceback recorded */
void Traceback::Print()
{
  cerr << "Last " << MaxTracebackLevels << " location traced" << endl;
  for (int i = 0; i < MaxTracebackLevels; i++) {
    cerr << Space(MaxTracebackLevels - (i + 1)) << "FILE: " << Trace[i].Filename;
    cerr << ", LINE : " << Trace[i].Line << endl;
  }
}

#endif
