#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int numIterations, numBodies, deltaTime;
struct body* nBodies;

#include "barnesHut.h"

#ifdef SERIAL
#include "serialBH.h"

void compute_accln(struct body* my, const struct body* other)
{
  double x_other = other->x, y_other = other->y, dist, mag;

  x_other -= my->x;
  y_other -= my->y;
  
  dist = pow(my->x, 2) + pow(my->y, 2);
  dist += soften;
  dist = pow(sqrt(dist), 3);
  mag = other->wt/dist;
  
  my->ax += mag*x_other;
  my->ay += mag*y_other;
}

void move_body(struct body* my)
{
  my->vx += my->ax * deltaTime;
  my->vy += my->ay * deltaTime;
  
  my->vx += damping;
  my->vy += damping;
  
  my->x += my->vx * deltaTime;
  my->y += my->vy * deltaTime;
}

void serialBarnesHut(const char* inputFileName, const char* outputFileName)
{
  int idx, idx2, iter;
  readFileSerial(inputFileName);
  
  for (iter = 0; iter < numIterations; iter++)
  {
    for (idx = 0; idx < numBodies; idx++)
    {
      for (idx2 = 0; idx2 < numBodies; idx2++)
      {
	if (idx2 != idx)
	  compute_accln(&nBodies[idx], &nBodies[idx2]);
      }    
    }
  
    for (idx = 0; idx < numBodies; idx++)
    {
      move_body(&nBodies[idx]);
    }
  }
  
  writeFileSerial(outputFileName);
}





#endif // SERIAL
