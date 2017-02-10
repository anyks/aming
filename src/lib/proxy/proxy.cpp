/* МОДУЛЬ ПРОКСИ СЕРВЕРА ANYKS */
/*
*	автор:				Юрий Николаевич Лобарев
*	skype:				efrantick
*	телефон:			+7(920)672-33-22
*	авторские права:	Все права принадлежат автору © Юрий Лобарев, 2017
*/

#include "proxy/proxy.h"

// Устанавливаем область видимости
using namespace std;

/**
 * signal_log Функция вывода значения сигнала в лог
 * @param num номер сигнала
 * @param ctx объект прокси сервера
 */
void Proxy::signal_log(int num, void * ctx){
	// Получаем объект прокси сервера
	Proxy * proxy = reinterpret_cast <Proxy *> (ctx);
	// Если подключение не передано
	if(proxy != NULL){
		// Определяем данные сигнала
		switch(num){
			case SIGABRT:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Process abort signal [%d]", num);									break;
			case SIGALRM:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Alarm clock [%d]", num);											break;
			case SIGBUS:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Access to an undefined portion of a memory object [%d]", num);	break;
			case SIGCHLD:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Child process terminated, stopped, or continued [%d]", num);		break;
			case SIGCONT:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Continue executing, if stopped [%d]", num);						break;
			case SIGFPE:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Erroneous arithmetic operation [%d]", num);						break;
			case SIGHUP:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Hangup [%d]", num);												break;
			case SIGILL:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Illegal instruction [%d]", num);									break;
			case SIGINT:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Terminal interrupt signal [%d]", num);							break;
			case SIGKILL:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Kill (cannot be caught or ignored) [%d]", num);					break;
			case SIGPIPE:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Write on a pipe with no one to read it [%d]", num);				break;
			case SIGQUIT:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Terminal quit signal [%d]", num);									break;
			case SIGSEGV:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Invalid memory reference [%d]", num);								break;
			case SIGSTOP:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Stop executing (cannot be caught or ignored) [%d]", num);			break;
			case SIGTERM:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Termination signal [%d]", num);									break;
			case SIGTSTP:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Terminal stop signal [%d]", num);									break;
			case SIGTTIN:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Background process attempting read [%d]", num);					break;
			case SIGTTOU:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Background process attempting write [%d]", num);					break;
			case SIGUSR1:	proxy->sys->log->write(LOG_MESSAGE, 0, "[=] User-defined signal 1 [%d]", num);								break;
			case SIGUSR2:	proxy->sys->log->write(LOG_MESSAGE, 0, "[=] User-defined signal 2 [%d]", num);								break;
			// case SIGPOLL: proxy->sys->log->write(LOG_ERROR, 0, "[-] Pollable event [%d]", num);										break;
			case SIGPROF:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Profiling timer expired [%d]", num);								break;
			case SIGSYS:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Bad sys call [%d]", num);											break;
			case SIGTRAP:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Trace / breakpoint trap [%d]", num);								break;
			case SIGURG:	proxy->sys->log->write(LOG_ERROR, 0, "[-] High bandwidth data is available at a socket [%d]", num);			break;
			case SIGVTALRM:	proxy->sys->log->write(LOG_ERROR, 0, "[-] Virtual timer expired [%d]", num);								break;
			case SIGXCPU:	proxy->sys->log->write(LOG_ERROR, 0, "[-] CPU time limit exceeded [%d]", num);								break;
			case SIGXFSZ:	proxy->sys->log->write(LOG_ERROR, 0, "[-] File size limit exceeded [%d]", num);								break;
		}
	}
	// Выходим
	return;
}
/**
 * clear_fantoms Функция чистки фантомных процессов
 * @param signal сигнал
 * @param ctx    объект прокси сервера
 */
