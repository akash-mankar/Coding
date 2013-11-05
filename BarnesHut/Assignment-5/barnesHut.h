#pragma once
#include <math.h>

#define TAG_NODE   1
#define TAG_BODY   2
#define TAG_TUPLE  3
#define TAG_START  10
#define TAG_STOP   11
#define TAG_END    100

#define soften 18.0
#define damping 0.1
#define theta 0.5

#define numChildren 4

struct body
{
  double x;       // x-coordinate
  double y;       // y-coordinate
  double wt;      // weight
  double vx;      // velocity along the x-axis
  double vy;      // velocity along the y-axis
  double ax;      // acceleration along the x-axis
  double ay;      // acceleration along the y-axis
};

#ifndef SERIAL

struct bodyProcTuple
{
  struct body aBody;
  int sourceIndex;
  int sourceProcRank;
  int destProcRank;
};

struct node
{
  double x_cm;       // x-coordinate
  double y_cm;       // y-coordinate
  double wt_tot;     // weight
  int numBodies;     // number of bodies
  double x_min;      // x-coordinate minimum value
  double x_max;      // x-coordinate maximum value
  double y_min;      // y-coordinate minimum value
  double y_max;      // y-coordinate maximum value
};

struct nodeData
{
  int rank;
  int parentRank;
  struct node* childValues;
  struct node* myValue;
  struct body* bodies;
};

// indices of the children of the quadtree
// expressed as directional coordinates
#define NW 0
#define NE 1
#define SW 2
#define SE 3

// The quadtree structure that represents the 
// local quadtree of a process
struct quadTree
{
  double x_cm;
  double y_cm;
  double wt_tot;
  double x_min;
  double x_max;
  double y_min;
  double y_max;
  int myLevel;
  struct body** bodies;
  int numBodies;
  struct quadTree* childTree[numChildren];
};

#endif
