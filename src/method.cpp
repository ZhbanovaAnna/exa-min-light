/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      method.cpp                                                  //
//                                                                         //
//  Purpose:   Source file for method class                                //
//                                                                         //
//  Author(s): Sysoyev A., Barkalov K.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#include "method.h"
//#include "local_method.h"
#include "exception.h"
#include "common.h"
#include <math.h>
#include <fstream>
using namespace std;

// ------------------------------------------------------------------------------------------------
//TMethod::TMethod(int _MaxNumOfTrials, double _Eps, double _r, int _m, int _L, EMapType _MapType,
//    TTask *_pTask, TSearchData *_pData, TOptimEstimation *_pEstimation)
TMethod::TMethod(int _MaxNumOfTrials, double _Eps, double _r, double _reserv, int _m, int _L, 
                 int _CurL, /*EMapType _MapType, /*TParameters _parameters, */TTask *_pTask, TSearchData *_pData)//:
//parameters(_parameters)
{
  MaxNumOfTrials = _MaxNumOfTrials;
  Epsilon = _Eps;
  r = _r; 
  m = _m;
  L = _L;
  CurL = _CurL;
  reserv = _reserv; // eps-��������������
  alfa = 15; // ���� ��������� ��������� - �������������
  //NumPoints = _parameters.NumPoints;;
  //MapType = _MapType;
  pTask = _pTask;
  pData = _pData;
  IterationCount = 0;
  OnIteration = 0;
  AchievedAccuracy = MaxDouble;
  // ������������� ����� � ����������� ���������
  InitAutoPrecision();
  // ������� ��������� � ������ ������������
  Evolvent = new TEvolvent(pTask->GetFreeN(), m);
  // ������ ��� ������� ��������
  pCurTrials = new TTrial[NumPoints];
  // ������ ����� ���� �� �������
  BestTrial.index = -2;

  BestIntervals = new TSearchInterval*[NumPoints];

}
// ----------------------------------------------------------------------------
void TMethod::InitAutoPrecision()
{
 // // if user has not set the precision by the command line,
 // // then set it to 1 / (2^((m + 1) * N) - 1)
 // if (Extended::GetPrecision() == 0.01)
 // {
 //   if (m * pTask->GetFreeN() <= 50)
	////����������������� ��� ������������� ���������
 //   //    if ((m + 1) * pTask->GetFreeN() < 50)
 //   {
 //     Extended::SetTypeID(etDouble);
 //   }
	////����������������� ��� ������������� ���������
 //   //    Extended::SetPrecision(1/(::pow(2., (m + 1) * pTask->GetFreeN()) - 1));
 //   Extended::SetPrecision(1/(::pow(2., m * pTask->GetFreeN()) ));
 // }
}

// ------------------------------------------------------------------------------------------------
TMethod::~TMethod()
{
  delete Evolvent;
  delete[] pCurTrials;
  delete[] BestIntervals;
}
/*
// ------------------------------------------------------------------------------------------------
void TMethod::SetTask(TTask *_pTask)
{
pTask = _pTask;
}

// ------------------------------------------------------------------------------------------------
void TMethod::SetSearchData(TSearchData *_pData)
{
pData = _pData;
}

// ------------------------------------------------------------------------------------------------
void TMethod::SetEstimation(TOptimEstimation *_pEstimation)
{
pEstimation = _pEstimation;
}
*/
// ------------------------------------------------------------------------------------------------
void TMethod::SetIterationHandler(tIterationHandler _OnIteration)
{
  OnIteration = _OnIteration;
}

