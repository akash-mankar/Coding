#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <string.h>

#include "barnesHut.h"

extern int cSize, myRank;
extern int numIterations, numTotalBodies;
extern float deltaTime;

#ifndef SERIAL
#include "parallelBH.h"

extern double g_x_max, g_x_min, g_y_max, g_y_min;
extern struct body* nBodies;
extern struct nodeData myData;
struct quadTree root;

MPI_Datatype mpi_node, mpi_body, mpi_tuple;
MPI_Op mpi_node_sum;
int partitions, **rankMatrix, s_proc, sqrt_part;
double x_length, y_length;
struct node* allProcData;

struct bodyProcTuple* bodiesToSend = NULL;
int numBodiesToSend;
MPI_Comm mpi_comm_leaf, mpi_comm_nodes;

void mpi_add_nodes(struct node* in, struct node* inout, int* len, MPI_Datatype* type)
{
  int idx;

  for (idx = 0; idx < *len; idx++)
  {
    inout[idx].x_cm += in[idx].x_cm;
    inout[idx].y_cm += in[idx].y_cm;
    inout[idx].wt_tot += in[idx].wt_tot;
    inout[idx].numBodies += in[idx].numBodies;
    inout[idx].x_min += in[idx].x_min;
    inout[idx].x_max += in[idx].x_max;
    inout[idx].y_min += in[idx].y_min;
    inout[idx].y_max += in[idx].y_max;
  }
}

void initTypes()
{
  MPI_Aint offsets[1], tupleOffSets[2], extent;
  MPI_Datatype oldtypes[1], tupleOldTypes[2];
  int blockcounts[1], tupleBlockCounts[2];

  offsets[0] = 0;
  oldtypes[0] = MPI_DOUBLE;
  blockcounts[0] = 8;
  
  MPI_Type_struct(1, blockcounts, offsets, oldtypes, &mpi_node);
  MPI_Type_commit(&mpi_node);

  blockcounts[0] = 7;
  MPI_Type_struct(1, blockcounts, offsets, oldtypes, &mpi_body);
  MPI_Type_commit(&mpi_body);

  MPI_Type_extent(mpi_body, &extent);
  tupleOffSets[0] = 0;
  tupleOffSets[1] = extent;
  tupleOldTypes[0] = mpi_body;
  tupleOldTypes[1] = MPI_INT;
  tupleBlockCounts[0] = 1;
  tupleBlockCounts[1] = 3;
  MPI_Type_struct(2, tupleBlockCounts, tupleOffSets, tupleOldTypes, &mpi_tuple);
  MPI_Type_commit(&mpi_tuple);

  MPI_Op_create((MPI_User_function *)mpi_add_nodes, 1, &mpi_node_sum);
}

void getIndices(int rank, int* y, int* x)
{
  int exp = 0;
  *x = 0; *y = 0;

  while (rank != 0)
  {
    *x += rank%2 * (1 << exp);
    rank = rank/2;

    *y += rank%2 * (1 << exp);
    rank = rank/2;
    
    exp++;
  }
}

int interleaveBits(int y, int x)
{
  int num = 0, exp = 0;
  
  while (x != 0 || y != 0)
  {
    num += (y%2) * (1 << exp);
    y = y/2;
    exp++;
    
    num += (x%2) * (1 << exp);
    x = x/2;
    exp++;
  }

  return num;
}

void createSeparateCommunicator()
{
  MPI_Group worldGroup, newGroup, nodesGroup;
  int n, *ranks, idx, *ranks1;

  n = cSize - s_proc;
  ranks = (int *)calloc(n, sizeof(int));
  ranks1 = (int *)calloc(cSize, sizeof(int));

  for (idx = 0; idx < n; idx ++)
  {
    ranks[idx] = idx + s_proc;
  }
  
  for (idx = 0; idx < cSize; idx++)
  {
    ranks1[idx] = idx;
  }

  MPI_Comm_group(MPI_COMM_WORLD, &worldGroup);
  MPI_Group_incl(worldGroup, n, ranks, &newGroup);
  MPI_Group_incl(worldGroup, cSize, ranks1, &nodesGroup);
  MPI_Comm_create(MPI_COMM_WORLD, newGroup, &mpi_comm_leaf);
  MPI_Comm_create(MPI_COMM_WORLD, nodesGroup, &mpi_comm_nodes);
}

