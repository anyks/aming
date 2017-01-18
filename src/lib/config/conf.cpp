/* МОДУЛЬ CONFIG ДЛЯ ПРОКСИ СЕРВЕРА ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#include "conf.h"

// Устанавливаем область видимости
using namespace std;

/**
 * rtrim Функция усечения указанных символов с правой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & Config::rtrim(string &str, const char * t){
	str.erase(str.find_last_not_of(t) + 1);
	return str;
}
/**
 * ltrim Функция усечения указанных символов с левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & Config::ltrim(string &str, const char * t){
	str.erase(0, str.find_first_not_of(t));
	return str;
}
/**
 * trim Функция усечения указанных символов с правой и левой стороны строки
 * @param  str строка для усечения
 * @param  t   список символов для усечения
 * @return     результирующая строка
 */
string & Config::trim(string &str, const char * t){
	return ltrim(rtrim(str, t), t);
}
/**
 * getResolver Метод извлечения dns серверов из строки в массив
 * @param  str строка с dns серверами
 * @return     массив с dns серверами
 */
vector <string> Config::getResolver(const string str){
	vector <string> resolver;
	// Если строка передана
	if(!str.empty()){
		string dns, delim = ",";
		string::size_type i = 0;
		string::size_type j = str.find(delim);
		u_int len = delim.length();
		// Выполняем разбиение строк
		while(j != string::npos){
			dns = str.substr(i, j - i);
			resolver.push_back(trim(dns));
			i = ++j + (len - 1);
			j = str.find(delim, j);
			if(j == string::npos){
				dns = str.substr(i, str.length());
				resolver.push_back(trim(dns));
			}
		}
		// Если данные не существуют то устанавливаем строку по умолчанию
		if(resolver.empty()) resolver.push_back(str);
	}
	// Выводим результат
	return resolver;
}
/**
 * isFileExist Функция проверки существования файла
 * @param  path адрес каталога
 * @return      результат проверки
 */
bool Config::isFileExist(const char * path){
	// Структура проверка статистики
	struct stat info;
	// Проверяем переданный нам адрес
	if(stat(path, &info) != 0) return false;
	// Если это файл
	return (info.st_mode & S_IFMT) != 0;
}
/**
 * getSizeBuffer Функция получения размера буфера в байтах
 * @param  speed пропускная способность сети в мегабитах
 * @return       размер буфера в байтах
 */
int Config::getSizeBuffer(float speed){
	/*
	* Help - http://www.securitylab.ru/analytics/243414.php
	*
	* 0.04 - Пропускная способность сети 40 милисекунд
	* 100 - Скорость в мегабитах (Мб) на пользователя
	* 8 - Количество бит в байте
	* 1000 - количество байт в мегабайте
	* (2 * 0.04) * ((100 * 1000) / 8)  = 1000 байт
	*
	*/
	// Размер буфера по умолчанию
	int size = (int) speed;
	// Если скорость установлена тогда расчитываем размер буфера
	if(speed > -1) size = ((2 * 0.04) * ((speed * 1000) / 8)) * 100;
	// Выводим результат
	return size;
}
/**
 * Config Конструктор модуля конфигурационного файла
 * @param filename адрес конфигурационного файла
 */
