/* СЕРВЕР ПРОКСИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2016
*/
// MacOS X
// g++ -std=c++11 -D_BSD_SOURCE -ggdb -Wall -pedantic -O3 -Werror=vla -o ./bin/http ./proxy/http.cpp ./lib/http.cpp ./lib/base64.cpp ./anyks.cpp -I/usr/local/include /usr/local/opt/libevent/lib/libevent.a
// Linux
// g++ -std=c++11 -ggdb -Wall -pedantic -O3 -Werror=vla -o ./bin/http ./proxy/http.cpp ./lib/http.cpp ./lib/base64.cpp ./anyks.cpp /usr/lib/x86_64-linux-gnu/libevent.a /usr/lib/gcc/x86_64-linux-gnu/4.9/libstdc++.a
// g++ -std=c++11 -ggdb -Wall -pedantic -O3 -Werror=vla -o ./bin/http ./proxy/http.cpp ./lib/http.cpp ./lib/base64.cpp ./anyks.cpp /usr/lib/x86_64-linux-gnu/libevent.a /usr/lib/x86_64-linux-gnu/5/libstdc++.a
// FreeBSD
// clang++ -std=c++11 -D_BSD_SOURCE -ggdb -Wall -pedantic -O3 -Werror=vla -o ./bin/http ./proxy/http.cpp ./lib/http.cpp ./lib/base64.cpp ./anyks.cpp -I/usr/local/include /usr/local/lib/libevent.a
// Debug:
// ulimit -c unlimited
// ./bin/http
// gdb ./bin/http ./http.core
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/signal.h>
#include "./proxy/http.h"

// Максимальное количество открытых сокетов (по дефолту в системе 1024)
#define MAX_FDS 1024 * 10
// Адрес файла pid
#define PID_FILE "/var/run/anyksProxy.pid"

// Пиды дочернего воркера
pid_t cpid = -1;

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
 * set_pidfile Функция создания pid файла
 * @param filename название файла pid
 */
void set_pidfile(const char * filename){
	// Открываем файл на запись
	FILE * f = fopen(filename, "w");
	// Если файл открыт
	if(f){
		// Записываем в файл pid процесса
		fprintf(f, "%u", getpid());
		// Закрываем файл
		fclose(f);
	}
}
/**
 * rm_pidfile Функция удаления pid файла
 * @param filename название файла pid
 * @param ext      тип ошибки
 */
void rm_pidfile(const char * filename, int ext){
	// Удаляем файл
	remove(filename);
	// Выходим из приложения
	exit(ext);
}
/**
 * signal_log Функция вывода значения сигнала в лог
 * @param signum номер сигнала
 */
void signal_log(int signum){
	// Определяем данные сигнала
	switch(signum){
		case SIGABRT:	printf("Сигнал посылаемый функцией abort() [%d]\n", signum);						break;
		case SIGALRM:	printf("Сигнал истечения времени, заданного alarm() [%d]\n", signum);				break;
		case SIGBUS:	printf("Неправильное обращение в физическую память [%d]\n", signum);				break;
		case SIGCHLD:	printf("Дочерний процесс завершен или остановлен [%d]\n", signum);					break;
		case SIGCONT:	printf("Продолжить выполнение ранее остановленного процесса [%d]\n", signum);		break;
		case SIGFPE:	printf("Ошибочная арифметическая операция [%d]\n", signum);							break;
		case SIGHUP:	printf("Закрытие терминала [%d]\n", signum);										break;
		case SIGILL:	printf("Недопустимая инструкция процессора [%d]\n", signum);						break;
		case SIGINT:	printf("Сигнал прерывания (Ctrl-C) с терминала [%d]\n", signum);					break;
		case SIGKILL:	printf("Безусловное завершение [%d]\n", signum);									break;
		case SIGPIPE:	printf("Запись в разорванное соединение (пайп, сокет) [%d]\n", signum);				break;
		case SIGQUIT:	printf("Сигнал «Quit» с терминала (Ctrl-\\) [%d]\n", signum);						break;
		case SIGSEGV:	printf("Нарушение при обращении в память [%d]\n", signum);							break;
		case SIGSTOP:	printf("Остановка выполнения процесса [%d]\n", signum);								break;
		case SIGTERM:	printf("Сигнал завершения (сигнал по умолчанию для утилиты kill) [%d]\n", signum);	break;
		case SIGTSTP:	printf("Сигнал остановки с терминала (Ctrl-Z) [%d]\n", signum);						break;
		case SIGTTIN:	printf("Попытка чтения с терминала фоновым процессом [%d]\n", signum);				break;
		case SIGTTOU:	printf("Попытка записи на терминал фоновым процессом [%d]\n", signum);				break;
		case SIGUSR1:	printf("Пользовательский сигнал № 1 [%d]\n", signum);								break;
		case SIGUSR2:	printf("Пользовательский сигнал № 2 [%d]\n", signum);								break;
		// case SIGPOLL:	printf("Событие, отслеживаемое poll() [%d]\n", signum);							break;
		case SIGPROF:	printf("Истечение таймера профилирования [%d]\n", signum);							break;
		case SIGSYS:	printf("Неправильный системный вызов [%d]\n", signum);								break;
		case SIGTRAP:	printf("Ловушка трассировки или брейкпоинт [%d]\n", signum);						break;
		case SIGURG:	printf("На сокете получены срочные данные [%d]\n", signum);							break;
		case SIGVTALRM:	printf("Истечение «виртуального таймера» [%d]\n", signum);							break;
		case SIGXCPU:	printf("Процесс превысил лимит процессорного времени [%d]\n", signum);				break;
		case SIGXFSZ:	printf("Процесс превысил допустимый размер файла [%d]\n", signum);					break;
	}
}
/**
 * sigpipe_handler Функция обработки сигнала SIGPIPE
 * @param signum номер сигнала
 */
