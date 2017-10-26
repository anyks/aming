// $ g++ -std=c++11 ./examples/tests.cpp -o ./bin/tests
// $ ./bin/tests

#include <regex>
#include <ctime>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <math.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

/**
 * rtrim Функция усечения указанных символов с правой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & rtrim(string &str, const char * t = " \t\n\r\f\v"){
	str.erase(str.find_last_not_of(t) + 1);
	return str;
}
/**
 * ltrim Функция усечения указанных символов с левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & ltrim(string &str, const char * t = " \t\n\r\f\v"){
	str.erase(0, str.find_first_not_of(t));
	return str;
}
/**
 * trim Функция усечения указанных символов с правой и левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & trim(string &str, const char * t = " \t\n\r\f\v"){
	return ltrim(rtrim(str, t), t);
}
/**
 * split Метод разбива строки на составляющие
 * @param  str   исходная строка
 * @param  delim разделитель
 * @return       массив составляющих строки
 */
vector <string> split(const string str, const string delim){
	// Результат данных
	vector <string> result;
	// Создаем новую строку
	string value = str;
	// Убираем пробелы в строке
	value = trim(value);
	// Если строка передана
	if(!value.empty()){
		string data;
		string::size_type i = 0;
		string::size_type j = value.find(delim);
		u_int len = delim.length();
		// Выполняем разбиение строк
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
		// Если данные не существуют то устанавливаем строку по умолчанию
		if(result.empty()) result.push_back(value);
	}
	// Выводим результат
	return result;
}

/*
map <string, auto> getDomain(const string str){
	// Создаем список данных
	map <string, auto> data;
	map <string, string> result;

	result.insert(pair <string, string> ("www", "rambler"));
	data.insert(pair <string, auto> ("com", result));
	//
	// Название ключа
	string key = "";
	// Разбиваем строку на составляющие
	vector <string> params = (oldData.empty() ? split(str, ".") : oldData), newParams;
	// Получаем размер массива
	const size_t len = params.size();
	// Если количество элементов больше 1
	if(len > 1){
		// Выполняем реверс данных
		reverse(begin(params), end(params));
		// Переходим по всему массиву данных
		for(u_int i = 0; i < len; i++){
			// Если это первый элемент то запоминаем его
			if(i == 0) key = params[i];
			// Формируем новый массив
			else newParams.push_back(params[i]);
		}
	// Копируем первый элемент
	} else key = params[0];
	// Заполняем структуру данных
	data.insert(pair <string, T> (key, rules));


	// {"com": {"co": {"rambler": "www"}}};
	//

	return data;
}
*/

/*
template <typename T, typename E>
auto compose(T a, E b) -> decltype(a+b) // decltype - позволяет определить тип на основе входного параметра
{
   return a+b;
}
auto c = compose(2, 3.14); // c - double
*/

// https://habrahabr.ru/post/182920/

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
	// Если строка передана
	if(!str.empty()){
		// Ключ и значение
		string key, value;
		// Значение домена
		map <string, string> data;

		//map <string, DD> k;
		
		
		
		// Получаем список параметров домена
		auto domain = split(str, ".");
		// Переходим по всему массиву
		for(u_int i = 0; i < domain.size(); i++){
			// Если значение не установлено
			if(value.empty()) value = domain[i];
			// Если ключ не установлен
			else if(key.empty()) key = domain[i];
			// Если пройдены два элемента и значения заполнены
			if(i > 1){
				// Формируем новый объект данных
				auto b = compose(domain[i], data);

				
				auto print = [b](int value){
					cout << value << "\n";
				};

				print(3);

			// Если ключ и значения заполнены
			} else if(!value.empty() && !key.empty()) {
				// Запоминаем данные объекта
				data = compose(key, value);
			}
		}
	}
}

int main(int len, char * buff[]){
	/*
	auto a = compose(string("rambler"), string("www"));
	a.insert(pair <string, string> (string("anyks"), string("ws")));

	auto b = compose("co", a);
	auto c = compose("com", b);




	int* p2 = nullptr;

	getDomain("www.rambler.co.com");

	cout << " ++++++++ " << c["com"]["co"]["anyks"] << endl;
	// auto d = getDomain("www.rambler.com");
	*/

	vector <int> result = {34, 15, 24, 1, 10, 92, 34, 11, 10, 15, 15, 24};

	cout << endl;

	for(auto it = result.cbegin(); it != result.cend(); ++it){
		cout << " (1): " << (* it) << " ";
	}

	cout << endl;

	// Сортируем
	sort(result.begin(), result.end());
	
	for(auto it = result.cbegin(); it != result.cend(); ++it){
		cout << " (2): " << (* it) << " ";
	}

	cout << endl;

	// Удаляем дубликаты
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