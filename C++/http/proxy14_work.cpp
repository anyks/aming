// sudo lsof -i | grep -E LISTEN
// otool -L http14
// MacOS X
// g++ -std=c++11 -D_BSD_SOURCE -Wall -pedantic -O3 -Werror=vla -o http14 http.cpp base64.cpp proxy14.cpp -I/usr/local/include -levent = dynamic
// g++ -std=c++11 -D_BSD_SOURCE -Wall -pedantic -O3 -Werror=vla -o http14 http.cpp base64.cpp proxy14.cpp -I/usr/local/include /usr/local/opt/libevent/lib/libevent.a = static
// Linux
// g++ -std=c++11 -Wall -pedantic -O3 -Werror=vla -o http14 http.cpp base64.cpp proxy14.cpp -levent = dynamic
// g++ -std=c++11 -Wall -pedantic -O3 -Werror=vla -o http14 http.cpp base64.cpp proxy14.cpp /usr/lib/x86_64-linux-gnu/libevent.a /usr/lib/x86_64-linux-gnu/5/libstdc++.a = static
// g++ -std=c++11 -Wall -pedantic -O3 -Werror=vla -o http14 http.cpp base64.cpp proxy14.cpp /usr/lib/x86_64-linux-gnu/libevent.a /usr/lib/gcc/x86_64-linux-gnu/4.9/libstdc++.a
// FreeBSD
// clang++ -std=c++11 -D_BSD_SOURCE -Wall -pedantic -O3 -Werror=vla -o http14 http.cpp base64.cpp proxy14.cpp -I/usr/local/include /usr/local/lib/libevent.a
// tcpdump -i vtnet0 -w tcpdump.out -s 1520 port 5555
// tcpdump -n -v 'tcp[tcpflags] & (tcp-fin| tcp-rst) != 0' | grep 46.39.231.200

/*
35 - deadlock (EDEADLK), 42 - (ENOMSG)
54 - ECONNRESET Exchange full (EXFULL)
57 - Socket is not cnnected
22 - EINVAL (invalid argument)

212.3.96.221
*/

// MacOS X
// export EVENT_NOKQUEUE=1
// brew uninstall --force tmux
// brew install --HEAD tmux
#include <stdlib.h>
#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/dns.h>
//#include <event2/visibility.h>
#include <event2/event-config.h>
#include "http.h"

// Устанавливаем область видимости
using namespace std;

// Флаги базы данных событий
#define EVLOOP_ONCE				0x01
#define EVLOOP_NONBLOCK			0x02
#define EVLOOP_NO_EXIT_ON_EMPTY	0x04

// Максимальное количество клиентов
#define MAX_CLIENTS -1 // 100
// Максимальный размер буфера
#define BUFFER_WRITE_SIZE 2048
// Максимальный размер буфера для чтения http данных
#define BUFFER_READ_SIZE 4096
// Максимальное количество открытых сокетов (по дефолту в системе 1024)
#define MAX_SOCKETS 1024
// Максимальное количество воркеров
#define MAX_WORKERS 1
// Порт сервера
#define SERVER_PORT 5555
// Таймаут времени на чтение
#define READ_TIMEOUT {12, 0}
// Таймаут времени на запись
#define WRITE_TIMEOUT {15, 0}
// Таймаут ожидания коннекта
#define KEEP_ALIVE_TIMEOUT {10, 0}

