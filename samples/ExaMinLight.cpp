/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      Main.cpp                                                    //
//                                                                         //
//  Purpose:   Console version of ExaMin system                            //
//                                                                         //
//  Author(s): Sysoyev A., Barkalov K.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


//#include <exception>
//#include <cstdlib>
#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include "mpi.h"
#define _USE_MATH_DEFINES
#include <math.h>

//#ifdef WIN32
//#include <windows.h>
//#endif

#include "method.h"
#include "Rastrigin.h"
#include "common.h"
#include "exception.h"

#include "process.h"
// ------------------------------------------------------------------------------------------------
double func(const double *y, int N)
{
double f;
int i;

f =10*N;
for(i=0 ; i < N; ++i)
{
//f += ((y[i]-(N-i)/(double)N)*(y[i]-(N-i)/(double)N) - 10 * cos(2 * M_PI * (y[i]-(N-i)/(double)N)));
f += ((y[i])*(y[i]) - 10 * cos(2 * M_PI * (y[i])));
}

return f;
}

double function(const double *y)
{
double res;// = (y[0]-0.1)*(y[0]-0.1)+0.1;

res = func(y, 3);

return res;
}


int main(int argc, char* argv[])
{
 /* double A[MaxDim] = {-1.1, -1.1, -1.1, -1.1, -1.1};
  double B[MaxDim] = {1.2, 1.2, 1.2, 1.2, 1.2};

  int MaxNumOfTrials = 100000;
  double eps = 0.01;
  double r = 2.3;
  double reserv = 0.001; 
  int m = 10;
  int L = 1;
  int curl = 0;
  int N = 4;
  int NumOfFunc = 1;
  int NumPoints = 1;
  //const tFunction f[MaxNumOfFunc] = {&function};
  const tFunction f[MaxNumOfFunc] = {objfn};
  bounds(A,B);
  TTask* task = new TTask(N, N, NumOfFunc, A, B, f);
  TSearchData* pData = new TSearchData(NumOfFunc);
  bool IsStop = false;
  TMethod * method;
//  TProcess *Process;

//#ifdef WIN32
//  unexpected_function prev, cur;
//  cur = &Unexpected;
//  prev = set_unexpected(cur);
//  set_terminate(Terminate);
//#endif
  try
  {
    TTrial OptimEstimation;
    int NumberOfTrials;
    method = new TMethod(MaxNumOfTrials, eps, r, reserv, m, L, curl, NumPoints, task, pData);  
    method->SetBounds();
    method->FirstIteration();
    while (!IsStop)
    {      
      method->CalculateIterationPoints();
      IsStop = method->CheckStopCondition();
      method->CalculateFunctionals();
      method->RenewSearchData();
      method->EstimateOptimum();
      method->FinalizeIteration();
    }
 
    OptimEstimation = method->GetOptimEstimation();
    printf("min = %lf \n",OptimEstimation.FuncValues[OptimEstimation.index]);
    for(int i=0;i<task->GetN();i++)
    {
      printf("x[%d] = %lf \n",i,OptimEstimation.y[i]);
    }
    NumberOfTrials = method->GetNumberOfTrials();
    printf("NumberOfTrials = %d\n",NumberOfTrials);

    //if (parameters.TypeProcess == SynchronousProcess)
    //  Process = new TProcess(A, B, NumOfFunc, F, parameters);
    //if (parameters.TypeProcess == SynchronousProcessNew)
    //  Process = new TNewProcess(A, B, NumOfFunc, F, parameters);
    //if (parameters.TypeProcess == AsynchronousProcess)
    //  Process = new TAsyncProcess(A, B, NumOfFunc, F, parameters);

    //Process->Solve();
    //delete Process;
  }
  catch(TException e)
  {
    printf("\nEXCEPTION in file: %s, line %d, function: %s", e.GetFile(), e.GetLine(),
      e.GetFunction());
    printf("\nDESCRIPTION: %s", e.GetDescription());

    //char ExcFname[256];
    //sprintf(ExcFname, "exception_%d.txt", ProcRank);
    //FILE* pException = fopen(ExcFname, "w");
    //if (pException != NULL)
    //{
    //  fprintf(pException, "EXCEPTION in file: %s, line: %d, function: %s",
    //    e.GetFile(), e.GetLine(), e.GetFunction());
    //  fprintf(pException, "\nDESCRIPTION: %s\n", e.GetDescription());
    //  fclose(pException);
    //}
    return 1;
  }
  catch(...)
  {
    printf("\nUNKNOWN EXCEPTION !!!");
    //char ExcFname[256];
    //sprintf(ExcFname, "exception_%d.txt", ProcRank);
    //FILE* pException = fopen(ExcFname, "w");
    //if (pException != NULL)
    //{
    //  fprintf(pException, "\nUNKNOWN EXCEPTION");
    //  fclose(pException);
    //}
    return 1;
  }

*/


  // задача
  int TaskDim = 4; 
  int NumOfFunc = 1;
  double A[MaxDim] = {-1.1, -1.1, -1.1, -1.1, -1.1}; 
  double B[MaxDim] = {1.2, 1.2, 1.2, 1.2, 1.2};
  tFunction F[MaxNumOfFunc] = {objfn};//{function};
  bounds(A,B);//Границы поиска

// дерево задач и процессов
  /*int NumOfTaskLevels = 1;*/ // максимальное число уровней в дереве - 5
  /*int DimInTaskLevel[] = {4, 0, 0, 0};*/
  /*int ChildInProcLevel[] = {0, 0, 0, 0};*/ // последний уровень - листья
  int MapInLevel[] = {1, 1, 1, 1}; // пока по одной развертке на уровень

// параметры метода
  /*int MaxNumOfPoints[] = {10000, 10000, 10000, 1000};
  double Eps[] = {0.01, 0.01, 0.01, 0.01};*/
  double r = 2.3;
  int m = 10;

  int MaxNumOfPoints[] = {10000, 10000};
  double Eps[] = {0.01, 0.01};
  int NumOfTaskLevels = 2;
  int DimInTaskLevel[] = {2, 2};
  int ChildInProcLevel[] = {3, 0};

  int ProcRank, ProcNum;
  int i; 

  TProcess *process;
  
  MPI_Init(&argc, &argv);
  if (MPI_Comm_size(MPI_COMM_WORLD, &ProcNum) != MPI_SUCCESS)
  {
    throw EXCEPTION("Error in MPI_Comm_size call");
  }
  if (MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank) != MPI_SUCCESS)
  {
    throw EXCEPTION("Error in MPI_Comm_rank call");
  }

  /*unexpected_function prev, cur;
  cur = &Unexpected;
  prev = set_unexpected(cur);
  set_terminate(Terminate);*/

  try
  {
    process = new TProcess(TaskDim, A, B, NumOfFunc, F, NumOfTaskLevels,
      DimInTaskLevel, ChildInProcLevel, MaxNumOfPoints,
      Eps, r, m);
  }
  catch(TException e)
  {
    printf("\nEXCEPTION in file: %s, line %d, function: %s", e.GetFile(), e.GetLine(),
      e.GetFunction());
    printf("\nDESCRIPTION: %s", e.GetDescription());

    //char ExcFname[256];
    //sprintf(ExcFname, "exception_%d.txt", ProcRank);
    //FILE* pException = fopen(ExcFname, "w");
    //if (pException != NULL)
    //{
    //  fprintf(pException, "EXCEPTION in file: %s, line: %d, function: %s",
    //    e.GetFile(), e.GetLine(), e.GetFunction());
    //  fprintf(pException, "\nDESCRIPTION: %s\n", e.GetDescription());
    //  fclose(pException);
    //}
    return 1;
  }
  catch(...)
  {
    printf("\nUNKNOWN EXCEPTION !!!");
    //char ExcFname[256];
    //sprintf(ExcFname, "exception_%d.txt", ProcRank);
    //FILE* pException = fopen(ExcFname, "w");
    //if (pException != NULL)
    //{
    //  fprintf(pException, "\nUNKNOWN EXCEPTION");
    //  fclose(pException);
    //}
    return 1;
  }

    try
  {
  process->Solve();
    }
  catch(TException e)
  {
    printf("\nEXCEPTION in file: %s, line %d, function: %s", e.GetFile(), e.GetLine(),
      e.GetFunction());
    printf("\nDESCRIPTION: %s", e.GetDescription());

    //char ExcFname[256];
    //sprintf(ExcFname, "exception_%d.txt", ProcRank);
    //FILE* pException = fopen(ExcFname, "w");
    //if (pException != NULL)
    //{
    //  fprintf(pException, "EXCEPTION in file: %s, line: %d, function: %s",
    //    e.GetFile(), e.GetLine(), e.GetFunction());
    //  fprintf(pException, "\nDESCRIPTION: %s\n", e.GetDescription());
    //  fclose(pException);
    //}
      MPI_Finalize();
    return 1;
  }
  catch(...)
  {
    printf("\nUNKNOWN EXCEPTION !!!");
    //char ExcFname[256];
    //sprintf(ExcFname, "exception_%d.txt", ProcRank);
    //FILE* pException = fopen(ExcFname, "w");
    //if (pException != NULL)
    //{
    //  fprintf(pException, "\nUNKNOWN EXCEPTION");
    //  fclose(pException);
    //}
      MPI_Finalize();
    return 1;
  }
  delete process;
  MPI_Finalize();
  return 0;
}
// - end of file ----------------------------------------------------------------------------------
