// sudo lsof -i | grep -E LISTEN
//  g++ -o socks5 socks5.cpp -lpthread

#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include <pthread.h>

#include <iostream>
#include <memory>
#include <string>
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


#include <execinfo.h>


using namespace std;

// Байты команд подключения
#define CMD_CONNECT			0x01
#define CMD_BIND			0x02
#define CMD_UDP_ASSOCIATIVE	0x03

// Байты типа запрашиваемого адреса
#define ATYP_IPV4	0x01
#define ATYP_DNAME	0x03
#define ATYP_IPV6	0x04

// Байты ответов сервера для запроса авторизации
#define METHOD_NOAUTH		0x00
#define METHOD_AUTH			0x02
#define METHOD_NOTAVAILABLE	0xff

// Байты ответа сервера для обработки запроса
#define RESP_SUCCEDED		0x00
#define RESP_GEN_ERROR		0x01

/* Handshake */

// Структура для идентификации входящего запроса клиента
struct MethodIdentificationPacket {
	uint8_t version, nmethods;
	/* uint8_t methods[nmethods]; */
} __attribute__((packed));

// Структура для чтения данных с сокета клиента
struct MethodSelectionPacket {
	uint8_t version, method;
	MethodSelectionPacket(uint8_t met) : version(5), method(met) {}
} __attribute__((packed));

/* Requests */

// Структура заголовков запроса socks5
struct SOCKS5RequestHeader {
	uint8_t version, cmd, rsv /* = 0x00 */, atyp;
} __attribute__((packed));

// Структура ip адреса и порта для запроса socks5
struct SOCK5IP4RequestBody {
	uint32_t ip_dst;
	uint16_t port;
} __attribute__((packed));

// Структура доменного имени для запроса socks5
struct SOCK5DNameRequestBody {
	uint8_t length;
	uint8_t dname;
} __attribute__((packed));

/* Responses */

// Структура содержащий данные ответа socks5
struct SOCKS5Response {
	uint8_t version, cmd, rsv /* = 0x00 */, atyp;
	uint32_t ip_src;
	uint16_t port_src;
	SOCKS5Response(bool succeded = true) : version(5), cmd(succeded ? RESP_SUCCEDED : RESP_GEN_ERROR), rsv(0), atyp(ATYP_IPV4) {}
} __attribute__((packed));

// Структура для передачи идентификатора сокета между процессами
typedef struct fbuff {
	long type;
	int socket;
} fmess;

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

// Временный каталог для файлов
string tmpdir		= "/tmp";
string nameSystem	= "anyksProxy";
string piddir		= "/var/run";

// Количество подключенных клиентов и максимально возможное количество подключений
uint32_t client_count = 0, max_clients = 100;

void sig_handler(int signum){
	std::cout << "signum" << signum << endl;
}

