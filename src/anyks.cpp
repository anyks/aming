/* СЕРВЕР ПРОКСИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2016
*/
// MacOS X
// g++ -std=c++11 -D_BSD_SOURCE -ggdb -Wall -pedantic -O3 -Werror=vla -lz -o ./bin/http ./proxy/http.cpp ./lib/http.cpp ./lib/base64.cpp ./lib/log.cpp ./anyks.cpp -I/usr/local/include /usr/local/opt/libevent/lib/libevent.a
// Linux
// g++ -std=c++11 -ggdb -Wall -pedantic -O3 -Werror=vla -lz -o ./bin/http ./proxy/http.cpp ./lib/http.cpp ./lib/base64.cpp ./lib/log.cpp ./anyks.cpp /usr/lib/x86_64-linux-gnu/libevent.a /usr/lib/gcc/x86_64-linux-gnu/4.9/libstdc++.a
// g++ -std=c++11 -ggdb -Wall -pedantic -O3 -Werror=vla -lz -o ./bin/http ./proxy/http.cpp ./lib/http.cpp ./lib/base64.cpp ./lib/log.cpp ./anyks.cpp /usr/lib/x86_64-linux-gnu/libevent.a /usr/lib/x86_64-linux-gnu/5/libstdc++.a
// FreeBSD
// clang++ -std=c++11 -D_BSD_SOURCE -ggdb -Wall -pedantic -O3 -Werror=vla -lz -o ./bin/http ./proxy/http.cpp ./lib/http.cpp ./lib/base64.cpp ./lib/log.cpp ./anyks.cpp -I/usr/local/include /usr/local/lib/libevent.a
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
#include "./lib/log.h"
#include "./proxy/http.h"

// Название и версия прокси-сервера
#define APP_NAME "ANYKS"
#define APP_VERSION "1.0"
#define APP_COPYRIGHT "ANYKS LLC"
#define APP_SITE "http://anyks.com"
#define APP_EMAIL "info@anyks.com"
#define APP_SUPPORT "support@anyks.com"
#define APP_AUTHOR "forman"

// Порты по умолчанию
#define HTTP_PORT 8080
#define SOCKS5_PORT 1080

// Максимальное количество открытых сокетов (по дефолту в системе 1024)
#define MAX_FDS 1024 * 10
// Адрес файла pid
#define PID_FILE "/var/run/anyksProxy.pid"

