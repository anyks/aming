// sudo lsof -i | grep -E LISTEN
// otool -L http9
// MacOS X
// g++ -std=c++11 -D_BSD_SOURCE -Wall -pedantic -O3 -Werror=vla -o http9 http.cpp base64.cpp proxy9.cpp -I/usr/local/include -levent = dynamic
// g++ -std=c++11 -D_BSD_SOURCE -Wall -pedantic -O3 -Werror=vla -o http9 http.cpp base64.cpp proxy9.cpp -I/usr/local/include /usr/local/opt/libevent/lib/libevent.a = static
// Linux
// g++ -std=c++11 -Wall -pedantic -O3 -Werror=vla -o http9 http.cpp base64.cpp proxy9.cpp -levent = dynamic
// g++ -std=c++11 -Wall -pedantic -O3 -Werror=vla -o http9 http.cpp base64.cpp proxy9.cpp /usr/lib/x86_64-linux-gnu/libevent.a /usr/lib/x86_64-linux-gnu/5/libstdc++.a = static
// g++ -std=c++11 -Wall -pedantic -O3 -Werror=vla -o http9 http.cpp base64.cpp proxy9.cpp /usr/lib/x86_64-linux-gnu/libevent.a /usr/lib/gcc/x86_64-linux-gnu/4.9/libstdc++.a
// FreeBSD
// clang++ -std=c++11 -D_BSD_SOURCE -Wall -pedantic -O3 -Werror=vla -o http9 http.cpp base64.cpp proxy9.cpp -I/usr/local/include /usr/local/lib/libevent.a

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
#include <sys/signal.h>
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
// Максимальный размер буфера
#define BUFFER_SIZE 50
// Максимальное количество открытых сокетов (по дефолту в системе 1024)
#define MAX_SOCKETS 1024
// Максимальное количество воркеров
#define MAX_WORKERS 4
// Таймаут ожидания для http 1.1
#define KEEP_ALIVE {5, 0}
// Таймаут ожидания коннекта
#define CONNECT_TIMEOUT {30, 0}

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
			vector <char>	data;			// Данные в буфере
			size_t			offset	= 0;	// Смещение в буфере
		};
		// Обработчики событий сервера
		struct Events {
			struct event * client = NULL;	// Событие клиента
			struct event * server = NULL;	// Событие сервера
		};
		// Файловые дескрипторы
		struct Sockets {
			evutil_socket_t client = -1;	// Сокет клиента
			evutil_socket_t server = -1;	// Сокет сервера
		};
		// Данные текущего сервера
		struct Server {
			u_int	port;	// Порт сервера
			string	host;	// Хост сервера
		};
	public:
		int					pid		= -1;		// Пид процесса
		bool				auth	= false;	// Флаг авторизации
		struct event_base	* base;				// База событий
		Http				* parser;			// Объект парсера
		Events				evs;				// Работа событий
		Sockets				fds;				// Сокеты
		Server				server;				// Параметры удаленного сервера
		Http::HttpQuery		response;			// Ответ системы
		Buffer				request;			// Данные запроса
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
			// Отключаем от сервиса (disconnect)
			if(fds.server >= 0){
				shutdown(fds.server, SHUT_RDWR);
				close(fds.server);
				fds.server = -1;
			}
			if(fds.client >= 0){
				shutdown(fds.client, SHUT_RDWR);
				close(fds.client);
				fds.client = -1;
			}
			// Удаляем события
			if(evs.server && evs.server->ev_base) event_free(evs.server);
			else if(evs.server && (evs.server->ev_base == NULL)) delete evs.server;
			if(evs.client && evs.client->ev_base) event_free(evs.client);
			else if(evs.client && (evs.client->ev_base == NULL)) delete evs.client;
			// Удаляем указатели
			evs.server = NULL;
			evs.client = NULL;
			// Если парсер не удален
			if(parser){
				// Удаляем парсер
				delete parser;
				// Запоминаем что данные удалены
				parser = NULL;
			}
			// Очищаем память выделенную для вектора
			vector <char> ().swap(request.data);
			vector <char> ().swap(response.entitybody);
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
	debug_message("Сигнал попытки записи в отключенный сокет!!!!");
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
	if(*ev){
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
	if(http){
		// Очищаем событие для сервера
		close_event(&http->evs.server);
		// Очищаем событие для клиента
		close_event(&http->evs.client);
	}
}
/**
 * free_data Функция очистки памяти для http прокси
 * @param arg объект для очистки
 */
