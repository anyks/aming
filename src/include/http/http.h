/* HTTP ПАРСЕР ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _HTTP_PARSER_ANYKS_
#define _HTTP_PARSER_ANYKS_

#include <regex>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <zlib.h>
#include <stdlib.h>
#include <sys/types.h>
#include "base64/base64.h"
#include "config/conf.h"
#include "general/general.h"

// Параметры сжатия gzip
#define MOD_GZIP_ZLIB_WINDOWSIZE	15
#define MOD_GZIP_ZLIB_CFACTOR		9
#define MOD_GZIP_ZLIB_BSIZE			8096
#define MOD_GZIP_ZLIB_CHUNK			1024

// Параметры парсера по умолчанию
#define PARSER_OPTIONS_DEFAULT OPT_AGENT | OPT_GZIP | OPT_KEEPALIVE | OPT_LOG

// Устанавливаем пространство имен
using namespace std;

/**
 * HttpData Класс http данных
 */
class HttpData {
	private:
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
				};
				// Заполненность данных
				bool end = false;
				// Сырые данные
				vector <u_char> raw;
				// Заголовки http запроса
				vector <Header> headers;
			public:
				/**
				 * getHeaders Метод получения заголовков
				 * @return сформированные заголовки
				 */
				const string getHeaders();
				/**
				 * getHeader Метод извлекает данные заголовка по его ключу
				 * @param  key ключ заголовка
				 * @return     строка с данными заголовка
				 */
				Header getHeader(const string key);
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
				 * clear Метод очистки данных
				 */
				void clear();
				/**
				 * remove Метод удаления заголовка по ключу
				 * @param key ключ заголовка
				 */
				void remove(const string key);
				/**
				 * append Метод добавления заголовка
				 * @param key ключ
				 * @param val значение
				 */
				void append(const string key, const string val);
				/**
				 * setEnd Метод установки завершения передачи данных
				 */
				void setEnd();
				/**
				 * create Метод создания объекта http заголовков
				 * @param buffer буфер с текстовыми данными
				 */
				const bool create(const char * buffer);
				/**
				 * isEnd Метод проверки завершения формирования заголовков
				 * @return результат проверки
				 */
				const bool isEnd();
				/**
				 * empty Метод определяет наличие данных
				 * @return проверка о наличи данных
				 */
				const bool empty();
				/**
				 * size Метод получения размеров сырых данных
				 * @return размер сырых данных
				 */
				const size_t size();
				/**
				 * getHeadersSize Метод получения размера
				 * @return данные размера
				 */
				const size_t getHeadersSize();
				/**
				 * cbegin Метод получения начального итератора
				 * @return начальный итератор
				 */
				vector <Header>::const_iterator cbegin() const noexcept;
				/**
				 * cend Метод получения конечного итератора
				 * @return конечный итератор
				 */
				vector <Header>::const_iterator cend() const noexcept;
				/**
				 * ~HttpHeaders Деструктор
				 */
				~HttpHeaders();
		};
		/**
		 * HttpBody Класс тела запроса
		 */
		class HttpBody {
			private:
				/**
				 * Chunk Структура чанков
				 */
				struct Chunk {
					// Размер чанка в 16-й системе
					string hsize = "0";
					// Данные чанка
					string content;
					/**
					 * operator = Оператор присваивания
					 * @param chunk сторонний объект чанка
					 * @return      указатель на текущий объект
					 */
					// Chunk & operator = (Chunk chunk);
					/**
					 * init Метод инициализации чанка
					 * @param data данные для присваивания
					 * @param size размер данных
					 */
					void init(const char * data, const size_t size);
					/**
					 * Chunk Конструктор
					 * @param data данные для присваивания
					 * @param size размер данных
					 */
					Chunk(const char * data = NULL, const size_t size = 0);
				};
				// Уровень сжатия
				u_int levelGzip;
				// Максимальный размер чанков в байтах
				size_t chunkSize = 0;
				// Заполненность данных
				bool end = false;
				// Активация режима внутреннего сжатия
				bool intGzip = false;
				// Активация режима внешнего сжатия
				bool extGzip = false;
				// Данные тела
				string body;
				// Данные тела в чистом виде
				string rody;
				// Сырые данные
				vector <u_char> raw;
				// Массив чанков
				vector <Chunk> chunks;
				/**
				 * compress_gzip Метод сжатия данных методом GZIP
				 * @param  str   строка для сжатия данных
				 * @return       результат сжатия
				 */
				const string compress_gzip(const string &str);
				/**
				 * decompress_gzip Метод рассжатия данных методом GZIP
				 * @param  str   строка для расжатия данных
				 * @return       результат расжатия
				 */
				const string decompress_gzip(const string &str);
				/**
				 * compressData Метод сжатия данных
				 * @param  buffer буфер с данными
				 * @param  size   размер передаваемых данных
				 * @return        данные сжатого чанка
				 */
				const string compressData(const char * buffer, const size_t size);
				/**
				 * createChunk Метод создания чанка
				 * @param buffer буфер с данными
				 * @param size   размер передаваемых данных
				 */
				void createChunk(const char * buffer, const size_t size);
			public:
				/**
				 * clear Метод сброса параметров
				 */
				void clear();
				/**
				 * setChunkSize Метод установки размера чанков
				 * @param size размер чанков в байтах
				 */
				void setChunkSize(const size_t size);
				/**
				 * setLevelGzip Метод установки уровня сжатия
				 * @param levelGzip уровень сжатия
				 */
				void setLevelGzip(const u_int levelGzip);
				/**
				 * setEnd Метод установки завершения передачи данных
				 * (активируется при отключении сервера от прокси, все это нужно для протокола HTTP1.0 при Connection = close)
				 */
				void setEnd();
				/**
				 * set Метод установки сырых данных
				 * @param data сырые данные
				 * @param size размер сырых данных
				 */
				void set(const u_char * data, size_t size);
				/**
				 * isEnd Метод проверки завершения формирования тела
				 * @return результат проверки
				 */
				const bool isEnd();
				/**
				 * isIntCompress Метод проверки на активацию внутреннего сжатия
				 * @return результат проверки
				 */
				const bool isIntCompress();
				/**
				 * isExtCompress Метод проверки на активацию внешнего сжатия
				 * @return результат проверки
				 */
				const bool isExtCompress();
				/**
				 * getLevelGzip Метод получения уровня сжатия
				 */
				const u_int getLevelGzip();
				/**
				 * getChunkSize Метод получения размера чанков
				 */
				const size_t getChunkSize();
				/**
				 * getBodySize Метод определения размера данных
				 * @param  chunked чанкованием
				 * @return         размер тела
				 */
				const size_t getBodySize(const bool chunked = false);
				/**
				 * size Метод получения размеров сырых данных
				 * @return размер сырых данных
				 */
				const size_t size();
				/**
				 * addData Метод добавления данных тела
				 * @param  buffer буфер с данными
				 * @param  size   размер передаваемых данных
				 * @param  length тип данных (0 - по умолчанию, 1 - чанки, все остальные - по размеру)
				 * @param  strict жесткие правила проверки (при установки данного флага, данные принимаются только в точном соответствии)
				 * @return        количество обработанных байт
				 */
				const size_t addData(const char * buffer, const size_t size, const size_t length = 0, const bool strict = false);
				/**
				 * data Метод получения сырых данных
				 * @return сырые данные
				 */
				const u_char * data();
				/**
				 * getBody Метод получения тела запроса
				 * @param  chunked чанкованием
				 * @return         данные тела запроса
				 */
				const string getBody(const bool chunked = false);
				/**
				 * getRawBody Метод получения тела данных в чистом виде
				 * @return данные тела запроса
				 */
				const string getRawBody();
				/**
				 * getChunks Метод получения списка чанков
				 */
				vector <Chunk> getChunks();
				/**
				 * HttpBody Конструктор
				 * @param chunkSize  максимальный размер каждого чанка (в байтах)
				 * @param levelGzip  уровень сжатия
				 * @param intGzip    активация режима внутреннего сжатия
				 * @param extGzip    активация режима внешнего сжатия
				 */
				HttpBody(const size_t chunkSize = 1024, const u_int levelGzip = Z_DEFAULT_COMPRESSION, const bool intGzip = false, const bool extGzip = false);
				/**
				 * ~HttpBody Деструктор
				 */
				~HttpBody();
		};
	private:
		/**
		 * Dump Структура дампа
		 */
		struct Dump {
			size_t status;		// Статус код http запроса
			size_t options;		// Параметры прокси сервера
			size_t http;		// http запрос
			size_t auth;		// Тип авторизации
			size_t path;		// Путь запроса
			size_t host;		// Хост запроса
			size_t port;		// Порт запроса
			size_t login;		// Логин
			size_t method;		// Метод запроса
			size_t appName;		// Название приложения
			size_t version;		// Версия протокола
			size_t protocol;	// Протокол запроса
			size_t password;	// Пароль
			size_t appVersion;	// Версия приложения
			size_t headers;		// Дамп заголовков
			size_t body;		// Дамп тела
		};
		/**
		 * Http Структура http данных
		 */
		struct Http {
			string text;	// Текст запроса
			string headers;	// Заголовки
			string body;	// Тело
		};
		/**
		 * Connect Структура подключения
		 */
		struct Connect {
			string host;		// Хост
			string port;		// Порт
			string path;		// Путь
			string protocol;	// Протокол
		};
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
		// Параметры сжатия
		bool intGzip;	// Активация внутреннего режима сжатия
		bool extGzip;	// Активация внешнего режима сжатия
		// Статус код http запроса
		u_int status;
		// Параметры прокси сервера
		u_short options;
		// Сырые данные
		vector <u_char> raw;
		// Данные запроса
		string http;		// http запрос
		string auth;		// Тип авторизации
		string path;		// Путь запроса
		string host;		// Хост запроса
		string port;		// Порт запроса
		string login;		// Логин
		string method;		// Метод запроса
		string version;		// Версия протокола
		string appName;		// Название приложения
		string protocol;	// Протокол запроса
		string password;	// Пароль
		string appVersion;	// Версия приложения
		// Тело http запроса
		HttpBody body;
		// Заголовки http запроса
		HttpHeaders headers;
		// Параметры GZip
		Gzip * gzipParams = NULL;
		// Шаблоны ответов
		unordered_map <u_short, Http> response;
		/**
		 * genDataConnect Метод генерации данных для подключения
		 */
		void genDataConnect();
		/**
		 * createRequest Функция создания ответа сервера
		 * @param code код ответа
		 */
		void createRequest(const u_short code);
		/**
		 * createHeadResponse Функция получения сформированного заголовков ответа
		 * @return собранные заголовки ответа
		 */
		const string createHeadResponse();
		/**
		 * createHeadRequest Функция получения сформированного заголовка запроса
		 * @return собранные заголовки ответа
		 */
		const string createHeadRequest();
		/**
		 * getConnection Функция извлечения данных подключения
		 * @param  str строка запроса
		 * @return     объект с данными запроса
		 */
		Connect getConnection(const string str);
	public:
		/**
		 * isIntGzip Метод проверки активации режима внутреннего сжатия
		 * @return результат проверки
		 */
		const bool isIntGzip();
		/**
		 * isExtGzip Метод проверки активации режима внешнего сжатия
		 * @return результат проверки
		 */
		const bool isExtGzip();
		/**
		 * isUpgrade Метод проверки желания сервера сменить протокол
		 * @return результат проверки
		 */
		const bool isUpgrade();
		/**
		 * isConnect Метод проверяет является ли метод, методом connect
		 * @return результат проверки на метод connect
		 */
		const bool isConnect();
		/**
		 * isClose Метод проверяет должно ли быть закрыто подключение
		 * @return результат проверки на закрытие подключения
		 */
		const bool isClose();
		/**
		 * isHttps Метод проверяет является ли подключение защищенным
		 * @return результат проверки на защищенное подключение
		 */
		const bool isHttps();
		/**
		 * isAlive Метод определения нужно ли держать соединение для прокси
		 * @return результат проверки
		 */
		const bool isAlive();
		/**
		 * isEmpty Если данные не созданы
		 * @return результат проверки
		 */
		const bool isEmpty();
		/**
		 * isEndHeaders Метод получения данных о заполненности заголовков
		 */
		const bool isEndHeaders();
		/**
		 * isEndBody Метод определения заполненности тела ответа данными
		 * @return результат проверки
		 */
		const bool isEndBody();
		/**
		 * compressIsAllowed Метод проверки активации режима сжатия данных на уровне прокси сервера
		 * @param  userAgent агент браузера если существует
		 * @return           результат проверки
		 */
		const bool compressIsAllowed(const string userAgent = "");
		/**
		 * rmHeaderInString Метод удаления заголовка из строки
		 * @param  header  заголовок
		 * @param  headers список заголовков
		 * @return         результат работы
		 */
		const bool rmHeaderInString(const string header, string &headers);
		/**
		 * addHeaderToString Метод добавления заголовка к строке заголовков
		 * @param  header  заголовок
		 * @param  value   значение заголовка
		 * @param  headers список заголовков
		 * @return         результат работы
		 */
		const bool addHeaderToString(const string header, const string value, string &headers);
		/**
		 * setRedirect Метод создания запроса редиректа
		 * @param response объект ответа
		 * @return         реузльтат установки редиректа
		 */
		const bool setRedirect(HttpData &response);
		/**
		 * parse Метод парсинга данных
		 * @param buffer  буфер с входящими запросами
		 * @param size    размер входящих данных
		 * @param name    название приложения
		 * @param options опции http парсера
		 * @return        размер обработанных байт
		 */
		const size_t parse(const char * buffer, const size_t size, const string name = APP_NAME, const u_short options = PARSER_OPTIONS_DEFAULT);
		/**
		 * getBodySize Метод получения размера тела http данных
		 * @return размер тела данных
		 */
		const size_t getBodySize();
		/**
		 * getRawBodySize Метод получения размера тела http данных в чистом виде
		 * @return размер тела данных
		 */
		const size_t getRawBodySize();
		/**
		 * setEntitybody Метод добавления данных вложения
		 * @param buffer буфер с данными вложения
		 * @param size   размер буфера
		 * @return       количество добавленных данных
		 */
		const size_t setEntitybody(const char * buffer, const size_t size);
		/**
		 * size Метод получения размеров дампа
		 * @return размер дампа
		 */
		const size_t size();
		/**
		 * getPort Метод получения порта запроса
		 * @return порт удаленного ресурса
		 */
		const u_int getPort();
		/**
		 * getStatus Метод получения статуса запроса
		 * @return статус запроса
		 */
		const u_int getStatus();
		/**
		 * getVersion Метод получения версии протокола запроса
		 * @return версия протокола запроса
		 */
		const float getVersion();
		/**
		 * getHttp Метод получения http запроса
		 * @return http запрос
		 */
		const string getHttp();
		/**
		 * getMethod Метод получения метода запроса
		 * @return метод запроса
		 */
		const string getMethod();
		/**
		 * getHost Метод получения хоста запроса
		 * @return хост запроса
		 */
		const string getHost();
		/**
		 * getPath Метод получения пути запроса
		 * @return путь запроса
		 */
		const string getPath();
		/**
		 * getProtocol Метод получения протокола запроса
		 * @return протокол запроса
		 */
		const string getProtocol();
		/**
		 * getAuth Метод получения метода авторизации запроса
		 * @return метод авторизации
		 */
		const string getAuth();
		/**
		 * getLogin Метод получения логина авторизации запроса
		 * @return логин авторизации
		 */
		const string getLogin();
		/**
		 * getPassword Метод получения пароля авторизации запроса
		 * @return пароль авторизации
		 */
		const string getPassword();
		/**
		 * getUseragent Метод получения юзерагента запроса
		 * @return юзерагент
		 */
		const string getUseragent();
		/**
		 * getHeader Метод извлекает данные заголовка по его ключу
		 * @param  key ключ заголовка
		 * @return     строка с данными заголовка
		 */
		const string getHeader(const string key);
		/**
		 * getResponseHeaders Метод получения заголовков http ответа
		 * @return сформированные заголовки ответа
		 */
		const string getResponseHeaders();
		/**
		 * getRequestHeaders Метод получения заголовков http запроса
		 * @return сформированные заголовки запроса
		 */
		const string getRequestHeaders();
		/**
		 * createHeadResponse Метод модификации заголовков в строке
		 * @param  str строка с данными заголовков
		 * @return     модифицированная строка заголовков
		 */
		const string modifyHeaderString(const string str);
		/**
		 * getBody Метод получения данных тела http
		 * @param  chunked метод чанкование
		 * @return         строка с данными тела
		 */
		const string getBody(const bool chunked = false);
		/**
		 * getResponseData Метод получения http данных ответа
		 * @param  chunked метод чанкование
		 * @return строка с данными ответа
		 */
		const string getResponseData(const bool chunked = false);
		/**
		 * getRequestData Метод получения http данных запроса
		 * @return строка с данными запроса
		 */
		const string getRequestData();
		/**
		 * getRawResponseData Метод получения http данных ответа в чистом виде
		 * @return строка с данными ответа
		 */
		const string getRawResponseData();
		/**
		 * getRawRequestData Метод получения http данных запроса в чистом виде
		 * @return строка с данными запроса
		 */
		const string getRawRequestData();
		/**
		 * data Метод создания дампа
		 * @return сформированный блок дампа
		 */
		const u_char * data();
		/**
		 * set Метод заливки дампа
		 * @param data сырые данные
		 * @param size размер сырых данных
		 */
		void set(const u_char * data, size_t size);
		/**
		 * clear Метод очистки структуры
		 */
		void clear();
		/**
		 * initBody Метод инициализации объекта тела
		 */
		void initBody();
		/**
		 * rmHeader Метод удаления заголовка
		 * @param key название заголовка
		 */
		void rmHeader(const string key);
		/**
		 * unsetGzip Метод снятия режима сжатия gzip
		 */
		void unsetGzip();
		/**
		 * setGzip Метод установки режима сжатия gzip
		 * @param intGzip активация внутреннего режима сжатия
		 * @param extGzip активация внешнего режима сжатия
		 */
		void setGzip(const bool intGzip = true, const bool extGzip = false);
		/**
		 * setBodyEnd Метод установки завершения сбора данных тела
		 */
		void setBodyEnd();
		/**
		 * setHeader Метод добавления нового заголовка
		 * @param key   ключ
		 * @param value значение
		 */
		void setHeader(const string key, const string value);
		/**
		 * setData Метод добавления данных
		 * @param  buffer  буфер с http данными
		 * @param  size    размер http данных
		 */
		void setData(const char * buffer, const size_t size);
		/**
		 * setGzipParams Метод установки параметров сжатия gzip
		 * @param params параметры сжатия
		 */
		void setGzipParams(void * params = NULL);
		/**
		 * setOptions Метод установки настроек прокси сервера
		 * @param options данные для установки
		 */
		void setOptions(const u_short options);
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
		void setPort(const u_int number);
		/**
		 * setStatus Метод установки статуса запроса
		 * @param number номер статуса для установки
		 */
		void setStatus(const u_int number);
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
		void setVersion(const float number);
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
		 * addHeader Метод добавления нового заголовка
		 * @param buffer буфер с данными заголовка
		 */
		void addHeader(const char * buffer = NULL);
		/**
		 * largeRequest Метод генерации ответа (слишком большой размер файла)
		 */
		void largeRequest();
		/**
		 * brokenRequest Метод генерации ответа (неудачного отправленного запроса)
		 */
		void brokenRequest();
		/**
		 * faultConnect Метод генерации ответа (неудачного подключения к удаленному серверу)
		 */
		void faultConnect();
		/**
		 * pageNotFound Метод генерации ответа (страница не найдена)
		 */
		void pageNotFound();
		/**
		 * faultAuth Метод генерации ответа (неудачной авторизации)
		 */
		void faultAuth();
		/**
		 * requiredAuth Метод генерации ответа (запроса ввода логина и пароля)
		 */
		void requiredAuth();
		/**
		 * authSuccess Метод генерации ответа (подтверждения авторизации)
		 */
		void authSuccess();
		/**
		 * create Метод создания объекта
		 * @param name    название приложения
		 * @param options опции http парсера
		 */
		void create(const string name, const u_short options);
		/**
		 * HttpData Конструктор
		 * @param name    название приложения
		 * @param options опции http парсера
		 */
		HttpData(const string name = APP_NAME, const u_short options = PARSER_OPTIONS_DEFAULT);
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
		};
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
		const bool isHttp(const string buffer);
		/**
		 * parse Функция извлечения данных из буфера
		 * @param buffer буфер с входящими запросами
		 * @param size   размер входящих данных
		 * @return       количество обработанных байтов
		 */
		const size_t parse(const char * buffer, const size_t size);
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
		 * create Метод создания объекта
		 * @param name    строка содержащая название прокси сервера
		 * @param options параметры прокси сервера
		 */
		void create(const string name, const u_short options);
		/**
		 * Http Конструктор
		 * @param name    строка содержащая название прокси сервера
		 * @param options параметры прокси сервера
		 */
		Http(const string name = APP_NAME, const u_short options = PARSER_OPTIONS_DEFAULT);
		/**
		 * ~Http Деструктор
		 */
		~Http();
};

#endif // _HTTP_PARSER_ANYKS_
