/* СЕРВЕР HTTP ПРОКСИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2016
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
#include "../lib/log.h"
#include "../lib/http.h"

// Устанавливаем область видимости
using namespace std;

// Флаги базы данных событий
#define EVLOOP_ONCE				0x01
#define EVLOOP_NONBLOCK			0x02
#define EVLOOP_NO_EXIT_ON_EMPTY	0x04

// Внутренний интерфейс
#define INTERNAL_IP "127.0.0.1"
// Внешний интерфейс
#define EXTERNAL_IP "0.0.0.0"
// Максимальное количество подключений с одного клиента
#define MAX_CONNECTS 100
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
 * Timeout Структура таймаутов
 */
struct Timeout {
	u_short read;		// Таймаут времени на чтение
	u_short write;		// Таймаут времени на запись
	u_short keepalive;	// Таймаут ожидания коннекта
} __attribute__((packed));
/**
 * BufferSize Структура размеров буфера
 */
struct BufferSize {
	int read;		// Буфер на чтение
	int write;		// Буфер на запись
} __attribute__((packed));
/**
 * Proxy Структура прокси сервера
 */
struct Proxy {
	string		name;		// Название прокси сервера
	string		version;	// Версия прокси сервера
	string		internal;	// Внутренний адрес прокси сервера
	string		external;	// Внешний адрес прокси сервера
	u_short		options;	// Параметры прокси сервера
	Timeout		timeout;	// Таймауты прокси сервера
	BufferSize	bsize;		// Размеры буферов
	LogApp		* log;		// Указатель на объект ведения логов
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
		inline size_t get(){
			// Выводим количество подключений
			return connects;
		}
		/**
		 * end Метод проверки на конец всех подключений
		 * @return проверка на достижения нуля
		 */
		inline bool end(){
			// Если количество подключений меньше 1 то сообщаем об этом
			return !(connects > 0);
		}
		/**
		 * inc Метод инкреминации количества подключений
		 */
		inline void inc(){
			// Выполняем инкремент
			connects++;
		}
		/**
		 * dec Метод декрементации количества подключений
		 */
		inline void dec(){
			// Выполняем декрементацию
			if(connects > 0) connects--;
		}
		/**
		 * lock Метод блокировки мютекса
		 */
		inline void lock(){
			// Лочим мютекс
			pthread_mutex_lock(&mutex);
		}
		/**
		 * unlock Метод разблокировки мютекса
		 */
		inline void unlock(){
			// Разлочим мютекс
			pthread_mutex_unlock(&mutex);
		}
		/**
		 * signal Метод отправки сигнала первому блокированному потоку
		 */
		inline void signal(){
			// Отправляем сигнал
			pthread_cond_signal(&condition);
		}
		/**
		 * broadcastSignal Метод отправки сигналов всем блокированным потокам
		 */
		inline void broadcastSignal(){
			// Выполняем вещание
			pthread_cond_broadcast(&condition);
		}
		/**
		 * wait Метод блокировки потока
		 */
		inline void wait(){
			// Блокируем поток
			pthread_cond_wait(&condition, &mutex);
		}
		/**
		 * Connects Конструктор
		 */
		Connects(){
			// Устанавливаем первоначальное значение коннекта
			connects = 1;
			// Инициализируем мютекс
			pthread_mutex_init(&mutex, 0);
			// Инициализируем переменную состояния
			pthread_cond_init(&condition, 0);
		}
		/**
		 * ~Connects Деструктор
		 */
		~Connects(){
			// Удаляем мютекс
			pthread_mutex_destroy(&mutex);
			// Удаляем переменную состояния
			pthread_cond_destroy(&condition);
		}
};
/**
 * BufferHttpProxy Класс для работы с данными прокси сервера
 */
class BufferHttpProxy {
	private:
		/**
		 * Events Буферы событий
		 */
		struct Events {
			struct bufferevent * client = NULL;	// Буфер событий клиента
			struct bufferevent * server = NULL; // Буфер событий сервера
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
			u_int	port;	// Порт
			string	host;	// Хост адрес
			string	mac;	// Мак адрес
		} __attribute__((packed));
		/**
		 * Sockets Сокеты клиента и сервера
		 */
		struct Sockets {
			evutil_socket_t client;	// Сокет клиента
			evutil_socket_t server;	// Сокет сервера
		} __attribute__((packed));
		/**
		 * appconn Функция которая добавляет или удаляет в список склиента
		 * @param flag флаг подключения или отключения клиента
		 */
		void appconn(bool flag);
	public:
		bool					auth = false;	// Флаг авторизации
		struct event_base		* base;			// База событий
		map <string, Connects>	* connects; 	// Список подключений к прокси серверу
		Http					* parser;		// Объект парсера
		HttpQuery				response;		// Ответ системы
		Request					request;		// Данные запроса
		Events					events;			// Буферы событий
		Server					server;			// Параметры удаленного сервера
		Server					client;			// Параметры подключившегося клиента
		Proxy					proxy;			// Параметры прокси сервера
		Sockets					sockets;		// Сокеты подключений
		/**
		 * begin Метод активации подключения
		 */
		void begin(){
			// Добавляем в список подключений
			appconn(true);
		}
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
		 * @param string  version версия ресурса
		 * @param u_short options параметры прокси сервера
		 */
		BufferHttpProxy(string name, string version, u_short options){
			// Создаем объект для работы с http заголовками
			parser = new Http(name, options, version);
		}
		/**
		 * ~BufferHttpProxy Деструктор
		 */
		~BufferHttpProxy(){

			cout << " ====== 1 " << endl;

			// Удаляем событие клиента
			free_client();

			cout << " ====== 2 " << endl;

			// Удаляем событие сервера
			free_server();

			cout << " ====== 3 " << endl;

			// Удаляем из списока подключений
			appconn(false);

			cout << " ====== 4 " << endl;

			// Если парсер не удален
			if(parser != NULL){
				// Удаляем парсер
				delete parser;
				// Запоминаем что данные удалены
				parser = NULL;
			}

			cout << " ====== 5 " << endl;

			// Очищаем память выделенную для вектора
			vector <char> ().swap(request.data);

			cout << " ====== 6 " << endl;
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
		 * @param log        указатель на объект ведения логов
		 * @param name       название прокси сервера
		 * @param version    версия прокси сервера
		 * @param internal   внутренний хост прокси сервера
		 * @param external   внешний хост прокси сервера
		 * @param port       порт прокси сервера
		 * @param buffrsize  размер буфера сокета на чтение
		 * @param buffwsize  размер буфера сокета на запись
		 * @param maxcls     максимальное количество подключаемых клиентов к прокси серверу (-1 автоматически)
		 * @param rtm        таймаут на чтение данных из сокета сервера
		 * @param wtm        таймаут на запись данных из сокета клиента и сервера
		 * @param katm       таймаут на чтение данных из сокета клиента
		 * @param options    опции прокси сервера
		 */
		HttpProxy(
			LogApp			* log		= NULL,
			const char		* name		= "anyks",
			const char		* version	= APP_VERSION,
			const char		* internal	= INTERNAL_IP,
			const char		* external	= EXTERNAL_IP,
			u_int			port		= SERVER_PORT,
			int				buffrsize	= BUFFER_READ_SIZE,
			int				buffwsize	= BUFFER_WRITE_SIZE,
			int				maxcls		= SOMAXCONN,
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

#endif // _HTTP_PROXY_ANYKS_
