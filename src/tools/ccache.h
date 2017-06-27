/* УТИЛИТА ЧИСТКИ УСТАРЕВШИХ ФЙЛОВ КЭША AMING */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
/*
* Example crontab:
* # crontab -e
* 47 6 * * 7 root /root/proxy/src/bin/ptcc -c /etc/aming/config.ini >> /var/log/ptcc.log 2>&1
*/
#ifndef _CACHE_CLEANER_AMING_
#define _CACHE_CLEANER_AMING_

#include <map>
#include <regex>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../include/config/conf.h"
#include "../include/general/general.h"

// Устанавливаем область видимости
using namespace std;

/**
 * CCache Класс для работы с устаревшим кэшем
 */
class CCache {
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
		// Объект конфигурационных данных
		Config * config = nullptr;
		/**
		 * handler Прототип колбека
		 */
		typedef void (* handler) (const string filename, void * ctx);
		/**
		* readDomain Метод чтения данных домена из файла
		* @param filename адрес файла кэша
		* @param data     указатель на данные домена
		*/
		void readDomain(const string filename, DataDNS * data);
		/**
		 * readCache Метод чтения данных из файла кэша
		 * @param filename адрес файла кэша
		 * @param data     данные запроса
		 */
		void readCache(const string filename, DataCache * data);
		/**
		 * checkDomains Функция проверки кэша доменов
		 * @param filename адрес файла записи
		 * @param ctx      объект приложения
		 */
		static void checkDomains(const string filename, void * ctx);
		/**
		 * checkCache Функция проверки кэша данных
		 * @param filename адрес файла записи
		 * @param ctx      объект приложения
		 */
		static void checkCache(const string filename, void * ctx);
		/**
		 * infoPatch Функция определяющая тип адреса
		 * @param  pathName  адрес файла или каталога
		 * @return           результат проверки (-1 - Не определено, 0 - Файл, 1 - Каталог)
		 */
		const short infoPatch(const string pathName);
		/**
		 * processDirectory Рекурсивная функция обхода содержимого каталога
		 * @param  curDir адрес текущего каталога
		 * @param  fn     функция обработчик содержимого файла
		 * @return        результат работы
		 */
		const u_long processDirectory(const string curDir, handler fn);
	public:
		/**
		 * CCache Конструктор
		 * @param config адрес конфигурационного файла
		 */
		CCache(const string config);
		/**
		 * ~CCache Деструктор
		 */
		~CCache();
};

#endif // _CACHE_CLEANER_AMING_