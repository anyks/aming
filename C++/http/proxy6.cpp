// sudo lsof -i | grep -E LISTEN
// otool -L http5
//  g++ -std=c++11 -Wall -pedantic -O3 -Werror=vla -lpthread -levent -I/usr/local/include -L/usr/local/lib -o http6 base64.cpp proxy6.cpp = dynamic
//  g++ -std=c++11 -Wall -pedantic -O3 -Werror=vla -lpthread -I/usr/local/include /usr/local/opt/libevent/lib/libevent.a -o http6 base64.cpp proxy6.cpp http.cpp = static

// MacOS X
// export EVENT_NOKQUEUE=1
// brew uninstall --force tmux
// brew install --HEAD tmux

#include <cstdlib>
//#include <cstring>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#include <map>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include <pthread.h>

#include <iostream>
#include <memory>
#include <sstream>
#include <algorithm>
#include <set>
#include <sys/stat.h>
#include <stdio.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>

//#include <event.h>
#include <event2/event.h>
#include <event2/event_struct.h>


#include <errno.h>
#include <list>
#include <fcntl.h>
#include <time.h>

#include "http.h"

// #include <sstream>

using namespace std;

// Класс для работы с сервером
class TServer {
	pthread_mutex_t mutex;

	public:
		// Конструктор
		TServer(){pthread_mutex_init(&mutex, NULL);}
		// Деструктор
		~TServer(){pthread_mutex_destroy(&mutex);}
		// Блокировка
		inline void lock(){pthread_mutex_lock(&mutex);}
		// Разблокировка
		inline void unlock(){pthread_mutex_unlock(&mutex);}
};

// Класс для работы с потоками клиента
class TClient {
	pthread_mutex_t mutex;
	pthread_cond_t condition;

	public:
		// Конструктор
		TClient(){
			pthread_mutex_init(&mutex, 0);
			pthread_cond_init(&condition, 0);
		}
		// Деструктор
		~TClient(){
			pthread_mutex_destroy(&mutex);
			pthread_cond_destroy(&condition);
		}
		// Блокировка
		inline void lock(){pthread_mutex_lock(&mutex);}
		// Разблокировка
		inline void unlock(){pthread_mutex_unlock(&mutex);}
		// Сигнал
		inline void signal(){pthread_cond_signal(&condition);}
		// Сигнал вещания
		inline void broadcastSignal(){pthread_cond_broadcast(&condition);}
		// Ожидание
		inline void wait(){pthread_cond_wait(&condition, &mutex);}
};

TServer get_host_lock;	// Блокировки запроса данных с хоста
TClient client_lock;	// Событие подключения клиента

// Количество подключенных клиентов и максимально возможное количество подключений
uint32_t client_count = 0, max_clients = 100;

/**
 * BufferHttpProxy Класс для работы с данными прокси сервера
 */
class BufferHttpProxy {
	private:
		// Буфер данных
		struct Buffer {
			string 	data;				// Данные в буфере
			size_t	size	= 4096,		// Максимальный размер буфера
					offset	= 0,		// Смещение в буфере
					total	= 0;		// Общее количество данных в буфере
		};
	public:
		struct event_base	* base = NULL;			// База событий
		Http				* parser = 0x00;		// Объект парсера
		string				response;				// Ответ системы
		Buffer				request;				// Данные запроса
		bool				auth		= false,	// Флаг авторизации
							ishttps		= false;	// Флаг зашифрованного соединения
		int					pid			= -1;		// Пид процесса
		evutil_socket_t		socServer	= -1,		// Сокет сервера
							socClient	= -1;		// Сокет клиента
		struct event		* evClient = NULL,		// Событие клиента
							* evServer = NULL;		// Событие сервера
		/**
		 * BufferHttpProxy Конструктор
		 * @param [string] name имя ресурса
		 */
		BufferHttpProxy(const char * name){
			// Создаем объект для работы с http заголовками
			parser = new Http(name);
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
			// Очищаем строку
			request.data.clear();
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
			return parser->parse(request.data);
		}
};

#define EVLOOP_ONCE				0x01
#define EVLOOP_NONBLOCK			0x02
#define EVLOOP_NO_EXIT_ON_EMPTY	0x04