// ------------------------------------------------------------------------------------------------
void TMethod::FirstIteration()
{
  int i, j;

  //�������
  //char filename[20];
  //sprintf(filename,"process_%d.dat",  pn);
  //ofstream ofs(filename);
  //ofs.close();

  // ��� ������ ��������, ���������� �������
  IterationCount = 1;
  // � ���������� ����������� ��������
  AchievedAccuracy = 1.0;
  // � ������ ��������
  BestTrial.index = -2;
  // ��������� �������� [0,1]
  TSearchInterval NewInterval;
  NewInterval.ind = IterationCount;
  NewInterval.K = 0;
  // ����� �����
  NewInterval.zl = MaxDouble;
  NewInterval.izl = -2;
  NewInterval.xl = 0.0;
  // ������ �����
  NewInterval.xr = 1.0;
//  NewInterval.dx = 1.0;
  NewInterval.zr = MaxDouble;
  NewInterval.izr = -2;
  // ������������� �����
  NewInterval.delta = 1.0;
  //��������� ��������
  TSearchInterval* p = pData->InsertInterval(NewInterval);
  // �� ������ �������� - ������������ ������ ��������
  for (i = 0; i < NumPoints; i++)
    BestIntervals[i] = p;
  // ���� ��������� - ������
  recalc = true;

  // ����� ������ �������� ���������� �� ������� �������
  // ���������� ������ NumPoints ����� c ����� h
  double h = 1.0 / (NumPoints + 1);
  for(i = 0; i < NumPoints; i++)
  {
    pCurTrials[i].x = (i + 1) * h;
    // ��������� ����� ����� �������� - ����� ������������ � ��������� ������� ������� y
    Evolvent->GetImage(pCurTrials[i].x,pCurTrials[i].y);
    // ������� ����������� ���������� � ������������ � ������� ���������
    for(j = 0; j < pTask->GetFreeN(); j++)
      pCurTrials[i].y[pTask->GetFixedN() + j] = pCurTrials[i].y[j];

    // ���������� ������������� ���������� - ��� ������ ����������� ������
    for(j = 0; j < pTask->GetFixedN(); j++)
      pCurTrials[i].y[j] = pTask->GetFixedY()[j];
  }
}

// ------------------------------------------------------------------------------------------------
void TMethod::InitIteration()
{
}
/*
// ------------------------------------------------------------------------------------------------
void TMethod::ReceiveLockPoints()
{
}
*/
// ------------------------------------------------------------------------------------------------
void TMethod::CalculateIterationPoints()
{
  if (IterationCount == 1)
  {
    return;
  }

  // ���� ������ ���� - �� ����������� ��� ��������������
  if (recalc)
  {
    // �������� ������� �������� ���������
    for (int v = 0; v <= BestTrial.index; v++)
    {
      if (v < BestTrial.index)
      {
        pData->Z[v] = -pData->M[v] * reserv;
      }
      else
      {
        if (BestTrial.FuncValues[v] != MaxDouble)
          pData->Z[v] = BestTrial.FuncValues[v];
        else 
          pData->Z[v] = 0;
      }
    }

//    TSearchInterval* si;
    pData->ClearQueue();
    for (TSearchInterval* si = pData->Reset(); !pData->IsEnd(); si = pData->GetNext())
    {
      si->R = CalculateGlobalR(si);
      si->locR = CalculateLocalR(si); 

      pData->PushToQueue(si);
    }
    // ����� ��������� ���� ��������
    recalc = false;
  }

  // ����� ���� ����� NumPoints ������ ������������� �� �������
  // ������� ���� ���� - ������� ���������� �������������
  // � ��� ������ ���� ������ ���������� ����������, �.�. �� ������ ���� ���������� NumPoints ���������
  pData->GetBestIntervals(BestIntervals, NumPoints);
  // ���� ��������� ������������ ������ CurTrials, � ������ ����������
  for (int i = 0; i < NumPoints; i++)
  {
    // ��������� x
    if (BestIntervals[i]->izl != BestIntervals[i]->izr)
    {
      pCurTrials[i].x = 0.5 * (BestIntervals[i]->xl + BestIntervals[i]->xr);
      //      pCurTrials[i].x = BestIntervals[i]->xl + 0.5*BestIntervals[i]->dx; 
    }
    else
    {
      pCurTrials[i].x = 0.5 * (BestIntervals[i]->xl + BestIntervals[i]->xr) - 
        (((BestIntervals[i]->zr - BestIntervals[i]->zl) > 0) ? 1: -1) * 
        pow(fabs(BestIntervals[i]->zr - BestIntervals[i]->zl) / 
        pData->M[BestIntervals[i]->izl], pTask->GetFreeN()) / 2 / r;
      //      pCurTrials[i].x = BestIntervals[i]->xl + (0.5*BestIntervals[i]->dx - (((BestIntervals[i]->zr - BestIntervals[i]->zl)>0)?1:-1)*pow(fabs(BestIntervals[i]->zr - BestIntervals[i]->zl)/pData->M[BestIntervals[i]->izl],pTask->GetFreeN())/(2*r));
    }

    // ��������� y
    // ��������� ����� ����� �������� - ����� ������������ � ��������� ������� ������� y
    Evolvent->GetImage(pCurTrials[i].x, pCurTrials[i].y);
    // ������� ����������� ���������� � ������������ � ������� ���������
    for (int j = 0; j < pTask->GetFreeN(); j++)
    {
      pCurTrials[i].y[pTask->GetFixedN() + j] = pCurTrials[i].y[j];
    }
    // ���������� ������������� ���������� - ��� ������ ����������� ������
    for (int j = 0; j < pTask->GetFixedN(); j++)
    {
      pCurTrials[i].y[j] = pTask->GetFixedY()[j];
    }
  }
}

