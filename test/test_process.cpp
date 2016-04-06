#include "process.h"
#include "test_common.h"
#include "Rastrigin.h"
//#include "testFunction.h"

#include <gtest.h>
#include "mpi.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define _N 4
#define _NUM_OF_FUNC 1
#define _NUM_OF_TASK_LEVELS 1
#define _R 2.3
#define _RESERV 0.001
#define _M 10
// ------------------------------------------------------------------------------------------------
class TProcessTest : public ::testing::Test 
{
protected:
  double A[MaxDim];
  double B[MaxDim];

  TProcess* process;
  TProcessTest():  process(0)
  {}

  void CreateProcess(int N = _N, int NumOfFunc = _NUM_OF_FUNC,
                     int NumOfTaskLevels = _NUM_OF_TASK_LEVELS, 
                     double r = _R, int m = _M,
                     int * dimInTaskLevel = NULL,
                     int * childInProcLevel = NULL,
                     int * maxNumOfPoints = NULL,
                     double * eps = NULL)
  {
    const tFunction f[MaxNumOfFunc] = {objfn};
    bounds(A,B);
    process = new TProcess(N, A, B, NumOfFunc, f, NumOfTaskLevels,
                           dimInTaskLevel, childInProcLevel, 
                           maxNumOfPoints, eps, r, m, false, true); 
  }

  ~TProcessTest()
  {
    delete process;
  }
};

TEST_F(TProcessTest, no_throws_when_create_with_correct_values)
{
  int MaxNumOfPoints[] = {10000, 10000};
  double Eps[] = {0.01, 0.01};
  int NumOfTaskLevels = 2;
  int DimInTaskLevel[] = {2, 2};
  int ChildInProcLevel[] = {3, 0};
  int ProcRank, ProcNum;
  if (MPI_Comm_size(MPI_COMM_WORLD, &ProcNum) != MPI_SUCCESS)
  {
    throw EXCEPTION("Error in MPI_Comm_size call");
  }
  if (MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank) != MPI_SUCCESS)
  {
    throw EXCEPTION("Error in MPI_Comm_rank call");
  }

  if (ProcNum == 4)
  {
    ASSERT_NO_THROW(CreateProcess(_N, _NUM_OF_FUNC, NumOfTaskLevels, 
      _R, _M, DimInTaskLevel, ChildInProcLevel,
      MaxNumOfPoints, Eps));
  }
  else
  {
    ASSERT_TRUE(true);
  }
}

TEST_F(TProcessTest, throws_when_create_with_not_positive_NumOfTaskLevels)
{
  int MaxNumOfPoints[] = {10000, 10000};
  double Eps[] = {0.01, 0.01};
  int NumOfTaskLevels = 0;
  int DimInTaskLevel[] = {_N, 0};
  int ChildInProcLevel[] = {0, 0};

  ASSERT_ANY_THROW(CreateProcess(_N, _NUM_OF_FUNC, NumOfTaskLevels, 
    _R, _M, DimInTaskLevel, ChildInProcLevel,
    MaxNumOfPoints, Eps));
}

TEST_F(TProcessTest, throws_when_create_with_not_positive_Dim)
{
  int MaxNumOfPoints[] = {10000, 10000};
  double Eps[] = {0.01, 0.01};
  int NumOfTaskLevels = _NUM_OF_TASK_LEVELS;
  int DimInTaskLevel[] = {_N, 0};
  int ChildInProcLevel[] = {0, 0};

  ASSERT_ANY_THROW(CreateProcess(0, _NUM_OF_FUNC, NumOfTaskLevels, 
    _R, _M, DimInTaskLevel, ChildInProcLevel,
    MaxNumOfPoints, Eps));
}

