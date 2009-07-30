
/*
 *    Common exceptions
 *
 *    Author: Edward Alston Anthony
 *
 */

#include <string>

using namespace std;

#ifndef _NO_CURRENT_ELEMENT_EXCEPTION_CLASS_
#define _NO_CURRENT_ELEMENT_EXCEPTION_CLASS_

class NoCurrentElementException {
  public:
    NoCurrentElementException(const char *cls, const char *func)
      : message(string("Exception in class ") +
                string(cls) +
                string("::") +
                string(func) +
                string("(): no current element")) { cerr << message << endl; }
    friend ostream& operator <<(ostream& out, NoCurrentElementException ex)
      { out << ex.message << endl; return out; }

  private:
    string message;
};

#endif


#ifndef _NOT_FOUND_EXCEPTION_CLASS_
#define _NOT_FOUND_EXCEPTION_CLASS_

class NotFoundException {
  public:
    NotFoundException(const char *cls, const char *func)
      : message(string("Exception in class ") +
                string(cls) +
                string("::") +
                string(func) +
                string("(): element not found")) { cerr << message << endl; }
    friend ostream& operator <<(ostream& out, NotFoundException ex)
      { out << ex.message << endl; return out; }

  private:
    string message;
}; 

#endif


#ifndef _INDEX_OUT_OF_BOUNDS_EXCEPTION_CLASS_
#define _INDEX_OUT_OF_BOUNDS_EXCEPTION_CLASS_

class IndexOutOfBoundsException {
  public:
    IndexOutOfBoundsException(const char *cls, const char *func)
      : message(string("Exception in class ") +
                string(cls) +
                string("::") +
                string(func) +
                string("(): index out of bound")) { cerr << message << endl; }
    friend ostream& operator << (ostream& out, IndexOutOfBoundsException ex)
      { out << ex.message << endl; return out; }

  private:
    string message;
};

#endif


#ifndef _FILE_NOT_FOUND_EXCEPTION_CLASS_
#define _FILE_NOT_FOUND_EXCEPTION_CLASS_
  
class FileNotFoundException {
  public:
    FileNotFoundException(const char *cls, const char *func, const char *file)
      : message(string("Exception in class ") +
                string(cls) +
                string("::") +
                string(func) +
                string("(): ") +
                string(file) +
                string(" not found")) { cerr << message << endl; }
    friend ostream& operator <<(ostream& out, FileNotFoundException ex)
      { out << ex.message << endl; }

  private:
    string message;
};

#endif

#ifndef _FOLDER_NOT_FOUND_EXCEPTION_CLASS_
#define _FOLDER_NOT_FOUND_EXCEPTION_CLASS_
  
class FolderNotFoundException {
  public:
    FolderNotFoundException(const char *cls, const char *func, const char *file)
      : message(string("Exception in class ") +
                string(cls) +
                string("::") +
                string(func) +
                string("(): ") +
                string(file) +
                string(" not found")) { cerr << message << endl; }
    friend ostream& operator <<(ostream& out, FolderNotFoundException ex)
      { out << ex.message << endl; }

  private:
    string message;
};

#endif

#ifndef _INCOMPATIBLE_MATRICES_EXCEPTION_CLASS_
#define _INCOMPATIBLE_MATRICES_EXCEPTION_CLASS_

class IncompatibleMatricesException {
  public:
    IncompatibleMatricesException(const char *cls, const char *func)
      : message(string("Exception in class ") +
                string(cls) +
                string("::") +
                string(func) +
                string("(): operation on incompatible matrices")) { cerr << message << endl; }
    friend ostream& operator << (ostream& out, IncompatibleMatricesException ex)
      { out << ex.message << endl; return out; }

  private:
    string message;
};
    
#endif


#ifndef _NOT_INVERTIBLE_MATRIX_EXCEPTION_CLASS_
#define _NOT_INVERTIBLE_MATRIX_EXCEPTION_CLASS_

class NotInvertibleMatrixException {
  public:
    NotInvertibleMatrixException(const char *cls, const char *func)
      : message(string("Exception in class ") +
                string(cls) +
                string("::") +
                string(func) +
                string("(): error computing matrix inverse")) { cerr << message << endl; }
    friend ostream& operator <<(ostream& out, NotInvertibleMatrixException ex)
      { out << ex.message << endl; return out; }

  private:
    string message;
};

#endif


#ifndef _MAXIMUM_WINDOWS_EXCEPTION_CLASS_
#define _MAXIMUM_WINDOWS_EXCEPTION_CLASS_

class MaximumWindowsException {
  public:
    MaximumWindowsException(const char *cls, const char *func)
      : message(string("Exception in class ") +
                string(cls) +
                string("::") +
                string(func) +
                string("(): maximum windows reached - cannot create more")) { cerr << message << endl; }
    friend ostream& operator <<(ostream& out, MaximumWindowsException ex)
      { out << ex.message << endl; return out; }

  private:
    string message;
};

#endif


#ifndef _INCOMPATIBLE_DIMENSIONS_EXCEPTION_CLASS_
#define _INCOMPATIBLE_DIMENSIONS_EXCEPTION_CLASS_

class IncompatibleDimensionsException {
  public:
    IncompatibleDimensionsException(const char *cls, const char *func)
      : message(string("Exception in class ") +
                string(cls) +
                string("::") +
                string(func) +
                string("(): dimensions not compatible - cannot perform operation")) { cerr << message << endl; }
    friend ostream& operator <<(ostream& out, IncompatibleDimensionsException ex)
      { out << ex.message << endl; return out; }

  private:
    string message;
};

#endif
