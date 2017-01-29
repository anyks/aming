#include <string>
#include <iostream>

// clang++ -Wall -O3 -pedantic -ggdb -g -std=c++11 -Werror=vla -o ./bin/test ./test.cpp

using namespace std;

int main(int argc, char * argv[]){
	
	string s = "/Volumes/Data/Work/tt.text";
	string r = "&#47;"; // &frasl;
	string f = "/";

	for(string::size_type n = 0; (n = s.find(f, n)) != string::npos; ++n){
		s.replace(n, f.length(), r);
	}

	cout << '[' << s << ']' << endl;

	f = "&#47;";
	r = "/";

	for(string::size_type n = 0; (n = s.find(f, n)) != string::npos; ++n){
		s.replace(n, f.length(), r);
	}

	cout << '[' << s << ']' << endl;
}