// ------------------------------------------------------------------------------------------------
bool TMethod::CheckStopCondition()
{
  bool res = false;

  double CurrentAccuracy = 0.1;
  if (CurrentAccuracy < AchievedAccuracy)
    AchievedAccuracy = CurrentAccuracy;

#ifndef GKLS
  if (AchievedAccuracy < Epsilon)
    res = true;
#else
  //���� ����� �������� � ����� ������ ���������
  if(pTask->GetFixedN() == 0)
  {
	  //�� ��������� �������� ��������� �� ��������� � ����������� ��������
	res = true;
//	double cnd = pow(parameters.Epsilon, 1.0 / parameters.Dimension);
	for (int i = 0; i < parameters.Dimension; i++)
	{
		double fabsx = fabs(BestTrial.y[i] - parameters.GKLS_global_cor[i]);
		double m = parameters.Epsilon * (parameters.GKLS_param.GKLS_domain_right[i] - parameters.GKLS_param.GKLS_domain_left[i]);
		if (fabsx > m)
		{
			res = res && false;
		}
	}

  }else
  {
	  //�������� ��������� �� ��������
	if (AchievedAccuracy < Epsilon)
		res = true;
  }
#endif

  if ( IterationCount >= MaxNumOfTrials )
    res = true;

  return res;
}


// ------------------------------------------------------------------------------------------------
void TMethod::CalculateFunctionals()
{
  int i, j;
  //��� ������� ���������� ������ � ����� ������ - ������� ��������� ����������� �����
  for (i = 0; i < NumPoints; i++)
  {
    //��������� ����� ����� �������� - ����� ������������ � ��������� ������� ������� y
    Evolvent->GetImage(pCurTrials[i].x, pCurTrials[i].y);

    // ������� ����������� ���������� � ������������ � ������� ���������
    for (j = 0; j < pTask->GetFreeN(); j++)
      pCurTrials[i].y[pTask->GetFixedN() + j] = pCurTrials[i].y[j];

    // ���������� ������������� ���������� - ��� ������ ����������� �������
    for (j = 0; j < pTask->GetFixedN(); j++)
      pCurTrials[i].y[j] = pTask->GetFixedY()[j];

    //���������� �������� MaxDouble
    for (j = 0; j < pTask->GetNumOfFunc(); j++)
      pCurTrials[i].FuncValues[j] = MaxDouble;

    //������ ����� ��������� �������
    for (j = 0; j < pTask->GetNumOfFunc(); j++)
    {
      //      NumberOfEvaluations[i]++;
      pCurTrials[i].FuncValues[j] = pTask->GetFuncs()[j](pCurTrials[i].y);
      if (j == pTask->GetNumOfFunc()-1 || pCurTrials[i].FuncValues[j] > 0)
      {
        pCurTrials[i].index = j;
        break;
      }
    }
    // ��� ��� ���������� - � ����� ������, �� ��������� �������� ���
    pCurTrials[i].K = 1;
  }
}

// ------------------------------------------------------------------------------------------------
double TMethod::CalculateGlobalR(TSearchInterval *p)
{

  double deltax=p->delta;
  double value;
  int v;
  if (p->izl == p->izr)
  {
    v = p->izl;
    value = 
      deltax + (p->zr - p->zl) * (p->zr - p->zl) / (deltax * pData->M[v] * pData->M[v] * r * r) -
      2 * (p->zr + p->zl - 2 * pData->Z[v]) / (r * pData->M[v]);
  }
  if (p->izr > p->izl)
  {
    v = p->izr;
    value = 2 * deltax - 4 * (p->zr - pData->Z[v]) / (r * pData->M[v]);
  }
  if (p->izr < p->izl)
  {
    v = p->izl;
    value = 2 * deltax - 4 * (p->zl - pData->Z[v]) / (r * pData->M[v]);
  }

  return value;
}

// ------------------------------------------------------------------------------------------------
double TMethod::CalculateLocalR(TSearchInterval *p)
{
  double deltax = p->delta;
  double value;
  int v;
  if (p->izl == p->izr)
  {
    v = p->izl;
    value = 
      p->R / (sqrt((p->zr - pData->Z[v]) * (p->zl - pData->Z[v])) / pData->M[v] + pow(1.5, -alfa));
  }
  if (p->izl > p->izr)
  {
    v = p->izl;
    value = p->R / ((p->zl - pData->Z[v]) / pData->M[v] + pow(1.5, -alfa));
  }
  if (p->izl < p->izr)
  {
    v = p->izr;
    value = p->R / ((p->zr - pData->Z[v]) / pData->M[v] + pow(1.5, -alfa));
  }

  return value;
}

