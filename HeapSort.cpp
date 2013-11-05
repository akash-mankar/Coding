#include <iostream>

#define SIZE 20

using namespace std;

void heapInsert(int);
void sortHeap(int);

int array[SIZE] = {0};
int arraySize = SIZE;

int main()
{
  int i;
  
  srand(time(NULL));
    
  cout << "Original Array:" << endl;

  for (i = 0; i < SIZE; i++)
  {
    int elem = rand() % 100;
    array[i] = elem;
    cout << array[i] << " ";
    heapInsert(i);
  }

  cout << endl << "Max Heap:" << endl;

  for (i = 0; i < SIZE; i++)
  {
    cout << array[i] << " ";
  }
  cout << endl << "Sorted Array:" << endl;
  
  for (i = 0; i < SIZE; i++)
  {
    sortHeap(i);
  }
  
  for (i = 0; i < SIZE; i++)
  {
    cout << array[i] << " ";
  }
  cout << endl;

  return 0;
}

void heapInsert(int index)
{
  int parentIndex = (index - 1)/2;

  if (parentIndex >= 0)
  {
    if (array[parentIndex] < array[index])
    {
      int temp = array[parentIndex];
      array[parentIndex] = array[index];
      array[index] = temp;
      heapInsert(parentIndex);
    }
  }
}

void sortHeap(int index)
{
  int lastIndex = SIZE - 1 - index;
  int currIndex = 0;
  int temp;

  temp = array[lastIndex];
  array[lastIndex] = array[0];
  array[0] = temp;

  arraySize --;

  while (1)
  {
    int leftIndex = (2*currIndex + 1);
    int rightIndex = (2*currIndex + 2);
    int swapIndex;
    
   if ((currIndex >= arraySize/2) ||
	(rightIndex == arraySize && array[currIndex] >= array[leftIndex]) ||
	(array[currIndex] >= array[leftIndex] && array[rightIndex]))
    {
      break;
    }

    if (rightIndex == arraySize ||
	array[leftIndex] > array[rightIndex])
    {
      swapIndex = leftIndex;
    }
    else
    {
      swapIndex = rightIndex;
    }

    temp = array[currIndex];
    array[currIndex] = array[swapIndex];
    array[swapIndex] = temp;
    currIndex = swapIndex;
  }
}


