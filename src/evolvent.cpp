﻿/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      evolvent.cpp                                                //
//                                                                         //
//  Purpose:   Source file for evolvent classes                            //
//                                                                         //
//  Author(s): Barkalov K., Sysoyev A.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "evolvent.h"
#include "exception.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// ------------------------------------------------------------------------------------------------
TEvolvent::TEvolvent(int _N, int _m): extNull(0.0), extOne(1.0), extHalf(0.5)
{
  int i;
  if ((_N < 1) || (_N > MaxDim))
  {
    throw EXCEPTION("N is out of range");
  }
  N = _N;
  y = new double[N];

  if ((_m < 2) || (_m > MaxM))
  {
    throw EXCEPTION("m is out of range");
  }
  m = _m;

  for (nexpExtended = extOne, i = 0; i < N; nexpExtended += nexpExtended, i++)
    ;
}

// ------------------------------------------------------------------------------------------------
TEvolvent::~TEvolvent()
{
  delete [] y;
}

// ----------------------------------------------------------------------------
void TEvolvent::SetBounds(const double* _A, const double* _B)
{
  for (int i = 0; i < N; i++)
  {
    A[i] = _A[i];
    B[i] = _B[i];
  }
}

// ----------------------------------------------------------------------------
void TEvolvent::CalculateNumbr(double *s, int *u, int *v, int *l)
// calculate s(u)=is,l(u)=l,v(u)=iv by u=iu
{
  int i, k1, k2, l1;
  double is, iff;

  iff = nexpExtended;
  is = extNull;
  k1 = -1;
  k2 = 0; 
  l1 = 0; 
  for (i = 0; i < N; i++)
  {
    iff = iff / 2;
    k2 =- k1 * u[i];
    v[i] = u[i];
    k1 = k2;
    if (k2 < 0)
      l1 = i;
    else
    {
      is += iff;
      *l = i;
    }
  }
  if (is == extNull)
    *l = N - 1;
  else
  {
    v[N - 1] = -v[N - 1];
    if (is == (nexpExtended - extOne))
      *l = N - 1;
    else
    {
      if (l1 == (N - 1))
        v[*l] = -v[*l];
      else
        *l = l1;
    }
  }
  *s = is;
}

// ----------------------------------------------------------------------------
void TEvolvent::CalculateNode(double is, int n, int *u, int *v, int *l)
// вычисление вспомогательного центра u(s) и соответствующих ему v(s) и l(s)
// calculate u=u[s], v=v[s], l=l[s] by is=s
{
  int n1, i, j, k1, k2, iq;
  double iff;
  double nexp;

  iq = 1;
  for (nexp = 1, i = 0; i < n; nexp += nexp, i++);
  n1 = n - 1;
  *l = 0;
  if (is == 0)
  {
    *l = n1;
    for (i = 0; i < n; i++)
    {
      u[i] =- 1;
      v[i] =- 1;
    }
  }
  else if (is == (nexpExtended - extOne))
  {
    *l = n1;
    u[0] = 1;
    v[0] = 1;
    for (i = 1; i < n; i++)
    {
      u[i] =- 1;
      v[i] =- 1;
    }
    v[n1] = 1;
  }
  else
  {
    iff = nexpExtended; 
    k1 =- 1;
    for (i = 0; i < n; i++)
    {
      iff = iff / 2;
      if (is >= iff)
      {
        if ((is == iff) && (is != extOne))
        {
          *l = i;
          iq =- 1;
        }
        is -= iff;
        k2 = 1;
      }
      else
      {
        k2 =- 1;
        if ((is == (iff - extOne)) && (is != extNull))
        {
          *l = i;
          iq = 1;
        }
      }
      j =- k1 * k2;
      v[i] = j;
      u[i] = j;
      k1 = k2;
    }
    v[*l] = v[*l] * iq;
    v[n1] =- v[n1];
  }
}

// ------------------------------------------------------------------------------------------------
void TEvolvent::transform_P_to_D()
{
  //if (N == 1) return;
  // transformation from hypercube P to hyperinterval D
  for (int i = 0; i < N; i++)
    y[i] = y[i] * (B[i] - A[i]) + (A[i] + B[i]) / 2;
}