void computeHierarchy()
{
  int exp = 0, sum = 0, idx, idx2;

  // Since every processor needs a cluster, compute the total number of clusters
  // Number of processors used: 1 + 4 + 4^2 + ...

  while ((sum + pow(numChildren, exp)) <= cSize)
  {
    sum += pow(numChildren, exp);
    exp++;
  }

  // TODO: Decide whether this is correct - 
  // reducing num procs to 1 + 4 + 4^2 + ...

  cSize = sum;
  exp = exp - 1;
  partitions = pow(numChildren, exp);
  s_proc = cSize - partitions;
  sqrt_part = sqrt(partitions);

  createSeparateCommunicator();

  // printf("%d %d %d %d\n", cSize, exp, s_proc, partitions);

  if (myRank < s_proc)
  {
    // If these are the internal nodes, then these will only 
    // have children which could be internal nodes or leaf nodes
    myData.rank = myRank;
    myData.parentRank = (int)floor(((double)myRank - 1.0)/(double)numChildren);
    myData.childValues = (struct node *)calloc(numChildren, sizeof(struct node));
    myData.myValue = (struct node*)calloc(1, sizeof(struct node));
    myData.bodies = NULL;
  }
  else if (myRank >= s_proc && myRank < cSize)
  {
    // If these are the leaf nodes, then these will only have bodies
    // and not any children nodes
    myData.rank = myRank;
    myData.parentRank = (int)floor(((double)myRank - 1.0)/(double)numChildren);
    myData.childValues = NULL;
    myData.myValue = (struct node*)calloc(1, sizeof(struct node));
    myData.bodies = (struct body*)calloc(1000, sizeof(struct body));
  }

  // Create Z-order rank matrix for rank-0 process 
  // This will help in assigning data at each step
  if (myRank == 0)
  {
    rankMatrix = (int **)malloc(sqrt_part * sizeof(int *));
    for (idx = 0; idx < sqrt_part; idx++)
    {
      rankMatrix[idx] = (int *)malloc(sqrt_part * sizeof(int));
    }

    for (idx = 0; idx < sqrt_part; idx++)
    {
      for (idx2 = 0; idx2 < sqrt_part; idx2++)
      {
	rankMatrix[idx][idx2] = s_proc + interleaveBits(idx, idx2);
#ifdef DBG
	printf("%d ", rankMatrix[idx][idx2]);
#endif
      }
#ifdef DBG
      printf("\n");
#endif
    }
  }
}

