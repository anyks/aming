/* HTTP ПАРСЕР ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _HTTP_PARSER_ANYKS_
#define _HTTP_PARSER_ANYKS_

#include <map>
#include <regex>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <sys/types.h>
#include "../base64/base64.h"
#include "../config/conf.h"

// Устанавливаем пространство имен
using namespace std;

/**
 * HttpHeaders Класс http заголовков
 */
class HttpHeaders {
	private:
		/**
		 * Header Структура содержащая данные заголовка
		 */
		struct Header {
			string head;	// Заголовок
			string value;	// значение заголовка
		} __attribute__((packed));
		// Заголовки http запроса
		map <string, Header> headers;
		/**
		 * split Функция разделения строк на составляющие
		 * @param str   строка для поиска
		 * @param delim разделитель
		 * @param v     результирующий вектор
		 */
		void split(const string &str, const string delim, vector <string> &v);
	public:
		/**
		 * getHeader Функция извлекает данные заголовка по его ключу
		 * @param  key ключ заголовка
		 * @return     строка с данными заголовка
		 */
		HttpHeaders::Header getHeader(string key);
		/**
		 * clear Метод очистки данных
		 */
		void clear();
		/**
		 * create Метод создания объекта http заголовков
		 * @param buffer буфер с текстовыми данными
		 */
		bool create(const char * buffer);
		/**
		 * empty Метод определяет наличие данных
		 * @return проверка о наличи данных
		 */
		bool empty();
		/**
		 * size Метод получения размера
		 * @return данные размера
		 */
		size_t size();
		/**
		 * cbegin Метод получения начального итератора
		 * @return начальный итератор
		 */
		map <string, Header>::const_iterator cbegin() const noexcept;
		/**
		 * cend Метод получения конечного итератора
		 * @return конечный итератор
		 */
		map <string, Header>::const_iterator cend() const noexcept;
		/**
		 * ~HttpHeaders Деструктор
		 */
		~HttpHeaders();
};
/**
 * HttpQuery Класс данных для выполнения запросов на удаленном сервере
 */
class HttpQuery {
	private:
		// Результирующий вектор
		vector <char> result;
	public:
		// Код сообщения
		short code;
		// Смещение в буфере
		size_t offset = 0;
		/**
		 * clear Метод очистки данных
		 */
		void clear();
		/**
		 * data Метод получения данных запроса
		 * @return данные запроса
		 */
		const char * data();
		/**
		 * size Метод получения размера
		 * @return данные размера
		 */
		size_t size();
		/**
		 * empty Метод определяет наличие данных
		 * @return проверка о наличи данных
		 */
		bool empty();
		/**
		 * HttpQuery Конструктор
		 * @param code       код сообщения
		 * @param mess       данные сообщения
		 * @param entitybody вложенные данные
		 */
		HttpQuery(short code = 0, string mess = "", vector <char> entitybody = {});
		/**
		 * ~HttpQuery Деструктор
		 */
		~HttpQuery();
};

/**
 * HttpData Класс http данных
 */
