/* МОДУЛЬ CONFIG ДЛЯ ПРОКСИ СЕРВЕРА ANYKS */
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
 * toCase Функция перевода в указанный регистр
 * @param  str  строка для перевода в указанных регистр
 * @param  flag флаг указания типа регистра
 * @return      результирующая строка
 */
string Config::toCase(string str, bool flag){
	// Переводим в указанный регистр
	transform(str.begin(), str.end(), str.begin(), (flag ? ::toupper : ::tolower));
	// Выводим результат
	return str;
}
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
 * split Метод разбива строки на составляющие
 * @param  str   исходная строка
 * @param  delim разделитель
 * @return       массив составляющих строки
 */
vector <string> Config::split(const string str, const string delim){
	// Результат данных
	vector <string> result;
	// Если строка передана
	if(!str.empty()){
		string data;
		string::size_type i = 0;
		string::size_type j = str.find(delim);
		u_int len = delim.length();
		// Выполняем разбиение строк
		while(j != string::npos){
			data = str.substr(i, j - i);
			result.push_back(trim(data));
			i = ++j + (len - 1);
			j = str.find(delim, j);
			if(j == string::npos){
				data = str.substr(i, str.length());
				result.push_back(trim(data));
			}
		}
		// Если данные не существуют то устанавливаем строку по умолчанию
		if(result.empty()) result.push_back(str);
	}
	// Выводим результат
	return result;
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
 * @param  str пропускная способность сети (bps, kbps, Mbps, Gbps)
 * @return     размер буфера в байтах
 */
long Config::getSizeBuffer(const string str){
	/*
	* Help - http://www.securitylab.ru/analytics/243414.php
	*
	* 0.04 - Пропускная способность сети 40 милисекунд
	* 100 - Скорость в мегабитах (Мб) на пользователя
	* 8 - Количество бит в байте
	* 1024000 - количество байт в мегабайте
	* (2 * 0.04) * ((100 * 1024000) / 8)  = 1000 байт
	*
	*/
	// Размер буфера в байтах
	long size = -1;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e("\\b([\\d\\.\\,]+)(bps|kbps|Mbps|Gbps)", regex::ECMAScript);
	// Выполняем поиск протокола
	regex_search(str, match, e);
	// Если данные найдены
	if(!match.empty() && (match.size() == 3)){
		// Запоминаем параметры
		string param = match[2].str();
		// Размерность скорости
		double dimension = 1;
		// Получаем значение скорости
		double speed = ::atof(match[1].str().c_str());
		// Проверяем являются ли переданные данные байтами (8, 16, 32, 64, 128, 256, 512, 1024 ...)
		bool isbite = !fmod(speed / 8, 2);
		// Если это байты
		if(param.compare("bps") == 0) dimension = 1;
		// Если это размерность в киллобитах
		else if(param.compare("kbps") == 0) dimension = (isbite ? 1000 : 1024);
		// Если это размерность в мегабитах
		else if(param.compare("Mbps") == 0) dimension = (isbite ? 1000000 : 1048576);
		// Если это размерность в гигабитах
		else if(param.compare("Gbps") == 0) dimension = (isbite ? 1000000000 : 1073741824);
		// Размер буфера по умолчанию
		size = (long) speed;
		// Если скорость установлена тогда расчитываем размер буфера
		if(speed > -1) size = (2 * 0.04) * ((speed * dimension) / 8);
	}
	// Выводим результат
	return size;
}
/**
 * getBytes Функция получения размера в байтах из строки
 * @param  str строка обозначения размерности
 * @return     размер в байтах
 */
size_t Config::getBytes(const string str){
	// Размер буфера в байтах
	size_t size = 0;
	// Результат работы регулярного выражения
	smatch match;
	// Устанавливаем правило регулярного выражения
	regex e("\\b([\\d\\.\\,]+)(B|KB|MB|GB)", regex::ECMAScript);
	// Выполняем поиск протокола
	regex_search(str, match, e);
	// Если данные найдены
	if(!match.empty() && (match.size() == 3)){
		// Запоминаем параметры
		string param = match[2].str();
		// Размерность скорости
		double dimension = 1;
		// Получаем значение размерности
		double value = ::atof(match[1].str().c_str());
		// Проверяем являются ли переданные данные байтами (8, 16, 32, 64, 128, 256, 512, 1024 ...)
		bool isbite = !fmod(value / 8, 2);
		// Если это байты
		if(param.compare("B") == 0) dimension = 1;
		// Если это размерность в киллобитах
		else if(param.compare("KB") == 0) dimension = (isbite ? 1000 : 1024);
		// Если это размерность в мегабитах
		else if(param.compare("MB") == 0) dimension = (isbite ? 1000000 : 1048576);
		// Если это размерность в гигабитах
		else if(param.compare("GB") == 0) dimension = (isbite ? 1000000000 : 1073741824);
		// Размер буфера по умолчанию
		size = (long) value;
		// Если скорость установлена тогда расчитываем размер буфера
		if(value > -1) size = (value * dimension);
	}
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
		// Тип ключа определения коннектов к прокси
		u_int connect_key = (string(CONNECTS_KEY).compare("mac") == 0 ? 1 : 0);
		// Тип прокси сервера
		u_int proxy_type, proxy_port;
		// Массив протоколов
		vector <string> ipVx = split(PROXY_IPV, "->");
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
			// Оптимизировать настройки операционной системы (нужен root доступ)
			PROXY_OPTIMOS,
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
			// Кеширование dns запросов
			CACHE_DNS,
			// Кеширование часто-запрашиваемых страниц
			CACHE_RESPONSE,
			// Каталог хранения кеш файлов
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
			getBytes(GZIP_CHUNK),
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
			getBytes(LOGS_SIZE),
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
			getBytes(CONNECTS_SIZE)
		};
		// Заполняем структуру ipv4
		this->ipv4 = {
			// Внешний интерфейс, через который будут уходить запросы от сервера
			IPV4_EXTERNAL,
			// IP адрес интерфейса на котором будут приниматься запросы от клиентов
			IPV4_INTERNAL,
			// Серверы DNS, используемые для преобразования имён вышестоящих серверов в адреса
			IPV4_RESOLVER
		};
		// Заполняем структуру ipv6
		this->ipv6 = {
			// Внешний интерфейс, через который будут уходить запросы от сервера
			IPV6_EXTERNAL,
			// IP адрес интерфейса на котором будут приниматься запросы от клиентов
			IPV6_INTERNAL,
			// Серверы DNS, используемые для преобразования имён вышестоящих серверов в адреса
			IPV6_RESOLVER
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
		// Активируем вывод или скрытие названия прокси сервера
		this->options = (this->options | (ini.GetBoolean("proxy", "hideagent", false) ? OPT_AGENT : OPT_NULL));
		// Попробовать обойти блокировки сайтов на уровне прокси (многие сайты могут работать не правильно)
		this->options = (this->options | (ini.GetBoolean("proxy", "deblock", false) ? OPT_DEBLOCK : OPT_NULL));
		// Активируем разрешение сжатия данных методом gzip
		this->options = (this->options | (ini.GetBoolean("gzip", "transfer", true) ? OPT_GZIP : OPT_NULL));
		// Активируем сжатие не сжатых данных
		this->options = (this->options | (ini.GetBoolean("gzip", "response", true) ? OPT_PGZIP : OPT_NULL));
		// Активируем разрешающий smart прокси сервер
		this->options = (this->options | ((ini.Get("proxy", "skill", "dumb").compare("smart") == 0) ? OPT_SMART : OPT_NULL));
		// Активируем постоянное соединение
		this->options = (this->options | (ini.GetBoolean("keepalive", "enabled", true) ? OPT_KEEPALIVE : OPT_NULL));
		// Активируем логирование данных
		this->options = (this->options | (ini.GetBoolean("logs", "enabled", true) ? OPT_LOG : OPT_NULL));
		// Уровень сжатия gzip
		int glevel = Z_DEFAULT_COMPRESSION;
		// Получаем уровень сжатия
		string gzipLevel = ini.Get("gzip", "level", "");
		// Если размер указан
		if(!gzipLevel.empty()){
			// Определяем тип сжатия
			if(gzipLevel.compare("default") == 0)		glevel = Z_DEFAULT_COMPRESSION;
			else if(gzipLevel.compare("best") == 0)		glevel = Z_BEST_COMPRESSION;
			else if(gzipLevel.compare("speed") == 0)	glevel = Z_BEST_SPEED;
			else if(gzipLevel.compare("no") == 0)		glevel = Z_NO_COMPRESSION;
		}
		// Тип ключа определения коннектов к прокси
		u_int connect_key = (toCase(ini.Get("connects", "key", CONNECTS_KEY)).compare("mac") == 0 ? 1 : 0);
		// Тип прокси сервера
		u_int proxy_type, proxy_port;
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
		// Массив протоколов
		vector <string> ipVx = split(ini.Get("proxy", "ipv", PROXY_IPV), "->");
		// Запоминаем внешнюю версию интернет протокола
		u_int proxy_intIPv = ::atoi(ipVx[0].c_str());
		// Запоминаем внутреннюю версию интернет протокола
		u_int proxy_extIPv = ::atoi(ipVx[1].c_str());
		// Если версия меньше 4 или больше 6 то устанавливаем версию по умолчанию
		proxy_intIPv = ((proxy_intIPv < 4) || (proxy_intIPv > 6) ? 4 : proxy_intIPv);
		proxy_extIPv = ((proxy_extIPv < 4) || (proxy_extIPv > 6) ? 4 : proxy_extIPv);
		// Массив dns серверов
		vector <string> resolver4 = split(ini.Get("ipv4", "resolver", ""), "|");
		vector <string> resolver6 = split(ini.Get("ipv6", "resolver", ""), "|");
		// Если ресолвер пустой тогда устанавливаем значение по умолчанию
		if(resolver4.empty()) resolver4 = IPV4_RESOLVER;
		if(resolver6.empty()) resolver6 = IPV6_RESOLVER;
		// Массив версий http протоколов
		vector <string> gvhttp = split(ini.Get("gzip", "vhttp", ""), "|");
		// Если версии не указаны тогда устанавливаем значение по умолчанию
		if(gvhttp.empty()) gvhttp = GZIP_VHTTP;
		// Массив параметров сжатия для проксированных запросов
		vector <string> gproxied = split(ini.Get("gzip", "proxied", ""), "|");
		// Если версии не указаны тогда устанавливаем значение по умолчанию
		if(gproxied.empty()) gproxied = GZIP_PROXIED;
		// Массив параметров сжатия для типов данных
		vector <string> gtypes = split(ini.Get("gzip", "types", ""), "|");
		// Если версии не указаны тогда устанавливаем значение по умолчанию
		if(gtypes.empty()) gtypes = GZIP_TYPES;
		// Заполняем структуру proxy
		this->proxy = {
			// Версия внутреннего протокола интернета (4, 6)
			proxy_intIPv,
			// Версия внешнего протокола интернета (4, 6)
			proxy_extIPv,
			// Тип прокси сервера (http, socks5, redirect)
			proxy_type,
			// Устанавливаем порт прокси сервера
			(u_int) ini.GetInteger("proxy", "port", proxy_port),
			// Максимальное количество воркеров
			(u_int) ini.GetInteger("proxy", "workers", MAX_WORKERS),
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
			// Оптимизировать настройки операционной системы (нужен root доступ)
			ini.GetBoolean("proxy", "optimos", PROXY_OPTIMOS),
			// Идентификатор группы пользователя под которым запускается прокси
			ini.Get("proxy", "user", PROXY_USER),
			// Идентификатор пользователя под которым запускается прокси
			ini.Get("proxy", "group", PROXY_GROUP),
			// Название сервиса
			ini.Get("proxy", "name", PROXY_NAME),
			// Адрес хранения pid файла
			ini.Get("proxy", "piddir", PID_DIR),
			// Адрес хранения конфигурационных файлов
			ini.Get("proxy", "confdir", CONFIG_DIR)
		};
		// Заполняем структуру gzip
		this->gzip = {
			// Разрешает или запрещает выдавать в ответе поле заголовка “Vary: Accept-Encoding”
			ini.GetBoolean("gzip", "vary", GZIP_VARY),
			// Тип сжатия (default - по умолчанию, best - лучшее сжатие, speed - лучшая скорость, no - без сжатия)
			glevel,
			// Минимальная длина данных после которых включается сжатие (работает только с Content-Length)
			ini.GetInteger("gzip", "length", GZIP_LENGTH),
			// Максимальный размер чанка в байтах
			getBytes(ini.Get("gzip", "chunk", GZIP_CHUNK)),
			// Не сжимать контент, UserAgent которого соответсвует регулярному выражению
			ini.Get("gzip", "regex", GZIP_REGEX),
			// Версия http протокола
			gvhttp,
			// Разрешает или запрещает сжатие ответа методом gzip для проксированных запросов
			gproxied,
			// Разрешает сжатие ответа методом gzip для указанных MIME-типов
			gtypes
		};
		// Заполняем структуру cache
		this->cache = {
			// Кеширование dns запросов
			ini.GetBoolean("cache", "dns", CACHE_DNS),
			// Кеширование часто-запрашиваемых страниц
			ini.GetBoolean("cache", "response", CACHE_RESPONSE),
			// Каталог хранения кеш файлов
			ini.Get("cache", "dir", CACHE_DIR)
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
			// Разрешить вывод логов в консоль
			ini.GetBoolean("logs", "console", LOGS_CONSOLE),
			// Разрешить ведение логов данных для обмена
			ini.GetBoolean("logs", "data", LOGS_DATA),
			// Разрешить ведение логов
			ini.GetBoolean("logs", "enabled", LOGS_ENABLED),
			// Размер файла лога
			getBytes(ini.Get("logs", "size", LOGS_SIZE)),
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
		// Контроль подключений клиента к серверу
		this->connects = {
			// Ключ по которому определяются подключения (ip = 0, mac = 1)
			connect_key,
			// Максимальное количество файловых дескрипторов
			(u_int) ini.GetInteger("connects", "fds", CONNECTS_FDS),
			// Максимальное количество подключений (одного клиента к прокси серверу)
			(u_int) ini.GetInteger("connects", "connect", CONNECTS_CONNECT),
			// Общее количество одновременных подключений к прокси серверу
			(int) ini.GetInteger("connects", "total", CONNECTS_TOTAL),
			// Максимальный размер скачиваемых данных в байтах
			getBytes(ini.Get("connects", "size", CONNECTS_SIZE))
		};
		// Заполняем структуру ipv4
		this->ipv4 = {
			// Внешний интерфейс, через который будут уходить запросы от сервера
			ini.Get("ipv4", "external", IPV4_EXTERNAL),
			// IP адрес интерфейса на котором будут приниматься запросы от клиентов
			ini.Get("ipv4", "internal", IPV4_INTERNAL),
			// Серверы DNS, используемые для преобразования имён вышестоящих серверов в адреса
			resolver4
		};
		// Заполняем структуру ipv6
		this->ipv6 = {
			// Внешний интерфейс, через который будут уходить запросы от сервера
			ini.Get("ipv6", "external", IPV6_EXTERNAL),
			// IP адрес интерфейса на котором будут приниматься запросы от клиентов
			ini.Get("ipv6", "internal", IPV6_INTERNAL),
			// Серверы DNS, используемые для преобразования имён вышестоящих серверов в адреса
			resolver6
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
			getSizeBuffer(ini.Get("speed", "input", BUFFER_READ_SIZE)),
			// Скорость исходящего подключения
			getSizeBuffer(ini.Get("speed", "output", BUFFER_WRITE_SIZE))
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