// Пиды дочернего воркера
pid_t cpid = -1;
// Объект log модуля
LogApp * log = NULL;

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
		case SIGABRT:	log->write(LOG_ERROR, "Сигнал посылаемый функцией abort() [%d]", signum);						break;
		case SIGALRM:	log->write(LOG_ERROR, "Сигнал истечения времени, заданного alarm() [%d]", signum);				break;
		case SIGBUS:	log->write(LOG_ERROR, "Неправильное обращение в физическую память [%d]", signum);				break;
		case SIGCHLD:	log->write(LOG_ERROR, "Дочерний процесс завершен или остановлен [%d]", signum);					break;
		case SIGCONT:	log->write(LOG_ERROR, "Продолжить выполнение ранее остановленного процесса [%d]", signum);		break;
		case SIGFPE:	log->write(LOG_ERROR, "Ошибочная арифметическая операция [%d]", signum);						break;
		case SIGHUP:	log->write(LOG_ERROR, "Закрытие терминала [%d]", signum);										break;
		case SIGILL:	log->write(LOG_ERROR, "Недопустимая инструкция процессора [%d]", signum);						break;
		case SIGINT:	log->write(LOG_ERROR, "Сигнал прерывания (Ctrl-C) с терминала [%d]", signum);					break;
		case SIGKILL:	log->write(LOG_ERROR, "Безусловное завершение [%d]", signum);									break;
		case SIGPIPE:	log->write(LOG_ERROR, "Запись в разорванное соединение (пайп, сокет) [%d]", signum);			break;
		case SIGQUIT:	log->write(LOG_ERROR, "Сигнал «Quit» с терминала (Ctrl-\\) [%d]", signum);						break;
		case SIGSEGV:	log->write(LOG_ERROR, "Нарушение при обращении в память [%d]", signum);							break;
		case SIGSTOP:	log->write(LOG_ERROR, "Остановка выполнения процесса [%d]", signum);							break;
		case SIGTERM:	log->write(LOG_ERROR, "Сигнал завершения (сигнал по умолчанию для утилиты kill) [%d]", signum);	break;
		case SIGTSTP:	log->write(LOG_ERROR, "Сигнал остановки с терминала (Ctrl-Z) [%d]", signum);					break;
		case SIGTTIN:	log->write(LOG_ERROR, "Попытка чтения с терминала фоновым процессом [%d]", signum);				break;
		case SIGTTOU:	log->write(LOG_ERROR, "Попытка записи на терминал фоновым процессом [%d]", signum);				break;
		case SIGUSR1:	log->write(LOG_ERROR, "Пользовательский сигнал № 1 [%d]", signum);								break;
		case SIGUSR2:	log->write(LOG_ERROR, "Пользовательский сигнал № 2 [%d]", signum);								break;
		// case SIGPOLL: log->write(LOG_ERROR, "Событие, отслеживаемое poll() [%d]", signum);							break;
		case SIGPROF:	log->write(LOG_ERROR, "Истечение таймера профилирования [%d]", signum);							break;
		case SIGSYS:	log->write(LOG_ERROR, "Неправильный системный вызов [%d]", signum);								break;
		case SIGTRAP:	log->write(LOG_ERROR, "Ловушка трассировки или брейкпоинт [%d]", signum);						break;
		case SIGURG:	log->write(LOG_ERROR, "На сокете получены срочные данные [%d]", signum);						break;
		case SIGVTALRM:	log->write(LOG_ERROR, "Истечение «виртуального таймера» [%d]", signum);							break;
		case SIGXCPU:	log->write(LOG_ERROR, "Процесс превысил лимит процессорного времени [%d]", signum);				break;
		case SIGXFSZ:	log->write(LOG_ERROR, "Процесс превысил допустимый размер файла [%d]", signum);					break;
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
	HttpProxy * http = new HttpProxy(log);
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
			// Выводим приглашение
			log->welcome(
				APP_NAME,		// название приложения
				"anyks",		// пользовательское название
				APP_VERSION,	// версия приложения
				"0.0.0.0",		// хост на котором поднято приложение
				true,			// активация IPv4
				false,			// активация IPv6
				true,			// активация обмена сжатыми данными
				true,			// сжимать полученные не сжатые данные
				true,			// активация умного прокси
				true,			// активация постоянных подключений
				true,			// тип поднятого прокси
				true,			// тип поднятого прокси
				true,			// активация коннект прокси
				-1,				// максимальное количество подключений
				HTTP_PORT,		// порт http прокси
				SOCKS5_PORT,	// порт socks5 прокси
				APP_COPYRIGHT,	// копирайт автора прокси
				APP_SITE,		// сайт автора прокси
				APP_EMAIL,		// адрес электронной почты автора
				APP_SUPPORT,	// адрес электронной почты службы поддержки
				APP_AUTHOR		// ник или имя автора
			);
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
				if(WIFEXITED(status)) log->write(LOG_ERROR, "exited, pid = %d, status = %d", cpid, WEXITSTATUS(status));
				// Если дочерний процесс убит
				else if(WIFSIGNALED(status)) log->write(LOG_ERROR, "killed by pid = %d, signal %d", cpid, WTERMSIG(status));
				// Если дочерний процесс остановлен
				else if(WIFSTOPPED(status)) log->write(LOG_ERROR, "stopped by pid = %d, signal %d", cpid, WSTOPSIG(status));
				// Если дочерний процесс прислал сообщение что нужно продолжить
				else if(WIFCONTINUED(status)) log->write(LOG_ERROR, "continued");
			// Продолжаем до тех пор пока статус не освободится
			} while(!WIFEXITED(status) && !WIFSIGNALED(status));
			// Перезапускаем дочерний процесс
			run_worker();
			// Выходим из приложения
			exit(EXIT_SUCCESS);
		}
	}
}
/**
 * main Главная функция приложения
 * @param  argc длина массива параметров
 * @param  argv массив параметров
 * @return      код выхода из приложения
 */
int main(int argc, char * argv[]){
	// Активируем локаль
	setlocale(LC_ALL, "");
	// Создаем модуль лога
	log = new LogApp(TOLOG_FILES | TOLOG_CONSOLE, "anyks", "/Volumes/Data/Work/proxy/src");
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
	// Удаляем лог
	delete log;
	// Выходим
	return 0;
}