class HttpData {
	private:
		/**
		 * Connect Структура подключения
		 */
		struct Connect {
			string host;		// Хост
			string port;		// Порт
			string protocol;	// Протокол
		} __attribute__((packed));
		// Основные переменные класса
		u_short			options;	// Параметры прокси сервера
		string			appName;	// Название приложения
		string			appVersion;	// Версия приложения
		string			query;		// Данные запроса
		string			http;		// http запрос
		string			auth;		// Тип авторизации
		string			method;		// Метод запроса
		string			path;		// Путь запроса
		string			protocol;	// Протокол запроса
		string			version;	// Версия протокола
		string			host;		// Хост запроса
		string			port;		// Порт запроса
		string			login;		// Логин
		string			password;	// Пароль
		string			useragent;	// UserAgent браузера
		string			connection;	// Заголовок connection
		string			request;	// Результирующий заголовок для запроса
		size_t			length = 0;	// Количество заголовков
		vector <char>	entitybody;	// Данные http вложений
		HttpHeaders		headers;	// Заголовки http запроса
		// Шаблоны ответов
		string html[12] = {
			// Подключение разрешено [0]
			"HTTP/1.0 200 Connection established\r\n"
			"Proxy-Agent: ProxyAnyks/1.0\r\n\r\n",
			// Продолжить подключение [1]
			"HTTP/1.1 100 Continue\r\n\r\n",
			// Требуется авторизация в прокси [2]
			"HTTP/1.0 407 Proxy Authentication Required\r\n"
			"Proxy-Agent: ProxyAnyks/1.0\r\n"
			"Proxy-Authenticate: Basic realm=\"proxy\"\r\n"
			"Proxy-Connection: close\r\n"
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n"
			"<html><head><title>407 Proxy Authentication Required</title></head>\r\n"
			"<body><h2>407 Proxy Authentication Required</h2>\r\n"
			"<h3>Access to requested resource disallowed by administrator or you need valid username/password to use this resource</h3>\r\n"
			"</body></html>\r\n",
			// Ошибка запроса [3]
			"HTTP/1.0 400 Bad Request\r\n"
			"Proxy-Agent: ProxyAnyks/1.0\r\n"
			"Proxy-Connection: close\r\n"
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n"
			"<html><head><title>400 Bad Request</title></head>\r\n"
			"<body><h2>400 Bad Request</h2></body></html>\r\n",
			// Страница не найдена [4]
			"HTTP/1.0 404 Not Found\r\n"
			"Proxy-Agent: ProxyAnyks/1.0\r\n"
			"Proxy-Connection: close\r\n"
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n"
			"<html><head><title>404 Not Found</title></head>\r\n"
			"<body><h2>404 Not Found</h2><h3>File not found</body></html>\r\n",
			// Доступ закрыт [5]
			"HTTP/1.0 403 Forbidden\r\n"
			"Proxy-Agent: ProxyAnyks/1.0\r\n"
			"Proxy-Connection: close\r\n"
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n"
			"<html><head><title>403 Access Denied</title></head>\r\n"
			"<body><h2>403 Access Denied</h2><h3>Access control list denies you to access this resource</body></html>\r\n",
			// Шлюз не доступен (хост не найден или ошибка подключения) [6]
			"HTTP/1.0 502 Bad Gateway\r\n"
			"Proxy-Agent: ProxyAnyks/1.0\r\n"
			"Proxy-Connection: close\r\n"
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n"
			"<html><head><title>502 Bad Gateway</title></head>\r\n"
			"<body><h2>502 Bad Gateway</h2><h3>Host Not Found or connection failed</h3></body></html>\r\n",
			// Сервис не доступен (вы исчерпали свой трафик) [7]
			"HTTP/1.0 503 Service Unavailable\r\n"
			"Proxy-Agent: ProxyAnyks/1.0\r\n"
			"Proxy-Connection: close\r\n"
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n"
			"<html><head><title>503 Service Unavailable</title></head>\r\n"
			"<body><h2>503 Service Unavailable</h2><h3>You have exceeded your traffic limit</h3></body></html>\r\n",
			// Сервис не доступен (обнаружена рекурсия) [8]
			"HTTP/1.0 503 Service Unavailable\r\n"
			"Proxy-Agent: ProxyAnyks/1.0\r\n"
			"Proxy-Connection: close\r\n"
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n"
			"<html><head><title>503 Service Unavailable</title></head>\r\n"
			"<body><h2>503 Service Unavailable</h2><h3>Recursion detected</h3></body></html>\r\n",
			// Сервис не доступен (Требуемое действие не поддерживается прокси сервером) [9]
			"HTTP/1.0 501 Not Implemented\r\n"
			"Proxy-Agent: ProxyAnyks/1.0\r\n"
			"Proxy-Connection: close\r\n"
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n"
			"<html><head><title>501 Not Implemented</title></head>\r\n"
			"<body><h2>501 Not Implemented</h2><h3>Required action is not supported by proxy server</h3></body></html>\r\n",
			// Сервис не доступен (Не удалось подключится к родительской прокси) [10]
			"HTTP/1.0 502 Bad Gateway\r\n"
			"Proxy-Agent: ProxyAnyks/1.0\r\n"
			"Proxy-Connection: close\r\n"
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n"
			"<html><head><title>502 Bad Gateway</title></head>\r\n"
			"<body><h2>502 Bad Gateway</h2><h3>Failed to connect parent proxy</h3></body></html>\r\n",
			// Внутренняя ошибка [11]
			"HTTP/1.0 500 Internal Error\r\n"
			"Proxy-Agent: ProxyAnyks/1.0\r\n"
			"Proxy-Connection: close\r\n"
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n"
			"<html><head><title>500 Internal Error</title></head>\r\n"
			"<body><h2>500 Internal Error</h2><h3>Internal proxy error during processing your request</h3></body></html>\r\n"
		};
		/**
		 * createHead Функция получения сформированного заголовка запроса
		 */
		void createHead();
		/**
		 * genDataConnect Метод генерации данных для подключения
		 */
		void genDataConnect();
		/**
		 * getConnection Функция извлечения данных подключения
		 * @param  str строка запроса
		 * @return     объект с данными запроса
		 */
		Connect getConnection(string str);
	public:
		/**
		 * clear Метод очистки структуры
		 */
		void clear();
		/**
		 * isConnect Метод проверяет является ли метод, методом connect
		 * @return результат проверки на метод connect
		 */
		bool isConnect();
		/**
		 * isClose Метод проверяет должно ли быть закрыто подключение
		 * @return результат проверки на закрытие подключения
		 */
		bool isClose();
		/**
		 * isHttps Метод проверяет является ли подключение защищенным
		 * @return результат проверки на защищенное подключение
		 */
		bool isHttps();
		/**
		 * isAlive Метод определения нужно ли держать соединение для прокси
		 * @return результат проверки
		 */
		bool isAlive();
		/**
		 * size Метод получения размера запроса
		 * @return размер запроса
		 */
		size_t size();
		/**
		 * getPort Метод получения порта запроса
		 * @return порт удаленного ресурса
		 */
		u_int getPort();
		/**
		 * getVersion Метод получения версии протокола запроса
		 * @return версия протокола запроса
		 */
		float getVersion();
		/**
		 * getHttp Метод получения http запроса
		 * @return http запрос
		 */
		string getHttp();
		/**
		 * getMethod Метод получения метода запроса
		 * @return метод запроса
		 */
		string getMethod();
		/**
		 * getHost Метод получения хоста запроса
		 * @return хост запроса
		 */
		string getHost();
		/**
		 * getPath Метод получения пути запроса
		 * @return путь запроса
		 */
		string getPath();
		/**
		 * getProtocol Метод получения протокола запроса
		 * @return протокол запроса
		 */
		string getProtocol();
		/**
		 * getAuth Метод получения метода авторизации запроса
		 * @return метод авторизации
		 */
		string getAuth();
		/**
		 * getLogin Метод получения логина авторизации запроса
		 * @return логин авторизации
		 */
		string getLogin();
		/**
		 * getPassword Метод получения пароля авторизации запроса
		 * @return пароль авторизации
		 */
		string getPassword();
		/**
		 * getUseragent Метод получения юзерагента запроса
		 * @return юзерагент
		 */
		string getUseragent();
		/**
		 * getQuery Метод получения буфера запроса
		 * @return буфер запроса
		 */
		string getQuery();
		/**
		 * setEntitybody Метод добавления данных вложения
		 * @param buffer буфер с данными вложения
		 * @param size   размер буфера
		 */
		bool setEntitybody(const char * buffer, size_t size);
		/**
		 * setMethod Метод установки метода запроса
		 * @param str строка с данными для установки
		 */
		void setMethod(const string str);
		/**
		 * setHost Метод установки хоста запроса
		 * @param str строка с данными для установки
		 */
		void setHost(const string str);
		/**
		 * setPort Метод установки порта запроса
		 * @param number номер порта для установки
		 */
		void setPort(u_int number);
		/**
		 * setPath Метод установки пути запроса
		 * @param str строка с данными для установки
		 */
		void setPath(const string str);
		/**
		 * setProtocol Метод установки протокола запроса
		 * @param str строка с данными для установки
		 */
		void setProtocol(const string str);
		/**
		 * setVersion Метод установки версии протокола запроса
		 * @param number номер версии протокола
		 */
		void setVersion(float number);
		/**
		 * setAuth Метод установки метода авторизации запроса
		 * @param str строка с данными для установки
		 */
		void setAuth(const string str);
		/**
		 * setUseragent Метод установки юзерагента запроса
		 * @param str строка с данными для установки
		 */
		void setUseragent(const string str);
		/**
		 * setClose Метод установки принудительного отключения после запроса
		 */
		void setClose();
		/**
		 * createRequest Функция создания ответа сервера
		 * @param  index   индекс в массиве ответа
		 * @param  request номер ответа
		 * @return         объект с данными ответа
		 */
		HttpQuery createRequest(u_short index, u_short request);
		/**
		 * brokenRequest Метод получения ответа (неудачного отправленного запроса)
		 * @return ответ в формате html
		 */
		HttpQuery brokenRequest();
		/**
		 * faultConnect Метод получения ответа (неудачного подключения к удаленному серверу)
		 * @return ответ в формате html
		 */
		HttpQuery faultConnect();
		/**
		 * pageNotFound Метод получения ответа (страница не найдена)
		 * @return ответ в формате html
		 */
		HttpQuery pageNotFound();
		/**
		 * faultAuth Метод получения ответа (неудачной авторизации)
		 * @return ответ в формате html
		 */
		HttpQuery faultAuth();
		/**
		 * requiredAuth Метод получения ответа (запроса ввода логина и пароля)
		 * @return ответ в формате html
		 */
		HttpQuery requiredAuth();
		/**
		 * authSuccess Метод получения ответа (подтверждения авторизации)
		 * @return ответ в формате html
		 */
		HttpQuery authSuccess();
		/**
		 * getRequest Метод получения сформированного http запроса
		 * @return сформированный http запрос
		 */
		HttpQuery getRequest();
		/**
		 * getHttpRequest Метод получения сформированного http запроса только с добавлением заголовков
		 * @return сформированный http запрос
		 */
		vector <char> getHttpRequest();
		/**
		 * init Метод инициализации класса
		 * @param  str     строка http запроса
		 * @param  name    название приложения
		 * @param  version версия приложения
		 * @param  options опции http парсера
		 * @return         данные http запроса
		 */
		void init(const string str, const string name, const string version, const u_short options);
		/**
		 * ~HttpData Деструктор
		 */
		~HttpData();
};

