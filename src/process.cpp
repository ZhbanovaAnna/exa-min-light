/*
* Copyright (C) 2015 University of Nizhni Novgorod
* This file is part of the Globalizer Light library, v 1.0
* 
* process.cpp - source file for process class
* 
* This file may be used under the terms of the GNU Lesser General Public 
* License version 2.1 as published by the Free Software Foundation 
* and appearing in the file LICENSE.LGPL included in the packaging of this 
* file. Please review the following information to ensure the GNU Lesser 
* General Public License version 2.1 requirements will be met: 
* http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
*/
#define _CRT_SECURE_NO_WARNINGS
#include <mpi.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "common.h"
#include "process.h"

#define DEBUG_MPI

// ------------------------------------------------------------------------------------------------
void ShowIterResults(TProcess *pProcess)
{
}

// ------------------------------------------------------------------------------------------------
TProcess::TProcess(int _N, double *_A, double *_B, int _NumOfFunc,
                   const tFunction* _F,  int _NumOfTaskLevels, 
                   int *_DimInTaskLevel, int *_ChildInProcLevel,
                   int *_MaxNumOfPoints, double *_Eps, double _r, int _m, bool _IsPrintFile, bool _PrintTestInfoFile) 
{
  //Clear file
  FILE* pf;
  pf = fopen("current_state.dat","w");
  fclose(pf);
  if(_NumOfTaskLevels <= 0)
  {
    throw string("NumOfTaskLevels is out of range");
  }
  NumOfTaskLevels = _NumOfTaskLevels;
  IsPrintFile = _IsPrintFile;
  PrintTestInfoFile = _PrintTestInfoFile;
  NumOfProcLevels = NumOfTaskLevels;
  DimInTaskLevel = new int[NumOfTaskLevels];
  ChildInProcLevel = new int[NumOfProcLevels];
  MaxNumOfPoints = new int[NumOfProcLevels];
  MapInLevel = new int[NumOfTaskLevels];
  Eps = new double[NumOfProcLevels];
  int dimTmp = 0;
  for(int i = 0; i < NumOfTaskLevels; i++)
  {
    dimTmp += _DimInTaskLevel[i];
  }
  if (dimTmp != _N)
  {
    throw string("Illegal value of DimInTaskLevel");
  }
  memcpy(DimInTaskLevel, _DimInTaskLevel, NumOfTaskLevels * sizeof(int)); 

  for(int i = 0; i < NumOfTaskLevels; i++)
  {
    if(_ChildInProcLevel[i] < 0)
    {
      throw string("ChildInProcLevel is out of range");
    }
  }    

  memcpy(ChildInProcLevel, _ChildInProcLevel, NumOfProcLevels * sizeof(int)); 
  memcpy(MaxNumOfPoints, _MaxNumOfPoints, NumOfProcLevels * sizeof(int)); 
  memcpy(Eps, _Eps, NumOfProcLevels * sizeof(double)); 
  ProcChild = NULL;
  Neighbours = NULL;
  for(int i = 0; i < NumOfTaskLevels; i++)
  {
    MapInLevel[i] = 1;
  }

  InitProcess();

  pTask = new TTask(_N, DimInTaskLevel[ProcLevel], _NumOfFunc, _A, _B, _F);
  pData = new TSearchData(/*parameters,*/ _NumOfFunc, DefaultSearchDataSize);

  // число порождаемых точек на итерации совпадает с числом потомков в дереве процессов,
  //  если процесс - не лист
 /* if (ProcLevel < NumOfProcLevels - 1)
    parameters.NumPoints = parameters.ChildInProcLevel[ProcLevel];
  TMethodFactory::CreateMethod(&pMethod, 
    parameters.MaxNumOfPoints[ProcLevel], 
    parameters.Eps[ProcLevel], 
    parameters.rs[ProcLevel], 
	parameters.rEps, // eps-reserv
    parameters.m, 
    parameters.MapInLevel[ProcLevel],  
    0, // номер развертки - можно вычислить, пока одна развертка с номером 0
    parameters.MapType, // вид развертки
    parameters,
    pTask, pData);*/
 pMethod = new TMethod(MaxNumOfPoints[ProcLevel], _Eps[ProcLevel], _r, 0.01/*eps-reserv*/, _m, MapInLevel[ProcLevel],  
	  0,//номер развертки - можно вычислить, пока одна развертка с номером 0	  
    /*_MapType,*///Вид развертки
    (ProcLevel < NumOfProcLevels - 1)?ChildInProcLevel[ProcLevel]:1,
// 	(NumOfProcLevels>1)?ChildInProcLevel[ProcLevel]:1,//число порождаемых точек на итерации - совпадает с числом потомков в дереве процессов,
													  //или же равно 1 при решении в 1 узле дерева
	pTask, pData);
  // отладка 
  int ProcNum; 
  MPI_Comm_rank(MPI_COMM_WORLD, &ProcNum);
  pMethod->pn = ProcNum;
  //================

  pMethod->SetIterationHandler(ShowIterResults);
}