void sigpipe_handler(int signum){
	// Логируем сообщение о сигнале
	signal_log(signum);
}
/**
 * sigchld_handler Функция обработки сигнала о появившемся зомби процессе SIGCHLD
 * @param signum номер сигнала
 */
void sigchld_handler(int signum) {
	// Логируем сообщение о сигнале
	signal_log(signum);
	// Избавляемся от зависших процессов
	while(waitpid(-1, NULL, WNOHANG) > 0);
}
/**
 * sigterm_handler Функция обработки сигналов завершения процессов SIGTERM
 * @param signum номер сигнала
 */
void sigterm_handler(int signum){
	// Логируем сообщение о сигнале
	signal_log(signum);
	// Если это родительский пид
	if(cpid > 0){
		// Удаляем дочерний воркер
		kill(cpid, SIGTERM);
		// Удаляем pid файл
		rm_pidfile(PID_FILE, EXIT_FAILURE);
	}
	// Выходим
	exit(0);
}
/**
 * sigsegv_handler Функция обработки сигналов ошибки сегментации SIGSEGV
 * @param signum номер сигнала
 */
void sigsegv_handler(int signum){
	// Логируем сообщение о сигнале
	signal_log(signum);
	// Если это родительский пид, удаляем pid файл
	if(cpid > 0) rm_pidfile(PID_FILE, EXIT_FAILURE);
	// перепосылка сигнала
	signal(signum, SIG_DFL);
	// Выходим
	exit(3);
}
/**
 * create_proxy Функция создания прокси-сервера
 */
void create_proxy(){
	// Установим максимальное кол-во дискрипторов которое можно открыть
	set_fd_limit(MAX_FDS);
	// Создаем объект для http прокси-сервера
	// HttpProxy * http = new HttpProxy("anyks", "0.0.0.0", SERVER_PORT, -1, -1, -1, READ_TIMEOUT, WRITE_TIMEOUT, KEEP_ALIVE_TIMEOUT, OPT_SMART);
	HttpProxy * http = new HttpProxy();
	// Очищаем выделенный объект
	delete http;
}
/**
 * run_worker Функция запуска воркера
 */
void run_worker(){
	// Запускаем воркер
	switch(cpid = fork()){
		// Если поток не создан
		case -1: {
			// Сообщаем что произошла ошибка потока
			perror("fork");
			// Выходим из потока
			exit(1);
		} break;
		// Если это дочерний поток значит все нормально и продолжаем работу
		case 0: create_proxy(); break;
		// Если это мастер процесс
		default: {
			// Статус выхода процесса
			int status;
			// Зацикливаем ожидание завершения дочернего процесса
			do {
				// Ожидаем завершение работы дочернего процесса
				pid_t pid = waitpid(cpid, &status, WUNTRACED | WCONTINUED);
				// Если дочерний процесс не определен тогда выходим
				if(pid == -1){
					perror("waitpid");
					exit(EXIT_FAILURE);
				}
				// Если дочерний процесс просто вышел
				if(WIFEXITED(status)) printf("exited, pid = %d, status = %d\n", cpid, WEXITSTATUS(status));
				// Если дочерний процесс убит
				else if(WIFSIGNALED(status)) printf("killed by pid = %d, signal %d\n", cpid, WTERMSIG(status));
				// Если дочерний процесс остановлен
				else if(WIFSTOPPED(status)) printf("stopped by pid = %d, signal %d\n", cpid, WSTOPSIG(status));
				// Если дочерний процесс прислал сообщение что нужно продолжить
				else if(WIFCONTINUED(status)) printf("continued\n");
			// Продолжаем до тех пор пока статус не освободится
			} while(!WIFEXITED(status) && !WIFSIGNALED(status));
			// Перезапускаем дочерний процесс
			run_worker();
			// Выходим из приложения
			exit(EXIT_SUCCESS);
		}
	}
}


