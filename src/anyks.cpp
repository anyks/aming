/* СЕРВЕР ПРОКСИ ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2016
*/
// MacOS X
// clang++ -Wall -O3 -pedantic -ggdb -g -std=c++11 -Werror=vla -lz -lpthread -o ./bin/http ./proxy/http.cpp ./lib/http/http.cpp ./lib/base64/base64.cpp ./lib/log/log.cpp ./lib/osopt/osopt.cpp ./lib/ini/ini.cpp ./lib/config/conf.cpp ./anyks.cpp -I/usr/local/include /usr/local/opt/libevent/lib/libevent.a
// Linux
// g++ -std=c++11 -ggdb -Wall -pedantic -O3 -Werror=vla -lz -lpthread -o ./bin/http ./proxy/http.cpp ./lib/http/http.cpp ./lib/base64/base64.cpp ./lib/log/log.cpp ./lib/osopt/osopt.cpp ./lib/ini/ini.cpp ./lib/config/conf.cpp ./anyks.cpp /usr/lib/x86_64-linux-gnu/libevent.a /usr/lib/gcc/x86_64-linux-gnu/4.9/libstdc++.a
// g++ -std=c++11 -ggdb -Wall -pedantic -O3 -Werror=vla -lz -lpthread -o ./bin/http ./proxy/http.cpp ./lib/http/http.cpp ./lib/base64/base64.cpp ./lib/log/log.cpp ./lib/osopt/osopt.cpp ./lib/ini/ini.cpp ./lib/config/conf.cpp ./anyks.cpp /usr/lib/x86_64-linux-gnu/libevent.a /usr/lib/x86_64-linux-gnu/5/libstdc++.a
// FreeBSD
// clang++ -std=c++11 -D_BSD_SOURCE -ggdb -Wall -pedantic -O3 -Werror=vla -lz -lpthread -o ./bin/http ./proxy/http.cpp ./lib/http/http.cpp ./lib/base64/base64.cpp ./lib/log/log.cpp ./lib/osopt/osopt.cpp ./lib/ini/ini.cpp ./lib/config/conf.cpp ./anyks.cpp -I/usr/local/include /usr/local/lib/libevent.a
// Debug:
// ulimit -c unlimited
// ./bin/http /Volumes/Data/Work/proxy/src/config.ini
// gdb ./bin/http ./http.core
// $ lldb --core "/cores/core.xxxxx"
//   (lldb) bt all
// Отладка в реальном режиме времени
// lldb ./bin/http /Volumes/Data/Work/proxy/src/config.ini
// lldb r
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <sys/resource.h>
#include "./lib/log/log.h"
#include "./lib/osopt/osopt.h"
#include "./lib/config/conf.h"
#include "./proxy/http.h"

// Пиды дочернего воркера
pid_t cpid = -1;
// Объект log модуля
LogApp * logfile = NULL;
// Объект конфигурационного файла
Config * config = NULL;
// Объект взаимодействия с ОС
OsOpt * osopt = NULL;
/**
 * signal_log Функция вывода значения сигнала в лог
 * @param signum номер сигнала
 */
