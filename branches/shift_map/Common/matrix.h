
/*
 *    Header file for the Matrix class
 *
 *    A simple matrix manipulation class
 *    
 *    Author: Edward Alston Anthony
 * 
 */

#include <iostream>
#include <string>
#include <math.h>
#include <iomanip>

#include "exceptions.h"
#ifdef USE_TRACEBACK
  #include "traceback.h"

  extern Traceback *Trace;
#endif


#ifndef _MATRIX_H_
#define _MATRIX_H_

#ifndef min
  #define min(a, b) (a < b ? a : b)
#endif

typedef struct {
  double *Col;
} matrixRowType;

class Matrix {
  private:
    matrixRowType *Row;
    int Rows, Cols;

  public:
    Matrix();
    Matrix(const int rows, const int cols);
	Matrix(const int rows, const int cols, double val);
    Matrix(const Matrix &src);
    ~Matrix();

    int NumOfRows() { return Rows; }
    int NumOfCols() { return Cols; }

    /* throws IndexOutOfBoundsException */
    void Set(int row, int col, double value);

    /* throws IndexOutOfBoundsException */
    double Get(int row, int col);

    void LoadZero();
    void LoadIdentity();
    void NormalizeBy(double d);
    void RoundOff(int exp);

    void MoveRow(int i, int j);
    void MultiplyRow(int row, double value);
    void AddRowToRow(int row1, int row2, double scale);
    void RREF();

    /* throws NotInvertibleMatrixException */
    Matrix *Inverse();

    Matrix& operator =(const Matrix &src);

    /* throws IncompatibleMatricesException */
    Matrix& operator +(const Matrix &src);

    /* throws IncompatibleMatricesException */
    Matrix& operator +=(const Matrix &src);

    /* throws IncompatibleMatricesException */
    Matrix& operator -(const Matrix &src);

    /* throws IncompatibleMatricesException */
    Matrix& operator -=(const Matrix &src);

    /* throws IncompatibleMatricesException */
    Matrix& operator *(const Matrix &src);

    Matrix& operator *(const double value);

    /* throws IncompatibleMatricesException */
    Matrix& operator *=(const Matrix &src);

    friend ostream& operator <<(ostream &out, const Matrix &src);
};

#endif
