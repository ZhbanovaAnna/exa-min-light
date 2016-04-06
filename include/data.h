/*
* Copyright (C) 2015 University of Nizhni Novgorod
* This file is part of the Globalizer Light library, v 1.0
* 
* data.h - header file for search data classes
* 
* This file may be used under the terms of the GNU Lesser General Public 
* License version 2.1 as published by the Free Software Foundation 
* and appearing in the file LICENSE.LGPL included in the packaging of this 
* file. Please review the following information to ensure the GNU Lesser 
* General Public License version 2.1 requirements will be met: 
* http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
*/ 

#ifndef __DATA_H__
#define __DATA_H__

#include "common.h"
//#include "extended.h"
#include "queue.h"
#include <stack>
//#include "parameters.h"
//#include <vector>
#include <string.h>
using namespace std;

// ------------------------------------------------------------------------------------------------
struct TTrial
{
  double x;
  double y[MaxDim];
  double FuncValues[MaxNumOfFunc];
  int index;
  int K;
};

// ------------------------------------------------------------------------------------------------
struct TSearchInterval
{
  double xl;   // левая граница интервала
  double xr;   // правая граница интервала
//  double   dx;   // обычная длина
  double   delta;// "гельдеровская" длина
  double   zl;   // значение последнего вычисленного функционала в xl
  int      izl;  // индекс последнего вычисленного фукнционала в xl
  double   zr;   // значение последнего вычисленного функционала в xr
  int      izr;  // индекс последнего вычисленного фукнционала в xr
  int      ind;  // номер итерации
  int      K;    // число "вложенных" итераций
  double   R;    // характеристика интервала (xl, xr)
  double   locR; // локальная характеристика интервала (xl, xr)
  double   *z;   // значения вычисленных функционалов в izl, кол-во - izl + 1
  TSearchInterval()
  {
    xl = 0.0;
//	dx = 0.0;
    xr = 0.0;
    zl = zr = R = locR = 0;
    izl = izr = ind = 0;
    z = NULL; // Выделение памяти происходит в момент добавления в матрицу
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
  int NumOfFuncs; // число функций задачи
  int MaxSize;    // максимальный размер МСП = максимальному числу итераций метода
  int Count;      // текущее число интервалов в дереве
  int CurIndex;   // текущий индекс, используется в итераторе
  TTreeNode *pRoot;
  TTreeNode *pCur;
  TTreeNode *pCurIter;
  stack<TTreeNode*> Stack;      // стек для итератора
  TPriorityQueue *pQueue;       // Очередь глобальных характеристик
  TPriorityQueue *pLocalQueue;  // Очередь локальных характеристик

  void DeleteTree(TTreeNode *pNode);
  unsigned char GetHeight(TTreeNode *p);
  int GetBalance(TTreeNode *p);
  void FixHeight(TTreeNode *p);
  TTreeNode* RotateRight(TTreeNode *p); // правый поворот вокруг p
  TTreeNode* RotateLeft(TTreeNode *p);  // левый поворот вокруг p
  TTreeNode* Balance(TTreeNode *p);     // балансировка узла p
  // вставка в дерево с корнем p (рекурсивная)
  TTreeNode* Insert(TTreeNode *p, TSearchInterval &pInterval);
  // поиск узла с нужным x по левой границе интервала (рекурсивный)
//  TTreeNode* Find(Extended x);
  TTreeNode* Find(TTreeNode *p, double x);
  // поиск узла по правой границе интервала
//  TTreeNode* FindR(TTreeNode *p, Extended x);
//  TParameters parameters; //параметры метода
public:
  TSearchData(/*TParameters _parameters,*/ int _NumOfFuncs, int _MaxSize = DefaultSearchDataSize);
  ~TSearchData();
  void Clear();
  TSearchInterval* InsertInterval(TSearchInterval &pInterval); // новый интервал (по xl)
  void UpdateInterval(TSearchInterval &pInterval); // обновление интервала (по xl)
//  TSearchInterval* GetIntervalByX(Extended x);
  TSearchInterval* GetIntervalByX(double x);
  // Получение интервала с максимальной хар-кой. Интервал берется из очереди. Если очередь пуста,
  //   то сначала будет вызван Refill()
  TSearchInterval* GetIntervalWithMaxR();
  // Итератор
  TSearchInterval* Reset();
  TSearchInterval* GetNext();
  bool IsEnd();

  // Получить интервал, предыдущий к указанному
  //   не относится к итератору, не меняет текущий узел в итераторе
//  TSearchInterval* GetPrev(TSearchInterval &pInterval);

// Для работы с очередью характеристик
  // вставка, если новый элемент больше минимального в очереди
  //   если при этом очередь полна, то замещение минимального
  void PushToQueue(TSearchInterval *pInterval);
  // Перезаполнение очереди (при ее опустошении или при смене оценки константы Липшица)
  void RefillQueue();

  // Остальные методы вынесены на всякий случай. Желательно обойтись без них
  bool IsQueueEmpty();
  bool IsQueueFull();
  void PopFromQueue(TSearchInterval **pInterval);
  void ClearQueue();

  int GetCount();

  double M[MaxNumOfFunc];//оценки констант Липшица
  double Z[MaxNumOfFunc];//минимальные значения функций задачи (для индексного метода)

  void GetBestIntervals(TSearchInterval** intervals,int count);
  void ResizeQueue(int size);
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
