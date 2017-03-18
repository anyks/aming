/* МОДУЛЬ УПРАВЛЕНИЯ КЭШЕМ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _CACHE_ANYKS_
#define _CACHE_ANYKS_

#include <map>
#include <regex>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/file.h>
#include "md5/md5.h"
#include "config/conf.h"
#include "nwk/nwk.h"
#include "log/log.h"
#include "http/http.h"

// Устанавливаем область видимости
using namespace std;

/**
 * Cache Класс управления кешем
 */
class Cache {
	private:
		/**
		 * DataDNS Структура параметров домена кэша
		 */
		struct DataDNS {
			time_t ttl;
			string ipv4;
			string ipv6;
		};
		/**
		 * Data Структура параметров кэша данных
		 */
		struct DataCache {
			time_t age;				// Время жизни кэша
			time_t date;			// Дата записи кэша прокси сервером
			time_t expires;			// Дата смерти кэша
			time_t modified;		// Дата последней модификации
			bool rvalid;			// Обязательная ревалидация (в случае установки такого заголовка, необходимо всегда обновлять контент как только время жизни истекло и игнорируя остальные правила)
			string etag;			// Идентификатор ETag
			HttpData::Dump http;	// Данные
		};
		/**
		 * ResultData  Структура с данными полученными из файла кэша
		 */
		struct ResultData {
			bool load;				// Данные получены
			string etag;			// Etag кэша
			string modified;		// Дата модификации кэша
			HttpData::Dump http;	// Сами кэш данные
		};
		// Объект лога
		LogApp * log = NULL;
		// Конфигурационные данные
		Config * config = NULL;
		/**
		 * getPathDomain Метод создания пути из доменного имени
		 * @param  domain название домена
		 * @return        путь к файлу кэша
		 */
		const string getPathDomain(const string domain);
		/**
		 * timeToStr Метод преобразования timestamp в строку
		 * @param  date дата в timestamp
		 * @return      строка содержащая дату
		 */
		const string timeToStr(const time_t date);
		/**
		 * toCase Функция перевода в указанный регистр
		 * @param  str  строка для перевода в указанных регистр
		 * @param  flag флаг указания типа регистра
		 * @return      результирующая строка
		 */
		const string toCase(string str, bool flag = false);
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
		 * split Метод разбива строки на составляющие
		 * @param  str   исходная строка
		 * @param  delim разделитель
		 * @return       массив составляющих строки
		 */
		vector <string> split(const string str, const string delim);
		/**
		 * addToPath Метод формирования адреса из пути и названия файла
		 * @param  path путь где хранится файл
		 * @param  file название файла
		 * @return      сформированный путь
		 */
		const string addToPath(const string path, const string file);
		/**
		 * getUid Функция вывода идентификатора пользователя
		 * @param  name имя пользователя
		 * @return      полученный идентификатор пользователя
		 */
		const uid_t getUid(const char * name);
		/**
		 * getGid Функция вывода идентификатора группы пользователя
		 * @param  name название группы пользователя
		 * @return      полученный идентификатор группы пользователя
		 */
		const gid_t getGid(const char * name);
		/**
		 * setOwner Функция установки владельца на каталог
		 * @param path путь к файлу или каталогу для установки владельца
		 */
		void setOwner(const char * path);
		/**
		 * readDomain Метод чтения данных домена из файла
		 * @param domain название домена
		 * @param data   указатель на данные домена
		 */
		void readDomain(const string domain, DataDNS * data);
		/**
		 * readCache Метод чтения данных из файла кэша
		 * @param domain название домена
		 * @param name   название запроса
		 * @param data   данные запроса
		 */
		void readCache(const string domain, const string name, DataCache * data);
		/**
		 * writeDomain Метод записи данных домена в файл
		 * @param domain название домена
		 * @param data   данные домена
		 */
		void writeDomain(const string domain, DataDNS data);
		/**
		 * writeCache Метод записи данных кэша
		 * @param domain название домена
		 * @param name   название запроса
		 * @param data   данные запроса
		 */
		void writeCache(const string domain, const string name, DataCache data);
		/**
		 * mkdir Метод рекурсивного создания каталогов
		 * @param path адрес каталогов
		 */
		void mkdir(const char * path);
		/**
		 * rmdir Метод удаления каталога и всего содержимого
		 * @param path путь до каталога
		 */
		const int rmdir(const char * path);
		/**
		 * strToTime Метод перевода строки в timestamp
		 * @param  date строка даты
		 * @return      timestamp
		 */
		const time_t strToTime(const char * date);
		/**
		 * makePath Функция создания каталога для хранения логов
		 * @param  path адрес для каталога
		 * @return      результат создания каталога
		 */
		const bool makePath(const char * path);
		/**
		 * is_number Функция проверки является ли строка числом
		 * @param  str строка для проверки
		 * @return     результат проверки
		 */
		const bool isNumber(const string &str);
		/**
		 * isDirExist Функция проверки существования каталога
		 * @param  path адрес каталога
		 * @return      результат проверки
		 */
		const bool isDirExist(const char * path);
		/**
		 * isFileExist Функция проверки существования файла
		 * @param  path адрес каталога
		 * @return      результат проверки
		 */
		const bool isFileExist(const char * path);
		/**
		 * isDomain Метод проверки на доменное имя
		 * @param  domain строка названия домена для проверки
		 * @return        результат проверки
		 */
		const bool isDomain(const string domain);
		/**
		 * isIpV4 Метод проверки на ip адрес, интернет протокола версии 4
		 * @param  ip строка ip адреса для проверки
		 * @return    результат проверки
		 */
		const bool isIpV4(const string ip);
		/**
		 * isIpV6 Метод проверки на ip адрес, интернет протокола версии 6
		 * @param  ip строка ip адреса для проверки
		 * @return    результат проверки
		 */
		const bool isIpV6(const string ip);
		/**
		 * checkEnabledCache Метод проверки, разрешено ли создавать кэш
		 * @param  http блок с данными запроса или ответа
		 * @return      результат проверки
		 */
		const bool checkEnabledCache(HttpData & http);
	public:
		/**
		 * getDomain Метод получения ip адреса домена
		 * @param  domain название домена
		 * @return        ip адрес домена
		 */
		const string getDomain(const string domain);
		/**
		 * getCache Метод получения данных кэша
		 * @param  http блок с данными запроса или ответа
		 * @return      объект с данными кэша
		 */
		ResultData getCache(HttpData & http);
		/**
		 * setDomain Метод записи домена в кэш
		 * @param  domain название домена
		 * @param  ip     ip адрес домена
		 */
		void setDomain(const string domain, const string ip);
		/**
		 * rmDomain Метод удаления домена из кэша
		 * @param domain название домена
		 */
		void rmDomain(const string domain);
		/**
		 * rmAllDomains Метод удаления всех доменов из кэша
		 */
		void rmAllDomains();
		/**
		 * setCache Метод сохранения кэша
		 * @param http блок с данными запроса или ответа
		 */
		void setCache(HttpData & http);
		/**
		 * rmCache Метод удаления кэша
		 * @param http блок с данными запроса или ответа
		 */
		void rmCache(HttpData & http);
		/**
		 * rmAllCache Метод удаления кэша всех сайтов
		 */
		void rmAllCache();
		/**
		 * Cache Конструктор
		 * @param log    объект лога для вывода информации
		 * @param config конфигурационные данные
		 */
		Cache(LogApp * log = NULL, Config * config = NULL);
};

#endif // _CACHE_ANYKS_