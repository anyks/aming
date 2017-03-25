/* МОДУЛЬ ОБЩИХ ФУНКЙИЙ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#include "general/general.h"

// Устанавливаем область видимости
using namespace std;

/**
 * cpydata Метод копирования из сырых данных в рабочие
 * @param data   буфер сырых данных
 * @param size   размер сырых данных
 * @param it     смещение
 * @param result результат куда копируются данные
 */
void cpydata(const u_char * data, size_t size, size_t &it, void * result){
	// Извлекаем данные
	memcpy(result, data + it, size);
	// Определяем смещение
	it += size;
}
/**
 * cpydata Метод копирования из сырых данных в рабочие
 * @param data   буфер сырых данных
 * @param size   размер сырых данных
 * @param it     смещение
 * @param result результат куда копируются данные
 */
void cpydata(const u_char * data, size_t size, size_t &it, string &result){
	// Выделяем динамически память
	char * buffer = new char [size];
	// Извлекаем данные адреса
	memcpy(buffer, data + it, size);
	// Запоминаем результат
	result.assign(buffer, size);
	// Определяем смещение
	it += size;
	// Удаляем полученные данные
	delete [] buffer;
}
/**
 * toCase Функция перевода в указанный регистр
 * @param  str  строка для перевода в указанных регистр
 * @param  flag флаг указания типа регистра
 * @return      результирующая строка
 */
const string toCase(string str, bool flag){
	// Переводим в указанный регистр
	transform(str.begin(), str.end(), str.begin(), (flag ? ::toupper : ::tolower));
	// Выводим результат
	return str;
}
/**
 * isNumber Функция проверки является ли строка числом
 * @param  str строка для проверки
 * @return     результат проверки
 */
const bool isNumber(const string &str){
	return !str.empty() && find_if(str.begin(), str.end(), [](char c){
		return !isdigit(c);
	}) == str.end();
}
/**
 * rtrim Функция усечения указанных символов с правой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & rtrim(string &str, const char * t){
	str.erase(str.find_last_not_of(t) + 1);
	return str;
}
/**
 * ltrim Функция усечения указанных символов с левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & ltrim(string &str, const char * t){
	str.erase(0, str.find_first_not_of(t));
	return str;
}
/**
 * trim Функция усечения указанных символов с правой и левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & trim(string &str, const char * t){
	return ltrim(rtrim(str, t), t);
}
/**
 * checkPort Функция проверки на качество порта
 * @param  port входная строка якобы содержащая порт
 * @return      результат проверки
 */
const bool checkPort(string str){
	// Если строка существует
	if(!str.empty()){
		// Преобразуем строку в цифры
		if(isNumber(str)){
			// Получаем порт
			u_int port = ::atoi(str.c_str());
			// Проверяем диапазон портов
			if((port > 0) && (port < 65536)) return true;
		}
	}
	// Сообщаем что ничего не нашли
	return false;
}
/**
 * replace Функция замены указанных фраз в строке
 * @param s строка в которой происходит замена
 * @param f искомая строка
 * @param r строка на замену
 */
void replace(string &s, const string f, const string r){
	// Переходим по всем найденным элементам и заменяем в них искомые фразы
	for(string::size_type n = 0; (n = s.find(f, n)) != string::npos; ++n){
		// Заменяем искомую фразу указанной
		s.replace(n, f.length(), r);
	}
}
/**
 * split Функция разделения строк на составляющие
 * @param str   строка для поиска
 * @param delim разделитель
 * @param v     результирующий вектор
 */
void split(const string &str, const string delim, vector <string> &v){
	string::size_type i = 0;
	string::size_type j = str.find(delim);
	size_t len = delim.length();
	// Выполняем разбиение строк
	while(j != string::npos){
		v.push_back(str.substr(i, j - i));
		i = ++j + (len - 1);
		j = str.find(delim, j);
		if(j == string::npos) v.push_back(str.substr(i, str.length()));
	}
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