// Сервера
map <string, evutil_socket_t> servers;

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
		bool				auth = false;	// Флаг авторизации
		struct event_base	* base;			// База событий
		Http				* parser;		// Объект парсера
		Http::HttpQuery		response;		// Ответ системы
		Request				request;		// Данные запроса
		Events				events;			// Буферы событий
		Server				server;			// Параметры удаленного сервера
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
		 * @param [string] name имя ресурса
		 */
		BufferHttpProxy(string name){
			// Создаем объект для работы с http заголовками
			parser = new Http(name);
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
 * sigpipe_handler Функция обработки сигнала SIGPIPE
 * @param signum номер сигнала
 */
void sigpipe_handler(int signum){
	// Выводим в консоль информацию
	cout << "Сигнал попытки записи в отключенный сокет!!!!" << endl;
}
/**
 * sigchld_handler Функция обработки сигнала о появившемся зомби процессе SIGCHLD
 * @param signum номер сигнала
 */
void sigchld_handler(int signum) {
	// Выводим в консоль информацию
	cout << "Дочерний процесс убит!!!!" << endl;
	// Избавляемся от зависших процессов
	while(waitpid(-1, NULL, WNOHANG) > 0);
}
/**
 * sigterm_handler Функция обработки сигналов завершения процессов SIGTERM
 * @param signum номер сигнала
 */
void sigterm_handler(int signum){
	// Выводим в консоль информацию
	cout << "Процесс убит!!!!" << endl;
	// Выходим
	exit(0);
}
/**
 * sighup_handler Функция обработки сигналов потери терминала SIGHUP
 * @param signum номер сигнала
 */
void sighup_handler(int signum){
	// Выводим в консоль информацию
	cout << "Терминал потерял связь!!!" << endl;
}
/**
 * set_non_block Функция отключения алгоритма Нейгла
 * @param fd файловый дескриптор (сокет)
 * @return   результат работы функции
 */
int set_tcpnodelay(evutil_socket_t fd){
	// Устанавливаем параметр
	int tcpnodelay = 1;
	// Устанавливаем TCP_NODELAY
	if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &tcpnodelay, sizeof(tcpnodelay)) < 0){
		// Выводим в консоль информацию
		printf("Cannot set TCP_NODELAY option on listen socket.\n");
		// Выходим
		return -1;
	}
	// Все удачно
	return 0;
}
/**
 * set_buffer_size Функция установки размеров буфера
 * @param  fd         файловый дескриптор (сокет)
 * @param  read_size  размер буфера на чтение
 * @param  write_size размер буфера на запись
 * @return            результат работы функции
 */
int set_buffer_size(evutil_socket_t fd, int read_size, int write_size){
	// Определяем размер массива опции
	socklen_t read_optlen	= sizeof(read_size);
	socklen_t write_optlen	= sizeof(write_size);
	// Устанавливаем размер буфера для сокета клиента и сервера
	if((setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &read_size, read_optlen) < 0)
	|| (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &write_size, write_optlen) < 0)){
		// Выводим в консоль информацию
		printf("Set buffer wrong!!!!\n");
		// Выходим
		return -1;
	}
	// Считываем установленный размер буфера
	if((getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &read_size, &read_optlen) < 0)
	|| (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &write_size, &write_optlen) < 0)){
		// Выводим в консоль информацию
		printf("Get buffer wrong!!!!\n");
		// Выходим
		return -1;
	}
	// Все удачно
	return 0;
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
 * appendToBuffer Функция добавления в буфер новых данных
 * @param data       ссылка на буфер данных
 * @param chunk_size размер одной порции данных
 * @param buffer     буфер с входящими данными
 */
void appendToBuffer(vector <char> &data, size_t chunk_size, const char * buffer){
	// Определяем количество общих данных
	size_t total = (!data.empty() ? data.size() - 1 : 0);
	// Устанавливаем параметры по умолчанию
	total += chunk_size;
	// Выполняем увеличение размера вектора
	data.resize(total + 1);
	// Вспомогательные переменные для заполнения вектора данными
	size_t i = total - chunk_size, j = 0;
	// Добавляем все полученные байты в массив
	while(i < total){
		// Заполняем вектор полученными символами
		data[i] = (char) buffer[j];
		i++;	// Изменяем смещение по вектору
		j++;	// Изменяем смещение по буферу данных
	}
	// Добавляем нулевой символ
	data[total] = '\0';
}


static void event_cb(struct bufferevent * bev, short events, void * ctx);
static void read_server_cb(struct bufferevent * bev, void * ctx);

/**
 * connect_server Функция создания сокета для подключения к удаленному серверу
 * @param  arg объект подключения
 * @return     результат подключения
 */
