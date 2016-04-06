#ifndef __RASTRIGIN_H__
#define __RASTRIGIN_H__
#include <math.h>

/*Provide dimension of the problem.  Integer >= 1.*/
const int N = 4;
/*Provide the diameter of the feasible region.*/
/*Provide maximum number of iterations.*/
#define MAXITER 100*N*N/*500*N */

#define PI 3.14159265359

void bounds(double lb[], double ub[]);
int feasible(double x[]);
double objfn(double x[]);
void bounds(double lb[], double ub[])
/*Provide lower and upper bounds for each of N variables.
 Number of bounds is equal to N.*/
{
  lb[0] = -2.2;
  ub[0] = 1.8;
  lb[1] = -2.2;
  ub[1] = 1.8;
  lb[2] = -2.2;
  ub[2] = 1.8;
  lb[3] = -2.2;
  ub[3] = 1.8;
  lb[4] = -2.2;
  ub[4] = 1.8;
}

int feasible(double x[])
/*Test feasibility of x.  Return 1 if feasible, 0 if not.*/
{
  return 1;
}

double objfn(const double x[])
/*Calculate objective function value of x[].*/
{

  int j;
  double sum=0.;

  for (j=0; j<N; j++)
  {
    sum+=x[j]*x[j]-10.*cos(2.*PI*x[j])+10.;
  }

  return (sum);

}

#endif