/* МОДУЛЬ CONFIG ДЛЯ ПРОКСИ СЕРВЕРА ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _CONFIG_ANYKS_
#define _CONFIG_ANYKS_

#include <string>
#include <iostream>
#include "../ini/ini.h"

// Название и версия прокси-сервера
#define APP_NAME "ANYKS"
#define APP_VERSION "1.0"
#define APP_COPYRIGHT "ANYKS LLC"
#define APP_SITE "http://anyks.com"
#define APP_EMAIL "info@anyks.com"
#define APP_SUPPORT "support@anyks.com"
#define APP_AUTHOR "forman"

// Основные флаги прокси сервера
#define OPT_NULL 0x00		// Флаг устанавливающий пустые данные NULL
#define OPT_CONNECT 0x01	// Флаг разрешения connect прокси сервера
#define OPT_AGENT 0x02		// Флаг вывода названия прокси сервера
#define OPT_GZIP 0x04		// Флаг разрешения сжатия данных методом gzip
#define OPT_SMART 0x08		// Флаг разрешающий smart прокси сервер
#define OPT_KEEPALIVE 0x10	// Флаг разрешающий постоянное соединение
#define OPT_LOG 0x20		// Флаг разрешающий логирование данных
#define OPT_PGZIP 0x40		// Флаг разрешающий сжатие не сжатых данных

// Параметры прокси сервера (http, socks5, redirect)
#define PROXY_TYPE "http"
#define PROXY_USER "nobody"
#define PROXY_GROUP "nobody"
#define PROXY_NAME "anyks"
#define PROXY_TRANSFER false
#define PROXY_REVERSE false
#define PROXY_FORWARD true
#define PROXY_DEDLOCK false
#define PROXY_OPTIMOS false
#define PROXY_BANDLIMIN false
#define PROXY_CACHE false
#define PROXY_DEBUG false
#define PROXY_DAEMON false
#define PROXY_IPV 4
#define PROXY_HTTP_PORT 8080
#define PROXY_SOCKS5_PORT 1080
#define PROXY_REDIRECT_PORT 1180

// Адреса конфигурационных файлов
#define PID_DIR "/var/run"
#define LOGS_DIR "/var/log"
#define CONFIG_DIR "/usr/local/etc"

// Максимальное количество открытых сокетов (по дефолту в системе 1024)
#define MAX_FDS 1024 * 10

// Максимальное количество подключений с одного клиента
#define MAX_CONNECTS 100

// Модуль разрешенных списков файлов
#define SITES_BLACK_LIST false
#define SITES_WHITE_LIST false

// Модуль IPv4
#define IPV4_EXTERNAL "0.0.0.0"
#define IPV4_INTERNAL "127.0.0.1"

// Модуль IPv6
#define IPV6_EXTERNAL "::0"
#define IPV6_INTERNAL "::1"

// Модуль логов
#define LOGS_ENABLED true
#define LOGS_FILES true

// Модуль авторизаций
#define AUTH_OS_USERS true
#define AUTH_FILE_USERS false
#define AUTH_NO false

// Модуль блокировок
#define BLOCK_MAX_TRYAUTH 10
#define BLOCK_AUTH false
#define BLOCK_LOOP true
#define BLOCK_TIME_LOOP "60s"

// Устанавливаем область видимости
using namespace std;

/**
 * Config Класс конфигурационных данных прокси-сервера
 */
class Config {
	private:
		/**
		 * Ipv4 Подключение по IPv4
		 */
		struct Ipv4 {
			string external;	// Внешний интерфейс, через который будут уходить запросы от сервера
			string internal;	// IP адрес интерфейса на котором будут приниматься запросы от клиентов
		} __attribute__((packed));
		/**
		 * Ipv6 Подключение по IPv6
		 */
		struct Ipv6 {
			string external;	// Внешний интерфейс, через который будут уходить запросы от сервера
			string internal;	// IP адрес интерфейса на котором будут приниматься запросы от клиентов
		} __attribute__((packed));
		/**
		 * Logs Параметры логов
		 */
		struct Logs {
			bool files;		// Разрешить хранить логи в файлах
			bool enabled;	// Разрешить ведение логов
		} __attribute__((packed));
		/**
		 * Authorization Параметры авторизации
		 */
		struct Authorization {
			bool osusers;	// Активировать авторизацию через пользователей в операционной системе
			bool listusers;	// Активировать авторизацию через файл со списком пользователей и ip/mac адресами
			bool noauth;	// Без авторизации
		} __attribute__((packed));
		/**
		 * Listsites Списки сайтов
		 */
		struct Listsites {
			bool blacklist;		// Активировать черный список сайтов или портов
			bool whitelist;		// Активировать белый список сайтов или портов
		} __attribute__((packed));
		/**
		 * Bloking Блокировка плохих запросов
		 */
		struct Bloking {
			u_int maxtryauth;		// 10 неудачных попыток авторизации
			bool blockauth;			// Разрешить блокирование пользователя после неудачных попыток авторизации
			bool blockloop;			// Разрешить блокирование зацикливающих запросов
			string timeblockloop;	// Время блокирования зацикливающих запросов (s - секунды, m - минуты, h - часы, d - дни, M - месяцы, y - годы)
		} __attribute__((packed));
		/**
		 * Proxy Параметры самого прокси-сервера
		 */
		struct Proxy {
			u_int ipver;	// Версия протокола интернета (4, 6)
			u_int type;		// Тип прокси сервера (http = 1, socks5 = 2, redirect = 3)
			u_int port;		// Порт прокси сервера
			u_int maxcon;	// Максимальное количество подключений (одного клиента к прокси серверу)
			u_int maxfds;	// Максимальное количество файловых дескрипторов
			bool debug;		// Активация режима отладки
			bool daemon;	// Запусть в виде демона
			bool reverse;	// Обратный прокси (доступ из сети в локальную сеть)
			bool transfer;	// Активация поддержки прокси листа
			bool forward;	// Прямой прокси (доступ во внешнюю сеть)
			bool deblock;	// Попробовать обойти блокировки сайтов на уровне прокси (многие сайты могут работать не правильно)
			bool optimos;	// Оптимизировать настройки операционной системы (нужен root доступ)
			bool bandlimin;	// Активировать ограничение трафика пользователей
			bool cache;		// Активировать кеширование часто-запрашиваемых страниц
			string user;	// Идентификатор группы пользователя под которым запускается прокси
			string group;	// Идентификатор пользователя под которым запускается прокси
			string name;	// Название сервиса
			string piddir;	// Адрес хранения pid файла
			string confdir;	// Адрес хранения конфигурационных файлов
		} __attribute__((packed));
		// Основные данные приложения
		Ipv6 ipv6;						// Подключение по IPv6
		Ipv4 ipv4;						// Подключение по IPv4
		Logs logs;						// Параметры логов
		Proxy proxy;					// Параметры самого прокси-сервера
		Bloking bloking;				// Блокировка плохих запросов
		Listsites listsites;			// Списки сайтов
		Authorization authorization;	// Параметры авторизации
		// Основные параметры прокси
		u_short options;
		// Адрес конфигурационного файла
		string filename;
	public:
		/**
		 * Config Конструктор модуля конфигурационного файла
		 * @param filename адрес конфигурационного файла
		 */
		Config(const string & filename);
};

#endif // _CONFIG_ANYKS_
