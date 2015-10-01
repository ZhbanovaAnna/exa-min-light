/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      data.h                                                      //
//                                                                         //
//  Purpose:   Header file for search data classes                         //
//                                                                         //
//  Author(s): Sysoyev A., Barkalov K.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __DATA_H__
#define __DATA_H__

#include "common.h"
#include "extended.h"
#include "queue.h"
#include <stack>
#include "parameters.h"
//#include <vector>
#include <string.h>
using namespace std;

// ------------------------------------------------------------------------------------------------
struct TTrial
{
  Extended  x;
  double y[MaxDim];
  double FuncValues[MaxNumOfFunc];
  int index;
  int K;
};

// ------------------------------------------------------------------------------------------------
struct TSearchInterval
{
  Extended xl;   // ����� ������� ���������
  Extended xr;   // ������ ������� ���������
//  double   dx;   // ������� �����
  double   delta;// "�������������" �����
  double   zl;   // �������� ���������� ������������ ����������� � xl
  int      izl;  // ������ ���������� ������������ ����������� � xl
  double   zr;   // �������� ���������� ������������ ����������� � xr
  int      izr;  // ������ ���������� ������������ ����������� � xr
  int      ind;  // ����� ��������
  int	   K;    // ����� "���������" ��������
  double   R;    // �������������� ��������� (xl, xr)
  double   locR; // ��������� �������������� ��������� (xl, xr)
  double   *z;   // �������� ����������� ������������ � izl, ���-�� - izl + 1
  TSearchInterval()
  {
    xl = 0.0;
//	dx = 0.0;
    xr = 0.0;
    zl = zr = R = locR = 0;
    izl = izr = ind = 0;
    z = NULL; // ��������� ������ ���������� � ������ ���������� � �������
  }
  TSearchInterval(const TSearchInterval &p)
  {
    xl = p.xl; zl = p.zl; izl = p.izl;
//    dx = p.dx; 
    xr = p.xr;
    zr = p.zr; izr = p.izr;
    ind = p.ind;
    K = p.K;
    R = p.R;
    locR = p.locR;
    delta = p.delta;
    z = NULL;
    if (p.z)
    {
      z = new double [izl + 1];
      memcpy(z, p.z, sizeof(double) * (izl + 1));
    }
  }
  ~TSearchInterval() { delete [] z; }
};

// ------------------------------------------------------------------------------------------------
struct TTreeNode
{
  TSearchInterval *pInterval;
  unsigned char Height;
  TTreeNode *pLeft;
  TTreeNode *pRight;
  TTreeNode(TSearchInterval &p)
  { 
    pInterval = new TSearchInterval(p); 
    Height = 1; 
    pLeft = pRight = NULL; 
  }
  ~TTreeNode() { delete pInterval; }
};

// ------------------------------------------------------------------------------------------------
class TSearchData
{
protected:
  int NumOfFuncs; // ����� ������� ������
  int MaxSize;    // ������������ ������ ��� = ������������� ����� �������� ������
  int Count;      // ������� ����� ���������� � ������
  int CurIndex;   // ������� ������, ������������ � ���������
  TTreeNode *pRoot;
  TTreeNode *pCur;
  TTreeNode *pCurIter;
  stack<TTreeNode*> Stack;      // ���� ��� ���������
  TPriorityQueue *pQueue;       // ������� ���������� �������������
  TPriorityQueue *pLocalQueue;  // ������� ��������� �������������

  void DeleteTree(TTreeNode *pNode);
  unsigned char GetHeight(TTreeNode *p);
  int GetBalance(TTreeNode *p);
  void FixHeight(TTreeNode *p);
  TTreeNode* RotateRight(TTreeNode *p); // ������ ������� ������ p
  TTreeNode* RotateLeft(TTreeNode *p);  // ����� ������� ������ p
  TTreeNode* Balance(TTreeNode *p);     // ������������ ���� p
  // ������� � ������ � ������ p (�����������)
  TTreeNode* Insert(TTreeNode *p, TSearchInterval &pInterval);
  // ����� ���� � ������ x �� ����� ������� ��������� (�����������)
//  TTreeNode* Find(Extended x);
  TTreeNode* Find(TTreeNode *p, Extended x);
  // ����� ���� �� ������ ������� ���������
//  TTreeNode* FindR(TTreeNode *p, Extended x);
  TParameters parameters; //��������� ������
public:
  TSearchData(TParameters _parameters, int _NumOfFuncs, int _MaxSize = DefaultSearchDataSize);
  ~TSearchData();
  void Clear();
  TSearchInterval* InsertInterval(TSearchInterval &pInterval); // ����� �������� (�� xl)
  void UpdateInterval(TSearchInterval &pInterval); // ���������� ��������� (�� xl)
//  TSearchInterval* GetIntervalByX(Extended x);
  TSearchInterval* GetIntervalByX(Extended x);
  // ��������� ��������� � ������������ ���-���. �������� ������� �� �������. ���� ������� �����,
  //   �� ������� ����� ������ Refill()
  TSearchInterval* GetIntervalWithMaxR();
  // ��������
  TSearchInterval* Reset();
  TSearchInterval* GetNext();
  bool IsEnd();

  // �������� ��������, ���������� � ����������
  //   �� ��������� � ���������, �� ������ ������� ���� � ���������
//  TSearchInterval* GetPrev(TSearchInterval &pInterval);

// ��� ������ � �������� �������������
  // �������, ���� ����� ������� ������ ������������ � �������
  //   ���� ��� ���� ������� �����, �� ��������� ������������
  void PushToQueue(TSearchInterval *pInterval);
  // �������������� ������� (��� �� ����������� ��� ��� ����� ������ ��������� �������)
  void RefillQueue();

  // ��������� ������ �������� �� ������ ������. ���������� �������� ��� ���
  bool IsQueueEmpty();
  bool IsQueueFull();
  void PopFromQueue(TSearchInterval **pInterval);
  void ClearQueue();

  int GetCount();

  double M[MaxNumOfFunc];//������ �������� �������
  double Z[MaxNumOfFunc];//����������� �������� ������� ������ (��� ���������� ������)

  void GetBestIntervals(TSearchInterval** intervals,int count);
};

// ------------------------------------------------------------------------------------------------
/*
class TOptimEstimation
{
protected:
  int    N;                        // number of parameters
  int    NumOfFunc;                // number of functionals
  double Y[MaxDim];                // global minimum coordinates
  double FuncValues[MaxNumOfFunc]; // values of functionals in Y point
  int    CurrentOptimumIndex;
  int    Iteration;                // number of iteration
public:
  TOptimEstimation(int _N, int _NumOfFunc);
  ~TOptimEstimation();

  int GetN() const;
  int GetNumOfFunc() const;

  const double* GetY() const;
  const double* GetFuncValues() const;
  int GetIteration() const;
  int GetCurrentOptimumIndex() const;

  double GetOptimumFuncValue();

  void SetY(const double* _Y);
  void SetFuncValues(const double* _FuncValues);
  void SetIteration(int _Iteration);
  void SetCurrentOptimumIndex(int _CurrentOptimumIndex);
};
*/

#endif
// - end of file ----------------------------------------------------------------------------------
