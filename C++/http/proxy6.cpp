// sudo lsof -i | grep -E LISTEN
// otool -L http5
// MacOS X
// g++ -std=c++11 -D_BSD_SOURCE -Wall -pedantic -O3 -Werror=vla -o http6 http.cpp base64.cpp proxy6.cpp -I/usr/local/include -lpthread -levent = dynamic
// g++ -std=c++11 -D_BSD_SOURCE -Wall -pedantic -O3 -Werror=vla -o http6 http.cpp base64.cpp proxy6.cpp -I/usr/local/include /usr/local/opt/libevent/lib/libevent.a -lpthread = static
// Linux
// g++ -std=c++11 -Wall -pedantic -O3 -Werror=vla -o http6 http.cpp base64.cpp proxy6.cpp -levent -lpthread = dynamic
// g++ -std=c++11 -Wall -pedantic -O3 -Werror=vla -o http6 http.cpp base64.cpp proxy6.cpp /usr/lib/x86_64-linux-gnu/libevent.a /usr/lib/x86_64-linux-gnu/5/libstdc++.a -lpthread = static

// MacOS X
// export EVENT_NOKQUEUE=1
// brew uninstall --force tmux
// brew install --HEAD tmux
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include "http.h"

// Устанавливаем область видимости
using namespace std;

// Флаги базы данных событий
#define EVLOOP_ONCE				0x01
#define EVLOOP_NONBLOCK			0x02
#define EVLOOP_NO_EXIT_ON_EMPTY	0x04

// Таймер ожидания защита от брута
#define TTL_CONNECT 0 // 0.5
// Максимальное количество клиентов
#define MAX_CLIENTS 10
// Максимальное количество открытых сокетов (по дефолту в системе 1024)
#define MAX_SOCKETS 1024
// Максимальное количество воркеров
#define MAX_WORKERS 4
// Таймаут ожидания для http 1.1
#define KEEP_ALIVE {30, 0}

// Временный каталог для файлов
string nameSystem	= "anyksHttp";
string piddir		= "/var/run";

/**
 * BufferHttpProxy Класс для работы с данными прокси сервера
 */
class BufferHttpProxy {
	private:
		// Буфер данных
		struct Buffer {
			char	* data	= NULL;	// Данные в буфере
			size_t	offset	= 0,	// Смещение в буфере
					total	= 0,	// Общее количество данных в буфере
					size	= 4096;	// Размер буфера
		};
		// Структура данных для http вложений
		struct http_body {
			char	* data;		// Данные вложений
			size_t	length = 0;	// Размер данных вложений
		};
		// Структура данных для выполнения запросов на удаленном сервере
		struct http_query {
			string		request;	// Запрос данных на удаленном сервере
			http_body	entitybody;	// Тело вложений в запросе
		};
	public:
		struct event_base	* base = NULL;			// База событий
		Http				* parser = NULL;		// Объект парсера
		struct event		* evClient = NULL,		// Событие клиента
							* evServer = NULL;		// Событие сервера
		http_query			* response = NULL;		// Ответ системы
		Buffer				request;				// Данные запроса
		bool				auth		= false,	// Флаг авторизации
							ishttps		= false;	// Флаг зашифрованного соединения
		int					pid			= -1;		// Пид процесса
		evutil_socket_t		socServer	= -1,		// Сокет сервера
							socClient	= -1;		// Сокет клиента
		/**
		 * requiredAuth Метод получения ответа (запроса ввода логина и пароля)
		 * @return объект с данными запроса
		 */
		http_query * requiredAuth(){
			// Формируем ответ клиенту
			return (parser != NULL ? reinterpret_cast <BufferHttpProxy::http_query *> (parser->requiredAuth()) : NULL);
		}
		/**
		 * faultAuth Метод получения ответа (неудачной авторизации)
		 * @return объект с данными запроса
		 */
		http_query * faultAuth(){
			// Формируем ответ клиенту
			return (parser != NULL ? reinterpret_cast <BufferHttpProxy::http_query *> (parser->faultAuth()) : NULL);
		}
		/**
		 * authSuccess Метод получения ответа (подтверждения авторизации)
		 * @return объект с данными запроса
		 */
		http_query * authSuccess(){
			// Формируем ответ клиенту
			return (parser != NULL ? reinterpret_cast <BufferHttpProxy::http_query *> (parser->authSuccess()) : NULL);
		}
		/**
		 * faultConnect Метод получения ответа (неудачного подключения к удаленному серверу)
		 * @return объект с данными запроса
		 */
		http_query * faultConnect(){
			// Формируем ответ клиенту
			return (parser != NULL ? reinterpret_cast <BufferHttpProxy::http_query *> (parser->faultConnect()) : NULL);
		}
		/**
		 * getQuery Метод получения сформированного http запроса
		 * @return объект с данными запроса
		 */
		http_query * getQuery(){
			// Формируем ответ клиенту
			return (parser != NULL ? reinterpret_cast <BufferHttpProxy::http_query *> (parser->getQuery()) : NULL);
		}
		/**
		 * BufferHttpProxy Конструктор
		 * @param [string] name имя ресурса
		 */
		BufferHttpProxy(string name){
			// Создаем объект для работы с http заголовками
			parser = new Http(name);
			// Создаем буфер с данными
			request.data = new char[(const size_t) request.size];
		}
		/**
		 * ~BufferHttpProxy Деструктор
		 */
		~BufferHttpProxy(){
			// Отключаем от сервиса (disconnect)
			if(socServer > 0){
				shutdown(socServer, SHUT_RDWR);
				close(socServer);
				socServer = -1;
			}
			if(socClient > 0){
				shutdown(socClient, SHUT_RDWR);
				close(socClient);
				socClient = -1;
			}
			// Удаляем события
			if((evServer != NULL) && (evServer->ev_base != NULL)) event_free(evServer);
			else if((evServer != NULL) && (evServer->ev_base == NULL)) delete evServer;
			if((evClient != NULL) && (evClient->ev_base != NULL)) event_free(evClient);
			else if((evClient != NULL) && (evClient->ev_base == NULL)) delete evClient;
			// Удаляем указатели
			evServer = NULL;
			evClient = NULL;
			// Если буфер не удален то удаляем его
			if(request.data != NULL){
				// Удаляем буфер
				delete [] request.data;
				// Запоминаем что данные удалены
				request.data = NULL;
			}
			// Если парсер не удален
			if(parser != NULL){
				// Удаляем парсер
				delete parser;
				// Запоминаем что данные удалены
				parser = NULL;
			}
		}
		/**
		 * parse Метод парсинга данных
		 * @return результат работы парсинга
		 */
		bool parse(){
			// Выполняем парсинг данных
			return parser->parse(request.data, request.total);
		}
};
/**
 * imess Структура для передачи идентификатора сокета между процессами
 */
