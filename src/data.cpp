/*
* Copyright (C) 2015 University of Nizhni Novgorod
* This file is part of the Globalizer Light library, v 1.0
* 
* data.cpp - source file for search data classes
* 
* This file may be used under the terms of the GNU Lesser General Public 
* License version 2.1 as published by the Free Software Foundation 
* and appearing in the file LICENSE.LGPL included in the packaging of this 
* file. Please review the following information to ensure the GNU Lesser 
* General Public License version 2.1 requirements will be met: 
* http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
*/

#include "data.h"
#include "exception.h"

bool operator<(const TSearchInterval& i1,const TSearchInterval& i2){return (i1.xl<i2.xl);}

// ------------------------------------------------------------------------------------------------
// TSearchData Methods
// ------------------------------------------------------------------------------------------------
TSearchData::TSearchData(/*TParameters _parameters, */int _NumOfFuncs,int _MaxSize)// : parameters(_parameters)
{
    if (_MaxSize <= 0)
    {
       throw EXCEPTION("MaxSize of SearchData is out of range");
    }
	MaxSize = _MaxSize;
	Count = 0;
	pRoot = pCur = NULL;
	pQueue = new TPriorityQueue();
	//
    if (_NumOfFuncs > MaxNumOfFunc || _NumOfFuncs <= 0)
    {
       throw EXCEPTION("NumOfFunc is out of range");
    }
	NumOfFuncs = _NumOfFuncs;
	for(int i=0;i<NumOfFuncs;i++)
	{
		M[i] = 1; //parameters.M;			//Начальное значение оценок констант Липшица равно 1 
		Z[i] = MaxDouble;	//Начальное значение минимумов - максимальное
	}
}

// ------------------------------------------------------------------------------------------------
TSearchData::~TSearchData()
{
  DeleteTree(pRoot);
  delete pQueue;
}

// ------------------------------------------------------------------------------------------------
void TSearchData::Clear()
{
	DeleteTree(pRoot);
	Count = 0;
	pRoot = pCur = NULL;
	ClearQueue();
	for(int i=0;i<NumOfFuncs;i++)
	{
		M[i] = 1;//parameters.M;			//Начальное значение оценок констант Липшица равно 1 
		Z[i] = MaxDouble;	//Начальное значение минимумов - максимальное
	}
}
// ------------------------------------------------------------------------------------------------
void TSearchData::GetBestIntervals(TSearchInterval** intervals,int count)
{
	//Здесь надо взять count лучших характеристик их очереди
	//Очередь пока одна - очередь глобальных характеристик
	for(int i=0;i<count;i++)
	{
		intervals[i] = GetIntervalWithMaxR();
	}
}

// ------------------------------------------------------------------------------------------------
void TSearchData::DeleteTree(TTreeNode *pNode)
{
  if (pNode != NULL)
  {
    DeleteTree(pNode->pLeft);
    DeleteTree(pNode->pRight);
    delete pNode;
  }
}

// ------------------------------------------------------------------------------------------------
unsigned char TSearchData::GetHeight(TTreeNode *p)
{
  return p ? p->Height: 0; 
}

// ------------------------------------------------------------------------------------------------
int TSearchData::GetBalance(TTreeNode *p)
{
  return GetHeight(p->pRight) - GetHeight(p->pLeft); 
}

// ------------------------------------------------------------------------------------------------
void TSearchData::FixHeight(TTreeNode *p)
{
  unsigned char hl = GetHeight(p->pLeft);
  unsigned char hr = GetHeight(p->pRight);
  p->Height = (hl > hr ? hl: hr) + 1;
}

// ------------------------------------------------------------------------------------------------
TTreeNode* TSearchData::RotateRight(TTreeNode *p)
{
  TTreeNode *pTmp = p->pLeft;
  p->pLeft = pTmp->pRight;
  pTmp->pRight = p;
  FixHeight(p);
  FixHeight(pTmp);

  return pTmp;
}

// ------------------------------------------------------------------------------------------------
TTreeNode* TSearchData::RotateLeft(TTreeNode *p)
{
  TTreeNode *pTmp = p->pRight;
  p->pRight = pTmp->pLeft;
  pTmp->pLeft = p;
  FixHeight(p);
  FixHeight(pTmp);

  return pTmp;
}

// ------------------------------------------------------------------------------------------------
TTreeNode* TSearchData::Balance(TTreeNode *p)
{
  FixHeight(p);
  if (GetBalance(p) == 2)
  {
    if (GetBalance(p->pRight) < 0)
      p->pRight = RotateRight(p->pRight);
    return RotateLeft(p);
  }
  if (GetBalance(p) == -2)
  {
    if (GetBalance(p->pLeft) > 0)
      p->pLeft = RotateLeft(p->pLeft);
    return RotateRight(p);
  }
  return p; // балансировка не нужна
}

// ------------------------------------------------------------------------------------------------
TTreeNode* TSearchData::Insert(TTreeNode *p, TSearchInterval &pInterval)
{
  if (!p)
  {
    // не безопасно, если будет несколько потоков
    pCur = new TTreeNode(pInterval);
    return pCur;
  }
  if (pInterval.xl == p->pInterval->xl)
    throw EXCEPTION("Interval already exists!");
  if (pInterval.xl < p->pInterval->xl)
    p->pLeft = Insert(p->pLeft, pInterval);
  else
//  if (pInterval.xl > p->pInterval->xl)
    p->pRight = Insert(p->pRight, pInterval);

  return Balance(p);
}