// ------------------------------------------------------------------------------------------------
TProcess::~TProcess()
{
  delete [] ProcChild;
  delete [] Neighbours;

  delete pTask;
  delete pData;
  delete pMethod;
}

// ------------------------------------------------------------------------------------------------
double TProcess::GetSolveTime()
{
  return duration;
}

// ------------------------------------------------------------------------------------------------
void TProcess::InitProcess()
{
  int ProcNum;           // общее число процессов в MPI
  int TotalNumOfProc;    // общее число процессов в дереве процессов
  int *BordersInProcNum;
  int i, j;

  if (MPI_Comm_size(MPI_COMM_WORLD, &ProcNum) != MPI_SUCCESS)
  {
    throw EXCEPTION("Error in MPI_Comm_size call");
  }
  if (MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank) != MPI_SUCCESS)
  {
    throw EXCEPTION("Error in MPI_Comm_rank call");
  }

  int numProc = 0;
  int numProcOnLevel = 1;

  for(int i = 0; i < NumOfTaskLevels; i++)
  {
    numProc += numProcOnLevel;
    numProcOnLevel = numProcOnLevel * ChildInProcLevel[i];
  }
  if(numProc != ProcNum)
  {
    throw string("Illegal value of ChildInProcLevel");
  }

//pMethod->pn = ProcRank;
  // вычисляем границы уровней в дереве процессов - BordersInProcNum
  BordersInProcNum = new int[NumOfProcLevels];
  int MapMultiplier = 1;//MapInLevel[0];
  int ChildMultiplier = 1;
  BordersInProcNum[0] = 1;//MapInLevel[0];
  for (i = 1; i < NumOfProcLevels; i++)
  {
    MapMultiplier = MapMultiplier * MapInLevel[i];
    ChildMultiplier = ChildMultiplier * ChildInProcLevel[i - 1];
    BordersInProcNum[i] = BordersInProcNum[i - 1] + ChildMultiplier * MapMultiplier;
  }
  TotalNumOfProc = BordersInProcNum[NumOfProcLevels - 1];
  if (TotalNumOfProc < ProcNum)
  {
    throw EXCEPTION("Wrong proc number");
  }

  //ProcRank = 2;
  // нулевой уровень в дереве - "корни"
  if (ProcRank < /*MapInLevel[0]*/1)
  {
    ProcLevel = 0;
    ProcParent = -1;
    ProcChild = new int [ChildInProcLevel[ProcLevel]];
    for (i = 0; i < ChildInProcLevel[ProcLevel]; i++)
      ProcChild[i] = MapInLevel[0] + (ProcRank * ChildInProcLevel[ProcLevel] + i) * MapInLevel[1];
  }
  else
  {
    int BorderAdditive;

    // вычисляем уровень процесса в дереве процессов
    ProcLevel = NumOfProcLevels - 1;
    for (i = NumOfProcLevels - 2; i >=1; i--)
      if (ProcRank < BordersInProcNum[i])
        ProcLevel = i;

    // вычисляем номер процесса-родителя
    MapMultiplier = MapInLevel[ProcLevel];
    BorderAdditive = 0;
    if (ProcLevel > 1)
      BorderAdditive = BordersInProcNum[ProcLevel - 2];
    ProcParent = (ProcRank - BordersInProcNum[ProcLevel - 1]) / 
      (ChildInProcLevel[ProcLevel - 1] * MapMultiplier) + BorderAdditive;

    // вычисляем номера процессов-потомков (потомки - "нулевые процессы" в своих группах)
    if (ProcLevel == NumOfProcLevels - 1)
    {
      ProcChild = new int[0];
    }
    if (ProcLevel < NumOfProcLevels - 1)
    {
      ProcChild = new int [ChildInProcLevel[ProcLevel]];
      MapMultiplier = 1;
      if (ProcLevel < NumOfProcLevels - 2)
        MapMultiplier = MapInLevel[ProcLevel + 1];
      for (i = 0; i < ChildInProcLevel[ProcLevel]; i++)
        ProcChild[i] = BordersInProcNum[ProcLevel] + ((ProcRank - BordersInProcNum[ProcLevel - 1]) * 
        ChildInProcLevel[ProcLevel] + i) * MapMultiplier;
    }
  }

  // вычисляем соседей - процессы, которые будут решать одну и ту же задачу, 
  //  используя множественную развертку
  int Remainder = (ProcRank - BordersInProcNum[ProcLevel - 1]) % 1/*MapInLevel[ProcLevel]*/;
  int st = ProcRank - Remainder;
  int fin = st + MapInLevel[ProcLevel];
  Neighbours = new int[MapInLevel[ProcLevel] - 1];
  j = 0;
  for (i = st; i < fin; i++)
  {
    if (i == ProcRank)
      continue;
    Neighbours[j++] = i;
  }

  delete [] BordersInProcNum;
}