typedef struct buff {
	long type;
	int socket;
} imess;
/**
 * debug_message Функция вывода отладочной информации
 * @param str текстовое сообщение
 */
void debug_message(string str){
	// Выводим сообщение в консоль
	cout << str.c_str() << endl;
}
/**
 * sigpipe_handler Функция обработки сигнала SIGPIPE
 * @param signum номер сигнала
 */
void sigpipe_handler(int signum){
	// Выводим в консоль информацию
	debug_message("Сигнал обрыва соединения во время записи!!!!");
}
/**
 * sigchld_handler Функция обработки сигнала о появившемся зомби процессе SIGCHLD
 * @param signum номер сигнала
 */
void sigchld_handler(int signum) {
	// Выводим в консоль информацию
	debug_message("Дочерний процесс убит!!!!");
	// Избавляемся от зависших процессов
	while(waitpid(-1, NULL, WNOHANG) > 0);
}
/**
 * sigterm_handler Функция обработки сигналов завершения процессов SIGTERM
 * @param signum номер сигнала
 */
void sigterm_handler(int signum){
	// Выводим в консоль информацию
	debug_message("Процесс убит!!!!");
	// Выходим
	exit(0);
}
/**
 * sighup_handler Функция обработки сигналов потери терминала SIGHUP
 * @param signum номер сигнала
 */
void sighup_handler(int signum){
	// Выводим в консоль информацию
	debug_message("Терминал потерял связь!!!");
}
/**
 * remove_pid Функция удаления pid файла
 * @param ext  тип ошибки
 */
void remove_pid(int ext){
	// Формируем имя файла
	string filename = piddir + string("/") + nameSystem + ".pid";
	// Удаляем файл
	remove(filename.c_str());
	// Выходим из приложения
	exit(ext);
}
/**
 * create_pid Функция записи pid файла
 * @param pid идентификатор pid файла
 */
void create_pid(pid_t pid){
	// Формируем имя файла
	string filename = piddir + string("/") + nameSystem + ".pid";
	// Удаляем файл
	remove(filename.c_str());
	// Открываем файл на запись
	FILE * fh = fopen(filename.c_str(), "w");
	// Выводим номер процесса
	cout << pid << endl;
	// Записываем считанные данные в файл
	fprintf(fh, "%d", pid);
	// Закрываем файл
	fclose(fh);
}
/**
 * set_non_block Функция установки неблокирующего сокета
 * @param fd файловый дескриптор (сокет)
 */
int set_non_block(evutil_socket_t fd){
	int flags;
	flags = fcntl(fd, F_GETFL);
	if(flags < 0) return flags;
	flags |= O_NONBLOCK;
	if(fcntl(fd, F_SETFL, flags) < 0) return -1;
	return 0;
}
/**
 * create_app_socket Функция создания сокета системы
 * @return файловый дескриптор (сокет)
 */
