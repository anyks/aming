/* СЕРВЕР HTTP ПРОКСИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _HTTP_PROXY_ANYKS_
#define _HTTP_PROXY_ANYKS_

#include <iostream>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/resource.h>
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include "../lib/log/log.h"
#include "../lib/http/http.h"
#include "../lib/config/conf.h"

// Устанавливаем область видимости
using namespace std;

// Флаги базы данных событий
#define EVLOOP_ONCE				0x01
#define EVLOOP_NONBLOCK			0x02
#define EVLOOP_NO_EXIT_ON_EMPTY	0x04

// Флаги таймеров
#define TM_SERVER 0x01
#define TM_CLIENT 0x02

/**
 * Proxy Структура прокси сервера
 */
struct Proxy {
	LogApp * log;		// Объект ведения логов
	Config * config;	// Объект конфигурационных данных
} __attribute__((packed));
/**
 * Connects Класс подключений к прокси серверу
 */
class Connects {
	private:
		size_t			connects;	// Количество подключений
		pthread_mutex_t	mutex;		// Мютекс
		pthread_cond_t	condition;	// Переменная состояния
	public:
		/**
		 * get Метод получения количества подключений
		 * @return количество активных подключений
		 */
		inline size_t get();
		/**
		 * end Метод проверки на конец всех подключений
		 * @return проверка на достижения нуля
		 */
		inline bool end();
		/**
		 * inc Метод инкреминации количества подключений
		 */
		inline void inc();
		/**
		 * dec Метод декрементации количества подключений
		 */
		inline void dec();
		/**
		 * lock Метод блокировки мютекса
		 */
		inline void lock();
		/**
		 * unlock Метод разблокировки мютекса
		 */
		inline void unlock();
		/**
		 * signal Метод отправки сигнала первому блокированному потоку
		 */
		inline void signal();
		/**
		 * broadcastSignal Метод отправки сигналов всем блокированным потокам
		 */
		inline void broadcastSignal();
		/**
		 * wait Метод блокировки потока
		 */
		inline void wait();
		/**
		 * Connects Конструктор
		 */
		Connects();
		/**
		 * ~Connects Деструктор
		 */
		~Connects();
};
/**
 * BufferHttpProxy Класс для работы с данными прокси сервера
 */
class BufferHttpProxy {
	private:
		// Мютекс
		pthread_mutex_t	mutex;
		/**
		 * Events Буферы событий
		 */
		struct Events {
			struct bufferevent * client = NULL;	// Буфер событий клиента
			struct bufferevent * server = NULL;	// Буфер событий сервера
		} __attribute__((packed));
		/**
		 * Request Буфер данных
		 */
		struct Request {
			vector <char>	data;			// Данные в буфере
			size_t			offset	= 0;	// Смещение в буфере
		} __attribute__((packed));
		/**
		 * Server Данные текущего сервера
		 */
		struct Server {
			u_int	port	= 0;	// Порт
			string	host	= "";	// Хост адрес
			string	mac		= "";	// Мак адрес
		} __attribute__((packed));
		/**
		 * Sockets Сокеты клиента и сервера
		 */
		struct Sockets {
			evutil_socket_t client = -1;	// Сокет клиента
			evutil_socket_t server = -1;	// Сокет сервера
		} __attribute__((packed));
		/**
		 * Client Структура с информацией о подключении клиента
		 */
		struct Client {
			bool	connect		= false;	// Метод connect это или нет
			bool	alive		= false;	// Постоянное подключение или нет
			bool	https		= false;	// Защищенное подключение или нет
			string	ip			= "";		// ip адрес клиента
			string	mac			= "";		// Мак адрес клиента
			string	useragent	= "";		// userAgent клиента
		} __attribute__((packed));
		/**
		 * lock Метод блокировки мютекса
		 */
		inline void lock();
		/**
		 * unlock Метод разблокировки мютекса
		 */
		inline void unlock();
		/**
		 * appconn Функция которая добавляет или удаляет в список склиента
		 * @param flag флаг подключения или отключения клиента
		 */
		void appconn(bool flag);
		/**
		 * free_socket Метод отключения сокета
		 * @param fd ссылка на файловый дескриптор (сокет)
		 */
		void free_socket(evutil_socket_t * fd);
		/**
		 * free_event Метод удаления буфера события
		 * @param event указатель на объект буфера события
		 */
		void free_event(struct bufferevent ** event);
	public:
		bool					auth = false;	// Флаг авторизации
		struct event_base		* base;			// База событий
		map <string, Connects>	* connects; 	// Список подключений к прокси серверу
		Http					parser;			// Объект парсера
		HttpQuery				response;		// Ответ системы
		HttpData				httpData;		// Данные http запроса
		Sockets					sockets;		// Сокеты подключений
		Events					events;			// Буферы событий
		Server					server;			// Параметры удаленного сервера
		Client					client;			// Параметры подключившегося клиента
		Proxy					proxy;			// Параметры прокси сервера
		/**
		 * begin Метод активации подключения
		 */
		void begin();
		/**
		 * close_client Метод закрытия соединения клиента
		 */
		void close_client();
		/**
		 * close_server Метод закрытия соединения сервера
		 */
		void close_server();
		/**
		 * set_timeout Метод установки таймаутов
		 * @param type  тип подключения (клиент или сервер)
		 * @param read  таймаут на чтение
		 * @param write таймаут на запись
		 */
		void set_timeout(u_short type, bool read = false, bool write = false);
		/**
		 * BufferHttpProxy Конструктор
		 * @param string  name    имя ресурса
		 * @param u_short options параметры прокси сервера
		 */
		BufferHttpProxy(string name, u_short options);
		/**
		 * ~BufferHttpProxy Деструктор
		 */
		~BufferHttpProxy();
};
/**
 * HttpProxy Класс http прокси
 */