// ------------------------------------------------------------------------------------------------
void TProcess::Solve()
{
  int FixedN;
  double FixedY[MaxDim];
  TTrial OptimEstimation, NewOptimEstimation;
  //  double OptimVal,NewOptimVal;
  int NumberOfTrials;
  MPI_Status status;
//  FILE* pf;

  if (ProcLevel == 0)
  {
    int qSize = max((int)pow(2.0, (int)(log((double)MaxNumOfPoints[ProcLevel])/log(2.0) - 2)) - 1, DefaultQueueSize);
    pData->ResizeQueue(qSize);

    Timer.Start();
    BeginIterations();
    OptimEstimation = pMethod->GetOptimEstimation();//.FuncValues[pMethod->GetOptimEstimation().index];
    while (!IsOptimumFound)
    {
      DoIteration();
      NewOptimEstimation = pMethod->GetOptimEstimation();
      if (NewOptimEstimation.FuncValues[NewOptimEstimation.index] != 
        OptimEstimation.FuncValues[OptimEstimation.index])
      {
        OptimEstimation = NewOptimEstimation;
        duration = Timer.GetTime();
        PrintOptimEstimationToFile(OptimEstimation);
      }
    }
    EndIterations();

    duration = Timer.GetTime();

    OptimEstimation = pMethod->GetOptimEstimation();

    PrintOptimEstimationToFile(OptimEstimation);

    //duration = Timer.GetTime();
    //if (parameters.IsPrintFile)
    //{
    //  pf = fopen("optim.dat", "a");

    //  fprintf(pf, "Iteration = %d\n", pMethod->GetIterationCount());
    //  fprintf(pf, "min = %lf \n", OptimEstimation.FuncValues[OptimEstimation.index]);
    //  for(int i = 0; i < pTask->GetN(); i++)
    //  {
    //    fprintf(pf, "x[%d] = %lf \n", i, OptimEstimation.y[i]);
    //  }
    //  fprintf(pf, "\nMu = %lf\n", pMethod->GetMu());
    //  NumberOfTrials = pMethod->GetNumberOfTrials();
    //  fprintf(pf, "NumberOfTrials = %d\n", NumberOfTrials);
    //  fprintf(pf, "Solve time = %f \n\n\n", duration);

    //  fclose(pf);
    //}

   /* PrintOptimEstimationToConsole(OptimEstimation);*/

    //printf("ProcLevel = %d\n", ProcLevel);
    //printf("Iteration = %d \n", pMethod->GetIterationCount());
    ////printf("Point = %d \n",pMethod->GetIterationCount() * parameters.NumPoints);
    //printf("Point = %d \n", pMethod->GetIterationCount() * pMethod->GetNumPoints());

    //printf("min = %lf \n", OptimEstimation.FuncValues[OptimEstimation.index]);
    //double maxc = 0;
    //for (int i = 0;i < pTask->GetN(); i++)
    //{
    //  printf("x[%d] = %lf \n", i, OptimEstimation.y[i]);
    //  if (fabs(OptimEstimation.y[i]) > maxc)
    //    maxc = fabs(OptimEstimation.y[i]);
    //}
    //printf("fmaxc = %lf \n",maxc);

    //NumberOfTrials = pMethod->GetNumberOfTrials();
    //printf("NumberOfTrials = %d\n", NumberOfTrials);

    //printf("\nSolve time = %lf", duration);
  }
  else
  {
    int qSize = max((int)pow(2.0, (int)(log((double)MaxNumOfPoints[ProcLevel]) / 
                     log(2.0) - 2)) - 1, DefaultQueueSize);
    pData->ResizeQueue(qSize);

    while (true)
    {
      MPI_Recv(&FixedN, 1, MPI_INT, ProcParent, TagChildStartSolve, MPI_COMM_WORLD, &status);
      if (FixedN == ChildStopMsg)
        break;
      MPI_Recv(FixedY, FixedN, MPI_DOUBLE, ProcParent, TagChildStartSolve, 
        MPI_COMM_WORLD, &status);
      pTask->SetFixed(FixedN, FixedY);

      pData->Clear();
      Timer.Start();
      BeginIterations();
      while (!IsOptimumFound)
      {
        DoIteration();
      }
      EndIterations();

      duration = Timer.GetTime();
      OptimEstimation = pMethod->GetOptimEstimation();
      NumberOfTrials = pMethod->GetNumberOfTrials();
      // Посылаем индекс точки
      MPI_Send(&OptimEstimation.index, 1, MPI_INT, ProcParent, TagChildSolved, MPI_COMM_WORLD);
      // Посылаем точку
      MPI_Send(OptimEstimation.y, pTask->GetN(), MPI_DOUBLE, ProcParent,
        TagChildSolved, MPI_COMM_WORLD);
      // Посылаем вычисленные значения в точке
      MPI_Send(OptimEstimation.FuncValues, pTask->GetNumOfFunc(), MPI_DOUBLE, ProcParent, 
        TagChildSolved, MPI_COMM_WORLD);
      // Посылаем число вложенных итераций
      MPI_Send(&NumberOfTrials, 1, MPI_INT, ProcParent, TagChildSolved, MPI_COMM_WORLD);
    }

    if (ProcLevel < NumOfProcLevels - 1)
    {
      int Msg, i;
      Msg = ChildStopMsg;
      for (i = 0; i < ChildInProcLevel[ProcLevel]; i++)
        MPI_Send(&Msg, 1, MPI_INT, ProcChild[i], TagChildStartSolve, MPI_COMM_WORLD);
    }
  }
}

