#include <thread>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

// clang++ -Wall -O3 -pedantic -ggdb -g -std=c++11 -Werror=vla -o ./bin/thread ./examples/thread.cpp

using namespace std;

void threadFunction(vector <int> * a){
     (* a)[0] += 15;
}

int main(int argc, char * argv[]){
	int d = 10;
	vector <int> k;

	vector <int> * t = &k;

	k.push_back(d);

	thread thr(threadFunction, t);
	thr.join();
	cout << k[0] << endl;
	return 0;
}