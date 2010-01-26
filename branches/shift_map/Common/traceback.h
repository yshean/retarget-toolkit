
/*
 *    Header file for the Traceback class
 *
 *    Maintains a list of last n-visted locations
 *    in the source code during runtime. If a
 *    SIGSEGV or similar signal is caught,
 *    the function Print() can be called to
 *    list the last few locations the execution
 *    of the program passed through.
 *
 *    Author: Edward Alston Anthony
 *
 */

#include <iostream>
#include <string>
#include <stdlib.h>

#include "exceptions.h"

#ifndef _TRACEBACK_H_
#define _TRACEBACK_H_

class Traceback {
  typedef struct {
    char Filename[512];
    int Line;
  } tracebackNodeType;

  public:
    Traceback(int Levels);
    ~Traceback();
    void Add(const char *file, int line);
    void Print();

  private:
    char *Space(int n);

    tracebackNodeType *Trace;
    int MaxTracebackLevels;
};

#endif