TEST_F(TProcessTest, throws_when_create_with_r_out_of_range)
{
  int MaxNumOfPoints[] = {10000, 10000};
  double Eps[] = {0.01, 0.01};
  int NumOfTaskLevels = _NUM_OF_TASK_LEVELS;
  int DimInTaskLevel[] = {_N, 0};
  int ChildInProcLevel[] = {0, 0};

  ASSERT_ANY_THROW(CreateProcess(_N, _NUM_OF_FUNC, NumOfTaskLevels, 
    1, _M, DimInTaskLevel, ChildInProcLevel,
    MaxNumOfPoints, Eps));
}

TEST_F(TProcessTest, throws_when_create_with_m_out_of_range)
{
  int MaxNumOfPoints[] = {10000, 10000};
  double Eps[] = {0.01, 0.01};
  int NumOfTaskLevels = _NUM_OF_TASK_LEVELS;
  int DimInTaskLevel[] = {_N, 0};
  int ChildInProcLevel[] = {0, 0};

  ASSERT_ANY_THROW(CreateProcess(_N, _NUM_OF_FUNC, NumOfTaskLevels, 
    _R, 1, DimInTaskLevel, ChildInProcLevel,
    MaxNumOfPoints, Eps));
}

TEST_F(TProcessTest, throws_when_create_with_eps_out_of_range)
{
  int MaxNumOfPoints[] = {10000, 10000};
  double Eps[] = {0.011, 0.01};
  int NumOfTaskLevels = _NUM_OF_TASK_LEVELS;
  int DimInTaskLevel[] = {_N, 0};
  int ChildInProcLevel[] = {0, 0};

  ASSERT_ANY_THROW(CreateProcess(_N, _NUM_OF_FUNC, NumOfTaskLevels, 
    _R, _M, DimInTaskLevel, ChildInProcLevel,
    MaxNumOfPoints, Eps));
}

TEST_F(TProcessTest, throws_when_create_with_MaxNumOfPoints_out_of_range)
{
  int MaxNumOfPoints[] = {-1, 10000};
  double Eps[] = {0.01, 0.01};
  int NumOfTaskLevels = _NUM_OF_TASK_LEVELS;
  int DimInTaskLevel[] = {_N, 0};
  int ChildInProcLevel[] = {0, 0};

  ASSERT_ANY_THROW(CreateProcess(_N, _NUM_OF_FUNC, NumOfTaskLevels, 
    _R, _M, DimInTaskLevel, ChildInProcLevel,
    MaxNumOfPoints, Eps));
}

TEST_F(TProcessTest, throws_when_create_with_DimInTaskLevel_out_of_range)
{
  int MaxNumOfPoints[] = {10000, 10000};
  double Eps[] = {0.01, 0.01};
  int NumOfTaskLevels = _NUM_OF_TASK_LEVELS;
  int DimInTaskLevel[] = {-1, 0};
  int ChildInProcLevel[] = {0, 0};

  ASSERT_ANY_THROW(CreateProcess(_N, _NUM_OF_FUNC, NumOfTaskLevels, 
    _R, _M, DimInTaskLevel, ChildInProcLevel,
    MaxNumOfPoints, Eps));
}

TEST_F(TProcessTest, throws_when_create_with_illegal_value_DimInTaskLevel)
{
  int MaxNumOfPoints[] = {10000, 10000};
  double Eps[] = {0.01, 0.01};
  int NumOfTaskLevels = 2;
  int DimInTaskLevel[] = {2, 1};
  int ChildInProcLevel[] = {3, 0};

  ASSERT_ANY_THROW(CreateProcess(_N, _NUM_OF_FUNC, NumOfTaskLevels, 
    _R, _M, DimInTaskLevel, ChildInProcLevel,
    MaxNumOfPoints, Eps));
}

