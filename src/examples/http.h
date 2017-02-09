/* СЕРВЕР HTTP ПРОКСИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/
#ifndef _HTTP_PROXY_ANYKS_
#define _HTTP_PROXY_ANYKS_

#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
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
#include "dns/dns.h"
#include "nwk/nwk.h"
#include "http/http.h"
#include "system/system.h"

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
 * Connects Класс подключений к прокси серверу
 */
class Connects {
	private:
		size_t					connects;	// Количество подключений
		condition_variable_any	condition;	// Переменная состояния
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
		inline void wait(recursive_mutex &mutx);
		/**
		 * Connects Конструктор
		 */
		Connects();
};
/**
 * ClientConnects Класс содержащий данные подключения одного клиента
 */
class ClientConnects {
	private:
		// Мютекс для блокировки потока
		mutex mtx;
		// Список подключений к прокси серверу
		map <string, unique_ptr <Connects>> connects;
	public:
		/**
		 * get Метод получения данных подключения клиента
		 * @param client идентификатор клиента
		 * @return       данные подключения клиента
		 */
		Connects * get(const string client);
		/**
		 * add Метод добавления данных подключения клиента
		 * @param client идентификатор клиента
		 */
		void add(const string client);
		/**
		 * rm Метод удаления данных подключения клиента
		 * @param client идентификатор клиента
		 */
		void rm(const string client);
};
/**
 * BufferHttpProxy Класс для работы с данными прокси сервера
 */
class BufferHttpProxy {
	private:
		// Мютекс для останова потока
		recursive_mutex lock_thread;
		// Мютекс для блокировки подключения
		recursive_mutex lock_connect;
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
		// Флаг авторизации
		bool auth = false;
		// Количество моих подключений к прокси
		u_int myconns = 1;
		// Объект парсера
		Http parser;
		// Параметры подключившегося клиента
		Client client;
		// Параметры удаленного сервера
		Server server;
		// Буферы событий
		Events events;
		// Сокеты подключений
		Sockets sockets;
		// Данные http заголовков
		HttpData headers;
		// Данные http запроса
		HttpData httpData;
		// Ответ системы
		HttpQuery response;
		// Параметры прокси сервера
		System * proxy = NULL;
		// Создаем объект dns ресолвера
		DNSResolver * dns = NULL;
		// База событий
		struct event_base * base = NULL;
		// Список подключений к прокси серверу
		ClientConnects * connects = NULL;
		/**
		 * lock Метод блокировки мютекса
		 */
		inline void lock();
		/**
		 * unlock Метод разблокировки мютекса
		 */
		inline void unlock();
		/**
		 * blockconnect Метод блокировки лишних коннектов
		 */
		void blockconnect();
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
		 * sleep Метод усыпления потока на время необходимое для соблюдения скоростного ограничения сети
		 * @param  size размер передаваемых данных
		 * @param  type тип передаваемого сообщения (true - чтение, false - запись)
		 * @return      время в секундах на которое следует задержать поток
		 */
		void sleep(size_t size, bool type);
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
		BufferHttpProxy(System * proxy);
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
		ClientConnects connects;
		// Идентификаторы процессов
		pid_t * pids = NULL;
		// Параметры прокси сервера
		System * server = NULL;
		// Создаем новую базу
		struct event_base * base = NULL;
		/**
		 * create_server Функция создания прокси сервера
		 * @return сокет прокси сервера
		 */
		evutil_socket_t create_server();
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
		 * socket_nosigpipe Функция установки отключения сигнала записи в оборванное подключение
		 * @param  fd   файловый дескриптор (сокет)
		 * @param  log  указатель на объект ведения логов
		 * @return      результат работы функции
		 */
		static int socket_nosigpipe(evutil_socket_t fd, LogApp * log);
		/**
		 * socket_nonblocking Функция установки неблокирующего сокета
		 * @param  fd   файловый дескриптор (сокет)
		 * @param  log  указатель на объект ведения логов
		 * @return      результат работы функции
		 */
		static int socket_nonblocking(evutil_socket_t fd, LogApp * log);
		/**
		 * socket_tcpcork Функция активации tcp_cork
		 * @param  fd   файловый дескриптор (сокет)
		 * @param  log  указатель на объект ведения логов
		 * @return      результат работы функции
		 */
		static int socket_tcpcork(evutil_socket_t fd, LogApp * log);
		/**
		 * socket_tcpnodelay Функция отключения алгоритма Нейгла
		 * @param  fd   файловый дескриптор (сокет)
		 * @param  log  указатель на объект ведения логов
		 * @return      результат работы функции
		 */
		static int socket_tcpnodelay(evutil_socket_t fd, LogApp * log);
		/**
		 * socket_reuseable Функция разрешающая повторно использовать сокет после его удаления
		 * @param  fd   файловый дескриптор (сокет)
		 * @param  log  указатель на объект ведения логов
		 * @return      результат работы функции
		 */
		static int socket_reuseable(evutil_socket_t fd, LogApp * log);
		/**
		 * socket_keepalive Функция устанавливает постоянное подключение на сокет
		 * @param  fd      файловый дескриптор (сокет)
		 * @param  log     указатель на объект ведения логов
		 * @param  cnt     максимальное количество попыток
		 * @param  idle    время через которое происходит проверка подключения
		 * @param  intvl   время между попытками
		 * @return         результат работы функции
		 */
		static int socket_keepalive(evutil_socket_t fd, LogApp * log, int cnt, int idle, int intvl);
		/**
		 * socket_buffersize Функция установки размеров буфера
		 * @param  fd         файловый дескриптор (сокет)
		 * @param  read_size  размер буфера на чтение
		 * @param  write_size размер буфера на запись
		 * @param  maxcon     максимальное количество подключений
		 * @param  log        указатель на объект ведения логов
		 * @return            результат работы функции
		 */
		static int socket_buffersize(evutil_socket_t fd, int read_size, int write_size, u_int maxcon, LogApp * log);
		/**
		 * check_auth Функция проверки логина и пароля
		 * @param ctx объект входящих данных
		 * @return    результат проверки подлинности
		 */
		static bool check_auth(void * ctx);
		/**
		 * isallow_remote_connect Функция проверяет разрешено ли подключение к удаленному серверу
		 * @param  ip  ip адрес удаленного сервера
		 * @param  ctx объект с данными подключения
		 * @return     результат проверки
		 */
		static bool isallow_remote_connect(const string ip, void * ctx);
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
		static void connection(void * ctx);
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
		/**
		 * HttpProxy::run_server Метод запуска прокси сервера
		 * @param socket сокет который слушает прокси сервер
		 */
		static void run_server(evutil_socket_t socket, void * ctx);
		/**
		 * run_works Метод запуска воркеров
		 * @param pids   указатель на массив пидов процессов
		 * @param socket сокет прокси сервера
		 * @param cur    текущее значение пида процесса
		 * @param max    максимальное значение пидов процессов
		 * @param ctx    объект прокси сервера
		 */
		static void run_works(pid_t * pids, evutil_socket_t socket, size_t cur, size_t max, void * ctx);
	public:
		/**
		 * HttpProxy Конструктор
		 * @param proxy объект параметров прокси сервера
		 */
		HttpProxy(System * proxy = NULL);
		/**
		 * ~HttpProxy Деструктор
		 */
		~HttpProxy();
};

#endif // _HTTP_PROXY_ANYKS_