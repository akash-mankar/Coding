#include <stdio.h>
#include <stdlib.h>

#include "barnesHut.h"

#ifndef SERIAL

extern struct nodeData myData;
extern struct quadTree root;
extern int myRank;
extern float deltaTime;

extern int nLevels;

void constructEmptyQuadTree(struct quadTree* tree, int level)
{
  tree->myLevel = level;
  if (level == nLevels)
  {
    tree->x_min = myData.myValue->x_min;
    tree->x_max = myData.myValue->x_max;
    tree->y_min = myData.myValue->y_min;
    tree->y_max = myData.myValue->y_max;
  }

  tree->x_cm = 0;
  tree->y_cm = 0;
  tree->wt_tot = 0;
  tree->numBodies = 0;
  tree->bodies = NULL;
  
  if (level > 0)
  {
    tree->childTree[NW] = (struct quadTree *)calloc(1, sizeof(struct quadTree));
    tree->childTree[NW]->x_min = tree->x_min;
    tree->childTree[NW]->x_max = (tree->x_min + tree->x_max)/2;
    tree->childTree[NW]->y_min = tree->y_min;
    tree->childTree[NW]->y_max = (tree->y_min + tree->y_max)/2;
    
    tree->childTree[NE] = (struct quadTree *)calloc(1, sizeof(struct quadTree));
    tree->childTree[NE]->x_min = (tree->x_min + tree->x_max)/2;
    tree->childTree[NE]->x_max = tree->x_max;
    tree->childTree[NE]->y_min = tree->y_min;
    tree->childTree[NE]->y_max = (tree->y_min + tree->y_max)/2;

    tree->childTree[SW] = (struct quadTree *)calloc(1, sizeof(struct quadTree));
    tree->childTree[SW]->x_min = tree->x_min;
    tree->childTree[SW]->x_max = (tree->x_min + tree->x_max)/2;
    tree->childTree[SW]->y_min = (tree->y_min + tree->y_max)/2;
    tree->childTree[SW]->y_max = tree->y_max;

    tree->childTree[SE] = (struct quadTree *)calloc(1, sizeof(struct quadTree));
    tree->childTree[SE]->x_min = (tree->x_min + tree->x_max)/2;
    tree->childTree[SE]->x_max = tree->x_max;
    tree->childTree[SE]->y_min = (tree->y_min + tree->y_max)/2;
    tree->childTree[SE]->y_max = tree->y_max;
    
    constructEmptyQuadTree(tree->childTree[NW], level - 1);
    constructEmptyQuadTree(tree->childTree[NE], level - 1);
    constructEmptyQuadTree(tree->childTree[SW], level - 1);
    constructEmptyQuadTree(tree->childTree[SE], level - 1);
  }
  else
  {
    tree->bodies = (struct body **)calloc(10, sizeof(struct body*));
  }
  return;
}

void addBodyToQuadTree(const struct body* aBody, struct quadTree* tree)
{
  int idx;

  tree->x_cm = (tree->x_cm * tree->wt_tot + aBody->x * aBody->wt)/(tree->wt_tot + aBody->wt);
  tree->y_cm = (tree->y_cm * tree->wt_tot + aBody->y * aBody->wt)/(tree->wt_tot + aBody->wt);
  tree->wt_tot += aBody->wt;
  
  if (tree->myLevel == 0)
  {
    // Add the ptr to the body to the tree->bodies 
    // array, and increment the numBodies
    // Also, if array is unallocated, then allocate or realloc it
    if (tree->numBodies%10 == 0)
    {
      // reallocate if allocated space is already taken up
      tree->bodies = (struct body **)realloc(tree->bodies, sizeof(struct body *)*(tree->numBodies + 10));
    }
    tree->bodies[tree->numBodies] = (struct body *)aBody;
    tree->numBodies++;
  }
  else
  {
    tree->numBodies++;
    // Check which subtree it belongs to and call the same
    // function recursively to add to it. Break when done.
    for (idx = NW; idx <= SE; idx++)
    {
      // Compare body's position with the limits of 
      // the region fo the child node
      if ((tree->childTree[idx]->x_min <= aBody->x && tree->childTree[idx]->x_max >= aBody->x) &&
	  (tree->childTree[idx]->y_min <= aBody->y && tree->childTree[idx]->y_max >= aBody->y))
      {
	addBodyToQuadTree(aBody, tree->childTree[idx]);
	break;
      }
    }
  }
}

void constructLocalQuadTree()
{
  int idx;

  constructEmptyQuadTree(&root, nLevels);

  for (idx = 0; idx < myData.myValue->numBodies; idx++)
  {
    addBodyToQuadTree(&myData.bodies[idx], &root);
  }
}

void compute_accln(struct body* my, double x_other, double y_other, double wt_other)
{
  double dist, mag;

  x_other -= my->x;
  y_other -= my->y;
  
  dist = pow(my->x, 2) + pow(my->y, 2);
  
  if (dist > 0.0)
  {
    dist += soften;
    dist = pow(sqrt(dist), 3);
    mag = wt_other/dist;
  
    my->ax += mag*x_other;
    my->ay += mag*y_other;
  }
}

void move_body(struct body* my)
{
  my->vx += my->ax * deltaTime;
  my->vy += my->ay * deltaTime;
  
  my->vx += damping;
  my->vy += damping;
  
  my->x += my->vx * deltaTime;
  my->y += my->vy * deltaTime;
  
  my->ax = 0;
  my->ay = 0;
}


#endif 