evutil_socket_t create_app_socket(){
	// Сокет сервера
	evutil_socket_t serversock;
	// Максимальное число клиентов
	int maxpending = MAX_CLIENTS;
	// Структура для сервера
	struct sockaddr_in echoserver;
	// Создаем сокет
	if((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		// Выводим в консоль информацию
		debug_message("[-] Could not create socket.");
		// Выходим
		return -1;
	}
	/* Очищаем структуру */
	memset(&echoserver, 0, sizeof(echoserver));
	// Указываем что это сетевой протокол Internet/IP
	echoserver.sin_family = AF_INET;
	// Указываем адрес прокси сервера
	echoserver.sin_addr.s_addr = htonl(INADDR_ANY); // inet_addr("127.0.0.1");
	// Указываем порт сервера
	echoserver.sin_port = htons(5556); // htons(SERVER_PORT);
	// Устанавливаемое значение
	int optval = 1;
	// Устанавливаем TCP_NODELAY
	if(setsockopt(serversock, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(int)) < 0){
		// Выводим в консоль информацию
		debug_message("Cannot set TCP_NODELAY option on listen socket.");
		// Выходим
		return -1;
	}
	// Выполняем биндинг сокета // ::bind (для мака)
	if(::bind(serversock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0){
		// Выводим в консоль информацию
		debug_message("[-] Bind error.");
		// Выходим
		return -1;
	}
	// Выполняем чтение сокета
	if(listen(serversock, maxpending) < 0){
		// Выводим в консоль информацию
		debug_message("[-] Listen error.");
		// Выходим
		return -1;
	}
	// Указываем что сокет не блокирующий
	if(set_non_block(serversock) < 0){
		// Выводим в консоль информацию
		debug_message("[-] Failed to set server socket to non-blocking.");
		// Выходим
		return -1;
	}
	// Выводим сокет системы
	return serversock;
}
/**
 * create_server_socket Функция создания сокета для подключения к удаленному серверу
 * @param  host хост сервера
 * @param  port порт сервера
 * @return      файловый дескриптор (сокет)
 */
evutil_socket_t create_server_socket(const char * host, int port){
	// Сокет подключения
	evutil_socket_t sock = 0;
	// Структура параметров подключения
	struct addrinfo param;
	// Указатель на результаты
	struct addrinfo * req;
	// Убедимся, что структура пуста
	memset(&param, 0, sizeof(param));
	// Неважно, IPv4 или IPv6
	param.ai_family = AF_UNSPEC;
	// TCP stream-sockets
	param.ai_socktype = SOCK_STREAM;
	// Если формат подключения указан не верно то сообщаем об этом
	if(getaddrinfo(host, to_string(port).c_str(), &param, &req) != 0){
		// Выводим в консоль информацию
		debug_message("Error in server address format!");
		// Выходим
		return -1;
	}
	// Создаем сокет, если сокет не создан то сообщаем об этом
	if((sock = socket(req->ai_family, req->ai_socktype, req->ai_protocol)) < 0){
		// Выводим в консоль информацию
		debug_message("Error in creating socket to server!");
		// Выходим
		return -1;
	}
	// Выполняем подключение к удаленному серверу, если подключение не выполненно то сообщаем об этом
	if(connect(sock, req->ai_addr, req->ai_addrlen) < 0){
		// Выводим в консоль информацию
		debug_message("Error in connecting to server!");
		// Выходим
		return -1;
	}
	// И освобождаем связанный список
	freeaddrinfo(req);
	// Указываем что сокет не блокирующий
	if(set_non_block(sock) < 0) debug_message("Failed to set server socket non-blocking");
	// Выводим созданный нами сокет
	return sock;
}
/**
 * check_auth Функция проверки логина и пароля
 * @param  http объект в котором содержится username и password
 * @return      результат проверки подлинности
 */
bool check_auth(Http * &http){
	// Логин
	const char * username = "zdD786KeuS";
	// Проль
	const char * password = "k.frolovv@gmail.com";
	// Проверяем логин и пароль
	if(!strcmp(http->getLogin().c_str(), username)
	&& !strcmp(http->getPassword().c_str(), password)) return true;
	else return false;
}
/**
 * close_socket Функция отключения от сервера
 * @param sock сокет для отключения
 */
void close_socket(evutil_socket_t &sock){
	// Если сокет существует
	if(sock > -1){
		// Отключаемся
		shutdown(sock, SHUT_RDWR);
		// Закрываем сокет
		close(sock);
		// Помечаем что сокет закрыт
		sock = -1;
	}
}
/**
 * close_event Функция очистки события
 * @param ev указатель на событие
 */
void close_event(struct event ** ev){
	// Очищаем событие
	if(*ev != NULL){
		// Очищаем событие
		event_free(*ev);
		// Обнуляем указатель события
		*ev = NULL;
	}
}
/**
 * close_events Функция закрытия всех событий
 * @param arg ссылка на объект подключения
 */
void close_events(BufferHttpProxy ** arg){
	// Получаем объект подключения
	BufferHttpProxy * http = *arg;
	// Если данные существуют
	if(http != NULL){
		// Очищаем событие для сервера
		close_event(&http->evServer);
		// Очищаем событие для клиента
		close_event(&http->evClient);
	}
}
/**
 * free_data Функция очистки памяти для http прокси
 * @param arg объект для очистки
 */
void free_data(BufferHttpProxy ** http){
	// Если данные еще не удалены
	if(*http != NULL){
		// Удаляем объект данных
		delete *http;
		// Присваиваем пустой адрес
		*http = NULL;
	}
}
/**
 * flash_connect Функция очистки подключения
 * @param arg ссылка на объект подключения
 * @param fd  файловый дескриптор (сокет)
 */
void flash_connect(evutil_socket_t fd, BufferHttpProxy ** arg){
	// Получаем объект подключения
	BufferHttpProxy * http = *arg;
	// Если данные существуют
	if(http != NULL){
		// Если сервер отключен тогда выходим
		if(http->socServer < 0) free_data(&http);
		// Если сервер не отключен
		else if(fd == http->socServer) {
			// Снимаем событие
			close_event(&http->evServer);
			// Отключаем сокет
			close_socket(http->socServer);
		}
	}
}
/**
 * on_http_connect Прототип функции подключения к серверу
 * @param fd    файловый дескриптор (сокет)
 * @param event событие на которое сработала функция обратного вызова
 * @param arg   объект передаваемый как значение
 */
//void on_http_connect(evutil_socket_t fd, short event, void * arg);
/**
 * on_http_request Прототип функции проверка запроса
 * @param fd    файловый дескриптор (сокет)
 * @param event событие на которое сработала функция обратного вызова
 * @param arg   объект передаваемый как значение
 */
//void on_http_request(evutil_socket_t fd, short event, void * arg);
/**
 * on_http_write_client Прототип функции вывода ответа клиенту
 * @param fd    файловый дескриптор (сокет)
 * @param event событие на которое сработала функция обратного вызова
 * @param arg   объект передаваемый как значение
 */
//void on_http_write_client(evutil_socket_t fd, short event, void * arg);
/**
 * do_http_proxy Функция обмена сообщениями между клиентом и сервером (https)
 * @param fd    файловый дескриптор (сокет)
 * @param event событие на которое сработала функция обратного вызова
 * @param arg   объект передаваемый как значение
 */
/*
void do_http_proxy(evutil_socket_t fd, short event, void * arg){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (arg);
	// Если подключение не передано
	if(!http) close_socket(fd);
	// Если все данные получены
	else {
		// Проверяем возникшие события
		switch(event){
			// Если это таймаут
			case 1: {
				// Выводим в консоль информацию
				debug_message("Таймаут клиент!!!!");
				// Если сервер отключился то отключаем все
				flash_connect(fd, &http);
			} break;
			// Если это чтение данных
			case 2: {
				// Проверяем нужно ли держать подключение
				bool alive = http->parser->isAlive();
				// Буфер для чтения данных из сокета
				char buffer[256];
				// Выполняем чтение данных из сокета
				int len = recv(fd, buffer, sizeof(buffer), 0);
				// Получаем сокет для ответа
				evutil_socket_t socket = (fd != http->socServer ? http->socServer : http->socClient);
				// Если данные не считаны значит клиент отключился
				if(len <= 0){
					// Если нужно работать в режиме HTTP/1.0
					// Если установлен заголовок Proxy-Connection: keep-alive и это HTTP/1.1
					// а также это не зашифрованное соединение то мы не отключаемся а ждем новых порций данных
					if(!alive || http->ishttps){
						// Выводим в консоль информацию
						debug_message("Work is done!");
						// Если сервер отключился то отключаем все
						flash_connect(fd, &http);
					}
				// Если данные считаны нормально
				} else if(socket > -1) {
					// Выполняем отправку данных на сокет клиента
					send(socket, (void *) buffer, len, 0);
				// Если сокет не существует тогда удаляем данные
				} else flash_connect(fd, &http);
			} break;
		}
	}
	// Выходим
	return;
}
*/
/**
 * on_http_proxy обмена сообщениями между клиентом и сервером (http)
 * @param fd    файловый дескриптор (сокет)
 * @param event событие на которое сработала функция обратного вызова
 * @param arg   объект передаваемый как значение
 */
/*
void on_http_proxy(evutil_socket_t fd, short event, void * arg){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (arg);
	// Если подключение не передано
	if(!http) close_socket(fd);
	// Если все данные получены
	else {
		// Обрабатываем входящее событие
		switch(event){
			// Если это таймаут
			case 1: {
				// Выводим в консоль информацию
				debug_message("Таймаут клиент!!!!");
				// Выполняем очистку подключения
				free_data(&http);
			} break;
			// Если это запись
			case 4: {
				// Получаем запрос
				string query = http->parser->getQuery();
				// Send data to client
				long len = write(fd, query.c_str() + http->request.offset, query.length() - http->request.offset);
				// Выполняем отправку данных до тех пор пока все не отправим
				if(len < (long) (query.length() - http->request.offset)){
					// Failed to send rest data, need to reschedule
					http->request.offset += len;
				// Если мы все отправили
				} else {
					// Устанавливаем таймаут ожидания запроса в 3 секунды
					struct timeval timeout = KEEP_ALIVE;
					// Закрываем события
					close_events(&http);
					// Создаем новое событие для клиента
					http->evClient = event_new(http->base, http->socClient, EV_TIMEOUT | EV_READ | EV_PERSIST, do_http_proxy, http);
					http->evServer = event_new(http->base, http->socServer, EV_TIMEOUT | EV_READ | EV_PERSIST, do_http_proxy, http);
					// Активируем события
					event_add(http->evClient, &timeout);
					event_add(http->evServer, &timeout);
				}
			} break;
		}
	}
	// Выходим
	return;
}
*/
/**
 * on_http_write_client Функция вывода ответа клиенту
 * @param fd    файловый дескриптор (сокет)
 * @param event событие на которое сработала функция обратного вызова
 * @param arg   объект передаваемый как значение
 */
/*
void on_http_write_client(evutil_socket_t fd, short event, void * arg){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (arg);
	// Если подключение не передано
	if(!http) close_socket(fd);
	// Если все данные получены
	else {
		// Обрабатываем входящее событие
		switch(event){
			// Если это таймаут
			case 1: {
				// Выводим в консоль информацию
				debug_message("Таймаут клиент!!!!");
				// Выполняем очистку подключения
				free_data(&http);
			} break;
			// Если это запись
			case 4: {
				// Send data to client
				long len = write(fd, http->response.c_str() + http->request.offset, http->response.length() - http->request.offset);
				// Выполняем отправку данных до тех пор пока все не отправим
				if(len < (long) (http->response.length() - http->request.offset)){
					// Failed to send rest data, need to reschedule
					http->request.offset += len;
				// Если мы все отправили
				} else {
					// Устанавливаем таймаут ожидания запроса в 3 секунды
					struct timeval timeout = KEEP_ALIVE;
					// Если это не зашифрованное соединение то просто выходим
					if(!http->ishttps){
						// Проверяем нужно ли держать подключение
						bool alive = http->parser->isAlive();
						// Если данные еще не удалены
						if(!alive) free_data(&http);
						// Если нужно держать подключение
						else {
							// Закрываем события
							close_events(&http);
							// Отключаемся от сервера
							close_socket(http->socServer);
							// Добавляем событие в базу
							http->evClient = event_new(http->base, http->socClient, EV_READ | EV_PERSIST, on_http_request, http);
							// Активируем событие
							event_add(http->evClient, &timeout);
						}
					// Если это зашифрованное соединение
					} else {
						// Закрываем события
						close_events(&http);
						// Создаем новое событие для клиента
						http->evClient = event_new(http->base, http->socClient, EV_TIMEOUT | EV_READ | EV_PERSIST, do_http_proxy, http);
						http->evServer = event_new(http->base, http->socServer, EV_TIMEOUT | EV_READ | EV_PERSIST, do_http_proxy, http);
						// Активируем события
						event_add(http->evClient, &timeout);
						event_add(http->evServer, &timeout);
					}
				}
			} break;
		}
	}
	// Выходим
	return;
}
*/
/**
 * prepare_request Функция обработки входящего запроса
 * @param fd   файловый дескриптор (сокет)
 * @param arg  объект передаваемый как значение
 * @param flag возникновение ошибки (либо чтение данных либо таймаут)
 */
/*
void prepare_request(evutil_socket_t fd, BufferHttpProxy ** arg, bool flag = false){
	// Выполняем приведение типов
	BufferHttpProxy * http = *arg;

	cout << " ---- " << http->request.data << " fd = " << fd << endl;

	// Выполняем парсинг полученных данных
	if((http->request.total > 0) && http->parse()){
		// Закрываем события
		close_events(&http);

		cout << " +++++ " << http->request.data << endl;

		// Устанавливаем таймаут ожидания запроса в 3 секунды
		struct timeval timeout = KEEP_ALIVE;
		// Обнуляем размер буфера
		http->request.total = 0;
		// Если это метод коннект
		bool methodConnect = (strcmp(http->parser->getMethod().c_str(), "connect") ? false : true);
		// Если авторизация не прошла
		if(!http->auth) http->auth = check_auth(http->parser);
		// Если нужно запросить пароль
		if(!http->auth && (!http->parser->getLogin().length()
		|| !http->parser->getPassword().length())){
			// Формируем ответ клиенту
			http->response = http->parser->requiredAuth();
		// Сообщаем что авторизация не удачная
		} else if(!http->auth) {
			// Формируем ответ клиенту
			http->response = http->parser->faultAuth();
		// Если авторизация прошла
		} else {
			// Определяем порт, если это шифрованное сообщение то будем считывать данные целиком
			if(methodConnect){
				// Выполняем подключение к серверу
				http->socServer = create_server_socket(http->parser->getHost().c_str(), http->parser->getPort());
				// Если сокет существует
				if(http->socServer > -1){
					// Формируем ответ клиенту
					http->response = http->parser->authSuccess();
					// Устанавливаем флаг защищенного соединения
					http->ishttps = true;
				// Если подключение не удачное то сообщаем об этом
				} else http->response = http->parser->faultConnect();
			// Иначе делаем запрос на получение данных
			} else {
				// Выполняем подключение к серверу
				http->socServer = create_server_socket(http->parser->getHost().c_str(), http->parser->getPort());
				// Сообщаем что авторизация удачная
				if(http->socServer > -1){
					
					cout << " ------------ " << http->parser->getQuery() << endl;

					// Указываем что нужно отключится сразу после отправки запроса
					if(http->parser->getVersion() > 1) http->parser->setClose();
					// Создаем новое событие для клиента
					http->evServer = event_new(http->base, http->socServer, EV_TIMEOUT | EV_WRITE | EV_PERSIST, on_http_proxy, http);
					// Активируем события
					event_add(http->evServer, &timeout);
				// Если подключение не удачное то сообщаем об этом
				} else http->response = http->parser->faultConnect();
			}
		}
		// Ответ готов
		if(http->response.length()){
			// Создаем новое событие для клиента
			http->evClient = event_new(http->base, fd, EV_TIMEOUT | EV_WRITE | EV_PERSIST, on_http_write_client, http);
			// Активируем события
			event_add(http->evClient, &timeout);
		}
	// Если это таймаут то выходим
	} else if(flag) {
		// Выводим в консоль информацию
		debug_message("Таймаут клиент, клиент так и не запросил данные!!!!");
		// Очищаем память
		free_data(&http);
	}
	// Выходим
	return;
}
*/

/**
 * prepare_request Функция обработки входящего запроса
 * @param fd   файловый дескриптор (сокет)
 * @param arg  объект передаваемый как значение
 * @param flag возникновение ошибки (либо чтение данных либо таймаут)
 */
void prepare_request(evutil_socket_t fd, BufferHttpProxy ** arg, bool flag = false){
	// Выполняем приведение типов
	BufferHttpProxy * http = *arg;
	// Выполняем парсинг полученных данных
	if((http->request.total > 0) && http->parse()){
		// Закрываем события
		close_events(&http);
		// Удаляем выделенную ранее память
		delete [] http->request.data;
		// Обнуляем размер буфера
		http->request.total = 0;
		// Устанавливаем таймаут ожидания запроса в 3 секунды
		struct timeval timeout = KEEP_ALIVE;
		// Если это метод коннект
		bool methodConnect = (strcmp(http->parser->getMethod().c_str(), "connect") ? false : true);
		// Если авторизация не прошла
		if(!http->auth) http->auth = check_auth(http->parser);
		// Если нужно запросить пароль
		if(!http->auth && (!http->parser->getLogin().length()
		|| !http->parser->getPassword().length())){
			// Формируем ответ клиенту
			http->response = http->requiredAuth();
		// Сообщаем что авторизация не удачная
		} else if(!http->auth) {
			// Формируем ответ клиенту
			http->response = http->faultAuth();
		// Если авторизация прошла
		} else {
			// Определяем порт, если это шифрованное сообщение то будем считывать данные целиком
			if(methodConnect){
				// Выполняем подключение к серверу
				http->socServer = create_server_socket(http->parser->getHost().c_str(), http->parser->getPort());
				// Если сокет существует
				if(http->socServer > -1){
					// Формируем ответ клиенту
					http->response = http->authSuccess();
					// Устанавливаем флаг защищенного соединения
					http->ishttps = true;
				// Если подключение не удачное то сообщаем об этом
				} else http->response = http->faultConnect();
			// Иначе делаем запрос на получение данных
			} else {
				// Выполняем подключение к серверу
				http->socServer = create_server_socket(http->parser->getHost().c_str(), http->parser->getPort());
				// Сообщаем что авторизация удачная
				if(http->socServer > -1){
					// Указываем что нужно отключится сразу после отправки запроса
					if(http->parser->getVersion() > 1) http->parser->setClose();
					// Формируем ответ клиенту
					http->response = http->getQuery();

					cout << " ++++++1 " << http->response->request.c_str() << endl;

					// Очищаем память
					free_data(&http);

					// Создаем новое событие для клиента
					//http->evServer = event_new(http->base, http->socServer, EV_TIMEOUT | EV_WRITE | EV_PERSIST, on_http_proxy, http);
					// Активируем события
					//event_add(http->evServer, &timeout);
					// Выходим
					return;
				// Если подключение не удачное то сообщаем об этом
				} else http->response = http->faultConnect();
			}
		}
		// Ответ готов
		if(http->response->request.length()){

			cout << " ++++++2 " << http->response->request.c_str() << endl;

			// Очищаем память
			free_data(&http);

			// Создаем новое событие для клиента
			//http->evClient = event_new(http->base, fd, EV_TIMEOUT | EV_WRITE | EV_PERSIST, on_http_write_client, http);
			// Активируем события
			//event_add(http->evClient, &timeout);
		}
	// Если это таймаут то выходим
	} else if(flag) {
		// Выводим в консоль информацию
		debug_message("Таймаут клиент, клиент так и не запросил данные!!!!");
		// Очищаем память
		free_data(&http);
	}
	// Выходим
	return;
}
/**
 * on_http_request Функция проверка запроса
 * @param fd    файловый дескриптор (сокет)
 * @param event событие на которое сработала функция обратного вызова
 * @param arg   объект передаваемый как значение
 */
void on_http_request(evutil_socket_t fd, short event, void * arg){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (arg);
	// Если подключение не передано
	if(!http) close_socket(fd);
	// Если все данные получены
	else {
		// Определяем возникшее событие
		switch(event){
			// Если это таймаут
			case 1: prepare_request(fd, &http, true); break;
			// Если это чтение
			case 2: {
				// Буфер для чтения данных из сокета
				char buffer[256];
				// Выполняем чтение данных из сокета
				int len = recv(fd, buffer, sizeof(buffer), 0);
				// Если данные не считаны то выполняем обработку входящего запроса
				if(len <= 0) prepare_request(fd, &http, true);
				// Если данные считаны нормально тогда продолжаем
				else {
					// Устанавливаем параметры по умолчанию
					http->request.total		+= len;
					http->request.offset	= 0;
					// Проверяем последние два символа
					short l1 = (short) buffer[len - 2];
					short l2 = (short) buffer[len - 1];
					// Если количество передаваемых данных превысило заложенный то расширяем максимальный размер
					if(http->request.total > http->request.size){
						// Увеличиваем размер буфера
						http->request.size *= 2;
						// Выделяем еще памяти под буфер
						http->request.data = (char *) realloc(http->request.data, http->request.size);
						// Если память выделить нельзя то выводим ошибку и выходим
						if(http->request.data == NULL){
							// Выводим в консоль информацию
							debug_message("Error in memory re-allocation!");
							// Очищаем память
							free_data(&http);
							// Выходим
							return;
						}
					}
					// Выполняем копирование данных в буфер памяти
					copy(buffer, buffer + len, http->request.data + (http->request.total - len));
					// Если это перенос строки "\r\n", выполняем обработку входящего запроса
					if(((l1 == 13) && (l2 == 10)) || (l2 == 0)) prepare_request(fd, &http, false);
				}
			} break;
		}
	}
	// Выходим
	return;
}
/**
 * on_http_connect Функция подключения к серверу
 * @param fd    файловый дескриптор (сокет)
 * @param event событие на которое сработала функция обратного вызова
 * @param arg   объект передаваемый как значение
 */
void on_http_connect(evutil_socket_t fd, short event, void * arg){
	// Время ожидания следующего запроса
	float ttl = TTL_CONNECT;
	// Устанавливаем таймаут
	if(ttl > 0) sleep(ttl);
	// Структуры получения данных сокета
	socklen_t	len = 0;
	sockaddr_in	client_addr;
	// Ожидаем подключение 3 секунд и 0 микросекунд
	struct timeval timeout = KEEP_ALIVE;
	// Accept incoming connection
	evutil_socket_t sock = accept(fd, reinterpret_cast <sockaddr *> (&client_addr), &len);
	// Если сокет не создан тогда выходим
	if(sock < 1) return;
	/* Set the client socket to non-blocking mode. */
	if(set_non_block(sock) < 0) debug_message("Failed to set client socket non-blocking");
	// Получаем объект базы событий
	struct event_base * base = reinterpret_cast <struct event_base *> (arg);
	// Создаем новый объект подключения
	BufferHttpProxy * http = new BufferHttpProxy(nameSystem);
	// Запоминаем базу событий
	http->base = base;
	// Запоминаем сокет подключения
	http->socClient = sock;
	// Добавляем событие в базу
	http->evClient = event_new(base, http->socClient, EV_READ | EV_PERSIST, on_http_request, http);
	// Активируем событие
	event_add(http->evClient, &timeout);
	// Выходим
	return;
}
/**
 * set_fd_limit Функция установки количество разрешенных файловых дескрипторов
 * @param  maxfd максимальное количество файловых дескрипторов
 * @return       количество установленных файловых дескрипторов
 */
int set_fd_limit(u_int maxfd){
	// Структура для установки лимитов
	struct rlimit lim;
	// зададим текущий лимит на кол-во открытых дискриптеров
	lim.rlim_cur = maxfd;
	// зададим максимальный лимит на кол-во открытых дискриптеров
	lim.rlim_max = maxfd;
	// установим указанное кол-во
	return setrlimit(RLIMIT_NOFILE, &lim);
}
/**
 * main Главная функция приложения
 * @param  argc длина массива параметров
 * @param  argv массив параметров
 * @return      код выхода из приложения
 */
int main(int argc, char * argv[]){
	// Статус воркера
	int status;
	// Максимальное количество воркеров
	const size_t max_works = MAX_WORKERS;
	// Наши ID процесса и сессии
	pid_t pid[max_works], sid;
	// Установим максимальное кол-во дискрипторов которое можно открыть
	set_fd_limit(MAX_SOCKETS);
	// Ответвляемся от родительского процесса
	// pid[0] = fork();
	// Если пид не создан тогда выходим
	// if(pid[0] < 0) remove_pid(EXIT_FAILURE);
	// Если с PID'ом все получилось, то родительский процесс можно завершить.
	// if(pid[0] > 0) remove_pid(EXIT_SUCCESS);
	// Изменяем файловую маску
	// umask(0);
	// Здесь можно открывать любые журналы
	// Создание нового SID для дочернего процесса
	sid = setsid();
	// Если идентификатор сессии дочернего процесса не существует
	if(sid < 0) remove_pid(EXIT_FAILURE);
	// Изменяем текущий рабочий каталог
	// if((chdir("/")) < 0) remove_pid(EXIT_FAILURE);
	// Закрываем стандартные файловые дескрипторы
	// close(STDIN_FILENO);
	// close(STDOUT_FILENO);
	// close(STDERR_FILENO);
	// Записываем пид процесса в файл
	create_pid(sid);
	// Получаем сокет
	evutil_socket_t socket = create_app_socket();
	// Проверяем все ли удачно
	if(socket == -1){
		// Выводим в консоль информацию
		debug_message("[-] Failed to create server");
		// Выходим
		return 1;
	}
	// Устанавливаем сигнал установки подключения
	signal(SIGPIPE, sigpipe_handler);	// Сигнал обрыва соединения во время записи
	signal(SIGCHLD, sigchld_handler);	// Дочерний процесс убит
	signal(SIGTERM, sigterm_handler);	// Процесс убит
	signal(SIGHUP, sighup_handler);		// Терминал потерял связь
	// Ключ по которому передаются данные между процессов
	key_t msgkey = ftok(".", getpid());
	// Получаем внешний идентификатор процесса
	int qid = msgget(msgkey, IPC_CREAT | 0660);
	// Буфер для передачи данных socket между процессами
	imess fork_buf;
	// Получаем размер буфера
	int lenfork_buf = sizeof(imess) - sizeof(long);
	// Освобождаем процесс
	msgctl(qid, IPC_RMID, 0);
	// Создаем дочерние потоки (от 1 потому что 0-й это этот же процесс)
	for(u_int i = 1; i < max_works; i++){
		// Создаем структуру для передачи сокета сервера
		fork_buf.type	= 1;		// Тип сообщения
		fork_buf.socket	= socket;	// Сокет сервера
		// Определяем тип потока
		switch(pid[i] = fork()){
			// Если поток не создан
			case -1:
				// Сообщаем что произошла ошибка потока
				perror("fork");
				// Выходим из потока
				exit(1);
			// Если это дочерний поток значит все нормально и продолжаем работу
			case 0: {
				// Выполняем получение данных сокета от родителя
				msgrcv(qid, &fork_buf, lenfork_buf, 1, 0);
				// Выводим в консоль информацию
				debug_message(string("Start service: pid = ") + to_string(getpid()) + string(", socket = ") + to_string(fork_buf.socket));
				// Создаем новую базу
				struct event_base * base = event_base_new();
				// Добавляем событие в базу
				struct event * evnt = event_new(base, fork_buf.socket, EV_READ | EV_PERSIST, on_http_connect, base);
				// Активируем событие
				event_add(evnt, NULL);
				// Активируем перебор базы событий
				event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY);
				// Закрываем сокет
				close_socket(fork_buf.socket);
				// Удаляем событие
				close_event(&evnt);
				// Удаляем базу
				event_base_free(base);
			} break;
			// Если это родитель то отправляем потомку идентификатор сокета
			default: msgsnd(qid, &fork_buf, lenfork_buf, 0);
		}
	}
	// Ждем завершение работы потомка (от 1 потому что 0-й это этот же процесс а он не может ждать завершения самого себя)
	for(u_int i = 1; i < max_works; i++){
		// Ожидаем завершения процесса
		waitpid(pid[i], &status, 0);
		// Выводим в консоль информацию
		debug_message(string("End service: pid = ") + to_string(pid[i]) + string(", status =") + to_string(WTERMSIG(status)));
	}
	// Освобождаем процесс
	msgctl(qid, IPC_RMID, 0);
	// Выходим
	remove_pid(EXIT_SUCCESS);
	// Выходим
	return 0;
}