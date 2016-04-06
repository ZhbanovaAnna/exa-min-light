/*
* Copyright (C) 2015 University of Nizhni Novgorod
* This file is part of the Globalizer Light library, v 1.0
* 
* method.cpp - source file for method class
* 
* This file may be used under the terms of the GNU Lesser General Public 
* License version 2.1 as published by the Free Software Foundation 
* and appearing in the file LICENSE.LGPL included in the packaging of this 
* file. Please review the following information to ensure the GNU Lesser 
* General Public License version 2.1 requirements will be met: 
* http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
*/

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
                 int _CurL, int _NumPoints,/*EMapType _MapType, /*TParameters _parameters, */TTask *_pTask, TSearchData *_pData)//:
//parameters(_parameters)
{
  if (_MaxNumOfTrials < 1)
  {
    throw EXCEPTION("MaxNumOfTrials is out of range");
  }
  MaxNumOfTrials = _MaxNumOfTrials;

  if ((_Eps <= 0.0) || (_Eps > 0.01))
  {
    throw EXCEPTION("Epsilon is out of range");
  }
  Epsilon = _Eps;

  if (_r <= 2.0)
  {
    throw EXCEPTION("r is out of range");
  }
  r = _r; 

  m = _m;

  if (_L < 1)
  {
    throw EXCEPTION("L is out of range");
  }
  L = _L;

  CurL = _CurL;

  if ((_reserv < 0.0) || (_reserv > 0.01))
  {
    throw EXCEPTION("Epsilon is out of range");
  }
  reserv = _reserv; // eps-резервирование
  alfa = 15; // пока локальная адаптация - фиксированная
  NumPoints = 1;
  NumPoints = _NumPoints;
  //MapType = _MapType;
  pTask = _pTask;
  pData = _pData;
  IterationCount = 0;
  OnIteration = 0;
  AchievedAccuracy = MaxDouble;
  // Инициализиуем числа с расширенной точностью
  InitAutoPrecision();
  // Создаем развертку с нужной размерностью
  Evolvent = new TEvolvent(pTask->GetFreeN(), m);
  // Массив для текущих итераций
  pCurTrials = new TTrial[NumPoints];
  // Лучшая точка пока не найдена
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
	////Раскомментировать для множественных разверток
 //   //    if ((m + 1) * pTask->GetFreeN() < 50)
 //   {
 //     Extended::SetTypeID(etDouble);
 //   }
	////Раскомментировать для множественных разверток
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

  //Отладка
  //char filename[20];
  //sprintf(filename,"process_%d.dat",  pn);
  //ofstream ofs(filename);
  //ofs.close();

  // Это первая итерация, сбрасываем счетчик
  IterationCount = 1;
  // И сбрасываем достигнутую точность
  AchievedAccuracy = 1.0;
  // И лучшую итерацию
  BestTrial.index = -2;
  // Формируем интервал [0,1]
  TSearchInterval NewInterval;
  NewInterval.ind = IterationCount;
  NewInterval.K = 0;
  // Левая точка
  NewInterval.zl = MaxDouble;
  NewInterval.izl = -2;
  NewInterval.xl = 0.0;
  // Правая точка
  NewInterval.xr = 1.0;
//  NewInterval.dx = 1.0;
  NewInterval.zr = MaxDouble;
  NewInterval.izr = -2;
  // Гельдеровская длина
  NewInterval.delta = 1.0;
  //Добавляем интервал
  TSearchInterval* p = pData->InsertInterval(NewInterval);
  // На первой итерации - единственный лучший интервал
  for (i = 0; i < NumPoints; i++)
    BestIntervals[i] = p;
  // Флаг пересчета - поднят
  recalc = true;

  // Точки первой итерации выбираются по особому правилу
  // Равномерно ставим NumPoints точек c шагом h
  double h = 1.0 / (NumPoints + 1);
  for(i = 0; i < NumPoints; i++)
  {
    pCurTrials[i].x = (i + 1) * h;
    // Вычисляем образ точки итерации - образ записывается в начальные позиции массива y
    Evolvent->GetImage(pCurTrials[i].x,pCurTrials[i].y);
    // Смещаем вычисленные координаты в соответствии с уровнем подзадачи
    for(j = 0; j < pTask->GetFreeN(); j++)
      pCurTrials[i].y[pTask->GetFixedN() + j] = pCurTrials[i].y[j];

    // Записываем фиксированные координаты - они всегда расположены вначае
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

  // Если поднят флаг - то пересчитать все характеристики
  if (recalc)
  {
    // Обновить текущие значение минимумов
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
    // После пересчета флаг опускаем
    recalc = false;
  }

  // Здесь надо взять NumPoints лучших характеристик из очереди
  // Очередь пока одна - очередь глобальных характеристик
  // В ней должно быть нужное количество интервалов, т.к. на первом шаге проводится NumPoints испытаний
  //if(pn == 0)
      //printf("NumPoints GetBestIntervals %d\n", NumPoints);
  pData->GetBestIntervals(BestIntervals, NumPoints);
    //if(pn == 0)
      //printf("end GetBestIntervals %d\n", NumPoints);

  // Пока заполняем одновременно вектор CurTrials, и вектор интервалов
  for (int i = 0; i < NumPoints; i++)
  {
      //if(pn == 0)

    // Вычисляем x
    if (BestIntervals[i]->izl != BestIntervals[i]->izr)
    {
          //printf("BestIntervals %lf  %lf\n", BestIntervals[i]->izr, BestIntervals[i]->izl);
            //printf("!!!!1 %lf  %lf\n", BestIntervals[i]->xl, BestIntervals[i]->xr);
      pCurTrials[i].x = 0.5 * (BestIntervals[i]->xl + BestIntervals[i]->xr);
      //      pCurTrials[i].x = BestIntervals[i]->xl + 0.5*BestIntervals[i]->dx; 
      //printf("!!!!1 %lf  %lf\n", BestIntervals[i]->xl, BestIntervals[i]->xr);
    }
    else
    {
        //if (IterationCount == 2)
            //printf("!!!!2 %lf  %lf\n", BestIntervals[i]->zr, BestIntervals[i]->zl);
      pCurTrials[i].x = 0.5 * (BestIntervals[i]->xl + BestIntervals[i]->xr) - 
        (((BestIntervals[i]->zr - BestIntervals[i]->zl) > 0) ? 1: -1) * 
        pow(fabs(BestIntervals[i]->zr - BestIntervals[i]->zl) / 
        pData->M[BestIntervals[i]->izl], pTask->GetFreeN()) / 2 / r;
           //printf("!!!!2 %lf  %lf\n", BestIntervals[i]->zr, BestIntervals[i]->zl);
      //      pCurTrials[i].x = BestIntervals[i]->xl + (0.5*BestIntervals[i]->dx - (((BestIntervals[i]->zr - BestIntervals[i]->zl)>0)?1:-1)*pow(fabs(BestIntervals[i]->zr - BestIntervals[i]->zl)/pData->M[BestIntervals[i]->izl],pTask->GetFreeN())/(2*r));
    }
  //if(pn == 0)
      //printf("!!!!s %lf\n", pCurTrials[i].x);

    	//Точка новой итерации должна быть в интервале, иначе - ошибка!!!
	if(pCurTrials[i].x <= BestIntervals[i]->xl || pCurTrials[i].x >= BestIntervals[i]->xr )
	{
		throw EXCEPTION("Point is outside the interval !");
	}
    // Вычисляем y
    // Вычисляем образ точки итерации - образ записывается в начальные позиции массива y
    Evolvent->GetImage(pCurTrials[i].x, pCurTrials[i].y);
      //printf("!!!!get image %d\n", NumPoints);
    // Смещаем вычисленные координаты в соответствии с уровнем подзадачи
    for (int j = 0; j < pTask->GetFreeN(); j++)
    {
      pCurTrials[i].y[pTask->GetFixedN() + j] = pCurTrials[i].y[j];
    }
      //if(pn == 0)
      //printf("!!!!s %d\n", NumPoints);
    // Записываем фиксированные координаты - они всегда расположены вначае
    for (int j = 0; j < pTask->GetFixedN(); j++)
    {
      pCurTrials[i].y[j] = pTask->GetFixedY()[j];
    }
  }
    //if(pn == 0)
      //printf("END CALC POINTS\n");
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
  //Если метор работает в корне дерева процессов
  if(pTask->GetFixedN() == 0)
  {
	  //то применяем критерий остановки по попаданию в окрестность минимума
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
	  //критерий остановки по точности
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
  //Это функция вызывается только в листе дерева - поэтому вычисляем функционалы здесь
  for (i = 0; i < NumPoints; i++)
  {
    //Вычисляем образ точки итерации - образ записывается в начальные позиции массива y
    Evolvent->GetImage(pCurTrials[i].x, pCurTrials[i].y);

    // Смещаем вычисленные координаты в соответствии с уровнем подзадачи
    for (j = 0; j < pTask->GetFreeN(); j++)
      pCurTrials[i].y[pTask->GetFixedN() + j] = pCurTrials[i].y[j];

    // Записываем фиксированные координаты - они всегда расположены вначале
    for (j = 0; j < pTask->GetFixedN(); j++)
      pCurTrials[i].y[j] = pTask->GetFixedY()[j];

    //Записываем значения MaxDouble
    for (j = 0; j < pTask->GetNumOfFunc(); j++)
      pCurTrials[i].FuncValues[j] = MaxDouble;

    //Теперь можно вычислять функцию
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
    // Так как вычисление - в листе дерева, то вложенных итераций нет
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
  // Дополнительные точки с отрицательным индексом не обрабатываем
  if (p->izl < 0 || p->izr < 0)
    return;

  // Реализация без просмотра вправо-влево, только текущий интервал!!!!
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
  //Отладка - надо потом раскомментировать вычисление "влево"
  i=p;
  while(i->iz>=0)
  {
  i--;
  if(i->iz == tek_index) break;
  }
  //Если индекс - отрицательный, то ничего не делаем
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
  i = pData->GetNext();//НЕВЕРНО - нужен переход вправо от текущей точки!!!!!
  //++++++++++++++++++++++++++++++++
  if(i->izl == tek_index)break;
  }
  //Если индекс - отрицательный (точка между отрезками), то ничего не делаем
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
  // После проведения испытаний - вставить результаты 
  for (int j = 0; j < NumPoints; j++)
  {
    // Заполнение интервала
    TSearchInterval NewInterval;
    NewInterval.ind = IterationCount;
    // Запоминаем число вложенных итераций
    NewInterval.K = pCurTrials[j].K;
    // Левая точка интервала - это точка очередного испытания
    NewInterval.xl = pCurTrials[j].x;
    NewInterval.izl = pCurTrials[j].index;
    NewInterval.zl = pCurTrials[j].FuncValues[pCurTrials[j].index];
    // Правая точка интервала - это правая точка интервала, в котором проведено испытание
    NewInterval.xr = BestIntervals[j]->xr;
    // Длина интервала
    //		NewInterval.dx = (BestIntervals[j]->dx - (NewInterval.xl - BestIntervals[j]->xl)).toDouble();
    NewInterval.izr = BestIntervals[j]->izr;
    NewInterval.zr = BestIntervals[j]->zr;
    // Гельдеровская длина интервала
    //NewInterval.delta = root(NewInterval.xr - NewInterval.xl, pTask->GetFreeN());
        NewInterval.delta = pow(NewInterval.xr - NewInterval.xl, 1.0 / pTask->GetFreeN());

    // Корректируем существующий интервал
    BestIntervals[j]->xr = NewInterval.xl;
    BestIntervals[j]->izr = NewInterval.izl;
    BestIntervals[j]->zr = NewInterval.zl;
    //    BestIntervals[j]->dx -= NewInterval.dx;

    // Обновляем достигнутую точность - по старой гельдеровской длине лучшего интервала
    if (BestIntervals[j]->delta < AchievedAccuracy)
    {
      AchievedAccuracy = BestIntervals[j]->delta;
    }
    // После чего вычисляем новую гельдеровскую длину лучшего интервала
    //BestIntervals[j]->delta = root(BestIntervals[j]->xr - BestIntervals[j]->xl, pTask->GetFreeN());
    BestIntervals[j]->delta = pow(BestIntervals[j]->xr - BestIntervals[j]->xl,1.0 / pTask->GetFreeN());

    // Интервал сформирован - можно добавлять
    // Вставка завершается корректно, или же выбрасывает исключение
    TSearchInterval* p = pData->InsertInterval(NewInterval);
    //Обработка началной итерации
    if (IterationCount == 1)
    {
      // Добавить следующий интервал для обработки
      if (j < NumPoints - 1)
      {
        BestIntervals[j + 1] = p;
      }
      recalc = true;
    }
    // Вычисляем оценку константы
    CalculateM(p);
    CalculateM(BestIntervals[j]);

    // Если полный пересчет не нужен - обновляем только очереди характеристик
    if (!recalc)
    {
      // Удалять интервалы из очереди не надо - они уже удалены в GetBestIntervals
      // Вставляем два новых интервала
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
  // Сравниваем значение в текущем оптимуме с текущими точками
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
      // Оптимум обновился - нужен пересчет
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
  // Задаем границы интервалов изменения параметров
  // Указатель на границы интервалов в подзадаче - это указатель на исходные границы,
  //   смещенный на число фиксированных размерностей
  Evolvent->SetBounds(pTask->GetA() + pTask->GetFixedN(),pTask->GetB() + pTask->GetFixedN());
}

// ------------------------------------------------------------------------------------------------
int TMethod::GetNumberOfTrials()
{
  // Если задача последнего уровня, т.е. сумма размерностей равна общей размерности
  if (pTask->GetN() == pTask->GetFixedN() + pTask->GetFreeN())
  {
    // Число итераций равно числу интервалов в таблице - 1 
    return pData->GetCount() - 1;
  }
  // Если задача верхнего уровня - надо просуммировать все данные из интервалов
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
  return pData->M[0]; // пока работаем только с одной функцией
}
// ------------------------------------------------------------------------------------------------
const double* TMethod::GetM()
{
  return pData->M; // возвращается указатель на массив оценок констант Липшица
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

double TMethod::GetAchievedAccuracy()
{
  return AchievedAccuracy;
}

void TMethod::PrintCurrentStateToFile(char* nameOfFile)
{
  FILE* pf;
  int i, j;
  pf = fopen(nameOfFile,"a");
  fprintf(pf, "IterationCount = %d\n", IterationCount);
  fprintf(pf, "AchievedAccuracy = %lf\n", AchievedAccuracy);
  fprintf(pf, "recalc = %b\n", recalc);
  fprintf(pf, "BestTrial.FuncValue = %lf\n", BestTrial.FuncValues[0]);
  fprintf(pf, "BestTrial.x = %lf\n", BestTrial.x);
  for (i = 0; i < pTask->GetN(); i++)
    fprintf(pf, "BestTrial.y[%d] = %lf\n", i, BestTrial.y[i]);
  for (j = 0; j < NumPoints; j++)
  {
    fprintf(pf, "pCurTrials[%d].FuncValue = %lf\n", j, pCurTrials[j].FuncValues[0]);
    fprintf(pf, "pCurTrials[%d].x = %lf\n", j, pCurTrials[j].x);
    for (i = 0; i < pTask->GetN(); i++)
      fprintf(pf, "pCurTrials[%d].y[%d] = %lf\n", j, i, pCurTrials[j].y[i]);
  }
  fclose(pf);
}
 
// - end of file ----------------------------------------------------------------------------------
