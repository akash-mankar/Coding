#include <iostream>

using namespace std;

void mergeSort(int* A, int lenA, int lenB);

int main(int argc, char* argv[])
{
  srand(time(NULL));

  int *arr = new int[10];

  for (int i = 0; i< 10; i++)
  {
    int temp = rand() % 100;
    arr[i] = temp;
    cout << temp << " ";
  }
  cout << endl;
  
  mergeSort(arr, 5, 5);

  for (int i = 0; i < 10; i++)
  {
    cout << arr[i] << " ";
  }
  cout << endl;

  return 0;
}

void mergeSort(int* A, int lenA, int lenB)
{
  int index = 0, posA = 0, posB = lenA;
 
  if (lenA != 1)
  {  
    mergeSort(A, lenA/2, (lenA - lenA/2));
  }

  if (lenB != 1)
  {
    mergeSort(A + lenA, lenB/2, (lenB - lenB/2));
  }
  
  int mid = lenA;
  
  while (posA < mid && posB < (lenA + lenB))
  {
    if (A[posA] <= A[posB])
	posA++;
    else
    {
      int temp = A[posB];
      
      for (int i = posB; i > posA; i--)
      {
	A[i] = A[i-1];
      }
      A[posA] = temp;
      posA++;
      mid++;
      posB++;
    }
  }
}