// ----------------------------------------------------------------------------
void TEvolvent::transform_D_to_P()
{
  //if (N == 1) return;
  // transformation from hyperinterval D to hypercube P
  for (int i = 0; i < N; i++)
    y[i] = (y[i] - (A[i] + B[i]) / 2) / (B[i] - A[i]);
}

// ----------------------------------------------------------------------------
double* TEvolvent::GetYOnX(const double& _x)
{
  if (N == 1)
  {
//    y[0] = _x.toDouble() - 0.5;
    y[0] = _x - 0.5;
    return y;
  }

  int iu[MaxDim];
  int iv[MaxDim];
  int l;
  double d;
  int mn;
  double r;
  int iw[MaxDim];
  int it, i, j;
  double is;

  d = _x;
  r = 0.5;
  it = 0;
  mn = m * N;
  for (i = 0; i < N; i++)
  {
    iw[i] = 1;
    y[i] = 0.0;
  }
  for (j = 0; j < m; j++)
  {
    if (_x == extOne)
    {
      is = nexpExtended - extOne;
      d = extNull; // d = 0.0;
    }
    else
    {
      //Код из старой версии - уточнить работоспособность при N > 32
      d *= nexpExtended;
//      is =(int)d.toDouble();
      is =(int)d;
      d  -= is;
    }
    CalculateNode(is, N, iu, iv, &l);
    i = iu[0];
    iu[0] = iu[it];
    iu[it] = i;
    i = iv[0];
    iv[0] = iv[it];
    iv[it] = i;
    if (l == 0)
      l = it;
    else if (l == it)
      l = 0;
    r *= 0.5;
    it = l;
    for (i = 0; i < N; i++)
    {
      iu[i] *= iw[i];
      iw[i] *= -iv[i];
      y[i] += r * iu[i];
    }
  }
  return y;
}

//-----------------------------------------------------------------------------
double TEvolvent::GetXOnY()
{
  int u[MaxDim], v[MaxDim];
  double x, r1;
  if (N == 1)
  {
    x = y[0] + 0.5;
    return x;
  }

  double  r;
  int w[MaxDim];
  int i, j, it, l;
  double is;

  for (i = 0; i < N; i++)
    w[i] = 1;
  r = 0.5;
  r1 = extOne;
  x = extNull;
  it = 0;
  for (j = 0; j < m; j++)
  {
    r *= 0.5;
    for (i = 0; i < N; i++)
    {
      u[i] = (y[i] < 0) ? -1 : 1;
      y[i] -= r * u[i];
      u[i] *= w[i];
    }
    i = u[0];
    u[0] = u[it];
    u[it] = i;
    CalculateNumbr(&is, u, v, &l);
    i = v[0];
    v[0] = v[it];
    v[it] = i;
    for (i = 0; i < N; i++)
      w[i] *= -v[i];
    if (l == 0)
      l = it;
    else
      if (l == it)
        l = 0;
    it = l;
    r1 = r1 / nexpExtended;
    x += r1 * is;
  }
  return x;
}

//----------------------------------------------------------------------------
void TEvolvent::GetImage(const double& x, double* _y)
{
  // x ---> y
  GetYOnX(x); // it saves return value to y, so no need to call operator= again

  transform_P_to_D();

  memcpy(_y, y, N * sizeof(double));
}

// ------------------------------------------------------------------------------------------------
TShiftedEvolvent::TShiftedEvolvent(int _N, int _m, int _L):
  TEvolvent(_N, _m)
{
  if ((_L < 0) || (_L > m))
  {
    throw EXCEPTION("L is out of range");
  }
  L = _L;
}

// ------------------------------------------------------------------------------------------------
TShiftedEvolvent::~TShiftedEvolvent()
{
}

// ------------------------------------------------------------------------------------------------
void TShiftedEvolvent::GetImage(const double& x, int EvolventNum, double* _y)
{
}

// ------------------------------------------------------------------------------------------------
void TShiftedEvolvent::GetPreimages(double* _y, double *x)
{
}

