#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <sys/types.h>
#include "base64.h"

// Устанавливаем пространство имен
using namespace std;

// Класс содержит данные парсинга http запроса
class Http {
	private:
		// Структура подключения
		struct connect {
			string	host,		// Хост
					port,		// Порт
					protocol;	// Протокол
		};
		// Основные переменные
		string	appname,		// Название прокси сервера
				appver,			// Версия системы
				query,			// Исходный запрос
				command,		// Команда запроса
				method,			// Метод запроса
				host,			// Хост запроса
				port,			// Порт запроса
				path,			// Путь запроса
				protocol,		// Протокол запроса
				version,		// Версия протокола
				connection,		// Заголовок connection
				pconnection,	// Заголовок proxy-connection
				useragent,		// UserAgent браузера
				auth,			// Тип авторизации
				login,			// Логин
				password,		// Пароль
				entitybody,		// Данные запроса для POST метода
				head;			// Результирующий заголовок для запроса
		// Массив остальных заголовков которые для нас не имеют значения
		vector <string> other;
		// Массив заголовков
		vector <string> headers;
		// Шаблоны ответов
		string html[12] = {
			// Подключение разрешено [0]
			"HTTP/1.0 200 Connection established\r\n"
			"Proxy-agent: ProxyAnyks/1.0\r\n\r\n",
			// Продолжить подключение [1]
			"HTTP/1.1 100 Continue\r\n\r\n",
			// Требуется авторизация в прокси [2]
			"HTTP/1.0 407 Proxy Authentication Required\r\n"
			"Proxy-agent: ProxyAnyks/1.0\r\n"
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
			"Proxy-agent: ProxyAnyks/1.0\r\n"
			"Proxy-Connection: close\r\n"
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n"
			"<html><head><title>400 Bad Request</title></head>\r\n"
			"<body><h2>400 Bad Request</h2></body></html>\r\n",
			// Страница не найдена [4]
			"HTTP/1.0 404 Not Found\r\n"
			"Proxy-agent: ProxyAnyks/1.0\r\n"
			"Proxy-Connection: close\r\n"
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n"
			"<html><head><title>404 Not Found</title></head>\r\n"
			"<body><h2>404 Not Found</h2><h3>File not found</body></html>\r\n",
			// Доступ закрыт [5]
			"HTTP/1.0 403 Forbidden\r\n"
			"Proxy-agent: ProxyAnyks/1.0\r\n"
			"Proxy-Connection: close\r\n"
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n"
			"<html><head><title>403 Access Denied</title></head>\r\n"
			"<body><h2>403 Access Denied</h2><h3>Access control list denies you to access this resource</body></html>\r\n",
			// Шлюз не доступен (хост не найден или ошибка подключения) [6]
			"HTTP/1.0 502 Bad Gateway\r\n"
			"Proxy-agent: ProxyAnyks/1.0\r\n"
			"Proxy-Connection: close\r\n"
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n"
			"<html><head><title>502 Bad Gateway</title></head>\r\n"
			"<body><h2>502 Bad Gateway</h2><h3>Host Not Found or connection failed</h3></body></html>\r\n",
			// Сервис не доступен (вы исчерпали свой трафик) [7]
			"HTTP/1.0 503 Service Unavailable\r\n"
			"Proxy-agent: ProxyAnyks/1.0\r\n"
			"Proxy-Connection: close\r\n"
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n"
			"<html><head><title>503 Service Unavailable</title></head>\r\n"
			"<body><h2>503 Service Unavailable</h2><h3>You have exceeded your traffic limit</h3></body></html>\r\n",
			// Сервис не доступен (обнаружена рекурсия) [8]
			"HTTP/1.0 503 Service Unavailable\r\n"
			"Proxy-agent: ProxyAnyks/1.0\r\n"
			"Proxy-Connection: close\r\n"
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n"
			"<html><head><title>503 Service Unavailable</title></head>\r\n"
			"<body><h2>503 Service Unavailable</h2><h3>Recursion detected</h3></body></html>\r\n",
			// Сервис не доступен (Требуемое действие не поддерживается прокси-сервером) [9]
			"HTTP/1.0 501 Not Implemented\r\n"
			"Proxy-agent: ProxyAnyks/1.0\r\n"
			"Proxy-Connection: close\r\n"
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n"
			"<html><head><title>501 Not Implemented</title></head>\r\n"
			"<body><h2>501 Not Implemented</h2><h3>Required action is not supported by proxy server</h3></body></html>\r\n",
			// Сервис не доступен (Не удалось подключится к родительской прокси) [10]
			"HTTP/1.0 502 Bad Gateway\r\n"
			"Proxy-agent: ProxyAnyks/1.0\r\n"
			"Proxy-Connection: close\r\n"
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n"
			"<html><head><title>502 Bad Gateway</title></head>\r\n"
			"<body><h2>502 Bad Gateway</h2><h3>Failed to connect parent proxy</h3></body></html>\r\n",
			// Внутренняя ошибка [11]
			"HTTP/1.0 500 Internal Error\r\n"
			"Proxy-agent: ProxyAnyks/1.0\r\n"
			"Proxy-Connection: close\r\n"
			"Content-type: text/html; charset=utf-8\r\n"
			"\r\n"
			"<html><head><title>500 Internal Error</title></head>\r\n"
			"<body><h2>500 Internal Error</h2><h3>Internal proxy error during processing your request</h3></body></html>\r\n"
		};
		/**
		 * Http::split Функция разбиения строки на указанные составляющие
		 * @param str   строка которую нужно разбить на составляющие
		 * @param delim разделитель в строке
		 * @return      массив строк полученных при разделении
		 */
		vector <string> split(const string str, const string delim);
		/**
		 * toCase Функция перевода в указанный регистр
		 * @param  str  строка для перевода в указанных регистр
		 * @param  flag флаг указания типа регистра
		 * @return      результирующая строка
		 */
		string toCase(string str, bool flag = false);
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
		 * getHeaderParam Функция получения содержимое заголовка
		 * @param  head  заголовок в котором ищем параметры
		 * @param  param параметр для поиска
		 * @return       найденный параметр
		 */
		string getHeaderParam(string head, string param);
		/**
		 * findHeaderParam Функция поиска значение заголовка
		 * @param  str строка поиска
		 * @param  buf буфер в котором осуществляется поиск
		 * @return     выводим значение заголовка
		 */
		string findHeaderParam(string str, string buf);
		/**
		 * getConnection Функция извлечения данных подключения
		 * @param  str строка запроса
		 * @return     объект с данными запроса
		 */
		connect getConnection(string str);
		/**
		 * parser Функция парсера http запроса
		 * @param buffer буфер входящих данных полученных из сокета
		 */
		void parser(string buffer);
		/**
		 * createHead Функция получения сформированного заголовка запроса
		 */
		void createHead();
	public:
		/**
		 * isAlive Метод определения нужно ли держать соединение для прокси
		 * @return результат проверки
		 */
		bool isAlive();
		/**
		 * isHttp Метод проверки на то http это или нет
		 * @param  buffer буфер входящих данных
		 * @return        результат проверки
		 */
		bool isHttp(const string buffer);
		/**
		 * parse Метод выполнения парсинга
		 * @param  buffer буфер входящих данных из сокета
		 * @return        результат определения завершения запроса
		 */
		bool parse(const string buffer);
		/**
		 * brokenRequest Метод получения ответа (неудачного отправленного запроса)
		 * @return ответ в формате html
		 */
		string brokenRequest();
		/**
		 * faultConnect Метод получения ответа (неудачного подключения к удаленному серверу)
		 * @return ответ в формате html
		 */
		string faultConnect();
		/**
		 * faultAuth Метод получения ответа (неудачной авторизации)
		 * @return ответ в формате html
		 */
		string faultAuth();
		/**
		 * requiredAuth Метод получения ответа (запроса ввода логина и пароля)
		 * @return ответ в формате html
		 */
		string requiredAuth();
		/**
		 * authSuccess Метод получения ответа (подтверждения авторизации)
		 * @return ответ в формате html
		 */
		string authSuccess();
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
		 * getQuery Метод получения сформированного http запроса
		 * @return сформированный http запрос
		 */
		string getQuery();
		/**
		 * getPort Метод получения порта запроса
		 * @return порт удаленного ресурса
		 */
		int getPort();
		/**
		 * getVersion Метод получения версии протокола запроса
		 * @return версия протокола запроса
		 */
		float getVersion();
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
		 * @param str строка с данными для установки
		 */
		void setPort(const string str);
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
		 * @param str строка с данными для установки
		 */
		void setVersion(const string str);
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
		 * Http Конструктор
		 * @param str строка содержащая название сервиса
		 */
		Http(const string str = "ANYKS", string ver = "1.0");
		/**
		 * Http Деструктор
		 */
		~Http();
};