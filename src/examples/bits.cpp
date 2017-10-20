#include <iostream>
#include <bitset>

// g++ ./examples/bits.cpp -o ./bin/bits


#include <iostream>
using namespace std;

// pIn - входной массив
// N - размер входного массива
// K - количество элементов в размещении

void PermutationWithRepetition(const char* pIn, int N, int K)
{
  char* pOut = new char[K + 1]; // строка из K символов плюс 1 символ для терминального 0
  pOut[K] = 0;                  // помещаем 0 в конец строки
  K--;
  int *stack = new int[K * 2],  // стек псевдорекурсии, глубина рекурсии K - 1
      *pTop = stack,            // вершина стека
      k = 0,                    // переменные цикла
      n = 0,
      j = 0;
  for (;;)                      // цикл псевдорекурсии
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
          *pTop++ = k;          // сохраняем k и n в стеке
          *pTop++ = n;
        }
        k++;                    // псевдорекурсивный вызов
        n = 0;
      }
    }
    if (pTop == stack)          // стек пуст, конец цикла
      break;

    n = *(--pTop);              // выталкиваем k и n из стека
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


  

  int h = n > m ? n : m; // размер массива а выбирается как max(n,m)
  a = new int[h];
  for (int i = 0; i < h; i++)
    a[i] = 1;
  Print(a, m);
  while (NextSet(a, n, m))
    Print(a, m);
  cin.get(); cin.get();
  return 0;
}


	/*
int main()
{
	const int size = 5;
    int n = 3;
 
    for (int v = (1 << n) - 1; v < (1 << size); )
    {
        unsigned int t = (v | (v - 1)) + 1;
		v = t | ((((t & -t) / (v & -v)) >> 1) - 1);
        std::cout << std::bitset<size> (v).to_string() << std::endl;
	}
	

	int k[] = {1,0,0,1};

	PermutationWithRepetition((const char *) &k, 4, 5);
}
*/