/**
 * Http Класс содержит данные парсинга http запроса
 */
class Http {
	private:
		/**
		 * HttpEnd Структура подержащая данные проверки, полной передачи данных
		 */
		struct HttpEnd {
			u_short	type = 0;
			size_t	begin = 0, end = 0;
		} __attribute__((packed));
		// Параметры прокси сервера
		u_short options;
		// Название и версия прокси сервера
		string name, version;
	public:
		// Массив объектов подключений
		vector <HttpData> httpData;
		/**
		 * isHttp Метод проверки на то http это или нет
		 * @param  buffer буфер входящих данных
		 * @return        результат проверки
		 */
		bool isHttp(const string buffer);
		/**
		 * parse Функция извлечения данных из буфера
		 * @param buffer буфер с входящими запросами
		 * @param size   размер входящих данных
		 * @param flag   обрабатывать весь блок данных
		 */
		size_t parse(const char * buffer, size_t size, bool flag = false);
		/**
		 * modify Функция модифицирования ответных данных
		 * @param data ссылка на данные полученные от сервера
		 */
		void modify(vector <char> &data);
		/**
		 * Http::clear Метод очистки всех полученных данных
		 */
		void clear();
		/**
		 * Http Конструктор
		 * @param name    строка содержащая название прокси сервера
		 * @param options параметры прокси сервера
		 */
		Http(const string name = APP_NAME, u_short options = (OPT_AGENT | OPT_GZIP | OPT_KEEPALIVE | OPT_LOG));
		/**
		 * ~Http Деструктор
		 */
		~Http();
};

#endif // _HTTP_PARSER_ANYKS_
