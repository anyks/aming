/* СЕРВЕР HTTP ПРОКСИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2016
*/
#include <iostream>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/resource.h>
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include "../lib/http.h"

// Устанавливаем область видимости
using namespace std;

// Флаги базы данных событий
#define EVLOOP_ONCE				0x01
#define EVLOOP_NONBLOCK			0x02
#define EVLOOP_NO_EXIT_ON_EMPTY	0x04

// Название и версия прокси-сервера
#define VERSION "1.0"

// Максимальное количество клиентов
#define MAX_CLIENTS -1 // 1024
// Максимальный размер буфера
#define BUFFER_WRITE_SIZE -1 // 2048
// Максимальный размер буфера для чтения http данных
#define BUFFER_READ_SIZE -1 // 4096
// Порт сервера
#define SERVER_PORT 5555
// Таймаут времени на чтение
#define READ_TIMEOUT 10
// Таймаут времени на запись
#define WRITE_TIMEOUT 8
// Таймаут ожидания коннекта
#define KEEP_ALIVE_TIMEOUT 5

/**
 * BufferHttpProxy Класс для работы с данными прокси сервера
 */
class BufferHttpProxy {
	private:
		// Буферы событий
		struct Events {
			struct bufferevent * client = NULL;	// Буфер событий клиента
			struct bufferevent * server = NULL; // Буфер событий сервера
		};
		// Буфер данных
		struct Request {
			vector <char>	data;			// Данные в буфере
			size_t			offset	= 0;	// Смещение в буфере
		};
		// Данные текущего сервера
		struct Server {
			u_int	port;	// Порт сервера
			string	host;	// Хост сервера
		};
	public:
		bool				auth = false;		// Флаг авторизации
		u_short				read_timeout;		// Таймаут времени на чтение
		u_short				write_timeout;		// Таймаут времени на запись
		u_short				keepalive_timeout;	// Таймаут ожидания коннекта
		u_short				options;			// Параметры прокси-сервера
		struct event_base	* base;				// База событий
		Http				* parser;			// Объект парсера
		Http::HttpQuery		response;			// Ответ системы
		Request				request;			// Данные запроса
		Events				events;				// Буферы событий
		Server				server;				// Параметры удаленного сервера
		// Размеры буферов на чтение и запись
		int buffer_read_size, buffer_write_size;
		/**
		 * free_client Метод удаления буфера клиента
		 */
		void free_client(){
			// Удаляем событие клиента
			if(events.client != NULL){
				// Удаляем буфер события
				bufferevent_free(events.client);
				// Устанавливаем что событие удалено
				events.client = NULL;
			}
		}
		/**
		 * free_server Метод удаления буфера сервера
		 */
		void free_server(){
			// Удаляем событие сервера
			if(events.server != NULL){
				// Удаляем буфер события
				bufferevent_free(events.server);
				// Устанавливаем что событие удалено
				events.server = NULL;
			}
		}
		/**
		 * BufferHttpProxy Конструктор
		 * @param string  name    имя ресурса
		 * @param u_short options параметры прокси-сервера
		 */
		BufferHttpProxy(string name, u_short options){
			// Создаем объект для работы с http заголовками
			parser = new Http(name, options, VERSION);
		}
		/**
		 * ~BufferHttpProxy Деструктор
		 */
		~BufferHttpProxy(){
			// Удаляем событие клиента
			free_client();
			// Удаляем событие сервера
			free_server();
			// Если парсер не удален
			if(parser != NULL){
				// Удаляем парсер
				delete parser;
				// Запоминаем что данные удалены
				parser = NULL;
			}
			// Очищаем память выделенную для вектора
			vector <char> ().swap(request.data);
		}
		/**
		 * parse Метод парсинга данных
		 * @return результат работы парсинга
		 */
		bool parse(){
			// Выполняем парсинг данных
			return parser->parse(request.data.data(), request.data.size() - 1);
		}
};
/**
 * HttpProxy Класс http прокси
 */
