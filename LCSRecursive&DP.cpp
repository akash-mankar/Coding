#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>

using namespace std;

int DynamicLCS(string s1, string s2, int row, int column)
{
	int **K = new int*[row];
	
	for (int i=0; i<= row; i++)
	{
		K[i] =  new int[column];
	}

   for (int i=0; i<=row; i++)
   {
     for (int j=0; j<=column; j++)
     {
       if (i == 0 || j == 0)
         K[i][j] = 0;
  
       else if(s1[i-1] == s2[j-1])
         K[i][j] = K[i-1][j-1] + 1;
  
       else
         K[i][j] = max(K[i-1][j], K[i][j-1]);
     }
   }

   return K[row][column];
}

int LCS (string s1, string s2, int row, int column)
{
	if (row == 0 || column == 0)
		return 0;

	if(s1[row-1] == s2 [column-1])
		return (1+LCS(s1, s2, row-1, column-1));
	else
		return max(LCS(s1, s2, row-1, column), LCS(s1, s2, row, column -1));
}

int main(int argc, char* argv[])
{
	cout << "Enter the two Strings";

	string s1, s2;
	cin >> s1 >> s2;

	//int longest = LCS(s1, s2, s1.length(),s2.length());

	int longest = DynamicLCS(s1, s2, s1.length(),s2.length());
	cout<< "Longest Common Subsequence length " << longest; 
	getchar();
	getchar();
}