/*
* Copyright (C) 2015 University of Nizhni Novgorod
* This file is part of the Globalizer Light library, v 1.0
* 
* queue.cpp - source file for priority queue class
* 
* This file may be used under the terms of the GNU Lesser General Public 
* License version 2.1 as published by the Free Software Foundation 
* and appearing in the file LICENSE.LGPL included in the packaging of this 
* file. Please review the following information to ensure the GNU Lesser 
* General Public License version 2.1 requirements will be met: 
* http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
*/

#include "queue.h"
#include "exception.h"

// ------------------------------------------------------------------------------------------------
TPriorityQueue::TPriorityQueue(int _MaxSize)
{
  int tmpPow2 = _MaxSize + 1;
  if (tmpPow2&(tmpPow2-1))
  {
    throw EXCEPTION("Max size of queue not divisible by power of two");
  }    
  MaxSize = _MaxSize;
  CurSize = 0;
  pMem = 0;
  try 
  {
    pMem = new TQueueElement[MaxSize];
  }
  catch(...)
  {
    throw EXCEPTION("Memory for queue is not allocated.");
  }

  if (pMem == 0)
  {
    throw EXCEPTION("Memory for queue is not allocated.");
  }
}

// ------------------------------------------------------------------------------------------------
TPriorityQueue::~TPriorityQueue()
{
  delete [] pMem;
}

// ------------------------------------------------------------------------------------------------
bool TPriorityQueue::IsEmpty()
{
  return CurSize == 0;
}
// ------------------------------------------------------------------------------------------------
int TPriorityQueue::GetSize()
{
  return CurSize;
}

// ------------------------------------------------------------------------------------------------
bool TPriorityQueue::IsFull()
{
  return CurSize == MaxSize;
}

// ------------------------------------------------------------------------------------------------
void TPriorityQueue::Push(double key, void *value)
{
  if (IsFull())
  {
    int MinInd = GetIndOfMinElem();
    if (key > pMem[MinInd].Key)
      DeleteMinElem();
    else 
      return;
  }
  CurSize++;
  pMem[CurSize - 1].Key = key;
  pMem[CurSize - 1].pValue = value;
  if (CurSize > 1)
    ReBuild(CurSize - 1);
}

// ------------------------------------------------------------------------------------------------
void TPriorityQueue::PushWithPriority(double key, void *value)
{
  if (CurSize == 0)
  {
    CurSize++;
    pMem[CurSize - 1].Key = key;
    pMem[CurSize - 1].pValue = value;
  }
  else
  {
    int MinInd = GetIndOfMinElem();

  // В очереди должны быть элементы с одинаковыми характеристиками!!!
    if (key >= pMem[MinInd].Key)
    {
      if (IsFull())
        DeleteMinElem();
      CurSize++;
      pMem[CurSize - 1].Key = key;
      pMem[CurSize - 1].pValue = value;
      if (CurSize > 1)
        ReBuild(CurSize - 1);
    }
  }
}

// ------------------------------------------------------------------------------------------------
void TPriorityQueue::Pop(double *key, void **value)
{
  if(CurSize != 0)
  {
      *key = pMem[0].Key;
      *value = pMem[0].pValue;
      pMem[0].Key = pMem[CurSize - 1].Key;
      pMem[0].pValue = pMem[CurSize - 1].pValue;
      CurSize--;
      if (CurSize > 1)
        ReBuild(0);
  }
  else
  {
      throw EXCEPTION("Cannot pop element from empty queue");
  }

}

// ------------------------------------------------------------------------------------------------
int TPriorityQueue::GetIndOfMinElem()
{
  int i, StartIndex;
  double min = MaxDouble;
  int MinIndex = -1;

  // Находим левую границу для поиска минимума. Левая граница - первый лист в куче.
  // Если последний уровень дерева заполнен не до конца, то первый лист в предпоследнем уровне.
  // Если у узла есть потомки, то в этом узле не может быть минимума (по свойству d-кучи)
  if (CurSize % 2) // Элемент CurSize - левый потомок своего предка
    StartIndex = (CurSize - 1) / 2; // предок первого свободного листа в куче - граница поиска
  else // Элемент CurSize - правый потомок своего предка
    StartIndex = (CurSize - 1) / 2 + 1; // граница поиска - следующий предок за предком CurSize

  for (i = StartIndex; i < CurSize; i++)
    if (min > pMem[i].Key)
    {
      MinIndex = i;
      min = pMem[i].Key;
    }
    
  return MinIndex;
}

// ------------------------------------------------------------------------------------------------
void TPriorityQueue::DeleteMinElem()
{
  int MinInd = GetIndOfMinElem();
  pMem[MinInd].Key = pMem[CurSize - 1].Key;
  pMem[MinInd].pValue = pMem[CurSize - 1].pValue;
  CurSize--;
  if (CurSize > 1)
    ReBuild(MinInd);
}

// ------------------------------------------------------------------------------------------------
void TPriorityQueue::ReBuild(int Index)
{
  int i, j, k, level = 0;
  if (Index == 0) // восстановление структуры двоичной кучи от корня (погружение)
  {
    i = Index;
    j = 2 * i + 1; // левый потомок корня (всегда существует)
    k = 2 * i + 2; // правый потомок корня (может отсутствовать)
//    if (k < CurSize - 1) // оба потомка существуют
    if (k < CurSize) // оба потомка существуют
      if (pMem[j].Key < pMem[k].Key)
        j = k;
    // теперь j - наибольший потомок узла i
    while (true)
    {
      if (pMem[i].Key >= pMem[j].Key)
        break;
      double tmp = pMem[i].Key;
      void *ptmp = pMem[i].pValue;
      pMem[i].Key = pMem[j].Key;
      pMem[i].pValue = pMem[j].pValue;
      pMem[j].Key = tmp;
      pMem[j].pValue = ptmp;
      i = j;
      j = 2 * i + 1; 
      k = 2 * i + 2; 
      if (j > CurSize - 1) // у узла i нет потомков
        break;
//      if (k < CurSize - 1) // оба потомка существуют
      if (k < CurSize) // оба потомка существуют
        if (pMem[j].Key < pMem[k].Key)
          j = k;
    }
  }
  else // восстановление структуры двоичной кучи от листа (всплытие)
  {
    i = Index;
    j = (i - 1) / 2; // предок узла i
    while ((i > 0) && (pMem[j].Key <= pMem[i].Key)) //А можно ли тут поставить <= вместо < ?
    {
      double tmp = pMem[i].Key;
      void *ptmp = pMem[i].pValue;
      pMem[i].Key = pMem[j].Key;
      pMem[i].pValue = pMem[j].pValue;
      pMem[j].Key = tmp;
      pMem[j].pValue = ptmp;
      i = j;
      j = (i - 1) / 2;
    }
  }
}

void TPriorityQueue::Clear()
{
  CurSize = 0;
}
// ------------------------------------------------------------------------------------------------
void TPriorityQueue::Resize(int size)
{
	CurSize = 0;
	MaxSize = size;
	delete pMem;
    pMem = new TQueueElement[MaxSize];
	/*pMem = new MinMaxHeap< TQueueElement, _less >(MaxSize);*/
}
// - end of file ----------------------------------------------------------------------------------