// ------------------------------------------------------------------------------------------------
TRotatedEvolvent::TRotatedEvolvent(int _N, int _m, int _L):
  TEvolvent(_N, _m)
{
  PlaneCount =  N * (N - 1) / 2;
  if ((_L < 1) || (_L > PlaneCount))
  {
    throw EXCEPTION("L is out of range");
  }
  L = _L;
  GetAllPlanes();
  PowOfHalf[0] = 1;
  for (int i = 1; i < m + 2; i++)
    PowOfHalf[i] = PowOfHalf[i - 1] / 2;
}

// ------------------------------------------------------------------------------------------------
TRotatedEvolvent::~TRotatedEvolvent()
{
}

// ------------------------------------------------------------------------------------------------
void TRotatedEvolvent::GetAllPlanes()
{
  const int k = 2; // Подмножества из двух элементов
  int plane[k];    // Два номера под элементы

  for (int i = 0; i < k; i++)
    plane[i] = i;

  if (N <= k)
  {
    for (int i = 0; i < k; i++)
    {
      Planes[0][i] = plane[i];
    }
    return;
  }
  int p = k - 1;
  int counter = 0; //счетчик числа перестановок
  while (p >= 0)
  {
    for (int i = 0; i < k; i++)
    {
      Planes[counter][i] = plane[i];
    }
    counter++;

    if (plane[k - 1] == N - 1)
    {
      p--;
    }
    else
    {
      p = k - 1;
    }

    if (p >= 0)
    {
      for (int i = k - 1; i >= p; i--)
      {
        plane[i] = plane[p] + i - p + 1;
      }
    }
  }
}

// ------------------------------------------------------------------------------------------------
void TRotatedEvolvent::GetImage(const double& x, int EvolventNum, double* _y)
{
  if (EvolventNum == 0 || L == 1)
  {
    TEvolvent::GetImage(x, _y);
    return;
  }

  int PlaneIndex = EvolventNum - 1; // теперь PlaneIndex - номер перестановки
  PlaneIndex = PlaneIndex % PlaneCount;
  
  GetYOnX(x);

  // shift to center for convenient rotation
  for (int i = 0; i < N; i++)
    y[i] += PowOfHalf[m + 1];

  // rotate
  double tmpCoord = y[Planes[PlaneIndex][1]];
  y[Planes[PlaneIndex][1]] = y[Planes[PlaneIndex][0]];
  y[Planes[PlaneIndex][0]] = - tmpCoord;

/*
  //Меняем знак преобразования, если число разверток больше числа плоскостей
  if (intx > PlaneCount)
  {
    y[Planes[PlaneIndex][0]] = -y[Planes[PlaneIndex][0]];
    y[Planes[PlaneIndex][1]] = -y[Planes[PlaneIndex][1]];
  }
*/

  // shift back to corner
  for (int i = 0; i < N; i++)
    y[i] -= PowOfHalf[m + 1];

  transform_P_to_D();
  memcpy(_y, y, N * sizeof(double));
}

// ------------------------------------------------------------------------------------------------
void TRotatedEvolvent::GetPreimages(double* _y, double *x)
{
  memcpy(y, _y, N * sizeof(double));
  transform_D_to_P();
  // прообраз для первой развертки
  x[0] = GetXOnY();

  if (L == 1)
    return;

  for (int i = 1; i <= L; i++)
  {
    memcpy(y, _y, N * sizeof(double));
    // обратное преобразование координат
    int PlaneIndex = (i - 1) % PlaneCount;

    double tmpCoord = y[Planes[PlaneIndex][1]];
    y[Planes[PlaneIndex][1]] = -_y[Planes[PlaneIndex][0]];
    y[Planes[PlaneIndex][0]] = tmpCoord;
/*
    if (i > PlaneCount)//Меняем знак преобразования, если число разверток больше числа плоскостей
    {
      y[Planes[PlaneIndex][0]] = -y[Planes[PlaneIndex][0]];
      y[Planes[PlaneIndex][1]] = -y[Planes[PlaneIndex][1]];
    }
*/
    // прообраз для i - 1 развертки
    x[i] = GetXOnY();
  }
}
// - end of file ----------------------------------------------------------------------------------