// ------------------------------------------------------------------------------------------------
void TMethod::CalculateM(TSearchInterval *p)
{
  // �������������� ����� � ������������� �������� �� ������������
  if (p->izl < 0 || p->izr < 0)
    return;

  // ���������� ��� ��������� ������-�����, ������ ������� ��������!!!!
  if (p->izl == p->izr)
  {
    double rpr = fabs(p->zr - p->zl) / p->delta;
    if (rpr > pData->M[p->izl] || pData->M[p->izl] == 1.0 && rpr > 0)
    {
      pData->M[p->izl] = rpr;
      recalc = true;
    }
  }
  /*
  TSearchInterval* i,p1;
  double rpr=0;
  //������� - ���� ����� ����������������� ���������� "�����"
  i=p;
  while(i->iz>=0)
  {
  i--;
  if(i->iz == tek_index) break;
  }
  //���� ������ - �������������, �� ������ �� ������
  if(i->iz >= 0)
  {
  rpr = fabs(i->z - p->z) / root( p->xl - i->xl,pTask->GetFreeN());
  if(rpr>pData->M[tek_index] || pData->M[tek_index]==1.0&&rpr>0)
  {
  pData->M[tek_index] = rpr;
  recalc = true;
  }
  }
  */
  /*
  i=p;
  while(i->izl >= 0)
  {
  //++++++++++++++++++++++++++++++++
  i = pData->GetNext();//������� - ����� ������� ������ �� ������� �����!!!!!
  //++++++++++++++++++++++++++++++++
  if(i->izl == tek_index)break;
  }
  //���� ������ - ������������� (����� ����� ���������), �� ������ �� ������
  if(i->izl>=0)
  {
  rpr = fabs(i->zl - p->zl) / root( i->xl - p->xl,pTask->GetFreeN());
  if(rpr>pData->M[tek_index] || pData->M[tek_index]==1.0&&rpr>0)
  {
  pData->M[tek_index]=rpr;
  recalc=true;
  }
  }
  */
}

// ------------------------------------------------------------------------------------------------
void TMethod::RenewSearchData()
{
  // ����� ���������� ��������� - �������� ���������� 
  for (int j = 0; j < NumPoints; j++)
  {
    // ���������� ���������
    TSearchInterval NewInterval;
    NewInterval.ind = IterationCount;
    // ���������� ����� ��������� ��������
    NewInterval.K = pCurTrials[j].K;
    // ����� ����� ��������� - ��� ����� ���������� ���������
    NewInterval.xl = pCurTrials[j].x;
    NewInterval.izl = pCurTrials[j].index;
    NewInterval.zl = pCurTrials[j].FuncValues[pCurTrials[j].index];
    // ������ ����� ��������� - ��� ������ ����� ���������, � ������� ��������� ���������
    NewInterval.xr = BestIntervals[j]->xr;
    // ����� ���������
    //		NewInterval.dx = (BestIntervals[j]->dx - (NewInterval.xl - BestIntervals[j]->xl)).toDouble();
    NewInterval.izr = BestIntervals[j]->izr;
    NewInterval.zr = BestIntervals[j]->zr;
    // ������������� ����� ���������
    //NewInterval.delta = root(NewInterval.xr - NewInterval.xl, pTask->GetFreeN());
        NewInterval.delta = pow(NewInterval.xr - NewInterval.xl, 1.0 / pTask->GetFreeN());

    // ������������ ������������ ��������
    BestIntervals[j]->xr = NewInterval.xl;
    BestIntervals[j]->izr = NewInterval.izl;
    BestIntervals[j]->zr = NewInterval.zl;
    //    BestIntervals[j]->dx -= NewInterval.dx;

    // ��������� ����������� �������� - �� ������ ������������� ����� ������� ���������
    if (BestIntervals[j]->delta < AchievedAccuracy)
    {
      AchievedAccuracy = BestIntervals[j]->delta;
    }
    // ����� ���� ��������� ����� ������������� ����� ������� ���������
    //BestIntervals[j]->delta = root(BestIntervals[j]->xr - BestIntervals[j]->xl, pTask->GetFreeN());
    BestIntervals[j]->delta = pow(BestIntervals[j]->xr - BestIntervals[j]->xl,1.0 / pTask->GetFreeN());

    // �������� ����������� - ����� ���������
    // ������� ����������� ���������, ��� �� ����������� ����������
    TSearchInterval* p = pData->InsertInterval(NewInterval);
    //��������� �������� ��������
    if (IterationCount == 1)
    {
      // �������� ��������� �������� ��� ���������
      if (j < NumPoints - 1)
      {
        BestIntervals[j + 1] = p;
      }
      recalc = true;
    }
    // ��������� ������ ���������
    CalculateM(p);
    CalculateM(BestIntervals[j]);

    // ���� ������ �������� �� ����� - ��������� ������ ������� �������������
    if (!recalc)
    {
      // ������� ��������� �� ������� �� ���� - ��� ��� ������� � GetBestIntervals
      // ��������� ��� ����� ���������
      p->R = CalculateGlobalR(p);
      pData->PushToQueue(p);

      BestIntervals[j]->R = CalculateGlobalR(BestIntervals[j]);
      pData->PushToQueue(BestIntervals[j]);
    }
  }
}