void sendReceiveCalcLimits()
{
  int idx, x_index, y_index;
  MPI_Status status;
  struct body tempBody = {0};

  if (myRank == 0)
  {
    // Calculate the extents of the data
    // for calculation of the limits of each cell's boundary

    for (idx = 0; idx < numTotalBodies; idx ++)
    {
      if (nBodies[idx].x > g_x_max)
	g_x_max = nBodies[idx].x;
      if (nBodies[idx].x < g_x_min)
	g_x_min = nBodies[idx].x;
      if (nBodies[idx].y > g_y_max)
	g_y_max = nBodies[idx].y;
      if (nBodies[idx].y < g_y_min)
	g_y_min = nBodies[idx].y;
    }

    // Boundary relaxation - this is done to make 
    // the nodes include the boundary nodes
    g_x_min = g_x_min - 1.0;
    g_x_max = g_x_max + 1.0;
    g_y_min = g_y_min - 1.0;
    g_y_max = g_y_max + 1.0;

    myData.myValue->x_max = g_x_max;
    myData.myValue->x_min = g_x_min;
    myData.myValue->y_max = g_y_max;
    myData.myValue->y_min = g_y_min;
  }

  // Every processor needs the global limits, 
  // since they need to work out their own limits
  MPI_Bcast(&g_x_max, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(&g_x_min, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(&g_y_max, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(&g_y_min, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  x_length = (g_x_max - g_x_min)/sqrt(partitions);
  y_length = (g_y_max - g_y_min)/sqrt(partitions);

#ifdef DBG
  if (myRank == 0)
    printf("%lg %lg %lg %lg\n", g_x_min, g_x_max, g_y_min, g_y_max);
#endif

  MPI_Barrier(MPI_COMM_WORLD);
  
  if (myRank == 0)
  {  
    // Assign each body to the appropriate processor
    // based on its position in the grid
    for (idx = 0; idx < numTotalBodies; idx++)
    {
      x_index = (int)((nBodies[idx].x - g_x_min)/x_length);
      y_index = (int)((nBodies[idx].y - g_y_min)/y_length);

      MPI_Send(&nBodies[idx], 1, mpi_body, rankMatrix[y_index][x_index], TAG_BODY, MPI_COMM_WORLD);
    }

    // All body assignments are complete, send every processor 
    // the notification that they should stop receiving bodies.
    for (idx = s_proc; idx < cSize; idx++)
    {
      MPI_Send(&tempBody, 1, mpi_body, idx, TAG_END, MPI_COMM_WORLD);
    }
  }
  else if (myRank >= s_proc && myRank < cSize)
  {
    // Each processor must receive bodies till it receives the body with
    // TAG_END as the tag
    while (1)
    {
      MPI_Recv(&tempBody, 1, mpi_body, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      if (status.MPI_TAG == TAG_END)
	break;
      else
      {
	// Need to realloc memory - we had optimistically allocated space
	// for 1000 bodies, so we add space for an additional 1000 when that's
	// exceeded.
	if (myData.myValue->numBodies%1000 == 0)
        {
	  myData.bodies = (struct body*)realloc(myData.bodies, sizeof(struct body) * (myData.myValue->numBodies + 1000));
	}
	myData.bodies[myData.myValue->numBodies++] = tempBody;
      }
    }

    // compute the limits of each of the leaf processors using the 
    // z-order curve computation method
    getIndices(myRank - s_proc, &y_index, &x_index);
    myData.myValue->x_min = g_x_min + x_length * x_index;
    myData.myValue->x_max = g_x_min + x_length * (x_index + 1);
    myData.myValue->y_min = g_y_min + y_length * y_index;
    myData.myValue->y_max = g_y_min + y_length * (y_index + 1);
  
    printf ("Rank %d Started QuadTree construction: \n", myRank);
    constructLocalQuadTree();
  }
}

void calculateTMandCM()
{
  double cmx = 0, cmy = 0, tm = 0, num = 0;
  int idx, childIdx;
  struct node tempNode;
  MPI_Status status;

  if (myRank >= s_proc && myRank < cSize)
  {
    printf("Rank: %d QTree numBodies: %d TM: %lg CM_X: %lg CM_Y: %lg X_MIN: %lg X_MAX: %lg Y_MIN: %lg Y_MAX: %lg\n", myRank, root.numBodies, root.wt_tot, root.x_cm, root.y_cm, root.x_min, root.x_max, root.y_min, root.y_max);
    
    for (idx = 0; idx < myData.myValue->numBodies; idx++)
    {
      tm += myData.bodies[idx].wt;
      cmx += myData.bodies[idx].x;
      cmy += myData.bodies[idx].y;
    }
    
    myData.myValue->wt_tot = tm;
    
    if (myData.myValue->numBodies > 0)
    {
      myData.myValue->x_cm = cmx/myData.myValue->numBodies;
      myData.myValue->y_cm = cmy/myData.myValue->numBodies;
    }

    //#ifdef DBG
    printf("--> Rank: %d numBodies: %d tot_wt: %lg x_cm: %lg y_cm: %lg x_min: %lg x_max: %lg y_min: %lg y_max: %lg\n", myRank, myData.myValue->numBodies, myData.myValue->wt_tot, myData.myValue->x_cm, myData.myValue->y_cm, myData.myValue->x_min, myData.myValue->x_max, myData.myValue->y_min, myData.myValue->y_max); 
    //#endif

    MPI_Send(myData.myValue, 1, mpi_node, myData.parentRank, TAG_NODE, MPI_COMM_WORLD); 
  }
  else if (myRank < s_proc)
  {
    // Receive data from all the children in case this is an 
    // internal node. The data from children is saved in order.
    for (idx = 0; idx < numChildren; idx++)
    {
      MPI_Recv(&tempNode, 1, mpi_node, MPI_ANY_SOURCE, TAG_NODE, MPI_COMM_WORLD, &status);
      childIdx = (status.MPI_SOURCE - 1)%numChildren;
      myData.childValues[childIdx] = tempNode;
    }

    // Compute center of mass and total mass from the data sent by children
    // Store the net center of mass in myData->myValue
    for (idx = 0; idx < numChildren; idx++)
    {
      tm += myData.childValues[idx].wt_tot;
      cmx += (myData.childValues[idx].x_cm * myData.childValues[idx].numBodies);
      cmy += (myData.childValues[idx].y_cm * myData.childValues[idx].numBodies);
      num += myData.childValues[idx].numBodies;
    }
    
    myData.myValue->wt_tot = tm;
    myData.myValue->numBodies = num;
    
    if (myData.myValue->numBodies > 0)
    {
      myData.myValue->x_cm = cmx/myData.myValue->numBodies;
      myData.myValue->y_cm = cmy/myData.myValue->numBodies;
    }
    
    if (myRank != 0)
    {
      myData.myValue->x_min = myData.childValues[0].x_min;
      myData.myValue->x_max = myData.childValues[1].x_max;
      myData.myValue->y_min = myData.childValues[0].y_min;
      myData.myValue->y_max = myData.childValues[2].y_max;

      MPI_Send(myData.myValue, 1, mpi_node, myData.parentRank, TAG_NODE, MPI_COMM_WORLD);
    }
  }
}

void constructLocallyEssentialTree()
{
  int idx;
  struct node tempNode;

  if (myRank < cSize)
  {
    allProcData[myRank] = *myData.myValue;

    MPI_Allreduce(MPI_IN_PLACE, allProcData, cSize, mpi_node, mpi_node_sum, mpi_comm_nodes);

#ifdef DBG
    if (myRank == 0)
    {
      printf("\nLocally Essential Tree at node: 0\n---------------------------------\n");
      for (idx = 0; idx < cSize; idx++)
      {
	printf("++++ Rank: %d x_cm: %lg y_cm: %lg wt: %lg numBodies: %d x_min: %lg x_max: %lg y_min: %lg y_max: %lg\n", idx, allProcData[idx].x_cm, allProcData[idx].y_cm, allProcData[idx].wt_tot, allProcData[idx].numBodies, allProcData[idx].x_min, allProcData[idx].x_max, allProcData[idx].y_min, allProcData[idx].y_max);
      }
    }
#endif
  }
}

double multipole(struct body* aBody, int procRank)
{
  double ratio, dist, width;

  // compute and return the ratio of the width of the bounding box
  // (we are assuming the y-coordinate for the width measurement)
  // and the distance from the point to the center of mass
  width = allProcData[procRank].y_max - allProcData[procRank].y_min;
  dist = sqrt(pow(allProcData[procRank].x_cm - aBody->x, 2) + 
	      pow(allProcData[procRank].y_cm - aBody->y, 2));

  ratio = width/dist;
#ifdef DBG
  printf("Multipole: Body (%lg, %lg) of rank: %d for target rank: %d is: %lg\n", aBody->x, aBody->y, myRank, procRank, ratio);
#endif
  return ratio;
}

void computeInteractionWithCM(struct body* aBody, int procRank)
{
  // compute the interaction with the center of mass of the intended
  // processor using the x_cm, y_cm and wt_tot fields
#ifdef DBG
  printf("+++ Rank: %d, body (%lg, %lg) computing interaction with CM of Rank: %d, CM (%lg, %lg)\n", myRank, aBody->x, aBody->y, procRank, allProcData[procRank].x_cm, allProcData[procRank].y_cm);
#endif
  compute_accln(aBody, 
		allProcData[procRank].x_cm, 
		allProcData[procRank].y_cm, 
		allProcData[procRank].wt_tot);
}

void startSending()
{
  int dummyVal = 0;
  MPI_Send(&dummyVal, 1, MPI_INT, 0, TAG_START, MPI_COMM_WORLD);
}

void stopSending()
{
  struct bodyProcTuple dummyVal = {0};
  MPI_Send(&dummyVal, 1, mpi_tuple, 0, TAG_STOP, MPI_COMM_WORLD);
}

void computeInteractionWithAllBodies(struct body* aBody, int index, int procRank)
{
  struct bodyProcTuple tempBodyProcTuple;
  int idx;
 
  // We'll be shipping off the body to the target process
  // so that the process can compute the force on it due to all 
  // its bodies. After this computation, the updated body is returned
#ifdef DBG
  printf("--- Rank: %d, body (%lg, %lg) interacting with all bodies of Rank: %d\n", myRank, aBody->x, aBody->y, procRank);
#endif

  if (myRank == procRank)
  {
    for (idx = 0; idx < myData.myValue->numBodies; idx++)
    {
      compute_accln(aBody, myData.bodies[idx].x, myData.bodies[idx].y, myData.bodies[idx].wt);
    }
  }
  else
  {
    tempBodyProcTuple.aBody = *aBody;
    tempBodyProcTuple.aBody.ax = 0;
    tempBodyProcTuple.aBody.ay = 0;
    tempBodyProcTuple.sourceProcRank = myRank;
    tempBodyProcTuple.destProcRank = procRank;
    tempBodyProcTuple.sourceIndex = index;
    
    MPI_Send(&tempBodyProcTuple, 1, mpi_tuple, 0, TAG_TUPLE, MPI_COMM_WORLD);
  }
}

void receiveBodies()
{
  int idx, dummyVal;
  struct bodyProcTuple tempTuple = {0};
  MPI_Status status;

  for (idx = s_proc; idx < cSize; idx++)
  {
    MPI_Recv(&dummyVal, 1, MPI_INT, MPI_ANY_SOURCE, TAG_START, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  while (1)
  {
    MPI_Recv(&tempTuple, 1, mpi_tuple, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    if (status.MPI_TAG == TAG_STOP)
    {
      break;
    }

    if (numBodiesToSend % 1000 == 0)
    {
      if (bodiesToSend == NULL)
	bodiesToSend = (struct bodyProcTuple *)malloc(1000 * sizeof(struct bodyProcTuple));
      else
	bodiesToSend = (struct bodyProcTuple *)realloc(bodiesToSend, (numBodiesToSend + 1000) * sizeof(struct bodyProcTuple));
    }

    bodiesToSend[numBodiesToSend] = tempTuple;
    numBodiesToSend++;
  }
  
  for (idx = s_proc; idx < cSize - 1; idx++)
  {
    MPI_Recv(&tempTuple, 1, mpi_tuple, MPI_ANY_SOURCE, TAG_STOP, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
}

void computeInteraction(struct body* aBody, int index, int procRank)
{
  int idx;

  if (multipole(aBody, procRank) < theta)
  {
    // compute force using the center of mass and 
    // total mass of the processor block 
    computeInteractionWithCM(aBody, procRank);
  }
  else
  {
    if (procRank >= s_proc)
    {
      // If this is a leaf node, then stop the recursion and 
      // compute the interactions with all the bodies in the leaf node
      computeInteractionWithAllBodies(aBody, index, procRank);
    }
    else
    {
      // If this is an internal node, then recursively call this very 
      // function to compute the interaction with each of the 4 children
      for (idx = 1; idx <= numChildren; idx++)
      {
	computeInteraction(aBody, index, procRank * numChildren + idx);
      }
    }
  }
}

void updateBody(const struct bodyProcTuple* temp)
{
  struct body* theBody = &myData.bodies[temp->sourceIndex];
  theBody->ax += temp->aBody.ax;
  theBody->ay += temp->aBody.ay;
}

void receiveComputeSendReceive()
{
  struct bodyProcTuple tempTuple = {0};
  MPI_Status status;
  int idx, idx2;

  while (1)
  {
    MPI_Recv(&tempTuple, 1, mpi_tuple, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    if (status.MPI_TAG == TAG_STOP)
    {
      break;
    }

    if (numBodiesToSend % 1000 == 0)
    {
      if (bodiesToSend == NULL)
	bodiesToSend = (struct bodyProcTuple *)malloc(1000 * sizeof(struct bodyProcTuple));
      else
	bodiesToSend = (struct bodyProcTuple *)realloc(bodiesToSend, (numBodiesToSend + 1000) * sizeof(struct bodyProcTuple));
    }

    bodiesToSend[numBodiesToSend] = tempTuple;
    numBodiesToSend++;
  }

  startSending();

  for (idx = 0; idx < numBodiesToSend; idx++)
  {
    for (idx2 = 0; idx2 < myData.myValue->numBodies; idx2++) 
    {
      compute_accln(&bodiesToSend[idx].aBody, myData.bodies[idx2].x, myData.bodies[idx2].y, myData.bodies[idx2].wt);
    }
    
    MPI_Send(&bodiesToSend[idx], 1, mpi_tuple, 0, TAG_TUPLE, MPI_COMM_WORLD);
  }

  MPI_Send(&tempTuple, 1, mpi_tuple, 0, TAG_STOP, MPI_COMM_WORLD);

  numBodiesToSend = 0;

  MPI_Barrier(mpi_comm_leaf);
 
  while (1)
  {
    MPI_Recv(&tempTuple, 1, mpi_tuple, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    if (status.MPI_TAG == TAG_STOP)
    {
      break;
    }
    
    updateBody(&tempTuple);
  }
}

void computeForces()
{
  int idx, dummyVal = 0;
  struct bodyProcTuple tempTuple = {0};

  if (myRank >= s_proc && myRank < cSize)
  {
#ifdef DBG
    for (idx = 0; idx < myData.myValue->numBodies; idx++)
    {
      printf("{R: %d, (%lg, %lg)}\n", myRank, myData.bodies[idx].x, myData.bodies[idx].y); 
    }
#endif

    startSending();

    MPI_Barrier(mpi_comm_leaf);
    
    for (idx = 0; idx < myData.myValue->numBodies; idx++)
    {
      computeInteraction(&myData.bodies[idx], idx, 0);
    }

    MPI_Barrier(mpi_comm_leaf);

    stopSending();

    receiveComputeSendReceive();
  }
  else if (myRank == 0)
  {
    receiveBodies();

    for (idx = 0; idx < numBodiesToSend; idx++)
    {
      MPI_Send(&bodiesToSend[idx], 1, mpi_tuple, bodiesToSend[idx].destProcRank, TAG_TUPLE, MPI_COMM_WORLD);
    }

    for (idx = s_proc; idx < cSize; idx++)
    {
      MPI_Send(&tempTuple, 1, mpi_tuple, idx, TAG_STOP, MPI_COMM_WORLD);
    }

    numBodiesToSend = 0;

    receiveBodies();

    for (idx = 0; idx < numBodiesToSend; idx++)
    {
      MPI_Send(&bodiesToSend[idx], 1, mpi_tuple, bodiesToSend[idx].sourceProcRank, TAG_TUPLE, MPI_COMM_WORLD);
    }

    for (idx = s_proc; idx < cSize; idx++)
    {
      MPI_Send(&tempTuple, 1, mpi_tuple, idx, TAG_STOP, MPI_COMM_WORLD);
    }
    
    numBodiesToSend = 0;
  }
}

void cleanupStuff()
{
  if (myRank < cSize)
  {
    if (myRank >= s_proc)
    {
      memset(myData.bodies, 0, myData.myValue->numBodies * sizeof(struct body));
    }
    else
    {
      memset(myData.childValues, 0, numChildren * sizeof(struct node));
    }
    
    memset(myData.myValue, 0, sizeof(struct node));
  }
}

void returnToSender()
{
  int tempVal, idx;
  MPI_Status status;

  if (myRank == 0)
  {
    memset(nBodies, 0, numTotalBodies * sizeof(struct body));
    
    for (idx = 0; idx < numTotalBodies; idx++)
    {
      MPI_Recv(&nBodies[idx], 1, mpi_body, MPI_ANY_SOURCE, TAG_BODY, MPI_COMM_WORLD, &status);
#ifdef DBG
      printf("Recvd: (%lg, %lg) from %d\n", nBodies[idx].x, nBodies[idx].y, status.MPI_SOURCE);
#endif
    }
  }
  else if (myRank >= s_proc && myRank < cSize)
  {
#ifdef DBG
    printf("Here for: %d with %d bodies\n", myRank, myData.myValue->numBodies);
#endif
    for (idx = 0; idx < myData.myValue->numBodies; idx ++)
    {
      MPI_Send(&myData.bodies[idx], 1, mpi_body, 0, TAG_BODY, MPI_COMM_WORLD);
    }
  }  
}

void moveBodies()
{
  int idx;

  if (myRank >= s_proc && myRank < cSize)
  {
    for (idx = 0; idx < myData.myValue->numBodies; idx++)
    {
      move_body(&myData.bodies[idx]);
    }
  }
}

void parallelBarnesHut(const char* inputFileName, const char* outputFileName)
{
  int iter = 0, idx;
  
  allProcData = (struct node*)calloc(cSize, sizeof(struct node));

  initTypes();
  
  // one-time action - read the input file and store the data in the 
  // nBodies array
  if (myRank == 0)
    readFile(inputFileName);

  // a one-time action of identifying the internal and leaf-nodes
  computeHierarchy();

  while (iter < numIterations)
  {
#ifdef DBG
    if (myRank == 0)
      printf("STARTING ITERATION %d\n", iter);
#endif

    // send the data to each node
    sendReceiveCalcLimits();

    // calculate the total mass and center of mass of each of the nodes
    // of the tree
    calculateTMandCM();

    // have all the nodes receive the entire tree that has the 
    // center of mass data for all the processors
    constructLocallyEssentialTree();

    // compute the forces for all the leaf nodes
    computeForces();

    // move the bodies
    moveBodies();

    // return all the bodies to the root node
    returnToSender();
    
    // Make sure everyone is on the same page!
    MPI_Barrier(MPI_COMM_WORLD);

    // clean up resources and prepare for next iteration
    cleanupStuff();
    iter++;
  }

  if (myRank == 0)
  {
    // Write the output to file
    writeFile(outputFileName);

    free(nBodies);
  }
  
  if (bodiesToSend != NULL)
    free(bodiesToSend);
  
  if (myRank >= s_proc && myRank < cSize)
  {
    free(allProcData);
    free(myData.myValue);
    free(myData.childValues);
  }
  
  if (myRank == 0)
  {
     for (idx = 0; idx < sqrt_part; idx++)
       free(rankMatrix[idx]);
     free(rankMatrix);
  }
}
#endif // SERIAL