bool connect_server(BufferHttpProxy ** arg){
	// Получаем объект подключения
	BufferHttpProxy * http = *arg;
	// Если данные существуют
	if(http){
		// Получаем данные хоста
		hostent * sh = gethostbyname(http->parser->getHost().c_str());
		// Если данные хоста найдены
		if(sh){
			// ip адрес ресурса
			string ip;
			// Получаем порт сервера
			u_int port = http->parser->getPort();
			// Извлекаем ip адрес
			for(u_int i = 0; sh->h_addr_list[i] != 0; ++i){
				struct in_addr addr;
				memcpy(&addr, sh->h_addr_list[i], sizeof(struct in_addr));
				ip = inet_ntoa(addr);
			}
			// Если хост и порт сервера не совпадают тогда очищаем данные
			if((http->events.server != NULL)
			&& ((http->server.host != ip)
			|| (http->server.port != port))) http->free_server();
			// Если сервер еще не подключен
			if(http->events.server == NULL){
				// Запоминаем хост и порт сервера
				http->server.host = ip;
				http->server.port = port;
				// Буфер порта
				char port_buf[6];
				// Сокет подключения
				evutil_socket_t sock = -1;
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
				// We want a TCP socket
				param.ai_protocol = IPPROTO_TCP;
				// Only return addresses we can use.
				param.ai_flags = EVUTIL_AI_ADDRCONFIG;
				// Convert the port to decimal.
				evutil_snprintf(port_buf, sizeof(port_buf), "%d", (int) port);
				// Если формат подключения указан не верно то сообщаем об этом
				if(evutil_getaddrinfo(ip.c_str(), port_buf, &param,  &req)){
					// Выводим в консоль информацию
					printf("Error in server address format! %s\n", ip.c_str());
					// Выходим
					return false;
				}
				// Создаем сокет, если сокет не создан то сообщаем об этом
				if((sock = socket(req->ai_family, req->ai_socktype, req->ai_protocol)) < 0){
					// Выводим в консоль информацию
					printf("Error in creating socket to server!\n");
					// Выходим
					return false;
				}
				// Если сокет не создан то выходим
				if(sock < 0) return false;
				// Создаем буфер событий для сервера
				http->events.server = bufferevent_socket_new(http->base, sock, BEV_OPT_CLOSE_ON_FREE);
				// Устанавливаем таймаут ожидания запроса
				struct timeval timeout_read		= READ_TIMEOUT;
				struct timeval timeout_write	= WRITE_TIMEOUT;
				// Устанавливаем таймауты
				bufferevent_set_timeouts(http->events.server, &timeout_read, &timeout_write);
				// Устанавливаем коллбеки
				bufferevent_setcb(http->events.server, read_server_cb, NULL, event_cb, http);
				// Активируем буферы событий на чтение и запись
				bufferevent_enable(http->events.server, EV_READ);
				// Очищаем буферы событий при завершении работы
				bufferevent_flush(http->events.server, EV_READ, BEV_FINISHED);
				// Выполняем подключение к удаленному серверу, если подключение не выполненно то сообщаем об этом
				if(bufferevent_socket_connect(http->events.server, req->ai_addr, req->ai_addrlen) < 0){
					// Выводим в консоль информацию
					printf("Error in connecting to server!\n");
					// Очищаем буфер сервера
					http->free_server();
					// Выходим
					return false;
				}
				// И освобождаем связанный список
				evutil_freeaddrinfo(req);
				// Если подключение постоянное
				if(http->parser->isAlive()){
					// Получаем сокеты
					evutil_socket_t server_fd = bufferevent_getfd(http->events.server);
					evutil_socket_t client_fd = bufferevent_getfd(http->events.client);
					// Устанавливаем TCP_NODELAY для сервера и клиента
					set_tcpnodelay(server_fd);
					set_tcpnodelay(client_fd);
				}
				// Выводим в консоль сообщение о новом коннекте
				printf("connect to host = %s [%s:%d] path = %s\n", http->parser->getHost().c_str(), http->server.host.c_str(), http->server.port, http->parser->getPath().c_str());
				// Сообщаем что все удачно
				return true;
			// Если сервер уже подключен, сообщаем что все удачно
			} else return true;
		// Очищаем буфер, если он существует
		} else http->free_server();
	}
	// Выходим
	return false;
}
/**
 * event_cb Функция обработка входящих событий
 * @param bev    буфер события
 * @param events произошедшее событие
 * @param ctx    объект входящих данных
 */