void free_data(BufferHttpProxy ** http){
	// Если данные еще не удалены
	if(*http){
		// Удаляем объект данных
		delete *http;
		// Присваиваем пустой адрес
		*http = NULL;
	}
}
/**
 * create_app_socket Функция создания сокета системы
 * @return файловый дескриптор (сокет)
 */
evutil_socket_t create_app_socket(){
	// Сокет сервера
	evutil_socket_t sock;
	// Максимальное число клиентов
	int maxpending = MAX_CLIENTS;
	// Структура для сервера
	struct sockaddr_in echoserver;
	// Создаем сокет
	if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
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
	echoserver.sin_port = htons(5555); // htons(SERVER_PORT);
	
	
	/*
	// Устанавливаемое значение
	int optval = 1;
	// Устанавливаем TCP_NODELAY
	if(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(int)) < 0){
		// Выводим в консоль информацию
		debug_message("Cannot set TCP_NODELAY option on listen socket.");
		// Выходим
		return -1;
	}
	*/

	// Маскируем ошибку о сигнале (записи в отключенный сокет)
	//int n = 1;
	//setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, &n, sizeof(n));

	// Выполняем биндинг сокета // ::bind (для мака)
	if(::bind(sock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0){
		// Выводим в консоль информацию
		debug_message("[-] Bind error.");
		// Выходим
		return -1;
	}
	// Выполняем чтение сокета
	if(listen(sock, maxpending) < 0){
		// Выводим в консоль информацию
		debug_message("[-] Listen error.");
		// Выходим
		return -1;
	}
	// Указываем что сокет не блокирующий
	if(set_non_block(sock) < 0){
		// Выводим в консоль информацию
		debug_message("[-] Failed to set server socket to non-blocking.");
		// Выходим
		return -1;
	}
	// Выводим сокет системы
	return sock;
}
/**
 * create_server_socket Функция создания сокета для подключения к удаленному серверу
 * @param  host хост сервера
 * @param  port порт сервера
 * @return      файловый дескриптор (сокет)
 */
evutil_socket_t create_server_socket(const char * host, int port){
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
	

	/*
	// Устанавливаемое значение
	int optval = 1;
	// Устанавливаем TCP_NODELAY
	if(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(int)) < 0){
		// Выводим в консоль информацию
		debug_message("Cannot set TCP_NODELAY option on listen socket.");
		// Выходим
		return -1;
	}
	*/

	
	// Указываем что сокет не блокирующий
	if(set_non_block(sock) < 0) debug_message("Failed to set server socket non-blocking");
	// Выводим созданный нами сокет
	return sock;
	
	/*
	// Структура определяющая тип адреса
	struct sockaddr_in serv_addr;
	// Структура определяющая параметры удаленного сервера
	struct hostent * server;
	// Выполняем создание сокета для подключения к удаленному серверу
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	// Если сокет не создан тогда выходим
	if(sockfd < 0) return -1;
	// Заполняем структуру типа адреса нулями
	bzero((char *) &serv_addr, sizeof(serv_addr));
	// Устанавливаем что удаленный адрес это ИНТЕРНЕТ
	serv_addr.sin_family = AF_INET;
	// Получаем данные хоста удаленного сервера по его названию
	server = gethostbyname(host);
	// Если хост не получен тогда разблокируем поток и выходим
	if(!server) return -1;
	// Выполняем копирование данных типа подключения
	bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
	// Устанавливаем порт для подключения к удаленному серверу
	serv_addr.sin_port = htons(port);
	// Выполняем коннект к серверу и получаем из него данные
	return (!connect(sockfd, (const sockaddr *) &serv_addr, sizeof(serv_addr)) ? sockfd : -1);
	*/
}
/**
 * connect_server Функция подключения к серверу
 * @param  arg объект подключения
 * @return     сокет сервера
 */
evutil_socket_t connect_server(BufferHttpProxy ** arg){
	// Получаем объект подключения
	BufferHttpProxy * http = *arg;
	// Сокет подключения
	evutil_socket_t sock = -1;
	// Если данные существуют
	if(http){
		// Отключаем событие для сервера
		close_event(&http->evs.server);
		// ip адрес ресурса
		string ip;
		// Порт ресурса
		u_int port = http->parser->getPort();
		// Получаем данные хоста
		hostent * sh = gethostbyname(http->parser->getHost().c_str());
		// Если данные хоста найдены
		if(sh){
			// Извлекаем ip адрес
			for(u_int i = 0; sh->h_addr_list[i] != 0; ++i){
				struct in_addr addr;
				memcpy(&addr, sh->h_addr_list[i], sizeof(struct in_addr));
				ip = inet_ntoa(addr);
			}
			// Если хост и порт сервера не совпадают
			if((http->server.host != ip) || (http->server.port != port)){
				// Отключаемся от сервера
				close_socket(http->fds.server);
			}
			// Получаем хост сервера
			http->server.host = ip;
			// Получаем порт сервера
			http->server.port = port;
			// Если данные адреса найдены
			if(!http->server.host.empty() && http->server.port){
				// Выводим в консоль данные о подключении
				if(http->fds.server < 0) debug_message(string("connect to host = ") + http->parser->getHost() + " [" + http->server.host + string(":") + to_string(http->server.port) + "]");
				// Выполняем подключение к серверу
				if(http->fds.server < 0) sock = create_server_socket(http->server.host.c_str(), http->server.port);
				// Иначе возвращаем сокет самого сервера
				else sock = http->fds.server;
			}
		}
	}
	// Выводим сокет
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
 * appendToBuffer Функция добавления в буфер новых данных
 * @param data       ссылка на буфер данных
 * @param chunk_size размер одной порции данных
 * @param buffer     буфер с входящими данными
 */
void appendToBuffer(vector <char> & data, size_t chunk_size, const char * buffer){
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
/**
 * on_http_connect Прототип функции подключения к серверу
 * @param fd    файловый дескриптор (сокет)
 * @param event событие на которое сработала функция обратного вызова
 * @param arg   объект передаваемый как значение
 */
void on_http_connect(evutil_socket_t fd, short event, void * arg);
/**
 * on_http_request Прототип функции проверка запроса
 * @param fd    файловый дескриптор (сокет)
 * @param event событие на которое сработала функция обратного вызова
 * @param arg   объект передаваемый как значение
 */
void on_http_request(evutil_socket_t fd, short event, void * arg);
/**
 * on_http_write_client Прототип функции вывода ответа клиенту
 * @param fd    файловый дескриптор (сокет)
 * @param event событие на которое сработала функция обратного вызова
 * @param arg   объект передаваемый как значение
 */
void on_http_write_client(evutil_socket_t fd, short event, void * arg);
/*
 * do_http_proxy Функция обмена сообщениями между клиентом и сервером (https)
 * @param fd    файловый дескриптор (сокет)
 * @param event событие на которое сработала функция обратного вызова
 * @param arg   объект передаваемый как значение
 */
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
				debug_message("Timeout https connect!!!!");
				// Если это клиент то удаляем событие
				if(fd == http->fds.client)
					// Отключаем событие для клиента
					close_event(&http->evs.client);
				// Закрываем соединение
				else free_data(&http);
			} break;
			// Если это чтение данных
			case 2: {
				// Буфер для чтения данных из сокета
				char buffer[BUFFER_SIZE];
				// Выполняем чтение данных из сокета
				int len = recv(fd, buffer, sizeof(buffer), 0);
				// Получаем сокет для ответа
				evutil_socket_t socket = (fd != http->fds.server ? http->fds.server : http->fds.client);
				// Если данные не считаны значит клиент отключился
				if((len <= 0) || (socket < 0)
				// Если при записи данных получаем ошибку, значит отключаемся
				|| ((len = send(socket, (void *) buffer, len, 0)) <= 0)) free_data(&http);

			} break;
		}
	}
	// Выходим
	return;
}
/**
 * on_http_read_server Функция обработки события чтения данных из сокета сервера
 * @param fd    файловый дескриптор (сокет)
 * @param event событие на которое сработала функция обратного вызова
 * @param arg   объект передаваемый как значение
 */
