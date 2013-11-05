#include <iostream>

using namespace std;

int times = 0;

void reverse(char* strToReverse, int len)
{
  int i;
  char temp;

  for (i = 0; i < len/2; i++)
  {
    times ++;
    temp = strToReverse[len - 1 - i];
    strToReverse[len - 1 - i] = strToReverse[i];
    strToReverse[i] = temp;
  }
}


int main()
{
  char* str = new char[512]; 
  char* temp = str;

  cin.getline(str, 512);

  reverse(str, strlen(str));

  for (int i = 0; i <= strlen(str); i++)
  {
    if (str[i] == ' ' || str[i] == '\0')
    {
      reverse(temp, (&str[i] - temp));
      temp = &str[i+1];
    }
  }	
  cout << str  << endl << strlen(str) << endl << times << endl;

  return 0;
}
