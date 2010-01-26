
/*
 *    Implementation file for the Matrix class
 *
 *    Author: Edward Alston Anthony
 * 
 */

#include "matrix.h"

Matrix::Matrix()
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
	//Matrix(0,0);
  Rows = 0;
  Cols = 0;
  Row = NULL;
}

Matrix::Matrix(const int rows, const int cols)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  Rows = rows;
  Cols = cols;
  Row = new matrixRowType[Rows];
  for (int i = 0; i < Rows; i++) {
    Row[i].Col = new double[Cols];
    for (int j = 0; j < Cols; j++)
      Row[i].Col[j] = 0;
  }
}

Matrix::Matrix(const int rows, const int cols, double val)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  Rows = rows;
  Cols = cols;
  Row = new matrixRowType[Rows];
  for (int i = 0; i < Rows; i++) {
    Row[i].Col = new double[Cols];
    for (int j = 0; j < Cols; j++)
      Row[i].Col[j] = val;
  }
}

Matrix::Matrix(const Matrix &src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  for (int i = 0; i < src.Rows; i++)
    for (int j = 0; j < src.Cols; j++)
    Row[i].Col[j] = src.Row[i].Col[j];
}

Matrix::~Matrix()
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  for (int i = 0; i < Rows; i++)
    delete Row[i].Col;
  delete Row;
}

void Matrix::LoadZero()
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  for (int i = 0; i < Rows; i++)
    for (int j = 0; j < Cols; j++)
      Row[i].Col[j] = 0;
}

void Matrix::LoadIdentity()
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  for (int i = 0; i < Rows; i++)
    for (int j = 0; j < Cols; j++)
      Row[i].Col[j] = ( i == j ? 1 : 0);
}

void Matrix::NormalizeBy(double d)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  for (int i = 0; i < Rows; i++)
    for (int j = 0; j < Cols; j++)
      Row[i].Col[j] = Row[i].Col[j] / d;
}

void Matrix::RoundOff(int exp)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  for (int i = 0; i < Rows; i++)
    for (int j = 0; j < Cols; j++)
      if (Row[i].Col[j] < pow((double)(10), (double)(exp)))
        Row[i].Col[j]= 0.0;
}

void Matrix::MoveRow(int i, int j)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  double *temp = Row[i-1].Col;
  Row[i-1].Col = Row[j-1].Col;
  Row[j-1].Col = temp;
}

void Matrix::MultiplyRow(int row, double value)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  for (int i = 0; i < Cols; i++) {
    Row[row-1].Col[i] *= value;
    if ((Row[row-1].Col[i]) == -0)
      Row[row-1].Col[i] = +0.0;
  }
}

void Matrix::AddRowToRow(int dst, int src, double scale)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  for (int i = 0; i < Cols; i++) {
    Row[dst-1].Col[i] += Row[src-1].Col[i] * scale;
    if ((Row[dst-1].Col[i]) == -0)
      Row[dst-1].Col[i] = +0.0;
  }
}

void Matrix::RREF()
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  int CurrentRow = 0;
  int CurrentCol = 0;
  bool found = false;

  while ((CurrentCol < Cols) &&
         (CurrentRow < Rows)) {
    for (int r = CurrentRow; r < Rows; r++)
      if (Row[r].Col[CurrentCol] != 0) {
        MoveRow(r+1, CurrentRow+1);
        found = true;
        break;
      }
    if (found) {
      MultiplyRow(CurrentRow+1, 1 / Row[CurrentRow].Col[CurrentCol]);
      for (int i = CurrentRow+1; i < Rows; i++)
        if (Row[i].Col[CurrentCol] != 0) {
          MultiplyRow(i+1, -1 / Row[i].Col[CurrentCol]);
          AddRowToRow(i+1, CurrentRow+1, 1);
        }
    }
    found = false;
    CurrentCol++;
    CurrentRow++;
  }

  CurrentRow--;
  CurrentCol--;
  while ((CurrentRow >= 0) &&
         (CurrentCol >= 0)) {
    for (int i = CurrentRow-1; i >= 0; i--)
      AddRowToRow(i+1, CurrentRow+1, -Row[i].Col[CurrentCol]);
    CurrentRow--;
    CurrentCol--;
  }
}

Matrix *Matrix::Inverse()
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  Matrix *result = new Matrix(Rows, 2*Cols);
  Matrix *result2 = new Matrix(Rows, Cols);

  if (Rows != Cols)
    throw NotInvertibleMatrixException("Matrix", "Inverse");

  for (int i = 0; i < Rows; i++) {
    for (int j = 0; j < Cols; j++)
      result->Row[i].Col[j] = Row[i].Col[j];
    for (int j = Cols; j < 2*Cols; j++)
      result->Row[i].Col[j] = ((j - Cols) == i ? 1 : 0);
  }
  result->RREF();

  for (int i = 0; i < Rows; i++)
    for (int j = 0; j < Cols; j++)
      result2->Row[i].Col[j] = result->Row[i].Col[Cols+j];
  delete result;
  return result2;
}