// ------------------------------------------------------------------------------------------------
void TProcess::PrintOptimEstimationToFile(TTrial OptimEstimation)
{
  int NumberOfTrials;
  FILE* pf;

  if (IsPrintFile)
  {
    pf = fopen("optim.dat","a");

    fprintf(pf, "Iteration = %d\n", pMethod->GetIterationCount());
    fprintf(pf, "min = %lf \n", OptimEstimation.FuncValues[OptimEstimation.index]);
    for (int i = 0; i < pTask->GetN(); i++)
      fprintf(pf, "x[%d] = %lf \n", i, OptimEstimation.y[i]);

    fprintf(pf, "\n");
	for (int i = 0; i < pTask->GetNumOfFunc(); i++)
      fprintf(pf, "M[%d] = %lf \n", i, pMethod->GetM()[i]);

    NumberOfTrials = pMethod->GetNumberOfTrials();
    fprintf(pf, "NumberOfTrials = %d\n", NumberOfTrials);
    fprintf(pf, "Solve time = %f \n\n\n", duration);

    fclose(pf);
  }
}

// ------------------------------------------------------------------------------------------------
void TProcess::PrintOptimEstimationToConsole(TTrial OptimEstimation)
{
int NumberOfTrials;

  printf("ProcLevel = %d\n", ProcLevel);
  printf("Iteration = %d \n", pMethod->GetIterationCount());
  printf("Point = %d \n",pMethod->GetIterationCount() * pMethod->GetNumPoints());

  printf("min = %lf \n", OptimEstimation.FuncValues[OptimEstimation.index]);
  for (int i = 0; i < pTask->GetN(); i++)
  {
    printf("x[%d] = %lf \n", i, OptimEstimation.y[i]);
  }
 // printf("fmaxc = %lf \n", maxDeviation(OptimEstimation.y));

  //printf("dif = %lf \n", maxDifference(OptimEstimation.FuncValues[OptimEstimation.index]));

  NumberOfTrials = pMethod->GetNumberOfTrials();

  printf("\nNumberOfTrials = %d\n", NumberOfTrials);

  printf("\nSolve time = %lf\n\n\n", duration);

}

// ------------------------------------------------------------------------------------------------
void TProcess::BeginIterations()
{
  IsOptimumFound = false;

  pMethod->SetBounds();

  pMethod->FirstIteration();
}

