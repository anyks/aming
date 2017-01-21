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
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include "../lib/log/log.h"
#include "../lib/http/http.h"
#include "../lib/config/conf.h"
#include "../lib/dns/dns.h"

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
};
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
		 * Headers Заголовки http запросов
		 */
		struct Headers {
			HttpHeaders	request;	// Заголовки http запроса
			HttpHeaders	response;	// Заголовки http ответа
		};
		/**
		 * Events Буферы событий
		 */
		struct Events {
			struct bufferevent * client = NULL;	// Буфер событий клиента
			struct bufferevent * server = NULL;	// Буфер событий сервера
		};
		/**
		 * Request Буфер данных
		 */
		struct Request {
			vector <char>	data;			// Данные в буфере
			size_t			offset	= 0;	// Смещение в буфере
		};
		/**
		 * Server Данные текущего сервера
		 */
		struct Server {
			u_int	port	= 0;	// Порт
			string	host	= "";	// Хост адрес
			string	mac		= "";	// Мак адрес
		};
		/**
		 * Sockets Сокеты клиента и сервера
		 */
		struct Sockets {
			evutil_socket_t client = -1;	// Сокет клиента
			evutil_socket_t server = -1;	// Сокет сервера
		};
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
		};
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
		void appconn(const bool flag);
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
		bool					auth		= false;	// Флаг авторизации
		struct event_base		* base		= NULL;		// База событий
		map <string, Connects>	* connects	= NULL; 	// Список подключений к прокси серверу
		DNSResolver				* dns;					// Создаем объект dns ресолвера
		Http					parser;					// Объект парсера
		HttpQuery				response;				// Ответ системы
		HttpData				httpData;				// Данные http запроса
		Headers					headers;				// Данные http заголовков
		Sockets					sockets;				// Сокеты подключений
		Events					events;					// Буферы событий
		Server					server;					// Параметры удаленного сервера
		Client					client;					// Параметры подключившегося клиента
		Proxy					proxy;					// Параметры прокси сервера
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
		 * close Метод закрытия подключения
		 */
		void close();
		/**
		 * set_timeout Метод установки таймаутов
		 * @param type  тип подключения (клиент или сервер)
		 * @param read  таймаут на чтение
		 * @param write таймаут на запись
		 */
		void set_timeout(const u_short type, bool read = false, bool write = false);
		/**
		 * BufferHttpProxy Конструктор
		 * @param proxy объект данных прокси сервера
		 */
		BufferHttpProxy(Proxy proxy);
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
		// Создаем новую базу
		struct event_base * base = NULL;
		/**
		 * create_server Функция создания прокси сервера
		 * @return сокет прокси сервера
		 */
		evutil_socket_t create_server();
		/**
		 * HttpProxy::run_server Метод запуска прокси сервера
		 * @param socket сокет который слушает прокси сервер
		 */
		void run_server(evutil_socket_t socket);
		/**
		 * get_mac Метод определения мак адреса клиента
		 * @param  ctx указатель на объект подключения
		 * @return     данные мак адреса
		 */
		static string get_mac(void * ctx);
		/**
		 * get_ip Функция получения данных ip адреса
		 * @param  family тип интернет протокола
		 * @param  ctx    указатель на объект подключения
		 * @return        данные ip адреса
		 */
		static string get_ip(int family, void * ctx);
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
		 * set_reuseaddr Функция разрешающая повторно использовать сокет после его удаления
		 * @param  fd   файловый дескриптор (сокет)
		 * @param  log  указатель на объект ведения логов
		 * @return      результат работы функции
		 */
		static int set_reuseaddr(evutil_socket_t fd, LogApp * log);
		/**
		 * set_keepalive Функция устанавливает постоянное подключение на сокет
		 * @param  fd      файловый дескриптор (сокет)
		 * @param  log     указатель на объект ведения логов
		 * @param  cnt     максимальное количество попыток
		 * @param  idle    время через которое происходит проверка подключения
		 * @param  intvl   время между попытками
		 * @return         результат работы функции
		 */
		static int set_keepalive(evutil_socket_t fd, LogApp * log, int cnt, int idle, int intvl);
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
		 * @param ctx  передаваемый объект
		 * @param flag флаг разрешающий новый запрос данных
		 */
		static void do_request(void * ctx, bool flag = false);
		/**
		 * resolve_cb Функция выполняющая ресолвинг домена
		 * @param ip  IP адрес сервера
		 * @param ctx передаваемый объект
		 */
		static void resolve_cb(const string ip, void * ctx);
		/**
		 * event_cb Функция обработка входящих событий
		 * @param bev    буфер события
		 * @param events произошедшее событие
		 * @param ctx    объект входящих данных
		 */
		static void event_cb(struct bufferevent * bev, short events, void * ctx);
		/**
		 * write_client_cb Функция записи данных в сокет клиента
		 * @param bev буфер события
		 * @param ctx передаваемый объект
		 */
		static void write_client_cb(struct bufferevent * bev, void * ctx);
		/**
		 * read_server_cb Функция чтения данных с сокета сервера
		 * @param bev буфер события
		 * @param ctx передаваемый объект
		 */
		static void read_server_cb(struct bufferevent * bev, void * ctx);
		/**
		 * read_client_cb Функция чтения данных с сокета клиента
		 * @param bev буфер события
		 * @param ctx передаваемый объект
		 */
		static void read_client_cb(struct bufferevent * bev, void * ctx);
		/**
		 * accept_cb Функция подключения к серверу
		 * @param fd    файловый дескриптор (сокет)
		 * @param event событие на которое сработала функция обратного вызова
		 * @param ctx   объект передаваемый как значение
		 */
		static void accept_cb(evutil_socket_t fd, short event, void * ctx);
	public:
		/**
		 * HttpProxy Конструктор
		 * @param log    объект ведения логов
		 * @param config объект конфигурационных данных
		 */
		HttpProxy(LogApp * log = NULL, Config * config = NULL);
};

#endif // _HTTP_PROXY_ANYKS_
