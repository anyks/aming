/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 16:59:42
*  copyright:  © 2017 anyks.com
*/
#include <iostream>
#include <bitset>




#include <iostream>
using namespace std;





void PermutationWithRepetition(const char* pIn, int N, int K)
{
  char* pOut = new char[K + 1]; 
  pOut[K] = 0;                  
  K--;
  int *stack = new int[K * 2],  
      *pTop = stack,            
      k = 0,                    
      n = 0,
      j = 0;
  for (;;)                      
  {
    while(n < N)
    {
      pOut[k] = pIn[n++];
      if (k == K)
        printf("%02d. %s\n", ++j, pOut);
      else
      {
        if (n < N)
        {
          *pTop++ = k;          
          *pTop++ = n;
        }
        k++;                    
        n = 0;
      }
    }
    if (pTop == stack)          
      break;

    n = *(--pTop);              
    k = *(--pTop);
  }
  delete[] pOut;
  delete[] stack;
}

bool NextSet(int *a, int n, int m)
{
  int j = m - 1;
  while (a[j] == n && j >= 0) j--;
  if (j < 0) return false;
  if (a[j] >= n)
    j--;
  a[j]++;
  if (j == m - 1) return true;
  for (int k = j + 1; k < m; k++)
    a[k] = a[j];
  return true;
}
void Print(int *a, int n) 
{
  static int num = 1;
  cout.width(3);
  cout << num++ << ": ";
  for (int i = 0; i < n; i++)
    cout << a[i] << " ";
  cout << endl;
}
int main() 
{
  int n, m, *a;
  cout << "N = ";
  cin >> n;
  cout << "M = ";
  cin >> m;


  

  int h = n > m ? n : m; 
  a = new int[h];
  for (int i = 0; i < h; i++)
    a[i] = 1;
  Print(a, m);
  while (NextSet(a, n, m))
    Print(a, m);
  cin.get(); cin.get();
  return 0;
}


	 