/*
void test(short options){
	if(options & OPT_CONNECT) cout << " OPT_CONNECT " << endl;
	if(options & OPT_NAME) cout << " OPT_NAME " << endl;
	if(options & OPT_GZIP) cout << " OPT_GZIP " << endl;
	if(options & OPT_SMART) cout << " OPT_SMART " << endl;
	if(options & OPT_KEEPALIVE) cout << " OPT_KEEPALIVE " << endl;
	if(options & OPT_LOG) cout << " OPT_LOG " << endl;
	if(options & OPT_PGZIP) cout << " OPT_PGZIP " << endl;
}
*/

/**
 * main Главная функция приложения
 * @param  argc длина массива параметров
 * @param  argv массив параметров
 * @return      код выхода из приложения
 */
int main(int argc, char * argv[]){
	/*
	// Наши ID процесса и сессии
	pid_t pid, sid;
	// Ответвляемся от родительского процесса
	pid = fork();
	// Если пид не создан тогда выходим
	if(pid < 0) rm_pidfile(PID_FILE, EXIT_FAILURE);
	// Если с PID'ом все получилось, то родительский процесс можно завершить.
	if(pid > 0) rm_pidfile(PID_FILE, EXIT_SUCCESS);
	// Изменяем файловую маску
	umask(0);
	// Здесь можно открывать любые журналы
	// Создание нового SID для дочернего процесса
	sid = setsid();
	// Если идентификатор сессии дочернего процесса не существует
	if(sid < 0) rm_pidfile(PID_FILE, EXIT_FAILURE);
	// Изменяем текущий рабочий каталог
	if((chdir("/")) < 0) rm_pidfile(PID_FILE, EXIT_FAILURE);
	// Закрываем стандартные файловые дескрипторы
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	// Создаем pid файл
	set_pidfile(PID_FILE);
	*/
	// Устанавливаем сигнал установки подключения
	signal(SIGPIPE, sigpipe_handler);	// Запись в разорванное соединение (пайп, сокет)
	// signal(SIGCHLD, sigchld_handler);// Дочерний процесс завершен или остановлен
	signal(SIGSEGV, sigsegv_handler);	// Нарушение при обращении в память (Segmentation fault: 11)
	signal(SIGILL, sigsegv_handler);	// Недопустимая инструкция процессора (Illegal instruction: 4)
	signal(SIGBUS, sigsegv_handler);	// Неправильное обращение в физическую память
	signal(SIGFPE, sigsegv_handler);	// Ошибочная арифметическая операция
	signal(SIGSYS, sigsegv_handler);	// Неправильный системный вызов
	signal(SIGTRAP, sigsegv_handler);	// Ловушка трассировки или брейкпоинт
	signal(SIGXCPU, sigsegv_handler);	// Процесс превысил лимит процессорного времени
	signal(SIGXFSZ, sigsegv_handler);	// Процесс превысил допустимый размер файла
	signal(SIGTERM, sigterm_handler);	// Сигнал завершения (сигнал по умолчанию для утилиты kill)
	signal(SIGINT, sigterm_handler);	// Сигнал прерывания (Ctrl-C) с терминала
	signal(SIGQUIT, sigterm_handler);	// Сигнал «Quit» с терминала (Ctrl-\)
	signal(SIGTSTP, sigterm_handler);	// Сигнал остановки с терминала (Ctrl-Z)
	signal(SIGHUP, sigterm_handler);	// Закрытие терминала
	signal(SIGTTIN, sigterm_handler);	// Попытка чтения с терминала фоновым процессом
	signal(SIGTTOU, sigterm_handler);	// Попытка записи на терминал фоновым процессом
	// Запускаем воркер
	run_worker();
	// Выходим
	return 0;
}