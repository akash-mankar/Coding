#include <stdio.h>
#include <stdlib.h>
#include <float.h>

int numIterations, numTotalBodies;
float deltaTime;

struct body* nBodies;

#include "barnesHut.h"

#ifdef SERIAL
#include "serialBH.h"
#else 
#include "mpi.h"
#include "parallelBH.h"
int cSize, cSizeOriginal, myRank;
struct nodeData myData;
double g_x_max = DBL_MIN, g_x_min = DBL_MAX, g_y_max = DBL_MIN, g_y_min = DBL_MAX;
#endif

void readFile(const char* inputFileName)
{
  FILE *fp = fopen(inputFileName, "r");
  int idx;

  if (fp == NULL)
  {
    printf("Could not open inputFile: %s\n", inputFileName);
    return;
  }

  fscanf(fp, "%d", &numTotalBodies);
  nBodies = (struct body *)calloc(numTotalBodies, sizeof(struct body));
  
  for(idx = 0; idx < numTotalBodies; idx++)
  {
    fscanf(fp, "%lg", &nBodies[idx].x);
    fscanf(fp, "%lg", &nBodies[idx].y);
    fscanf(fp, "%lg", &nBodies[idx].wt);
    fscanf(fp, "%lg", &nBodies[idx].vx);
    fscanf(fp, "%lg", &nBodies[idx].vy);
  }

  fclose(fp);
}

void writeFile(const char* outputFileName)
{
  FILE* fp = fopen(outputFileName, "w");
  int idx;

  if (fp == NULL)
  {
    printf("Could not open outputFile: %s\n", outputFileName);
    return;
  }

  fprintf(fp, "%d\n", numTotalBodies);
  
  for (idx = 0; idx < numTotalBodies; idx++)
  {
    fprintf(fp, "%lg %lg %lg %lg %lg\n", nBodies[idx].x, nBodies[idx].y, nBodies[idx].wt, nBodies[idx].vx, nBodies[idx].vy); 
  }
  
  fclose(fp);
}

int main(int _argc, char** _argv)
{
  int rc;

#ifndef SERIAL
  rc = MPI_Init(&_argc, &_argv);
  if (rc != MPI_SUCCESS)
  {
    printf("Error initializing MPI: rc = %d.\n", rc);
    return -1;
  }
  
  MPI_Comm_size(MPI_COMM_WORLD, &cSize);
  MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
  cSizeOriginal = cSize;

#ifdef DBG 
  if (myRank == 0)
  {
    printf("MPI Comm Size: %d\n", cSize);
  }
  
  printf("Rank: %d PID: %d\n", myRank, getpid());
#endif // DBG
#endif // serial 
  
  if (_argc != 5)
  {
    printf("Incorrect number of arguments!\n");

#ifndef SERIAL
    if (myRank == 0)
    {
      MPI_Abort(MPI_COMM_WORLD, -1);
    }
#endif // serial
    return -1;
  }

  numIterations = atoi(_argv[1]);
  deltaTime = atof(_argv[2]);
  
#ifdef SERIAL
  serialBarnesHut(_argv[3], _argv[4]);
#else
  parallelBarnesHut(_argv[3], _argv[4]);
  MPI_Finalize();
#endif // SERIAL
}


