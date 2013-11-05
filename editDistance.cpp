#include <iostream>
#include <string>
#include <string.h>

using namespace std;


int minThree(int x, int y, int z)
{
  return (x < y ? (x < z ? x : (z < y ? z : y)) : (y < z ? y : (z < x ? z : x)));
}

int EditDistance(const string& A, const string& B)
{
  int** distance = new int*[A.size() + 1];
  for (int idx = 0; idx <= A.size(); idx++)
  {
    distance[idx] = new int[B.size() + 1];
    memset(distance[idx], 0, (B.size() + 1) * sizeof(int));
  }

  for (int idx = 0; idx <= A.size(); idx++)
  {
    distance[idx][0] = idx;
  }

  for (int idx = 0; idx <= B.size(); idx++)
  {
    distance[0][idx] = idx;
  }

  for (int idx = 1; idx <= A.size(); idx++)
  {
    for (int idx2 = 1; idx2 <= B.size(); idx2++)
    {
      if (A[idx-1] == B[idx2-1])
      {
        distance[idx][idx2] = minThree(1 + distance[idx-1][idx2], 1 + distance[idx][idx2-1], distance[idx-1][idx2-1]);
      }
      else
      {
        distance[idx][idx2] = minThree(1 + distance[idx-1][idx2], 1 + distance[idx][idx2-1], 1 + distance[idx-1][idx2-1]);
      }
    }
  }

  for (int idx = 0; idx <= A.size(); idx++)
  {
    for (int idx2 = 0; idx2 <= B.size(); idx2++)
    {
      cout << distance[idx][idx2] << "  ";
    }
    cout << endl;
  }

  return distance[A.size()][B.size()];
}


int main(int _argc, char** _argv)
{
  string A, B;
  int dist;

  if (_argc != 3)
    return -1;

  A = _argv[1];
  B = _argv[2];

  dist = EditDistance(A, B);

  cout << "Edit Distance of A and B: " << dist << endl;

  return 0;
}