// Приведение типа -Wint-to-void-pointer-cast
// Или нужно установить параметр сборки: -Wno-int-to-void-pointer-cast
#define INTTOVOID_POINTER(i) ((char *) NULL + (i))

// Структура для передачи идентификатора сокета между процессами
typedef struct fbuff {
	long type;
	int socket;
} fmess;

// Временный каталог для файлов
string nameSystem	= "anyksHttp";
string piddir		= "/var/run";

std::vector <int> sockets;

void sig_handler(int signum){
	//std::cout << "signum" << signum << endl;

	cout << "Сигнал обрыва соединения во время записи!!!!" << endl;
}

// Функция обработки сигнала о появившемся зомби процессе
void sigchld_handler(int signal) {
	
	cout << " Дочерний процесс убит!!!!" << endl;

	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// Функция обработки сигналов завершения процессов
void sigterm_handler(int signal){
	
	cout << "Процесс убит!!!!" << endl;

	// close(client_sock); // Закрываем клиентский сокет
	// close(server_sock); // Закрываем серверный сокет
	exit(0); // Выходим
}


void sig_handler1(int signum){
	//std::cout << "signum" << signum << endl;

	cout << "Сигнал обрыва соединения во время записи1!!!!" << endl;
}

// Функция обработки сигнала о появившемся зомби процессе
void sigchld_handler1(int signal) {
	
	cout << " Дочерний процесс убит1!!!!" << endl;

	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// Функция обработки сигналов завершения процессов
void sigterm_handler1(int signal){
	
	cout << "Процесс убит1!!!!" << endl;

	// close(client_sock); // Закрываем клиентский сокет
	// close(server_sock); // Закрываем серверный сокет
	exit(0); // Выходим
}

void sighup_handler1(int signal){
	cout << "Терминал потерял связь1!!!" << endl;
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
 * @param pid  идентификатор pid файла
 */
void create_pid(pid_t pid){
	// Формируем имя файла
	string filename = piddir + string("/") + nameSystem + ".pid";
	// Удаляем файл
	remove(filename.c_str());
	// Открываем файл на запись
	FILE * fh = fopen(filename.c_str(), "w");
	// Выводим номер процесса
	std::cout << pid << endl;
	// Записываем считанные данные в файл
	fprintf(fh, "%d", pid);
	// Закрываем файл
	fclose(fh);
}

/**
 * Set a socket to non-blocking mode.
 */
int setnonblock(int fd){
	int flags;

	flags = fcntl(fd, F_GETFL);

	if(flags < 0) return flags;

	flags |= O_NONBLOCK;

	if(fcntl(fd, F_SETFL, flags) < 0) return -1;

	return 0;
}

int create_listen_socket(){
	// Сокет сервера
	int serversock;
	// Максимальное число клиентов
	int maxpending = 5;
	// Структура для сервера
	struct sockaddr_in echoserver;
	// Создаем сокет
	if((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
	// if((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_UDP)) < 0){
		cout << "[-] Could not create socket.\n";
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
	// Выполняем биндинг сокета // ::bind (для мака)
	if(::bind(serversock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0){
		cout << "[-] Bind error.\n";
		return -1;
	}
	// Выполняем чтение сокета
	if(listen(serversock, maxpending) < 0){
		cout << "[-] Listen error.\n";
		return -1;
	}
	// Указываем что сокет не блокирующий
	if(setnonblock(serversock) < 0){
		cout << "[-] Failed to set server socket to non-blocking.\n";
		return -1;
	}
	return serversock;
}

// Функция проверки логина и пароля
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


// Функция создания сокета для подключения к удаленному серверу
int createServerSocket(const char * host, int port){
	// Сокет подключения
	int sock = 0;
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
	if(getaddrinfo(host, std::to_string(port).c_str(), &param, &req) != 0){
		std::cout << "Error in server address format!" << endl;
		return -1;
	}
	// Создаем сокет, если сокет не создан то сообщаем об этом
	if((sock = socket(req->ai_family, req->ai_socktype, req->ai_protocol)) < 0){
		std::cout << "Error in creating socket to server!" << endl;
		return -1;
	}
	// Выполняем подключение к удаленному серверу, если подключение не выполненно то сообщаем об этом
	if(connect(sock, req->ai_addr, req->ai_addrlen) < 0){
		std::cout << "Error in connecting to server!" << endl;
		return -1;
	}
	// И освобождаем связанный список
	freeaddrinfo(req);
	/* Set the client socket to non-blocking mode. */
	if(setnonblock(sock) < 0) cout << "Failed to set server socket non-blocking" << endl;
	// Выводим созданный нами сокет
	return sock;
}

// Функция отправки данных клиенту
bool sendClient(int sock, const char * buffer, size_t length){
	// Если данные переданы верные
	if((sock > -1) && (buffer != NULL)){
		// Общее количество отправленных байт
		int total = 0, bytes = 1;
		// Отправляем данные до тех пор пока не уйдут
		while(total < length){
			// Если произошла ошибка отправки то сообщаем об этом и выходим
			if((bytes = send(sock, (void *) (buffer + total), length - total, 0)) < 0) return false;
			// Считаем количество отправленных байт
			total += bytes;
		}
	}
	// Сообщаем что все удачно отправлено
	return true;
}

// Функция записи в сокет сервера запроса
bool writeToServerSocket(int socket, const char * buffer, size_t length){
	// Количество загруженных байтов
	int bytes;
	// Общее количество загруженных байтов
	size_t total = 0;
	// Выполняем отправку до тех пор пока все не отдадим
	while(total < length){
		// Если данные не отправились то сообщаем об этом
		if((bytes = send(socket, (void *) (buffer + total), length - total, 0)) < 0){
			std::cout << "Error in sending to server!" << endl;
			return false;
		}
		// Увеличиваем количество отправленных данных
		total += bytes;
	}
	// Сообщаем что все удачно
	return true;
}

// Функция отправки данных из сокета сервера в сокет клиенту
bool writeToClient(int client_socket, int server_socket){
	// Количество загруженных байтов
	int bytes;
	// Максимальный размер буфера
	const size_t max_bufsize = 256;
	// Буфер для чтения данных из сокета
	char buffer[max_bufsize];
	// Выполняем чтение данных из сокета сервера до тех пор пока не считаем все полностью
	while((bytes = recv(server_socket, buffer, max_bufsize, 0)) > 0){
		// Выполняем отправку данных на сокет клиента
		sendClient(client_socket, buffer, bytes);
		// Заполняем буфер нулями
		memset(buffer, 0, sizeof(buffer));
	}
	// Если байты считаны не правильно то сообщаем об этом
	if(bytes < 0){
		// Выводим сообщение об ошибке
		std::cout << "Yo..!! Error while recieving from server!" << endl;
		// Сообщаем что произошла ошибка
		return false;
	}
	// Сообщаем что все удачно
	return true;
}

// Функция отправки данных в сокет
int send_sock(int sock, const char * buffer, uint32_t size){
	int index = 0, ret;
	while(size){
		if((ret = send(sock, &buffer[index], size, 0)) <= 0) return (!ret) ? index : -1;
		index += ret;
		size -= ret;
	}
	return index;
}

// Handle server request {{{
void serve_proxy(int fd, short event, void * arg){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (arg);
	// Если подключение не передано
	if(!http){
		// Отключаем клиента
		shutdown(fd, SHUT_RDWR);
		// Закрываем сокет
		close(fd);
		// Выходим
		return;
	}
	// Если это таймаут
	if(event == 1){
		cout << " Таймаут сервер!!!! " << endl;
		// Если данные еще не удалены
		if(http != NULL){
			// Удаляем объект данных
			delete http;
			// Присваиваем пустой адрес
			http = NULL;
		}
		// Выходим
		return;
	// Если это чтение данных
	} else if(event == 2){
		// Создаем буфер для чтения данных
		vector <char> buffer(256);
		// Выполняем чтение данных из сокета
		int len = recv(fd, buffer.data(), 256, 0);
		// Если данные не считаны значит клиент отключился
		if(len <= 0){
			cout << "Server disconnected." << " socket = " << fd << endl;
			// Если данные еще не удалены
			if(http != NULL){
				// Удаляем объект данных
				delete http;
				// Присваиваем пустой адрес
				http = NULL;
			}
			// Выходим
			return;
		// Если данные считаны нормально
		} else if(http->socClient) {
			// Записываем данные полученные из сокета
			http->request.data.append(buffer.cbegin(), buffer.cend());
			// Отправляем удаленному клиенту полученный буфер данных
			send(http->socClient, http->request.data.c_str(), len, 0);
			// Очищаем буфер
			http->request.data.clear();
		}
	}
}

// Handle client request {{{
void clien_proxy(int fd, short event, void * arg){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (arg);
	// Если подключение не передано
	if(!http){
		// Отключаем клиента
		shutdown(fd, SHUT_RDWR);
		// Закрываем сокет
		close(fd);
		// Выходим
		return;
	}
	// Если это таймаут
	if(event == 1){
		cout << " Таймаут клиент!!!! " << endl;
		// Если данные еще не удалены
		if(http != NULL){
			// Удаляем объект данных
			delete http;
			// Присваиваем пустой адрес
			http = NULL;
		}
		// Выходим
		return;
	// Если это чтение данных
	} else if(event == 2){
		// Создаем буфер для чтения данных
		vector <char> buffer(256);
		// Выполняем чтение данных из сокета
		int len = recv(fd, buffer.data(), 256, 0);
		// Если данные не считаны значит клиент отключился
		if(len <= 0){
			cout << "Client disconnected." << " socket = " << fd << endl;
			// Если данные еще не удалены
			if(http != NULL){
				// Удаляем объект данных
				delete http;
				// Присваиваем пустой адрес
				http = NULL;
			}
			// Выходим
			return;
		// Если данные считаны нормально
		} else if(http->socServer) {
			// Записываем данные полученные из сокета
			http->request.data.append(buffer.cbegin(), buffer.cend());
			// Отправляем удаленному клиенту полученный буфер данных
			send(http->socServer, http->request.data.c_str(), len, 0);
			// Очищаем буфер
			http->request.data.clear();
		}
	}
}

// Handle client responce {{{
void client_write(int fd, short event, void * arg){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (arg);
	// Если подключение не передано
	if(!http){
		// Отключаем клиента
		shutdown(fd, SHUT_RDWR);
		// Закрываем сокет
		close(fd);
		// Выходим
		return;
	}
	// Send data to client
	size_t len = write(fd, http->response.c_str() + http->request.offset, http->response.length() - http->request.offset);
	// Выполняем отправку данных до тех пор пока все не отправим
	if(len < (http->response.length() - http->request.offset)){
		// Failed to send rest data, need to reschedule
		http->request.offset += len;
	// Если мы все отправили
	} else {
		// Если это не зашифрованное соединение то просто выходим
		if(!http->ishttps){
			// Если данные еще не удалены
			if(http != NULL){
				// Удаляем объект данных
				delete http;
				// Присваиваем пустой адрес
				http = NULL;
			}
			// Выходим
			return;
		// Если это зашифрованное соединение
		} else {
			// Устанавливаем таймаут ожидания запроса в 3 секунды
			struct timeval timeout = {3, 0};
			// Обнуляем размер буфера
			http->request.total = 0;
			// Очищаем буфер данных
			http->request.data.clear();
			// Очищаем событие для клиента
			if(http->evClient != NULL){
				// Удаляем событие
				event_free(http->evClient);
				// Обнуляем событие
				http->evClient = NULL;
			}
			// Создаем новое событие для клиента
			http->evClient = event_new(http->base, http->socClient, EV_TIMEOUT | EV_READ | EV_PERSIST, clien_proxy, http);
			http->evServer = event_new(http->base, http->socServer, EV_TIMEOUT | EV_READ | EV_PERSIST, serve_proxy, http);
			// Активируем события
			event_add(http->evClient, &timeout);
			event_add(http->evServer, &timeout);
		}
	}
	return;
}
//}}}

void on_request(int fd, short event, void * arg){
	// Получаем объект подключения
	BufferHttpProxy * http = reinterpret_cast <BufferHttpProxy *> (arg);
	// Если подключение не передано
	if(!http){
		// Отключаем клиента
		shutdown(fd, SHUT_RDWR);
		// Закрываем сокет
		close(fd);
		// Выходим
		return;
	}
	// Если сработал таймаут
	if(event == 1){
		cout << "Timeout to client" << " socket = " << fd << endl;
		// Если данные еще не удалены
		if(http != NULL){
			// Удаляем объект данных
			delete http;
			// Присваиваем пустой адрес
			http = NULL;
		}
		// Выходим
		return;
	// Если это не таймаут то продолжаем работу
	} else {
		// Создаем буфер для чтения данных
		vector <char> buffer((const size_t) http->request.size);
		// Выполняем чтение данных из сокета
		int len = recv(fd, buffer.data(), buffer.size(), 0);
		// Если данные не считаны значит клиент отключился
		if(len == 0){
			cout << "Client disconnected." << " socket = " << fd << endl;
			// Если данные еще не удалены
			if(http != NULL){
				// Удаляем объект данных
				delete http;
				// Присваиваем пустой адрес
				http = NULL;
			}
			// Выходим
			return;
		// Если данные вообще не считаны тогда также выходим
		} else if(len < 0){
			cout << "Socket failure, disconnecting client." << " socket = " << fd << endl;
			// Если данные еще не удалены
			if(http != NULL){
				// Удаляем объект данных
				delete http;
				// Присваиваем пустой адрес
				http = NULL;
			}
			// Выходим
			return;
		// Если данные считаны нормально тогда продолжаем
		} else {
			// Записываем данные полученные из сокета
			http->request.data.append(buffer.cbegin(), buffer.cend());
			// Устанавливаем параметры по умолчанию
			http->response			= "";
			http->request.total		+= len;
			http->request.offset	= 0;
			// Выполняем парсинг полученных данных
			if(http->parse()){
				// Если это метод коннект
				bool methodConnect = (strcmp(http->parser->getMethod().c_str(), "connect") ? false : true);
				// Если авторизация не прошла
				if(!http->auth) http->auth = check_auth(http->parser);
				// Если нужно запросить пароль
				if(!http->auth && (!http->parser->getLogin().length() || !http->parser->getPassword().length())){
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
						http->socServer = createServerSocket(http->parser->getHost().c_str(), http->parser->getPort());
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
						cout << " Отправляем запрос на незашифрованное соединение!!!" << endl;
					}
				}
				// Ответ готов
				if(http->response.length()){
					// Устанавливаем таймаут для клиента 3 секунды
					struct timeval timeout = {3, 0};
					// Очищаем событие для клиента
					if(http->evClient != NULL){
						// Удаляем событие
						event_free(http->evClient);
						// Обнуляем событие
						http->evClient = NULL;
					}
					// Создаем новое событие для клиента
					http->evClient = event_new(http->base, fd, EV_TIMEOUT | EV_READ | EV_PERSIST, client_write, http);
					// Активируем события
					event_add(http->evClient, &timeout);
				}
			}
		}
	}
	// Выходим
	return;
}


void sighup_function(int fd, short event, void * arg){
	cout << " Сработал сигнал на сокете = " << fd << endl;
}

/**
 * on_connect Событие на новое подключение к серверу
 * @param fd    файловый дескриптор текущего сервера
 * @param event флаг события
 * @param arg   объект данных подключения
 */
void on_connect(int fd, short event, void * arg){
	// Время ожидания следующего запроса
	//float ttl = 0.5; // 5;
	// Устанавливаем таймаут
	//sleep(ttl);
	// Структуры получения данных сокета
	socklen_t	len = 0;
	sockaddr_in	client_addr;
	// Ожидаем подключение 3 секунд и 0 микросекунд
	struct timeval timeout = {3, 0};
	// Accept incoming connection
	evutil_socket_t sock = accept(fd, reinterpret_cast <sockaddr *> (&client_addr), &len);
	// Если сокет не создан тогда выходим
	if(sock < 1) return;
	/* Set the client socket to non-blocking mode. */
	if(setnonblock(sock) < 0) cout << "Failed to set client socket non-blocking" << endl;
	// Получаем объект базы событий
	struct event_base * base = reinterpret_cast <struct event_base *> (arg);
	// Создаем новый объект подключения
	BufferHttpProxy * http = new BufferHttpProxy("anyks");
	// Запоминаем базу событий
	http->base = base;
	// Запоминаем сокет подключения
	http->socClient = sock;
	// Добавляем событие в базу
	http->evClient = event_new(base, http->socClient, EV_READ | EV_PERSIST, on_request, http);
	/*
	struct event * sign = evsignal_new(base, SIGHUP, sighup_function, NULL);
	event_add(sign, NULL);
	*/
	// Активируем событие
	event_add(http->evClient, &timeout);
	// Выходим
	return;
}

// Функция установки количество разрешенных файловых дескрипторов
int set_fd_limit(int maxfd){
	// Структура для установки лимитов
	struct rlimit lim;
	// зададим текущий лимит на кол-во открытых дискриптеров
	lim.rlim_cur = maxfd;
	// зададим максимальный лимит на кол-во открытых дискриптеров
	lim.rlim_max = maxfd;
	// установим указанное кол-во
	return setrlimit(RLIMIT_NOFILE, &lim);
}

int main(int argc, char * argv[]){
	/*
	struct event ev;
	struct timeval time;
	time.tv_sec = 1;
	time.tv_usec = 0;

	event_init();
	event_set(&ev, 0, EV_PERSIST, my_function, NULL);
	evtimer_add(&ev, &time);
	event_dispatch();
	*/

	// Максимальное количество файловых дескрипторов
	int maxfd = 1024; // (по дефолту в системе 1024)
	// Максимальное количество воркеров
	const size_t max_works = 4;
	// Наши ID процесса и сессии
	pid_t pid[max_works], sid;
	// Структура для клиента
	//struct sockaddr_in echoclient;
	// Установим максимальное кол-во дискрипторов которое можно открыть
	set_fd_limit(maxfd);
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
	// Определяем количество возможных передаваемых данных
	//uint32_t clientlen = sizeof(echoclient);
	// Получаем сокет
	int socket = create_listen_socket();
	// Проверяем все ли удачно
	if(socket == -1){
		cout << "[-] Failed to create server\n";
		return 1;
	}
	// Устанавливаем сигнал установки подключения
	signal(SIGPIPE, sig_handler);		// Сигнал обрыва соединения во время записи
	signal(SIGCHLD, sigchld_handler);	// Дочерний процесс убит
	signal(SIGTERM, sigterm_handler);	// Процесс убит
	signal(SIGHUP, sighup_handler1);	// Терминал потерял связь
	// Ключ по которому передаются данные между процессов
	key_t msgkey = ftok(".", getpid());
	// Получаем внешний идентификатор процесса
	int qid = msgget(msgkey, IPC_CREAT | 0660);
	// Буфер для передачи данных socket между процессами
	fmess fork_buf;
	// Получаем размер буфера
	int lenfork_buf = sizeof(fmess) - sizeof(long);
	// Освобождаем процесс
	msgctl(qid, IPC_RMID, 0);

	int status;

	// Создаем дочерние потоки (от 1 потому что 0-й это этот же процесс)
	for(int i = 1; i < max_works; i++){
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

				std::cout << "pid = " << (int) getpid() << " === " << fork_buf.socket << endl;

				// Создаем новую базу
				struct event_base * base = event_base_new();


				// Добавляем событие в базу
				struct event * evnt = event_new(base, fork_buf.socket, EV_READ | EV_PERSIST, on_connect, base);

				// struct event * sign = evsignal_new(base, SIGHUP, sighup_function, NULL);

				// Активируем событие
				event_add(evnt, NULL);
				// event_add(sign, NULL);
				// Активируем перебор базы событий
				event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY);

				// Отключаем клиента
				shutdown(fork_buf.socket, SHUT_RDWR);
				// Закрываем сокет
				close(fork_buf.socket);
				event_free(evnt);
				//event_free(sign);
				event_base_free(base);


			} break;
			// Если это родитель то отправляем потомку идентификатор сокета
			default: msgsnd(qid, &fork_buf, lenfork_buf, 0);
		}
	}
	// Ждем завершение работы потомка (от 1 потому что 0-й это этот же процесс а он не может ждать завершения самого себя)
	for(int i = 1; i < max_works; i++){
		waitpid(pid[i], &status, 0);

		cout << " status = " << WTERMSIG(status) << endl;
	}

	cout << " Процессы кончились " << endl;

	// Освобождаем процесс
	msgctl(qid, IPC_RMID, 0);

	

	// Выходим
	remove_pid(EXIT_SUCCESS);
	// Выходим
	return 0;
}