// Функция обработки сигнала о появившемся зомби процессе
void sigchld_handler(int signal) {
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// Функция обработки сигналов завершения процессов
void sigterm_handler(int signal){
	// close(client_sock); // Закрываем клиентский сокет
	// close(server_sock); // Закрываем серверный сокет
	exit(0); // Выходим
}

// Функция получения данных из сокета
int recv_sock(int sock, char * buffer, uint32_t size){
	int index = 0, ret;
	while(size){
		if((ret = recv(sock, &buffer[index], size, 0)) <= 0) return (!ret) ? index : -1;
		index += ret;
		size -= ret;
	}
	return index;
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

// Функция преобразования integer в string
string int_to_str(uint32_t ip){
	// Создаем поток для преобразования байтов
	ostringstream oss;
	// Выполняем переход по всем байтам
	for(unsigned i = 0; i < 4; i++){
		// Выполняем смещение байтов
		oss << ((ip >> (i * 8) ) & 0xFF);
		// Получаем точку
		if(i != 3) oss << '.';
	}
	// Выводим результат
	return oss.str();
}

// Функция чтения из сокета указанного количества байтов
int read_variable_string(int sock, uint8_t *buffer, uint8_t max_sz){
	if(recv_sock(sock, (char *) buffer, 1) != 1 || buffer[0] > max_sz) return false;
	uint8_t sz = buffer[0];
	if(recv_sock(sock, (char *) buffer, sz) != sz) return -1;
	return sz;
}

// Функция подключения к удаленному хосту
int connect_to_host(uint32_t ip, uint16_t port){
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
	// Преобразуем хост адрес в виде числа в строку
	string ip_string = int_to_str(ip);
	// Блокируем поток для получения ip адреса
	get_host_lock.lock();
	// Получаем данные хоста удаленного сервера по его названию
	server = gethostbyname(ip_string.c_str());
	// Если хост не получен тогда разблокируем поток и выходим
	if(!server){
		// Разблокируем поток
		get_host_lock.unlock();
		// Сообщаем что коннект не удался
		return -1;
	}
	// Выполняем копирование данных типа подключения
	bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
	// Разблокируем поток
	get_host_lock.unlock();
	// Устанавливаем порт для подключения к удаленному серверу
	serv_addr.sin_port = htons(port);
	// Выполняем коннект к серверу и получаем из него данные
	return (!connect(sockfd, (const sockaddr *) &serv_addr, sizeof(serv_addr)) ? sockfd : -1);
}

// Функция установки fd сокетам
void set_fds(int sock1, int sock2, fd_set *fds){
	// Очищаем набор файловых дескрипторов
	FD_ZERO(fds);
	// Устанавливаем файловые дескрипторы для сокетов
	FD_SET(sock1, fds);
	FD_SET(sock2, fds);
}

// Функция передачи полученных данных с сервера запросов, клиенту который подключился через прокси
void do_proxy(int client, int conn, char * buffer){
	fd_set readfds; // Создаем набор файловых дескрипторов
	// Определяем количество файловых дескрипторов
	int result, nfds = max(client, conn) + 1;
	// Добавляем в набор файловый дескриптор
	set_fds(client, conn, &readfds);
	// Перебираем все файловые дескрипторы из набора
	while((result = select(nfds, &readfds, 0, 0, 0)) > 0){
		// Если файловый дескриптор существует
		if(FD_ISSET(client, &readfds)){
			// Считываем данные из сокета удаленного клиента в буфер
			int recvd = recv(client, buffer, 256, 0);
			// Если ничего не считано то выходим
			if(recvd <= 0) return;
			// Отправляем удаленному клиенту полученный буфер данных
			send_sock(conn, buffer, recvd);
		}
		// Если файловый дескриптор для сокета сервера существует
		if(FD_ISSET(conn, &readfds)){
			// Считываем данные из сокета подключившегося клиента в буфер
			int recvd = recv(conn, buffer, 256, 0);
			// Если ничего не считано то выходим
			if(recvd <= 0) return;
			// Отправляем подключившемуся клиенту полученный буфер данных
			send_sock(client, buffer, recvd);
		}
		// Добавляем в набор файловый дескриптор
		set_fds(client, conn, &readfds);
	}
}

// Функция запроса данных с сервера
bool handle_request(int sock, char * buffer){
	// Создаем заголовки запроса socks5
	SOCKS5RequestHeader header;
	// Запонляем их данными из сокета
	recv_sock(sock, (char *) &header, sizeof(SOCKS5RequestHeader));
	// Проверяем правильность заполненных заголовков
	if(header.version != 5 || header.cmd != CMD_CONNECT || header.rsv != 0) return false;
	// Создаем клиентский сокет для запроса данных с удаленного сервера
	int client_sock = -1;
	// Проверяем какой тип подключения у нас требуют
	switch(header.atyp){
		// Если это IPv4 адрес
		case ATYP_IPV4: {
			// Создаем структуру запроса
			SOCK5IP4RequestBody req;
			// Заполняем структуру данными по ip адресу и порту
			if(recv_sock(sock, (char *) &req, sizeof(SOCK5IP4RequestBody)) != sizeof(SOCK5IP4RequestBody)) break;
			// Выполняем подключение к удаленному серверу
			client_sock = connect_to_host(req.ip_dst, ntohs(req.port));
			// Выходим
			break;
		}
		// Если запрос идет по домену
		case ATYP_DNAME: {
			// Создаем структуру запроса
			SOCK5DNameRequestBody req;
			// Заполняем структуру данными домена
			if(recv_sock(sock, (char *) &req, sizeof(SOCK5DNameRequestBody)) != sizeof(SOCK5DNameRequestBody)) break;
			// Выполняем подключение к удаленному серверу
			client_sock = connect_to_host(req.dname, ntohs(80));
			// Выходим
			break;
		}
		// Для всех ост
		default: return false;
	}
	if(client_sock == -1) return false;
	// Создаем структуру ответа
	SOCKS5Response response;
	// Устанавливаем ip адрес нашего сервера
	response.ip_src = inet_addr("127.0.0.1");
	// Устанавливаем порт нашего сервера
	response.port_src = htons(5555);
	// Отправляем клиенту ответа
	send_sock(sock, (const char *) &response, sizeof(SOCKS5Response));
	// Отправляем клиенту данные полученные с сайта
	do_proxy(client_sock, sock, buffer);
	// Отключаемся от стороннего сервера который выдает контент
	shutdown(client_sock, SHUT_RDWR);
	// Отключаем сокет клиента для подключения к внешнему серверу с контентом
	close(client_sock);
	// Сообщаем что все удачно
	return true;
}

// Функция проверки логина и пароля
bool check_auth(int sock){
	// Логин
	char * username = "TNjFZRCZ3C";
	// Проль
	char * password = "techhost@agro24.ru";
	// Буфер для входящих данных
	uint8_t buffer[128];
	// Считываем данные из сокета, если не верные то валидация не пройдена
	if(recv_sock(sock, (char *) buffer, 1) != 1 || buffer[0] != 1) return false;
	// Считываем логин пользователя
	int sz = read_variable_string(sock, buffer, 127);
	// Если логин не передан тогда авторизация не пройдена
	if(sz == -1) return false;
	// Устанавливаем нулевой последний байт
	buffer[sz] = 0;
	// Проверяем логин
	if(strcmp((char *) buffer, username)) return false;
	// Считываем пароль пользователя
	sz = read_variable_string(sock, buffer, 127);
	// Если пароль не передан тогда авторизация не пройдена
	if(sz == -1) return false;
	// Устанавливаем нулевой последний байт
	buffer[sz] = 0;
	// Проверяем пароль
	if(strcmp((char *) buffer, password)) return false;
	// Создаем буфер ответа
	buffer[0] = 0x01;
	buffer[1] = 0x00;
	// Сообщаем сокету что все удачно
	return send_sock(sock, (const char *) buffer, 2) == 2;
}

// Функция обработки входящих данных с клиента
bool handle_handshake(int sock, char * buffer){
	// Структура данных для чтения входящих параметров
	MethodIdentificationPacket packet;
	// Считываем данные из сокета
	int read_size = recv_sock(sock, (char *) &packet, sizeof(MethodIdentificationPacket));
	// Если считано данных не достаточно или версия пртокола не 5 то выходим
	if(read_size != sizeof(MethodIdentificationPacket) || packet.version != 5) return false;
	// Считываем данные в буфер, с указанным методом
	if(recv_sock(sock, buffer, packet.nmethods) != packet.nmethods) return false;
	// Создаем структуру для отправки клиенту
	MethodSelectionPacket response(METHOD_NOTAVAILABLE);
	// Устанавливаем тип авторизации
	for(unsigned i(0); i < packet.nmethods; ++i){
		// Сообщаем что авторизация не требуется
		if(buffer[i] == METHOD_NOAUTH) response.method = METHOD_NOAUTH;
		// Сообщаем что авторизация требуется
		// if(buffer[i] == METHOD_AUTH) response.method = METHOD_AUTH;
	}
	// Отправляем требование клиенту
	if(send_sock(sock, (const char *) &response, sizeof(MethodSelectionPacket)) != sizeof(MethodSelectionPacket) || response.method == METHOD_NOTAVAILABLE) return false;
	// Выполняем проверку авторизации
	return ((response.method == METHOD_AUTH) ? check_auth(sock) : true);
}

// Функция обработки данных из потока
void * handle_connection(void * arg){
	// Создаем сокет
	int sock = (uint64_t) arg;
	// Создаем буфер
	char * buffer = new char[256];
	// Выполняем проверку авторизации и спрашиваем что надо клиенту, если удачно то делаем запрос на получение данных
	if(handle_handshake(sock, buffer)) handle_request(sock, buffer);
	// Выключаем подключение
	shutdown(sock, SHUT_RDWR);
	// Закрываем сокет
	close(sock);
	// Удаляем буфер
	delete[] buffer;
	// Блокируем поток клиента
	client_lock.lock();
	// Уменьшаем количество подключенных клиентов
	client_count--;
	// Если количество подключенных клиентов достигла предела то сообщаем об этом
	if(client_count == max_clients - 1) client_lock.signal();
	// Разблокируем клиента
	client_lock.unlock();
	// Выходим из приложения
	// exit(0);
	// Сообщаем что все удачно
	return 0;
}

// Функция обработки потоков ядра
bool spawn_thread(pthread_t * thread, void * data){
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 64 * 1024);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	return !pthread_create(thread, &attr, handle_connection, data);
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
		std::cout << "[-] Could not create socket.\n";
		return -1;
	}
	/* Очищаем структуру */
	memset(&echoserver, 0, sizeof(echoserver));
	// Указываем что это сетевой протокол Internet/IP
	echoserver.sin_family = AF_INET;
	// Указываем адрес прокси сервера
	echoserver.sin_addr.s_addr = inet_addr("127.0.0.1"); // htonl(INADDR_ANY);
	// Указываем порт сервера
	echoserver.sin_port = htons(5555); // htons(SERVER_PORT);
	// Выполняем биндинг сокета
	if(bind(serversock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0){
		std::cout << "[-] Bind error.\n";
		return -1;
	}
	// Выполняем чтение сокета
	if(listen(serversock, maxpending) < 0){
		std::cout << "[-] Listen error.\n";
		return -1;
	}
	return serversock;
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
	// Максимальное количество файловых дескрипторов
	int maxfd = 1024; // (по дефолту в системе 1024)
	// Максимальное количество воркеров
	int max_works = 4;
	// Наши ID процесса и сессии
	pid_t pid[max_works], sid;
	// Вспомогательные переменные
	int sockfd, newsockfd, portno;
	// Структура для клиента
	struct sockaddr_in echoclient;
	// Время ожидания следующего запроса
	int ttl = 1; // 5;
	// Установим максимальное кол-во дискрипторов которое можно открыть
	set_fd_limit(maxfd);
	// Ответвляемся от родительского процесса
	pid[0] = fork();
	// Если пид не создан тогда выходим
	if(pid[0] < 0) remove_pid(EXIT_FAILURE);
	// Если с PID'ом все получилось, то родительский процесс можно завершить.
	if(pid[0] > 0) remove_pid(EXIT_SUCCESS);
	// Изменяем файловую маску
	umask(0);
	// Здесь можно открывать любые журналы
	// Создание нового SID для дочернего процесса
	sid = setsid();
	// Если идентификатор сессии дочернего процесса не существует
	if(sid < 0) remove_pid(EXIT_FAILURE);
	// Изменяем текущий рабочий каталог
	if((chdir("/")) < 0) remove_pid(EXIT_FAILURE);
	// Закрываем стандартные файловые дескрипторы
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	// Записываем пид процесса в файл
	create_pid(sid);
	// Определяем количество возможных передаваемых данных
	uint32_t clientlen = sizeof(echoclient);
	// Получаем сокет
	int socket = create_listen_socket();
	// Проверяем все ли удачно
	if(socket == -1){
		std::cout << "[-] Failed to create server\n";
		return 1;
	}
	// Устанавливаем сигнал установки подключения
	signal(SIGPIPE, sig_handler);		// Сигнал обрыва соединения во время записи
	signal(SIGCHLD, sigchld_handler);	// Дочерний процесс убит
	signal(SIGTERM, sigterm_handler);	// Процесс убит
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
			case 0:
				// Выполняем получение данных сокета от родителя
				msgrcv(qid, &fork_buf, lenfork_buf, 1, 0);

				std::cout << "pid = " << (int) getpid() << endl;

				// Бесконечный цикл
				while(true){
					// Создаем сокет клиента
					int clientsock;
					// Лочим клиента
					client_lock.lock();
					// Если количество клиентов превысило предел то ставим клиента в ожидание
					if(client_count == max_clients) client_lock.wait();
					// Разлочим клиента
					client_lock.unlock();
					// Проверяем разрешено ли подключение клиента
					if((clientsock = accept(fork_buf.socket, (struct sockaddr *) &echoclient, &clientlen)) > 0){
						// Выполняем блокировку клиента
						client_lock.lock();
						// Увеличиваем количество подключенных клиентов
						client_count++;
						// Создаем поток
						pthread_t thread;
						// Выполняем активацию потока
						spawn_thread(&thread, (void *) clientsock);
						// Выполняем разблокирование клиента
						client_lock.unlock();
						// Засыпаем на указанное количество секунд
						sleep(ttl);
					}
				}
			break;
			// Если это родитель то отправляем потомку идентификатор сокета
			default: msgsnd(qid, &fork_buf, lenfork_buf, 0);
		}
	}
	// Ждем завершение работы потомка (от 1 потому что 0-й это этот же процесс а он не может ждать завершения самого себя)
	for(int i = 1; i < max_works; i++) waitpid(pid[i], NULL, 0);
	// Освобождаем процесс
	msgctl(qid, IPC_RMID, 0);
	// Выходим
	remove_pid(EXIT_SUCCESS);
}