// ------------------------------------------------------------------------------------------------
void TMethod::EstimateOptimum()
{
  // ���������� �������� � ������� �������� � �������� �������
  for (int j = 0; j < NumPoints; j++)
  {
    if (pCurTrials[j].index > BestTrial.index || pCurTrials[j].index == BestTrial.index && 
      pCurTrials[j].FuncValues[BestTrial.index] < BestTrial.FuncValues[BestTrial.index])
    {
      BestTrial.index = pCurTrials[j].index;
      BestTrial.x = pCurTrials[j].x;
      memcpy(BestTrial.FuncValues, pCurTrials[j].FuncValues, pTask->GetNumOfFunc() *
        sizeof(double));

      memcpy(BestTrial.y, pCurTrials[j].y, pTask->GetN() * sizeof(double));
      // ������� ��������� - ����� ��������
      recalc = true;
    }
  }
}

// ------------------------------------------------------------------------------------------------
void TMethod::FinalizeIteration()
{
  IterationCount++;
}

// ------------------------------------------------------------------------------------------------
void TMethod::IterationHandler(TProcess *pProcess)
{
  if (OnIteration)
    OnIteration(pProcess);
}

// ------------------------------------------------------------------------------------------------
TTrial TMethod::GetOptimEstimation()
{
  return BestTrial;
}

// ------------------------------------------------------------------------------------------------
TTrial* TMethod::GetCurTrials()
{
  return pCurTrials;
}

// ------------------------------------------------------------------------------------------------
void TMethod::SetBounds()
{
  // ������ ������� ���������� ��������� ����������
  // ��������� �� ������� ���������� � ��������� - ��� ��������� �� �������� �������,
  //   ��������� �� ����� ������������� ������������
  Evolvent->SetBounds(pTask->GetA() + pTask->GetFixedN(),pTask->GetB() + pTask->GetFixedN());
}

// ------------------------------------------------------------------------------------------------
int TMethod::GetNumberOfTrials()
{
  // ���� ������ ���������� ������, �.�. ����� ������������ ����� ����� �����������
  if (pTask->GetN() == pTask->GetFixedN() + pTask->GetFreeN())
  {
    // ����� �������� ����� ����� ���������� � ������� - 1 
    return pData->GetCount() - 1;
  }
  // ���� ������ �������� ������ - ���� �������������� ��� ������ �� ����������
  int NumberOfTrials = 0;

  //TSearchInterval* i;
  for (TSearchInterval* i = pData->Reset(); !pData->IsEnd(); i = pData->GetNext())
  {
    NumberOfTrials += i->K;
  }

  return NumberOfTrials;
}

// ------------------------------------------------------------------------------------------------
double TMethod::GetMu()
{
  return pData->M[0]; // ���� �������� ������ � ����� ��������
}

// ------------------------------------------------------------------------------------------------
void TMethod::LocalSearch()
{
	//if (parameters.doLocalVerification &&
	//	GetOptimEstimation().index == pTask->GetNumOfFunc() - 1)
	//{
	//	TLocalMethod localMethod(parameters, pTask, GetOptimEstimation());
	//	BestTrial = localMethod.StartOptimization();
	//}
}

// ------------------------------------------------------------------------------------------------
void TMethod::SetNumPoints(int _NumPoints)
{
  NumPoints = _NumPoints;
}

// ------------------------------------------------------------------------------------------------
int TMethod::GetNumPoints()
{
  return NumPoints;
}

// ------------------------------------------------------------------------------------------------
TSearchInterval** TMethod::GetBestIntervals()
{
  return BestIntervals;
}
// - end of file ----------------------------------------------------------------------------------