void on_http_read_server(evutil_socket_t fd, short event, void * arg){
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
				debug_message("Timeout server!!!!");
				// Сообщаем что сервер отключился по таймауту
				free_data(&http);
			} break;
			// Если это чтение
			case 2: {
				// Буфер для чтения данных из сокета
				char buffer[BUFFER_SIZE];
				// Выполняем чтение данных из сокета
				int len = recv(fd, buffer, sizeof(buffer), 0);
				// Если данные не считаны значит клиент отключился
				if(len <= 0) free_data(&http);
				// Выполняем проверку прислали все данные или нет
				else if(http->fds.client > -1) {
					// Если данные не могут быть отправлены тогда выходим
					if((len = send(http->fds.client, (void *) buffer, len, 0)) <= 0) free_data(&http);
					// Если данные удачно отправлены
					else {
						// Склеиваем полученные данные
						appendToBuffer(http->request.data, len, buffer);
						// Проверяем все ли данные переданы
						if(http->parser->checkEnd(
							http->request.data.data(),
							http->request.data.size() - 1
						).type){
							// Закрываем события
							close_events(&http);
							// Очищаем буфер данных
							http->request.data.clear();
							// Если подключение держать не надо тогда выходим
							if(!http->parser->isAlive()) free_data(&http);
							// Если нужно держать подключение
							else {
								// Очищаем введенные ранее данные
								http->parser->clear();
								// Добавляем событие в базу
								http->evs.client = event_new(http->base, http->fds.client, EV_READ | EV_PERSIST, on_http_request, http);
								// Устанавливаем таймаут ожидания запроса в 3 секунды
								struct timeval timeout = CONNECT_TIMEOUT;
								// Активируем событие
								event_add(http->evs.client, &timeout);
							}
						}
					}
				// Иначе выходим
				} else free_data(&http);
			} break;
		}
	}
}
/**
 * on_http_write_server Функция обработки события записи данных в сокет сервера
 * @param fd    файловый дескриптор (сокет)
 * @param event событие на которое сработала функция обратного вызова
 * @param arg   объект передаваемый как значение
 */
