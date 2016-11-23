#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

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
// Структура http данных
struct http_data {
	string					http;		// http запрос
	size_t					length = 0;	// Количество заголовков
	map <string, string>	headers;	// Заголовки http запроса
};
/**
 * split Функция разделения строк на составляющие
 * @param str   строка для поиска
 * @param delim разделитель
 * @param v     результирующий вектор
 */
void split(const string &str, const string delim, vector <string> &v){
	string::size_type i = 0;
	string::size_type j = str.find(delim);
	u_int len = delim.length();
	// Выполняем разбиение строк
	while(j != string::npos){
		v.push_back(str.substr(i, j - i));
		i = ++j + (len - 1);
		j = str.find(delim, j);
		if(j == string::npos) v.push_back(str.substr(i, str.length()));
	}
}
/**
 * getHeaders Функция извлечения данных http запроса
 * @param  str строка http запроса
 * @return     данные http запроса
 */
http_data getHeaders(string str){
	// Создаем структуру данных
	http_data data;
	// Проверяем существуют ли данные
	if(str.length()){
		// Если конец запроса найден
		if(str.find("\r\n\r\n") != string::npos){
			// Массив строк
			vector <string> strings, params;
			// Выполняем разбиение строк
			split(str, "\r\n", strings);
			// Если строки найдены
			if(strings.size()){
				// Позиция найденного разделителя
				size_t pos;
				// Запоминаем http запрос
				data.http = strings[0];
				// Переходим по всему массиву строк
				for(int i = 1; i < strings.size(); i++){
					// Выполняем поиск разделитель
					pos = strings[i].find(":");
					// Если разделитель найден
					if(pos != string::npos){
						// Получаем ключ
						string key = strings[i].substr(0, pos);
						// Получаем значение
						string val = strings[i].substr(pos + 1, strings[i].length() - (pos + 1));
						// Запоминаем найденны параметры
						data.headers.insert(pair <string, string>(trim(key), trim(val)));
					}
				}
				// Получаем длину массива заголовков
				data.length = data.headers.size();
			}
		}
	}
	// Выводим результат
	return data;
}

int main(int argc, char * argv[]){
	
	// Тестовый заголовок
	const char * str = "POST http://ocsp.digicert.com/ HTTP/1.1\r\n"
				"Host: ocsp.digicert.com\r\n"
				"User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.12; rv:49.0) Gecko/20100101 Firefox/49.0\r\n"
				"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
				"Accept-Language: ru-RU,ru;q=0.8,en-US;q=0.5,en;q=0.3\r\n"
				"Accept-Encoding: gzip, deflate\r\n"
				"Content-Length: 83\r\n"
				"Content-Type: application/ocsp-request\r\n"
				"Proxy-Authorization: Basic emRENzg2S2V1UzprLmZyb2xvdnZAZ21haWwuY29t\r\n"
				"Connection: keep-alive\r\n"
				"\r\n";
	/*
	// Выполняем парсинг http запроса
	http_data data = getHeaders(str);

	// Если данные существуют
	if(data.length){

		cout << "http = " << data.http.c_str() << endl;

		cout << "Content-Length = " << data.headers.find("Content-Length")->second << endl;

		for(map <string, string>::iterator it = data.headers.begin(); it != data.headers.end(); ++it)
			cout << it->first << " => " << it->second << '\n';
	}
	*/

	char dd[13] = "Hello World!";

	dd[12] = '\0';

	vector <char> v;

	v.assign(dd, dd + 13);

	for(int i = 0; i < v.size(); i++){
		cout << " ++++ " << v[i] << endl;
	}

	const char * kk = v.data();

	cout << " --- " << kk << endl;

	return 0;
}