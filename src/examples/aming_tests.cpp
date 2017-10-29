/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/29/2017 16:59:42
*  copyright:  © 2017 anyks.com
*/



#include <regex>
#include <ctime>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <math.h>
#include <sys/t/y/p/e/s/./h/>/
/#/i/n/c/l/u/d/e/ /</s/t/d/i/o/./h/>/
/#/i/n/c/l/u/d/e/ /</s/t/d/l/i/b/./h/>/
/
/u/s/i/n/g/ /n/a/m/e/s/p/a/c/e/ /s/t/d/;/
/

 * rtrim Функция усечения указанных символов с правой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
/s/t/r/i/n/g/ /&/ /r/t/r/i/m/(/s/t/r/i/n/g/ /&/s/t/r/,/ /c/o/n/s/t/ /c/h/a/r/  
string & ltrim(string &str, const char * t = " \t\n\r\f\v"){
	str.erase(0, str.find_first_not_of(t));
	return str;
}
 
string & trim(string &str, const char * t = " \t\n\r\f\v"){
	return ltrim(rtrim(str, t), t);
}
 
vector <string> split(const string str, const string delim){
	
	vector <string> result;
	
	string value = str;
	
	value = trim(value);
	
	if(!value.empty()){
		string data;
		string::size_type i = 0;
		string::size_type j = value.find(delim);
		u_int len = delim.length();
		
		while(j != string::npos){
			data = value.substr(i, j - i);
			result.push_back(trim(data));
			i = ++j + (len - 1);
			j = value.find(delim, j);
			if(j == string::npos){
				data = value.substr(i, value.length());
				result.push_back(trim(data));
			}
		}
		
		if(result.empty()) result.push_back(value);
	}
	
	return result;
}

 

 



template <typename T, typename E>
map <T, E> compose(T a, E b){
   map <T, E> data;
   data.insert(pair <T, E> (a, b));
   return data;
}

template <typename T>
struct DD {
	T val;
};

void getDomain(const string str){
	
	if(!str.empty()){
		
		string key, value;
		
		map <string, string> data;

		
		
		
		
		
		auto domain = split(str, ".");
		
		for(u_int i = 0; i < domain.size(); i++){
			
			if(value.empty()) value = domain[i];
			
			else if(key.empty()) key = domain[i];
			
			if(i > 1){
				
				auto b = compose(domain[i], data);

				
				auto print = [b](int value){
					cout << value << "\n";
				};

				print(3);

			
			} else if(!value.empty() && !key.empty()) {
				
				data = compose(key, value);
			}
		}
	}
}

int main(int len, char * buff[]){
	 

	vector <int> result = {34, 15, 24, 1, 10, 92, 34, 11, 10, 15, 15, 24};

	cout << endl;

	for(auto it = result.cbegin(); it != result.cend(); ++it){
		cout << " (1): " << (* it) << " ";
	}

	cout << endl;

	
	sort(result.begin(), result.end());
	
	for(auto it = result.cbegin(); it != result.cend(); ++it){
		cout << " (2): " << (* it) << " ";
	}

	cout << endl;

	
	result.resize(unique(result.begin(), result.end()) - result.begin());

	for(auto it = result.cbegin(); it != result.cend(); ++it){
		cout << " (3): " << (* it) << " ";
	}

	cout << endl;

	if(find(result.begin(), result.end(), 92) != result.end())
		cout << " ++++++++ YES +++++++++++ " << endl;
	else cout << " ++++++++ NO +++++++++++ " << endl;

	cout << endl;


	struct DD1 {
		int id;
		int dd = 2;
	};



	struct find_id : unary_function<DD1, bool> {
		int id;
		find_id(int id):id(id) { }
		bool operator()(DD1 const& m) const {
			return m.id == id;
		}
	};

	vector <DD1> data;

	for(int i = 0; i < 1000; i++){
		DD1 dd1;
		dd1.id = i;
		data.push_back(dd1);
	}

	auto it = find_if(data.begin(), data.end(), find_id(999));

	cout << " ---------- " << it->id << " == " << it->dd << endl;

	cout << endl;


	return 0;
}