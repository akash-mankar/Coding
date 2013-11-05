#include <iostream>
#include "header.h"
#include <climits>

vector<struct node*> linearDag;

void init()
{
  struct node *Start, *A, *B, *C, *D, *E, *F, *End;

  Start = new node(5);
  A = new node(2);
  B = new node(8);
  C = new node(6);
  D = new node(3);
  E = new node(6);
  F = new node(9);
  End = new node(7);

  linearDag.push_back(Start);
  linearDag.push_back(A);
  linearDag.push_back(B);
  linearDag.push_back(C);
  linearDag.push_back(D);
  linearDag.push_back(E);
  linearDag.push_back(F);
  linearDag.push_back(End);

  for (auto iter = linearDag.begin(); iter != linearDag.end(); iter++)
  {
    struct node* current = *(iter);

    for (auto innerIter = iter + 1; innerIter != linearDag.end(); innerIter++)
    {
      struct node* temp = *(innerIter);
      if (temp->label > current->label)
      {
        struct edge* newEdge = new edge(current, temp, 1);
      }
    } 
  }
}

int main()
{
  init();
  vector <struct node*> optimal;

  for (auto iter = linearDag.begin(); iter != linearDag.end(); iter++)
  {
    struct node* current = *(iter);
    vector <struct node*>& tempVector = current->optimal;
 
    for (auto edgeIter = current->inEdges.begin(); edgeIter != current->inEdges.end(); edgeIter++)
    {
      struct edge* currEdge = *(edgeIter);
      if (currEdge->source->optimal.size() > tempVector.size())
      {
        tempVector = currEdge->source->optimal;
      }
    }

    current->optimal.assign(tempVector.begin(), tempVector.end());
    current->optimal.push_back(current);

    if (current->optimal.size() > optimal.size())
    {
      optimal.clear();
      optimal.assign(current->optimal.begin(), current->optimal.end());
    }
  }
  
  for (auto iter = optimal.begin(); iter != optimal.end(); iter++)
  {
    cout << (*iter)->label << " ";
  }
  cout << endl;

  return 0;
}
