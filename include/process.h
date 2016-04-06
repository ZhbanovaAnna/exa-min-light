/*
* Copyright (C) 2015 University of Nizhni Novgorod
* This file is part of the Globalizer Light library, v 1.0
* 
* process.h - file for process class
* 
* This file may be used under the terms of the GNU Lesser General Public 
* License version 2.1 as published by the Free Software Foundation 
* and appearing in the file LICENSE.LGPL included in the packaging of this 
* file. Please review the following information to ensure the GNU Lesser 
* General Public License version 2.1 requirements will be met: 
* http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
*/

#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "data.h"
#include "method.h"
#include "task.h"
#include "exception.h"
/*#include "parameters.h"*/
#include "performance.h"

//extern const int MaxNumOfTaskLevels;

// ------------------------------------------------------------------------------------------------
class TProcess
{
protected:
  // ќбщие данные дл€ всех процессов
  // ќбщие данные дл€ всех процессов
//  int N;                 // размерность исходной задачи
//  int NumOfFunc;         // число функционалов задачи
  int NumOfTaskLevels;   // число уровней в дереве задач
                         // максимальное число уровней в дереве - 5
  int *DimInTaskLevel;   // число размерностей на каждом уровне дерева задач
                         //   размер - NumOfTaskLevels
  int *ChildInProcLevel; // число потомков у процессов на уровн€х с 0 до NumOfTaskLevels - 2
                         //   размер - NumOfProcLevels - 1
                         //   уровень NumOfTaskLevels - 1 - процессы-листь€
  int *MapInLevel;       // число разверток (L + 1) на каждом уровне дерева процессов
                         //   размер - NumOfProcLevels
                         //   последний уровень по разверткам не параллелитс€
  int *MaxNumOfPoints;   // максимальное число итераций дл€ процессов на каждом уровне 
                         //   размер - NumOfProcLevels
  double *Eps;           // точность решени€ задачи на каждом уровне
                         //   размер - NumOfProcLevels

  int NumOfProcLevels;   // число уровней в дереве процессов = NumOfTaskLevels

  // „астные данные конкретного процесса
  int ProcRank;           // ранг в MPI
  int ProcLevel;          // уровень процесса в дереве процессов
  int ProcParent;         // ранг процесса-родител€
  int *ProcChild;         // потомки данного процесса
                          //   размер - ChildInProcLevel[ProcLevel]
  int *Neighbours;        // соседи данного процесса - процессы, которые будут решать одну и ту же задачу, 
                          //   использу€ множественную развертку
                          //   размер - MapInLevel[ProcLevel]

  TPerformance Timer;
  double duration;        // врем€ решени€ задачи

  bool IsOptimumFound;
  bool IsPrintFile;
  bool PrintTestInfoFile;
  TTask *pTask;
  TSearchData *pData;
//  TOptimEstimation *pEstimation;
  TMethod *pMethod;

 /* TParameters parameters;*/

  // построение дерева процессов, заполнение частных данных процесса
  void InitProcess();
  // печать текущего минимума в файл
  void PrintOptimEstimationToFile(TTrial OptimEstimation);
  // печать текущего минимума на экран
  virtual void PrintOptimEstimationToConsole(TTrial OptimEstimation);

  virtual void BeginIterations();
  virtual void DoIteration();
  virtual void EndIterations();
  virtual void CalculateFunctionals();

  void ReceiveLockPoints();
  void ReceiveCalcPoints();
public:
  TProcess(int _N, double *_A, double *_B, int _NumOfFunc, const tFunction* _F,
           int _NumOfTaskLevels, int *_DimInTaskLevel, int *_ChildInProcLevel,
           int *_MaxNumOfPoints, double *_Eps, double _r, int _m, bool _IsPrintFile = false, bool _PrintTestInfoFile = false);
  ~TProcess();
  double GetSolveTime();
  void Solve();
  int GetProcLevel(){return ProcLevel;}
  int GetIterationCountRoot();
};

/*void ShowIterResults(TProcess *pProcess);*/

#endif
// - end of file ----------------------------------------------------------------------------------
