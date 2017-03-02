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
 * ConnectClients Класс всех подключенных пользователей
 */
class ConnectClients {
	public:
		/**
		 * Locker Структура для блокировки потока
		 */
		struct Freeze {
			// Мютекс для захвата потока
			mutex mtx;
			// Переменная состояния
			condition_variable cond;
		};
	private:
		/**
		 * Client Класс всех подключений пользователя
		 */
		class Client {
			private:
				// Мютекс для захвата потока
				mutex mtx;
				// Ключ клиента
				string key;
				// Блокировщик лишних потоков
				Freeze freeze;
				// Максимально возможное количество подключений
				u_int max = 0;
				// Количество активных клиентов
				u_int connects = 0;
				/**
				 * rm Метод удаления подключения из объекта клиента
				 */
				void rm();
			public:
				// Коллбек для удаления текущего клиента
				function <void (const string)> remove;
				/**
				 * add Метод добавления нового подключения в объект клиента
				 * @param ctx передаваемый указатель на объект
				 */
				void add(void * ctx);
		};
		// Мютекс для захвата потока
		mutex mtx;
		// Массив всех активных клиентов
		map <string, unique_ptr <Client>> clients;
		/**
		 * rm Метод удаления объекта подключившихся клиентов
		 * @param key ключ клиента
		 */
		void rm(const string key);
	public:
		/**
		 * add Метод добавления нового подключения в объект пользователя
		 * @param ctx передаваемый указатель на объект
		 */
		void add(void * ctx);
};
/**
 * BufferHttpProxy Класс для работы с данными прокси сервера
 */
