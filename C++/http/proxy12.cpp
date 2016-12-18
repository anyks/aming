// sudo lsof -i | grep -E LISTEN
// otool -L http12
// MacOS X
// g++ -std=c++11 -D_BSD_SOURCE -Wall -pedantic -O3 -Werror=vla -o http12 http.cpp base64.cpp proxy12.cpp -I/usr/local/include -levent = dynamic
// g++ -std=c++11 -D_BSD_SOURCE -Wall -pedantic -O3 -Werror=vla -o http12 http.cpp base64.cpp proxy12.cpp -I/usr/local/include /usr/local/opt/libevent/lib/libevent.a = static
// Linux
// g++ -std=c++11 -Wall -pedantic -O3 -Werror=vla -o http12 http.cpp base64.cpp proxy12.cpp -levent = dynamic
// g++ -std=c++11 -Wall -pedantic -O3 -Werror=vla -o http12 http.cpp base64.cpp proxy12.cpp /usr/lib/x86_64-linux-gnu/libevent.a /usr/lib/x86_64-linux-gnu/5/libstdc++.a = static
// g++ -std=c++11 -Wall -pedantic -O3 -Werror=vla -o http12 http.cpp base64.cpp proxy12.cpp /usr/lib/x86_64-linux-gnu/libevent.a /usr/lib/gcc/x86_64-linux-gnu/4.9/libstdc++.a
// FreeBSD
// clang++ -std=c++11 -D_BSD_SOURCE -Wall -pedantic -O3 -Werror=vla -o http12 http.cpp base64.cpp proxy12.cpp -I/usr/local/include /usr/local/lib/libevent.a
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
//#include <event2/visibility.h>
#include <event2/event-config.h>
#include "http.h"

// Устанавливаем область видимости
using namespace std;

// Флаги базы данных событий
#define EVLOOP_ONCE				0x01
#define EVLOOP_NONBLOCK			0x02
#define EVLOOP_NO_EXIT_ON_EMPTY	0x04

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

struct info {
    const char *name;
    size_t total_drained;
};

static void read_cb(evutil_socket_t fd, short event, void * arg){
	struct evbuffer * evb = evbuffer_new();
	ev_ssize_t read = evbuffer_read(evb, fd, 2048);

	size_t len = evbuffer_get_length(evb);

	char * data = new char[len];
	evbuffer_copyout(evb, data, len);

	data[len] = '\0';

	printf("=========: %s\n", data);

	// Удаляем данные из буфера
	evbuffer_drain(evb, len);
	delete [] data;


	evbuffer_free(evb);
}