static void event_cb(struct bufferevent * bev, short events, void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если объект существует
	if(http){
		// Получаем текущий сокет
		evutil_socket_t current_fd = bufferevent_getfd(bev);
		// Получаем сокет клиента
		evutil_socket_t client_fd = bufferevent_getfd(http->events.client);
		// Определяем для кого вызвано событие
		string subject = (current_fd == client_fd ? "client" : "server");
		// Если подключение удачное
		if(events & BEV_EVENT_CONNECTED){
			// Выводим сообщение в консоль
			printf("Connect %s okay.\n", subject.c_str());
		// Если это ошибка или завершение работы
		} else if(events & (BEV_EVENT_ERROR | BEV_EVENT_EOF | BEV_EVENT_TIMEOUT)) {
			// Если это ошибка
			if(events & BEV_EVENT_ERROR){
				// Получаем данные ошибки
				int err = bufferevent_socket_get_dns_error(bev);
				// Если ошибка существует, выводим сообщение в консоль
				if(err) printf("DNS error: %s\n", evutil_gai_strerror(err));
			}
			// Сообщаем что произошло отключение
			printf("Closing %s\n", subject.c_str());
			// Отключаемся
			free_data(&http);
		}
	// Отключаемся
	} else bufferevent_free(bev);
	// Выходим
	return;
}
/**
 * read_server_cb Функция чтения данных с сокета сервера
 * @param bev буфер события
 * @param ctx передаваемый объект
 */
static void read_server_cb(struct bufferevent * bev, void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http != NULL){
		// Получаем буферы входящих данных и исходящих
		struct evbuffer * input = bufferevent_get_input(http->events.server);
		struct evbuffer * output = bufferevent_get_output(http->events.client);
		// Выводим ответ сервера
		if(evbuffer_add_buffer(output, input) < 0) free_data(&http);
	}
	// Выходим
	return;
}
/**
 * read_client_cb Функция чтения данных с сокета клиента
 * @param bev буфер события
 * @param ctx передаваемый объект
 */
static void read_client_cb(struct bufferevent * bev, void * ctx){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (ctx);
	// Если подключение не передано
	if(http != NULL){
		// Получаем буферы входящих данных и исходящих
		struct evbuffer * input = bufferevent_get_input(bev);
		// Если авторизация прошла, и коннект произведен
		if((http->response.code == 200) && http->parser->isConnect()){
			// Получаем буферы входящих данных и исходящих
			struct evbuffer * output = bufferevent_get_output(http->events.server);
			// Выводим ответ сервера
			if(evbuffer_add_buffer(output, input) < 0) free_data(&http);
		// Если это обычный запрос
		} else {
			// Получаем размер входящих данных
			size_t len = evbuffer_get_length(input);
			// Создаем буфер данных
			char * buffer = new char[len];
			// Копируем в буфер полученные данные
			evbuffer_copyout(input, buffer, len);
			// Склеиваем полученные данные
			appendToBuffer(http->request.data, len, buffer);
			// Удаляем данные из буфера
			evbuffer_drain(input, len);
			// Удаляем буфер данных
			delete [] buffer;
			// Выполняем парсинг полученных данных
			if(!http->request.data.empty() && http->parse()){
				// Скидываем количество отправляемых данных
				http->request.offset = 0;
				// Очищаем буфер данных
				http->request.data.clear();
				// Очищаем объект ответа
				http->response.clear();
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
					// Если сокет существует
					if(connect_server(&http)){
						// Определяем порт, если это метод connect
						if(http->parser->isConnect())
							// Формируем ответ клиенту
							http->response = http->parser->authSuccess();
						// Иначе делаем запрос на получение данных
						else {
							// Указываем что нужно отключится сразу после отправки запроса
							if(!http->parser->isAlive()) http->parser->setClose();
							// Формируем запрос на сервер
							http->response = http->parser->getQuery();
							// Отправляем серверу сообщение
							if(bufferevent_write(http->events.server, http->response.data(), http->response.size()) < 0) free_data(&http);
							// Выходим
							return;
						}
					// Если подключение не удачное то сообщаем об этом
					} else http->response = http->parser->faultConnect();
				}
				// Ответ готов
				if(!http->response.empty()){
					// Отправляем клиенту сообщение
					if(bufferevent_write(bev, http->response.data(), http->response.size()) < 0) free_data(&http);
				// Отключаемся
				} else free_data(&http);
			}
		}
	}
	// Выходим
	return;
}
/**
 * accept_connect Событие подключения к серверу
 * @param listener объект подключения
 * @param fd       файловый дескриптор (сокет) клиента
 * @param address  адрес клиента
 * @param socklen  размер входящих данных
 * @param ctx      передаваемый объект
 */