class HttpProxy {
	private:
		// Название прокси-сервера
		string name;
		// Параметры прокси-сервера
		u_short options;
		// Таймеры
		u_short read_timeout;		// Таймаут времени на чтение
		u_short write_timeout;		// Таймаут времени на запись
		u_short keepalive_timeout;	// Таймаут ожидания коннекта
		// Размеры буферов на чтение и запись
		int buffer_read_size, buffer_write_size;
		// Слушатель порат
		struct evconnlistener * listener = NULL;
		/**
		 * HttpProxy::get_host Функция получения данных хоста
		 * @param  address структура параметров подключения
		 * @param  socklen размер структуры
		 * @return         данные полученного хоста
		 */
		string get_host(struct sockaddr * address, int socklen);
		/**
		 * free_http Функция очистки объекта http
		 * @param arg объект для очистки
		 */
		static void free_http(BufferHttpProxy ** arg);
		/**
		 * set_non_block Функция отключения алгоритма Нейгла
		 * @param fd файловый дескриптор (сокет)
		 * @return   результат работы функции
		 */
		static int set_tcpnodelay(evutil_socket_t fd);
		/**
		 * append_to_buffer Функция добавления в буфер новых данных
		 * @param data       ссылка на буфер данных
		 * @param chunk_size размер одной порции данных
		 * @param buffer     буфер с входящими данными
		 */
		static void append_to_buffer(vector <char> &data, size_t chunk_size, const char * buffer);
		/**
		 * set_buffer_size Функция установки размеров буфера
		 * @param  fd         файловый дескриптор (сокет)
		 * @param  read_size  размер буфера на чтение
		 * @param  write_size размер буфера на запись
		 * @return            результат работы функции
		 */
		static int set_buffer_size(evutil_socket_t fd, int read_size, int write_size);
		/**
		 * check_auth Функция проверки логина и пароля
		 * @param ctx объект входящих данных
		 * @return    результат проверки подлинности
		 */
		static bool check_auth(void * ctx);
		/**
		 * connect_server Функция создания сокета для подключения к удаленному серверу
		 * @param ctx объект входящих данных
		 * @return    результат подключения
		 */
		static int connect_server(void * ctx);
		/**
		 * event Функция обработка входящих событий
		 * @param bev    буфер события
		 * @param events произошедшее событие
		 * @param ctx    объект входящих данных
		 */
		static void event(struct bufferevent * bev, short events, void * ctx);
		/**
		 * read_server Функция чтения данных с сокета сервера
		 * @param bev буфер события
		 * @param ctx передаваемый объект
		 */
		static void read_server(struct bufferevent * bev, void * ctx);
		/**
		 * read_client Функция чтения данных с сокета клиента
		 * @param bev буфер события
		 * @param ctx передаваемый объект
		 */
		static void read_client(struct bufferevent * bev, void * ctx);
		/**
		 * accept_error Событие возникновения ошибки подключения
		 * @param listener объект подключения
		 * @param ctx      передаваемый объект
		 */
		static void accept_error(struct evconnlistener * listener, void * ctx);
		/**
		 * accept_connect Событие подключения к серверу
		 * @param listener объект подключения
		 * @param fd       файловый дескриптор (сокет) клиента
		 * @param address  адрес клиента
		 * @param socklen  размер входящих данных
		 * @param ctx      передаваемый объект
		 */
		static void accept_connect(struct evconnlistener * listener, evutil_socket_t fd, struct sockaddr * address, int socklen, void * ctx);
	public:
		/**
		 * HttpProxy Конструктор
		 * @param name       название прокси-сервера
		 * @param host       хост прокси-сервера
		 * @param port       порт прокси-сервера
		 * @param buffrsize  размер буфера сокета на чтение
		 * @param buffwsize  размер буфера сокета на запись
		 * @param maxcls     максимальное количество подключаемых клиентов к прокси-серверу (-1 автоматически)
		 * @param rtm        таймаут на чтение данных из сокета сервера
		 * @param wtm        таймаут на запись данных из сокета клиента и сервера
		 * @param katm       таймаут на чтение данных из сокета клиента
		 * @param options    опции прокси-сервера
		 */
		HttpProxy(
			const char *	name		= "anyks",
			const char *	host		= "0.0.0.0",
			u_int			port		= SERVER_PORT,
			int				buffrsize	= BUFFER_READ_SIZE,
			int				buffwsize	= BUFFER_WRITE_SIZE,
			int				maxcls		= MAX_CLIENTS,
			u_short			rtm			= READ_TIMEOUT,
			u_short			wtm			= WRITE_TIMEOUT,
			u_short			katm		= KEEP_ALIVE_TIMEOUT,
			u_short			options		= OPT_CONNECT | OPT_AGENT | OPT_GZIP | OPT_KEEPALIVE | OPT_LOG | OPT_PGZIP
		);
		/**
		 * HttpProxy Конструктор
		 */
		~HttpProxy();
};