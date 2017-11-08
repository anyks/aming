/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       11/08/2017 16:52:48
*  copyright:  © 2017 anyks.com
*/
#include <thread>
#include <vector>
#include <string>
#include <iostream>

using namespace std;



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