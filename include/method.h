/*
* Copyright (C) 2015 University of Nizhni Novgorod
* This file is part of the Globalizer Light library, v 1.0
* 
* method.h - file for method class
* 
* This file may be used under the terms of the GNU Lesser General Public 
* License version 2.1 as published by the Free Software Foundation 
* and appearing in the file LICENSE.LGPL included in the packaging of this 
* file. Please review the following information to ensure the GNU Lesser 
* General Public License version 2.1 requirements will be met: 
* http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
*/

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
  int               MaxNumOfTrials;   // максимальное число итераций (испытаний)
  int               IterationCount;   // число выполненных итераций
  double            Epsilon;              // точность решения задачи
  double            AchievedAccuracy; // достигнутая точность
  double            r;                // надежность метода (> 1)
  int               m;                // плотность построения развертки (точность 1/2^m по к-те)
  double            reserv;           // параметр eps-резервирования, 0<=reserv<=0.1
  int               L;                // 1 <= число разверток (<= m - для сдвиговой)
  int               CurL;             // номер текущей развертки - нумерация с 0
  int               NumPoints;        // число точек, порождаемых методом на 1 итерации
//  EMapType          MapType;          // тип развертки (сдвиговая, вращаемая)
  bool              recalc;           // истина, если нужен пересчет характеристик

  TTask             *pTask;
  TSearchData       *pData;
  TEvolvent         *Evolvent;        // развертка

  tIterationHandler OnIteration;

  TTrial            *pCurTrials;
  TTrial            BestTrial;

//  TParameters parameters;             // параметры метода

  TSearchInterval** BestIntervals;    // массив указателей на лучше интервалы (левая!!! точка)

  int alfa;                           // Коэффициент локальной адаптации, от 1(глоб.) до 20(лок.) поиск

  //Инициализация чисел с расширенной точностью
  virtual void InitAutoPrecision();
  //Вычисление "глобальной" характеристики
  virtual double CalculateGlobalR(TSearchInterval* i);
  //Вычисление "локальной" характеристики - должна быть вычислена глобальная!!!
  virtual double CalculateLocalR(TSearchInterval *p);
  //Вычисление оценки константы Липшица
  virtual void CalculateM(TSearchInterval* p);
public:
  TMethod(int _MaxNumOfTrials, double _Eps, double _r, double _reserv, int _m, int _L, int _CurL,  int _NumPoints,
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
  void IterationHandler(TProcess *pProcess); // call-back функция для обработки результатов итерации метода
  void FinalizeIteration();
  int GetIterationCount(){return IterationCount;}

  TTrial GetOptimEstimation();
  TTrial* GetCurTrials();
  void SetBounds();
  //Сбор статистики
  int GetNumberOfTrials();
  double GetMu();
  const double* GetM();
  //Локальное уточнение из текущей лучшей точки
  void LocalSearch();

  void SetNumPoints(int _NumPoints);
  int GetNumPoints();
  TSearchInterval **GetBestIntervals();
  double GetAchievedAccuracy();
  void PrintCurrentStateToFile(char* nameOfFile);

  //отладка
  int pn;
};

#endif
// - end of file ----------------------------------------------------------------------------------
