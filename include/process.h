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
  // ����� ������ ��� ���� ���������
  // ����� ������ ��� ���� ���������
//  int N;                 // ����������� �������� ������
//  int NumOfFunc;         // ����� ������������ ������
  int NumOfTaskLevels;   // ����� ������� � ������ �����
                         // ������������ ����� ������� � ������ - 5
  int *DimInTaskLevel;   // ����� ������������ �� ������ ������ ������ �����
                         //   ������ - NumOfTaskLevels
  int *ChildInProcLevel; // ����� �������� � ��������� �� ������� � 0 �� NumOfTaskLevels - 2
                         //   ������ - NumOfProcLevels - 1
                         //   ������� NumOfTaskLevels - 1 - ��������-������
  int *MapInLevel;       // ����� ��������� (L + 1) �� ������ ������ ������ ���������
                         //   ������ - NumOfProcLevels
                         //   ��������� ������� �� ���������� �� ������������
  int *MaxNumOfPoints;   // ������������ ����� �������� ��� ��������� �� ������ ������ 
                         //   ������ - NumOfProcLevels
  double *Eps;           // �������� ������� ������ �� ������ ������
                         //   ������ - NumOfProcLevels

  int NumOfProcLevels;   // ����� ������� � ������ ��������� = NumOfTaskLevels

  // ������� ������ ����������� ��������
  int ProcRank;           // ���� � MPI
  int ProcLevel;          // ������� �������� � ������ ���������
  int ProcParent;         // ���� ��������-��������
  int *ProcChild;         // ������� ������� ��������
                          //   ������ - ChildInProcLevel[ProcLevel]
  int *Neighbours;        // ������ ������� �������� - ��������, ������� ����� ������ ���� � �� �� ������, 
                          //   ��������� ������������� ���������
                          //   ������ - MapInLevel[ProcLevel]

  TPerformance Timer;
  double duration;        // ����� ������� ������

  bool IsOptimumFound;
  bool IsPrintFile;
  bool PrintTestInfoFile;
  TTask *pTask;
  TSearchData *pData;
//  TOptimEstimation *pEstimation;
  TMethod *pMethod;

 /* TParameters parameters;*/

  // ���������� ������ ���������, ���������� ������� ������ ��������
  void InitProcess();
  // ������ �������� �������� � ����
  void PrintOptimEstimationToFile(TTrial OptimEstimation);
  // ������ �������� �������� �� �����
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
