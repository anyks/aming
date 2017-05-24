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
#include <vector>
#include <regex>
#include <iostream>
#include <math.h>
#include <zlib.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "ini/ini.h"
#include "general/general.h"

// Данные приложения
#define APP_NAME "ANYKS"
#define APP_VERSION "1.0"
#define APP_YEAR "2017"
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
#define OPT_DEBLOCK 0x80	// Флаг разрешающий обход блокировок

// Параметры прокси сервера (http, socks5, redirect)
#define PROXY_TYPE "http"
#define PROXY_USER "nobody"
#define PROXY_GROUP "nogroup"
#define PROXY_NAME "anyks"
#define PROXY_TRANSFER false
#define PROXY_REVERSE false
#define PROXY_FORWARD true
#define PROXY_OPTIMOS false
#define PROXY_DEBUG false
#define PROXY_DAEMON false
#define PROXY_IPV6ONLY true
#define PROXY_SUBNET false
#define PROXY_IPV "4 -> 4"
#define PROXY_HTTP_PORT 8080
#define PROXY_SOCKS5_PORT 1080
#define PROXY_REDIRECT_PORT 1180

// Параметры модуля кеширования
#define CACHE_DNS true
#define CACHE_RESPONSE false
#define CACHE_DTTL "1d"
#define CACHE_DIR "/var/cache"

// Адреса конфигурационных файлов
#define PID_DIR "/var/run"

// Адрес конфигурационного файла
#ifdef __linux__
	#define CONFIG_DIR "/etc"
#else
	#define CONFIG_DIR "/usr/local/etc"
#endif

// Максимальное количество воркеров (0 - auto)
#define MAX_WORKERS 0
// Максимальное количество воркеров которое разрешено создать
#define MMAX_WORKERS 300

// Модуль контроля подключений клиента к серверу
#define CONNECTS_KEY "ip"
#define CONNECTS_TOTAL 100
#define CONNECTS_CONNECT 100
#define CONNECTS_FDS 1024 * 10
#define CONNECTS_SIZE "5MB"

// Модуль скрытия заголовков
#define RM_HEADERS_REQUEST false
#define RM_HEADERS_RESPONSE false

// Модуль установки заголовков
#define SET_HEADERS_REQUEST false
#define SET_HEADERS_RESPONSE false

// Модуль IPv4
#define IPV4_EXTERNAL "0.0.0.0"
#define IPV4_INTERNAL "127.0.0.1"
#define IPV4_RESOLVER {"8.8.8.8", "8.8.4.4"}

// Модуль IPv6
#define IPV6_EXTERNAL "::0"
#define IPV6_INTERNAL "::1"
#define IPV6_RESOLVER {"2001:4860:4860::8888", "2001:4860:4860::8844"}

// Размеры буферов клиента
#define BUFFER_WRITE_SIZE "auto"
#define BUFFER_READ_SIZE "auto"

// Таймауты подключения
#define TIMEOUTS_READ 5
#define TIMEOUTS_WRITE 15

// Модуль логов
#define LOGS_ENABLED false
#define LOGS_FILES true
#define LOGS_CONSOLE true
#define LOGS_DATA false
#define LOGS_SIZE "1MB"
#define LOGS_DIR "/var/log"

// Модуль постоянного подключения
#define KEEPALIVE_CNT 6
#define KEEPALIVE_IDLE 3
#define KEEPALIVE_INTVL 2

// Модуль авторизаций
#define AUTH_OS_USERS true
#define AUTH_FILE_USERS false
#define AUTH_ENABLED false

// Модуль GZIP
#define GZIP_VARY false
#define GZIP_LENGTH 100
#define GZIP_CHUNK "4KB"
#define GZIP_REGEX "msie6"
#define GZIP_VHTTP {"1.0", "1.1"}
#define GZIP_PROXIED {"private", "no-cache", "no-store"}
#define GZIP_TYPES {"text/html", "text/css", "text/plain", "text/xml", "text/javascript", "text/csv"}

