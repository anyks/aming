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
#include <iostream>
#include <sys/stat.h>
#include "../ini/ini.h"

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
#define PROXY_CACHE false
#define PROXY_DEBUG false
#define PROXY_DAEMON false
#define PROXY_IPV 4
#define PROXY_HTTP_PORT 8080
#define PROXY_SOCKS5_PORT 1080
#define PROXY_REDIRECT_PORT 1180
#define PROXY_RESOLVER {"8.8.8.8", "8.8.4.4"}

// Адреса конфигурационных файлов
#define PID_DIR "/var/run"
#define LOGS_DIR "/var/log"
#define CONFIG_DIR "/usr/local/etc"

// Максимальное количество открытых сокетов (по дефолту в системе 1024)
#define MAX_FDS 1024 * 10

// Максимальное количество подключений с одного клиента
#define MAX_CONNECTS 100

// Общее количество одновременных подключений к прокси серверу (-1 = auto)
#define ALL_CONNECTS -1

// Модуль скрытия заголовков
#define RM_HEADERS_REQUEST false
#define RM_HEADERS_RESPONSE false

// Модуль установки заголовков
#define SET_HEADERS_REQUEST false
#define SET_HEADERS_RESPONSE false

// Модуль IPv4
#define IPV4_EXTERNAL "0.0.0.0"
#define IPV4_INTERNAL "127.0.0.1"

// Модуль IPv6
#define IPV6_EXTERNAL "::0"
#define IPV6_INTERNAL "::1"

// Размеры буферов клиента
#define BUFFER_WRITE_SIZE -1
#define BUFFER_READ_SIZE -1

// Таймауты подключения
#define TIMEOUTS_READ 75
#define TIMEOUTS_WRITE 60

// Модуль логов
#define LOGS_ENABLED true
#define LOGS_FILES true
#define LOGS_SIZE 1024

// Модуль авторизаций
#define AUTH_OS_USERS true
#define AUTH_FILE_USERS false
#define AUTH_ENABLED false

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
			int read;		// Буфер на чтение
			int write;		// Буфер на запись
		};
		/**
		 * Ipv4 Подключение по IPv4
		 */
		struct Ipv4 {
			string external;	// Внешний интерфейс, через который будут уходить запросы от сервера
			string internal;	// IP адрес интерфейса на котором будут приниматься запросы от клиентов
		};
		/**
		 * Ipv6 Подключение по IPv6
		 */
		struct Ipv6 {
			string external;	// Внешний интерфейс, через который будут уходить запросы от сервера
			string internal;	// IP адрес интерфейса на котором будут приниматься запросы от клиентов
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
			bool enabled;	// Разрешить ведение логов
			u_int size;		// Размер файла лога в Кб
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
		 * Proxy Параметры самого прокси-сервера
		 */
		struct Proxy {
			int allcon;					// Общее количество одновременных подключений к прокси серверу
			u_int ipver;				// Версия протокола интернета (4, 6)
			u_int type;					// Тип прокси сервера (http = 1, socks5 = 2, redirect = 3)
			u_int port;					// Порт прокси сервера
			u_int maxcon;				// Максимальное количество подключений (одного клиента к прокси серверу)
			u_int maxfds;				// Максимальное количество файловых дескрипторов
			bool debug;					// Активация режима отладки
			bool daemon;				// Запусть в виде демона
			bool reverse;				// Обратный прокси (доступ из сети в локальную сеть)
			bool transfer;				// Активация поддержки прокси листа
			bool forward;				// Прямой прокси (доступ во внешнюю сеть)
			bool deblock;				// Попробовать обойти блокировки сайтов на уровне прокси (многие сайты могут работать не правильно)
			bool optimos;				// Оптимизировать настройки операционной системы (нужен root доступ)
			bool cache;					// Активировать кеширование часто-запрашиваемых страниц
			string user;				// Идентификатор группы пользователя под которым запускается прокси
			string group;				// Идентификатор пользователя под которым запускается прокси
			string name;				// Название сервиса
			string piddir;				// Адрес хранения pid файла
			string confdir;				// Адрес хранения конфигурационных файлов
			vector <string> resolver;	// Массив со списком dns серверов
		};
		// Адрес конфигурационного файла
		string filename;
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
		 * getResolver Метод извлечения dns серверов из строки в массив
		 * @param  str строка с dns серверами
		 * @return     массив с dns серверами
		 */
		vector <string> getResolver(const string str);
		/**
		 * getSizeBuffer Функция получения размера буфера в байтах
		 * @param  speed пропускная способность сети в мегабитах
		 * @return       размер буфера в байтах
		 */
		int getSizeBuffer(float speed = -1);
		/**
		 * isFileExist Функция проверки существования файла
		 * @param  path адрес каталога
		 * @return      результат проверки
		 */
		bool isFileExist(const char * path);
	public:
		// Основные данные приложения
		Ipv6 ipv6;			// Подключение по IPv6
		Ipv4 ipv4;			// Подключение по IPv4
		Logs logs;			// Параметры логов
		Proxy proxy;		// Параметры самого прокси-сервера
		Header rmheader;	// Удалять указанные http заголовки из запроса или ответа
		Header setheader;	// Установить указанные http заголовки в запрос или ответ
		Firewall firewall;	// Параметры файервола
		Timeouts timeouts;	// Таймауты подключений
		BufferSize buffers;	// Размеры буферов передачи данных
		Authorization auth;	// Параметры авторизации
		// Основные параметры прокси
		u_short options;
		/**
		 * Config Конструктор модуля конфигурационного файла
		 * @param filename адрес конфигурационного файла
		 */
		Config(const string filename = "");
};

#endif // _CONFIG_ANYKS_
