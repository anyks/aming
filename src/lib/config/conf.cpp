/* МОДУЛЬ CONFIG ДЛЯ ПРОКСИ СЕРВЕРА AMING */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#include "config/conf.h"

// Устанавливаем область видимости
using namespace std;

/**
 * Config Конструктор модуля конфигурационного файла
 * @param filename адрес конфигурационного файла
 */
Config::Config(const string filename){
	// Запоминаем адрес файла с конфигурационным файлом
	this->filename = filename;
	// Если каталог с конфигурационным файлом не найден
	if(this->filename.empty() || !Anyks::isFileExist(this->filename.c_str())){
		// Запоминаем адрес конфигурационного файла MacOS X и FreeBSD
		this->filename = (string(CONFIG_DIR) + string("/") + string(PROXY_NAME) + string("/config.ini"));
	}
	// Инициализируем парсер ini файла
	INI ini(this->filename);
	// Если во время чтения файла возникли ошибки
	if(ini.isError()){
		// Выводим сообщение об ошибке
		printf("Can't load config file %s\n", this->filename.c_str());
		// Формируем опции по умолчанию
		this->options = (OPT_CONNECT | OPT_AGENT | OPT_GZIP | OPT_KEEPALIVE | OPT_LOG | OPT_PGZIP);
		// Получаем тип прокси сервера
		string type = PROXY_TYPE;
		// Тип ключа определения коннектов к прокси
		u_int connect_key = (string(CONNECTS_KEY).compare("mac") == 0 ? 1 : 0);
		// Тип прокси сервера
		u_int proxy_type, proxy_port;
		// Массив протоколов
		auto ipVx = Anyks::split(PROXY_IPV, "->");
		// Запоминаем внешнюю версию интернет протокола
		const u_int proxy_intIPv = ::atoi(ipVx[0].c_str());
		// Запоминаем внутреннюю версию интернет протокола
		const u_int proxy_extIPv = ::atoi(ipVx[1].c_str());
		// Если прокси сервер это http
		if(type.compare("http") == 0) proxy_type = 1;
		else if(type.compare("socks5") == 0) proxy_type = 2;
		else if(type.compare("redirect") == 0) proxy_type = 3;
		else proxy_type = 1;
		// Определяем порт прокси сервера
		switch(proxy_type){
			// Порт http прокси
			case 1: proxy_port = PROXY_HTTP_PORT;		break;
			// Порт socks5 прокси
			case 2: proxy_port = PROXY_SOCKS5_PORT;		break;
			// Порт redirect прокси
			case 3: proxy_port = PROXY_REDIRECT_PORT;	break;
		}
		// Заполняем структуру proxy
		this->proxy = {
			// Версия внутреннего протокола интернета (4, 6)
			proxy_intIPv,
			// Версия внешнего протокола интернета (4, 6)
			proxy_extIPv,
			// Тип прокси сервера (http, socks5, redirect)
			proxy_type,
			// Устанавливаем порт прокси сервера
			proxy_port,
			// Максимальное количество воркеров
			MAX_WORKERS,
			// Активация режима отладки
			PROXY_DEBUG,
			// Запусть в виде демона
			PROXY_DAEMON,
			// Обратный прокси (доступ из сети в локальную сеть)
			PROXY_REVERSE,
			// Активация поддержки прокси листа
			PROXY_TRANSFER,
			// Прямой прокси (доступ во внешнюю сеть)
			PROXY_FORWARD,
			// Активация режима отображения IPv4 в IPv6
			PROXY_IPV6ONLY,
			// Активация режима мульти-сетевого взаимодействия
			PROXY_SUBNET,
			// Оптимизировать настройки операционной системы (нужен root доступ)
			PROXY_OPTIMOS,
			// Активация конвеерной обработки
			PROXY_PIPELINING,
			// Идентификатор группы пользователя под которым запускается прокси
			PROXY_USER,
			// Идентификатор пользователя под которым запускается прокси
			PROXY_GROUP,
			// Название сервиса
			PROXY_NAME,
			// Адрес хранения pid файла
			PID_DIR,
			// Адрес хранения конфигурационных файлов
			CONFIG_DIR
		};
		// Заполняем структуру cache
		this->cache = {
			// Кэширование dns запросов
			CACHE_DNS,
			// Кэширование часто-запрашиваемых страниц
			CACHE_RESPONSE,
			// Время жизни dns кэша в секундах
			(time_t) Anyks::getSeconds(CACHE_DTTL),
			// Каталог хранения кэш файлов
			CACHE_DIR
		};
		// Заполняем структуру gzip
		this->gzip = {
			// Разрешает или запрещает выдавать в ответе поле заголовка “Vary: Accept-Encoding”
			GZIP_VARY,
			// Тип сжатия (default - по умолчанию, best - лучшее сжатие, speed - лучшая скорость, no - без сжатия)
			Z_DEFAULT_COMPRESSION,
			// Минимальная длина данных после которых включается сжатие (работает только с Content-Length)
			GZIP_LENGTH,
			// Максимальный размер чанка в байтах
			Anyks::getBytes(GZIP_CHUNK),
			// Не сжимать контент, UserAgent которого соответсвует регулярному выражению
			GZIP_REGEX,
			// Версия http протокола
			GZIP_VHTTP,
			// Разрешает или запрещает сжатие ответа методом gzip для проксированных запросов
			GZIP_PROXIED,
			// Разрешает сжатие ответа методом gzip для указанных MIME-типов
			GZIP_TYPES
		};
		// Заполняем структуру firewall
		this->firewall = {
			// Количество неудачных попыток авторизации
			FIREWALL_MAX_TRYAUTH,
			// Разрешить блокирование пользователя после неудачных попыток авторизации
			FIREWALL_AUTH,
			// Разрешить блокирование зацикливающих запросов
			FIREWALL_LOOP,
			// Активировать черный список сайтов или портов
			FIREWALL_BLACK_LIST,
			// Активировать белый список сайтов или портов
			FIREWALL_WHITE_LIST,
			// Активировать ограничение трафика пользователей
			FIREWALL_BANDLIMIN,
			// Время блокирования зацикливающих запросов (s - секунды, m - минуты, h - часы, d - дни, M - месяцы, y - годы)
			FIREWALL_TIME_LOOP
		};
		// Заполняем структуру auth
		this->auth = {
			// Активировать авторизацию через пользователей в операционной системе
			AUTH_OS_USERS,
			// Активировать авторизацию через файл со списком пользователей и ip/mac адресами
			AUTH_FILE_USERS,
			// Разрешить авторизацию пользователя
			AUTH_ENABLED
		};
		// Заполняем структуру logs
		this->logs = {
			// Разрешить хранить логи в файлах
			LOGS_FILES,
			// Разрешить вывод логов в консоль
			LOGS_CONSOLE,
			// Разрешить ведение логов данных для обмена
			LOGS_DATA,
			// Разрешить ведение логов
			LOGS_ENABLED,
			// Размер файла лога
			Anyks::getBytes(LOGS_SIZE),
			// Адрес хранения логов
			LOGS_DIR
		};
		// Контроль подключений клиента к серверу
		this->connects = {
			// Ключ по которому определяются подключения (ip = 0, mac = 1)
			connect_key,
			// Максимальное количество файловых дескрипторов
			CONNECTS_FDS,
			// Максимальное количество подключений (одного клиента к прокси серверу)
			CONNECTS_CONNECT,
			// Общее количество одновременных подключений к прокси серверу
			CONNECTS_TOTAL,
			// Максимальный размер скачиваемых данных в байтах
			Anyks::getBytes(CONNECTS_SIZE)
		};
		// Заполняем структуру ipv4
		this->ipv4 = {
			// IP адрес интерфейса на котором будут приниматься запросы от клиентов
			IPV4_INTERNAL,
			// Внешний интерфейс, через который будут уходить запросы от сервера
			IPV4_EXTERNAL,
			// Серверы DNS, используемые для преобразования имён вышестоящих серверов в адреса
			IPV4_RESOLVER
		};
		// Заполняем структуру ipv6
		this->ipv6 = {
			// IP адрес интерфейса на котором будут приниматься запросы от клиентов
			IPV6_INTERNAL,
			// Внешний интерфейс, через который будут уходить запросы от сервера
			IPV6_EXTERNAL,
			// Серверы DNS, используемые для преобразования имён вышестоящих серверов в адреса
			IPV6_RESOLVER
		};
		// Заполняем структуру ldap
		this->ldap = {
			// Активация модуля LDAP
			LDAP_ENABLED,
			// Версия протокола LDAP
			LDAP_VER,
			// Тип поиска LDAP
			LDAP_SCOPE,
			// Адрес сервера LDAP
			LDAP_SERVER,
			// DN пользователя LDAP
			LDAP_USERDN,
			// Фильтр поиска LDAP
			LDAP_FILTER,
			// DN администратора LDAP
			LDAP_BINDDN,
			// Пароль администратора LDAP
			LDAP_BINDPW
		};
		// Заполняем структуру timeouts
		this->timeouts = {
			// Таймаут времени на чтение
			(size_t) Anyks::getSeconds(TIMEOUTS_READ),
			// Таймаут времени на запись
			(size_t) Anyks::getSeconds(TIMEOUTS_WRITE),
			// Таймаут на работу в режиме переключения протоколов
			(size_t) Anyks::getSeconds(TIMEOUTS_UPGRADE)
		};
		// Заполняем структуру buffers
		this->buffers = {
			// Скорость входящего подключения
			Anyks::getSizeBuffer(BUFFER_READ_SIZE),
			// Скорость исходящего подключения
			Anyks::getSizeBuffer(BUFFER_WRITE_SIZE)
		};
		// Заполняем структуру постоянного подключения keepalive
		this->keepalive = {
			// Максимальное количество попыток
			KEEPALIVE_CNT,
			// Интервал времени в секундах через которое происходит проверка подключения
			KEEPALIVE_IDLE,
			// Интервал времени в секундах между попытками
			KEEPALIVE_INTVL
		};
	// Если все нормально то выполняем извлечение данных из конфигурационного файла
	} else {
		// Активируем разрешение connect прокси сервера
		this->options = (ini.getBoolean("proxy", "connect", true) ? OPT_CONNECT : OPT_NULL);
		// Активируем разрешение переключения протокола прокси сервера
		this->options = (this->options | (ini.getBoolean("proxy", "upgrade", false) ? OPT_UPGRADE : OPT_NULL));
		// Активируем вывод или скрытие названия прокси сервера
		this->options = (this->options | (ini.getBoolean("proxy", "agent", true) ? OPT_AGENT : OPT_NULL));
		// Попробовать обойти блокировки сайтов на уровне прокси (многие сайты могут работать не правильно)
		this->options = (this->options | (ini.getBoolean("proxy", "deblock", false) ? OPT_DEBLOCK : OPT_NULL));
		// Активируем разрешение сжатия данных методом gzip
		this->options = (this->options | (ini.getBoolean("gzip", "transfer", true) ? OPT_GZIP : OPT_NULL));
		// Активируем сжатие не сжатых данных
		this->options = (this->options | (ini.getBoolean("gzip", "response", true) ? OPT_PGZIP : OPT_NULL));
		// Активируем разрешающий smart прокси сервер
		this->options = (this->options | ((ini.getString("proxy", "skill", "dumb").compare("smart") == 0) ? OPT_SMART : OPT_NULL));
		// Активируем постоянное соединение
		this->options = (this->options | (ini.getBoolean("keepalive", "enabled", true) ? OPT_KEEPALIVE : OPT_NULL));
		// Активируем логирование данных
		this->options = (this->options | (ini.getBoolean("logs", "enabled", true) ? OPT_LOG : OPT_NULL));
		// Уровень сжатия gzip
		int glevel = Z_DEFAULT_COMPRESSION;
		// Получаем уровень сжатия
		const string gzipLevel = ini.getString("gzip", "level");
		// Если размер указан
		if(!gzipLevel.empty()){
			// Определяем тип сжатия
			if(gzipLevel.compare("default") == 0)		glevel = Z_DEFAULT_COMPRESSION;
			else if(gzipLevel.compare("best") == 0)		glevel = Z_BEST_COMPRESSION;
			else if(gzipLevel.compare("speed") == 0)	glevel = Z_BEST_SPEED;
			else if(gzipLevel.compare("no") == 0)		glevel = Z_NO_COMPRESSION;
		}
		// Тип ключа определения коннектов к прокси
		u_int connect_key = (Anyks::toCase(ini.getString("connects", "key", CONNECTS_KEY)).compare("mac") == 0 ? 1 : 0);
		// Тип прокси сервера
		u_int proxy_type, proxy_port;
		// Получаем тип прокси сервера
		const string type = ini.getString("proxy", "type", PROXY_TYPE);
		// Если прокси сервер это http
		if(type.compare("http") == 0) proxy_type = 1;
		else if(type.compare("socks5") == 0) proxy_type = 2;
		else if(type.compare("redirect") == 0) proxy_type = 3;
		else proxy_type = 1;
		// Определяем порт прокси сервера
		switch(proxy_type){
			// Порт http прокси
			case 1: proxy_port = PROXY_HTTP_PORT;		break;
			// Порт socks5 прокси
			case 2: proxy_port = PROXY_SOCKS5_PORT;		break;
			// Порт redirect прокси
			case 3: proxy_port = PROXY_REDIRECT_PORT;	break;
		}
		// Получаем данные режима мульти-сетевого взаимодействия
		bool proxy_subnet = ini.getBoolean("proxy", "subnet", PROXY_SUBNET);
		// Массив протоколов
		auto ipVx = Anyks::split(ini.getString("proxy", "ipv", PROXY_IPV), "->");
		// Массив внешних ip адресов
		auto externalIPv4 = Anyks::split(ini.getString("ipv4", "external"), "|");
		auto externalIPv6 = Anyks::split(ini.getString("ipv6", "external"), "|");
		// Массив dns серверов
		auto resolver4 = Anyks::split(ini.getString("ipv4", "resolver"), "|");
		auto resolver6 = Anyks::split(ini.getString("ipv6", "resolver"), "|");
		// Массив параметров сжатия для проксированных запросов
		auto gproxied = Anyks::split(ini.getString("gzip", "proxied"), "|");
		// Массив версий http протоколов
		auto gvhttp = Anyks::split(ini.getString("gzip", "vhttp"), "|");
		// Массив параметров сжатия для типов данных
		auto gtypes = Anyks::split(ini.getString("gzip", "types"), "|");
		// Запоминаем внешнюю и внутреннюю версию интернет протокола
		u_int proxy_intIPv = ::atoi(ipVx[0].c_str());
		u_int proxy_extIPv = ::atoi(ipVx[1].c_str());
		// Если ресолвер пустой тогда устанавливаем значение по умолчанию
		if(externalIPv4.empty()) externalIPv4 = IPV4_EXTERNAL;
		if(externalIPv6.empty()) externalIPv6 = IPV6_EXTERNAL;
		// Если ресолвер пустой тогда устанавливаем значение по умолчанию
		if(resolver4.empty()) resolver4 = IPV4_RESOLVER;
		if(resolver6.empty()) resolver6 = IPV6_RESOLVER;
		// Если версии не указаны тогда устанавливаем значение по умолчанию
		if(gproxied.empty()) gproxied = GZIP_PROXIED;
		// Если версии не указаны тогда устанавливаем значение по умолчанию
		if(gvhttp.empty()) gvhttp = GZIP_VHTTP;
		// Если версии не указаны тогда устанавливаем значение по умолчанию
		if(gtypes.empty()) gtypes = GZIP_TYPES;
		// Если версия меньше 4 или больше 6 то устанавливаем версию по умолчанию
		proxy_intIPv = ((proxy_intIPv < 4) || (proxy_intIPv > 6) ? 4 : proxy_intIPv);
		proxy_extIPv = ((proxy_extIPv < 4) || (proxy_extIPv > 6) ? 4 : proxy_extIPv);
		// Если ни один из сетевых интерфейсов не принадлежит IPv6 тогда отключаем мульти-сетевое взаимодействие
		if((proxy_intIPv != 6) && (proxy_extIPv != 6)) proxy_subnet = false;
		// Заполняем структуру proxy
		this->proxy = {
			// Версия внутреннего протокола интернета (4, 6)
			proxy_intIPv,
			// Версия внешнего протокола интернета (4, 6)
			proxy_extIPv,
			// Тип прокси сервера (http, socks5, redirect)
			proxy_type,
			// Устанавливаем порт прокси сервера
			(u_int) ini.getUNumber("proxy", "port", proxy_port),
			// Максимальное количество воркеров
			(u_int) ini.getUNumber("proxy", "workers", MAX_WORKERS),
			// Активация режима отладки
			ini.getBoolean("proxy", "debug", PROXY_DEBUG),
			// Запусть в виде демона
			ini.getBoolean("proxy", "daemon", PROXY_DAEMON),
			// Обратный прокси (доступ из сети в локальную сеть)
			ini.getBoolean("proxy", "reverse", PROXY_REVERSE),
			// Активация поддержки прокси листа
			ini.getBoolean("proxy", "transfer", PROXY_TRANSFER),
			// Прямой прокси (доступ во внешнюю сеть)
			ini.getBoolean("proxy", "forward", PROXY_FORWARD),
			// Активация режима отображения IPv4 в IPv6
			ini.getBoolean("proxy", "ipv6only", PROXY_IPV6ONLY),
			// Активация режима мульти-сетевого взаимодействия
			proxy_subnet,
			// Оптимизировать настройки операционной системы (нужен root доступ)
			ini.getBoolean("proxy", "optimos", PROXY_OPTIMOS),
			// Активация конвеерной обработки
			ini.getBoolean("proxy", "pipelining", PROXY_PIPELINING),
			// Идентификатор группы пользователя под которым запускается прокси
			ini.getString("proxy", "user", PROXY_USER),
			// Идентификатор пользователя под которым запускается прокси
			ini.getString("proxy", "group", PROXY_GROUP),
			// Название сервиса
			ini.getString("proxy", "name", PROXY_NAME),
			// Адрес хранения pid файла
			ini.getString("proxy", "piddir", PID_DIR),
			// Адрес хранения конфигурационных файлов
			ini.getString("proxy", "dir", CONFIG_DIR)
		};
		// Заполняем структуру gzip
		this->gzip = {
			// Разрешает или запрещает выдавать в ответе поле заголовка “Vary: Accept-Encoding”
			ini.getBoolean("gzip", "vary", GZIP_VARY),
			// Тип сжатия (default - по умолчанию, best - лучшее сжатие, speed - лучшая скорость, no - без сжатия)
			glevel,
			// Минимальная длина данных после которых включается сжатие (работает только с Content-Length)
			ini.getNumber("gzip", "length", GZIP_LENGTH),
			// Максимальный размер чанка в байтах
			Anyks::getBytes(ini.getString("gzip", "chunk", GZIP_CHUNK)),
			// Не сжимать контент, UserAgent которого соответсвует регулярному выражению
			ini.getString("gzip", "regex", GZIP_REGEX),
			// Версия http протокола
			gvhttp,
			// Разрешает или запрещает сжатие ответа методом gzip для проксированных запросов
			gproxied,
			// Разрешает сжатие ответа методом gzip для указанных MIME-типов
			gtypes
		};
		// Заполняем структуру cache
		this->cache = {
			// Кэширование dns запросов
			ini.getBoolean("cache", "dns", CACHE_DNS),
			// Кэширование часто-запрашиваемых страниц
			ini.getBoolean("cache", "dat", CACHE_RESPONSE),
			// Время жизни dns кэша в секундах
			(time_t) Anyks::getSeconds(ini.getString("cache", "dttl", CACHE_DTTL)),
			// Каталог хранения кэш файлов
			ini.getString("cache", "dir", CACHE_DIR)
		};
		// Заполняем структуру firewall
		this->firewall = {
			// Количество неудачных попыток авторизации
			(u_int) ini.getUNumber("firewall", "maxtryauth", FIREWALL_MAX_TRYAUTH),
			// Разрешить блокирование пользователя после неудачных попыток авторизации
			ini.getBoolean("firewall", "blockauth", FIREWALL_AUTH),
			// Разрешить блокирование зацикливающих запросов
			ini.getBoolean("firewall", "blockloop", FIREWALL_LOOP),
			// Активировать черный список сайтов или портов
			ini.getBoolean("firewall", "blacklist", FIREWALL_BLACK_LIST),
			// Активировать белый список сайтов или портов
			ini.getBoolean("firewall", "whitelist", FIREWALL_WHITE_LIST),
			// Активировать ограничение трафика пользователей
			ini.getBoolean("firewall", "bandlimin", FIREWALL_BANDLIMIN),
			// Время блокирования зацикливающих запросов (s - секунды, m - минуты, h - часы, d - дни, M - месяцы, y - годы)
			ini.getString("firewall", "timeblockloop", FIREWALL_TIME_LOOP)
		};
		// Заполняем структуру auth
		this->auth = {
			// Активировать авторизацию через пользователей в операционной системе
			ini.getBoolean("authorization", "osusers", AUTH_OS_USERS),
			// Активировать авторизацию через файл со списком пользователей и ip/mac адресами
			ini.getBoolean("authorization", "listusers", AUTH_FILE_USERS),
			// Разрешить авторизацию пользователя
			ini.getBoolean("authorization", "enabled", AUTH_ENABLED)
		};
		// Заполняем структуру logs
		this->logs = {
			// Разрешить хранить логи в файлах
			ini.getBoolean("logs", "files", LOGS_FILES),
			// Разрешить вывод логов в консоль
			ini.getBoolean("logs", "console", LOGS_CONSOLE),
			// Разрешить ведение логов данных для обмена
			ini.getBoolean("logs", "data", LOGS_DATA),
			// Разрешить ведение логов
			ini.getBoolean("logs", "enabled", LOGS_ENABLED),
			// Размер файла лога
			Anyks::getBytes(ini.getString("logs", "size", LOGS_SIZE)),
			// Адрес хранения логов
			ini.getString("logs", "dir", LOGS_DIR)
		};
		// Контроль подключений клиента к серверу
		this->connects = {
			// Ключ по которому определяются подключения (ip = 0, mac = 1)
			connect_key,
			// Максимальное количество файловых дескрипторов
			(u_int) ini.getUNumber("connects", "fds", CONNECTS_FDS),
			// Максимальное количество подключений (одного клиента к прокси серверу)
			(u_int) ini.getUNumber("connects", "connect", CONNECTS_CONNECT),
			// Общее количество одновременных подключений к прокси серверу
			(int) ini.getNumber("connects", "total", CONNECTS_TOTAL),
			// Максимальный размер скачиваемых данных в байтах
			Anyks::getBytes(ini.getString("connects", "size", CONNECTS_SIZE))
		};
		// Заполняем структуру ipv4
		this->ipv4 = {
			// IP адрес интерфейса на котором будут приниматься запросы от клиентов
			ini.getString("ipv4", "internal", IPV4_INTERNAL),
			// Внешний интерфейс, через который будут уходить запросы от сервера
			externalIPv4,
			// Серверы DNS, используемые для преобразования имён вышестоящих серверов в адреса
			resolver4
		};
		// Заполняем структуру ipv6
		this->ipv6 = {
			// IP адрес интерфейса на котором будут приниматься запросы от клиентов
			ini.getString("ipv6", "internal", IPV6_INTERNAL),
			// Внешний интерфейс, через который будут уходить запросы от сервера
			externalIPv6,
			// Серверы DNS, используемые для преобразования имён вышестоящих серверов в адреса
			resolver6
		};
		// Заполняем структуру ldap
		this->ldap = {
			// Активация модуля LDAP
			ini.getBoolean("authorization", "ldap", LDAP_ENABLED),
			// Версия протокола LDAP
			(u_int) ini.getUNumber("ldap", "version", LDAP_VER),
			// Тип поиска LDAP
			ini.getString("ldap", "scope", LDAP_SCOPE),
			// Адрес сервера LDAP
			ini.getString("ldap", "server", LDAP_SERVER),
			// DN пользователя LDAP
			ini.getString("ldap", "userdn", LDAP_USERDN),
			// Фильтр поиска LDAP
			ini.getString("ldap", "filter", LDAP_FILTER),
			// DN администратора LDAP
			ini.getString("ldap", "binddn", LDAP_BINDDN),
			// Пароль администратора LDAP
			ini.getString("ldap", "bindpw", LDAP_BINDPW)
		};
		// Заполняем структуру timeouts
		this->timeouts = {
			// Таймаут времени на чтение
			(size_t) Anyks::getSeconds(ini.getString("timeouts", "read", TIMEOUTS_READ)),
			// Таймаут времени на запись
			(size_t) Anyks::getSeconds(ini.getString("timeouts", "write", TIMEOUTS_WRITE)),
			// Таймаут на работу в режиме переключения протоколов
			(size_t) Anyks::getSeconds(ini.getString("timeouts", "upgrade", TIMEOUTS_UPGRADE))
		};
		// Заполняем структуру buffers
		this->buffers = {
			// Скорость входящего подключения
			Anyks::getSizeBuffer(ini.getString("speed", "input", BUFFER_READ_SIZE)),
			// Скорость исходящего подключения
			Anyks::getSizeBuffer(ini.getString("speed", "output", BUFFER_WRITE_SIZE))
		};
		// Заполняем структуру постоянного подключения keepalive
		this->keepalive = {
			// Максимальное количество попыток
			(int) ini.getNumber("keepalive", "keepcnt", KEEPALIVE_CNT),
			// Интервал времени в секундах через которое происходит проверка подключения
			(int) ini.getNumber("keepalive", "keepidle", KEEPALIVE_IDLE),
			// Интервал времени в секундах между попытками
			(int) ini.getNumber("keepalive", "keepintvl", KEEPALIVE_INTVL)
		};
	}
}
