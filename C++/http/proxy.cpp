// sudo lsof -i | grep -E LISTEN
//  g++ -o socks5 http.cpp -lpthread
//  g++ -std=c++11 -Wall -pedantic -O3 -Werror=vla -lpthread -o http proxy.cpp

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


// #include <sstream>
#include <vector>

using namespace std;

// Приведение типа -Wint-to-void-pointer-cast
// Или нужно установить параметр сборки: -Wno-int-to-void-pointer-cast
#define INTTOVOID_POINTER(i) ((char *) NULL + (i))

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
string nameSystem	= "anyksHttp";
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

	std::cout << " buffer read " << buffer << endl;

	return index;
}

// Функция отправки данных в сокет
int send_sock(int sock, const char * buffer, uint32_t size){
	int index = 0, ret;
	
	std::cout << " buffer write " << buffer << endl;

	while(size){
		if((ret = send(sock, &buffer[index], size, 0)) <= 0) return (!ret) ? index : -1;
		index += ret;
		size -= ret;
	}
	return index;
}

// Функция разбиения строки на указанные составляющие
std::vector<std::string> split(const std::string str, const char * delim){
	// Начальный символ для обрезки строки
	int i = 0;
	// Результирующий вектор
	std::vector<std::string> result;
	// Строка в которой производим поиск
	std::string s = str.c_str();
	// Позиция разделителя в строке
	int pos = s.find(delim);
	// Перебираем строку до тех пор пока не переберем её целиком
	while(pos != std::string::npos){
		// Добавляем найденную позицию в строке
		result.push_back(s.substr(i, pos - i));
		// Запоминаем новую позицию
		i = strlen(delim) + pos;
		// Выполняем поиск новой позиции
		pos = s.find(delim, i);
		// Если позиция достигнута тогда продолжаем дальше
		if(pos == std::string::npos) result.push_back(s.substr(i, s.length()));
	}
	// Выводим результат
	return result;
}

/*
void writeToclientSocket(const char* buff_to_server,int sockfd,int buff_length)
{

	string temp;

	temp.append(buff_to_server);

	int totalsent = 0;

	int senteach;

	while (totalsent < buff_length) {
		if ((senteach = send(sockfd, (void *) (buff_to_server + totalsent), buff_length - totalsent, 0)) < 0) {
			fprintf (stderr," Error in sending to server ! \n");
				exit (1);
		}
		totalsent += senteach;

	}

}
*/


// Функция обработки входящих данных с клиента
bool handle_handshake(int sock, char * buffer){
	// Количество считанных данных из буфера
	int bits_size = 0;
	// Считываем данные из сокета
	while(strstr(buffer, "\r\n\r\n") == NULL){
		// Считываем данные из буфера
		bits_size += recv(sock, &buffer[bits_size], 256, 0);
		// Устанавливаем завершение строки
		buffer[bits_size] = '\0';
	}

	std::cout << " -------- " << buffer << " bites " << bits_size << endl;

	std::vector <std::string> x = split(buffer, "\r\n");

	for(int i = 0; i < (x.size() - 2); i++){
		std::cout << " i = " << i << " string = " << x[i] << endl;
	}

	//const char * dd = "HTTP/1.1 200 Connection established\r\n\r\n";
	// writeToclientSocket(dd, sock, strlen(dd));

	// Очищаем память выделенную для вектора
	std::vector <std::string> ().swap(x);

	return false;
	/*
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
	*/
}

// Функция обработки данных из потока
void * handle_connection(void * arg){
	// Создаем сокет
	int sock = (uint64_t) arg;
	// Создаем буфер
	char * buffer = new char[5000];
	// Выполняем проверку авторизации и спрашиваем что надо клиенту, если удачно то делаем запрос на получение данных
	handle_handshake(sock, buffer);
	// if(handle_handshake(sock, buffer)) handle_request(sock, buffer);
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
	echoserver.sin_addr.s_addr = htonl(INADDR_ANY); // inet_addr("127.0.0.1");
	// Указываем порт сервера
	echoserver.sin_port = htons(5556); // htons(SERVER_PORT);
	// Выполняем биндинг сокета // ::bind (для мака)
	if(::bind(serversock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0){
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
	const size_t max_works = 4;
	// Наши ID процесса и сессии
	pid_t pid[max_works], sid;
	// Структура для клиента
	struct sockaddr_in echoclient;
	// Время ожидания следующего запроса
	float ttl = 0.5; // 5;
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
						spawn_thread(&thread, INTTOVOID_POINTER(clientsock));
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
	// Выходим
	return 0;
}