static void accept_connect(struct evconnlistener * listener, evutil_socket_t fd, struct sockaddr * address, int socklen, void * ctx){
	// Получаем базу событий
	struct event_base * base = evconnlistener_get_base(listener);
	// Создаем новый объект подключения
	BufferHttpProxy * http = new BufferHttpProxy("anyks");
	// Запоминаем базу событий
	http->base = base;
	// Создаем буфер событий
	http->events.client = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	// Устанавливаем размеры буферов
	set_buffer_size(fd, BUFFER_READ_SIZE, BUFFER_WRITE_SIZE);
	// Устанавливаем таймаут ожидания запроса
	struct timeval timeout_read		= KEEP_ALIVE_TIMEOUT;
	struct timeval timeout_write	= WRITE_TIMEOUT;
	// Устанавливаем таймауты
	bufferevent_set_timeouts(http->events.client, &timeout_read, &timeout_write);
	// Устанавливаем коллбеки
	bufferevent_setcb(http->events.client, read_client_cb, NULL, event_cb, http);
	// Активируем буферы событий на чтение и запись
	bufferevent_enable(http->events.client, EV_READ);
	// Очищаем буферы событий при завершении работы
	bufferevent_flush(http->events.client, EV_READ, BEV_FINISHED);
	// Выходим
	return;
}
/**
 * accept_error Событие возникновения ошибки подключения
 * @param listener объект подключения
 * @param ctx      передаваемый объект
 */
static void accept_error(struct evconnlistener * listener, void * ctx){
	// Получаем базу событий
	struct event_base * base = evconnlistener_get_base(listener);
	// Получаем сообщение об ошибке
	int err = EVUTIL_SOCKET_ERROR();
	// Выводим сообщение в консоль
	fprintf(stderr, "Got an error %d (%s) on the listener. Shutting down.\n", err, evutil_socket_error_to_string(err));
	// Удаляем базу событий
	event_base_loopexit(base, NULL);
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
	// Установим максимальное кол-во дискрипторов которое можно открыть
	set_fd_limit(MAX_SOCKETS);
	// Структура для создания сервера приложения
	struct sockaddr_in sin;
	// Создаем новую базу
	struct event_base * base = event_base_new();
	// Структура определяющая параметры сервера приложений
	struct hostent * server = gethostbyname("0.0.0.0");
	// Очищаем всю структуру
	memset(&sin, 0, sizeof(sin));
	// Listen on 0.0.0.0
	// sin.sin_addr.s_addr = htonl(0);
	// This is an INET address
	sin.sin_family = AF_INET;
	// Указываем адрес прокси сервера
	sin.sin_addr.s_addr = *((unsigned long *) server->h_addr);
	// Listen on the given port.
	sin.sin_port = htons(SERVER_PORT);
	// Устанавливаем сигнал установки подключения
	signal(SIGPIPE, sigpipe_handler);	// Сигнал обрыва соединения во время записи
	signal(SIGCHLD, sigchld_handler);	// Дочерний процесс убит
	signal(SIGTERM, sigterm_handler);	// Процесс убит
	signal(SIGHUP, sighup_handler);		// Терминал потерял связь
	// Вешаем приложение на порт
	struct evconnlistener * listener = evconnlistener_new_bind(
		base, accept_connect, NULL,
		LEV_OPT_REUSEABLE |
		LEV_OPT_THREADSAFE |
		LEV_OPT_CLOSE_ON_FREE |
		LEV_OPT_LEAVE_SOCKETS_BLOCKING,
		MAX_CLIENTS, (struct sockaddr *) &sin, sizeof(sin)
	);
	// Если подключение не удалось
	if(!listener){
		// Выводим сообщение об ошибке
		perror("Couldn't create listener");
		// Выходим
		return 1;
	}
	// Получаем сокет приложения
	evutil_socket_t socket = evconnlistener_get_fd(listener);
	// Устанавливаем неблокирующий режим
	set_tcpnodelay(socket);
	// Размер буфера на чтение
	int buffer_read_size = ((MAX_CLIENTS > 0 ? MAX_CLIENTS : 1) * BUFFER_READ_SIZE * MAX_WORKERS);
	// Размер буфера на запись
	int buffer_write_size = ((MAX_CLIENTS > 0 ? MAX_CLIENTS : 1) * BUFFER_WRITE_SIZE * MAX_WORKERS);
	// Устанавливаем размеры буферов
	set_buffer_size(socket, buffer_read_size, buffer_write_size);
	// Устанавливаем обработчик на получение ошибок
	evconnlistener_set_error_cb(listener, accept_error);
	// Активируем перебор базы событий
	event_base_dispatch(base);
	// Активируем перебор базы событий
	// event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY);
	// Выходим
	return 0;
}