void signal_log(int signum){
	// Определяем данные сигнала
	switch(signum){
		case SIGABRT:	logfile->write(LOG_ERROR, "Process abort signal [%d]", signum);									break;
		case SIGALRM:	logfile->write(LOG_ERROR, "Alarm clock [%d]", signum);											break;
		case SIGBUS:	logfile->write(LOG_ERROR, "Access to an undefined portion of a memory object [%d]", signum);	break;
		case SIGCHLD:	logfile->write(LOG_ERROR, "Child process terminated, stopped, or continued [%d]", signum);		break;
		case SIGCONT:	logfile->write(LOG_ERROR, "Continue executing, if stopped [%d]", signum);						break;
		case SIGFPE:	logfile->write(LOG_ERROR, "Erroneous arithmetic operation [%d]", signum);						break;
		case SIGHUP:	logfile->write(LOG_ERROR, "Hangup [%d]", signum);												break;
		case SIGILL:	logfile->write(LOG_ERROR, "Illegal instruction [%d]", signum);									break;
		case SIGINT:	logfile->write(LOG_ERROR, "Terminal interrupt signal [%d]", signum);							break;
		case SIGKILL:	logfile->write(LOG_ERROR, "Kill (cannot be caught or ignored) [%d]", signum);					break;
		case SIGPIPE:	logfile->write(LOG_ERROR, "Write on a pipe with no one to read it [%d]", signum);				break;
		case SIGQUIT:	logfile->write(LOG_ERROR, "Terminal quit signal [%d]", signum);									break;
		case SIGSEGV:	logfile->write(LOG_ERROR, "Invalid memory reference [%d]", signum);								break;
		case SIGSTOP:	logfile->write(LOG_ERROR, "Stop executing (cannot be caught or ignored) [%d]", signum);			break;
		case SIGTERM:	logfile->write(LOG_ERROR, "Termination signal [%d]", signum);									break;
		case SIGTSTP:	logfile->write(LOG_ERROR, "Terminal stop signal [%d]", signum);									break;
		case SIGTTIN:	logfile->write(LOG_ERROR, "Background process attempting read [%d]", signum);					break;
		case SIGTTOU:	logfile->write(LOG_ERROR, "Background process attempting write [%d]", signum);					break;
		case SIGUSR1:	logfile->write(LOG_ERROR, "User-defined signal 1 [%d]", signum);								break;
		case SIGUSR2:	logfile->write(LOG_ERROR, "User-defined signal 2 [%d]", signum);								break;
		// case SIGPOLL: logfile->write(LOG_ERROR, "Pollable event [%d]", signum);										break;
		case SIGPROF:	logfile->write(LOG_ERROR, "Profiling timer expired [%d]", signum);								break;
		case SIGSYS:	logfile->write(LOG_ERROR, "Bad system call [%d]", signum);										break;
		case SIGTRAP:	logfile->write(LOG_ERROR, "Trace / breakpoint trap [%d]", signum);								break;
		case SIGURG:	logfile->write(LOG_ERROR, "High bandwidth data is available at a socket [%d]", signum);			break;
		case SIGVTALRM:	logfile->write(LOG_ERROR, "Virtual timer expired [%d]", signum);								break;
		case SIGXCPU:	logfile->write(LOG_ERROR, "CPU time limit exceeded [%d]", signum);								break;
		case SIGXFSZ:	logfile->write(LOG_ERROR, "File size limit exceeded [%d]", signum);								break;
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
		if(osopt != NULL) osopt->rmPid(EXIT_FAILURE);
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
	if((cpid > 0) && (osopt != NULL)){
		// Удаляем pid файл
		osopt->rmPid(EXIT_FAILURE);
	}
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
	osopt->setFdLimit();
	// Выводим приглашение
	logfile->welcome();
	// Создаем объект для http прокси-сервера
	HttpProxy http = HttpProxy(logfile, config);
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
			exit(0);
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
				if(pid < 0){
					perror("waitpid");
					exit(EXIT_FAILURE);
				}
				// Если дочерний процесс просто вышел
				if(WIFEXITED(status)) logfile->write(LOG_ERROR, "exited, pid = %d, status = %d", cpid, WEXITSTATUS(status));
				// Если дочерний процесс убит
				else if(WIFSIGNALED(status)) logfile->write(LOG_ERROR, "killed by pid = %d, signal %d", cpid, WTERMSIG(status));
				// Если дочерний процесс остановлен
				else if(WIFSTOPPED(status)) logfile->write(LOG_ERROR, "stopped by pid = %d, signal %d", cpid, WSTOPSIG(status));
				// Если дочерний процесс прислал сообщение что нужно продолжить
				else if(WIFCONTINUED(status)) logfile->write(LOG_ERROR, "continued");
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
	// Активируем локаль приложения
	setlocale(LC_ALL, "");
	// setlocale(LC_ALL, "en_US.UTF-8");
	// Получаем адрес конфигурационного файла
	string configfile = (argc >= 2 ? argv[1] : "");
	// Создаем объект конфигурации
	config = new Config(configfile);
	// Создаем модуль лога
	logfile = new LogApp(config, TOLOG_FILES | TOLOG_CONSOLE);
	// Устанавливаем настройки операционной системы
	osopt = new OsOpt(logfile, config);
	// Выполняем запуск приложения от имени пользователя
	osopt->privBind();
	// Если запуск должен быть в виде демона
	if(!config->proxy.debug && config->proxy.daemon){
		// Ответвляемся от родительского процесса
		pid_t pid = fork();
		// Если пид не создан тогда выходим
		if(pid < 0) osopt->rmPid(EXIT_FAILURE);
		// Если с PID'ом все получилось, то родительский процесс можно завершить.
		if(pid > 0) osopt->rmPid(EXIT_SUCCESS);
		// Изменяем файловую маску
		umask(0);
		// Здесь можно открывать любые журналы
		// Создание нового SID для дочернего процесса
		pid_t sid = setsid();
		// Если идентификатор сессии дочернего процесса не существует
		if(sid < 0) osopt->rmPid(EXIT_FAILURE);
		// Изменяем текущий рабочий каталог
		if((chdir("/")) < 0) osopt->rmPid(EXIT_FAILURE);
		// Закрываем стандартные файловые дескрипторы
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		// Создаем pid файл
		osopt->mkPid();
	}
	// Если режим отладки не включен
	if(!config->proxy.debug){
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
	// Запускаем прокси сервер в главном потоке
	} else create_proxy();
	// Удаляем объект взаимодействия с ОС
	delete osopt;
	// Удаляем лог
	delete logfile;
	// Удаляем конфиг
	delete config;
	// Выходим
	return 0;
}