class HttpProxy {
	private:
		// Список подключений к прокси серверу
		map <string, Connects> connects;
		// Параметры прокси сервера
		Proxy server;
		// Слушатель порат
		struct evconnlistener * listener = NULL;
		/**
		 * getmac Метод определения мак адреса клиента
		 * @param  address структура параметров подключения
		 * @return данные мак адреса
		 */
		static string getmac(struct sockaddr * address);
		/**
		 * gethost Функция получения данных хоста
		 * @param  address структура параметров подключения
		 * @param  socklen размер структуры
		 * @return         данные полученного хоста
		 */
		static string gethost(struct sockaddr * address, int socklen);
		/**
		 * spawn_thread Функция создания треда
		 * @param  thread объект треда
		 * @param  ctx    передаваемый объект
		 * @return        результат работы функции
		 */
		static bool spawn_thread(pthread_t * thread, void * ctx);
		/**
		 * set_nonblock Функция установки неблокирующего сокета
		 * @param  fd   файловый дескриптор (сокет)
		 * @param  log  указатель на объект ведения логов
		 * @return      результат работы функции
		 */
		static int set_nonblock(evutil_socket_t fd, LogApp * log);
		/**
		 * set_non_block Функция отключения алгоритма Нейгла
		 * @param  fd   файловый дескриптор (сокет)
		 * @param  log  указатель на объект ведения логов
		 * @return      результат работы функции
		 */
		static int set_tcpnodelay(evutil_socket_t fd, LogApp * log);
		/**
		 * set_buffer_size Функция установки размеров буфера
		 * @param  fd         файловый дескриптор (сокет)
		 * @param  read_size  размер буфера на чтение
		 * @param  write_size размер буфера на запись
		 * @param  log        указатель на объект ведения логов
		 * @return            результат работы функции
		 */
		static int set_buffer_size(evutil_socket_t fd, int read_size, int write_size, LogApp * log);
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
		 * connection Функция обработки данных подключения в треде
		 * @param ctx передаваемый объект
		 */
		static void * connection(void * ctx);
		/**
		 * do_request Функция запроса данных у сервера
		 * @param bev  буфер события
		 * @param ctx  передаваемый объект
		 * @param flag флаг разрешающий новый запрос данных
		 */
		static void do_request(struct bufferevent * bev, void * ctx, bool flag = false);
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
		 * @param log    объект ведения логов
		 * @param config объект конфигурационных данных
		 */
		HttpProxy(LogApp * log = NULL, Config * config = NULL);
		/**
		 * HttpProxy Конструктор
		 */
		~HttpProxy();
};

#endif // _HTTP_PROXY_ANYKS_