static void echo_read_cb(struct bufferevent *bev, void *ctx){

	/* This callback is invoked when there is data to read on bev. */
struct evbuffer *input = bufferevent_get_input(bev);
struct evbuffer *output = bufferevent_get_output(bev);

size_t len = evbuffer_get_length(input);
char *data;
data = new char[len];
evbuffer_copyout(input, data, len);

printf("+++++++++++: %s\n", data);

/* Copy all the data from the input buffer to the output buffer. */
//evbuffer_add_buffer(output, input);

// Удаляем данные из буфера
evbuffer_drain(input, len);
delete [] data;



     /*
    char tmp[4096];
        size_t n;
        int i;

        struct evbuffer * input = bufferevent_get_input(bev);

        size_t len = evbuffer_get_length(input);

        size_t t = evbuffer_get_contiguous_space(input);

        while ((n = evbuffer_remove(input, tmp, sizeof(tmp))) > 0) {
	        
	    }

	    char * s = new char[len];

	    copy(tmp, tmp + len, s);

	    cout << " ==== " << s << endl;

	    delete [] s;
	    */
        
        /*
        while (1) {
                n = bufferevent_read(bev, tmp, sizeof(tmp));
                if (n <= 0)
                        break;
               // for (i=0; i<n; ++i) tmp[i] = toupper(tmp[i]);

                cout << " ==== " << tmp << endl;

                //bufferevent_write(bev, tmp, n);
        }
        */

        //cout << " +++++ len = " << len << " === " << t << endl;

        /*
        size_t len1;
        char * k = evbuffer_readln(input, &len1, EVBUFFER_EOL_CRLF);

        cout << " ------- " << k << " ==== " << len1 << endl;
		*/
	
        //free(ctx);
        //bufferevent_free(bev);


        /* This callback is invoked when there is data to read on bev. */
       // struct evbuffer *input = bufferevent_get_input(bev);
        //struct evbuffer *output = bufferevent_get_output(bev);

		/*
        //struct info *inf = ctx;

        struct info * inf = reinterpret_cast <struct info *> (ctx);
		*/
	/*
	    struct evbuffer * input = bufferevent_get_input(bev);
	    size_t len = evbuffer_get_length(input);
	    if (len) {
	        //inf->total_drained += len;
	        evbuffer_drain(input, len);
	        printf("Drained %lu bytes from\n", (unsigned long) len);
	    }
	    */
		/*

	    char buf[4096];
	    int n;
	    struct evbuffer *input = bufferevent_get_input(bev);
	    while ((n = evbuffer_remove(input, buf, sizeof(buf))) > 0) {
	        //fwrite(buf, 1, n, stdout);
	    }
	    

	    cout << " +++++ " << buf << endl;
		*/


        //evbuffer_add_printf(bufferevent_get_output(bev), "GET %s\r\n", "test");

        /* Copy all the data from the input buffer to the output buffer. */
       // evbuffer_add_buffer(output, input);
}

static void
echo_event_cb(struct bufferevent *bev, short events, void *ctx)
{
	if (events & BEV_EVENT_CONNECTED) {
         printf("Connect okay.\n");
    } else if (events & (BEV_EVENT_ERROR|BEV_EVENT_EOF)) {
         //struct event_base * base = reinterpret_cast <struct event_base *> (ctx);
         if (events & BEV_EVENT_ERROR) {
                 int err = bufferevent_socket_get_dns_error(bev);
                 if (err)
                         printf("DNS error: %s\n", evutil_gai_strerror(err));
         }
         printf("Closing\n");
         bufferevent_free(bev);
         //event_base_loopexit(base, NULL);
    }
}

/*
int bufferevent_set_max_single_read(struct bufferevent *bev, size_t size){
	struct bufferevent_private *bevp;
	BEV_LOCK(bev);
	bevp = BEV_UPCAST(bev);
	if (size == 0 || size > EV_SSIZE_MAX)
		bevp->max_single_read = MAX_SINGLE_READ_DEFAULT;
	else
		bevp->max_single_read = size;
	BEV_UNLOCK(bev);
	return 0;
}
*/

static void accept_connect(struct evconnlistener * listener, evutil_socket_t fd, struct sockaddr * address, int socklen, void *ctx){
	// Структуры получения данных сокета
	socklen_t	len = 0;
	sockaddr_in	client_addr;
	// Получаем размер буфера
	int buffer_read_size	= BUFFER_READ_SIZE;
	int buffer_write_size	= BUFFER_WRITE_SIZE;
	// Определяем размер массива опции
	socklen_t read_optlen	= sizeof(buffer_read_size);
	socklen_t write_optlen	= sizeof(buffer_write_size);
	// Устанавливаем размер буфера для сокета клиента и сервера
	if((setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *) &buffer_read_size, read_optlen) < 0)
	|| (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *) &buffer_write_size, write_optlen) < 0)){
		// Выводим в консоль информацию
		//debug_message("Set buffer wrong!!!!");
		// Выходим
		return;
	}
	// Считываем установленный размер буфера
	if((getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &buffer_read_size, &read_optlen) < 0)
	|| (getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buffer_write_size, &write_optlen) < 0)){
		// Выводим в консоль информацию
		//debug_message("Get buffer wrong!!!!");
		// Выходим
		return;
	}
	/*
	// Если размер буфера не может быть такой установлен для записывающей операции
	if(buffer_read_size < BUFFER_READ_SIZE)
		// Выводим в консоль сообщение
		debug_message(string("Wrong buffer, you mast set buffer read size = ") + to_string(buffer_read_size));
	// Если размер буфера не может быть такой установлен для считывающей операции
	if(buffer_write_size < BUFFER_WRITE_SIZE)
		// Выводим в консоль сообщение
		debug_message(string("Wrong buffer, you mast set buffer write size = ") + to_string(buffer_write_size));
	*/
	// We got a new connection! Set up a bufferevent for it.
	struct event_base * base = evconnlistener_get_base(listener);

	// Добавляем событие в базу
	struct event * client = event_new(base, fd, EV_TIMEOUT | EV_READ | EV_PERSIST, read_cb, NULL);
	// Активируем событие
	event_add(client, NULL);

	
	/* We got a new connection! Set up a bufferevent for it. */
       
      