// ------------------------------------------------------------------------------------------------
//TTreeNode* TSearchData::Find(Extended x)
TTreeNode* TSearchData::Find(TTreeNode *p, double x)
{
  TTreeNode *res;
  if (!p)
    return p;
  if (x < p->pInterval->xl)
    res = Find(p->pLeft, x);
  else if (x > p->pInterval->xl)
    res = Find(p->pRight, x);
  else
    res = p;

  return res;
}

// ------------------------------------------------------------------------------------------------
//TTreeNode* TSearchData::Find(Extended x)
/*
TTreeNode* TSearchData::FindR(TTreeNode *p, Extended x)
{
  TTreeNode *res;
  if (!p)
    return p;
  if (x < p->pInterval->xr)
    res = Find(p->pLeft, x);
  else if (x > p->pInterval->xr)
    res = Find(p->pRight, x);
  else
    res = p;

  return res;
}
*/
// ------------------------------------------------------------------------------------------------
TSearchInterval* TSearchData::InsertInterval(TSearchInterval &pInterval)
{
  pRoot = Insert(pRoot, pInterval);
  Count++;
  return pCur->pInterval;
}

// ------------------------------------------------------------------------------------------------
void TSearchData::UpdateInterval(TSearchInterval &pInterval)
{
  pCur = Find(pRoot, pInterval.xl);
  if (pCur)
  {
    (*pCur->pInterval) = pInterval;
  }
}

// ------------------------------------------------------------------------------------------------
//  TSearchInterval* GetIntervalByX(Extended x);
TSearchInterval* TSearchData::GetIntervalByX(double x)
{
  pCur = Find(pRoot, x);
  if (pCur)
    return pCur->pInterval;
  else
    return NULL;
}

// ------------------------------------------------------------------------------------------------
TSearchInterval* TSearchData::Reset()
{
  TTreeNode *pNode = pCurIter = pRoot;
  CurIndex = 0;

  while (!Stack.empty())
    Stack.pop();
  while (pNode != NULL)
  { 
    Stack.push(pNode);
    pCurIter = pNode;
    pNode = pNode->pLeft;
  }
  if (pCurIter)
    return pCurIter->pInterval;
  else 
    return NULL;
}

// ------------------------------------------------------------------------------------------------
TSearchInterval* TSearchData::GetNext()
{
  if (!IsEnd())
  {
    TTreeNode *pNode = pCurIter = pCurIter->pRight;
    Stack.pop();
    while (pNode != NULL)
    {
      Stack.push(pNode);
      pCurIter = pNode;
      pNode = pNode->pLeft;
    }
    if ((pCurIter == NULL) && !Stack.empty())
      pCurIter = Stack.top();
    CurIndex++;
    if (pCurIter)//А не надо ли тут pCurIter?
      return pCurIter->pInterval;
    else 
      return NULL;
  }
    else 
      return NULL;
  }
/*
// ------------------------------------------------------------------------------------------------
TSearchInterval* TSearchData::GetPrev(TSearchInterval &pInterval)
{
// ищем интервал, у которого pInterval.xl является правой границей
//   это и будет предыдущий 
// реализация не оптимальна (но и не безумная, благодаря сбалансированности дерева
//   чтобы сделать лучше, нужно перестраивать дерево (например, добавлять указатель на предка)
  if (CurIndex == 0)
    return NULL;
  pCur = FindR(pRoot, pInterval.xl);
  if (pCur)
    return pCur->pInterval;
  else
    return NULL;
}
*/
// ------------------------------------------------------------------------------------------------
bool TSearchData::IsEnd()
{
  return CurIndex == Count;
}

// ------------------------------------------------------------------------------------------------
TSearchInterval* TSearchData::GetIntervalWithMaxR()
{
  TSearchInterval *pRes = NULL;
  if (pQueue->IsEmpty() || pQueue->GetSize()==1)
    RefillQueue();
//  else
  PopFromQueue(&pRes);
  return pRes;
}

// ------------------------------------------------------------------------------------------------
void TSearchData::RefillQueue()
{
  TSearchInterval *p;

  pQueue->Clear(); // Если очередь уже была пуста, то вызов лишний, но он из одного действия, так
                   //   что ставить проверку нет смысла
  // Обход всех интервалов, вставка их в очередь
  //   Push() сначала заполняет очередь, потом вставляет с замещением
  for (p = Reset(); !IsEnd(); p = GetNext())
    pQueue->Push(p->R, p);
}

// ------------------------------------------------------------------------------------------------
void TSearchData::PushToQueue(TSearchInterval *pInterval)
{
  if (pInterval == 0)
  {
    throw EXCEPTION("Cannot push NULL pointer to queue.");
  }

  //На начальном этапе в очередь записываем все данные
  if ( Count <= DefaultQueueSize ) pQueue->Push(pInterval->R, pInterval);
  else
    pQueue->PushWithPriority(pInterval->R, pInterval);
}

// ------------------------------------------------------------------------------------------------
bool TSearchData::IsQueueEmpty()
{
  return pQueue->IsEmpty();
}

// ------------------------------------------------------------------------------------------------
bool TSearchData::IsQueueFull()
{
  return pQueue->IsFull();
}

// ------------------------------------------------------------------------------------------------
void TSearchData::PopFromQueue(TSearchInterval **pInterval)
{
  void *p;
  double R;
  pQueue->Pop(&R, &p);
  *pInterval = (TSearchInterval*)p;
}

// ------------------------------------------------------------------------------------------------
void TSearchData::ClearQueue()
{
  pQueue->Clear();
}
// ------------------------------------------------------------------------------------------------
int TSearchData::GetCount()
{
  return Count;
}
// ------------------------------------------------------------------------------------------------
void TSearchData::ResizeQueue(int size)
{
	pQueue->Resize(size);
}