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
#include "general/general.h"

// Устанавливаем область видимости
using namespace std;

/**
 * Cache Класс управления кешем
 */
class Cache {
	private:
		/**
		 * DataDNS Класс параметров домена кэша
		 */
		class DataDNS {
			private:
				/**
				 * Структура размеров
				 */
				struct Map {
					size_t ttl;
					size_t ipv4;
					size_t ipv6;
				};
				// Сырые данные
				vector <u_char> raw;
			public:
				// Время жизни кэша
				time_t ttl = 0;
				// Размер адреса IPv4
				string ipv4;
				// Размер адреса IPv6
				string ipv6;
				/**
				 * size Метод получения размеров сырых данных
				 * @return размер сырых данных
				 */
				const size_t size();
				/**
				 * data Метод получения сырых данных
				 * @return сырые данные
				 */
				const u_char * data();
				/**
				 * set Метод установки сырых данных
				 * @param data сырые данные
				 * @param size размер сырых данных
				 */
				void set(const u_char * data, size_t size);
				/**
				 * ~DataDNS Деструктор
				 */
				~DataDNS();
		};
		/**
		 * Data Класс параметров кэша данных
		 */
		class DataCache {
			private:
				/**
				 * Структура размеров
				 */
				struct Map {
					size_t age;
					size_t date;
					size_t expires;
					size_t modified;
					size_t valid;
					size_t etag;
					size_t cache;
				};
				// Сырые данные
				vector <u_char> raw;
			public:
				time_t age;				// Время жизни кэша
				time_t date;			// Дата записи кэша прокси сервером
				time_t expires;			// Дата смерти кэша
				time_t modified;		// Дата последней модификации
				bool valid;				// Обязательная ревалидация
				string etag;			// Идентификатор ETag
				vector <u_char> http;	// Данные кэша
				/**
				 * size Метод получения размеров сырых данных
				 * @return размер сырых данных
				 */
				const size_t size();
				/**
				 * data Метод получения сырых данных
				 * @return сырые данные
				 */
				const u_char * data();
				/**
				 * set Метод установки сырых данных
				 * @param data сырые данные
				 * @param size размер сырых данных
				 */
				void set(const u_char * data, size_t size);
				/**
				 * ~DataCache Деструктор
				 */
				~DataCache();
		};
		/**
		 * ResultData  Структура с данными полученными из файла кэша
		 */
		struct ResultData {
			// Обязательная ревалидация
			bool valid;
			// Возраст кэша
			time_t age;
			// Etag кэша
			string etag;
			// Дата модификации кэша
			string modified;
			// Данные кэша
			vector <u_char> http;
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
		 * @param http блок с данными запроса или ответа
		 * @param data данные запроса
		 */
		void readCache(HttpData &http, DataCache * data);
		/**
		 * writeDomain Метод записи данных домена в файл
		 * @param domain название домена
		 * @param data   данные домена
		 */
		void writeDomain(const string domain, DataDNS data);
		/**
		 * writeCache Метод записи данных кэша
		 * @param http блок с данными запроса или ответа
		 * @param data данные запроса
		 */
		void writeCache(HttpData &http, DataCache data);
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
		const bool checkEnabledCache(HttpData &http);
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
		ResultData getCache(HttpData &http);
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
		void setCache(HttpData &http);
		/**
		 * rmCache Метод удаления кэша
		 * @param http блок с данными запроса или ответа
		 */
		void rmCache(HttpData &http);
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