// Модуль блокировок
#define FIREWALL_MAX_TRYAUTH 10
#define FIREWALL_AUTH false
#define FIREWALL_LOOP true
#define FIREWALL_BLACK_LIST false
#define FIREWALL_WHITE_LIST false
#define FIREWALL_BANDLIMIN false
#define FIREWALL_TIME_LOOP "60s"

// Устанавливаем область видимости
using namespace std;

/**
 * Config Класс конфигурационных данных прокси-сервера
 */
class Config {
	private:
		/**
		 * Gzip Параметры сжатия данных на уровне прокси сервера
		 */
		struct Gzip {
			bool vary;					// Разрешает или запрещает выдавать в ответе поле заголовка “Vary: Accept-Encoding”
			int level;					// Тип сжатия (default - по умолчанию, best - лучшее сжатие, speed - лучшая скорость, no - без сжатия)
			long length;				// Минимальная длина данных после которых включается сжатие (работает только с Content-Length)
			size_t chunk;				// Максимальный размер чанка в байтах
			string regex;				// Не сжимать контент, UserAgent которого соответсвует регулярному выражению
			vector <string> vhttp;		// Версия http протокола
			vector <string> proxied;	// Разрешает или запрещает сжатие ответа методом gzip для проксированных запросов
			vector <string> types;		// Разрешает сжатие ответа методом gzip для указанных MIME-типов
		};
		/**
		 * Timeout Структура таймаутов
		 */
		struct Timeouts {
			u_short read;	// Таймаут времени на чтение
			u_short write;	// Таймаут времени на запись
		};
		/**
		 * BufferSize Структура размеров буфера
		 */
		struct BufferSize {
			long read;		// Буфер на чтение
			long write;		// Буфер на запись
		};
		/**
		 * Ipv4 Подключение по IPv4
		 */
		struct Ipv4 {
			string external;			// Внешний интерфейс, через который будут уходить запросы от сервера
			string internal;			// IP адрес интерфейса на котором будут приниматься запросы от клиентов
			vector <string> resolver;	// Массив со списком dns серверов
		};
		/**
		 * Ipv6 Подключение по IPv6
		 */
		struct Ipv6 {
			string external;			// Внешний интерфейс, через который будут уходить запросы от сервера
			string internal;			// IP адрес интерфейса на котором будут приниматься запросы от клиентов
			vector <string> resolver;	// Массив со списком dns серверов
		};
		/**
		 * Keepalive Параметры постоянного подключения
		 */
		struct Keepalive {
			int keepcnt;	// Максимальное количество попыток
			int keepidle;	// Интервал времени в секундах через которое происходит проверка подключения
			int keepintvl;	// Интервал времени в секундах между попытками
		};
		/**
		 * Header http заголовки из запроса или ответа
		 */
		struct Header {
			bool request;	// Убирать заголовки в запросе
			bool response;	// Убирать заголовки в ответе
		};
		/**
		 * Logs Параметры логов
		 */
		struct Logs {
			bool files;		// Разрешить хранить логи в файлах
			bool console;	// Разрешить вывод логов в консоль
			bool data;		// Разрешить ведение логов данных для обмена
			bool enabled;	// Разрешить ведение логов
			size_t size;	// Размер файла лога в Кб
			string dir;		// Адрес каталога для хранения логов
		};
		/**
		 * Authorization Параметры авторизации
		 */
		struct Authorization {
			bool osusers;	// Активировать авторизацию через пользователей в операционной системе
			bool listusers;	// Активировать авторизацию через файл со списком пользователей и ip/mac адресами
			bool enabled;	// Разрешить авторизацию пользователя
		};
		/**
		 * Firewall Блокировка плохих запросов
		 */
		struct Firewall {
			u_int maxtryauth;		// 10 неудачных попыток авторизации
			bool blockauth;			// Разрешить блокирование пользователя после неудачных попыток авторизации
			bool blockloop;			// Разрешить блокирование зацикливающих запросов
			bool blacklist;			// Активировать черный список сайтов или портов
			bool whitelist;			// Активировать белый список сайтов или портов
			bool bandlimin;			// Активировать ограничение трафика пользователей
			string timeblockloop;	// Время блокирования зацикливающих запросов (s - секунды, m - минуты, h - часы, d - дни, M - месяцы, y - годы)
		};
		/**
		 * Os Структура параметров операционной системы
		 */
		struct Os {
			u_int ncpu;		// Количество cpu
			string cpu;		// Название cpu
			string name;	// Название os
		};
		/**
		 * Cache Структура настроек модуля кеширования
		 */
		struct Cache {
			bool dns;		// Кеширование dns запросов
			bool dat;		// Кеширование часто-запрашиваемых страниц
			time_t dttl;	// Время жизни dns кэша в секундах
			string dir;		// Каталог хранения кеш файлов
		};
		/**
		 * Connects Контроль подключений клиента к серверу
		 */
		struct Connects {
			u_int key;		// Ключ по которому определяются подключения (ip = 0, mac = 1)
			u_int fds;		// Максимальное количество файловых дескрипторов
			u_int connect;	// Максимальное количество подключений (одного клиента к прокси серверу)
			int total;		// Общее количество одновременных подключений к прокси серверу
			size_t size;	// Максимальный размер скачиваемых данных в байтах
		};
		/**
		 * Proxy Параметры самого прокси-сервера
		 */
		struct Proxy {
			u_int intIPv;	// Версия протокола интернета внутреннего (4, 6)
			u_int extIPv;	// Версия протокола интернета внешнего (4, 6)
			u_int type;		// Тип прокси сервера (http = 1, socks5 = 2, redirect = 3)
			u_int port;		// Порт прокси сервера
			u_int workers;	// Количество воркеров
			bool debug;		// Активация режима отладки
			bool daemon;	// Запусть в виде демона
			bool reverse;	// Обратный прокси (доступ из сети в локальную сеть)
			bool transfer;	// Активация поддержки прокси листа
			bool forward;	// Прямой прокси (доступ во внешнюю сеть)
			bool ipv6only;	// Активация режима отображения IPv4 в IPv6
			bool subnet;	// Активация режима мульти-сетевого взаимодействия когда выход на сеть определяется по ip адресу и в зависимости от него выбирается ipv6 или ipv4 сеть
			bool optimos;	// Оптимизировать настройки операционной системы (нужен root доступ)
			string user;	// Идентификатор группы пользователя под которым запускается прокси
			string group;	// Идентификатор пользователя под которым запускается прокси
			string name;	// Название сервиса
			string piddir;	// Адрес хранения pid файла
			string dir;		// Адрес хранения конфигурационных файлов
		};
		// Адрес конфигурационного файла
		string filename;
		/**
		 * getSizeBuffer Функция получения размера буфера в байтах
		 * @param  str пропускная способность сети (bps, kbps, Mbps, Gbps)
		 * @return     размер буфера в байтах
		 */
		long getSizeBuffer(const string str);
		/**
		 * getBytes Функция получения размера в байтах из строки
		 * @param  str строка обозначения размерности
		 * @return     размер в байтах
		 */
		size_t getBytes(const string str);
		/**
		 * getSeconds Функция получения размера в секундах из строки
		 * @param  str строка обозначения размерности
		 * @return     размер в секундах
		 */
		size_t getSeconds(const string str);
	public:
		// Основные данные приложения
		Os os;					// Данные операционной системы
		Ipv6 ipv6;				// Подключение по IPv6
		Ipv4 ipv4;				// Подключение по IPv4
		Logs logs;				// Параметры логов
		Gzip gzip;				// Параметры gzip
		Cache cache;			// Параметры кеширования
		Proxy proxy;			// Параметры самого прокси-сервера
		Header rmheader;		// Удалять указанные http заголовки из запроса или ответа
		Header setheader;		// Установить указанные http заголовки в запрос или ответ
		Connects connects;		// Контроль подключений клиента к серверу
		Firewall firewall;		// Параметры файервола
		Timeouts timeouts;		// Таймауты подключений
		BufferSize buffers;		// Размеры буферов передачи данных
		Authorization auth;		// Параметры авторизации
		Keepalive keepalive;	// Постоянное подключение
		// Основные параметры прокси
		u_short options;
		/**
		 * Config Конструктор модуля конфигурационного файла
		 * @param filename адрес конфигурационного файла
		 */
		Config(const string filename = "");
};

#endif // _CONFIG_ANYKS_