void Proxy::clear_fantoms(int signal, void * ctx){
	// Получаем данные прокси
	Proxy * proxy = reinterpret_cast <Proxy *> (ctx);
	// Если подключение не передано
	if(proxy != NULL){
		// Логируем сообщение о сигнале
		signal_log(signal, proxy);
		// Получаем данные пидов созданных балансером
		System::Pids pids = proxy->sys->readPids();
		// Убиваем все дочерние процессы балансера
		for(u_int i = 0; i < pids.len; i++) kill(pids.pids[i], SIGTERM);
		// Если это родительский пид
		if(proxy->cpid || proxy->mpid){
			// Удаляем дочерний воркер
			if(proxy->cpid) kill(proxy->cpid, SIGTERM);
			if(proxy->mpid) kill(proxy->mpid, SIGTERM);
			// Если существует объект OS то удаляем пид
			if(proxy->sys->os != NULL) proxy->sys->os->rmPid(EXIT_FAILURE);
		}
	}
	// Выходим
	return;
}
/**
 * siginfo_cb Функция обработки информационных сигналов
 * @param fd    файловый дескриптор (сокет)
 * @param event возникшее событие
 * @param ctx   объект прокси сервера
 */
void Proxy::siginfo_cb(evutil_socket_t fd, short event, void * ctx){
	// Получаем объект сигнала
	SignalBuffer * buffer = reinterpret_cast <SignalBuffer *> (ctx);
	// Если подключение не передано
	if(buffer != NULL){
		// Логируем сообщение о сигнале
		signal_log(buffer->signal, buffer->proxy);
	}
	// Выходим
	return;
}
/**
 * siguser_cb Функция обработки пользовательских сигналов
 * @param fd    файловый дескриптор (сокет)
 * @param event возникшее событие
 * @param ctx   объект прокси сервера
 */
void Proxy::siguser_cb(evutil_socket_t fd, short event, void * ctx){
	// Получаем объект сигнала
	SignalBuffer * buffer = reinterpret_cast <SignalBuffer *> (ctx);
	// Если подключение не передано
	if(buffer != NULL){
		// Логируем сообщение о сигнале
		signal_log(buffer->signal, buffer->proxy);
		// Получаем данные прокси
		Proxy * proxy = reinterpret_cast <Proxy *> (buffer->proxy);
		// Если подключение не передано
		if(proxy != NULL){
			// Получаем данные пидов созданных балансером
			System::Pids pids = proxy->sys->readPids();
			// Отправляем идентификаторы балансеру
			proxy->sys->sendPids(pids.pids, pids.len);
			// Убиваем все дочерние процессы балансера
			for(u_int i = 0; i < pids.len; i++) kill(pids.pids[i], SIGTERM);
		}
	}
	// Выходим
	return;
}
/**
 * sigchld_cb Функция обработки отключения дочерних процессов
 * @param fd    файловый дескриптор (сокет)
 * @param event возникшее событие
 * @param ctx   объект прокси сервера
 */
/*
void Proxy::sigchld_cb(evutil_socket_t fd, short event, void * ctx){
	// Получаем объект сигнала
	SignalBuffer * buffer = reinterpret_cast <SignalBuffer *> (ctx);
	// Если подключение не передано
	if(buffer != NULL){
		// Логируем сообщение о сигнале
		signal_log(buffer->signal, buffer->proxy);
		// Избавляемся от зависших процессов
		while(waitpid(-1, NULL, WNOHANG) > 0);
	}
	// Выходим
	return;
}
*/
/**
 * sigsegv_cb Функция обработки сигналов с дампом памяти
 * @param fd    файловый дескриптор (сокет)
 * @param event возникшее событие
 * @param ctx   объект прокси сервера
 */
void Proxy::sigsegv_cb(evutil_socket_t fd, short event, void * ctx){
	// Получаем объект сигнала
	SignalBuffer * buffer = reinterpret_cast <SignalBuffer *> (ctx);
	// Если подключение не передано
	if(buffer != NULL){
		// Выполняем очистку дочерних процессов
		clear_fantoms(buffer->signal, buffer->proxy);
		// перепосылка сигнала
		signal(buffer->signal, SIG_DFL);
		// Выходим
		exit(SIGQUIT);
	}
	// Выходим
	return;
}
/**
 * sigexit_cb Функция обработки сигналов безусловного завершения работы
 * @param fd    файловый дескриптор (сокет)
 * @param event возникшее событие
 * @param ctx   объект прокси сервера
 */
void Proxy::sigexit_cb(evutil_socket_t fd, short event, void * ctx){
	// Получаем объект сигнала
	SignalBuffer * buffer = reinterpret_cast <SignalBuffer *> (ctx);
	// Если подключение не передано
	if(buffer != NULL){
		// Выполняем очистку дочерних процессов
		clear_fantoms(buffer->signal, buffer->proxy);
		// Выходим
		exit(EXIT_SUCCESS);
	}
	// Выходим
	return;
}
/**
 * create_proxy Функция создания прокси-сервера
 */
