/* МОДУЛЬ ОБЩИХ ФУНКЙИЙ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _GENERAL_ANYKS_
#define _GENERAL_ANYKS_

#include <string>
#include <vector>
#include <stdlib.h>
#include <sys/types.h>

// Устанавливаем область видимости
using namespace std;

/**
 * cpydata Метод копирования из сырых данных в рабочие
 * @param data   буфер сырых данных
 * @param size   размер сырых данных
 * @param it     смещение
 * @param result результат куда копируются данные
 */
void cpydata(const u_char * data, size_t size, size_t &it, void * result);
/**
 * cpydata Метод копирования из сырых данных в рабочие
 * @param data   буфер сырых данных
 * @param size   размер сырых данных
 * @param it     смещение
 * @param result результат куда копируются данные
 */
void cpydata(const u_char * data, size_t size, size_t &it, string &result);
/**
 * toCase Функция перевода в указанный регистр
 * @param  str  строка для перевода в указанных регистр
 * @param  flag флаг указания типа регистра
 * @return      результирующая строка
 */
const string toCase(string str, bool flag = false);
/**
 * isNumber Функция проверки является ли строка числом
 * @param  str строка для проверки
 * @return     результат проверки
 */
const bool isNumber(const string &str);
/**
 * rtrim Функция усечения указанных символов с правой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & rtrim(string &str, const char * t = " \t\n\r\f\v");
/**
 * ltrim Функция усечения указанных символов с левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & ltrim(string &str, const char * t = " \t\n\r\f\v");
/**
 * trim Функция усечения указанных символов с правой и левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & trim(string &str, const char * t = " \t\n\r\f\v");
/**
 * checkPort Функция проверки на качество порта
 * @param  port входная строка якобы содержащая порт
 * @return      результат проверки
 */
const bool checkPort(string str);
/**
 * replace Функция замены указанных фраз в строке
 * @param s строка в которой происходит замена
 * @param f искомая строка
 * @param r строка на замену
 */
void replace(string &s, const string f, const string r);
/**
 * split Функция разделения строк на составляющие
 * @param str   строка для поиска
 * @param delim разделитель
 * @param v     результирующий вектор
 */
void split(const string &str, const string delim, vector <string> &v);
/**
 * split Метод разбива строки на составляющие
 * @param  str   исходная строка
 * @param  delim разделитель
 * @return       массив составляющих строки
 */
vector <string> split(const string str, const string delim);

#endif // _GENERAL_ANYKS_