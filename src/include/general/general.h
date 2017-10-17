/* МОДУЛЬ ОБЩИХ ФУНКЙИЙ AMING */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _GENERAL_AMING_
#define _GENERAL_AMING_

#include <regex>
#include <pwd.h>
#include <grp.h>
#include <string>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>

// Типы данных системы
#define AMING_NULL 0x00
#define AMING_IP 0x01
#define AMING_NETWORK 0x02
#define AMING_IPV4 0x03
#define AMING_IPV6 0x04
#define AMING_DOMAIN 0x05
#define AMING_MAC 0x06
#define AMING_USER 0x07
#define AMING_GROUP 0x08
#define AMING_HTTP_ACTION 0x09
#define AMING_HTTP_METHOD 0x10
#define AMING_HTTP_TRAFFIC 0x11

// Типы кодировок паролей
#define AMING_TYPE_AUTH_BASIC 0x01

// Параметры прокси-сервера
#define PROXY_HTTP_METHODS "OPTIONS|GET|HEAD|POST|PUT|PATCH|DELETE|TRACE|CONNECT"

// Устанавливаем область видимости
using namespace std;

/**
* Anyks Пространство имен основных функций
*/
namespace Anyks { 
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
	/**
	* getUid Функция вывода идентификатора пользователя
	* @param  name имя пользователя
	* @return      полученный идентификатор пользователя
	*/
	uid_t getUid(const char * name);
	/**
	* getGid Функция вывода идентификатора группы пользователя
	* @param  name название группы пользователя
	* @return      полученный идентификатор группы пользователя
	*/
	gid_t getGid(const char * name);
	/**
	* setOwner Функция установки владельца на каталог
	* @param path  путь к файлу или каталогу для установки владельца
	* @param user  данные пользователя
	* @param group идентификатор группы
	*/
	void setOwner(const char * path, const string user, const string group);
	/**
	* mkDir Метод рекурсивного создания каталогов
	* @param path адрес каталогов
	*/
	void mkDir(const char * path);
	/**
	* mDir Метод удаления каталога и всего содержимого
	* @param  path путь до каталога
	* @return      количество дочерних элементов
	*/
	const int rmDir(const char * path);
	/**
	* makePath Функция создания каталога для хранения логов
	* @param  path  адрес для каталога
	* @param  user  данные пользователя
	* @param  group идентификатор группы
	* @return       результат создания каталога
	*/
	bool makePath(const char * path, const string user, const string group);
	/**
	* isDirExist Функция проверки существования каталога
	* @param  path адрес каталога
	* @return      результат проверки
	*/
	bool isDirExist(const char * path);
	/**
	* isFileExist Функция проверки существования файла
	* @param  path адрес каталога
	* @return      результат проверки
	*/
	bool isFileExist(const char * path);
	/**
	* addToPath Метод формирования адреса из пути и названия файла
	* @param  path путь где хранится файл
	* @param  file название файла
	* @return      сформированный путь
	*/
	const string addToPath(const string path, const string file);
	/**
	* strToTime Метод перевода строки в timestamp
	* @param  date строка даты
	* @return      timestamp
	*/
	const time_t strToTime(const char * date);
	/**
	* timeToStr Метод преобразования timestamp в строку
	* @param  date дата в timestamp
	* @return      строка содержащая дату
	*/
	const string timeToStr(const time_t date);
	/**
	* getSizeBuffer Функция получения размера буфера в байтах
	* @param  str пропускная способность сети (bps, kbps, Mbps, Gbps)
	* @return     размер буфера в байтах
	*/
	const long getSizeBuffer(const string str);
	/**
	* getBytes Функция получения размера в байтах из строки
	* @param  str строка обозначения размерности
	* @return     размер в байтах
	*/
	const size_t getBytes(const string str);
	/**
	* getSeconds Функция получения размера в секундах из строки
	* @param  str строка обозначения размерности
	* @return     размер в секундах
	*/
	const size_t getSeconds(const string str);
	/**
	* isAddress Метод проверки на то является ли строка адресом
	* @param  address строка адреса для проверки
	* @return         результат проверки
	*/
	const bool isAddress(const string address);
	/**
	* getTypeAmingByString Метод определения типа данных из строки
	* @param  str строка с данными
	* @return     определенный тип данных
	*/
	const u_int getTypeAmingByString(const string str);
	/**
	* checkDomain Метод определения определения соответствия домена маски
	* @param  domain название домена
	* @param  mask   маска домена для проверки
	* @return        результат проверки
	*/
	const bool checkDomainByMask(const string domain, const string mask);
}

#endif // _GENERAL_AMING_