void Proxy::create_proxy(){
	// Выводим приглашение
	this->sys->log->welcome();
	// Выполняем запуск приложения от имени пользователя
	this->sys->os->privBind();
	// Установим максимальное кол-во дискрипторов которое можно открыть
	this->sys->os->setFdLimit();
	// Создаем объект для http прокси-сервера
	HttpProxy http(this->sys);
}
/**
 * run_worker Функция запуска воркера
 */
void Proxy::run_worker(){
	// Запускаем воркер
	switch(this->cpid = fork()){
		// Если поток не создан
		case -1: {
			// Сообщаем что произошла ошибка потока
			perror("fork");
			// Выходим из потока
			exit(EXIT_FAILURE);
		} break;
		// Если это дочерний поток значит все нормально и продолжаем работу
		case 0: create_proxy(); break;
		// Если это мастер процесс
		default: {
			// Статус выхода процесса
			int status;
			// Выводим в лог сообщение
			this->sys->log->write(LOG_MESSAGE, 900, "[+] start balancer proccess, pid = %d", this->cpid);
			// Зацикливаем ожидание завершения дочернего процесса
			do {
				// Ожидаем завершение работы дочернего процесса
				pid_t pid = waitpid(this->cpid, &status, WUNTRACED | WCONTINUED);
				// Если дочерний процесс не определен тогда выходим
				if(pid < 0){
					perror("waitpid");
					exit(EXIT_FAILURE);
				}
				// Если дочерний процесс просто вышел
				if(WIFEXITED(status)){
					// Выводим в лог сообщение
					this->sys->log->write(LOG_ERROR, 0, "exited, pid = %d, status = %d", this->cpid, WEXITSTATUS(status));
					// Если это безусловное завершение работы
					if(WEXITSTATUS(status) == SIGSTOP) exit(SIGSTOP);
				}
				// Если дочерний процесс убит
				else if(WIFSIGNALED(status)) this->sys->log->write(LOG_ERROR, 0, "killed by pid = %d, signal %d", this->cpid, WTERMSIG(status));
				// Если дочерний процесс остановлен
				else if(WIFSTOPPED(status)) this->sys->log->write(LOG_ERROR, 0, "stopped by pid = %d, signal %d", this->cpid, WSTOPSIG(status));
				// Если дочерний процесс прислал сообщение что нужно продолжить
				// else if(WIFCONTINUED(status)) this->sys->log->write(LOG_ERROR, 0, "continued");
			// Продолжаем до тех пор пока статус не освободится
			} while(!WIFEXITED(status) && !WIFSIGNALED(status));
			// Получаем данные пидов созданных балансером
			System::Pids pids = this->sys->readPids();
			// Убиваем все дочерние процессы балансера
			for(u_int i = 0; i < pids.len; i++) kill(pids.pids[i], SIGTERM);
			// Перезапускаем дочерний процесс
			run_worker();
			// Выходим из приложения
			exit(EXIT_SUCCESS);
		}
	}
}
/**
 * Proxy Конструктор
 * @param configfile адрес конфигурационного файла
 */
