/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/27/2017 18:42:35
*  copyright:  © 2017 anyks.com
*/
#include <string>
#include <iostream>



using namespace std;

int main(int argc, char * argv[]){
	
	string s = "/Volumes/Data/Work/tt.text";
	string r = "&#47;"; 
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