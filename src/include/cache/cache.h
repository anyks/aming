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
#include <stdlib.h>
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
					size_t ipv;
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
				u_int ipv;				// Версия интернет протокола
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
			// Версия интернет протокола
			u_int ipv;
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