Proxy::Proxy(string configfile){
	// Создаем объект конфигурации
	this->sys = new System(configfile);
	// Если запуск должен быть в виде демона
	if(!this->sys->config->proxy.debug
	&& this->sys->config->proxy.daemon){
		// Ответвляемся от родительского процесса
		pid_t pid = fork();
		// Если пид не создан тогда выходим
		if(pid < 0) this->sys->os->rmPid(EXIT_FAILURE);
		// Если с PID'ом все получилось, то родительский процесс можно завершить.
		if(pid > 0) this->sys->os->rmPid(EXIT_SUCCESS);
		// Изменяем файловую маску
		umask(0);
		// Здесь можно открывать любые журналы
		// Создание нового SID для дочернего процесса
		pid_t sid = setsid();
		// Если идентификатор сессии дочернего процесса не существует
		if(sid < 0) this->sys->os->rmPid(EXIT_FAILURE);
		// Изменяем текущий рабочий каталог
		if((chdir("/")) < 0) this->sys->os->rmPid(EXIT_FAILURE);
		// Закрываем стандартные файловые дескрипторы
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		// Создаем pid файл
		this->sys->os->mkPid();
	}
	// Если режим отладки не включен
	if(!this->sys->config->proxy.debug){
		// Создаем воркер управляющий балансером
		this->mpid = fork();
		// Определяем тип созданного воркера
		switch(this->mpid){
			// Если поток не создан
			case -1: {
				// Сообщаем что произошла ошибка потока
				perror("fork");
				// Выходим из потока
				exit(EXIT_FAILURE);
			} break;
			// Запускаем воркер который следит за балансером
			case 0: run_worker(); break;
			default: {
				// Выводим в лог сообщение
				this->sys->log->write(LOG_MESSAGE, 500, "[+] start master proccess, pid = %d", getpid());
				this->sys->log->write(LOG_MESSAGE, 700, "[+] start slave proccess, pid = %d", this->mpid);
				// Создаем базу событий
				this->base = event_base_new();
				// Создаем буфер сигнала
				// Сигналы пользовательские
				this->siguser.push_back({SIGUSR1, this});
				// Сигналы информирования
				this->siginfo.push_back({SIGPIPE, this});
				// Сигналы точного выхода из приложения с удалением пида и удалением дочерних процессов
				this->sigexit.push_back({SIGHUP, this});
				this->sigexit.push_back({SIGINT, this});
				this->sigexit.push_back({SIGQUIT, this});
				this->sigexit.push_back({SIGTERM, this});
				this->sigexit.push_back({SIGTSTP, this});
				// this->sigexit.push_back({SIGSTOP, this});
				this->sigexit.push_back({SIGTTIN, this});
				this->sigexit.push_back({SIGTTOU, this});
				// Сигналы точного выхода из приложения с удалением пида и созданием дампа памяти
				this->sigsegv.push_back({SIGILL, this});
				this->sigsegv.push_back({SIGTRAP, this});
				this->sigsegv.push_back({SIGFPE, this});
				this->sigsegv.push_back({SIGBUS, this});
				this->sigsegv.push_back({SIGSEGV, this});
				this->sigsegv.push_back({SIGSYS, this});
				this->sigsegv.push_back({SIGXCPU, this});
				this->sigsegv.push_back({SIGXFSZ, this});
				// Создаем событие
				// Добавляем в сигналы все информационные
				for(u_int i = 0; i < this->siginfo.size(); i++)
					// Добавляем сигналы
					this->signals.push_back(
						evsignal_new(
							this->base,
							this->siguser[i].signal,
							&Proxy::siguser_cb,
							&this->siguser[i]
						)
					);
				// Добавляем в сигналы все информационные
				for(u_int i = 0; i < this->siginfo.size(); i++)
					// Добавляем сигналы
					this->signals.push_back(
						evsignal_new(
							this->base,
							this->siginfo[i].signal,
							&Proxy::siginfo_cb,
							&this->siginfo[i]
						)
					);
				// Добавляем в сигналы все для простого выхода
				for(u_int i = 0; i < this->sigexit.size(); i++)
					// Добавляем сигналы
					this->signals.push_back(
						evsignal_new(
							this->base,
							this->sigexit[i].signal,
							&Proxy::sigexit_cb,
							&this->sigexit[i]
						)
					);
				// Добавляем в сигналы все для принудительного выхода с созданием дампа ядра
				for(u_int i = 0; i < this->sigsegv.size(); i++)
					// Добавляем сигналы
					this->signals.push_back(
						evsignal_new(
							this->base,
							this->sigsegv[i].signal,
							&Proxy::sigsegv_cb,
							&this->sigsegv[i]
						)
					);
				// Активируем ловушки сигналов
				for(u_int i = 0; i < this->signals.size(); i++)
					// Активируем ловушку
					evsignal_add(this->signals[i], NULL);
				// Активируем перебор базы событий
				event_base_loop(this->base, EVLOOP_NO_EXIT_ON_EMPTY);
			}
		}
	// Запускаем прокси сервер в главном потоке
	} else create_proxy();
}
/**
 * ~Proxy Деструктор
 */
Proxy::~Proxy(){
	// Очищаем событие сигнала
	for(u_int i = 0; i < this->signals.size(); i++){
		// Очищаем ловушку
		if(this->signals[i] != NULL) event_free(this->signals[i]);
	}
	// Очищаем базу данных событий
	if(this->base != NULL) event_base_free(this->base);
	// Удаляем объект системных настроек
	delete this->sys;
}