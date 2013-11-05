#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
  char* str = argv[1];
  int len = strlen(str), N = 2*len+1;
  char* hashStr = new char[N];
  int* hashInt = new int[N];
  bool hash = true;
  int i = 0, index = 0;
  
  while (i <= len)
  {
    hashInt[index] = 0;
    if (hash)
    {
      hash = false;
      hashStr[index++] = '#';
    }
    else
    {
      hashStr[index++] = str[i];
      hash = true;
      if (i == len)
      {
	hashStr[index] = 0;
	break;
      }
      i++;
    }
  }

  for (i = 1; i < N/2; i++)
  {
    for (int j = 0; j < N; j++)
    {
      if ((j - i >= 0) && (j + i < N) && (hashInt[j] == (i - 1)))
      {
		if (hashStr[j-i] == hashStr[j+i])
			hashInt[j]++;
      }
    }
  } 

  cout << hashStr << endl;
  for (i = 0; i < N; i++)
  {
    cout << hashInt[i];
  }
  cout << endl;
  return 0;
}