/*
        struct event_base *base = evconnlistener_get_base(listener);
        struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

        evutil_socket_t socket_app = evconnlistener_get_fd(listener);

        //bufferevent_set_max_single_read(bev, 1024);
		//bufferevent_set_max_single_write(bev, 1024);
		//
		//ev_ssize_t t = bufferevent_get_max_to_read(bev);
		//ev_ssize_t bufferevent_get_max_to_write(struct bufferevent *bev);

		//

		//cout << " ===== " << t << endl;
		//
		
		//bufferevent_setwatermark(bev, EV_READ, 128, 0);

        bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);

        bufferevent_enable(bev, EV_READ|EV_WRITE);

        bufferevent_flush(bev, EV_READ | EV_WRITE, BEV_FINISHED);

*/

       // struct evbuffer * input = bufferevent_get_input(bev);

        //int tk = evbuffer_expand(input, 2048);

        //cout << " ----- tk ----- " << tk << endl;
        //evbuffer_add(bufferevent_get_output(bev), message, 1024);

       // delete [] message;

	// Выходим
	return;
}

static void accept_error(struct evconnlistener * listener, void * ctx){
		struct event_base *base = evconnlistener_get_base(listener);
		int err = EVUTIL_SOCKET_ERROR();
		fprintf(stderr, "Got an error %d (%s) on the listener. Shutting down.\n", err, evutil_socket_error_to_string(err));

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
	// Создание нового SID для дочернего процесса
	pid_t sid = setsid();
	// Структура для создания сервера приложения
	struct sockaddr_in sin;
	// Создаем новую базу
	struct event_base * base = event_base_new();
	// Очищаем всю структуру
	memset(&sin, 0, sizeof(sin));
	// This is an INET address
	sin.sin_family = AF_INET;
	// Listen on 0.0.0.0
	sin.sin_addr.s_addr = htonl(0);
	// Listen on the given port.
	sin.sin_port = htons(SERVER_PORT);
	// Устанавливаем сигнал установки подключения
	signal(SIGPIPE, sigpipe_handler);	// Сигнал обрыва соединения во время записи
	signal(SIGCHLD, sigchld_handler);	// Дочерний процесс убит
	signal(SIGTERM, sigterm_handler);	// Процесс убит
	signal(SIGHUP, sighup_handler);		// Терминал потерял связь
	// Вешаем приложение на порт
	struct evconnlistener * listener = evconnlistener_new_bind(base, accept_connect, NULL, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1, (struct sockaddr *) &sin, sizeof(sin));
	// Если подключение не удалось
	if(!listener){
		// Выводим сообщение об ошибке
		perror("Couldn't create listener");
		// Выходим
		return 1;
	}
	// Устанавливаем обработчик на получение ошибок
	evconnlistener_set_error_cb(listener, accept_error);
	// Активируем перебор базы событий
	event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY);
	// Удаляем базу
	event_base_free(base);
	// Выходим
	return 0;
}