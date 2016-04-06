/*
* Copyright (C) 2015 University of Nizhni Novgorod
* This file is part of the Globalizer Light library, v 1.0
* 
* queue.h - header file for priority queue class
* 
* This file may be used under the terms of the GNU Lesser General Public 
* License version 2.1 as published by the Free Software Foundation 
* and appearing in the file LICENSE.LGPL included in the packaging of this 
* file. Please review the following information to ensure the GNU Lesser 
* General Public License version 2.1 requirements will be met: 
* http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
*/

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "common.h"

// ------------------------------------------------------------------------------------------------
struct TQueueElement
{
  double Key;
  void *pValue;
};

// ------------------------------------------------------------------------------------------------
// Приоритетная очередь на двоичной куче
class TPriorityQueue
{
protected:
  int MaxSize;
  int CurSize;
  TQueueElement *pMem;
  int GetIndOfMinElem();  // находит индекс минимального элемента в очереди
  void DeleteMinElem();   // удаляет минимальный элемент в очереди
  void ReBuild(int Index);
public:
  // размер очереди должен быть равен 2^k - 1
  TPriorityQueue(int _MaxSize = DefaultQueueSize);
  ~TPriorityQueue();
  int GetSize();
  bool IsEmpty();
  bool IsFull();
  // обычная вставка в очередь, пока она не полна
  //   если очередь полна, то вставка, только если новый элемент больше минимального в очереди
  // можно использовать этот метод только при перезаполнении очереди
  void Push(double key, void *value);
  // вставка, если новый элемент больше минимального в очереди
  //   если при этом очередь полна, то замещение минимального
  void PushWithPriority(double key, void *value);
  void Pop(double *key, void **value);
  void Clear();
  void Resize(int size);
};

#endif
// - end of file ----------------------------------------------------------------------------------