Config::Config(const string filename){
	// Запоминаем адрес файла с конфигурационным файлом
	this->filename = filename;
	// Если каталог с конфигурационным файлом не найден
	if(this->filename.empty() || !isFileExist(this->filename.c_str())){
		// Если это OS Linux
		#ifdef __linux__
			// Запоминаем адрес конфигурационного файла Linux
			this->filename = (string("/etc/") + string(PROXY_NAME) + string("/config.ini"));
		// Если это OS Apple или FreeBSD
		#elif __APPLE__ || __MACH__ || __FreeBSD__
			// Запоминаем адрес конфигурационного файла MacOS X и FreeBSD
			this->filename = (string("/usr/local/etc/") + string(PROXY_NAME) + string("/config.ini"));
		#endif
	}
	// Инициализируем парсер ini файла
	INI ini(this->filename);
	// Если во время чтения файла возникли ошибки
	if(ini.ParseError() < 0){
		// Выводим сообщение об ошибке
		printf("Can't load config file %s\n", this->filename.c_str());
		// Формируем опции по умолчанию
		this->options = (OPT_CONNECT | OPT_AGENT | OPT_GZIP | OPT_KEEPALIVE | OPT_LOG | OPT_PGZIP);
		// Получаем тип прокси сервера
		string type = PROXY_TYPE;
		// Тип прокси сервера
		u_int proxy_type, proxy_port;
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
			// Общее количество одновременных подключений к прокси серверу
			ALL_CONNECTS,
			// Версия протокола интернета (4, 6)
			PROXY_IPV,
			// Тип прокси сервера (http, socks5, redirect)
			proxy_type,
			// Устанавливаем порт прокси сервера
			proxy_port,
			// Максимальное количество подключений (одного клиента к прокси серверу)
			MAX_CONNECTS,
			// Максимальное количество файловых дескрипторов
			MAX_FDS,
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
			// Попробовать обойти блокировки сайтов на уровне прокси (многие сайты могут работать не правильно)
			PROXY_DEDLOCK,
			// Оптимизировать настройки операционной системы (нужен root доступ)
			PROXY_OPTIMOS,
			// Активировать кеширование часто-запрашиваемых страниц
			PROXY_CACHE,
			// Идентификатор группы пользователя под которым запускается прокси
			PROXY_USER,
			// Идентификатор пользователя под которым запускается прокси
			PROXY_GROUP,
			// Название сервиса
			PROXY_NAME,
			// Адрес хранения pid файла
			PID_DIR,
			// Адрес хранения конфигурационных файлов
			CONFIG_DIR,
			// Список dns серверов
			PROXY_RESOLVER
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
			// Разрешить ведение логов
			LOGS_ENABLED,
			// Размер файла лога в Кб
			LOGS_SIZE,
			// Адрес хранения логов
			LOGS_DIR
		};
		// Удалять указанные http заголовки из запроса или ответа
		this->rmheader = {
			// Убирать заголовки в запросе
			RM_HEADERS_REQUEST,
			// Убирать заголовки в ответе
			RM_HEADERS_RESPONSE
		};
		// Устанавливать указанные http заголовки в запрос или ответ
		this->setheader = {
			// Установить заголовки в запрос
			SET_HEADERS_REQUEST,
			// Установить заголовки в ответ
			SET_HEADERS_RESPONSE
		};
		// Заполняем структуру ipv4
		this->ipv4 = {
			// Внешний интерфейс, через который будут уходить запросы от сервера
			IPV4_EXTERNAL,
			// IP адрес интерфейса на котором будут приниматься запросы от клиентов
			IPV4_INTERNAL
		};
		// Заполняем структуру ipv6
		this->ipv6 = {
			// Внешний интерфейс, через который будут уходить запросы от сервера
			IPV6_EXTERNAL,
			// IP адрес интерфейса на котором будут приниматься запросы от клиентов
			IPV6_INTERNAL
		};
		// Заполняем структуру timeouts
		this->timeouts = {
			// Таймаут времени на чтение
			TIMEOUTS_READ,
			// Таймаут времени на запись
			TIMEOUTS_WRITE
		};
		// Заполняем структуру buffers
		this->buffers = {
			// Скорость входящего подключения
			getSizeBuffer(BUFFER_READ_SIZE),
			// Скорость исходящего подключения
			getSizeBuffer(BUFFER_WRITE_SIZE)
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
		this->options = (ini.GetBoolean("proxy", "connect", true) ? OPT_CONNECT : OPT_NULL);
		// Активируем вывод названия прокси сервера
		this->options = (this->options | (ini.GetBoolean("proxy", "headname", true) ? OPT_AGENT : OPT_NULL));
		// Активируем разрешение сжатия данных методом gzip
		this->options = (this->options | (ini.GetBoolean("gzip", "transfer", true) ? OPT_GZIP : OPT_NULL));
		// Активируем сжатие не сжатых данных
		this->options = (this->options | (ini.GetBoolean("gzip", "response", true) ? OPT_PGZIP : OPT_NULL));
		// Активируем разрешающий smart прокси сервер
		this->options = (this->options | ((ini.Get("proxy", "skill", "dumb").compare("smart") == 0) ? OPT_SMART : OPT_NULL));
		// Активируем постоянное соединение
		this->options = (this->options | (ini.GetBoolean("keepalive", "keepalive", true) ? OPT_KEEPALIVE : OPT_NULL));
		// Активируем логирование данных
		this->options = (this->options | (ini.GetBoolean("logs", "enabled", true) ? OPT_LOG : OPT_NULL));
		// Тип прокси сервера
		u_int proxy_type, proxy_port, proxy_ipv;
		// Получаем версию протокола интернета
		proxy_ipv = ini.GetInteger("proxy", "ipver", PROXY_IPV);
		// Если версия меньше 4 или больше 6 то устанавливаем версию по умолчанию
		proxy_ipv = ((proxy_ipv < 4) || (proxy_ipv > 6) ? PROXY_IPV : proxy_ipv);
		// Получаем тип прокси сервера
		string type = ini.Get("proxy", "type", PROXY_TYPE);
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
		// Массив dns серверов
		vector <string> resolver = getResolver(ini.Get("proxy", "resolver", ""));
		// Если ресолвер пустой тогда устанавливаем значение по умолчанию
		if(resolver.empty()) resolver = PROXY_RESOLVER;
		// Заполняем структуру proxy
		this->proxy = {
			// Общее количество одновременных подключений к прокси серверу
			(int) ini.GetInteger("proxy", "allcon", ALL_CONNECTS),
			// Версия протокола интернета (4, 6)
			proxy_ipv,
			// Тип прокси сервера (http, socks5, redirect)
			proxy_type,
			// Устанавливаем порт прокси сервера
			(u_int) ini.GetInteger("proxy", "port", proxy_port),
			// Максимальное количество подключений (одного клиента к прокси серверу)
			(u_int) ini.GetInteger("proxy", "maxcon", MAX_CONNECTS),
			// Максимальное количество файловых дескрипторов
			(u_int) ini.GetInteger("proxy", "maxfds", MAX_FDS),
			// Активация режима отладки
			ini.GetBoolean("proxy", "debug", PROXY_DEBUG),
			// Запусть в виде демона
			ini.GetBoolean("proxy", "daemon", PROXY_DAEMON),
			// Обратный прокси (доступ из сети в локальную сеть)
			ini.GetBoolean("proxy", "reverse", PROXY_REVERSE),
			// Активация поддержки прокси листа
			ini.GetBoolean("proxy", "transfer", PROXY_TRANSFER),
			// Прямой прокси (доступ во внешнюю сеть)
			ini.GetBoolean("proxy", "forward", PROXY_FORWARD),
			// Попробовать обойти блокировки сайтов на уровне прокси (многие сайты могут работать не правильно)
			ini.GetBoolean("proxy", "deblock", PROXY_DEDLOCK),
			// Оптимизировать настройки операционной системы (нужен root доступ)
			ini.GetBoolean("proxy", "optimos", PROXY_OPTIMOS),
			// Активировать кеширование часто-запрашиваемых страниц
			ini.GetBoolean("proxy", "cache", PROXY_CACHE),
			// Идентификатор группы пользователя под которым запускается прокси
			ini.Get("proxy", "user", PROXY_USER),
			// Идентификатор пользователя под которым запускается прокси
			ini.Get("proxy", "group", PROXY_GROUP),
			// Название сервиса
			ini.Get("proxy", "name", PROXY_NAME),
			// Адрес хранения pid файла
			ini.Get("proxy", "piddir", PID_DIR),
			// Адрес хранения конфигурационных файлов
			ini.Get("proxy", "confdir", CONFIG_DIR),
			// Список dns серверов
			resolver
		};
		// Заполняем структуру firewall
		this->firewall = {
			// Количество неудачных попыток авторизации
			(u_int) ini.GetInteger("firewall", "maxtryauth", FIREWALL_MAX_TRYAUTH),
			// Разрешить блокирование пользователя после неудачных попыток авторизации
			ini.GetBoolean("firewall", "blockauth", FIREWALL_AUTH),
			// Разрешить блокирование зацикливающих запросов
			ini.GetBoolean("firewall", "blockloop", FIREWALL_LOOP),
			// Активировать черный список сайтов или портов
			ini.GetBoolean("firewall", "blacklist", FIREWALL_BLACK_LIST),
			// Активировать белый список сайтов или портов
			ini.GetBoolean("firewall", "whitelist", FIREWALL_WHITE_LIST),
			// Активировать ограничение трафика пользователей
			ini.GetBoolean("firewall", "bandlimin", FIREWALL_BANDLIMIN),
			// Время блокирования зацикливающих запросов (s - секунды, m - минуты, h - часы, d - дни, M - месяцы, y - годы)
			ini.Get("firewall", "timeblockloop", FIREWALL_TIME_LOOP)
		};
		// Заполняем структуру auth
		this->auth = {
			// Активировать авторизацию через пользователей в операционной системе
			ini.GetBoolean("authorization", "osusers", AUTH_OS_USERS),
			// Активировать авторизацию через файл со списком пользователей и ip/mac адресами
			ini.GetBoolean("authorization", "listusers", AUTH_FILE_USERS),
			// Разрешить авторизацию пользователя
			ini.GetBoolean("authorization", "enabled", AUTH_ENABLED)
		};
		// Заполняем структуру logs
		this->logs = {
			// Разрешить хранить логи в файлах
			ini.GetBoolean("logs", "files", LOGS_FILES),
			// Разрешить ведение логов
			ini.GetBoolean("logs", "enabled", LOGS_ENABLED),
			// Размер файла лога в Кб
			(u_int) ini.GetInteger("logs", "size", LOGS_SIZE),
			// Адрес хранения логов
			ini.Get("logs", "dir", LOGS_DIR)
		};
		// Удалять указанные http заголовки из запроса или ответа
		this->rmheader = {
			// Убирать заголовки в запросе
			ini.GetBoolean("rmheader", "request", RM_HEADERS_REQUEST),
			// Убирать заголовки в ответе
			ini.GetBoolean("rmheader", "response", RM_HEADERS_RESPONSE)
		};
		// Установить указанные http заголовки в запрос или ответ
		this->setheader = {
			// Установить заголовки в запрос
			ini.GetBoolean("setheader", "request", SET_HEADERS_REQUEST),
			// Установить заголовки в ответ
			ini.GetBoolean("setheader", "response", SET_HEADERS_RESPONSE)
		};
		// Заполняем структуру ipv4
		this->ipv4 = {
			// Внешний интерфейс, через который будут уходить запросы от сервера
			ini.Get("ipv4", "external", IPV4_EXTERNAL),
			// IP адрес интерфейса на котором будут приниматься запросы от клиентов
			ini.Get("ipv4", "internal", IPV4_INTERNAL)
		};
		// Заполняем структуру ipv6
		this->ipv6 = {
			// Внешний интерфейс, через который будут уходить запросы от сервера
			ini.Get("ipv6", "external", IPV6_EXTERNAL),
			// IP адрес интерфейса на котором будут приниматься запросы от клиентов
			ini.Get("ipv6", "internal", IPV6_INTERNAL)
		};
		// Заполняем структуру timeouts
		this->timeouts = {
			// Таймаут времени на чтение
			(u_short) ini.GetInteger("timeouts", "read", TIMEOUTS_READ),
			// Таймаут времени на запись
			(u_short) ini.GetInteger("timeouts", "write", TIMEOUTS_WRITE)
		};
		// Заполняем структуру buffers
		this->buffers = {
			// Скорость входящего подключения
			getSizeBuffer((float) ini.GetReal("speed", "input", BUFFER_READ_SIZE)),
			// Скорость исходящего подключения
			getSizeBuffer((float) ini.GetReal("speed", "output", BUFFER_WRITE_SIZE))
		};
		// Заполняем структуру постоянного подключения keepalive
		this->keepalive = {
			// Максимальное количество попыток
			(int) ini.GetInteger("keepalive", "keepcnt", KEEPALIVE_CNT),
			// Интервал времени в секундах через которое происходит проверка подключения
			(int) ini.GetInteger("keepalive", "keepidle", KEEPALIVE_IDLE),
			// Интервал времени в секундах между попытками
			(int) ini.GetInteger("keepalive", "keepintvl", KEEPALIVE_INTVL)
		};
	}
}