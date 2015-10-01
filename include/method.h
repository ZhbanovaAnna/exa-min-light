/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//             LOBACHEVSKY STATE UNIVERSITY OF NIZHNY NOVGOROD             //
//                                                                         //
//                       Copyright (c) 2015 by UNN.                        //
//                          All Rights Reserved.                           //
//                                                                         //
//  File:      method.h                                                    //
//                                                                         //
//  Purpose:   Header file for method class                                //
//                                                                         //
//  Author(s): Barkalov K., Sysoyev A.                                     //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __METHOD_H__
#define __METHOD_H__

#include "common.h"
#include "task.h"
#include "data.h"
#include "evolvent.h"
//#include "parameters.h"

// ------------------------------------------------------------------------------------------------
class TMethod
{
protected:
  int               MaxNumOfTrials;   // ������������ ����� �������� (���������)
  int               IterationCount;   // ����� ����������� ��������
  double            Epsilon;              // �������� ������� ������
  double            AchievedAccuracy; // ����������� ��������
  double            r;                // ���������� ������ (> 1)
  int               m;                // ��������� ���������� ��������� (�������� 1/2^m �� �-��)
  double            reserv;           // �������� eps-��������������, 0<=reserv<=0.1
  int               L;                // 1 <= ����� ��������� (<= m - ��� ���������)
  int               CurL;             // ����� ������� ��������� - ��������� � 0
  int               NumPoints;        // ����� �����, ����������� ������� �� 1 ��������
//  EMapType          MapType;          // ��� ��������� (���������, ���������)
  bool              recalc;           // ������, ���� ����� �������� �������������

  TTask             *pTask;
  TSearchData       *pData;
  TEvolvent         *Evolvent;        // ���������

  tIterationHandler OnIteration;

  TTrial            *pCurTrials;
  TTrial            BestTrial;

//  TParameters parameters;             // ��������� ������

  TSearchInterval** BestIntervals;    // ������ ���������� �� ����� ��������� (�����!!! �����)

  int alfa;                           // ����������� ��������� ���������, �� 1(����.) �� 20(���.) �����

  //������������� ����� � ����������� ���������
  virtual void InitAutoPrecision();
  //���������� "����������" ��������������
  virtual double CalculateGlobalR(TSearchInterval* i);
  //���������� "���������" �������������� - ������ ���� ��������� ����������!!!
  virtual double CalculateLocalR(TSearchInterval *p);
  //���������� ������ ��������� �������
  virtual void CalculateM(TSearchInterval* p);
public:
  TMethod(int _MaxNumOfTrials, double _Eps, double _r, double _reserv, int _m, int _L, int _CurL, 
    /*EMapType _MapType, /*TParameters _parameters, */TTask *_pTask, TSearchData *_pData);
  virtual ~TMethod();
  /*
  void SetTask(TTask *_pTask);
  void SetSearchData(TSearchData *_pData);
  void SetEstimation(TOptimEstimation *_pEstimation);
  */
  void SetIterationHandler(tIterationHandler _OnIteration);

  void FirstIteration();
  void InitIteration();
  //  void ReceiveLockPoints();
  void CalculateIterationPoints();
  bool CheckStopCondition();
  virtual void CalculateFunctionals();
  //  void ReceiveCalcPoints();
  void RenewSearchData();
  void EstimateOptimum();
  void IterationHandler(TProcess *pProcess); // call-back ������� ��� ��������� ����������� �������� ������
  void FinalizeIteration();
  int GetIterationCount(){return IterationCount;}

  TTrial GetOptimEstimation();
  TTrial* GetCurTrials();
  void SetBounds();
  //���� ����������
  int GetNumberOfTrials();
  double GetMu();
  //��������� ��������� �� ������� ������ �����
  void LocalSearch();

  void SetNumPoints(int _NumPoints);
  int GetNumPoints();
  TSearchInterval **GetBestIntervals();

  //�������
  int pn;
};

#endif
// - end of file ----------------------------------------------------------------------------------
