#pragma once 
#include <vector>
#include <string>
using namespace std;

struct node
{ 
  int label;
  vector <struct edge*> inEdges;
  vector <struct edge*> outEdges;
  vector <struct node*> optimal;
 
  node(int l)
  { 
    label = l;
  }
};

struct edge
{
  struct node* source;
  struct node* destination;
  int weight;

  edge(struct node* src, struct node* dest, int wt)
  {
    source = src;
    destination = dest;
    weight = wt;
    source->outEdges.push_back(this);
    destination->inEdges.push_back(this);
  }
};