// ------------------------------------------------------------------------------------------------
void TProcess::DoIteration()
{
  bool IsStop;
  if(pMethod->GetIterationCount() % 10 == 0 && ProcRank == 0)
  {  
    //printf("PrintCurrentStateToFile  %d \n", pMethod->GetIterationCount());
    pMethod->PrintCurrentStateToFile("current_state.dat");
  }
  pMethod->InitIteration();
  ReceiveLockPoints();
    //if(ProcRank == 0)
      //printf("CalculateIterationPoints   ");
  pMethod->CalculateIterationPoints();
  //if(ProcRank == 0)
     // printf("ReceiveLockPoints   ");
  ReceiveLockPoints();
      //if(ProcRank == 0)
      //printf("CheckStopCondition   ");
  IsStop = pMethod->CheckStopCondition();
  //if(ProcRank == 0)
    //printf("before isstop   \n");
  if (!IsStop)
  {
    if (ProcLevel == NumOfProcLevels - 1)
      pMethod->CalculateFunctionals();
    else
      CalculateFunctionals();
    ReceiveCalcPoints();
	//Провести оценку оптимума нужно до обновления данных,  т.к. в этой функции может быть поднят флаг recalc
    pMethod->EstimateOptimum();
	//Все случаи поднятия флага recalc обработаны, можно обновлять базу
    pMethod->RenewSearchData();
    pMethod->FinalizeIteration();
    pMethod->IterationHandler(this);
  }
  else
  {
    IsOptimumFound = true;

    if (ProcLevel == 0)
    {
      int Msg, i;
      Msg = ChildStopMsg;
      for (i = 0; i < ChildInProcLevel[ProcLevel]; i++)
        MPI_Send(&Msg, 1, MPI_INT, ProcChild[i], TagChildStartSolve, MPI_COMM_WORLD);
    }
  }
}

// ------------------------------------------------------------------------------------------------
void TProcess::EndIterations()
{
  //Локальное уточнение найденного решения
  pMethod->LocalSearch();
}

// ------------------------------------------------------------------------------------------------
void TProcess::ReceiveLockPoints()
{

}

// ------------------------------------------------------------------------------------------------
void TProcess::ReceiveCalcPoints()
{

}

// ------------------------------------------------------------------------------------------------
void TProcess::CalculateFunctionals()
{
  MPI_Status status;
  TTrial *CurTrials;
  int FixedN;
  double FixedY[MaxDim];
  int i, j;

  CurTrials = pMethod->GetCurTrials();
  for(i = 0; i < ChildInProcLevel[ProcLevel]; i++)
    for(j = 0; j < pTask->GetNumOfFunc(); j++)
      CurTrials[i].FuncValues[j] = MaxDouble;

  FixedN = pTask->GetFixedN() + DimInTaskLevel[ProcLevel];
  for (i = 0; i < ChildInProcLevel[ProcLevel]; i++)
  {
    MPI_Send(&FixedN, 1, MPI_INT, ProcChild[i], TagChildStartSolve, MPI_COMM_WORLD);
    // Фиксированные переменные должны возникать из массива текущих итераций
    memcpy(FixedY, CurTrials[i].y, FixedN * sizeof(double));
    MPI_Send(FixedY, FixedN, MPI_DOUBLE, ProcChild[i], TagChildStartSolve, MPI_COMM_WORLD);
  }

  for (i = 0; i < ChildInProcLevel[ProcLevel]; i++)
  {
    // Принимаем индекс точки
    MPI_Recv(&CurTrials[i].index, 1, MPI_INT, ProcChild[i],
      TagChildSolved, MPI_COMM_WORLD, &status);
    // Принимаем точку
    MPI_Recv(CurTrials[i].y, pTask->GetN(), MPI_DOUBLE, ProcChild[i], 
      TagChildSolved, MPI_COMM_WORLD, &status);
    // Принимаем значения функционалов
    MPI_Recv(CurTrials[i].FuncValues, pTask->GetNumOfFunc(), MPI_DOUBLE, ProcChild[i],
      TagChildSolved, MPI_COMM_WORLD, &status);
    // Принимаем число вложенных итераций
    MPI_Recv(&CurTrials[i].K, 1, MPI_INT, ProcChild[i],
      TagChildSolved, MPI_COMM_WORLD, &status);
  }
}

int TProcess::GetIterationCountRoot()
{
  return pMethod->GetIterationCount();
}

// - end of file ----------------------------------------------------------------------------------