TEST_F(TProcessTest, throws_when_create_with_ChildInProcLevel_out_of_range)
{
  int MaxNumOfPoints[] = {10000, 10000};
  double Eps[] = {0.01, 0.01};
  int NumOfTaskLevels = 1;
  int DimInTaskLevel[] = {4, 0};
  int ChildInProcLevel[] = {-1, 0};

  ASSERT_ANY_THROW(CreateProcess(_N, _NUM_OF_FUNC, NumOfTaskLevels, 
    _R, _M, DimInTaskLevel, ChildInProcLevel,
    MaxNumOfPoints, Eps));
}
TEST_F(TProcessTest, throws_when_create_with_illegal_value_ChildInProcLevel)
{
  int MaxNumOfPoints[] = {10000, 10000};
  double Eps[] = {0.01, 0.01};
  int NumOfTaskLevels = 2;
  int DimInTaskLevel[] = {2, 2};
  int ChildInProcLevel[] = {2, 0};
  int ProcRank, ProcNum;
    if (MPI_Comm_size(MPI_COMM_WORLD, &ProcNum) != MPI_SUCCESS)
  {
    throw EXCEPTION("Error in MPI_Comm_size call");
  }
  if (MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank) != MPI_SUCCESS)
  {
    throw EXCEPTION("Error in MPI_Comm_rank call");
  }

  if (ProcNum == 4)
  {
    ASSERT_ANY_THROW(CreateProcess(_N, _NUM_OF_FUNC, NumOfTaskLevels, 
      _R, _M, DimInTaskLevel, ChildInProcLevel,
      MaxNumOfPoints, Eps));
  }
  else
  {
    ASSERT_TRUE(true);
  }
}
TEST_F(TProcessTest, check_states_of_method_one_process)
{
  char currentFile[] = "current_state.dat";
  int MaxNumOfPoints[] = {10000, 10000};
  double Eps[] = {0.01, 0.01};
  int ProcRank, ProcNum;
  int NumOfTaskLevels = _NUM_OF_TASK_LEVELS;
  int DimInTaskLevel[] = {_N, 0};
  int ChildInProcLevel[] = {0, 0};

  if (MPI_Comm_size(MPI_COMM_WORLD, &ProcNum) != MPI_SUCCESS)
  {
    throw EXCEPTION("Error in MPI_Comm_size call");
  }
  if (MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank) != MPI_SUCCESS)
  {
    throw EXCEPTION("Error in MPI_Comm_rank call");
  }

  if (ProcNum == 1)
  {
    CreateProcess(_N, _NUM_OF_FUNC, NumOfTaskLevels, 
                  _R, _M, DimInTaskLevel, ChildInProcLevel,
                  MaxNumOfPoints, Eps);

    process->Solve();

    CheckMetodIteration("state.dat", currentFile,
      process->GetIterationCountRoot() / 10);
  }
  else 
  {
    ASSERT_TRUE(true);
  }
}

TEST_F(TProcessTest, check_states_of_method_MPI_tree)
{
  char currentFile[] = "current_state.dat";
  int MaxNumOfPoints[] = {10000, 10000};
  double Eps[] = {0.01, 0.01};
  int ProcRank, ProcNum;
  int NumOfTaskLevels = 2;
  int DimInTaskLevel[] = {2, 2};
  int ChildInProcLevel[] = {3, 0};

  if (MPI_Comm_size(MPI_COMM_WORLD, &ProcNum) != MPI_SUCCESS)
  {
    throw EXCEPTION("Error in MPI_Comm_size call");
  }
  if (MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank) != MPI_SUCCESS)
  {
    throw EXCEPTION("Error in MPI_Comm_rank call");
  }

  if (ProcNum == 4)
  {
    CreateProcess(_N, _NUM_OF_FUNC, NumOfTaskLevels, 
                  _R, _M, DimInTaskLevel, ChildInProcLevel,
                  MaxNumOfPoints, Eps);

    process->Solve();

    if (ProcRank == 0)
    {
      CheckMetodIteration("state_tree.dat", currentFile,
        process->GetIterationCountRoot() / 10);
    }
  }
  else 
  {
    ASSERT_TRUE(true);
  }
}