void on_http_write_server(evutil_socket_t fd, short event, void * arg){
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
				debug_message("Timeout server!!!!");
				// Выполняем очистку подключения
				free_data(&http);
			} break;
			// Если это запись
			case 4: {
				// Определяем длину ответа
				size_t len_data = http->response.mess.length();
				// Определяем сколько данных уже отправлено
				size_t send_size = len_data - http->request.offset;
				// Определяем сколько данных нужно отправить
				if(send_size > BUFFER_SIZE) send_size = BUFFER_SIZE;
				// Если количество отправляемых данных больше 0 то отправляем
				if(send_size > 0){
					// Отправляем запрос на сервер
					int len = send(fd, (void *) (http->response.mess.c_str() + http->request.offset), send_size, 0);
					// Если данные не отправились то выходим
					if(len <= 0){
						// Выводим в консоль информацию
						debug_message("Server disconnect, broken write!!!!");
						// Выполняем очистку подключения
						free_data(&http);
						// Выходим
						break;
					}
					// Увеличиваем количество отправленных данных
					http->request.offset += len;
				// Если все данные отправлены
				} else {
					// Закрываем события
					close_events(&http);
					// Если вложения присутствуют то отправляем и их
					if(!http->response.entitybody.empty()){
						// Количество отправленных данных
						int send_body = 0;
						// Определяем длину вложений
						size_t totalsend_body = 0;
						// Определяем общее количество данных
						size_t size_body = http->response.entitybody.size();
						// Получаем данные отправляемых вложений
						const char * data = http->response.entitybody.data();
						// Определяем сколько данных уже отправлено
						send_size = size_body - totalsend_body;
						// Определяем сколько данных нужно отправить
						if(send_size > BUFFER_SIZE) send_size = BUFFER_SIZE;
						// Выполняем отправку до тех пор пока все данные не отправлены
						while(totalsend_body < size_body){
							// Выполняем отправку данных
							if((send_body = send(fd, (void *) (data + totalsend_body), send_size, 0)) <= 0){
								// Отключаемся от сервера
								free_data(&http);
								// Выходим из функции
								return;
							}
							// Запоминаем количество отправленных данных
							totalsend_body += send_body;
						}
					}
					// Создаем новое событие для чтения данных с сервера
					http->evs.server = event_new(http->base, http->fds.server, EV_TIMEOUT | EV_READ | EV_PERSIST, on_http_read_server, http);
					// Устанавливаем таймаут ожидания запроса в 3 секунды
					struct timeval timeout = KEEP_ALIVE;
					// Активируем события
					event_add(http->evs.server, &timeout);
				}
			} break;
		}
	}
	// Выходим
	return;
}
/**
 * on_http_write_client Функция обработки события записи данных в сокет клиента
 * @param fd    файловый дескриптор (сокет)
 * @param event событие на которое сработала функция обратного вызова
 * @param arg   объект передаваемый как значение
 */
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
				debug_message("Timeout client!!!!");
				// Выполняем очистку подключения
				free_data(&http);
			} break;
			// Если это запись
			case 4: {
				// Определяем длину ответа
				size_t len_data = http->response.mess.length();
				// Определяем сколько данных уже отправлено
				size_t send_size = len_data - http->request.offset;
				// Определяем сколько данных нужно отправить
				if(send_size > BUFFER_SIZE) send_size = BUFFER_SIZE;
				// Если количество отправляемых данных больше 0 то отправляем
				if(send_size > 0){
					// Отправляем запрос на сервер
					int len = send(fd, (void *) (http->response.mess.c_str() + http->request.offset), send_size, 0);
					// Если данные не отправились то выходим
					if(len <= 0){
						// Выводим в консоль информацию
						debug_message("Client disconnect, broken write!!!!");
						// Выполняем очистку подключения
						free_data(&http);
						// Выходим
						break;
					}
					// Увеличиваем количество отправленных данных
					http->request.offset += len;
				// Если все данные отправлены
				} else {
					// Если авторизация прошла
					if(http->response.code == 200){
						// Если это защищенное подключение
						if(http->parser->isConnect()){
							// Закрываем события
							close_events(&http);
							// Создаем новое событие для клиента
							http->evs.client = event_new(http->base, http->fds.client, EV_TIMEOUT | EV_READ | EV_PERSIST, do_http_proxy, http);
							http->evs.server = event_new(http->base, http->fds.server, EV_TIMEOUT | EV_READ | EV_PERSIST, do_http_proxy, http);
							// Устанавливаем таймаут ожидания запроса в 3 секунды
							struct timeval timeout = KEEP_ALIVE;
							// Активируем события
							event_add(http->evs.client, &timeout);
							event_add(http->evs.server, &timeout);
							// Выходим из функции
							return;
						}
					}
					// Отключаемся от клиента
					free_data(&http);
				}
			} break;
		}
	}
	// Выходим
	return;
}
/**
 * prepare_request Функция обработки входящего запроса
 * @param fd   файловый дескриптор (сокет)
 * @param arg  объект передаваемый как значение
 */