void Matrix::Set(int row, int col, double value)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if (((row - 1) >= Rows) ||
      ((row - 1) < 0) ||
      ((col - 1) >= Cols) ||
      ((col - 1) < 0))
    throw IndexOutOfBoundsException("Matrix", "Set");

  Row[row - 1].Col[col - 1] = value;
}

double Matrix::Get(int row, int col)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if (((row - 1) >= Rows) ||
      ((row - 1) < 0) ||
      ((col - 1) >= Cols) ||
      ((col - 1) < 0))
    throw IndexOutOfBoundsException("Matrix", "Get");

  return Row[row - 1].Col[col - 1];
}

Matrix& Matrix::operator =(const Matrix &src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if (Cols>0)
  {
	for (int i = 0; i < Rows; i++)
		delete [] Row[i].Col;
	delete [] Row;
  }

  Rows = src.Rows;
  Cols = src.Cols;
  Row = new matrixRowType[Rows];
  for (int i = 0; i < Rows; i++)
    Row[i].Col = new double[Cols];

  for (int i = 0; i < Rows; i++)
    for (int j = 0; j < Cols; j++)
      Row[i].Col[j] = src.Row[i].Col[j];
  return *this;
}

Matrix& Matrix::operator +(const Matrix &src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if ((src.Rows != Rows) ||
      (src.Cols != Cols))
    throw IncompatibleMatricesException("Matrix", "+");

  Matrix *result = new Matrix(Rows, Cols);

  for (int i = 0; i < Rows; i++)
    for (int j = 0; j < Cols; j++)
      result->Row[i].Col[j] = Row[i].Col[j] + src.Row[i].Col[j];

  return *result;
}

Matrix& Matrix::operator +=(const Matrix &src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if ((src.Rows != Rows) ||
      (src.Cols != Cols))
    throw IncompatibleMatricesException("Matrix", "+=");

  Matrix *result = new Matrix(Rows, Cols);

  for (int i = 0; i < Rows; i++)
    for (int j = 0; j < Cols; j++)
      result->Row[i].Col[j] = Row[i].Col[j] + src.Row[i].Col[j];

  return *result;
}

Matrix& Matrix::operator -(const Matrix &src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if ((src.Rows != Rows) ||
      (src.Cols != Cols))
    throw IncompatibleMatricesException("Matrix", "+");

  Matrix *result = new Matrix(Rows, Cols);

  for (int i = 0; i < Rows; i++)
    for (int j = 0; j < Cols; j++)
      result->Row[i].Col[j] = Row[i].Col[j] - src.Row[i].Col[j];

  return *result;
}

Matrix& Matrix::operator -=(const Matrix &src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if ((src.Rows != Rows) ||
      (src.Cols != Cols))
    throw IncompatibleMatricesException("Matrix", "+");

  *this = *this - src;
  return *this;
}

Matrix& Matrix::operator *(const Matrix &src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if (src.Rows != Cols)
    throw IncompatibleMatricesException("Matrix", "+");

  Matrix *result = new Matrix(Rows, src.Cols);
  double sum = 0.0;

  for (int i = 0; i < result->Rows; i++)
    for (int j = 0; j < result->Cols; j++) {
      sum = 0.0;
      for (int k = 0; k < Cols; k++)
        sum += (Row[i].Col[k] * src.Row[k].Col[j]);
      result->Row[i].Col[j] = sum;
    }
  return *result;
}

Matrix& Matrix::operator *(const double value)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  Matrix *result = new Matrix(Rows, Cols);

  for (int i = 0; i < Rows; i++)
    for (int j = 0; j < Cols; j++)
      result->Row[i].Col[j] = Row[i].Col[j] * value;
  return *result;
}

Matrix& Matrix::operator *=(const Matrix &src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  if (src.Rows != Cols)
    throw IncompatibleMatricesException("Matrix", "+");

  *this = *this * src;
  return *this;
}

ostream& operator <<(ostream &out, const Matrix &src)
{
#ifdef USE_TRACEBACK
  Trace->Add(__FILE__, __LINE__);
#endif
  out << "[\n";
  for (int i = 0; i < src.Rows; i++) {
    out << "  [ ";
    for (int j = 0; j < src.Cols; j++)
      out << setw(8) << (double) src.Row[i].Col[j] << " ";
    out << "]\n";
  }
  out << "]\n";
  return out;
}