class BufferHttpProxy {
	private:
		// Мютекс для захвата потока
		mutex mtx;
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
			string	request		= "";		// Данные запроса
		};
		/**
		 * toCase Функция перевода в указанный регистр
		 * @param  str  строка для перевода в указанных регистр
		 * @param  flag флаг указания типа регистра
		 * @return      результирующая строка
		 */
		const string toCase(const string str, const bool flag = false);
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
		// Блокиратор лишнего потока
		void * frze = NULL;
		// Флаг авторизации
		bool auth = false;
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
		HttpData httpResponse;
		// Данные http запроса
		HttpData httpRequest;
		// Параметры прокси сервера
		System * proxy = NULL;
		// Создаем объект dns ресолвера
		DNSResolver * dns = NULL;
		// База событий
		struct event_base * base = NULL;
		// Коллбек для удаления текущего подключения
		function <void (void)> remove;
		// Коллбек для проверки на доступность подключений
		function <bool (void)> isFull;
		// Коллбек для определения количества активных подключений
		function <u_int (void)> activeConnects;
		/**
		 * closeClient Метод закрытия соединения клиента
		 */
		void closeClient();
		/**
		 * closeServer Метод закрытия соединения сервера
		 */
		void closeServer();
		/**
		 * close Метод закрытия подключения
		 */
		void close();
		/**
		 * freeze Метод заморозки потока
		 */
		void freeze();
		/**
		 * setCompress Метод проверки активации режима сжатия данных на уровне прокси сервера
		 */
		void setCompress();
		/**
		 * checkUpgrade Метод проверки на желание смены протокола
		 */
		void checkUpgrade();
		/**
		 * checkClose Метод проверки на отключение от сервера
		 */
		void checkClose();
		/**
		 * sleep Метод усыпления потока на время необходимое для соблюдения скоростного ограничения сети
		 * @param  size размер передаваемых данных
		 * @param  type тип передаваемого сообщения (true - чтение, false - запись)
		 * @return      время в секундах на которое следует задержать поток
		 */
		void sleep(const size_t size, const bool type);
		/**
		 * setTimeout Метод установки таймаутов
		 * @param type  тип подключения (клиент или сервер)
		 * @param read  таймаут на чтение
		 * @param write таймаут на запись
		 */
		void setTimeout(const u_short type, const bool read = false, const bool write = false);
		/**
		 * sendClient Метод отправки данных на клиент
		 * @param http объект http данных
		 */
		void sendClient(HttpData &http);
		/**
		 * sendServer Метод отправки данных на сервер
		 * @param http объект http данных
		 */
		void sendServer(HttpData &http);
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
		// Мютекс для захвата потока
		mutex mtx;
		// Список подключенных клиентов к прокси серверу
		ConnectClients clients;
		// Идентификаторы процессов
		pid_t * pids = NULL;
		// Параметры прокси сервера
		System * server = NULL;
		// Создаем новую базу
		struct event_base * base = NULL;
		/**
		 * create_client Метод создания нового клиента
		 * @param ip  адрес интернет протокола клиента
		 * @param mac аппаратный адрес сетевого интерфейса клиента
		 * @param fd  файловый дескриптор (сокет) подключившегося клиента
		 */
		void create_client(const string ip, const string mac, const evutil_socket_t fd);
		/**
		 * create_server Функция создания прокси сервера
		 * @return сокет прокси сервера
		 */
		const evutil_socket_t create_server();
		/**
		 * get_mac Метод определения мак адреса клиента
		 * @param  ctx указатель на объект подключения
		 * @return     данные мак адреса
		 */
		static const string get_mac(void * ctx);
		/**
		 * get_ip Функция получения данных ip адреса
		 * @param  family тип интернет протокола
		 * @param  ctx    указатель на объект подключения
		 * @return        данные ip адреса
		 */
		static const string get_ip(const int family, void * ctx);
		/**
		 * socket_nosigpipe Функция установки отключения сигнала записи в оборванное подключение
		 * @param  fd   файловый дескриптор (сокет)
		 * @param  log  указатель на объект ведения логов
		 * @return      результат работы функции
		 */
		static const int socket_nosigpipe(const evutil_socket_t fd, LogApp * log);
		/**
		 * socket_nonblocking Функция установки неблокирующего сокета
		 * @param  fd   файловый дескриптор (сокет)
		 * @param  log  указатель на объект ведения логов
		 * @return      результат работы функции
		 */
		static const int socket_nonblocking(const evutil_socket_t fd, LogApp * log);
		/**
		 * socket_tcpcork Функция активации tcp_cork
		 * @param  fd   файловый дескриптор (сокет)
		 * @param  log  указатель на объект ведения логов
		 * @return      результат работы функции
		 */
		static const int socket_tcpcork(const evutil_socket_t fd, LogApp * log);
		/**
		 * socket_tcpnodelay Функция отключения алгоритма Нейгла
		 * @param  fd   файловый дескриптор (сокет)
		 * @param  log  указатель на объект ведения логов
		 * @return      результат работы функции
		 */
		static const int socket_tcpnodelay(const evutil_socket_t fd, LogApp * log);
		/**
		 * socket_reuseable Функция разрешающая повторно использовать сокет после его удаления
		 * @param  fd   файловый дескриптор (сокет)
		 * @param  log  указатель на объект ведения логов
		 * @return      результат работы функции
		 */
		static const int socket_reuseable(const evutil_socket_t fd, LogApp * log);
		/**
		 * socket_keepalive Функция устанавливает постоянное подключение на сокет
		 * @param  fd      файловый дескриптор (сокет)
		 * @param  log     указатель на объект ведения логов
		 * @param  cnt     максимальное количество попыток
		 * @param  idle    время через которое происходит проверка подключения
		 * @param  intvl   время между попытками
		 * @return         результат работы функции
		 */
		static const int socket_keepalive(const evutil_socket_t fd, LogApp * log, const int cnt, const int idle, const int intvl);
		/**
		 * socket_buffersize Функция установки размеров буфера
		 * @param  fd         файловый дескриптор (сокет)
		 * @param  read_size  размер буфера на чтение
		 * @param  write_size размер буфера на запись
		 * @param  maxcon     максимальное количество подключений
		 * @param  log        указатель на объект ведения логов
		 * @return            результат работы функции
		 */
		static const int socket_buffersize(const evutil_socket_t fd, const int read_size, const int write_size, const u_int maxcon, LogApp * log);
		/**
		 * check_auth Функция проверки логина и пароля
		 * @param ctx объект входящих данных
		 * @return    результат проверки подлинности
		 */
		static const bool check_auth(void * ctx);
		/**
		 * isallow_remote_connect Функция проверяет разрешено ли подключение к удаленному серверу
		 * @param  ip  ip адрес удаленного сервера
		 * @param  ctx объект с данными подключения
		 * @return     результат проверки
		 */
		static const bool isallow_remote_connect(const string ip, void * ctx);
		/**
		 * connect_server Функция создания сокета для подключения к удаленному серверу
		 * @param ctx объект входящих данных
		 * @return    результат подключения
		 */
		static const int connect_server(void * ctx);
		/**
		 * send_http_data Функция отправки незашифрованных данных клиенту
		 * @param ctx передаваемый объект
		 */
		static void send_http_data(void * ctx);
		/**
		 * do_request Функция запроса данных у сервера
		 * @param ctx передаваемый объект
		 */
		static void do_request(void * ctx);
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
		static void event_cb(struct bufferevent * bev, const short events, void * ctx);
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
		static void accept_cb(const evutil_socket_t fd, const short event, void * ctx);
		/**
		 * HttpProxy::run_server Метод запуска прокси сервера
		 * @param fd  сокет (файловый дескриптор) который слушает прокси сервер
		 * @param ctx объект прокси сервера
		 */
		static void run_server(const evutil_socket_t fd, void * ctx);
		/**
		 * run_works Метод запуска воркеров
		 * @param pids указатель на массив пидов процессов
		 * @param fd   сокет (файловый дескриптор) прокси сервера
		 * @param cur  текущее значение пида процесса
		 * @param max  максимальное значение пидов процессов
		 * @param ctx  объект прокси сервера
		 */
		static void run_works(pid_t * pids, const evutil_socket_t socket, const size_t cur, const size_t max, void * ctx);
	public:
		/**
		 * connection Функция обработки данных подключения в треде
		 * @param ctx передаваемый объект
		 */
		static void connection(void * ctx);
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