void prepare_request(evutil_socket_t fd, BufferHttpProxy ** arg){
	// Выполняем приведение типов
	BufferHttpProxy * http = *arg;
	// Выполняем парсинг полученных данных
	if(!http->request.data.empty() && http->parse()){
		// Закрываем события
		close_events(&http);
		// Очищаем буфер данных
		http->request.data.clear();
		// Устанавливаем таймаут ожидания запроса в 3 секунды
		struct timeval timeout = KEEP_ALIVE;
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
			// Выполняем подключение к серверу
			http->fds.server = connect_server(&http);
			// Если сокет существует
			if(http->fds.server > -1){
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
					// Создаем новое событие для сервера
					http->evs.server = event_new(http->base, http->fds.server, EV_TIMEOUT | EV_WRITE | EV_PERSIST, on_http_write_server, http);
					// Активируем события
					event_add(http->evs.server, &timeout);
					// Выходим
					return;
				}
			// Если подключение не удачное то сообщаем об этом
			} else http->response = http->parser->faultConnect();
		}
		// Ответ готов
		if(!http->response.mess.empty()){
			// Создаем новое событие для клиента
			http->evs.client = event_new(http->base, fd, EV_TIMEOUT | EV_WRITE | EV_PERSIST, on_http_write_client, http);
			// Активируем события
			event_add(http->evs.client, &timeout);
		// Очищаем память
		} else free_data(&http);
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
			case 1: free_data(&http); break;
			// Если это чтение
			case 2: {
				// Буфер для чтения данных из сокета
				char buffer[BUFFER_SIZE];
				// Выполняем чтение данных из сокета
				int len = recv(fd, buffer, sizeof(buffer), 0);
				// Если данные не считаны то выполняем обработку входящего запроса
				if(len <= 0) free_data(&http);
				// Если данные считаны нормально тогда продолжаем
				else {
					// Скидываем количество отправляемых данных
					http->request.offset = 0;
					// Проверяем последние два символа
					short l1 = (short) buffer[len - 2];
					short l2 = (short) buffer[len - 1];
					// Склеиваем полученные данные
					appendToBuffer(http->request.data, len, buffer);
					// Если это перенос строки "\r\n", выполняем обработку входящего запроса
					if(((l1 == 13) && (l2 == 10)) || (l2 == 0)) prepare_request(fd, &http);
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
	http->fds.client = sock;
	// Добавляем событие в базу
	http->evs.client = event_new(base, http->fds.client, EV_READ | EV_PERSIST, on_http_request, http);
	// Ожидаем подключение 3 секунд и 0 микросекунд
	struct timeval timeout = CONNECT_TIMEOUT;
	// Активируем событие
	event_add(http->evs.client, &timeout);
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